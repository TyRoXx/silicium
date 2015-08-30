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
		Si::absolute_path const &module_temporaries,
		Si::absolute_path const &module_permanent,
		Si::absolute_path const &application_build_dir,
		Si::sink<char, Si::success> &output
		)
	{
		Si::absolute_path const using_gtest_source = silicium / Si::relative_path("cdm/application/using_gtest");
		Si::absolute_path const gtest_source = silicium / Si::relative_path("cdm/original_sources/gtest-1.7.0");
		cdm::gtest_paths const gtest_installed = cdm::install_gtest(gtest_source, module_temporaries, module_permanent, Si::cmake_exe);
		std::vector<Si::os_string> arguments;
		arguments.push_back(Si::os_string(SILICIUM_SYSTEM_LITERAL("-DGTEST_INCLUDE_DIRS=")) + to_os_string(gtest_installed.include));
		arguments.push_back(Si::os_string(SILICIUM_SYSTEM_LITERAL("-DGTEST_LIBRARIES=")) + to_os_string(gtest_installed.library) + SILICIUM_SYSTEM_LITERAL(";") + to_os_string(gtest_installed.library_main));
		arguments.push_back(Si::to_os_string(using_gtest_source.c_str()));
		BOOST_REQUIRE_EQUAL(0, Si::run_process(Si::cmake_exe, arguments, application_build_dir, output));
	}
}

BOOST_AUTO_TEST_CASE(test_using_gtest)
{
	Si::absolute_path const tmp = Si::temporary_directory(Si::throw_) / *Si::path_segment::create("cdm_test");
	Si::absolute_path const module_temporaries = tmp / *Si::path_segment::create("module_temporaries");
	Si::absolute_path const module_permanent = tmp / *Si::path_segment::create("module_permanent");
	Si::absolute_path const application_build_dir = tmp / *Si::path_segment::create("application_build_dir");
	Si::recreate_directories(module_temporaries, Si::throw_);
	Si::recreate_directories(module_permanent, Si::throw_);
	Si::recreate_directories(application_build_dir, Si::throw_);
	auto output = Si::Sink<char, Si::success>::erase(Si::ostream_ref_sink(std::cerr));
	configure(module_temporaries, module_permanent, application_build_dir, output);
	{
		std::vector<Si::os_string> arguments;
		arguments.push_back(SILICIUM_SYSTEM_LITERAL("--build"));
		arguments.push_back(SILICIUM_SYSTEM_LITERAL("."));
		BOOST_REQUIRE_EQUAL(0, Si::run_process(Si::cmake_exe, arguments, application_build_dir, output));
	}
	{
		std::vector<Si::os_string> arguments;
		BOOST_REQUIRE_EQUAL(0, Si::run_process(application_build_dir / *Si::path_segment::create("using_gtest"), arguments, application_build_dir, output));
	}
}
