#ifndef SILICIUM_PROCESS_HANDLE_HPP
#define SILICIUM_PROCESS_HANDLE_HPP

#ifdef _WIN32
#include <silicium/win32/process_handle.hpp>
#else
#include <silicium/linux/process_handle.hpp>
#endif

#endif
