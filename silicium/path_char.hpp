#ifndef SILICIUM_PATH_CHAR_HPP
#define SILICIUM_PATH_CHAR_HPP

#include <silicium/is_handle.hpp>

namespace Si
{
	typedef
#ifdef _WIN32
		wchar_t
#else
		char
#endif
		native_path_char;

	BOOST_STATIC_ASSERT(is_handle<native_path_char>::value);
}

#endif
