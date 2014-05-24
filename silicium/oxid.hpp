#ifndef SILICIUM_OXID_HPP
#define SILICIUM_OXID_HPP

#include <silicium/build_result.hpp>
#include <silicium/directory_builder.hpp>
#include <functional>
#include <boost/filesystem/path.hpp>
#include <git2.h>

namespace Si
{
	namespace oxid
	{
		typedef std::function<Si::build_result (
				boost::filesystem::path const &cloned,
				boost::filesystem::path const &build,
				Si::directory_builder &artifacts
				)> test_runner;
		typedef std::function<std::string (Si::build_result result, git_commit const &source)> commit_message_formatter;

		void check_build(
				boost::filesystem::path const &source_location,
				boost::filesystem::path const &results_repository,
				std::string const &branch,
				boost::filesystem::path const &workspace,
				boost::filesystem::path const &git_executable,
				commit_message_formatter const &make_commit_message,
				test_runner const &run_tests);
	}
}

#endif
