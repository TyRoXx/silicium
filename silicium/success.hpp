#ifndef SILICIUM_SUCCESS_HPP
#define SILICIUM_SUCCESS_HPP

#include <silicium/config.hpp>
#include <silicium/explicit_operator_bool.hpp>
#include <boost/config.hpp>

namespace Si
{
    struct success
    {
        SILICIUM_USE_RESULT
        bool operator!() const BOOST_NOEXCEPT
        {
            return true;
        }

        SILICIUM_EXPLICIT_OPERATOR_BOOL()
    };
}

#endif
