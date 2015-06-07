#ifndef SILICIUM_FILE_SOURCE_HPP
#define SILICIUM_FILE_SOURCE_HPP

#include <silicium/source/generator_source.hpp>
#include <silicium/error_or.hpp>
#include <silicium/config.hpp>
#include <silicium/memory_range.hpp>
#include <silicium/file_handle.hpp>
#include <boost/system/error_code.hpp>
#include <functional>

#ifdef __linux__
#	include <unistd.h>
#endif

namespace Si
{
	inline error_or<std::size_t> read(native_file_descriptor file, mutable_memory_range destination)
	{
#ifdef _WIN32
		DWORD read_bytes = 0;
		DWORD const reading = static_cast<DWORD>(std::min<size_t>(destination.size(), std::numeric_limits<DWORD>::max()));
		if (!ReadFile(file, destination.begin(), reading, &read_bytes, nullptr))
		{
			DWORD error = GetLastError();
			if (error == ERROR_BROKEN_PIPE)
			{
				//end of pipe
				return static_cast<std::size_t>(0);
			}
			return boost::system::error_code(error, boost::system::system_category());
		}
#else
		ssize_t const read_bytes = ::read(file, destination.begin(), destination.size());
		if (read_bytes < 0)
		{
			return boost::system::error_code(errno, boost::system::system_category());
		}
#endif
		if (read_bytes == 0)
		{
			//end of file
			return static_cast<std::size_t>(0);
		}
		return static_cast<std::size_t>(read_bytes);
	}

	typedef Si::error_or<Si::memory_range> file_read_result;

	inline auto make_file_source(native_file_descriptor file, iterator_range<char *> read_buffer)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
		-> generator_source<std::function<Si::optional<file_read_result>()>>
#endif
	{
		return make_generator_source(
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
			std::function<Si::optional<file_read_result>()>
#endif
			([file, read_buffer]() -> Si::optional<file_read_result>
		{
			error_or<std::size_t> read_result = read(file, read_buffer);
			if (read_result.is_error())
			{
				return file_read_result{read_result.error()};
			}
			else if (read_result.get() == 0)
			{
				return none;
			}
			return file_read_result{make_memory_range(read_buffer.begin(), read_buffer.begin() + read_result.get())};
		}));
	}
}

#endif
