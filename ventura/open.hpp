#ifndef VENTURA_OPEN_HPP
#define VENTURA_OPEN_HPP

#ifdef _WIN32
#	include <ventura/win32/open.hpp>
#else
#	include <ventura/posix/open.hpp>
#endif

#endif
