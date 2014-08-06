#ifndef SILICIUM_REACTIVE_WIN32_FILE_SYSTEM_WATCHER_HPP
#define SILICIUM_REACTIVE_WIN32_FILE_SYSTEM_WATCHER_HPP

#include <reactive/win32/directory_changes.hpp>
#include <reactive/file_notification.hpp>
#include <boost/optional.hpp>

namespace rx
{
#ifdef _WIN32
	namespace win32
	{
		boost::optional<file_notification_type> to_portable_file_notification_type(DWORD action)
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

		boost::optional<rx::file_notification> to_portable_file_notification(win32::file_notification &&original)
		{
			auto const type = to_portable_file_notification_type(original.action);
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
			: impl(enumerate(win32::directory_changes(watched, true)), win32::to_portable_file_notification)
			, work(io)
		{
		}

		virtual void async_get_one(observer<element_type> &receiver) SILICIUM_OVERRIDE
		{
			return impl.async_get_one(receiver);
		}

		virtual void cancel() SILICIUM_OVERRIDE
		{
			return impl.cancel();
		}

	private:

		//TODO: save the memory for the function pointer
		conditional_transformer<
			file_notification,
			enumerator<win32::directory_changes>,
			boost::optional<file_notification>(*)(win32::file_notification &&)
		> impl;
		boost::asio::io_service::work work;
	};
#endif
}

#endif
