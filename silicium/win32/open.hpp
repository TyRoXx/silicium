#ifndef SILICIUM_WIN32_OPEN_HPP
#define SILICIUM_WIN32_OPEN_HPP

#include <silicium/error_or.hpp>
#include <silicium/file_descriptor.hpp>
#include <boost/filesystem/path.hpp>

namespace Si
{
	inline error_or<file_descriptor> open_reading(boost::filesystem::path const &name)
	{
		native_file_handle const fd = ::CreateFileW(name.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_ALWAYS, 0, NULL);
		if (fd == INVALID_HANDLE_VALUE)
		{
			return boost::system::error_code(GetLastError(), boost::system::system_category());
		}
		return file_descriptor(fd);
	}

	inline error_or<file_descriptor> create_file(boost::filesystem::path const &name)
	{
		native_file_handle const fd = ::CreateFileW(name.c_str(), GENERIC_WRITE, FILE_SHARE_WRITE, nullptr, CREATE_NEW, 0, NULL);
		if (fd == INVALID_HANDLE_VALUE)
		{
			return boost::system::error_code(GetLastError(), boost::system::system_category());
		}
		return file_descriptor(fd);
	}
}

#endif
