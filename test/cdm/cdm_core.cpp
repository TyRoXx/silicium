#include <cdm_core/core.hpp>
#include <modules/cdm_cppnetlib.hpp>
#include <modules/cdm_gtest.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(cdm_core_trivial)
{
	auto const directory_containing_test_exe = Si::get_current_executable_path().get().to_boost_path().parent_path();
#ifdef _MSC_VER
	auto const build_type = directory_containing_test_exe.leaf();
#endif
	auto const description_library = directory_containing_test_exe.parent_path()
#ifdef _MSC_VER
		.parent_path()
#endif
		/ "cdm" / "websocketpp" /
#ifdef _MSC_VER
		build_type / "cdm_websocketpp.dll"
#else
		"libcdm_websocketpp.so"
#endif
		;
	cdm::dynamic_library_description const loaded = cdm::load_dynamic_library_description(*Si::absolute_path::create(description_library)).move_value();
	BOOST_CHECK_EQUAL("websocketpp", loaded.cached_description.name);
	BOOST_CHECK(!loaded.library.empty());
}

BOOST_AUTO_TEST_CASE(test_cdm_gtest)
{
	Si::absolute_path const this_file = *Si::absolute_path::create(__FILE__);
	Si::absolute_path const test_cdm = *Si::parent(this_file);
	Si::absolute_path const test = *Si::parent(test_cdm);
	Si::absolute_path const silicium = *Si::parent(test);
	Si::absolute_path const gtest_source = silicium / Si::relative_path("cdm/original_sources/gtest-1.7.0");
	Si::absolute_path const tmp = Si::temporary_directory();
	Si::absolute_path const build_dir = tmp / *Si::path_segment::create("build");
	Si::absolute_path const install_dir = tmp / *Si::path_segment::create("install");
	Si::throw_if_error(Si::recreate_directories(build_dir));
	Si::throw_if_error(Si::recreate_directories(install_dir));
	cdm::gtest_paths const built = cdm::install_gtest(gtest_source, build_dir, install_dir);
	BOOST_CHECK_EQUAL(install_dir / *Si::path_segment::create("include"), built.include);
	BOOST_CHECK(boost::filesystem::exists(built.include.to_boost_path()));
	BOOST_CHECK(boost::filesystem::exists(built.library.to_boost_path()));
	BOOST_CHECK(boost::filesystem::exists(built.library_main.to_boost_path()));
}

BOOST_AUTO_TEST_CASE(test_cdm_cppnetlib)
{
	Si::absolute_path const this_file = *Si::absolute_path::create(__FILE__);
	Si::absolute_path const test_cdm = *Si::parent(this_file);
	Si::absolute_path const test = *Si::parent(test_cdm);
	Si::absolute_path const silicium = *Si::parent(test);
	Si::absolute_path const source = silicium / Si::relative_path("cdm/original_sources/cpp-netlib-0.11.2-final");
	Si::absolute_path const tmp = Si::temporary_directory();
	Si::absolute_path const build_dir = tmp / *Si::path_segment::create("build");
	Si::absolute_path const install_dir = tmp / *Si::path_segment::create("install");
	Si::throw_if_error(Si::recreate_directories(build_dir));
	Si::throw_if_error(Si::recreate_directories(install_dir));
	cdm::cppnetlib_paths const built = cdm::install_cppnetlib(source, build_dir, install_dir);
	BOOST_CHECK_EQUAL(install_dir / *Si::path_segment::create("include"), built.include);
	BOOST_CHECK(boost::filesystem::exists(built.include.to_boost_path()));
	BOOST_CHECK(boost::filesystem::exists(built.uri_library.to_boost_path()));
	BOOST_CHECK(boost::filesystem::exists(built.client_connections_library.to_boost_path()));
	BOOST_CHECK(boost::filesystem::exists(built.server_parsers_library.to_boost_path()));
}
