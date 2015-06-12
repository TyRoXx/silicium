#ifndef SILICIUM_THROW_LAST_ERROR_HPP
#define SILICIUM_THROW_LAST_ERROR_HPP

#include <boost/system/system_error.hpp>
#include <silicium/win32/win32.hpp>

namespace Si
{
	inline void throw_last_error()
	{
#ifdef _WIN32
		throw boost::system::system_error(boost::system::error_code(GetLastError(), boost::system::system_category()));
#else
		throw boost::system::system_error(boost::system::error_code(errno, boost::system::system_category()));
#endif
	}
}

#endif
