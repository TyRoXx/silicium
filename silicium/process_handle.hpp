#ifndef SILICIUM_PROCESS_HANDLE_HPP
#define SILICIUM_PROCESS_HANDLE_HPP

#ifdef _WIN32
#include <silicium/win32/process_handle.hpp>
#else
#include <silicium/linux/process_handle.hpp>
#endif

namespace Si
{
#if SILICIUM_HAS_IS_HANDLE
	BOOST_STATIC_ASSERT(is_handle<process_handle>::value);
#endif
}

#endif
