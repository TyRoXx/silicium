#ifndef SILICIUM_FILE_SOURCE_HPP
#define SILICIUM_FILE_SOURCE_HPP

#include <silicium/source.hpp>
#include <silicium/fast_variant.hpp>
#include <boost/system/error_code.hpp>
#include <unistd.h>

namespace Si
{
	using file_read_result = Si::fast_variant<std::size_t, boost::system::error_code>;

	inline auto make_file_source(int file, boost::iterator_range<char *> read_buffer)
	{
		return Si::make_generator_source<file_read_result>([file, read_buffer]() -> boost::optional<file_read_result>
		{
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
			return file_read_result{static_cast<std::size_t>(read_bytes)};
		});
	}
}

#endif
