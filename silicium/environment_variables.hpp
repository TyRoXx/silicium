#ifndef SILICIUM_DYNAMIC_LIBRARY_HPP
#define SILICIUM_DYNAMIC_LIBRARY_HPP

#include <silicium/c_string.hpp>
#include <silicium/win32/win32.hpp>

namespace Si
{
	SILICIUM_USE_RESULT
	boost::system::error_code set_environment_variable(os_c_string key, os_c_string value)
	{
#ifdef _WIN32
		if (!SetEnvironmentVariableW(key.c_str(), value.c_str()))
		{
			return boost::system::error_code(GetLastError(), boost::system::system_category());
		}
#else
		if (setenv(key.c_str(), value.c_str(), 1) != 0)
		{
			int err = errno;
			return boost::system::error_code(err, boost::system::system_category());
		}
#endif
		return{};
	}
}

#endif
