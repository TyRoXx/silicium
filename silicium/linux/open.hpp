#ifndef SILICIUM_LINUX_OPEN_HPP
#define SILICIUM_LINUX_OPEN_HPP

#include <silicium/error_or.hpp>
#include <silicium/file_descriptor.hpp>
#include <boost/filesystem/path.hpp>

#ifdef __linux__
#	include <fcntl.h>
#endif

namespace Si
{
	inline error_or<file_descriptor> open_reading(boost::filesystem::path const &name)
	{
		native_file_handle const fd = ::open(name.c_str(), O_RDONLY);
		if (fd < 0)
		{
			return boost::system::error_code(errno, boost::system::system_category());
		}
		return file_descriptor(fd);
	}
}

#endif
