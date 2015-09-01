#include <cdm_cppnetlib.hpp>
#include <cdm_gtest.hpp>
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
	Si::absolute_path const tmp = Si::temporary_directory(Si::throw_);
	Si::absolute_path const build_dir = tmp / *Si::path_segment::create("build");
	Si::absolute_path const install_dir = tmp / *Si::path_segment::create("install");
	Si::recreate_directories(build_dir, Si::throw_);
	Si::recreate_directories(install_dir, Si::throw_);
	cdm::gtest_paths const built = cdm::install_gtest(source, build_dir, install_dir, Si::cmake_exe);
	BOOST_CHECK_EQUAL(install_dir / *Si::path_segment::create("include"), built.include);
	BOOST_CHECK(boost::filesystem::exists(built.include.to_boost_path()));
	BOOST_CHECK(boost::filesystem::exists(built.library.to_boost_path()));
	BOOST_CHECK(boost::filesystem::exists(built.library_main.to_boost_path()));
}

BOOST_AUTO_TEST_CASE(test_cdm_cppnetlib)
{
	Si::absolute_path const source = silicium / Si::relative_path("cdm/original_sources/cpp-netlib-0.11.2-final");
	Si::absolute_path const tmp = Si::temporary_directory(Si::throw_);
	Si::absolute_path const install_dir = tmp / *Si::path_segment::create("install");
	Si::recreate_directories(install_dir, Si::throw_);
	cdm::cppnetlib_paths const built = cdm::install_cppnetlib(source, install_dir, Si::cmake_exe);
	BOOST_CHECK_EQUAL(install_dir, built.cmake_prefix_path);
	BOOST_CHECK(boost::filesystem::exists(install_dir.to_boost_path() / "cppnetlibTargets.cmake"));
	BOOST_CHECK(boost::filesystem::exists(install_dir.to_boost_path() / "libs/network/src/libcppnetlib-client-connections.so"));
	BOOST_CHECK(boost::filesystem::exists(install_dir.to_boost_path() / "libs/network/src/libcppnetlib-server-parsers.so"));
	BOOST_CHECK(boost::filesystem::exists(install_dir.to_boost_path() / "libs/network/src/libcppnetlib-uri.so"));
}

