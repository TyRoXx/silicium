#ifndef SILICIUM_OBSERVABLE_SINGLE_DIRECTORY_WATCHER_HPP
#define SILICIUM_OBSERVABLE_SINGLE_DIRECTORY_WATCHER_HPP

#ifdef _WIN32
#	include <silicium/win32/single_directory_watcher.hpp>
#elif defined(__linux__)
#	include <silicium/linux/single_directory_watcher.hpp>
#else
#	define SILICIUM_HAS_SINGLE_DIRECTORY_WATCHER 0
#endif

#endif
