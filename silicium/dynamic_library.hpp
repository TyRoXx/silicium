#ifndef SILICIUM_DYNAMIC_LIBRARY_HPP
#define SILICIUM_DYNAMIC_LIBRARY_HPP

#include <silicium/detail/basic_dynamic_library.hpp>

#ifdef __linux__
#	include <silicium/linux/dynamic_library_impl.hpp>
#endif

#ifdef _WIN32
#	include <silicium/win32/dynamic_library_impl.hpp>
#endif

namespace Si
{
	using dynamic_library = detail::basic_dynamic_library<
#ifdef __linux__
		linux::dynamic_library_impl
#else
		win32::dynamic_library_impl
#endif
	>;

	BOOST_STATIC_ASSERT(sizeof(dynamic_library) == sizeof(void *));
}

#endif
