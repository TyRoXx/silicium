#include <silicium/process.hpp>
#include <silicium/to_unique.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/filesystem/operations.hpp>

namespace Si
{
	BOOST_AUTO_TEST_CASE(run_process_3_which)
	{
		auto const results = run_process("/usr/bin/which", {"which"}, "/");
		BOOST_CHECK_EQUAL(0, results.exit_code);
		std::string const expected = "/usr/bin/which\n";
		BOOST_CHECK_EQUAL(expected, std::string(begin(results.output), end(results.output)));
	}

	BOOST_AUTO_TEST_CASE(run_process_1_which)
	{
		process_parameters parameters;
		parameters.executable = "/usr/bin/which";
		parameters.arguments.emplace_back("which");
		parameters.current_path = boost::filesystem::current_path();
		std::vector<char> stdout;
		parameters.stdout = to_unique(make_iterator_sink<char>(std::back_inserter(stdout)));
		int result = run_process(parameters);
		BOOST_CHECK_EQUAL(0, result);
		std::string const expected = "/usr/bin/which\n";
		BOOST_CHECK_EQUAL(expected, std::string(begin(stdout), end(stdout)));
	}
}
