#ifndef SILICIUM_PATH_CHAR_HPP
#define SILICIUM_PATH_CHAR_HPP

namespace Si
{
	typedef
#ifdef _WIN32
		wchar_t
#else
		char
#endif
		native_path_char;
}

#endif
