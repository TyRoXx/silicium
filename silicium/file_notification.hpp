#ifndef SILICIUM_FILE_NOTIFICATION_HPP
#define SILICIUM_FILE_NOTIFICATION_HPP

#include <boost/detail/scoped_enum_emulation.hpp>
#include <silicium/path.hpp>

namespace Si
{
	BOOST_SCOPED_ENUM_DECLARE_BEGIN(file_notification_type)
	{
		add,
		remove,
		move_self,
		remove_self,
		change_content,
		change_metadata,
		change_content_or_metadata
	}
	BOOST_SCOPED_ENUM_DECLARE_END(file_notification_type)

	struct file_notification
	{
		file_notification_type type;
		path name;
		bool is_directory;

		file_notification() BOOST_NOEXCEPT
			: type(file_notification_type::change_content_or_metadata)
		    , is_directory(false)
		{
		}

		file_notification(file_notification_type type, path name, bool is_directory) BOOST_NOEXCEPT
			: type(type)
			, name(std::move(name))
		    , is_directory(is_directory)
		{
		}
	};
}

#endif
