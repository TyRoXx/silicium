#ifndef VENTURA_WIN32_OVERLAPPED_DIRECTORY_CHANGES_HPP
#define VENTURA_WIN32_OVERLAPPED_DIRECTORY_CHANGES_HPP

#include <algorithm>
#include <boost/asio.hpp>
#include <ventura/absolute_path.hpp>
#include <silicium/exchange.hpp>
#include <silicium/throw_last_error.hpp>
#include <silicium/error_or.hpp>
#include <silicium/observable/observer.hpp>
#include <ventura/win32/file_notification.hpp>

namespace ventura
{
	namespace win32
	{
		struct overlapped_directory_changes
		{
			typedef Si::error_or<std::vector<file_notification>> element_type;

			overlapped_directory_changes()
				: is_recursive(false)
				, io(nullptr)
				, receiver_(nullptr)
				, received(0)
			{
			}

			overlapped_directory_changes(overlapped_directory_changes &&other)
				: is_recursive(std::move(other.is_recursive))
				, io(other.io)
				, receiver_(other.receiver_)
				, received(0)
				, watch_file(std::move(other.watch_file))
			{
			}

			explicit overlapped_directory_changes(boost::asio::io_service &io, absolute_path const &watched, bool is_recursive)
				: is_recursive(is_recursive)
				, io(&io)
				, receiver_(nullptr)
				, received(0)
				, watch_file(CreateFileW(
					watched.c_str(),
					FILE_LIST_DIRECTORY,
					FILE_SHARE_WRITE | FILE_SHARE_READ | FILE_SHARE_DELETE,
					NULL,
					OPEN_EXISTING,
					FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
					NULL
				))
			{
				if (watch_file.get() == INVALID_HANDLE_VALUE)
				{
					Si::throw_last_error();
				}
				auto &service = boost::asio::use_service<boost::asio::detail::win_iocp_io_service>(io);
				boost::system::error_code ec;
				service.register_handle(watch_file.get(), ec);
				if (!!ec)
				{
					throw boost::system::system_error(ec);
				}
			}

			~overlapped_directory_changes()
			{
				if (watch_file.get() == nullptr ||
					watch_file.get() == INVALID_HANDLE_VALUE)
				{
					return;
				}
				if (!receiver_)
				{
					return;
				}
				if (!CancelIoEx(watch_file.get(), nullptr))
				{
					DWORD error = GetLastError();
					if (error != ERROR_NOT_FOUND)
					{
						std::terminate();
					}
				}
			}

			overlapped_directory_changes &operator = (overlapped_directory_changes &&other)
			{
				is_recursive = other.is_recursive;
				io = other.io;
				receiver_ = other.receiver_;
				watch_file = std::move(other.watch_file);
				return *this;
			}

			void async_get_one(Si::ptr_observer<Si::observer<element_type>> receiver)
			{
				assert(!receiver_);
				receiver_ = receiver.get();
				boost::asio::windows::overlapped_ptr overlapped(*io, [this](boost::system::error_code ec, std::size_t)
				{
					if (!!ec)
					{
						Si::exchange(this->receiver_, nullptr)->got_element(ec);
						return;
					}
					std::vector<file_notification> notifications;
					for (char *next_event = buffer.data();;)
					{
						FILE_NOTIFY_INFORMATION const &notification = reinterpret_cast<FILE_NOTIFY_INFORMATION const &>(*next_event);
						relative_path name(notification.FileName + 0, notification.FileName + (notification.FileNameLength / sizeof(WCHAR)));
						notifications.emplace_back(file_notification(notification.Action, std::move(name)));
						if (0 == notification.NextEntryOffset)
						{
							break;
						}
						next_event += notification.NextEntryOffset;
					}
					Si::exchange(this->receiver_, nullptr)->got_element(std::move(notifications));
				});
				DWORD const actions = FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_CREATION | FILE_NOTIFY_CHANGE_ATTRIBUTES;
				bool const ok = (0 != ReadDirectoryChangesW(watch_file.get(), buffer.data(), static_cast<DWORD>(buffer.size()), is_recursive, actions, &received, overlapped.get(), nullptr));
				if (!ok)
				{
					DWORD last_error = GetLastError();
					if (last_error != ERROR_IO_PENDING)
					{
						overlapped.complete(boost::system::error_code(last_error, boost::system::native_ecat), 0);
						return;
					}
				}
				overlapped.release();
			}

		private:

			bool is_recursive;
			boost::asio::io_service *io;
			Si::observer<element_type> *receiver_;
			DWORD received;
			std::array<char, 0x10000> buffer;
			Si::win32::unique_handle watch_file;

			SILICIUM_DISABLE_COPY(overlapped_directory_changes)
		};
	}
}

#endif
