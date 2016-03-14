#ifndef SILICIUM_NATIVE_FILE_DESCRIPTOR_HPP
#define SILICIUM_NATIVE_FILE_DESCRIPTOR_HPP

#ifdef _WIN32
#include <silicium/win32/win32.hpp>
#endif

namespace Si
{
#ifdef _WIN32
	typedef HANDLE native_file_descriptor;

	native_file_descriptor const no_file_handle = INVALID_HANDLE_VALUE;
#else
	typedef int native_file_descriptor;

	native_file_descriptor const no_file_handle = -1;
#endif
}

#endif
