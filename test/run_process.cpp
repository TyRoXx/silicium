#include <silicium/process.hpp>
#include <boost/test/unit_test.hpp>

namespace Si
{
	BOOST_AUTO_TEST_CASE(run_process_cat)
	{
		process_output output = run_process("cat", {}, true);
		BOOST_CHECK_EQUAL(0, output.exit_status);
		BOOST_CHECK(std::vector<char>() == output.stdout);
	}
}
