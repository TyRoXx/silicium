#ifndef SILICIUM_GET_LAST_ERROR_HPP
#define SILICIUM_GET_LAST_ERROR_HPP

#include <silicium/config.hpp>
#include <boost/system/error_code.hpp>

#ifdef _WIN32
#include <silicium/win32/win32.hpp>
#else
#include <errno.h>
#endif

namespace Si
{
    SILICIUM_USE_RESULT
    inline boost::system::error_code get_last_error()
    {
        auto const err =
#ifdef _WIN32
            ::GetLastError();
#else
            errno;
#endif
        return boost::system::error_code(err, boost::system::system_category());
    }
}

#endif
