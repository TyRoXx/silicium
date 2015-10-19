#ifndef VENTURA_FILE_SIZE_HPP
#define VENTURA_FILE_SIZE_HPP

#include <silicium/file_handle.hpp>
#include <silicium/error_or.hpp>
#include <silicium/get_last_error.hpp>

#ifndef _WIN32
#	include <sys/stat.h>
#endif

namespace ventura
{
	/// Returns the size of the regular file given by the file descriptor or none if the
	/// file is not regular.
	inline Si::error_or<Si::optional<boost::uint64_t>> file_size(Si::native_file_descriptor file)
	{
#ifdef _WIN32
		LARGE_INTEGER size;
		if (!GetFileSizeEx(file, &size))
		{
			return Si::get_last_error();
		}
		assert(size.QuadPart >= 0);
		return static_cast<boost::uint64_t>(size.QuadPart);
#else
		struct stat buffer;
		if (fstat(file, &buffer) < 0)
		{
			return Si::get_last_error();
		}
		if ((buffer.st_mode & S_IFMT) != S_IFREG)
		{
			return Si::none;
		}
		return static_cast<boost::uint64_t>(buffer.st_size);
#endif
	}
}

#endif
