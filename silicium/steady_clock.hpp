#ifndef SILICIUM_STEADY_CLOCK_HPP
#define SILICIUM_STEADY_CLOCK_HPP

#include <silicium/config.hpp>
#include <boost/chrono/system_clocks.hpp>
#include <chrono>

namespace Si
{
#if defined(BOOST_CHRONO_HAS_CLOCK_STEADY)
#define SILICIUM_HAS_STEADY_CLOCK 1
	using boost::chrono::steady_clock;
	namespace chrono = boost::chrono;
#elif !SILICIUM_GCC46
#define SILICIUM_HAS_STEADY_CLOCK 1
	using std::chrono::steady_clock;
	namespace chrono = std::chrono;
#else
#define SILICIUM_HAS_STEADY_CLOCK 0
	namespace chrono = boost::chrono;
#endif

#if SILICIUM_HAS_STEADY_CLOCK
	typedef steady_clock steady_clock_if_available;
#else
	typedef boost::chrono::system_clock steady_clock_if_available;
#endif
}

#endif
