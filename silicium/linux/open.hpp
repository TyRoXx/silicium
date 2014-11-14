#ifndef SILICIUM_LINUX_OPEN_HPP
#define SILICIUM_LINUX_OPEN_HPP

#include <silicium/error_or.hpp>
#include <silicium/file_handle.hpp>
#include <boost/filesystem/path.hpp>

#ifdef __linux__
#	include <fcntl.h>
#endif

namespace Si
{
	inline error_or<file_handle> open_reading(boost::filesystem::path const &name)
	{
		native_file_descriptor const fd = ::open(name.c_str(), O_RDONLY);
		if (fd < 0)
		{
			return boost::system::error_code(errno, boost::system::system_category());
		}
		return file_handle(fd);
	}

	inline error_or<file_handle> create_file(boost::filesystem::path const &name)
	{
		native_file_descriptor const fd = ::open(name.c_str(), O_WRONLY | O_CREAT | O_EXCL, S_IWUSR | S_IRUSR | S_IRGRP | S_IROTH);
		if (fd < 0)
		{
			return boost::system::error_code(errno, boost::system::system_category());
		}
		return file_handle(fd);
	}

	inline error_or<file_handle> overwrite_file(boost::filesystem::path const &name)
	{
		native_file_descriptor const fd = ::open(name.c_str(), O_RDWR | O_TRUNC | O_CREAT, S_IWUSR | S_IRUSR | S_IRGRP | S_IROTH);
		if (fd < 0)
		{
			return boost::system::error_code(errno, boost::system::system_category());
		}
		return file_handle(fd);
	}

	inline error_or<file_handle> open_read_write(boost::filesystem::path const &name)
	{
		native_file_descriptor const fd = ::open(name.c_str(), O_RDWR | O_CREAT, S_IWUSR | S_IRUSR | S_IRGRP | S_IROTH);
		if (fd < 0)
		{
			return boost::system::error_code(errno, boost::system::system_category());
		}
		return file_handle(fd);
	}
}

#endif
