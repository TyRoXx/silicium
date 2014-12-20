#include <silicium/git/oxid.hpp>
#include <silicium/read_file.hpp>
#include <silicium/write_file.hpp>
#include <silicium/run_process.hpp>
#include <silicium/sink/virtualized_sink.hpp>
#include <silicium/git/repository.hpp>
#include <silicium/sink/iterator_sink.hpp>
#include <boost/optional.hpp>
#include <boost/filesystem/operations.hpp>

namespace Si
{
	namespace oxid
	{
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
					boost::filesystem::path const &git_executable,
					std::string const &branch,
					std::string const &source_location,
					boost::filesystem::path const &cloned_dir)
			{
				std::vector<char> output;
				auto output_sink = virtualize_sink(make_container_sink(output));
				if (0 != run_process(git_executable, {"clone", "-b", branch, "--depth", "1", source_location, cloned_dir.string()}, boost::filesystem::current_path(), output_sink))
				{
					std::string what = "git clone failed";
					what += "\n";
					what.append(begin(output), end(output));
					throw std::runtime_error(std::move(what));
				}
			}

			void push(
					boost::filesystem::path const &git_executable,
					boost::filesystem::path const &repository,
					std::string message)
			{
				std::vector<char> output;
				auto output_sink = virtualize_sink(make_container_sink(output));
				auto const fail = [&output](std::string const &description)
				{
					std::string what = description;
					what += "\n";
					what.append(begin(output), end(output));
					throw std::runtime_error(std::move(what));
				};
				{
					Si::process_parameters parameters;
					parameters.executable = git_executable;
					parameters.arguments = {"add", "-A", "."};
					parameters.current_path = repository;
					parameters.out = &output_sink;
					int const exit_code = Si::run_process(parameters);
					if (exit_code != 0)
					{
						fail("git add failed");
					}
				}

				{
					Si::process_parameters parameters;
					parameters.executable = git_executable;
					parameters.arguments = {"commit", "-m", std::move(message)};
					parameters.current_path = repository;
					parameters.out = &output_sink;
					int const exit_code = Si::run_process(parameters);
					if (exit_code != 0)
					{
						fail("git commit failed");
					}
				}

				{
					Si::process_parameters parameters;
					parameters.executable = git_executable;
					parameters.arguments = {"push"};
					parameters.current_path = repository;
					parameters.out = &output_sink;
					int const exit_code = Si::run_process(parameters);
					if (exit_code != 0)
					{
						fail("git push failed");
					}
				}
			}

			Si::build_result build_commit(
					boost::filesystem::path const &git_executable,
					std::string const &branch,
					std::string const &source_location,
					boost::filesystem::path const &commit_dir,
					Si::directory_builder &reports,
					test_runner const &run_tests)
			{
				auto const cloned_dir = commit_dir / "source";
				clone(git_executable, branch, source_location, cloned_dir);

				auto const build_dir = commit_dir / "build";
				boost::filesystem::create_directories(build_dir);
				return run_tests(cloned_dir, build_dir, reports);
			}

			boost::optional<git_oid> find_new_commit(
					git_repository &source,
					std::string const &branch,
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
		}

		void check_build(
				boost::filesystem::path const &source_location,
				boost::filesystem::path const &results_repository,
				std::string const &branch,
				boost::filesystem::path const &workspace,
				boost::filesystem::path const &git_executable,
				commit_message_formatter const &make_commit_message,
				test_runner const &run_tests)
		{
			auto const last_built_file_name = make_last_built_file_name(results_repository);
			auto const source = Si::git::open_repository(source_location);
			auto const new_commit_id = find_new_commit(*source, branch, last_built_file_name);
			if (!new_commit_id)
			{
				return;
			}

			auto const new_commit = git::lookup_commit(*source, *new_commit_id);
			if (!new_commit)
			{
				throw std::runtime_error("Head of branch " + branch + " is not a commit");
			}

			auto const formatted_build_oid = Si::git::format_oid(*new_commit_id);
			auto const temporary_location = workspace / formatted_build_oid;
			boost::filesystem::create_directories(temporary_location);

			Si::filesystem_directory_builder results(results_repository);
			auto const reports = results.create_subdirectory(formatted_build_oid);
			auto const result = build_commit(git_executable, branch, source_location.string(), temporary_location, *reports, run_tests);

			//delete all the temporary files first
			boost::filesystem::remove_all(temporary_location);

			set_last_built(last_built_file_name, *new_commit_id);

			auto const commit_message = make_commit_message(result, *new_commit);
			push(git_executable, results_repository, commit_message);
		}
	}
}
