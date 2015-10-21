#ifndef SILICIUM_LINUX_NATIVE_FILE_DESCRIPTOR_HPP
#define SILICIUM_LINUX_NATIVE_FILE_DESCRIPTOR_HPP

#include <silicium/throw_last_error.hpp>
#include <boost/config.hpp>
#include <unistd.h>

namespace Si
{
	typedef int native_file_descriptor;

	native_file_descriptor const no_file_handle = -1;

	inline void terminating_close(native_file_descriptor file) BOOST_NOEXCEPT
	{
		if (close(file) < 0)
		{
			// it is intended that this will terminate the process because of noexcept
			throw_last_error();
		}
	}
}

#endif
