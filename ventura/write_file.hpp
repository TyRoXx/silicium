#ifndef SILICIUM_WRITE_FILE_HPP
#define SILICIUM_WRITE_FILE_HPP

#include <ventura/open.hpp>
#include <silicium/c_string.hpp>
#include <ventura/sink/file_sink.hpp>
#include <silicium/sink/append.hpp>

#define SILICIUM_HAS_WRITE_FILE SILICIUM_HAS_FILE_SINK

namespace Si
{
#if SILICIUM_HAS_WRITE_FILE
	SILICIUM_USE_RESULT
	inline boost::system::error_code write_file(native_path_string name, memory_range data)
	{
		Si::error_or<Si::file_handle> const file = Si::overwrite_file(name);
		if (file.is_error())
		{
			return file.error();
		}
		Si::file_sink sink(file.get().handle);
		return Si::append(sink, Si::file_sink_element{data});
	}
#endif
}

#endif
