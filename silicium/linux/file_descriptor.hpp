#ifndef SILICIUM_LINUX_FILE_DESCRIPTOR_HPP
#define SILICIUM_LINUX_FILE_DESCRIPTOR_HPP

#include <boost/config.hpp>
#include <boost/filesystem/path.hpp>
#include <unistd.h>
#include <fcntl.h>

namespace Si
{
	namespace linux
	{
		inline void terminating_close(int file) BOOST_NOEXCEPT
		{
			if (close(file) < 0)
			{
				//it is intended that this will terminate the process because of noexcept
				throw boost::system::system_error(errno, boost::system::system_category());
			}
		}

		struct file_descriptor : private boost::noncopyable
		{
			int handle;

			file_descriptor() BOOST_NOEXCEPT
				: handle(-1)
			{
			}

			file_descriptor(file_descriptor &&other) BOOST_NOEXCEPT
				: handle(-1)
			{
				swap(other);
			}

			explicit file_descriptor(int handle) BOOST_NOEXCEPT
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

			~file_descriptor() BOOST_NOEXCEPT
			{
				if (handle >= 0)
				{
					terminating_close(handle);
				}
			}
		};

		inline Si::linux::file_descriptor open_reading(boost::filesystem::path const &name)
		{
			int const fd = ::open(name.c_str(), O_RDONLY);
			if (fd < 0)
			{
				boost::throw_exception(boost::system::system_error(boost::system::error_code(errno, boost::system::system_category())));
			}
			return Si::linux::file_descriptor(fd);
		}
	}
}

#endif
