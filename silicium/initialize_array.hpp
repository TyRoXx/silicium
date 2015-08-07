#ifndef SILICIUM_INITIALIZE_ARRAY_HPP
#define SILICIUM_INITIALIZE_ARRAY_HPP

#include <silicium/config.hpp>

#ifdef _MSC_VER
#	define SILICIUM_INITIALIZE_ARRAY(...) { __VA_ARGS__ }
#else
#	define SILICIUM_INITIALIZE_ARRAY(...) {{ __VA_ARGS__ }}
#endif

#endif
