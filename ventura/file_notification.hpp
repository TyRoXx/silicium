#ifndef VENTURA_FILE_NOTIFICATION_HPP
#define VENTURA_FILE_NOTIFICATION_HPP

#include <boost/detail/scoped_enum_emulation.hpp>
#include <ventura/relative_path.hpp>

namespace ventura
{
#ifdef BOOST_SCOPED_ENUM_DECLARE_BEGIN
	BOOST_SCOPED_ENUM_DECLARE_BEGIN(file_notification_type)
#else
	enum class file_notification_type
#endif
	{
		add,
		remove,
		move_self,
		remove_self,
		change_content,
		change_metadata,
		change_content_or_metadata
	}
#ifdef BOOST_SCOPED_ENUM_DECLARE_END
	BOOST_SCOPED_ENUM_DECLARE_END(file_notification_type)
#else
	;
#endif

	struct file_notification
	{
		file_notification_type type;
		relative_path name;
		bool is_directory;

		file_notification() BOOST_NOEXCEPT
			: type(file_notification_type::change_content_or_metadata)
		    , is_directory(false)
		{
		}

		file_notification(file_notification_type type, relative_path name, bool is_directory) BOOST_NOEXCEPT
			: type(type)
			, name(std::move(name))
		    , is_directory(is_directory)
		{
		}
	};

#if SILICIUM_HAS_IS_HANDLE
	BOOST_STATIC_ASSERT(Si::is_handle<file_notification>::value);
#endif
}

#endif
