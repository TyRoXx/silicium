#ifndef VENTURA_WIN32_OPEN_HPP
#define VENTURA_WIN32_OPEN_HPP

#include <silicium/error_or.hpp>
#include <silicium/file_handle.hpp>
#include <silicium/c_string.hpp>
#include <silicium/get_last_error.hpp>

namespace ventura
{
	SILICIUM_USE_RESULT
	inline Si::error_or<Si::file_handle> open_reading(Si::native_path_string name)
	{
		Si::native_file_descriptor const fd = ::CreateFileW(name.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_ALWAYS, 0, NULL);
		if (fd == INVALID_HANDLE_VALUE)
		{
			return Si::get_last_error();
		}
		return Si::file_handle(fd);
	}

	SILICIUM_USE_RESULT
	inline Si::error_or<Si::file_handle> create_file(Si::native_path_string name)
	{
		Si::native_file_descriptor const fd = ::CreateFileW(name.c_str(), GENERIC_WRITE, FILE_SHARE_WRITE, nullptr, CREATE_NEW, 0, NULL);
		if (fd == INVALID_HANDLE_VALUE)
		{
			return Si::get_last_error();
		}
		return Si::file_handle(fd);
	}

	SILICIUM_USE_RESULT
	inline Si::error_or<Si::file_handle> overwrite_file(Si::native_path_string name)
	{
		Si::native_file_descriptor const fd = ::CreateFileW(name.c_str(), GENERIC_WRITE, FILE_SHARE_WRITE, nullptr, CREATE_ALWAYS, 0, NULL);
		if (fd == INVALID_HANDLE_VALUE)
		{
			return Si::get_last_error();
		}
		return Si::file_handle(fd);
	}

	SILICIUM_USE_RESULT
	inline Si::error_or<Si::file_handle> open_read_write(Si::native_path_string name)
	{
		Si::native_file_descriptor const fd = ::CreateFileW(name.c_str(), GENERIC_WRITE | GENERIC_READ, FILE_SHARE_WRITE, nullptr, CREATE_NEW, 0, NULL);
		if (fd == INVALID_HANDLE_VALUE)
		{
			return Si::get_last_error();
		}
		return Si::file_handle(fd);
	}
}

#endif
