#include <silicium/run_process.hpp>
#include <silicium/to_unique.hpp>
#include <silicium/sink/iterator_sink.hpp>
#include <silicium/sink/virtualized_sink.hpp>
#include <silicium/source/range_source.hpp>
#include <boost/test/unit_test.hpp>
#if SILICIUM_HAS_RUN_PROCESS
#	include <boost/filesystem/operations.hpp>
#endif

namespace Si
{
#if SILICIUM_HAS_RUN_PROCESS
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
		auto sink = virtualize_sink(make_iterator_sink<char>(std::back_inserter(out)));
		parameters.out = &sink;
		int result = run_process(parameters);
		BOOST_CHECK_EQUAL(2, result);
		std::size_t const windows7whereHelpSize = 1830;
		BOOST_CHECK_EQUAL(windows7whereHelpSize, out.size());
	}
#endif

	namespace
	{
		Si::native_path_string const absolute_root(
	#ifdef _WIN32
			L"C:/"
	#else
			"/"
	#endif
		);
	}

	BOOST_AUTO_TEST_CASE(run_process_from_nonexecutable)
	{
		process_parameters parameters;
		parameters.executable = boost::filesystem::path(absolute_root.c_str()) / SILICIUM_SYSTEM_LITERAL("does-not-exist");
		parameters.current_path = boost::filesystem::current_path();
		BOOST_CHECK_EXCEPTION(run_process(parameters), boost::system::system_error, [](boost::system::system_error const &e)
		{
			return e.code() == boost::system::error_code(ENOENT, boost::system::system_category());
		});
	}

#ifndef _WIN32
	BOOST_AUTO_TEST_CASE(run_process_standard_input)
	{
		process_parameters parameters;
		parameters.executable = "/bin/cat";
		parameters.current_path = boost::filesystem::current_path();
		auto message = Si::make_c_str_range("Hello, cat");
		auto input = Si::Source<char>::erase(Si::make_range_source(message));
		parameters.in = &input;
		std::vector<char> output_buffer;
		auto output = Si::Sink<char>::erase(Si::make_container_sink(output_buffer));
		parameters.out = &output;
		BOOST_CHECK_EQUAL(0, run_process(parameters));
		BOOST_CHECK_EQUAL_COLLECTIONS(message.begin(), message.end(), output_buffer.begin(), output_buffer.end());
	}
#endif

#endif
}
