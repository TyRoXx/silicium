#ifndef SILICIUM_SILICIUM_CONFIG_HPP
#define SILICIUM_SILICIUM_CONFIG_HPP

#ifdef _MSC_VER
#	define SILICIUM_UNREACHABLE() __assume(false)
#else
#	define SILICIUM_UNREACHABLE() __builtin_unreachable()
#endif

#endif
