#ifndef SILICIUM_OPEN_HPP
#define SILICIUM_OPEN_HPP

#ifdef _WIN32
#	include <silicium/win32/open.hpp>
#else
#	include <silicium/posix/open.hpp>
#endif

#endif
