#ifndef SILICIUM_OVERRIDE_HPP
#define SILICIUM_OVERRIDE_HPP

#if defined(__GNUC__) && (__GNUC__ <= 4) && (__GNUC__ < 4 || __GNUC_MINOR__ <= 6)
#	define SILICIUM_OVERRIDE
#else
#	define SILICIUM_OVERRIDE override
#endif

#endif
