#include <silicium/process.hpp>
#include <silicium/build_result.hpp>
#include <silicium/to_unique.hpp>
#include <silicium/read_file.hpp>
#include <silicium/write_file.hpp>
#include <silicium/directory_builder.hpp>
#include <silicium/tcp_trigger.hpp>
#include <silicium/git/repository.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem/operations.hpp>

namespace
{
	boost::filesystem::path make_last_built_file_name(boost::filesystem::path const &output_location)
	{
		return output_location / ("lastbuild.txt");
	}

	boost::optional<git_oid> get_last_built(boost::filesystem::path const &last_build_file_name)
	{
		if (!boost::filesystem::exists(last_build_file_name))
		{
			return boost::none;
		}
		auto str = Si::read_file(last_build_file_name);
		git_oid id;
		Si::git::throw_if_libgit2_error(git_oid_fromstrn(&id, str.data(), str.size()));
		return id;
	}

	void set_last_built(
			boost::filesystem::path const &last_build_file_name,
			git_oid const &built)
	{
		auto const id_str = Si::git::format_oid(built);
		Si::write_file(last_build_file_name, id_str.data(), id_str.size());
	}

	void clone(
			std::string const &branch,
			std::string const &source_location,
			boost::filesystem::path const &cloned_dir)
	{
		git_clone_options options = GIT_CLONE_OPTIONS_INIT;
		options.checkout_branch = branch.c_str(); //TODO: clone the reference directly without repeating the branch name
		Si::git::clone(source_location, cloned_dir, &options);
	}

	void push(
			boost::filesystem::path const &repository,
			Si::sink<char> *git_log,
			std::string message)
	{
		{
			Si::process_parameters parameters;
			parameters.executable = "/usr/bin/git";
			parameters.arguments = {"add", "-A", "."};
			parameters.current_path = repository;
			parameters.stdout = git_log;
			int const exit_code = Si::run_process(parameters);
			if (exit_code != 0)
			{
				throw std::runtime_error{"git add failed"};
			}
		}

		{
			Si::process_parameters parameters;
			parameters.executable = "/usr/bin/git";
			parameters.arguments = {"commit", "-m", std::move(message)};
			parameters.current_path = repository;
			parameters.stdout = git_log;
			int const exit_code = Si::run_process(parameters);
			if (exit_code != 0)
			{
				throw std::runtime_error{"git commit failed"};
			}
		}

		{
			Si::process_parameters parameters;
			parameters.executable = "/usr/bin/git";
			parameters.arguments = {"push"};
			parameters.current_path = repository;
			parameters.stdout = git_log;
			int const exit_code = Si::run_process(parameters);
			if (exit_code != 0)
			{
				throw std::runtime_error{"git push failed"};
			}
		}
	}

	Si::build_result run_tests(
			boost::filesystem::path const &source,
			boost::filesystem::path const &build_dir,
			Si::directory_builder &artifacts,
			unsigned parallelization)
	{
		if (Si::run_process("/usr/bin/cmake", {source.string()}, build_dir, *artifacts.begin_artifact("cmake.log")) != 0)
		{
			return Si::build_failure{"CMake failed"};
		}
		if (Si::run_process("/usr/bin/make", {"-j" + boost::lexical_cast<std::string>(parallelization)}, build_dir, *artifacts.begin_artifact("make.log")) != 0)
		{
			return Si::build_failure{"make failed"};
		}
		if (Si::run_process("test/test", {}, build_dir, *artifacts.begin_artifact("test.log")) != 0)
		{
			return Si::build_failure{"tests failed"};
		}
		return Si::build_success{};
	}

	Si::build_result build_commit(
			std::string const &branch,
			std::string const &source_location,
			boost::filesystem::path const &commit_dir,
			Si::directory_builder &reports,
			unsigned parallelization)
	{
		auto const cloned_dir = commit_dir / "source";
		clone(branch, source_location, cloned_dir);

		auto const build_dir = commit_dir / "build";
		boost::filesystem::create_directories(build_dir);
		return run_tests(cloned_dir, build_dir, reports, parallelization);
	}

	boost::optional<git_oid> find_new_commit(
			git_repository &source,
			std::string const &branch,
			boost::filesystem::path const &results_repository,
			boost::filesystem::path const &last_built_file_name)
	{
		auto const full_branch_name = ("refs/heads/" + branch);
		auto const ref_to_build = Si::git::lookup(source, full_branch_name.c_str());
		if (!ref_to_build)
		{
			throw std::runtime_error(branch + " branch does not exist");
		}
		git_oid oid_to_build;
		Si::git::throw_if_libgit2_error(git_reference_name_to_id(&oid_to_build, &source, full_branch_name.c_str()));
		auto const ref_last_built = get_last_built(last_built_file_name);
		if (!ref_last_built || !git_oid_equal(&*ref_last_built, &oid_to_build))
		{
			return oid_to_build;
		}
		return boost::none;
	}

	void check_build(
			boost::filesystem::path const &source_location,
			std::string const &branch,
			boost::filesystem::path const &workspace,
			unsigned parallelization)
	{
		auto const results_repository = workspace / "results.git";
		auto const last_built_file_name = make_last_built_file_name(results_repository);
		auto const source = Si::git::open_repository(source_location);
		auto const new_commit = find_new_commit(*source, branch, results_repository, last_built_file_name);
		if (!new_commit)
		{
			return;
		}

		auto const formatted_build_oid = Si::git::format_oid(*new_commit);
		auto const temporary_location = workspace / formatted_build_oid;
		boost::filesystem::create_directories(temporary_location);

		Si::filesystem_directory_builder results(results_repository);
		auto const reports = results.create_subdirectory(formatted_build_oid);
		build_commit(branch, source_location.string(), temporary_location, *reports, parallelization);
		set_last_built(last_built_file_name, *new_commit);

		auto git_log = Si::make_file_sink(temporary_location / "git_commit.log");
		push(results_repository, git_log.get(), "built by silicium");
	}
}

int main(int argc, char **argv)
{
	if (argc < 3)
	{
		return 1;
	}
	boost::filesystem::path const source_location = argv[1];
	boost::filesystem::path const workspace = argv[2];
	unsigned parallelization = 1;
	if (argc >= 4)
	{
		parallelization = boost::lexical_cast<unsigned>(argv[3]);
	}

	auto const build = [&]
	{
		try
		{
			check_build(source_location, "master", workspace, parallelization);
		}
		catch (std::exception const &ex)
		{
			//continue to run
			std::cerr << ex.what() << '\n';
		}
	};

	boost::asio::io_service io;
	Si::tcp_trigger external_build_trigger(io, boost::asio::ip::tcp::endpoint(boost::asio::ip::address_v4(), 12345));
	std::function<void ()> wait_for_trigger;
	wait_for_trigger = [&wait_for_trigger, &external_build_trigger, &build]
	{
		external_build_trigger.async_wait([&wait_for_trigger, &build]
		{
			build();
			wait_for_trigger();
		});
	};
	wait_for_trigger();

	build();
	io.run();
}
