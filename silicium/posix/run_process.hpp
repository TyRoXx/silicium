#ifndef SILICIUM_POSIX_PROCESS_HPP
#define SILICIUM_POSIX_PROCESS_HPP

#include <silicium/sink/ptr_sink.hpp>
#include <silicium/sink/buffering_sink.hpp>
#include <silicium/process_parameters.hpp>
#include <silicium/file_handle.hpp>
#include <silicium/posix/pipe.hpp>
#include <boost/filesystem/operations.hpp>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/prctl.h>

namespace Si
{
	namespace detail
	{
		template <class CharSink>
		inline void copy_all(int source, CharSink &&destination)
		{
			for (;;)
			{
				auto buffer_helper = make_buffering_sink(std::forward<CharSink>(destination));
				auto const buffer = buffer_helper.make_append_space(std::numeric_limits<std::size_t>::max());
				assert(!buffer.empty());
				auto const rc = read(source, buffer.begin(), buffer.size());
				if (rc == 0)
				{
					break;
				}
				if (rc < 0)
				{
					throw boost::system::system_error(errno, boost::system::system_category());
				}
				buffer_helper.make_append_space(static_cast<std::size_t>(rc));
				buffer_helper.flush_append_space();
			}
		}
	}

	SILICIUM_DEPRECATED
	inline int run_process(process_parameters const &parameters)
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

		pipe stdout;
		if (parameters.out)
		{
			stdout = make_pipe().get();
		}

		auto stdin = make_pipe().get();
		auto child_error = make_pipe().get();

		pid_t const forked = fork();
		if (forked < 0)
		{
			throw boost::system::system_error(errno, boost::system::system_category());
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

			if (parameters.out)
			{
				if (dup2(stdout.write.handle, STDOUT_FILENO) < 0)
				{
					fail();
				}
				if (dup2(stdout.write.handle, STDERR_FILENO) < 0)
				{
					fail();
				}
				stdout.close();
			}
			else
			{
				//drop output
				dup2(open("/dev/null", O_RDWR), STDOUT_FILENO);
				dup2(open("/dev/null", O_RDWR), STDERR_FILENO);
			}

			if (dup2(stdin.read.handle, STDIN_FILENO) < 0)
			{
				fail();
			}
			else
			{
				//provide empty input
				dup2(open("/dev/null", O_RDWR), STDIN_FILENO);
			}
			stdin.close();

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
			child_error.write.close();

			if (parameters.out)
			{
				stdout.write.close();
				detail::copy_all(stdout.read.handle, ref_sink(*parameters.out));
			}
			stdin.read.close();

			int status = 0;
			if (waitpid(forked, &status, 0) < 0)
			{
				throw boost::system::system_error(errno, boost::system::system_category());
			}

			int error = 0;
			ssize_t read_error = read(child_error.read.handle, &error, sizeof(error));
			if (read_error < 0)
			{
				throw boost::system::system_error(errno, boost::system::system_category());
			}
			if (read_error != 0)
			{
				assert(read_error == sizeof(error));
				throw boost::system::system_error(error, boost::system::system_category());
			}

			int const exit_status = WEXITSTATUS(status);
			return exit_status;
		}
	}
}

#endif
