#ifndef SILICIUM_POSIX_PIPE_HPP
#define SILICIUM_POSIX_PIPE_HPP

#include <silicium/file_handle.hpp>
#include <silicium/error_or.hpp>
#ifdef _WIN32
#	include <silicium/win32/win32.hpp>
#else
#	include <fcntl.h>
#endif

namespace Si
{
	namespace detail
	{
#ifndef _WIN32
		inline boost::system::error_code set_close_on_exec(native_file_descriptor file) BOOST_NOEXCEPT
		{
			if (fcntl(file, F_SETFD, fcntl(file, F_GETFD) | FD_CLOEXEC) < 0)
			{
				return boost::system::error_code(errno, boost::system::native_ecat);
			}
			return {};
		}
#endif
	}

	struct pipe
	{
		file_handle write, read;

		pipe() BOOST_NOEXCEPT
		{
		}

		void close() BOOST_NOEXCEPT
		{
			pipe().swap(*this);
		}

		void swap(pipe &other) BOOST_NOEXCEPT
		{
			write.swap(other.write);
			read.swap(other.read);
		}

#if !SILICIUM_COMPILER_GENERATES_MOVES
		pipe(pipe &&other) BOOST_NOEXCEPT
			: write(std::move(other.write))
			, read(std::move(other.read))
		{
		}

		pipe &operator = (pipe &&other) BOOST_NOEXCEPT
		{
			write = std::move(other.write);
			read = std::move(other.read);
			return *this;
		}

		SILICIUM_DELETED_FUNCTION(pipe(pipe const &))
		SILICIUM_DELETED_FUNCTION(pipe &operator = (pipe const &))
#endif
	};

	inline error_or<pipe> make_pipe() BOOST_NOEXCEPT
	{
#ifdef _WIN32
		SECURITY_ATTRIBUTES security{};
		security.nLength = sizeof(security);
		security.bInheritHandle = TRUE;
		HANDLE read, write;
		if (!CreatePipe(&read, &write, &security, 0))
		{
			return boost::system::error_code(::GetLastError(), boost::system::native_ecat);
		}
		pipe result;
		result.read = file_handle(read);
		result.write = file_handle(write);
		return std::move(result);
#else
		std::array<int, 2> fds;
		if (::pipe(fds.data()) < 0)
		{
			return boost::system::error_code(errno, boost::system::system_category());
		}
		pipe result;
		result.read = file_handle(fds[0]);
		result.write = file_handle(fds[1]);
		return std::move(result);
#endif
	}
}

#endif
