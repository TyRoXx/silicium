#ifndef SILICIUM_NATIVE_FILE_DESCRIPTOR_HPP
#define SILICIUM_NATIVE_FILE_DESCRIPTOR_HPP

#include <silicium/throw_last_error.hpp>
#include <boost/config.hpp>
#ifdef _WIN32
#include <silicium/win32/win32.hpp>
#else
#include <unistd.h>
#endif

namespace Si
{
#ifdef _WIN32
	typedef HANDLE native_file_descriptor;

	native_file_descriptor const no_file_handle = INVALID_HANDLE_VALUE;

	inline void terminating_close(native_file_descriptor file) BOOST_NOEXCEPT
	{
		if (!CloseHandle(file))
		{
			throw_last_error();
		}
	}
#else
	typedef int native_file_descriptor;

	native_file_descriptor const no_file_handle = -1;

	inline void terminating_close(native_file_descriptor file) BOOST_NOEXCEPT
	{
		if (close(file) < 0)
		{
			// it is intended that this will terminate the process because of
			// noexcept
			throw_last_error();
		}
	}
#endif
}

#endif
