#ifndef VENTURA_REACTIVE_DIRECTORY_CHANGES_HPP
#define VENTURA_REACTIVE_DIRECTORY_CHANGES_HPP

#include <algorithm>
#include <boost/asio/io_service.hpp>
#include <silicium/observable/observer.hpp>
#include <ventura/win32/file_notification.hpp>
#include <silicium/config.hpp>
#include <ventura/path.hpp>
#include <silicium/error_or.hpp>
#include <silicium/exchange.hpp>
#include <boost/ref.hpp>
#include <boost/utility/in_place_factory.hpp>

#if SILICIUM_HAS_EXCEPTIONS
#include <future>

namespace ventura
{
	namespace win32
	{
		struct directory_changes
		{
			using element_type = std::vector<file_notification>;

			directory_changes();
			directory_changes(directory_changes &&other);
			explicit directory_changes(boost::filesystem::path const &watched, bool is_recursive);
			~directory_changes();
			directory_changes &operator = (directory_changes &&other);
			void async_get_one(ptr_observer<observer<element_type>> receiver);

		private:

			bool is_recursive;
			observer<element_type> *receiver_ = nullptr;

			struct immovable_state : private boost::noncopyable
			{
				boost::asio::io_service read_dispatcher;
				std::future<void> read_runner;
				optional<boost::asio::io_service::work> read_active;

				immovable_state()
					: read_active(some, read_dispatcher)
				{
				}

				~immovable_state()
				{
					read_active = none;
					read_dispatcher.stop();
					read_runner.get();
				}
			};

			//this indirection is needed for movability
			std::unique_ptr<immovable_state> immovable;

			//The watch file comes after the threading state so that it will be destroyed first to unblock a possible read operation
			//in the reading thread.
			Si::win32::unique_handle watch_file;

			SILICIUM_DISABLE_COPY(directory_changes)
		};

		inline directory_changes::directory_changes()
		{
		}

		inline directory_changes::directory_changes(directory_changes &&other)
			: is_recursive(std::move(other.is_recursive))
			, receiver_(other.receiver_)
			, watch_file(std::move(other.watch_file))
			, immovable(std::move(other.immovable))
		{
		}

		inline directory_changes::directory_changes(boost::filesystem::path const &watched, bool is_recursive)
			: is_recursive(is_recursive)
			, watch_file(CreateFileW(
				watched.c_str(),
				FILE_LIST_DIRECTORY,
				FILE_SHARE_WRITE | FILE_SHARE_READ | FILE_SHARE_DELETE,
				NULL,
				OPEN_EXISTING,
				FILE_FLAG_BACKUP_SEMANTICS,
				NULL
			))
			, immovable(make_unique<immovable_state>())
		{
			if (watch_file.get() == INVALID_HANDLE_VALUE)
			{
				throw boost::system::system_error(::GetLastError(), boost::system::native_ecat);
			}

			//start running after the creation of work
			auto &dispatcher = immovable->read_dispatcher;
			immovable->read_runner = std::async(std::launch::async, [&dispatcher]{ dispatcher.run(); });
		}

		inline directory_changes::~directory_changes()
		{
			if (watch_file.get() != nullptr &&
				watch_file.get() != INVALID_HANDLE_VALUE &&
				!CancelIoEx(watch_file.get(), nullptr))
			{
				DWORD error = GetLastError();
				if (error != ERROR_NOT_FOUND)
				{
					std::terminate();
				}
			}
		}

		inline directory_changes &directory_changes::operator = (directory_changes &&other)
		{
			is_recursive = other.is_recursive;
			receiver_ = other.receiver_;
			watch_file = std::move(other.watch_file);
			immovable = std::move(other.immovable);
			return *this;
		}

		inline void directory_changes::async_get_one(ptr_observer<observer<element_type>> receiver)
		{
			assert(!receiver_);
			receiver_ = receiver.get();
			assert(immovable);
			immovable->read_dispatcher.post([this]
			{
				DWORD received = 0;
				std::array<char, 0x10000> buffer;
				DWORD const actions = FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_CREATION | FILE_NOTIFY_CHANGE_ATTRIBUTES;
				if (!ReadDirectoryChangesW(watch_file.get(), buffer.data(), static_cast<DWORD>(buffer.size()), is_recursive, actions, &received, nullptr, nullptr))
				{
					//TODO: handle ERROR_NOTIFY_ENUM_DIR (overflow of event queue)
					return;
				}
				std::vector<file_notification> notifications;
				for (char *next_event = buffer.data(); ;)
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
		}
	}
}
#endif

#endif
