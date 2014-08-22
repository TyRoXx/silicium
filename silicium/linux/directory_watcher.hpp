#ifndef SILICIUM_REACTIVE_LINUX_DIRECTORY_WATCHER_HPP
#define SILICIUM_REACTIVE_LINUX_DIRECTORY_WATCHER_HPP

#include <silicium/linux/inotify.hpp>
#include <silicium/file_notification.hpp>
#include <silicium/transform_if_initialized.hpp>
#include <silicium/enumerate.hpp>
#include <silicium/ref.hpp>
#include <boost/optional.hpp>

namespace Si
{
#ifdef __linux__
	namespace linux
	{
		boost::optional<file_notification_type> to_portable_file_notification_type(boost::uint32_t mask)
		{
			switch (mask)
			{
			case IN_CREATE: return file_notification_type::add;
			case IN_DELETE: return file_notification_type::remove;
			case IN_MODIFY: return file_notification_type::change;
			default:
				return boost::none;
			}
		}

		boost::optional<Si::file_notification> to_portable_file_notification(linux::file_notification &&original)
		{
			auto const type = to_portable_file_notification_type(original.mask);
			if (!type)
			{
				return boost::none;
			}
			return Si::file_notification(*type, std::move(original.name));
		}
	}

	struct directory_watcher
	{
		typedef file_notification element_type;

		directory_watcher()
		{
		}

		explicit directory_watcher(boost::asio::io_service &io, boost::filesystem::path const &watched)
			: inotify(io)
			, impl(enumerate(ref(inotify)), linux::to_portable_file_notification)
			, root(inotify.watch(watched, IN_ALL_EVENTS).value())
		{
		}

		void async_get_one(observer<element_type> &receiver)
		{
			impl.async_get_one(receiver);
		}

		void cancel()
		{
			impl.cancel();
		}

	private:

		linux::inotify_observable inotify;
		//TODO: save the memory for the function pointer
		conditional_transformer<
			file_notification,
			enumerator<ptr_observable<std::vector<linux::file_notification>, linux::inotify_observable *>>,
			boost::optional<file_notification>(*)(linux::file_notification &&)
		> impl;
		linux::watch_descriptor root;
	};
#endif
}

#endif
