#include <silicium/async_process.hpp>
#include <silicium/observable/spawn_coroutine.hpp>
#include <silicium/observable/spawn_observable.hpp>
#include <silicium/observable/transform.hpp>
#include <silicium/observable/ref.hpp>
#include <silicium/observable/thread.hpp>
#include <silicium/sink/iterator_sink.hpp>
#include <silicium/std_threading.hpp>
#include <boost/test/unit_test.hpp>

#if SILICIUM_HAS_EXCEPTIONS
#	include <boost/filesystem/operations.hpp>
#endif

namespace
{
	struct process_output
	{
		int exit_code;
		std::string output, error;
	};

#if SILICIUM_HAS_EXPERIMENTAL_READ_FROM_ANONYMOUS_PIPE
	process_output run_process(Si::async_process_parameters parameters, std::vector<std::pair<Si::os_char const *, Si::os_char const *>> environment_variables = {})
	{
		Si::pipe standard_input = SILICIUM_MOVE_IF_COMPILER_LACKS_RVALUE_QUALIFIERS(Si::make_pipe().get());
		Si::pipe standard_output = SILICIUM_MOVE_IF_COMPILER_LACKS_RVALUE_QUALIFIERS(Si::make_pipe().get());
		Si::pipe standard_error = SILICIUM_MOVE_IF_COMPILER_LACKS_RVALUE_QUALIFIERS(Si::make_pipe().get());

		Si::async_process process = SILICIUM_MOVE_IF_COMPILER_LACKS_RVALUE_QUALIFIERS(
			Si::launch_process(
				parameters,
				standard_input.read.handle,
				standard_output.write.handle,
				standard_error.write.handle,
				std::move(environment_variables)
			).get()
		);
		standard_input.read.close();
		standard_output.write.close();
		standard_error.write.close();

		boost::asio::io_service io;

		process_output result;

		Si::experimental::read_from_anonymous_pipe(io, Si::make_container_sink(result.output), std::move(standard_output.read));
		Si::experimental::read_from_anonymous_pipe(io, Si::make_container_sink(result.error), std::move(standard_error.read));

		io.run();

		result.exit_code = process.wait_for_exit().get();
		return result;
	}
#endif
}

#if !defined(_WIN32) && SILICIUM_HAS_ABSOLUTE_PATH_OPERATIONS
BOOST_AUTO_TEST_CASE(async_process_unix_which)
{
	Si::async_process_parameters parameters;
	parameters.executable = *Si::absolute_path::create("/usr/bin/which");
	parameters.arguments.emplace_back("which");
	parameters.current_path = Si::get_current_working_directory();

	process_output result = run_process(parameters);

	BOOST_CHECK_EQUAL("/usr/bin/which\n", result.output);
	BOOST_CHECK_EQUAL("", result.error);
	BOOST_CHECK_EQUAL(0, result.exit_code);
}
#endif

#if defined(_WIN32) && SILICIUM_HAS_ABSOLUTE_PATH_OPERATIONS
BOOST_AUTO_TEST_CASE(async_process_win32_where)
{
	Si::async_process_parameters parameters;
	parameters.executable = *Si::absolute_path::create(L"C:\\Windows\\System32\\where.exe");
	parameters.current_path = Si::get_current_working_directory();

	process_output result = run_process(parameters);

	std::size_t const windows7whereHelpSize = 1830;
	BOOST_CHECK_EQUAL(windows7whereHelpSize, result.output.size());
	BOOST_CHECK_EQUAL("", result.error);
	BOOST_CHECK_EQUAL(2, result.exit_code);
}
#endif

namespace
{
	Si::absolute_path const arbitrary_root_dir = *Si::absolute_path::create(
#ifdef _WIN32
		L"C:\\"
#else
		"/"
#endif
	);
}

#if SILICIUM_HAS_ABSOLUTE_PATH_OPERATIONS
BOOST_AUTO_TEST_CASE(async_process_executable_not_found)
{
	Si::async_process_parameters parameters;
	parameters.executable = arbitrary_root_dir / "does-not-exist";
	parameters.current_path = Si::get_current_working_directory();

	BOOST_CHECK_EXCEPTION(run_process(parameters), boost::system::system_error, [](boost::system::system_error const &ex)
	{
		return ex.code() == boost::system::errc::no_such_file_or_directory;
	});
}
#endif

#ifdef _WIN32
BOOST_AUTO_TEST_CASE(async_process_environment_variables)
{
	Si::async_process_parameters parameters;
	parameters.executable = *Si::absolute_path::create(SILICIUM_SYSTEM_LITERAL("C:\\Windows\\System32\\cmd.exe"));
	parameters.current_path = Si::get_current_working_directory();
	parameters.arguments.emplace_back(SILICIUM_SYSTEM_LITERAL("/C"));
	parameters.arguments.emplace_back(SILICIUM_SYSTEM_LITERAL("set"));

	std::vector<std::pair<Si::os_char const *, Si::os_char const *>> const environment_variables
	{
		{L"key", L"value"}
	};
	process_output const output = run_process(parameters, environment_variables);
	BOOST_CHECK_EQUAL(0, output.exit_code);
	BOOST_CHECK_NE(std::string::npos, output.output.find("key=value\r\n"));
}
#endif
