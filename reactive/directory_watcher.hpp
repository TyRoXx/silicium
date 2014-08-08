#ifndef SILICIUM_REACTIVE_DIRECTORY_WATCHER_HPP
#define SILICIUM_REACTIVE_DIRECTORY_WATCHER_HPP

#ifdef _WIN32
#	include <reactive/win32/directory_watcher.hpp>
#endif

#ifdef __linux__
#	include <reactive/linux/directory_watcher.hpp>
#endif

#endif
