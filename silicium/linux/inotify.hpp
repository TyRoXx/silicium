#ifndef SILICIUM_REACTIVE_LINUX_INOTIFY_HPP
#define SILICIUM_REACTIVE_LINUX_INOTIFY_HPP

#include <silicium/observable/observer.hpp>
#include <silicium/error_or.hpp>
#include <silicium/exchange.hpp>
#include <silicium/linux/inotify_watch_descriptor.hpp>
#include <silicium/path.hpp>
#include <boost/asio/posix/stream_descriptor.hpp>
#include <boost/swap.hpp>
#include <boost/optional.hpp>
#include <boost/ref.hpp>
#include <dirent.h>

namespace Si
{
	namespace linux
	{
		struct file_notification
		{
			boost::uint32_t mask;
			path name;

			file_notification() BOOST_NOEXCEPT
			    : mask(0)
			{
			}

			explicit file_notification(boost::uint32_t mask, path name) BOOST_NOEXCEPT
			    : mask(mask)
			    , name(std::move(name))
			{
			}
		};

		struct inotify_observable
		{
			typedef std::vector<file_notification> element_type;

			inotify_observable() BOOST_NOEXCEPT
			{
			}

			explicit inotify_observable(boost::asio::io_service &io)
				: notifier(Si::make_unique<boost::asio::posix::stream_descriptor>(boost::ref(io)))
			{
				int fd = inotify_init();
				if (fd < 0)
				{
					throw boost::system::system_error(errno, boost::system::posix_category);
				}
				try
				{
					notifier->assign(fd);
				}
				catch (...)
				{
					close(fd);
					throw;
				}
			}

			error_or<watch_descriptor> watch(boost::filesystem::path const &target, boost::uint32_t mask) BOOST_NOEXCEPT
			{
				assert(notifier);
				int const wd = inotify_add_watch(notifier->native_handle(), target.c_str(), mask);
				if (wd < 0)
				{
					return boost::system::error_code(errno, boost::system::posix_category);
				}
				return watch_descriptor(notifier->native_handle(), wd);
			}

			template <class Observer>
			void async_get_one(Observer &&receiver)
			{
				std::size_t const min_buffer_size = sizeof(inotify_event) + NAME_MAX + 1;
				std::size_t const additional_buffer = 8192;
				read_buffer.resize(min_buffer_size + additional_buffer);
				assert(notifier);
				notifier->async_read_some(
					boost::asio::buffer(read_buffer),
					[this, receiver = std::forward<Observer>(receiver)](boost::system::error_code error, std::size_t bytes_read) mutable
				{
					if (error)
					{
						if (error == boost::asio::error::operation_aborted)
						{
							return;
						}
						throw std::logic_error("not implemented");
					}
					else
					{
						std::vector<file_notification> changes;
						for (std::size_t i = 0; i < bytes_read; )
						{
							inotify_event const &event = *reinterpret_cast<inotify_event const *>(read_buffer.data() + i);
							changes.emplace_back(file_notification{event.mask, path(event.name + 0, std::find(event.name + 0, event.name + event.len, '\0'))});
							i += sizeof(inotify_event);
							i += event.len;
						}
						std::forward<Observer>(receiver).got_element(std::move(changes));
					}
				});
			}

		private:

			std::unique_ptr<boost::asio::posix::stream_descriptor> notifier;
			std::vector<char> read_buffer;
		};
	}
}

#endif
