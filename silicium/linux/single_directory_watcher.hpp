#ifndef SILICIUM_LINUX_SINGLE_DIRECTORY_WATCHER_HPP
#define SILICIUM_LINUX_SINGLE_DIRECTORY_WATCHER_HPP

#include <silicium/linux/inotify.hpp>
#include <silicium/file_notification.hpp>
#include <silicium/optional.hpp>
#include <silicium/observable/transform_if_initialized.hpp>
#include <silicium/observable/enumerate.hpp>
#include <silicium/observable/ref.hpp>
#include <silicium/observable/function_observer.hpp>
#include <boost/optional.hpp>

#define SILICIUM_HAS_SINGLE_DIRECTORY_WATCHER SILICIUM_HAS_INOTIFY_OBSERVABLE

namespace Si
{
#if defined(__linux__) && SILICIUM_HAS_SINGLE_DIRECTORY_WATCHER
	namespace linux
	{
		namespace detail
		{
			inline bool are_set(boost::uint32_t bitset, boost::uint32_t tested_bits)
			{
				return (bitset & tested_bits) == tested_bits;
			}
		}

		Si::optional<file_notification_type> to_portable_file_notification_type(boost::uint32_t mask)
		{
			using detail::are_set;
			if (are_set(mask, IN_MOVED_TO) || are_set(mask, IN_CREATE))
			{
				return file_notification_type::add;
			}
			if (are_set(mask, IN_MOVED_FROM) || are_set(mask, IN_DELETE))
			{
				return file_notification_type::remove;
			}
			if (are_set(mask, IN_CLOSE_WRITE) || are_set(mask, IN_MODIFY))
			{
				return file_notification_type::change_content;
			}
			if (are_set(mask, IN_MOVE_SELF))
			{
				return file_notification_type::move_self;
			}
			if (are_set(mask, IN_DELETE_SELF))
			{
				return file_notification_type::remove_self;
			}
			if (are_set(mask, IN_ATTRIB))
			{
				return file_notification_type::change_metadata;
			}
			return Si::none;
		}

		Si::optional<Si::file_notification> to_portable_file_notification(linux::file_notification &&original, Si::relative_path const &root)
		{
			auto const type = to_portable_file_notification_type(original.mask);
			if (!type)
			{
				return Si::none;
			}
			return Si::file_notification(*type, root / std::move(original.name), (original.mask & IN_ISDIR) == IN_ISDIR);
		}
	}

	struct single_directory_watcher
	{
		typedef file_notification element_type;

		single_directory_watcher()
		{
		}

		explicit single_directory_watcher(boost::asio::io_service &io, absolute_path const &watched)
			: inotify(io)
			, impl(enumerate(ref(inotify)), [](linux::file_notification &&notification) { return linux::to_portable_file_notification(std::move(notification), Si::relative_path()); })
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
		conditional_transformer<
			file_notification,
			enumerator<ptr_observable<std::vector<linux::file_notification>, linux::inotify_observable *>>,
			std::function<Si::optional<file_notification>(linux::file_notification &&)>,
			function_observer<std::function<void (optional<file_notification>)>>
		> impl;
		linux::watch_descriptor root;
	};
#endif
}

#endif
