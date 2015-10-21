#ifndef SILICIUM_READ_FILE_HPP
#define SILICIUM_READ_FILE_HPP

#include <silicium/error_or.hpp>
#include <silicium/memory_range.hpp>
#include <silicium/native_file_descriptor.hpp>

namespace Si
{
	inline Si::error_or<std::size_t> read(Si::native_file_descriptor file, Si::mutable_memory_range destination)
	{
#ifdef _WIN32
		DWORD read_bytes = 0;
		DWORD const reading =
		    static_cast<DWORD>(std::min<size_t>(destination.size(), std::numeric_limits<DWORD>::max()));
		if (!ReadFile(file, destination.begin(), reading, &read_bytes, nullptr))
		{
			DWORD error = GetLastError();
			if (error == ERROR_BROKEN_PIPE)
			{
				// end of pipe
				return static_cast<std::size_t>(0);
			}
			return boost::system::error_code(error, boost::system::system_category());
		}
#else
		ssize_t const read_bytes = ::read(file, destination.begin(), static_cast<std::size_t>(destination.size()));
		if (read_bytes < 0)
		{
			return Si::get_last_error();
		}
#endif
		if (read_bytes == 0)
		{
			// end of file
			return static_cast<std::size_t>(0);
		}
		return static_cast<std::size_t>(read_bytes);
	}
}

#endif
