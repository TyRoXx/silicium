#ifndef SILICIUM_OPEN_HPP
#define SILICIUM_OPEN_HPP

#ifdef __linux__
#	include <silicium/linux/open.hpp>
#endif

#ifdef _WIN32
#	include <silicium/win32/open.hpp>
#endif

#endif
