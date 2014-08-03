#include <reactive/win32/directory_changes.hpp>
#include <reactive/exchange.hpp>
#include <Windows.h>
#include <array>

namespace rx
{
	namespace win32
	{
		directory_changes::directory_changes(boost::filesystem::path const &watched)
			: watch_file(CreateFileW(
				watched.c_str(),
				FILE_LIST_DIRECTORY,
				FILE_SHARE_WRITE | FILE_SHARE_READ | FILE_SHARE_DELETE,
				NULL,
				OPEN_EXISTING,
				FILE_FLAG_BACKUP_SEMANTICS,
				NULL
			))
			, read_active(read_dispatcher)
		{
			if (watch_file.get() == INVALID_HANDLE_VALUE)
			{
				throw boost::system::system_error(::GetLastError(), boost::system::native_ecat);
			}

			//start running after the creation of work
			read_runner = std::async(std::launch::async, [this]{ this->read_dispatcher.run(); });
		}

		void directory_changes::async_get_one(observer<element_type> &receiver)
		{
			assert(!receiver_);
			receiver_ = &receiver;
			read_dispatcher.post([this]
			{
				DWORD received = 0;
				std::array<char, 0x10000> buffer;
				DWORD const actions = FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_DIR_NAME;
				if (!ReadDirectoryChangesW(watch_file.get(), buffer.data(), buffer.size(), TRUE, actions, &received, nullptr, nullptr))
				{
					//TODO: handle ERROR_NOTIFY_ENUM_DIR (overflow of event queue)
					throw std::logic_error("to do: error handling");
				}
				std::vector<file_notification> notifications;
				for (char *next_event = buffer.data(); ;)
				{
					FILE_NOTIFY_INFORMATION const &notification = reinterpret_cast<FILE_NOTIFY_INFORMATION const &>(*next_event);
					boost::filesystem::path name(notification.FileName, notification.FileName + (notification.FileNameLength / sizeof(WCHAR)));
					notifications.emplace_back(file_notification(notification.Action, std::move(name)));
					if (0 == notification.NextEntryOffset)
					{
						break;
					}
					next_event += notification.NextEntryOffset;
				}
				rx::exchange(this->receiver_, nullptr)->got_element(std::move(notifications));
			});
		}

		void directory_changes::cancel()
		{
			throw std::logic_error("to do");
		}
	}
}
