#include <silicium/process.hpp>
#include <silicium/to_unique.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/filesystem/operations.hpp>

namespace Si
{
	BOOST_AUTO_TEST_CASE(run_process_1_which)
	{
		process_parameters parameters;
		parameters.executable = "/usr/bin/which";
		parameters.arguments.emplace_back("which");
		parameters.current_path = boost::filesystem::current_path();
		std::vector<char> stdout;
		auto sink = make_iterator_sink<char>(std::back_inserter(stdout));
		parameters.stdout = &sink;
		int result = run_process(parameters);
		BOOST_CHECK_EQUAL(0, result);
		std::string const expected = "/usr/bin/which\n";
		BOOST_CHECK_EQUAL(expected, std::string(begin(stdout), end(stdout)));
	}

	BOOST_AUTO_TEST_CASE(run_process_from_nonexecutable)
	{
		process_parameters parameters;
		parameters.executable = "does-not-exist";
		parameters.current_path = boost::filesystem::current_path();
		BOOST_CHECK_EXCEPTION(run_process(parameters), boost::system::system_error, [](boost::system::system_error const &e)
		{
			return e.code() == boost::system::error_code(ENOENT, boost::system::system_category());
		});
	}
}
