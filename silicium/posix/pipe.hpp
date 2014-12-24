#ifndef SILICIUM_POSIX_PIPE_HPP
#define SILICIUM_POSIX_PIPE_HPP

#include <silicium/file_handle.hpp>

namespace Si
{
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

	inline pipe make_pipe()
	{
		std::array<int, 2> fds;
		if (::pipe(fds.data()) < 0)
		{
			throw boost::system::system_error(errno, boost::system::system_category());
		}
		pipe result;
		result.read  = file_handle(fds[0]);
		result.write = file_handle(fds[1]);
		return result;
	}
}

#endif