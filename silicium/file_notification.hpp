#ifndef SILICIUM_REACTIVE_FILE_NOTIFICATION_HPP
#define SILICIUM_REACTIVE_FILE_NOTIFICATION_HPP

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

		file_notification() BOOST_NOEXCEPT
			: type(file_notification_type::change_content_or_metadata)
		{
		}

		file_notification(file_notification_type type, path name) BOOST_NOEXCEPT
			: type(type)
			, name(std::move(name))
		{
		}
	};
}

#endif
