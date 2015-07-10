#ifndef SILICIUM_WRITE_HPP
#define SILICIUM_WRITE_HPP

#include <silicium/error_or.hpp>
#include <silicium/native_file_descriptor.hpp>
#include <silicium/memory_range.hpp>
#include <limits>

namespace Si
{
	SILICIUM_USE_RESULT
	inline error_or<std::size_t> write(native_file_descriptor file, memory_range data)
	{
		std::size_t total_written = 0;
		do
		{
#ifdef _WIN32
			DWORD written = 0;
			assert(std::numeric_limits<decltype(total_written)>::max() >= std::numeric_limits<decltype(written)>::max());
			DWORD const piece = static_cast<DWORD>(std::min(
				static_cast<std::size_t>(std::numeric_limits<DWORD>::max()),
				data.size() - total_written
			));
			if (!WriteFile(file, data.begin() + total_written, piece, &written, nullptr))
			{
				return boost::system::error_code(GetLastError(), boost::system::system_category());
			}
#else
			ssize_t const written = ::write(file, data.begin() + total_written, data.size() - total_written);
			if (written < 0)
			{
				return boost::system::error_code(errno, boost::system::system_category());
			}
#endif
			if (written == 0)
			{
				break;
			}
			total_written += written;
		}
		while (total_written < static_cast<std::size_t>(data.size()));
		return total_written;
	}
}

#endif
