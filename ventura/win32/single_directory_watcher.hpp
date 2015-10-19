#ifndef VENTURA_WIN32_SINGLE_DIRECTORY_WATCHER_HPP
#define VENTURA_WIN32_SINGLE_DIRECTORY_WATCHER_HPP

#include <ventura/win32/overlapped_directory_changes.hpp>
#include <ventura/file_notification.hpp>
#include <ventura/absolute_path.hpp>
#include <silicium/observable/error_or_enumerate.hpp>
#include <silicium/observable/ref.hpp>
#include <silicium/observable/transform_if_initialized.hpp>
#include <silicium/observable/function_observer.hpp>
#include <ventura/absolute_path.hpp>
#include <silicium/optional.hpp>
#include <boost/ref.hpp>
#include <boost/utility/in_place_factory.hpp>

#define VENTURA_HAS_SINGLE_DIRECTORY_WATCHER SILICIUM_HAS_ERROR_OR_ENUMERATE

namespace ventura
{
#if defined(_WIN32) && VENTURA_HAS_SINGLE_DIRECTORY_WATCHER
	namespace win32
	{
		inline Si::optional<file_notification_type> to_portable_file_notification_type(DWORD action)
		{
			switch (action)
			{
			case FILE_ACTION_RENAMED_NEW_NAME:
			case FILE_ACTION_ADDED:
				return file_notification_type::add;

			case FILE_ACTION_RENAMED_OLD_NAME:
			case FILE_ACTION_REMOVED:
				return file_notification_type::remove;

			case FILE_ACTION_MODIFIED:
				return file_notification_type::change_content_or_metadata;

			default:
				return Si::none; //TODO
			}
		}

		inline Si::optional<Si::error_or<ventura::file_notification>> to_portable_file_notification(Si::error_or<win32::file_notification> &&original)
		{
			if (original.is_error())
			{
				return Si::error_or<ventura::file_notification>(original.error());
			}
			auto const type = to_portable_file_notification_type(original.get().action);
			if (!type)
			{
				return Si::none;
			}
			return Si::error_or<ventura::file_notification>(ventura::file_notification(*type, std::move(original.get().name), true));
		}
	}
	
	struct single_directory_watcher
	{
		typedef Si::error_or<file_notification> element_type;

		single_directory_watcher()
		{
		}

		explicit single_directory_watcher(boost::asio::io_service &io, ventura::absolute_path const &watched)
			: impl(Si::error_or_enumerate(win32::overlapped_directory_changes(io, watched, false)), win32::to_portable_file_notification)
			, work(boost::in_place(boost::ref(io)))
		{
		}

		template <class Observer>
		void async_get_one(Observer &&observer)
		{
			impl.async_get_one(
				Si::function_observer<std::function<void(Si::optional<Si::error_or<file_notification>>)>>(
				[observer
#if SILICIUM_COMPILER_HAS_EXTENDED_CAPTURE
					= std::forward<Observer>(observer)
#endif
				](Si::optional<Si::error_or<file_notification>> element) mutable
				{
					if (element)
					{
						std::move(observer).got_element(std::move(*element));
					}
					else
					{
						std::move(observer).ended();
					}
				})
			);
		}

	private:

		//TODO: save the memory for the function pointer
		Si::conditional_transformer<
			Si::error_or<file_notification>,
			Si::error_or_enumerator<win32::overlapped_directory_changes>,
			Si::optional<Si::error_or<file_notification>>(*)(Si::error_or<win32::file_notification> &&),
			Si::function_observer<std::function<void(Si::optional<Si::error_or<file_notification>>)>>
		> impl;
		boost::optional<boost::asio::io_service::work> work;
	};
#endif
}

#endif
