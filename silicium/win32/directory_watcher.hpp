#ifndef SILICIUM_REACTIVE_WIN32_DIRECTORY_WATCHER_HPP
#define SILICIUM_REACTIVE_WIN32_DIRECTORY_WATCHER_HPP

#include <silicium/win32/directory_changes.hpp>
#include <silicium/file_notification.hpp>
#include <silicium/observable/enumerate.hpp>
#include <silicium/observable/ref.hpp>
#include <silicium/observable/transform_if_initialized.hpp>
#include <boost/optional.hpp>
#include <boost/ref.hpp>

namespace Si
{
#ifdef _WIN32
	namespace win32
	{
		inline boost::optional<file_notification_type> to_portable_file_notification_type(DWORD action)
		{
			switch (action)
			{
			case FILE_ACTION_ADDED: return file_notification_type::add;
			case FILE_ACTION_REMOVED: return file_notification_type::remove;
			case FILE_ACTION_MODIFIED: return file_notification_type::change;
			default:
				return boost::none; //TODO
			}
		}

		inline boost::optional<Si::file_notification> to_portable_file_notification(win32::file_notification &&original)
		{
			auto const type = to_portable_file_notification_type(original.action);
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
			: impl(enumerate(win32::directory_changes(watched, false)), win32::to_portable_file_notification)
			, work(boost::in_place(boost::ref(io)))
		{
		}

		void async_get_one(observer<element_type> &receiver)
		{
			return impl.async_get_one(receiver);
		}

	private:

		//TODO: save the memory for the function pointer
		conditional_transformer<
			file_notification,
			enumerator<win32::directory_changes>,
			boost::optional<file_notification>(*)(win32::file_notification &&)
		> impl;
		boost::optional<boost::asio::io_service::work> work;
	};
#endif
}

#endif
