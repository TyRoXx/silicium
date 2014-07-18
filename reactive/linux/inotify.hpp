#ifndef SILICIUM_REACTIVE_LINUX_INOTIFY_HPP
#define SILICIUM_REACTIVE_LINUX_INOTIFY_HPP

#include <reactive/observable.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/asio/posix/stream_descriptor.hpp>
#include <sys/inotify.h>
#include <dirent.h>

namespace rx
{
	namespace linux
	{
		struct file_notification
		{
			boost::uint32_t mask;
			boost::filesystem::path name;
		};

		struct watch_descriptor
		{
			watch_descriptor() BOOST_NOEXCEPT
			{
			}

			watch_descriptor(int notifier, int watch) BOOST_NOEXCEPT
				: notifier(notifier)
				, watch(watch)
			{
			}

			watch_descriptor(watch_descriptor &&other) BOOST_NOEXCEPT
				: notifier(other.notifier)
				, watch(other.watch)
			{
				other.notifier = -1;
			}

			~watch_descriptor() BOOST_NOEXCEPT
			{
				if (notifier == -1)
				{
					return;
				}
				inotify_rm_watch(notifier, watch);
			}

			watch_descriptor &operator = (watch_descriptor &&other) BOOST_NOEXCEPT
			{
				boost::swap(notifier, other.notifier);
				boost::swap(watch, other.watch);
				return *this;
			}

		private:

			int notifier = -1;
			int watch = -1;
		};

		struct inotify_observable : observable<std::vector<file_notification>>
		{
			typedef std::vector<file_notification> element_type;

			explicit inotify_observable(boost::asio::io_service &io)
				: notifier(io)
			{
				int fd = inotify_init();
				if (fd < 0)
				{
					throw boost::system::system_error(errno, boost::system::posix_category);
				}
				try
				{
					notifier.assign(fd);
				}
				catch (...)
				{
					close(fd);
					throw;
				}
			}

			watch_descriptor watch(boost::filesystem::path const &target, boost::uint32_t mask)
			{
				int wd = inotify_add_watch(notifier.native_handle(), target.string().c_str(), mask);
				if (wd < 0)
				{
					throw boost::system::system_error(errno, boost::system::posix_category);
				}
				return watch_descriptor(notifier.native_handle(), wd);
			}

			virtual void async_get_one(observer<element_type> &receiver) SILICIUM_OVERRIDE
			{
				assert(!receiver_);
				std::size_t const min_buffer_size = sizeof(inotify_event) + NAME_MAX + 1;
				std::size_t const additional_buffer = 8192;
				read_buffer.resize(min_buffer_size + additional_buffer);
				notifier.async_read_some(boost::asio::buffer(read_buffer), [this](boost::system::error_code error, std::size_t bytes_read)
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
							changes.emplace_back(file_notification{event.mask, event.name});
							i += sizeof(inotify_event);
							i += event.len;
						}
						exchange(this->receiver_, nullptr)->got_element(std::move(changes));
					}
				});
				receiver_ = &receiver;
			}

			virtual void cancel() SILICIUM_OVERRIDE
			{
				assert(receiver_);
				notifier.cancel();
				receiver_ = nullptr;
			}

		private:

			boost::asio::posix::stream_descriptor notifier;
			std::vector<char> read_buffer;
			observer<element_type> *receiver_ = nullptr;
		};
	}
}

#endif
