#ifndef VENTURA_FILE_SOURCE_HPP
#define VENTURA_FILE_SOURCE_HPP

#include <silicium/read_file.hpp>
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

namespace ventura
{
	typedef Si::error_or<Si::memory_range> file_read_result;

	inline auto make_file_source(Si::native_file_descriptor file, Si::iterator_range<char *> read_buffer)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
		-> Si::generator_source<std::function<Si::optional<file_read_result>()>>
#endif
	{
		return Si::make_generator_source(
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
			std::function<Si::optional<file_read_result>()>
#endif
			([file, read_buffer]() -> Si::optional<file_read_result>
		{
			Si::error_or<std::size_t> read_result = read(file, read_buffer);
			if (read_result.is_error())
			{
				return file_read_result(read_result.error());
			}
			else if (read_result.get() == 0)
			{
				return Si::none;
			}
			return file_read_result(Si::make_memory_range(read_buffer.begin(), read_buffer.begin() + read_result.get()));
		}));
	}
}

#endif
