#include <silicium/process.hpp>
#include <silicium/to_unique.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/system/system_error.hpp>
#include <algorithm>
#include <memory>

#ifdef __linux__
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#endif

namespace Si
{
	process_parameters::process_parameters()
		: out(nullptr)
		, err(nullptr)
		, in(nullptr)
	{
	}

#ifdef __linux__
	namespace detail
	{
		namespace
		{
			void terminating_close(int file) BOOST_NOEXCEPT
			{
				if (close(file) < 0)
				{
					//it is intended that this will terminate the process because of noexcept
					throw boost::system::system_error(errno, boost::system::system_category());
				}
			}

			struct file_closer
			{
				void operator()(int *file) const noexcept
				{
					assert(file);
					terminating_close(*file);
				}
			};

			void copy_all(int source, sink<char> &destination)
			{
				for (;;)
				{
					buffering_sink<char> buffer_helper(destination);
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
					commit(buffer_helper, static_cast<std::size_t>(rc));
				}
			}

			void set_close_on_exec(int file)
			{
				fcntl(file, F_SETFD, fcntl(file, F_GETFD) | FD_CLOEXEC);
			}

			struct file_descriptor : private boost::noncopyable
			{
				int handle;

				file_descriptor() BOOST_NOEXCEPT
					: handle(-1)
				{
				}

				file_descriptor(file_descriptor &&other) BOOST_NOEXCEPT
					: handle(-1)
				{
					swap(other);
				}

				explicit file_descriptor(int handle) BOOST_NOEXCEPT
					: handle(handle)
				{
				}

				file_descriptor &operator = (file_descriptor &&other) BOOST_NOEXCEPT
				{
					swap(other);
					return *this;
				}

				void swap(file_descriptor &other) BOOST_NOEXCEPT
				{
					using std::swap;
					swap(handle, other.handle);
				}

				void close() BOOST_NOEXCEPT
				{
					file_descriptor().swap(*this);
				}

				~file_descriptor() BOOST_NOEXCEPT
				{
					if (handle >= 0)
					{
						terminating_close(handle);
					}
				}
			};

			struct pipe
			{
				file_descriptor write, read;

				void close() BOOST_NOEXCEPT
				{
					pipe().swap(*this);
				}

				void swap(pipe &other) BOOST_NOEXCEPT
				{
					write.swap(other.write);
					read.swap(other.read);
				}
			};

			pipe make_pipe()
			{
				std::array<int, 2> fds;
				if (::pipe(fds.data()) < 0)
				{
					throw boost::system::system_error(errno, boost::system::system_category());
				}
				pipe result;
				result.read  = file_descriptor(fds[0]);
				result.write = file_descriptor(fds[1]);
				return result;
			}
		}
	}

	int run_process(process_parameters const &parameters)
	{
		auto executable = parameters.executable.string();
		auto arguments = parameters.arguments;
		std::vector<char *> argument_pointers;
		argument_pointers.emplace_back(&executable[0]);
		std::transform(begin(arguments), end(arguments), std::back_inserter(argument_pointers), [](std::string &arg)
		{
			return &arg[0];
		});
		argument_pointers.emplace_back(nullptr);

		detail::pipe stdout;
		if (parameters.stdout)
		{
			stdout = detail::make_pipe();
		}

		auto stdin = detail::make_pipe();
		auto child_error = detail::make_pipe();

		pid_t const forked = fork();
		if (forked < 0)
		{
			throw boost::system::system_error(errno, boost::system::system_category());
		}

		//child
		if (forked == 0)
		{
			auto const fail = [&child_error]() __attribute__ ((__noreturn__))
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

			if (parameters.stdout)
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

			if (dup2(stdin.read.handle, STDIN_FILENO) < 0)
			{
				fail();
			}
			stdin.close();

			child_error.read.close();
			detail::set_close_on_exec(child_error.write.handle);

			boost::filesystem::current_path(parameters.current_path);

			execvp(parameters.executable.c_str(), argument_pointers.data());
			fail();
		}

		//parent
		else
		{
			child_error.write.close();

			if (parameters.stdout)
			{
				stdout.write.close();
				detail::copy_all(stdout.read.handle, *parameters.stdout);
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
#endif //__linux__

#ifdef _WIN32
	int run_process(process_parameters const &)
	{
		throw std::logic_error("Si::run_process is not yet implemented on WIN32");
	}
#endif

	int run_process(
			boost::filesystem::path executable,
			std::vector<std::string> arguments,
			boost::filesystem::path current_path,
			Si::sink<char> &output)
	{
		Si::process_parameters parameters;
		parameters.executable = std::move(executable);
		parameters.arguments = std::move(arguments);
		parameters.current_path = std::move(current_path);
		parameters.out = &output;
		return Si::run_process(parameters);
	}
}
