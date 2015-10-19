#ifndef VENTURA_RUN_PROCESS_HPP
#define VENTURA_RUN_PROCESS_HPP

#include <ventura/async_process.hpp>
#include <silicium/write.hpp>
#include <silicium/sink/multi_sink.hpp>
#include <boost/range/algorithm/transform.hpp>

namespace ventura
{
	template <class T>
	bool extract(T &destination, Si::optional<T> &&source)
	{
		if (source)
		{
			destination = std::forward<T>(*source);
			return true;
		}
		return false;
	}
}

#define VENTURA_HAS_RUN_PROCESS (SILICIUM_HAS_EXCEPTIONS && VENTURA_HAS_EXPERIMENTAL_READ_FROM_ANONYMOUS_PIPE && VENTURA_HAS_LAUNCH_PROCESS)

#if VENTURA_HAS_RUN_PROCESS
#include <future>

namespace ventura
{
	inline int run_process(process_parameters const &parameters)
	{
		async_process_parameters async_parameters;
		async_parameters.executable = parameters.executable;
		async_parameters.arguments = parameters.arguments;
		async_parameters.current_path = parameters.current_path;
		auto input = Si::make_pipe().move_value();
		auto std_output = Si::make_pipe().move_value();
		auto std_error = Si::make_pipe().move_value();
		async_process process =
			launch_process(async_parameters, input.read.handle, std_output.write.handle, std_error.write.handle, parameters.additional_environment, parameters.inheritance).move_value();

		boost::asio::io_service io;

		boost::promise<void> stop_polling;
		boost::shared_future<void> stopped_polling = stop_polling.get_future().share();

		auto std_output_consumer = Si::make_multi_sink<char, Si::success>([&parameters]()
		{
			return Si::make_iterator_range(&parameters.out, &parameters.out + (parameters.out != nullptr));
		});
		experimental::read_from_anonymous_pipe(io, std_output_consumer, std::move(std_output.read), stopped_polling);

		auto std_error_consumer = Si::make_multi_sink<char, Si::success>([&parameters]()
		{
			return Si::make_iterator_range(&parameters.err, &parameters.err + (parameters.err != nullptr));
		});
		experimental::read_from_anonymous_pipe(io, std_error_consumer, std::move(std_error.read), stopped_polling);

		input.read.close();
		std_output.write.close();
		std_error.write.close();

		auto copy_input = std::async(std::launch::async, [&input, &parameters]()
		{
			if (!parameters.in)
			{
				return;
			}
			for (;;)
			{
				Si::optional<char> const c = Si::get(*parameters.in);
				if (!c)
				{
					break;
				}
				Si::error_or<size_t> written = write(input.write.handle, Si::make_memory_range(&*c, 1));
				if (written.is_error())
				{
					//process must have exited
					break;
				}
				assert(written.get() == 1);
			}
			input.write.close();
		});

#ifdef _WIN32
		auto waited = std::async(std::launch::deferred, [&process, &stop_polling, &io]()
		{
			int rc = process.wait_for_exit().get();
			io.stop();
			stop_polling.set_value();
			return rc;
		});
		io.run();
		copy_input.get();
		return waited.get();
#else
		io.run();
		copy_input.get();
		return process.wait_for_exit().get();
#endif
	}

	SILICIUM_USE_RESULT
	inline int run_process(
		boost::filesystem::path executable,
		std::vector<Si::noexcept_string> arguments,
		boost::filesystem::path current_path,
		Si::Sink<char, Si::success>::interface &output)
	{
		process_parameters parameters;
		parameters.executable = absolute_path::create(std::move(executable)).or_throw([]{ throw std::invalid_argument("The executable must be an absolute path."); });
		boost::range::transform(arguments, std::back_inserter(parameters.arguments), [&parameters](Si::noexcept_string const &argument)
		{
			return Si::to_os_string(argument);
		});
		parameters.current_path = absolute_path::create(std::move(current_path)).or_throw([]{ throw std::invalid_argument("The current directory must be an absolute path."); });
		parameters.out = &output;
		parameters.err = &output;
		return run_process(parameters);
	}

	SILICIUM_USE_RESULT
	inline Si::error_or<int> run_process(
		absolute_path executable,
		std::vector<Si::os_string> arguments,
		absolute_path current_directory,
		Si::Sink<char, Si::success>::interface &output
		)
	{
		process_parameters parameters;
		parameters.executable = std::move(executable);
		parameters.arguments = std::move(arguments);
		parameters.current_path = std::move(current_directory);
		parameters.out = &output;
		parameters.err = &output;
		return run_process(parameters);
	}

	SILICIUM_USE_RESULT
	inline Si::error_or<int> run_process(
		absolute_path executable,
		std::vector<Si::os_string> arguments,
		absolute_path current_directory,
		Si::Sink<char, Si::success>::interface &output,
		std::vector<std::pair<Si::os_char const *, Si::os_char const *>> additional_environment,
		environment_inheritance inheritance
		)
	{
		process_parameters parameters;
		parameters.executable = std::move(executable);
		parameters.arguments = std::move(arguments);
		parameters.current_path = std::move(current_directory);
		parameters.out = &output;
		parameters.err = &output;
		parameters.additional_environment = std::move(additional_environment);
		parameters.inheritance = inheritance;
		return run_process(parameters);
	}
}
#endif

#endif
