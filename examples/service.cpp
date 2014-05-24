#include <silicium/process.hpp>
#include <silicium/build_result.hpp>
#include <silicium/directory_allocator.hpp>
#include <silicium/to_unique.hpp>
#include <silicium/git/repository.hpp>
#include <silicium/read_file.hpp>
#include <boost/asio.hpp>
#include <fstream>

namespace
{
	struct tcp_trigger
	{
		explicit tcp_trigger(boost::asio::io_service &io, boost::asio::ip::tcp::endpoint address)
			: m_acceptor(io, address, true)
		{
		}

		template <class Handler>
		void async_wait(Handler handler)
		{
			if (m_accepting)
			{
				m_accepting->close();
			}
			m_accepting.reset(new boost::asio::ip::tcp::socket(m_acceptor.get_io_service()));
			m_acceptor.async_accept(*m_accepting, [this, handler](boost::system::error_code error)
			{
				if (error)
				{
					//TODO
				}
				else
				{
					m_accepting->shutdown(boost::asio::ip::tcp::socket::shutdown_both);
					handler();
				}
			});
		}

		void cancel()
		{
			m_acceptor.cancel();
		}

	private:

		boost::asio::ip::tcp::acceptor m_acceptor;
		std::unique_ptr<boost::asio::ip::tcp::socket> m_accepting;
	};

	boost::filesystem::path make_last_built_file_name(
			boost::filesystem::path const &output_location,
			std::string const &branch)
	{
		return output_location / (branch + ".lastbuild.txt");
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

	void write_file(boost::filesystem::path const &name, char const *data, std::size_t size)
	{
		std::ofstream file(name.string(), std::ios::binary);
		if (!file)
		{
			throw std::runtime_error("Could not open file " + name.string() + " for writing");
		}
		file.write(data, size);
		if (!file)
		{
			throw std::runtime_error("Could not write to file " + name.string());
		}
	}

	void set_last_built(
			git_repository &repository,
			boost::filesystem::path const &last_build_file_name,
			git_reference const &built)
	{
		char const * const name = git_reference_name(&built);
		git_oid commit_id;
		Si::git::throw_if_libgit2_error(git_reference_name_to_id(&commit_id, &repository, name));
		auto const id_str = Si::git::format_oid(commit_id);
		write_file(last_build_file_name, id_str.data(), id_str.size());
	}

	struct directory_builder
	{
		virtual ~directory_builder()
		{
		}

		virtual std::unique_ptr<Si::sink<char>> begin_artifact(std::string const &name) = 0;
		virtual std::unique_ptr<directory_builder> create_subdirectory(std::string const &name) = 0;
	};

	Si::build_result make(
			boost::filesystem::path const &source,
			boost::filesystem::path const &build_dir,
			directory_builder &artifacts)
	{
		{
			Si::process_parameters parameters;
			parameters.executable = "/usr/bin/cmake";
			parameters.arguments = {source.string()};
			parameters.current_path = build_dir;
			parameters.stdout = artifacts.begin_artifact("cmake.log");
			int const exit_code = Si::run_process(parameters);
			if (exit_code != 0)
			{
				return Si::build_failure{"CMake failed"};
			}
		}
		{
			Si::process_parameters parameters;
			parameters.executable = "/usr/bin/make";
			parameters.arguments = {"-j4"};
			parameters.current_path = build_dir;
			parameters.stdout = artifacts.begin_artifact("make.log");
			int const exit_code = Si::run_process(parameters);
			if (exit_code != 0)
			{
				return Si::build_failure{"make failed"};
			}
		}
		{
			Si::process_parameters parameters;
			parameters.executable = build_dir / "test/test";
			parameters.current_path = build_dir / "test";
			parameters.stdout = artifacts.begin_artifact("test.log");
			int const exit_code = Si::run_process(parameters);
			if (exit_code != 0)
			{
				return Si::build_failure{"tests failed"};
			}
		}
		return Si::build_success{};
	}

	struct filesystem_directory_builder : directory_builder
	{
		explicit filesystem_directory_builder(boost::filesystem::path destination)
			: m_destination(std::move(destination))
		{
		}

		virtual std::unique_ptr<Si::sink<char>> begin_artifact(std::string const &name) override
		{
			auto const file_name = m_destination / name;
			std::unique_ptr<std::ostream> file(new std::ofstream(file_name.string(), std::ios::binary));
			if (!*file)
			{
				throw std::runtime_error("Cannot open file for writing: " + file_name.string());
			}
			return std::unique_ptr<Si::sink<char>>(new Si::ostream_sink(std::move(file)));
		}

		virtual std::unique_ptr<directory_builder> create_subdirectory(std::string const &name) override
		{
			auto sub = m_destination / name;
			boost::filesystem::create_directories(sub);
			return std::unique_ptr<directory_builder>(new filesystem_directory_builder(sub));
		}

	private:

		boost::filesystem::path m_destination;
	};

	void clone(
			std::string const &branch,
			boost::filesystem::path const &source_location,
			boost::filesystem::path const &cloned_dir)
	{
		git_clone_options options = GIT_CLONE_OPTIONS_INIT;
		options.checkout_branch = branch.c_str(); //TODO: clone the reference directly without repeating the branch name
		Si::git::clone(source_location.string(), cloned_dir, &options);
	}

	Si::build_result build_commit(
			std::string const &branch,
			boost::filesystem::path const &source_location,
			boost::filesystem::path const &commit_dir,
			directory_builder &reports)
	{
		auto const cloned_dir = commit_dir / "source";
		clone(branch, source_location, cloned_dir);

		auto const build_dir = commit_dir / "build";
		boost::filesystem::create_directories(build_dir);
		return make(cloned_dir, build_dir, reports);
	}

	void check_build(
			boost::filesystem::path const &source_location,
			boost::filesystem::path const &workspace)
	{
		std::string const branch = "master";
		auto const source = Si::git::open_repository(source_location);
		auto const full_branch_name = ("refs/heads/" + branch);
		auto const ref_to_build = Si::git::lookup(*source, full_branch_name.c_str());
		if (!ref_to_build)
		{
			throw std::runtime_error(branch + " branch does not exist");
		}
		git_oid oid_to_build;
		Si::git::throw_if_libgit2_error(git_reference_name_to_id(&oid_to_build, source.get(), full_branch_name.c_str()));
		auto const results_repository = workspace / "results.git";
		auto const last_built_file_name = make_last_built_file_name(results_repository, branch);
		auto const ref_last_built = get_last_built(last_built_file_name);
		if (ref_last_built &&
			git_oid_equal(&*ref_last_built, &oid_to_build))
		{
			//nothing to do
			return;
		}

		auto const formatted_build_oid = Si::git::format_oid(oid_to_build);
		auto const temporary_location = workspace / formatted_build_oid;
		boost::filesystem::create_directories(temporary_location);

		filesystem_directory_builder results(results_repository);
		auto const reports = results.create_subdirectory(formatted_build_oid);
		build_commit(branch, source_location, temporary_location, *reports);
		set_last_built(*source, last_built_file_name, *ref_to_build);
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

	auto const build = [&]
	{
		try
		{
			check_build(source_location, workspace);
		}
		catch (std::exception const &ex)
		{
			//continue to run
			std::cerr << ex.what() << '\n';
		}
	};

	boost::asio::io_service io;
	tcp_trigger external_build_trigger(io, boost::asio::ip::tcp::endpoint(boost::asio::ip::address_v4(), 12345));
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
