#ifndef SILICIUM_REACTIVE_LINUX_INOTIFY_HPP
#define SILICIUM_REACTIVE_LINUX_INOTIFY_HPP

#include <silicium/observable.hpp>
#include <silicium/override.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/asio/posix/stream_descriptor.hpp>
#include <boost/swap.hpp>
#include <boost/optional.hpp>
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

		struct inotify_observable : observable<std::vector<file_notification>>, boost::noncopyable
		{
			typedef std::vector<file_notification> element_type;

			inotify_observable();
			explicit inotify_observable(boost::asio::io_service &io);
			watch_descriptor watch(boost::filesystem::path const &target, boost::uint32_t mask);
			watch_descriptor watch(boost::filesystem::path const &target, boost::uint32_t mask, boost::system::error_code &ec);
			virtual void async_get_one(observer<element_type> &receiver) SILICIUM_OVERRIDE;
			virtual void cancel() SILICIUM_OVERRIDE;

		private:

			boost::optional<boost::asio::posix::stream_descriptor> notifier;
			std::vector<char> read_buffer;
			observer<element_type> *receiver_ = nullptr;
		};
	}
}

#endif
