#define CDM_CONFIGURE_NAMESPACE a0038871
#include "../../cdm/application/using_cppnetlib/cdm.hpp"
#include <boost/test/unit_test.hpp>

namespace
{
	Si::absolute_path const this_file = *Si::absolute_path::create(__FILE__);
	Si::absolute_path const test_cdm = *Si::parent(this_file);
	Si::absolute_path const test = *Si::parent(test_cdm);
	Si::absolute_path const silicium = *Si::parent(test);
}

BOOST_AUTO_TEST_CASE(test_using_cppnetlib)
{
	Si::absolute_path const app_source = silicium / Si::relative_path("cdm/application/using_cppnetlib");
	Si::absolute_path const tmp = Si::temporary_directory(Si::throw_) / *Si::path_segment::create("cdm_test");
	Si::absolute_path const module_temporaries = tmp / *Si::path_segment::create("module_temporaries");
	Si::absolute_path const module_permanent = tmp / *Si::path_segment::create("module_permanent");
	Si::absolute_path const application_build_dir = tmp / *Si::path_segment::create("application_build_dir");
	Si::recreate_directories(module_temporaries, Si::throw_);
	Si::recreate_directories(module_permanent, Si::throw_);
	Si::recreate_directories(application_build_dir, Si::throw_);
	auto output = Si::Sink<char, Si::success>::erase(Si::ostream_ref_sink(std::cerr));
	CDM_CONFIGURE_NAMESPACE::configure(module_temporaries, module_permanent, app_source, application_build_dir, output);
	{
		std::vector<Si::os_string> arguments;
		arguments.push_back(SILICIUM_SYSTEM_LITERAL("--build"));
		arguments.push_back(SILICIUM_SYSTEM_LITERAL("."));
		BOOST_REQUIRE_EQUAL(0, Si::run_process(Si::cmake_exe, arguments, application_build_dir, output));
	}
	{
		std::vector<Si::os_string> arguments;
		BOOST_REQUIRE_EQUAL(0, Si::run_process(application_build_dir / *Si::path_segment::create("using_cppnetlib"), arguments, application_build_dir, output));
	}
}
