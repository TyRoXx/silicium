#ifndef SILICIUM_LINUX_FILE_DESCRIPTOR_HPP
#define SILICIUM_LINUX_FILE_DESCRIPTOR_HPP

#include <silicium/exchange.hpp>
#include <boost/config.hpp>
#include <boost/system/system_error.hpp>
#include <boost/throw_exception.hpp>
#include <unistd.h>

namespace Si
{
	namespace linux
	{
		using native_file_handle = int;

		native_file_handle const no_file_handle = -1;

		inline void terminating_close(native_file_handle file) BOOST_NOEXCEPT
		{
			if (close(file) < 0)
			{
				//it is intended that this will terminate the process because of noexcept
				boost::throw_exception(boost::system::system_error(errno, boost::system::system_category()));
			}
		}

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
				if (handle >= 0)
				{
					terminating_close(handle);
				}
			}
		};
	}
}

#endif
