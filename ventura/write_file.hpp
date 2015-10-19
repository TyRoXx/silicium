#ifndef VENTURA_WRITE_FILE_HPP
#define VENTURA_WRITE_FILE_HPP

#include <ventura/open.hpp>
#include <silicium/c_string.hpp>
#include <ventura/sink/file_sink.hpp>
#include <silicium/sink/append.hpp>

#define VENTURA_HAS_WRITE_FILE SILICIUM_HAS_FILE_SINK

namespace ventura
{
#if VENTURA_HAS_WRITE_FILE
	SILICIUM_USE_RESULT
	inline boost::system::error_code write_file(Si::native_path_string name, Si::memory_range data)
	{
		Si::error_or<Si::file_handle> const file = overwrite_file(name);
		if (file.is_error())
		{
			return file.error();
		}
		file_sink sink(file.get().handle);
		return Si::append(sink, file_sink_element{data});
	}
#endif
}

#endif
