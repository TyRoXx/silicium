#ifndef SILICIUM_WIN32_NATIVE_FILE_DESCRIPTOR_HPP
#define SILICIUM_WIN32_NATIVE_FILE_DESCRIPTOR_HPP

#include <silicium/throw_last_error.hpp>
#include <silicium/win32/win32.hpp>
#include <boost/config.hpp>

namespace ventura
{
	typedef HANDLE native_file_descriptor;

	native_file_descriptor const no_file_handle = INVALID_HANDLE_VALUE;

	inline void terminating_close(native_file_descriptor file) BOOST_NOEXCEPT
	{
		if (!CloseHandle(file))
		{
			throw_last_error();
		}
	}
}

#endif
