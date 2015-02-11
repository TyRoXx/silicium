#ifndef SILICIUM_REACTIVE_LINUX_DIRECTORY_WATCHER_HPP
#define SILICIUM_REACTIVE_LINUX_DIRECTORY_WATCHER_HPP

#include <silicium/linux/inotify.hpp>
#include <silicium/file_notification.hpp>
#include <silicium/observable/transform_if_initialized.hpp>
#include <silicium/observable/enumerate.hpp>
#include <silicium/observable/ref.hpp>
#include <silicium/observable/function_observer.hpp>
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
			case IN_MOVED_TO:
			case IN_CREATE:
				return file_notification_type::add;

			case IN_MOVED_FROM:
			case IN_DELETE:
				return file_notification_type::remove;

			case IN_CLOSE_WRITE:
			case IN_MODIFY:
				return file_notification_type::change;

			case IN_MOVE_SELF:
				return file_notification_type::move_self;

			case IN_DELETE_SELF:
				return file_notification_type::remove_self;

			case IN_ATTRIB:
				return file_notification_type::change_metadata;

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
			, root(get(inotify.watch(watched, (IN_MODIFY | IN_CLOSE_WRITE | IN_MOVED_FROM | IN_MOVED_TO | IN_CREATE | IN_DELETE | IN_DELETE_SELF | IN_MOVE_SELF | IN_ATTRIB))))
		{
		}

		template <class Observer>
		void async_get_one(Observer &&receiver)
		{
			impl.async_get_one(
				function_observer<std::function<void (optional<file_notification>)>>(
					[observer = std::forward<Observer>(receiver)](optional<file_notification> element) mutable
					{
						if (element)
						{
							std::move(observer).got_element(std::move(*element));
						}
						else
						{
							std::move(observer).ended();
						}
					}
				)
			);
		}

	private:

		linux::inotify_observable inotify;
		//TODO: save the memory for the function pointer
		conditional_transformer<
			file_notification,
			enumerator<ptr_observable<std::vector<linux::file_notification>, linux::inotify_observable *>>,
			boost::optional<file_notification>(*)(linux::file_notification &&),
			function_observer<std::function<void (optional<file_notification>)>>
		> impl;
		linux::watch_descriptor root;
	};
#endif
}

#endif
