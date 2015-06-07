#ifndef SILICIUM_WIN32_OPEN_HPP
#define SILICIUM_WIN32_OPEN_HPP

#include <silicium/error_or.hpp>
#include <silicium/file_handle.hpp>
#include <silicium/c_string.hpp>

namespace Si
{
	SILICIUM_USE_RESULT
	inline error_or<file_handle> open_reading(native_path_string name)
	{
		native_file_descriptor const fd = ::CreateFileW(name.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_ALWAYS, 0, NULL);
		if (fd == INVALID_HANDLE_VALUE)
		{
			return boost::system::error_code(GetLastError(), boost::system::system_category());
		}
		return file_handle(fd);
	}

	SILICIUM_USE_RESULT
	inline error_or<file_handle> create_file(native_path_string name)
	{
		native_file_descriptor const fd = ::CreateFileW(name.c_str(), GENERIC_WRITE, FILE_SHARE_WRITE, nullptr, CREATE_NEW, 0, NULL);
		if (fd == INVALID_HANDLE_VALUE)
		{
			return boost::system::error_code(GetLastError(), boost::system::system_category());
		}
		return file_handle(fd);
	}

	SILICIUM_USE_RESULT
	inline error_or<file_handle> overwrite_file(native_path_string name)
	{
		native_file_descriptor const fd = ::CreateFileW(name.c_str(), GENERIC_WRITE, FILE_SHARE_WRITE, nullptr, CREATE_ALWAYS, 0, NULL);
		if (fd == INVALID_HANDLE_VALUE)
		{
			return boost::system::error_code(GetLastError(), boost::system::system_category());
		}
		return file_handle(fd);
	}

	SILICIUM_USE_RESULT
	inline error_or<file_handle> open_read_write(native_path_string name)
	{
		native_file_descriptor const fd = ::CreateFileW(name.c_str(), GENERIC_WRITE | GENERIC_READ, FILE_SHARE_WRITE, nullptr, CREATE_NEW, 0, NULL);
		if (fd == INVALID_HANDLE_VALUE)
		{
			return boost::system::error_code(GetLastError(), boost::system::system_category());
		}
		return file_handle(fd);
	}
}

#endif
