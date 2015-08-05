#ifndef SILICIUM_RUN_PROCESS_HPP
#define SILICIUM_RUN_PROCESS_HPP

#include <silicium/async_process.hpp>
#include <silicium/write.hpp>
#include <silicium/sink/multi_sink.hpp>
#include <boost/range/algorithm/transform.hpp>

namespace Si
{
	template <class T>
	bool extract(T &destination, optional<T> &&source)
	{
		if (source)
		{
			destination = std::forward<T>(*source);
			return true;
		}
		return false;
	}
}

#define SILICIUM_HAS_RUN_PROCESS (SILICIUM_HAS_EXCEPTIONS && SILICIUM_HAS_EXPERIMENTAL_READ_FROM_ANONYMOUS_PIPE)

#if SILICIUM_HAS_RUN_PROCESS
#include <future>

namespace Si
{
	inline int run_process(process_parameters const &parameters)
	{
		async_process_parameters async_parameters;
		if (!extract(async_parameters.executable, absolute_path::create(parameters.executable)))
		{
			throw std::invalid_argument("a process can only be started with an absolute path to the executable");
		}
		if (!extract(async_parameters.current_path, absolute_path::create(parameters.current_path)))
		{
			throw std::invalid_argument("a process can only be started with an absolute path as the working directory");
		}
		boost::range::transform(parameters.arguments, std::back_inserter(async_parameters.arguments), [](noexcept_string const &argument)
		{
			return to_os_string(argument);
		});
		auto input = make_pipe().move_value();
		auto std_output = make_pipe().move_value();
		auto std_error = make_pipe().move_value();
		async_process process =
			launch_process(async_parameters, input.read.handle, std_output.write.handle, std_error.write.handle, std::vector<std::pair<os_char const *, os_char const *>>(), environment_inheritance::inherit).move_value();

		boost::asio::io_service io;

		auto std_output_consumer = make_multi_sink<char, success>([&parameters]()
		{
			return make_iterator_range(&parameters.out, &parameters.out + (parameters.out != nullptr));
		});
		experimental::read_from_anonymous_pipe(io, std_output_consumer, std::move(std_output.read));

		auto std_error_consumer = make_multi_sink<char, success>([&parameters]()
		{
			return make_iterator_range(&parameters.err, &parameters.err + (parameters.err != nullptr));
		});
		experimental::read_from_anonymous_pipe(io, std_error_consumer, std::move(std_error.read));

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
				optional<char> const c = Si::get(*parameters.in);
				if (!c)
				{
					break;
				}
				error_or<size_t> written = write(input.write.handle, make_memory_range(&*c, 1));
				if (written.is_error())
				{
					//process must have exited
					break;
				}
				assert(written.get() == 1);
			}
			input.write.close();
		});

		io.run();
		copy_input.get();

		return process.wait_for_exit().get();
	}

	inline int run_process(
		boost::filesystem::path executable,
		std::vector<noexcept_string> arguments,
		boost::filesystem::path current_path,
		Sink<char, success>::interface &output)
	{
		Si::process_parameters parameters;
		parameters.executable = std::move(executable);
		parameters.arguments = std::move(arguments);
		parameters.current_path = std::move(current_path);
		parameters.out = &output;
		return Si::run_process(parameters);
	}
}
#endif

#endif
