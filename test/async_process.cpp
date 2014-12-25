#include <silicium/async_process.hpp>
#include <silicium/observable/spawn_coroutine.hpp>
#include <silicium/observable/spawn_observable.hpp>
#include <silicium/observable/transform.hpp>
#include <silicium/observable/ref.hpp>
#include <silicium/sink/iterator_sink.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/filesystem/operations.hpp>

namespace
{
	template <class CharSink>
	void begin_reading_output(boost::asio::io_service &io, CharSink &&destination, Si::file_handle &&file)
	{
		Si::spawn_coroutine([&io, destination, &file](Si::spawn_context yield)
		{
			Si::process_output output_reader(Si::make_unique<Si::process_output::stream>(io, file.handle));
			file.release();
			for (;;)
			{
				auto piece = yield.get_one(Si::ref(output_reader));
				assert(piece);
				if (piece->is_error())
				{
					break;
				}
				Si::memory_range data = piece->get();
				if (data.empty())
				{
					break;
				}
				Si::append(destination, data);
			}
		});
	}

	struct process_output
	{
		int exit_code;
		std::string output, error;
	};

	process_output run_process(Si::async_process_parameters parameters)
	{
		Si::pipe standard_input = Si::make_pipe().get();
		Si::pipe standard_output = Si::make_pipe().get();
		Si::pipe standard_error = Si::make_pipe().get();

		Si::async_process process = Si::launch_process(parameters, standard_input.read.handle, standard_output.write.handle, standard_error.write.handle).get();
		standard_input.read.close();
		standard_output.write.close();
		standard_error.write.close();

		boost::asio::io_service io;

		process_output result;

		begin_reading_output(io, Si::make_container_sink(result.output), std::move(standard_output.read));
		begin_reading_output(io, Si::make_container_sink(result.error), std::move(standard_error.read));

		io.run();

		result.exit_code = process.wait_for_exit().get();
		return result;
	}
}

#ifndef _WIN32
BOOST_AUTO_TEST_CASE(async_process_unix_which)
{
	Si::async_process_parameters parameters;
	parameters.executable = "/usr/bin/which";
	parameters.arguments.emplace_back("which");
	parameters.current_path = boost::filesystem::current_path();

	process_output result = run_process(parameters);

	BOOST_CHECK_EQUAL("/usr/bin/which\n", result.output);
	BOOST_CHECK_EQUAL("", result.error);
	BOOST_CHECK_EQUAL(0, result.exit_code);
}
#endif

#ifdef _WIN32
BOOST_AUTO_TEST_CASE(async_process_win32_where)
{
	Si::async_process_parameters parameters;
	parameters.executable = L"C:\\Windows\\System32\\where.exe";
	parameters.current_path = boost::filesystem::current_path();

	process_output result = run_process(parameters);

	std::size_t const windows7whereHelpSize = 1830;
	BOOST_CHECK_EQUAL(windows7whereHelpSize, result.output.size());
	BOOST_CHECK_EQUAL("", result.error);
	BOOST_CHECK_EQUAL(2, result.exit_code);
}
#endif

BOOST_AUTO_TEST_CASE(async_process_executable_not_found)
{
	Si::async_process_parameters parameters;
	parameters.executable = "does-not-exist";
	parameters.current_path = boost::filesystem::current_path();

	BOOST_CHECK_EXCEPTION(run_process(parameters), boost::system::system_error, [](boost::system::system_error const &ex)
	{
		return ex.code() == boost::system::errc::no_such_file_or_directory;
	});
}
