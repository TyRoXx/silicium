#include <silicium/process.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/filesystem/operations.hpp>

namespace Si
{
	BOOST_AUTO_TEST_CASE(run_process_which)
	{
		process_output output = run_process("/usr/bin/which", {"which"}, boost::filesystem::current_path(), true);
		BOOST_CHECK_EQUAL(0, output.exit_status);
		std::string const expected = "/usr/bin/which\n";
		BOOST_REQUIRE(output.stdout);
		BOOST_CHECK_EQUAL(expected, std::string(begin(*output.stdout), end(*output.stdout)));
	}

	template <class T>
	std::unique_ptr<T> to_unique(T value)
	{
		return std::unique_ptr<T>(new T(std::move(value)));
	}

	BOOST_AUTO_TEST_CASE(run_process_which_2)
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
