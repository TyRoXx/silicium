#include <cdm_cppnetlib.hpp>
#include <cdm_gtest.hpp>
#include <silicium/cmake.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/thread/thread.hpp>

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
	BOOST_CHECK_EQUAL(install_dir / *Si::path_segment::create("gtest") / *Si::path_segment::create("include"), built.include);
	BOOST_CHECK(boost::filesystem::exists(built.include.to_boost_path()));
	BOOST_CHECK(boost::filesystem::exists(built.library.to_boost_path()));
	BOOST_CHECK(boost::filesystem::exists(built.library_main.to_boost_path()));
}

BOOST_AUTO_TEST_CASE(test_cdm_cppnetlib)
{
	Si::absolute_path const source = silicium / Si::relative_path("cdm/original_sources/cpp-netlib-0.11.2-final");
	Si::absolute_path const tmp = Si::temporary_directory(Si::throw_);
	Si::absolute_path const modules = tmp / *Si::path_segment::create("cdm_modules");
	Si::recreate_directories(modules, Si::throw_);
	unsigned const make_parallelism =
#ifdef SILICIUM_TESTS_RUNNING_ON_TRAVIS_CI
		2;
#else
		boost::thread::hardware_concurrency();
#endif
	Si::absolute_path const build_dir = tmp / *Si::path_segment::create("build");
	Si::recreate_directories(build_dir, Si::throw_);
	cdm::cppnetlib_paths const built = cdm::install_cppnetlib(source, build_dir, modules, Si::cmake_exe, make_parallelism);
	BOOST_CHECK(Si::file_exists(built.cmake_prefix_path / Si::relative_path("cppnetlib/cppnetlibConfig.cmake"), Si::throw_));
	BOOST_CHECK(Si::file_exists(built.cmake_prefix_path / Si::relative_path("cppnetlib/cppnetlibConfigVersion.cmake"), Si::throw_));
	BOOST_CHECK(Si::file_exists(built.cmake_prefix_path / Si::relative_path("cppnetlib/cppnetlibTargets.cmake"), Si::throw_));
	BOOST_CHECK(Si::file_exists(built.cmake_prefix_path / Si::relative_path("cppnetlib/cppnetlibTargets-noconfig.cmake"), Si::throw_));
}
