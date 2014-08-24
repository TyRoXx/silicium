#ifndef SILICIUM_FILE_SOURCE_HPP
#define SILICIUM_FILE_SOURCE_HPP

#include <silicium/source.hpp>
#include <silicium/fast_variant.hpp>
#include <silicium/file_descriptor.hpp>
#include <boost/system/error_code.hpp>
#include <functional>

#ifdef __linux__
#	include <unistd.h>
#endif

namespace Si
{
	using file_read_result = Si::fast_variant<std::size_t, boost::system::error_code>;

	inline auto make_file_source(native_file_handle file, boost::iterator_range<char *> read_buffer)
#ifdef _MSC_VER
		-> generator_source<file_read_result, std::function<boost::optional<file_read_result>()>>
#endif
	{
		return make_generator_source<file_read_result>(
#ifdef _MSC_VER
			std::function<boost::optional<file_read_result>()>
#endif
			([file, read_buffer]() -> boost::optional<file_read_result>
		{
#ifdef _WIN32
			DWORD read_bytes = 0;
			if (!ReadFile(file, read_buffer.begin(), read_buffer.size(), &read_bytes, nullptr))
			{
				return file_read_result{boost::system::error_code(GetLastError(), boost::system::system_category())};
			}
#else
			ssize_t const read_bytes = ::read(file, read_buffer.begin(), read_buffer.size());
			if (read_bytes == 0)
			{
				//end of file
				return boost::none;
			}
			if (read_bytes < 0)
			{
				return file_read_result{boost::system::error_code(errno, boost::system::system_category())};
			}
#endif
			return file_read_result{static_cast<std::size_t>(read_bytes)};
		}));
	}
}

#endif
