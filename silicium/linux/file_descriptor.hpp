#ifndef SILICIUM_LINUX_FILE_DESCRIPTOR_HPP
#define SILICIUM_LINUX_FILE_DESCRIPTOR_HPP

#include <boost/config.hpp>
#include <boost/system/system_error.hpp>
#include <boost/throw_exception.hpp>
#include <unistd.h>

namespace Si
{
	typedef int native_file_descriptor;

	native_file_descriptor const no_file_handle = -1;

	inline void terminating_close(native_file_descriptor file) BOOST_NOEXCEPT
	{
		if (close(file) < 0)
		{
			//it is intended that this will terminate the process because of noexcept
			boost::throw_exception(boost::system::system_error(errno, boost::system::system_category()));
		}
	}
}

#endif
