#ifndef SILICIUM_REACTIVE_FILE_SYSTEM_WATCHER_HPP
#define SILICIUM_REACTIVE_FILE_SYSTEM_WATCHER_HPP

#ifdef _WIN32
#	include <reactive/win32/file_system_watcher.hpp>
#endif

#ifdef __linux__
#	include <reactive/linux/file_system_watcher.hpp>
#endif

#endif
