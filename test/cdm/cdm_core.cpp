#include <cdm_core/core.hpp>
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
