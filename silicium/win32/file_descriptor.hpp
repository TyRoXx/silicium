#ifndef SILICIUM_WIN32_FILE_DESCRIPTOR_HPP
#define SILICIUM_WIN32_FILE_DESCRIPTOR_HPP

#include <boost/config.hpp>
#include <boost/system/system_error.hpp>
#include <boost/throw_exception.hpp>

#ifdef _WIN32
#	include <Windows.h>
#endif

namespace Si
{
	using native_file_handle = HANDLE;

	native_file_handle const no_file_handle = INVALID_HANDLE_VALUE;

	inline void terminating_close(native_file_handle file) BOOST_NOEXCEPT
	{
		if (!CloseHandle(file))
		{
			boost::throw_exception(boost::system::system_error(GetLastError(), boost::system::system_category()));
		}
	}
}

#endif
