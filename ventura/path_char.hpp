#ifndef VENTURA_PATH_CHAR_HPP
#define VENTURA_PATH_CHAR_HPP

#include <silicium/is_handle.hpp>

namespace ventura
{
	typedef
#ifdef _WIN32
		wchar_t
#else
		char
#endif
		native_path_char;

	BOOST_STATIC_ASSERT(Si::is_handle<native_path_char>::value);
}

#endif
