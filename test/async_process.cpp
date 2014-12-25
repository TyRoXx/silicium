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
			Si::process_output output_reader(Si::make_unique<boost::asio::posix::stream_descriptor>(io, file.handle));
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
}

BOOST_AUTO_TEST_CASE(async_process_unix_which)
{
	Si::async_process_parameters parameters;
	parameters.executable = "/usr/bin/which";
	parameters.arguments.emplace_back("which");
	parameters.current_path = boost::filesystem::current_path();

	Si::async_process process = Si::launch_process(parameters).get();
	boost::asio::io_service io;

	std::string standard_output;
	begin_reading_output(io, Si::make_container_sink(standard_output), std::move(process.standard_output));

	std::string standard_error;
	begin_reading_output(io, Si::make_container_sink(standard_error), std::move(process.standard_error));

	io.run();

	BOOST_CHECK_EQUAL("/usr/bin/which\n", standard_output);
	BOOST_CHECK_EQUAL("", standard_error);
	BOOST_CHECK_EQUAL(0, process.wait_for_exit().get());
}
