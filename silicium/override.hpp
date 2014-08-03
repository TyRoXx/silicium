#ifndef SILICIUM_OVERRIDE_HPP
#define SILICIUM_OVERRIDE_HPP

#if defined(__GNUC__) && (__GNUC__ <= 4) && (__GNUC__ < 4 || __GNUC_MINOR__ <= 6)
#	define SILICIUM_OVERRIDE
#	define SILICIUM_FINAL
#else
#	define SILICIUM_OVERRIDE override
#	define SILICIUM_FINAL final
#endif

#define SILICIUM_UNREACHABLE() throw std::logic_error("Marked unreachable, " __FILE__ ":" BOOST_STRINGIZE(__LINE__))

#endif
