#ifndef SILICIUM_DYNAMIC_LIBRARY_HPP
#define SILICIUM_DYNAMIC_LIBRARY_HPP

#include <silicium/detail/basic_dynamic_library.hpp>

#ifdef _WIN32
#	include <silicium/win32/dynamic_library_impl.hpp>
#else
#	include <silicium/linux/dynamic_library_impl.hpp>
#endif

#include <boost/static_assert.hpp>

namespace Si
{
	typedef detail::basic_dynamic_library<
#ifdef _WIN32
		win32::dynamic_library_impl
#else
		linux::dynamic_library_impl
#endif
	> dynamic_library;

	BOOST_STATIC_ASSERT(sizeof(dynamic_library) == sizeof(void *));
}

#endif
