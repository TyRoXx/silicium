#ifndef SILICIUM_WRITE_FILE_HPP
#define SILICIUM_WRITE_FILE_HPP

#include <silicium/open.hpp>
#include <silicium/c_string.hpp>
#include <silicium/sink/file_sink.hpp>

namespace Si
{
	SILICIUM_USE_RESULT
	boost::system::error_code write_file(native_path_string name, char const *data, std::size_t size)
	{
		Si::error_or<Si::file_handle> const file = Si::overwrite_file(name);
		if (file.is_error())
		{
			return file.error();
		}
		Si::file_sink sink(file.get().handle);
		return Si::append(sink, Si::file_sink_element{Si::make_memory_range(data, size)});
	}
}

#endif
