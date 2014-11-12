#ifndef SILICIUM_FILE_SOURCE_HPP
#define SILICIUM_FILE_SOURCE_HPP

#include <silicium/source/generator_source.hpp>
#include <silicium/error_or.hpp>
#include <silicium/file_descriptor.hpp>
#include <boost/system/error_code.hpp>
#include <functional>

#ifdef __linux__
#	include <unistd.h>
#endif

namespace Si
{
	typedef Si::error_or<std::size_t> file_read_result;

	inline auto make_file_source(native_file_handle file, iterator_range<char *> read_buffer)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
		-> generator_source<std::function<boost::optional<file_read_result>()>>
#endif
	{
		return make_generator_source(
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
			std::function<boost::optional<file_read_result>()>
#endif
			([file, read_buffer]() -> boost::optional<file_read_result>
		{
#ifdef _WIN32
			DWORD read_bytes = 0;
			DWORD const reading = static_cast<DWORD>(std::min<size_t>(read_buffer.size(), std::numeric_limits<DWORD>::max()));
			if (!ReadFile(file, read_buffer.begin(), reading, &read_bytes, nullptr))
			{
				return file_read_result{boost::system::error_code(GetLastError(), boost::system::system_category())};
			}
#else
			ssize_t const read_bytes = ::read(file, read_buffer.begin(), read_buffer.size());
			if (read_bytes < 0)
			{
				return file_read_result{boost::system::error_code(errno, boost::system::system_category())};
			}
#endif
			if (read_bytes == 0)
			{
				//end of file
				return boost::none;
			}
			return file_read_result{static_cast<std::size_t>(read_bytes)};
		}));
	}
}

#endif
