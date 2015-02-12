#ifndef SILICIUM_OBSERVABLE_SINGLE_DIRECTORY_WATCHER_HPP
#define SILICIUM_OBSERVABLE_SINGLE_DIRECTORY_WATCHER_HPP

#ifdef _WIN32
#	include <silicium/win32/single_directory_watcher.hpp>
#endif

#ifdef __linux__
#	include <silicium/linux/single_directory_watcher.hpp>
#endif

#endif
