#include <modules/cdm_cppnetlib.hpp>
#include <modules/cdm_gtest.hpp>
#include <silicium/cmake.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/test/unit_test.hpp>

namespace
{
	Si::absolute_path const this_file = *Si::absolute_path::create(__FILE__);
	Si::absolute_path const test_cdm = *Si::parent(this_file);
	Si::absolute_path const test = *Si::parent(test_cdm);
	Si::absolute_path const silicium = *Si::parent(test);
}

BOOST_AUTO_TEST_CASE(test_cdm_gtest)
{
	Si::absolute_path const source = silicium / Si::relative_path("cdm/original_sources/gtest-1.7.0");
	Si::absolute_path const tmp = Si::temporary_directory();
	Si::absolute_path const build_dir = tmp / *Si::path_segment::create("build");
	Si::absolute_path const install_dir = tmp / *Si::path_segment::create("install");
	Si::throw_if_error(Si::recreate_directories(build_dir));
	Si::throw_if_error(Si::recreate_directories(install_dir));
	cdm::gtest_paths const built = cdm::install_gtest(source, build_dir, install_dir, Si::cmake_exe);
	BOOST_CHECK_EQUAL(install_dir / *Si::path_segment::create("include"), built.include);
	BOOST_CHECK(boost::filesystem::exists(built.include.to_boost_path()));
	BOOST_CHECK(boost::filesystem::exists(built.library.to_boost_path()));
	BOOST_CHECK(boost::filesystem::exists(built.library_main.to_boost_path()));
}

BOOST_AUTO_TEST_CASE(test_cdm_cppnetlib)
{
	Si::absolute_path const source = silicium / Si::relative_path("cdm/original_sources/cpp-netlib-0.11.2-final");
	Si::absolute_path const tmp = Si::temporary_directory();
	Si::absolute_path const install_dir = tmp / *Si::path_segment::create("install");
	Si::throw_if_error(Si::recreate_directories(install_dir));
	cdm::cppnetlib_paths const built = cdm::install_cppnetlib(source, install_dir, Si::cmake_exe);
	BOOST_CHECK_EQUAL(install_dir, built.cmake_prefix_path);
	BOOST_CHECK(boost::filesystem::exists(install_dir.to_boost_path() / "cppnetlibTargets.cmake"));
	BOOST_CHECK(boost::filesystem::exists(install_dir.to_boost_path() / "libs/network/src/libcppnetlib-client-connections.so"));
	BOOST_CHECK(boost::filesystem::exists(install_dir.to_boost_path() / "libs/network/src/libcppnetlib-server-parsers.so"));
	BOOST_CHECK(boost::filesystem::exists(install_dir.to_boost_path() / "libs/network/src/libcppnetlib-uri.so"));
}

BOOST_AUTO_TEST_CASE(test_using_gtest)
{
	Si::absolute_path const using_gtest_source = silicium / Si::relative_path("cdm/application/using_gtest");
	Si::absolute_path const gtest_source = silicium / Si::relative_path("cdm/original_sources/gtest-1.7.0");
	Si::absolute_path const tmp = Si::temporary_directory();
	Si::absolute_path const build_dir = tmp / *Si::path_segment::create("build");
	Si::absolute_path const install_dir = tmp / *Si::path_segment::create("install");
	Si::throw_if_error(Si::recreate_directories(build_dir));
	Si::throw_if_error(Si::recreate_directories(install_dir));
	cdm::gtest_paths const gtest_installed = cdm::install_gtest(gtest_source, build_dir, install_dir, Si::cmake_exe);
	auto output = Si::Sink<char, Si::success>::erase(Si::ostream_ref_sink(std::cerr));
	{
		std::vector<Si::os_string> arguments;
		arguments.push_back(Si::os_string(SILICIUM_SYSTEM_LITERAL("-DGTEST_INCLUDE_DIRS=")) + gtest_installed.include.c_str());
		arguments.push_back(Si::os_string(SILICIUM_SYSTEM_LITERAL("-DGTEST_LIBRARIES=")) + to_os_string(gtest_installed.library) + SILICIUM_SYSTEM_LITERAL(";") + to_os_string(gtest_installed.library_main));
		arguments.push_back(Si::to_os_string(using_gtest_source.c_str()));
		Si::throw_if_error(Si::recreate_directories(build_dir));
		BOOST_REQUIRE_EQUAL(0, Si::run_process(Si::cmake_exe, arguments, build_dir, output));
	}
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
