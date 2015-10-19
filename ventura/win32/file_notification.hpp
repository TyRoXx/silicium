#ifndef VENTURA_WIN32_FILE_NOTICATION_HPP
#define VENTURA_WIN32_FILE_NOTICATION_HPP

#include <ventura/relative_path.hpp>
#include <silicium/win32/win32.hpp>

namespace ventura
{
	namespace win32
	{
		struct file_notification
		{
			DWORD action;
			relative_path name;

			file_notification()
				: action(0)
			{
			}

			file_notification(DWORD action, relative_path name)
				: action(action)
				, name(std::move(name))
			{
			}
		};
	}
}

#endif
