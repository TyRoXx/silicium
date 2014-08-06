#ifndef SILICIUM_REACTIVE_LINUX_FILE_SYSTEM_WATCHER_HPP
#define SILICIUM_REACTIVE_LINUX_FILE_SYSTEM_WATCHER_HPP

#include <reactive/linux/inotify.hpp>
#include <reactive/file_notification.hpp>
#include <reactive/transform_if_initialized.hpp>
#include <reactive/enumerate.hpp>
#include <reactive/ref.hpp>
#include <boost/optional.hpp>

namespace rx
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

		boost::optional<rx::file_notification> to_portable_file_notification(linux::file_notification &&original)
		{
			auto const type = to_portable_file_notification_type(original.mask);
			if (!type)
			{
				return boost::none;
			}
			return rx::file_notification(*type, std::move(original.name));
		}
	}

	struct file_system_watcher : observable<file_notification>
	{
		typedef file_notification element_type;

		file_system_watcher()
		{
		}

		explicit file_system_watcher(boost::asio::io_service &io, boost::filesystem::path const &watched)
			: inotify(io)
			, impl(enumerate(ref(inotify)), linux::to_portable_file_notification)
			, root(inotify.watch(watched, IN_ALL_EVENTS))
		{
		}

		virtual void async_get_one(observer<element_type> &receiver) SILICIUM_OVERRIDE
		{
			impl.async_get_one(receiver);
		}

		virtual void cancel() SILICIUM_OVERRIDE
		{
			impl.cancel();
		}

	private:

		linux::inotify_observable inotify;
		//TODO: save the memory for the function pointer
		conditional_transformer<
			file_notification,
			enumerator<reference<std::vector<linux::file_notification>>>,
			boost::optional<file_notification>(*)(linux::file_notification &&)
		> impl;
		linux::watch_descriptor root;
	};
#endif
}

#endif
