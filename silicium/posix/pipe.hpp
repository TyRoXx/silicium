#ifndef SILICIUM_POSIX_PIPE_HPP
#define SILICIUM_POSIX_PIPE_HPP

#include <silicium/file_handle.hpp>
#include <silicium/error_or.hpp>
#include <fcntl.h>

namespace Si
{
	namespace detail
	{
		inline boost::system::error_code set_close_on_exec(native_file_descriptor file)
		{
			if (fcntl(file, F_SETFD, fcntl(file, F_GETFD) | FD_CLOEXEC) < 0)
			{
				return boost::system::error_code(errno, boost::system::native_ecat);
			}
			return {};
		}
	}

	struct pipe
	{
		file_handle write, read;

		void close() BOOST_NOEXCEPT
		{
			pipe().swap(*this);
		}

		void swap(pipe &other) BOOST_NOEXCEPT
		{
			write.swap(other.write);
			read.swap(other.read);
		}
	};

	inline error_or<pipe> make_pipe()
	{
		std::array<int, 2> fds;
		if (::pipe(fds.data()) < 0)
		{
			return boost::system::error_code(errno, boost::system::system_category());
		}
		pipe result;
		result.read  = file_handle(fds[0]);
		result.write = file_handle(fds[1]);
		return std::move(result);
	}
}

#endif
