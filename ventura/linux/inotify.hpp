#ifndef VENTURA_REACTIVE_LINUX_INOTIFY_HPP
#define VENTURA_REACTIVE_LINUX_INOTIFY_HPP

#include <silicium/observable/observer.hpp>
#include <silicium/error_or.hpp>
#include <silicium/exchange.hpp>
#include <silicium/throw_last_error.hpp>
#include <ventura/linux/inotify_watch_descriptor.hpp>
#include <ventura/absolute_path.hpp>
#include <ventura/path_segment.hpp>
#include <boost/asio/posix/stream_descriptor.hpp>
#include <boost/swap.hpp>
#include <boost/optional.hpp>
#include <boost/ref.hpp>
#include <dirent.h>

#define VENTURA_HAS_INOTIFY_OBSERVABLE SILICIUM_HAS_EXCEPTIONS

namespace ventura
{
	namespace linux
	{
#if VENTURA_HAS_INOTIFY_OBSERVABLE
		struct file_notification
		{
			boost::uint32_t mask;
			path_segment name;
			int watch_descriptor;

			file_notification() BOOST_NOEXCEPT
			    : mask(0)
			    , watch_descriptor(-1)
			{
			}

			explicit file_notification(boost::uint32_t mask, path_segment name, int watch_descriptor) BOOST_NOEXCEPT
			    : mask(mask)
			    , name(std::move(name))
			    , watch_descriptor(watch_descriptor)
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
					Si::throw_last_error();
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

			Si::error_or<watch_descriptor> watch(absolute_path const &target, boost::uint32_t mask) BOOST_NOEXCEPT
			{
				assert(notifier);
				int const wd = inotify_add_watch(notifier->native_handle(), target.c_str(), mask);
				if (wd < 0)
				{
					return Si::get_last_error();
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
					[this, SILICIUM_CAPTURE_EXPRESSION(receiver, std::forward<Observer>(receiver))](boost::system::error_code error, std::size_t bytes_read) mutable
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
							Si::optional<path_segment> segment = path_segment::create(boost::filesystem::path(event.name + 0, std::find(event.name + 0, event.name + event.len, '\0')));
							assert(segment);
							changes.emplace_back(file_notification{event.mask, std::move(*segment), event.wd});
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
#endif
	}
}

#endif
