#ifndef SILICIUM_DYNAMIC_LIBRARY_HPP
#define SILICIUM_DYNAMIC_LIBRARY_HPP

#include <silicium/c_string.hpp>
#include <silicium/get_last_error.hpp>
#ifdef _WIN32
#	include <silicium/win32/win32.hpp>
#endif

namespace Si
{
	SILICIUM_USE_RESULT
	boost::system::error_code set_environment_variable(os_c_string key, os_c_string value)
	{
#ifdef _WIN32
		if (!SetEnvironmentVariableW(key.c_str(), value.c_str()))
		{
			return get_last_error();
		}
#else
		if (setenv(key.c_str(), value.c_str(), 1) != 0)
		{
			return get_last_error();
		}
#endif
		return{};
	}
}

#endif
