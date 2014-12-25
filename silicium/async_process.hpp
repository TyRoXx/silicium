#ifndef SILICIUM_ASYNC_PROCESS_HPP
#define SILICIUM_ASYNC_PROCESS_HPP

#include <silicium/process_parameters.hpp>
#include <silicium/file_handle.hpp>
#include <silicium/process_handle.hpp>
#include <silicium/error_or.hpp>
#include <silicium/posix/pipe.hpp>
#include <silicium/observable/virtualized.hpp>
#include <silicium/asio/reading_observable.hpp>
#include <boost/filesystem/operations.hpp>

#ifdef _WIN32

#else
#include <boost/asio/posix/stream_descriptor.hpp>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/prctl.h>
#endif

namespace Si
{
	struct async_process_parameters
	{
		boost::filesystem::path executable;

		/// the values for the child's argv[1...]
		std::vector<std::string> arguments;

		/// must be an existing path, otherwise the child cannot launch properly
		boost::filesystem::path current_path;
	};

#ifdef _WIN32

#else
	struct process_output
	{
		typedef error_or<memory_range> element_type;

		process_output()
		{
		}

		explicit process_output(std::unique_ptr<boost::asio::posix::stream_descriptor> pipe_reader)
			: m_pipe_reader(std::move(pipe_reader))
			, m_buffer(4096)
			, m_observable(*m_pipe_reader, make_memory_range(m_buffer))
		{
		}

		template <class Observer>
		void async_get_one(Observer &&observer)
		{
			return m_observable.async_get_one(std::forward<Observer>(observer));
		}

	private:

		std::unique_ptr<boost::asio::posix::stream_descriptor> m_pipe_reader;
		std::vector<char> m_buffer;
		asio::reading_observable<boost::asio::posix::stream_descriptor> m_observable;
	};

	struct async_process
	{
		process_handle process;
		file_handle child_error;

		async_process() BOOST_NOEXCEPT
		{
		}

		async_process(process_handle process, file_handle child_error) BOOST_NOEXCEPT
			: process(std::move(process))
			, child_error(std::move(child_error))
		{
		}

		async_process(async_process &&) BOOST_NOEXCEPT = default;
		async_process &operator = (async_process &&) BOOST_NOEXCEPT = default;

		~async_process() BOOST_NOEXCEPT
		{
		}

		error_or<int> wait_for_exit() BOOST_NOEXCEPT
		{
			int error = 0;
			ssize_t read_error = read(child_error.handle, &error, sizeof(error));
			if (read_error < 0)
			{
				return boost::system::error_code(errno, boost::system::system_category());
			}
			if (read_error != 0)
			{
				assert(read_error == sizeof(error));
				return boost::system::error_code(error, boost::system::system_category());
			}
			return process.wait_for_exit();
		}
	};

	inline error_or<async_process> launch_process(
		async_process_parameters parameters,
		native_file_descriptor standard_input,
		native_file_descriptor standard_output,
		native_file_descriptor standard_error)
	{
		auto executable = parameters.executable.string();
		auto arguments = parameters.arguments;
		std::vector<char *> argument_pointers;
		argument_pointers.emplace_back(const_cast<char *>(executable.c_str()));
		std::transform(begin(arguments), end(arguments), std::back_inserter(argument_pointers), [](std::string &arg)
		{
			return &arg[0];
		});
		argument_pointers.emplace_back(nullptr);

		pipe child_error = make_pipe().get();

		pid_t const forked = fork();
		if (forked < 0)
		{
			return boost::system::error_code(errno, boost::system::system_category());
		}

		//child
		if (forked == 0)
		{
			auto const fail_with_error = [&child_error](int error) SILICIUM_NORETURN
			{
				ssize_t written = write(child_error.write.handle, &error, sizeof(error));
				if (written != sizeof(error))
				{
					_exit(1);
				}
				child_error.write.close();
				_exit(0);
			};

			auto const fail = [fail_with_error]() SILICIUM_NORETURN
			{
				fail_with_error(errno);
			};

			if (dup2(standard_output, STDOUT_FILENO) < 0)
			{
				fail();
			}
			if (dup2(standard_error, STDERR_FILENO) < 0)
			{
				fail();
			}
			if (dup2(standard_input, STDIN_FILENO) < 0)
			{
				fail();
			}

			child_error.read.close();

			boost::system::error_code ec = detail::set_close_on_exec(child_error.write.handle);
			if (ec)
			{
				fail_with_error(ec.value());
			}

			boost::filesystem::current_path(parameters.current_path, ec);
			if (ec)
			{
				fail_with_error(ec.value());
			}

			//close inherited file descriptors
			long max_fd = sysconf(_SC_OPEN_MAX);
			for (int i = 3; i < max_fd; ++i)
			{
				if (i == child_error.write.handle)
				{
					continue;
				}
				close(i); //ignore errors because we will close many non-file-descriptors
			}

			//kill the child when the parent exits
			if (prctl(PR_SET_PDEATHSIG, SIGHUP) < 0)
			{
				fail();
			}

			execvp(parameters.executable.c_str(), argument_pointers.data());
			fail();
		}

		//parent
		else
		{
			return async_process(process_handle(forked), std::move(child_error.read));
		}
	}
#endif
}

#endif
