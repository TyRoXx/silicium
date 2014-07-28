#ifndef SILICIUM_REACTIVE_DIRECTORY_CHANGES_HPP
#define SILICIUM_REACTIVE_DIRECTORY_CHANGES_HPP

#include <reactive/observable.hpp>
#include <silicium/win32.hpp>
#include <silicium/override.hpp>
#include <boost/filesystem/path.hpp>

namespace rx
{
	namespace win32
	{
		struct file_notification
		{

		};

		struct directory_changes : observable<file_notification>
		{
			using element_type = file_notification;

			explicit directory_changes(boost::filesystem::path const &watched);
			virtual void async_get_one(observer<element_type> &receiver) SILICIUM_OVERRIDE;
			virtual void cancel() SILICIUM_OVERRIDE;

		private:

			observer<element_type> *receiver_ = nullptr;
			Si::win32::unique_handle watch_file;
		};
	}
}

#endif
