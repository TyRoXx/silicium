#include <silicium/process.hpp>
#include <boost/test/unit_test.hpp>

namespace Si
{
	BOOST_AUTO_TEST_CASE(run_process_cat)
	{
		process_output output = run_process("/usr/bin/which", {"which"}, boost::filesystem::current_path(), true);
		BOOST_CHECK_EQUAL(0, output.exit_status);
		std::string const expected = "/usr/bin/which\n";
		BOOST_REQUIRE(output.stdout);
		BOOST_CHECK_EQUAL(expected, std::string(begin(*output.stdout), end(*output.stdout)));
	}
}
