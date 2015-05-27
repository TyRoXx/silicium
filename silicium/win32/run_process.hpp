#ifndef SILICIUM_WIN32_PROCESS_HPP
#define SILICIUM_WIN32_PROCESS_HPP

#include <silicium/process_parameters.hpp>
#include <silicium/error_code.hpp>
#include <silicium/posix/pipe.hpp>
#include <silicium/os_string.hpp>
#include <silicium/sink/ptr_sink.hpp>
#include <silicium/sink/buffering_sink.hpp>
#include <cassert>
#include <stdexcept>

namespace Si
{
	namespace win32
	{
		inline winapi_string build_command_line(std::vector<std::string> const &arguments)
		{
			winapi_string command_line;
			for (auto a = begin(arguments); a != end(arguments); ++a)
			{
				if (a != begin(arguments))
				{
					command_line += L" ";
				}
				command_line += utf8_to_winapi_string(a->data(), a->size());
			}
			return command_line;
		}

		template <class ByteSink>
		void copy_whole_pipe(HANDLE pipe_in, ByteSink &&sink_out)
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
					auto buffer = buffered_out.make_append_space(1);
					DWORD read_bytes = 0;
					BOOL const read_result = ReadFile(pipe_in, buffer.begin(), static_cast<DWORD>(buffer.size()), &read_bytes, nullptr);
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
				if (ReadFile(pipe_in, buffer.begin(), available, &read_bytes, nullptr))
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
	}

	SILICIUM_DEPRECATED
	inline int run_process(process_parameters const &parameters)
	{
		win32::winapi_string const &executable = parameters.executable.wstring();
		std::vector<std::string> all_arguments;
		all_arguments.emplace_back("\"" + parameters.executable.string() + "\"");
		all_arguments.insert(all_arguments.end(), parameters.arguments.begin(), parameters.arguments.end());
		win32::winapi_string command_line = win32::build_command_line(all_arguments);
		SECURITY_ATTRIBUTES security{};
		security.nLength = sizeof(security);
		security.bInheritHandle = TRUE;
		auto output = SILICIUM_MOVE_IF_COMPILER_LACKS_RVALUE_QUALIFIERS(make_pipe().get());
		STARTUPINFOW startup{};
		startup.cb = sizeof(startup);
		startup.dwFlags |= STARTF_USESTDHANDLES;
		startup.hStdError = parameters.out ? output.write.handle : INVALID_HANDLE_VALUE;
		startup.hStdInput = INVALID_HANDLE_VALUE;
		startup.hStdOutput = parameters.out ? output.write.handle : INVALID_HANDLE_VALUE;
		PROCESS_INFORMATION process{};
		if (!CreateProcessW(executable.c_str(), &command_line[0], &security, nullptr, TRUE, CREATE_NO_WINDOW, nullptr, parameters.current_path.c_str(), &startup, &process))
		{
			throw boost::system::system_error(::GetLastError(), boost::system::native_ecat);
		}
		win32::unique_handle thread_closer(process.hThread);
		win32::unique_handle process_closer(process.hProcess);
		output.write.close();
		if (parameters.out)
		{
			win32::copy_whole_pipe(output.read.handle, ref_sink(*parameters.out));
		}
		WaitForSingleObject(process.hProcess, INFINITE);
		DWORD exit_code = 1;
		if (!GetExitCodeProcess(process.hProcess, &exit_code))
		{
			throw boost::system::system_error(::GetLastError(), boost::system::native_ecat);
		}
		return exit_code;
	}
}

#endif
