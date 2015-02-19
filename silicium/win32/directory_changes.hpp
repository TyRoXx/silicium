#ifndef SILICIUM_REACTIVE_DIRECTORY_CHANGES_HPP
#define SILICIUM_REACTIVE_DIRECTORY_CHANGES_HPP

#include <boost/asio/io_service.hpp>
#include <silicium/observable/observer.hpp>
#include <silicium/win32/win32.hpp>
#include <silicium/config.hpp>
#include <silicium/path.hpp>
#include <silicium/error_or.hpp>
#include <boost/optional.hpp>
#include <boost/ref.hpp>
#include <future>

namespace Si
{
	namespace win32
	{
		struct file_notification
		{
			DWORD action = 0;
			path name;

			file_notification()
			{
			}

			file_notification(DWORD action, path name)
				: action(action)
				, name(std::move(name))
			{
			}
		};

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
				boost::optional<boost::asio::io_service::work> read_active;

				immovable_state()
					: read_active(boost::in_place(boost::ref(read_dispatcher)))
				{
				}

				~immovable_state()
				{
					read_active.reset();
					read_dispatcher.stop();
					read_runner.get();
				}
			};

			//this indirection is needed for movability
			std::unique_ptr<immovable_state> immovable;

			//The watch file comes after the threading state so that it will be destroyed first to unblock a possible read operation
			//in the reading thread.
			Si::win32::unique_handle watch_file;

			BOOST_DELETED_FUNCTION(directory_changes(directory_changes const &));
			BOOST_DELETED_FUNCTION(directory_changes &operator = (directory_changes const &));
		};
	}
}

#endif
