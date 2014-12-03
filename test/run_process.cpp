#include <silicium/process.hpp>
#include <silicium/to_unique.hpp>
#include <silicium/sink/iterator_sink.hpp>
#include <silicium/sink/virtualized_sink.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/filesystem/operations.hpp>

namespace Si
{
#ifndef _WIN32
	BOOST_AUTO_TEST_CASE(run_process_1_unix_which)
	{
		process_parameters parameters;
		parameters.executable = "/usr/bin/which";
		parameters.arguments.emplace_back("which");
		parameters.current_path = boost::filesystem::current_path();
		std::vector<char> out;
		auto sink = Si::virtualize_sink(make_iterator_sink<char>(std::back_inserter(out)));
		parameters.out = &sink;
		int result = run_process(parameters);
		BOOST_CHECK_EQUAL(0, result);
		std::string const expected = "/usr/bin/which\n";
		BOOST_CHECK_EQUAL(expected, std::string(begin(out), end(out)));
	}
#endif

#ifdef _WIN32
	BOOST_AUTO_TEST_CASE(run_process_1_win32_cmd)
	{
		process_parameters parameters;
		parameters.executable = "C:\\Windows\\System32\\where.exe";
		parameters.current_path = boost::filesystem::current_path();
		std::vector<char> out;
		auto sink = make_iterator_sink<char>(std::back_inserter(out));
		parameters.out = &sink;
		int result = run_process(parameters);
		BOOST_CHECK_EQUAL(2, result);
		std::size_t const windows7whereHelpSize = 1830;
		BOOST_CHECK_EQUAL(windows7whereHelpSize, out.size());
	}
#endif

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
