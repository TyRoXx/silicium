#ifndef SILICIUM_WIN32_FILE_NOTICATION_HPP
#define SILICIUM_WIN32_FILE_NOTICATION_HPP

#include <silicium/path.hpp>
#include <silicium/win32/win32.hpp>

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
	}
}

#endif
