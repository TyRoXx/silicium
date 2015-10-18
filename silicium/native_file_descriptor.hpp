#ifndef SILICIUM_NATIVE_FILE_DESCRIPTOR_HPP
#define SILICIUM_NATIVE_FILE_DESCRIPTOR_HPP

#ifdef _WIN32
#	include <silicium/win32/native_file_descriptor.hpp>
#else
#	include <silicium/posix/native_file_descriptor.hpp>
#endif

#endif
