#ifndef SILICIUM_LINUX_OPEN_HPP
#define SILICIUM_LINUX_OPEN_HPP

#include <silicium/error_or.hpp>
#include <silicium/linux/file_descriptor.hpp>
#include <boost/filesystem/path.hpp>
#include <fcntl.h>

namespace Si
{
	namespace linux
	{
		inline error_or<Si::linux::file_descriptor> open_reading(boost::filesystem::path const &name)
		{
			int const fd = ::open(name.c_str(), O_RDONLY);
			if (fd < 0)
			{
				return boost::system::error_code(errno, boost::system::system_category());
			}
			return Si::linux::file_descriptor(fd);
		}
	}
}

#endif
