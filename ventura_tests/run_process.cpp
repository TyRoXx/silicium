#include <ventura/run_process.hpp>
#include <silicium/to_unique.hpp>
#include <ventura/file_operations.hpp>
#include <silicium/sink/iterator_sink.hpp>
#include <silicium/sink/virtualized_sink.hpp>
#include <silicium/source/range_source.hpp>
#include <boost/test/unit_test.hpp>
#if VENTURA_HAS_RUN_PROCESS
#	include <boost/filesystem/operations.hpp>
#endif

namespace Si
{
#if VENTURA_HAS_RUN_PROCESS
#ifndef _WIN32
	BOOST_AUTO_TEST_CASE(run_process_1_unix_which)
	{
		ventura::process_parameters parameters;
		parameters.executable = *ventura::absolute_path::create("/usr/bin/which");
		parameters.arguments.emplace_back("which");
		parameters.current_path = ventura::get_current_working_directory(Si::throw_);
		std::vector<char> out;
		auto sink = Si::virtualize_sink(make_iterator_sink<char>(std::back_inserter(out)));
		parameters.out = &sink;
		int result = ventura::run_process(parameters);
		BOOST_CHECK_EQUAL(0, result);
		std::string const expected = "/usr/bin/which\n";
		BOOST_CHECK_EQUAL(expected, std::string(begin(out), end(out)));
	}
#endif

#ifdef _WIN32
	BOOST_AUTO_TEST_CASE(run_process_1_win32_cmd)
	{
		ventura::process_parameters parameters;
		parameters.executable = *ventura::absolute_path::create(L"C:\\Windows\\System32\\where.exe");
		parameters.current_path = ventura::get_current_working_directory(Si::throw_);
		std::vector<char> out;
		auto sink = Si::virtualize_sink(make_iterator_sink<char>(std::back_inserter(out)));
		parameters.out = &sink;
		int result = ventura::run_process(parameters);
		BOOST_CHECK_EQUAL(2, result);
		std::size_t const windows7whereHelpSize = 1830;
		std::size_t const windowsServer2012whereHelpSize = 1705;
		BOOST_CHECK_GE(windows7whereHelpSize, out.size());
		BOOST_CHECK_LE(windowsServer2012whereHelpSize, out.size());
	}
#endif

	namespace
	{
		ventura::absolute_path const absolute_root = *ventura::absolute_path::create(
	#ifdef _WIN32
			L"C:/"
	#else
			"/"
	#endif
		);
	}

	BOOST_AUTO_TEST_CASE(run_process_from_nonexecutable)
	{
		ventura::process_parameters parameters;
		parameters.executable = absolute_root / SILICIUM_SYSTEM_LITERAL("does-not-exist");
		parameters.current_path = ventura::get_current_working_directory(Si::throw_);
		BOOST_CHECK_EXCEPTION(ventura::run_process(parameters), boost::system::system_error, [](boost::system::system_error const &e)
		{
			return e.code() == boost::system::error_code(ENOENT, boost::system::system_category());
		});
	}

#ifndef _WIN32
	BOOST_AUTO_TEST_CASE(run_process_standard_input)
	{
		ventura::process_parameters parameters;
		parameters.executable = *ventura::absolute_path::create("/bin/cat");
		parameters.current_path = ventura::get_current_working_directory(Si::throw_);
		auto message = Si::make_c_str_range("Hello, cat");
		auto input = Si::Source<char>::erase(Si::make_range_source(message));
		parameters.in = &input;
		std::vector<char> output_buffer;
		auto output = Si::Sink<char>::erase(Si::make_container_sink(output_buffer));
		parameters.out = &output;
		BOOST_CHECK_EQUAL(0, ventura::run_process(parameters));
		BOOST_CHECK_EQUAL_COLLECTIONS(message.begin(), message.end(), output_buffer.begin(), output_buffer.end());
	}
#endif

#endif
}
