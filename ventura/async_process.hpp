#ifndef VENTURA_ASYNC_PROCESS_HPP
#define VENTURA_ASYNC_PROCESS_HPP

#include <silicium/os_string.hpp>
#include <ventura/absolute_path.hpp>
#include <ventura/process_parameters.hpp>
#include <silicium/file_handle.hpp>
#include <silicium/process_handle.hpp>
#include <silicium/posix/pipe.hpp>
#include <silicium/observable/virtualized.hpp>
#include <silicium/observable/spawn_coroutine.hpp>
#include <silicium/observable/spawn_observable.hpp>
#include <silicium/observable/thread.hpp>
#include <silicium/observable/ref.hpp>
#include <silicium/sink/buffering_sink.hpp>
#include <ventura/absolute_path.hpp>
#include <silicium/asio/posting_observable.hpp>
#include <silicium/asio/process_output.hpp>
#include <silicium/std_threading.hpp>
#include <silicium/sink/append.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/future.hpp>

#if SILICIUM_HAS_EXCEPTIONS
#	include <boost/filesystem/operations.hpp>
#endif

#ifndef _WIN32
#	include <fcntl.h>
#	include <sys/wait.h>
#	ifdef __linux__
#		include <sys/prctl.h>
#	endif
#endif

//TODO: avoid the Boost filesystem operations that require exceptions
#define VENTURA_HAS_LAUNCH_PROCESS SILICIUM_HAS_EXCEPTIONS

namespace ventura
{
	struct async_process_parameters
	{
		absolute_path executable;

		/// the values for the child's argv[1...]
		std::vector<Si::os_string> arguments;

		/// must be an existing path, otherwise the child cannot launch properly
		absolute_path current_path;
	};

	struct async_process
	{
		Si::process_handle process;
#ifndef _WIN32
		Si::file_handle child_error;
#endif

		async_process() BOOST_NOEXCEPT
		{
		}

#ifdef _WIN32
		explicit async_process(Si::process_handle process) BOOST_NOEXCEPT
			: process(std::move(process))
		{
		}
#else
		explicit async_process(Si::process_handle process, Si::file_handle child_error) BOOST_NOEXCEPT
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

		Si::error_or<int> wait_for_exit() BOOST_NOEXCEPT
		{
#ifndef _WIN32
			int error = 0;
			ssize_t read_error = read(child_error.handle, &error, sizeof(error));
			if (read_error < 0)
			{
				return Si::get_last_error();
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
	
#if VENTURA_HAS_LAUNCH_PROCESS

#ifdef _WIN32
	namespace detail
	{
		inline Si::os_string build_command_line(std::vector<Si::os_string> const &arguments)
		{
			Si::os_string command_line;
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

	inline Si::error_or<async_process> launch_process(
		async_process_parameters parameters,
		Si::native_file_descriptor standard_input,
		Si::native_file_descriptor standard_output,
		Si::native_file_descriptor standard_error,
		std::vector<std::pair<Si::os_char const *, Si::os_char const *>> environment,
		environment_inheritance inheritance)
	{
		std::vector<Si::os_string> all_arguments;
		all_arguments.emplace_back(L"\"" + parameters.executable.underlying().wstring() + L"\"");
		all_arguments.insert(all_arguments.end(), parameters.arguments.begin(), parameters.arguments.end());
		Si::win32::winapi_string command_line = detail::build_command_line(all_arguments);

		SECURITY_ATTRIBUTES security = {};
		security.nLength = sizeof(security);
		security.bInheritHandle = TRUE;

		STARTUPINFOW startup = {};
		startup.cb = sizeof(startup);
		startup.dwFlags |= STARTF_USESTDHANDLES;
		startup.hStdError = standard_error;
		startup.hStdInput = standard_input;
		startup.hStdOutput = standard_output;

		DWORD flags = CREATE_NO_WINDOW;
		std::vector<WCHAR> environment_block;
		if (!environment.empty() || (inheritance == environment_inheritance::no_inherit))
		{
			flags |= CREATE_UNICODE_ENVIRONMENT;

			//@environment will contain pointers into this block of memory:
			std::vector<Si::os_char> mutable_parent_variables;

			switch (
#if SILICIUM_VC2012
				static_cast<int>
#endif
				(inheritance))
			{
			case environment_inheritance::inherit:
				{
					Si::os_char const * const parent_variables = GetEnvironmentStringsW();
					Si::os_char const *terminator_found = parent_variables;
					while (*terminator_found != L'\0')
					{
						terminator_found += wcslen(terminator_found) + 1;
					}
					mutable_parent_variables.assign(parent_variables, terminator_found + 1);
					for (auto i = mutable_parent_variables.begin(); *i != L'\0';)
					{
						auto assign = std::find(i, mutable_parent_variables.end(), L'=');
						if (assign == i)
						{
							//There is a variable called "=C:" that is generated by Windows (1).
							//The equality sign is not allowed in a variable name (2), but
							//they do it anyway.
							//(1) https://msdn.microsoft.com/en-us/library/windows/desktop/ms682425%28v=vs.85%29.aspx
							//(2) https://msdn.microsoft.com/en-us/library/windows/desktop/ms682653%28v=vs.85%29.aspx
							//The solution here: Treat any leading equality sign as a part of the name
							//to keep this simply.
							assign = std::find(assign + 1, mutable_parent_variables.end(), L'=');
						}
						*assign = L'\0';
						Si::os_char const * const key = &*i;
						Si::os_char const * const value = (&*assign) + 1;
						environment.emplace_back(std::make_pair(key, value));
						i = assign + 1 + wcslen(value) + 1;
					}
					break;
				}

			case environment_inheritance::no_inherit:
				{
					break;
				}
			}

			typedef std::pair<Si::os_char const *, Si::os_char const *> environment_entry;
			std::sort(environment.begin(), environment.end(), [](environment_entry const &left, environment_entry const &right)
			{
				return (wcscmp(left.first, right.first) < 0);
			});
			for (environment_entry const &entry : environment)
			{
				environment_block.insert(environment_block.end(), entry.first, entry.first + wcslen(entry.first));
				environment_block.emplace_back('=');
				std::size_t const zero_terminated = 1;
				environment_block.insert(environment_block.end(), entry.second, entry.second + wcslen(entry.second) + zero_terminated);
			}
			if (environment_block.empty())
			{
				{
					auto const key_and_assignment = Si::make_c_str_range(L"=C:=");
					environment_block.insert(environment_block.end(), key_and_assignment.begin(), key_and_assignment.end());
				}
				std::size_t const begin_of_value = environment_block.size();
				std::size_t estimated_value_size = 10;
				for (;;)
				{
					std::size_t const size_including_buffer = std::max(environment_block.size(), begin_of_value + estimated_value_size);
					environment_block.resize(size_including_buffer);
					DWORD const buffer_size = static_cast<DWORD>(size_including_buffer - begin_of_value);
					DWORD const actual_value_size = GetEnvironmentVariableW(
						L"=C:",
						environment_block.data() + begin_of_value,
						buffer_size);
					if (actual_value_size <= buffer_size)
					{
						environment_block.resize(begin_of_value + actual_value_size + 1);
						break;
					}
					estimated_value_size = actual_value_size;
				}
			}
			environment_block.emplace_back(L'\0');
		}

		PROCESS_INFORMATION process = {};
		if (!CreateProcessW(
			parameters.executable.c_str(), &command_line[0], &security, nullptr, TRUE,
			flags, environment_block.empty() ? NULL : environment_block.data(),
			parameters.current_path.c_str(), &startup, &process))
		{
			return Si::get_last_error();
		}

		Si::win32::unique_handle thread_closer(process.hThread);
		Si::process_handle process_closer(process.hProcess);
		return async_process(std::move(process_closer));
	}
#else
	inline Si::error_or<async_process> launch_process(
		async_process_parameters parameters,
		Si::native_file_descriptor standard_input,
		Si::native_file_descriptor standard_output,
		Si::native_file_descriptor standard_error,
		std::vector<std::pair<Si::os_char const *, Si::os_char const *>> environment,
		environment_inheritance inheritance)
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

		Si::pipe child_error = Si::make_pipe().move_value();

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

			boost::system::error_code ec = Si::detail::set_close_on_exec(child_error.write.handle);
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

#ifdef __linux__
			//kill the child when the parent exits
			if (prctl(PR_SET_PDEATHSIG, SIGHUP) < 0)
			{
				fail();
			}
#else
			//TODO: OSX etc
#endif

			switch (inheritance)
			{
			case environment_inheritance::inherit:
				for (auto const &var : environment)
				{
					int result = setenv(var.first, var.second, 1);
					if (result != 0)
					{
						fail_with_error(errno);
					}
				}
				execvp(parameters.executable.c_str(), argument_pointers.data());
				fail();
				break;

			case environment_inheritance::no_inherit:
				{
					std::vector<char *> environment_for_exec;
					for (auto const &entry : environment)
					{
						auto const first_length = std::strlen(entry.first);
						auto const second_length = std::strlen(entry.second);
						char * const formatted = new char[first_length + 1 + second_length + 1];
						std::copy_n(entry.first, first_length, formatted);
						formatted[first_length] = '=';
						std::copy_n(entry.second, second_length, formatted + first_length + 1);
						formatted[first_length + 1 + second_length] = '\0';
						environment_for_exec.emplace_back(formatted);
					}
					environment_for_exec.emplace_back(nullptr);
#ifdef __linux__
					execvpe
#else
					execve
#endif
						(parameters.executable.c_str(), argument_pointers.data(), environment_for_exec.data());
					fail();
					break;
				}
			}

			SILICIUM_UNREACHABLE();
		}

		//parent
		else
		{
			return async_process(Si::process_handle(forked), std::move(child_error.read));
		}
	}
#endif

#endif

#define VENTURA_HAS_EXPERIMENTAL_READ_FROM_ANONYMOUS_PIPE (SILICIUM_HAS_THREAD_OBSERVABLE && SILICIUM_HAS_BUFFERING_SINK)

#if VENTURA_HAS_EXPERIMENTAL_READ_FROM_ANONYMOUS_PIPE && defined(_WIN32)
	namespace win32
	{
		template <class ByteSink>
		void copy_whole_pipe(HANDLE pipe_in, ByteSink &&sink_out, boost::shared_future<void> stop_polling)
		{
			auto buffered_out = make_buffering_sink(std::forward<ByteSink>(sink_out));
			for (;;)
			{
				auto buffer = buffered_out.make_append_space((std::numeric_limits<DWORD>::max)());
				DWORD read_bytes = 0;
				DWORD available = 0;
				DWORD left = 0;
				BOOL const peeked = PeekNamedPipe(pipe_in, buffer.begin(), static_cast<DWORD>(buffer.size()), &read_bytes, &available, &left);
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
					switch (stop_polling.wait_for(boost::chrono::milliseconds(300)))
					{
					case std::future_status::ready:
						return;

					case std::future_status::deferred:
					case std::future_status::timeout:
						break;
					}
					continue;
				}
				if (ReadFile(pipe_in, buffer.begin(), available, &read_bytes, nullptr))
				{
					assert(available == read_bytes);
					buffered_out.make_append_space(read_bytes);
					buffered_out.flush_append_space();
				}
				else
				{
					Si::throw_last_error();
				}
			}
			buffered_out.flush();
		}
	}
#endif
	namespace experimental
	{
#if VENTURA_HAS_EXPERIMENTAL_READ_FROM_ANONYMOUS_PIPE
		//TODO: find a more generic API for reading from a pipe portably
		template <class CharSink>
		void read_from_anonymous_pipe(boost::asio::io_service &io, CharSink &&destination, Si::file_handle file, boost::shared_future<void> stop_polling)
		{
#ifdef _WIN32
			auto copyable_file = Si::to_shared(std::move(file));
			auto work = std::make_shared<boost::asio::io_service::work>(io);
			auto stop_polling_shared = Si::to_shared(std::move(stop_polling));
			Si::spawn_observable(
				Si::asio::make_posting_observable(
					io,
					Si::make_thread_observable<Si::std_threading>([work, copyable_file, destination, stop_polling_shared]()
					{
						win32::copy_whole_pipe(copyable_file->handle, destination, std::move(*stop_polling_shared));
						return Si::nothing();
					})
				)
			);
#elif SILICIUM_HAS_SPAWN_COROUTINE
			boost::ignore_unused_variable_warning(stop_polling);
			auto copyable_file = Si::to_shared(std::move(file));
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
#else
			boost::ignore_unused_variable_warning(stop_polling);
			typedef typename std::decay<CharSink>::type clean_destination;
			struct pipe_reader : std::enable_shared_from_this<pipe_reader>
			{
				pipe_reader(boost::asio::io_service &io, Si::file_handle file, clean_destination destination)
					: m_output(Si::make_unique<Si::process_output::stream>(io, file.handle))
					, m_destination(std::move(destination))
				{
					file.release();
				}

				void start()
				{
					auto this_ = this->shared_from_this();
					m_output.async_get_one(Si::make_function_observer([this_](optional<error_or<memory_range>> piece)
					{
						assert(piece);
						if (piece->is_error())
						{
							return;
						}
						Si::memory_range data = piece->get();
						if (data.empty())
						{
							return;
						}
						Si::append(this_->m_destination, data);
						this_->start();
					}));
				}

			private:

				Si::process_output m_output;
				clean_destination m_destination;
			};
			auto reader = std::make_shared<pipe_reader>(io, std::move(file), std::forward<CharSink>(destination));
			reader->start();
#endif
		}
#endif
	}
}

#endif
