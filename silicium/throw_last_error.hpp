#ifndef SILICIUM_THROW_LAST_ERROR_HPP
#define SILICIUM_THROW_LAST_ERROR_HPP

#include <silicium/get_last_error.hpp>
#ifdef _WIN32
#	include <silicium/win32/win32.hpp>
#endif
#include <boost/system/system_error.hpp>
#include <boost/throw_exception.hpp>

namespace Si
{
	inline void throw_last_error()
	{
		boost::throw_exception(boost::system::system_error(get_last_error()));
	}
}

#endif
