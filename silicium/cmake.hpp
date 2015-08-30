#ifndef SILICIUM_CMAKE_HPP
#define SILICIUM_CMAKE_HPP

#include <silicium/absolute_path.hpp>

namespace Si
{
	Si::absolute_path const cmake_exe = *Si::absolute_path::create(
#ifdef _WIN32
		L"C:\\Program Files (x86)\\CMake\\bin\\cmake.exe"
#else
		"/usr/bin/cmake"
#endif
		);
}

#endif
