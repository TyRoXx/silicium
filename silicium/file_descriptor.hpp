#ifndef SILICIUM_FILE_DESCRIPTOR_HPP
#define SILICIUM_FILE_DESCRIPTOR_HPP

#ifdef __linux__
#	include <silicium/linux/file_descriptor.hpp>
#endif

#ifdef _WIN32
#	include <silicium/win32/file_descriptor.hpp>
#endif

#include <silicium/exchange.hpp>

namespace Si
{
	struct file_descriptor : private boost::noncopyable
	{
		native_file_handle handle = no_file_handle;

		file_descriptor() BOOST_NOEXCEPT
		{
		}

		file_descriptor(file_descriptor &&other) BOOST_NOEXCEPT
		{
			swap(other);
		}

		explicit file_descriptor(native_file_handle handle) BOOST_NOEXCEPT
			: handle(handle)
		{
		}

		file_descriptor &operator = (file_descriptor &&other) BOOST_NOEXCEPT
		{
			swap(other);
			return *this;
		}

		void swap(file_descriptor &other) BOOST_NOEXCEPT
		{
			using std::swap;
			swap(handle, other.handle);
		}

		void close() BOOST_NOEXCEPT
		{
			file_descriptor().swap(*this);
		}

		native_file_handle release() BOOST_NOEXCEPT
		{
			return Si::exchange(handle, no_file_handle);
		}

		~file_descriptor() BOOST_NOEXCEPT
		{
			if (handle != no_file_handle)
			{
				terminating_close(handle);
			}
		}
	};
}

#endif
