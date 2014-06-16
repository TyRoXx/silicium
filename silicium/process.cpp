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

#ifdef _WIN32
#include <Windows.h>
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
		if (parameters.out)
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

			if (parameters.out)
			{
				stdout.write.close();
				detail::copy_all(stdout.read.handle, *parameters.out);
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
	namespace
	{
		typedef std::basic_string<WCHAR> winapi_string;

		winapi_string utf8_to_winapi_string(char const *original, size_t length)
		{
			if (length > static_cast<size_t>((std::numeric_limits<int>::max)()))
			{
				throw std::invalid_argument("Input string is too long for WinAPI");
			}
			int const output_size = MultiByteToWideChar(CP_UTF8, 0, original, static_cast<int>(length), nullptr, 0);
			assert(output_size >= 0);
			if (output_size == 0)
			{
				assert(GetLastError() == ERROR_NO_UNICODE_TRANSLATION);
				throw std::invalid_argument("Input string is not UTF-8");
			}
			winapi_string result;
			result.resize(static_cast<size_t>(output_size));
			if (!result.empty())
			{
				MultiByteToWideChar(CP_UTF8, 0, original, static_cast<int>(length), &result[0], result.size());
			}
			return result;
		}

		winapi_string utf8_to_winapi_string(std::string const &str)
		{
			return utf8_to_winapi_string(str.data(), str.size());
		}

		winapi_string build_command_line(std::vector<std::string> const &arguments)
		{
			winapi_string command_line;
			for (auto a = begin(arguments); a != end(arguments); ++a)
			{
				if ((a + 1) != end(arguments))
				{
					command_line += L" ";
				}
				command_line += utf8_to_winapi_string(a->data(), a->size());
			}
			return command_line;
		}

		struct handle_closer
		{
			void operator()(HANDLE h) const
			{
				CloseHandle(h);
			}
		};

		typedef std::unique_ptr<VOID, handle_closer> unique_handle;

		struct pipe
		{
			unique_handle read, write;

			pipe() BOOST_NOEXCEPT
			{
			}

			pipe(pipe &&other) BOOST_NOEXCEPT
			{
				swap(other);
			}

			pipe &operator = (pipe &&other) BOOST_NOEXCEPT
			{
				swap(other);
				return *this;
			}

			void swap(pipe &other) BOOST_NOEXCEPT
			{
				read.swap(other.read);
				write.swap(other.write);
			}
		};

		pipe create_anonymous_pipe()
		{
			HANDLE read, write;
			if (!CreatePipe(&read, &write, nullptr, 0))
			{
				throw boost::system::system_error(::GetLastError(), boost::system::native_ecat);
			}
			pipe result;
			result.read.reset(read);
			result.write.reset(write);
			return result;
		}
	}

	int run_process(process_parameters const &parameters)
	{
		winapi_string const executable = utf8_to_winapi_string(parameters.executable.string());
		winapi_string command_line = build_command_line(parameters.arguments);
		SECURITY_ATTRIBUTES security{};
		security.nLength = sizeof(security);
		security.bInheritHandle = TRUE;
		auto output = create_anonymous_pipe();
		STARTUPINFOW startup{};
		startup.cb = sizeof(startup);
		startup.dwFlags |= STARTF_USESTDHANDLES;
		startup.hStdError = output.write.get();
		startup.hStdInput = INVALID_HANDLE_VALUE;
		startup.hStdOutput = output.write.get();
		PROCESS_INFORMATION process{};
		if (!CreateProcessW(executable.c_str(), &command_line[0], &security, nullptr, TRUE, CREATE_NO_WINDOW, nullptr, parameters.current_path.c_str(), &startup, &process))
		{
			throw boost::system::system_error(::GetLastError(), boost::system::native_ecat);
		}
		//output.write.reset();
		if (parameters.out)
		{
			Si::buffering_sink<char> buffered_out(*parameters.out);
			for (;;)
			{
				auto buffer = buffered_out.make_append_space((std::numeric_limits<DWORD>::max)());
				DWORD read_bytes = 0;
				DWORD available = 0;
				DWORD left = 0;
				BOOL const peeked = PeekNamedPipe(output.read.get(), buffer.begin(), static_cast<DWORD>(buffer.size()), &read_bytes, &available, &left);
				if (!peeked)
				{
					auto error = ::GetLastError();
					if (error == ERROR_BROKEN_PIPE)
					{
						buffered_out.make_append_space(read_bytes);
						buffered_out.flush_append_space();
						break;
					}
					throw boost::system::system_error(error, boost::system::native_ecat);
				}
				if (available == 0)
				{
					auto buffer = buffered_out.make_append_space(1);
					DWORD read_bytes = 0;
					BOOL const read_result = ReadFile(output.read.get(), buffer.begin(), static_cast<DWORD>(buffer.size()), &read_bytes, nullptr);
					if (read_result)
					{
						buffered_out.flush_append_space();
						continue;
					}
					else
					{
						auto error = ::GetLastError();
						if (error == ERROR_BROKEN_PIPE)
						{
							buffered_out.make_append_space(read_bytes);
							buffered_out.flush_append_space();
							break;
						}
						throw boost::system::system_error(error, boost::system::native_ecat);
					}
				}
				if (ReadFile(output.read.get(), buffer.begin(), available, &read_bytes, nullptr))
				{
					assert(available == read_bytes);
					buffered_out.make_append_space(read_bytes);
					buffered_out.flush_append_space();
				}
				else
				{
					throw boost::system::system_error(::GetLastError(), boost::system::native_ecat);
				}
			}
			buffered_out.flush();
		}
		WaitForSingleObject(process.hProcess, INFINITE);
		DWORD exit_code = 1;
		if (!GetExitCodeProcess(process.hProcess, &exit_code))
		{
			throw boost::system::system_error(::GetLastError(), boost::system::native_ecat);
		}
		CloseHandle(process.hThread);
		CloseHandle(process.hProcess);
		return exit_code;
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
