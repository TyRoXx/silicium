#ifndef VENTURA_CMAKE_HPP
#define VENTURA_CMAKE_HPP

#include <ventura/absolute_path.hpp>

namespace ventura
{
	absolute_path const cmake_exe = *absolute_path::create(
#ifdef _WIN32
		L"C:\\Program Files (x86)\\CMake\\bin\\cmake.exe"
#else
		"/usr/bin/cmake"
#endif
		);
}

#endif
