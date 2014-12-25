#ifndef SILICIUM_ASYNC_PROCESS_HPP
#define SILICIUM_ASYNC_PROCESS_HPP

#include <silicium/process_parameters.hpp>
#include <silicium/file_handle.hpp>
#include <silicium/error_or.hpp>
#include <silicium/posix/pipe.hpp>
#include <silicium/observable/virtualized.hpp>
#include <silicium/asio/reading_observable.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/asio/posix/stream_descriptor.hpp>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/prctl.h>

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

	struct process_handle
	{
		process_handle() BOOST_NOEXCEPT
			: m_id(-1)
		{
		}

		explicit process_handle(pid_t id)
			: m_id(id)
		{
		}

		~process_handle() BOOST_NOEXCEPT
		{
			if (m_id < 0)
			{
				return;
			}
			wait_for_exit();
		}

		process_handle(process_handle &&other) BOOST_NOEXCEPT
			: m_id(-1)
		{
			swap(other);
		}

		process_handle &operator = (process_handle &&other) BOOST_NOEXCEPT
		{
			swap(other);
			return *this;
		}

		void swap(process_handle &other) BOOST_NOEXCEPT
		{
			boost::swap(m_id, other.m_id);
		}

		error_or<int> wait_for_exit() BOOST_NOEXCEPT
		{
			int status = 0;
			int wait_id = Si::exchange(m_id, -1);
			assert(wait_id >= 1);
			if (waitpid(wait_id, &status, 0) < 0)
			{
				return boost::system::error_code(errno, boost::system::system_category());
			}
			int const exit_status = WEXITSTATUS(status);
			return exit_status;
		}

	private:

		pid_t m_id;

		SILICIUM_DELETED_FUNCTION(process_handle(process_handle const &))
		SILICIUM_DELETED_FUNCTION(process_handle &operator = (process_handle const &))
	};

	struct async_process
	{
		process_handle process;
		file_handle child_error;
		file_handle standard_output;
		file_handle standard_error;
		file_handle standard_input;

		async_process()
		{
		}

		async_process(process_handle process, file_handle child_error, file_handle standard_output, file_handle standard_error, file_handle standard_input)
			: process(std::move(process))
			, child_error(std::move(child_error))
			, standard_output(std::move(standard_output))
			, standard_error(std::move(standard_error))
			, standard_input(std::move(standard_input))
		{
		}

		async_process(async_process &&) = default;
		async_process &operator = (async_process &&) = default;

		~async_process()
		{
		}

		error_or<int> wait_for_exit()
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
				return boost::system::error_code(errno, boost::system::system_category());
			}
			return process.wait_for_exit();
		}
	};

	inline error_or<async_process> launch_process(async_process_parameters parameters)
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

		pipe stdout = make_pipe();
		pipe stderr = make_pipe();
		pipe stdin = make_pipe();
		pipe child_error = make_pipe();

		pid_t const forked = fork();
		if (forked < 0)
		{
			return boost::system::error_code(errno, boost::system::system_category());
		}

		//child
		if (forked == 0)
		{
			auto const fail = [&child_error]()
#if defined(__GNUC__) && !defined(__clang__)
				__attribute__ ((__noreturn__))
#endif
			{
				int error = errno;
				ssize_t written = write(child_error.write.handle, &error, sizeof(error));
				if (written != sizeof(error))
				{
					_exit(1);
				}
				child_error.write.close();
				_exit(0);
			};

			if (dup2(stdout.write.handle, STDOUT_FILENO) < 0)
			{
				fail();
			}
			if (dup2(stderr.write.handle, STDERR_FILENO) < 0)
			{
				fail();
			}
			if (dup2(stdin.read.handle, STDIN_FILENO) < 0)
			{
				fail();
			}

			child_error.read.close();
			detail::set_close_on_exec(child_error.write.handle);

			boost::filesystem::current_path(parameters.current_path);

			//close inherited file descriptors
			long max_fd = sysconf(_SC_OPEN_MAX);
			for (int i = 3; i < max_fd; ++i)
			{
				if (i == child_error.write.handle)
				{
					continue;
				}
				close(i);
			}

			//kill the child when the parent exits
			prctl(PR_SET_PDEATHSIG, SIGHUP);

			execvp(parameters.executable.c_str(), argument_pointers.data());
			fail();
		}

		//parent
		else
		{
			process_handle process(forked);
			return async_process(std::move(process), std::move(child_error.read), std::move(stdout.read), std::move(stderr.read), std::move(stdin.write));
		}
	}
}

#endif
