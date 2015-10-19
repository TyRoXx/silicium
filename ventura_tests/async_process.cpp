#include <ventura/async_process.hpp>
#include <silicium/observable/spawn_coroutine.hpp>
#include <silicium/observable/spawn_observable.hpp>
#include <silicium/observable/transform.hpp>
#include <silicium/observable/ref.hpp>
#include <silicium/observable/thread.hpp>
#include <silicium/sink/iterator_sink.hpp>
#include <silicium/std_threading.hpp>
#include <silicium/environment_variables.hpp>
#include <ventura/run_process.hpp>
#include <ventura/file_operations.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/assign/list_of.hpp>

#if SILICIUM_HAS_EXCEPTIONS
#	include <boost/filesystem/operations.hpp>
#endif

namespace
{
	struct process_output
	{
		int exit_code;
		Si::noexcept_string output, error;
	};

#if VENTURA_HAS_EXPERIMENTAL_READ_FROM_ANONYMOUS_PIPE && VENTURA_HAS_LAUNCH_PROCESS
	process_output run_process(
		ventura::async_process_parameters parameters,
		std::vector<std::pair<Si::os_char const *, Si::os_char const *>> environment_variables,
		ventura::environment_inheritance inheritance)
	{
		Si::pipe standard_input = Si::make_pipe().move_value();
		Si::pipe standard_output = Si::make_pipe().move_value();
		Si::pipe standard_error = Si::make_pipe().move_value();

		ventura::async_process process =
			ventura::launch_process(
				parameters,
				standard_input.read.handle,
				standard_output.write.handle,
				standard_error.write.handle,
				std::move(environment_variables),
				inheritance
			).move_value();
		standard_input.read.close();
		standard_output.write.close();
		standard_error.write.close();

		boost::asio::io_service io;

		process_output result;

		boost::promise<void> stop_polling;
		boost::shared_future<void> stopped_polling = stop_polling.get_future().share();

		ventura::experimental::read_from_anonymous_pipe(io, Si::make_container_sink(result.output), std::move(standard_output.read), stopped_polling);
		ventura::experimental::read_from_anonymous_pipe(io, Si::make_container_sink(result.error), std::move(standard_error.read), stopped_polling);

		io.run();

		result.exit_code = process.wait_for_exit().get();
		return result;
	}
#endif
}

#if !defined(_WIN32) && VENTURA_HAS_ABSOLUTE_PATH_OPERATIONS && VENTURA_HAS_LAUNCH_PROCESS
BOOST_AUTO_TEST_CASE(async_process_unix_which)
{
	ventura::async_process_parameters parameters;
	parameters.executable = *ventura::absolute_path::create("/usr/bin/which");
	parameters.arguments.emplace_back("which");
	parameters.current_path = ventura::get_current_working_directory(Si::throw_);

	process_output result = run_process(parameters, std::vector<std::pair<Si::os_char const *, Si::os_char const *>>(), ventura::environment_inheritance::inherit);

	BOOST_CHECK_EQUAL("/usr/bin/which\n", result.output);
	BOOST_CHECK_EQUAL("", result.error);
	BOOST_CHECK_EQUAL(0, result.exit_code);
}
#endif

#if defined(_WIN32) && VENTURA_HAS_ABSOLUTE_PATH_OPERATIONS && VENTURA_HAS_RUN_PROCESS
BOOST_AUTO_TEST_CASE(async_process_win32_where)
{
	ventura::async_process_parameters parameters;
	parameters.executable = *ventura::absolute_path::create(L"C:\\Windows\\System32\\where.exe");
	parameters.current_path = ventura::get_current_working_directory(Si::throw_);

	process_output result = run_process(parameters, std::vector<std::pair<Si::os_char const *, Si::os_char const *>>(), ventura::environment_inheritance::no_inherit);

	std::size_t const windows7whereHelpSize = 1830;
	std::size_t const windowsServer2012whereHelpSize = 1705;
	BOOST_CHECK_GE(windows7whereHelpSize, result.output.size());
	BOOST_CHECK_LE(windowsServer2012whereHelpSize, result.output.size());
	BOOST_CHECK_EQUAL("", result.error);
	BOOST_CHECK_EQUAL(2, result.exit_code);
}
#endif

namespace
{
	ventura::absolute_path const arbitrary_root_dir = *ventura::absolute_path::create(
#ifdef _WIN32
		L"C:\\"
#else
		"/"
#endif
	);
}

#if VENTURA_HAS_ABSOLUTE_PATH_OPERATIONS && VENTURA_HAS_RUN_PROCESS
BOOST_AUTO_TEST_CASE(async_process_executable_not_found)
{
	ventura::async_process_parameters parameters;
	parameters.executable = arbitrary_root_dir / "does-not-exist";
	parameters.current_path = ventura::get_current_working_directory(Si::throw_);

	BOOST_CHECK_EXCEPTION(run_process(parameters, std::vector<std::pair<Si::os_char const *, Si::os_char const *>>(), ventura::environment_inheritance::no_inherit), boost::system::system_error, [](boost::system::system_error const &ex)
	{
		return ex.code() == boost::system::errc::no_such_file_or_directory;
	});
}
#endif

#if VENTURA_HAS_ABSOLUTE_PATH_OPERATIONS && VENTURA_HAS_RUN_PROCESS
namespace
{
	void test_environment_variables(
		ventura::environment_inheritance const tested_inheritance,
		std::vector<std::pair<Si::os_char const *, Si::os_char const *>> const additional_variables)
	{
		ventura::async_process_parameters parameters;
		parameters.executable = *ventura::absolute_path::create(
#ifdef _WIN32
			SILICIUM_SYSTEM_LITERAL("C:\\Windows\\System32\\cmd.exe")
#else
			"/usr/bin/env"
#endif
			);
		parameters.current_path = ventura::get_current_working_directory(Si::throw_);
#ifdef _WIN32
		parameters.arguments.emplace_back(SILICIUM_SYSTEM_LITERAL("/C"));
		parameters.arguments.emplace_back(SILICIUM_SYSTEM_LITERAL("set"));
#endif

		BOOST_REQUIRE(!Si::set_environment_variable(SILICIUM_SYSTEM_LITERAL("silicium_parent_key"), SILICIUM_SYSTEM_LITERAL("parent_value")));

		process_output const output = run_process(parameters, additional_variables, tested_inheritance);
		BOOST_CHECK_EQUAL(0, output.exit_code);

		for (auto const &variable : additional_variables)
		{
			BOOST_CHECK_NE(std::string::npos, output.output.find(Si::to_utf8_string(variable.first) + '=' + Si::to_utf8_string(variable.second)));
		}

		auto const parent_key_found = output.output.find("silicium_parent_key=parent_value");
		switch (tested_inheritance)
		{
		case ventura::environment_inheritance::inherit:
			BOOST_CHECK_NE(std::string::npos, parent_key_found);
			break;

		case ventura::environment_inheritance::no_inherit:
			BOOST_CHECK_EQUAL(std::string::npos, parent_key_found);
			break;
		}
	}
}

namespace
{
	std::vector<std::pair<Si::os_char const *, Si::os_char const *>> const additional_variables = boost::assign::list_of(
		std::make_pair(SILICIUM_SYSTEM_LITERAL("key"), SILICIUM_SYSTEM_LITERAL("value"))
	);
}

BOOST_AUTO_TEST_CASE(async_process_environment_variables_inherit_additional_vars)
{
	test_environment_variables(ventura::environment_inheritance::inherit, additional_variables);
}

BOOST_AUTO_TEST_CASE(async_process_environment_variables_no_inherit_additional_vars)
{
	test_environment_variables(ventura::environment_inheritance::no_inherit, additional_variables);
}

BOOST_AUTO_TEST_CASE(async_process_environment_variables_inherit)
{
	test_environment_variables(ventura::environment_inheritance::inherit, std::vector<std::pair<Si::os_char const *, Si::os_char const *>>());
}

BOOST_AUTO_TEST_CASE(async_process_environment_variables_no_inherit)
{
	test_environment_variables(ventura::environment_inheritance::no_inherit, std::vector<std::pair<Si::os_char const *, Si::os_char const *>>());
}
#endif
