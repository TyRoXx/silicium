#ifndef SILICIUM_FILE_HANDLE_HPP
#define SILICIUM_FILE_HANDLE_HPP

#ifdef _WIN32
#	include <silicium/win32/native_file_descriptor.hpp>
#else
#	include <silicium/posix/native_file_descriptor.hpp>
#endif

#include <silicium/exchange.hpp>
#include <silicium/config.hpp>

namespace Si
{
	struct file_handle
	{
		native_file_descriptor handle;

		file_handle() BOOST_NOEXCEPT
			: handle(no_file_handle)
		{
		}

		file_handle(file_handle &&other) BOOST_NOEXCEPT
			: handle(no_file_handle)
		{
			swap(other);
		}

		explicit file_handle(native_file_descriptor handle) BOOST_NOEXCEPT
			: handle(handle)
		{
		}

		file_handle &operator = (file_handle &&other) BOOST_NOEXCEPT
		{
			swap(other);
			return *this;
		}

		void swap(file_handle &other) BOOST_NOEXCEPT
		{
			using std::swap;
			swap(handle, other.handle);
		}

		void close() BOOST_NOEXCEPT
		{
			file_handle().swap(*this);
		}

		native_file_descriptor release() BOOST_NOEXCEPT
		{
			return Si::exchange(handle, no_file_handle);
		}

		~file_handle() BOOST_NOEXCEPT
		{
			if (handle != no_file_handle)
			{
				terminating_close(handle);
			}
		}

	private:

		SILICIUM_DELETED_FUNCTION(file_handle(file_handle const &))
		SILICIUM_DELETED_FUNCTION(file_handle &operator = (file_handle const &))
	};
}

#endif