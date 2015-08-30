#include "../../cdm/application/using_gtest/cdm.hpp"
#include <silicium/cmake.hpp>
#include <boost/test/unit_test.hpp>

namespace
{
	Si::absolute_path const this_file = *Si::absolute_path::create(__FILE__);
	Si::absolute_path const test_cdm = *Si::parent(this_file);
	Si::absolute_path const test = *Si::parent(test_cdm);
	Si::absolute_path const silicium = *Si::parent(test);

	void configure(
		Si::absolute_path const &build_dir,
		Si::absolute_path const &install_dir,
		Si::sink<char, Si::success> &output
		)
	{
		Si::absolute_path const using_gtest_source = silicium / Si::relative_path("cdm/application/using_gtest");
		Si::absolute_path const gtest_source = silicium / Si::relative_path("cdm/original_sources/gtest-1.7.0");
		cdm::gtest_paths const gtest_installed = cdm::install_gtest(gtest_source, build_dir, install_dir, Si::cmake_exe);
		std::vector<Si::os_string> arguments;
		arguments.push_back(Si::os_string(SILICIUM_SYSTEM_LITERAL("-DGTEST_INCLUDE_DIRS=")) + to_os_string(gtest_installed.include));
		arguments.push_back(Si::os_string(SILICIUM_SYSTEM_LITERAL("-DGTEST_LIBRARIES=")) + to_os_string(gtest_installed.library) + SILICIUM_SYSTEM_LITERAL(";") + to_os_string(gtest_installed.library_main));
		arguments.push_back(Si::to_os_string(using_gtest_source.c_str()));
		Si::recreate_directories(build_dir, Si::throw_);
		BOOST_REQUIRE_EQUAL(0, Si::run_process(Si::cmake_exe, arguments, build_dir, output));
	}
}

BOOST_AUTO_TEST_CASE(test_using_gtest)
{
	Si::absolute_path const tmp = Si::temporary_directory(Si::throw_);
	Si::absolute_path const build_dir = tmp / *Si::path_segment::create("build");
	Si::absolute_path const install_dir = tmp / *Si::path_segment::create("install");
	Si::recreate_directories(build_dir, Si::throw_);
	Si::recreate_directories(install_dir, Si::throw_);
	auto output = Si::Sink<char, Si::success>::erase(Si::ostream_ref_sink(std::cerr));
	configure(build_dir, install_dir, output);
	{
		std::vector<Si::os_string> arguments;
		arguments.push_back(SILICIUM_SYSTEM_LITERAL("--build"));
		arguments.push_back(SILICIUM_SYSTEM_LITERAL("."));
		BOOST_REQUIRE_EQUAL(0, Si::run_process(Si::cmake_exe, arguments, build_dir, output));
	}
	{
		std::vector<Si::os_string> arguments;
		BOOST_REQUIRE_EQUAL(0, Si::run_process(build_dir / *Si::path_segment::create("using_gtest"), arguments, build_dir, output));
	}
}
