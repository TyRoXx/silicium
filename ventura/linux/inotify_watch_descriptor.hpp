#ifndef VENTURA_LINUX_INOTIFY_WATCH_DESCRIPTOR_HPP
#define VENTURA_LINUX_INOTIFY_WATCH_DESCRIPTOR_HPP

#include <silicium/config.hpp>
#include <boost/swap.hpp>
#include <sys/inotify.h>

namespace ventura
{
	namespace linux
	{
		struct watch_descriptor
		{
			watch_descriptor() BOOST_NOEXCEPT
				: notifier(-1)
				, watch(-1)
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

			void release() BOOST_NOEXCEPT
			{
				notifier = -1;
				watch = -1;
			}

			int get_watch_descriptor() const BOOST_NOEXCEPT
			{
				return watch;
			}

		private:

			int notifier;
			int watch;
		};
	}
}

#endif
