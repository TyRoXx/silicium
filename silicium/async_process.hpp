#ifndef SILICIUM_ASYNC_PROCESS_HPP
#define SILICIUM_ASYNC_PROCESS_HPP

#include <silicium/os_string.hpp>
#include <silicium/process_parameters.hpp>
#include <silicium/file_handle.hpp>
#include <silicium/process_handle.hpp>
#include <silicium/error_or.hpp>
#include <silicium/posix/pipe.hpp>
#include <silicium/observable/virtualized.hpp>
#include <silicium/observable/spawn_coroutine.hpp>
#include <silicium/observable/ref.hpp>
#include <silicium/asio/process_output.hpp>
#include <silicium/absolute_path.hpp>
#include <silicium/asio/posting_observable.hpp>

#ifndef _WIN32
#	include <fcntl.h>
#	include <sys/wait.h>
#	include <sys/prctl.h>
#endif

namespace Si
{
	struct async_process_parameters
	{
		Si::absolute_path executable;

		/// the values for the child's argv[1...]
		std::vector<os_string> arguments;

		/// must be an existing path, otherwise the child cannot launch properly
		Si::absolute_path current_path;
	};

	struct async_process
	{
		process_handle process;
#ifndef _WIN32
		file_handle child_error;
#endif

		async_process() BOOST_NOEXCEPT
		{
		}

#ifdef _WIN32
		explicit async_process(process_handle process) BOOST_NOEXCEPT
			: process(std::move(process))
		{
		}
#else
		explicit async_process(process_handle process, file_handle child_error) BOOST_NOEXCEPT
			: process(std::move(process))
			, child_error(std::move(child_error))
		{
		}
#endif

#if SILICIUM_COMPILER_GENERATES_MOVES
		async_process(async_process &&) BOOST_NOEXCEPT = default;
		async_process &operator = (async_process &&)BOOST_NOEXCEPT = default;
#else
		async_process(async_process &&other) BOOST_NOEXCEPT
			: process(std::move(other.process))
#ifndef _WIN32
			, child_error(std::move(other.child_error))
#endif
		{
		}

		async_process &operator = (async_process &&other) BOOST_NOEXCEPT
		{
			process = std::move(other.process);
#ifndef _WIN32
			child_error = std::move(other.child_error);
#endif
			return *this;
		}

		SILICIUM_DELETED_FUNCTION(async_process(async_process const &))
		SILICIUM_DELETED_FUNCTION(async_process &operator = (async_process const &))
	public:
#endif

		~async_process() BOOST_NOEXCEPT
		{
		}

		error_or<int> wait_for_exit() BOOST_NOEXCEPT
		{
#ifndef _WIN32
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
#endif
			return process.wait_for_exit();
		}
	};
	
#ifdef _WIN32
	namespace detail
	{
		inline os_string build_command_line(std::vector<os_string> const &arguments)
		{
			os_string command_line;
			for (auto a = begin(arguments); a != end(arguments); ++a)
			{
				if (a != begin(arguments))
				{
					command_line += L" ";
				}
				command_line += *a;
			}
			return command_line;
		}
	}

	inline error_or<async_process> launch_process(
		async_process_parameters parameters,
		native_file_descriptor standard_input,
		native_file_descriptor standard_output,
		native_file_descriptor standard_error)
	{
		std::vector<os_string> all_arguments;
		all_arguments.emplace_back(L"\"" + parameters.executable.underlying().wstring() + L"\"");
		all_arguments.insert(all_arguments.end(), parameters.arguments.begin(), parameters.arguments.end());
		win32::winapi_string command_line = detail::build_command_line(all_arguments);

		SECURITY_ATTRIBUTES security{};
		security.nLength = sizeof(security);
		security.bInheritHandle = TRUE;

		STARTUPINFOW startup{};
		startup.cb = sizeof(startup);
		startup.dwFlags |= STARTF_USESTDHANDLES;
		startup.hStdError = standard_error;
		startup.hStdInput = standard_input;
		startup.hStdOutput = standard_output;
		PROCESS_INFORMATION process{};
		if (!CreateProcessW(parameters.executable.c_str(), &command_line[0], &security, nullptr, TRUE, CREATE_NO_WINDOW, nullptr, parameters.current_path.c_str(), &startup, &process))
		{
			return boost::system::error_code(::GetLastError(), boost::system::native_ecat);
		}

		win32::unique_handle thread_closer(process.hThread);
		process_handle process_closer(process.hProcess);
		return async_process(std::move(process_closer));
	}
#else
	inline error_or<async_process> launch_process(
		async_process_parameters parameters,
		native_file_descriptor standard_input,
		native_file_descriptor standard_output,
		native_file_descriptor standard_error)
	{
		auto executable = parameters.executable.underlying();
		auto arguments = parameters.arguments;
		std::vector<char *> argument_pointers;
		argument_pointers.emplace_back(const_cast<char *>(executable.c_str()));
		std::transform(begin(arguments), end(arguments), std::back_inserter(argument_pointers), [](Si::noexcept_string &arg)
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

			boost::filesystem::current_path(parameters.current_path.to_boost_path(), ec);
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

	namespace experimental
	{
		//TODO: find a more generic API for reading from a pipe portably
		template <class CharSink>
		void read_from_anonymous_pipe(boost::asio::io_service &io, CharSink &&destination, Si::file_handle file)
		{
			auto copyable_file = Si::to_shared(std::move(file));
#ifdef _WIN32
			auto work = std::make_shared<boost::asio::io_service::work>(io);
			Si::spawn_observable(
				Si::asio::make_posting_observable(
					io,
					Si::make_thread_observable<Si::std_threading>([work, copyable_file, destination]()
					{
						Si::win32::copy_whole_pipe(copyable_file->handle, destination);
						return Si::nothing();
					})
				)
			);
#else
			Si::spawn_coroutine([&io, destination, copyable_file](Si::spawn_context yield)
			{
				Si::process_output output_reader(Si::make_unique<Si::process_output::stream>(io, copyable_file->handle));
				copyable_file->release();
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
#endif
		}
	}
}

#endif
