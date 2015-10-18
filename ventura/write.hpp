#ifndef SILICIUM_WRITE_HPP
#define SILICIUM_WRITE_HPP

#include <silicium/error_or.hpp>
#include <silicium/get_last_error.hpp>
#include <silicium/native_file_descriptor.hpp>
#include <silicium/memory_range.hpp>
#include <limits>

namespace ventura
{
	SILICIUM_USE_RESULT
	inline Si::error_or<std::size_t> write(Si::native_file_descriptor file, Si::memory_range data)
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
				return Si::get_last_error();
			}
#else
			ssize_t const written = ::write(file, data.begin() + total_written, static_cast<std::size_t>(data.size()) - total_written);
			if (written < 0)
			{
				return Si::get_last_error();
			}
#endif
			if (written == 0)
			{
				break;
			}
			total_written += static_cast<std::size_t>(written);
		}
		while (total_written < static_cast<std::size_t>(data.size()));
		return total_written;
	}
}

#endif
