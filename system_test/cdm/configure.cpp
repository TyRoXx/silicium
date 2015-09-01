#include "../../cdm/building_configure/configure.hpp"
#include <boost/test/unit_test.hpp>
#include <silicium/file_operations.hpp>
#include <silicium/sink/ostream_sink.hpp>

namespace
{
	Si::absolute_path const this_file = *Si::absolute_path::create(__FILE__);
	Si::absolute_path const test_cdm = *Si::parent(this_file);
	Si::absolute_path const test = *Si::parent(test_cdm);
	Si::absolute_path const silicium = *Si::parent(test);
	Si::absolute_path const using_gtest_source = silicium / Si::relative_path("cdm/application/using_gtest");
}

BOOST_AUTO_TEST_CASE(test_run_configure_command_line)
{
	Si::absolute_path const temporary_root = Si::temporary_directory(Si::throw_) / Si::relative_path("cdm_system_test");
	Si::recreate_directories(temporary_root, Si::throw_);
	Si::absolute_path const configure = temporary_root / Si::relative_path("configure");
	Si::absolute_path const &application = using_gtest_source;
	auto output = Si::Sink<char, Si::success>::erase(Si::ostream_ref_sink(std::cerr));
	Si::absolute_path const modules = temporary_root / Si::relative_path("modules");
	Si::create_directories(modules, Si::throw_);
	Si::absolute_path const application_build = temporary_root / Si::relative_path("application_build");
	Si::create_directories(application_build, Si::throw_);
	cdm::do_configure(temporary_root, modules, application, application_build, output);
}
