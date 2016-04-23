#ifndef SILICIUM_STEADY_CLOCK_HPP
#define SILICIUM_STEADY_CLOCK_HPP

#include <silicium/config.hpp>
#include <boost/chrono/system_clocks.hpp>
#include <chrono>

namespace Si
{
#define SILICIUM_HAS_STEADY_CLOCK 1

#if defined(BOOST_CHRONO_HAS_CLOCK_STEADY)
    using boost::chrono::steady_clock;
    namespace chrono = boost::chrono;
#else
    using std::chrono::steady_clock;
    namespace chrono = std::chrono;
#endif

    typedef steady_clock steady_clock_if_available;
}

#endif
