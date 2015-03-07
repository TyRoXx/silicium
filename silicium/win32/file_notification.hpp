#ifndef SILICIUM_WIN32_FILE_NOTICATION_HPP
#define SILICIUM_WIN32_FILE_NOTICATION_HPP

#include <silicium/relative_path.hpp>
#include <silicium/win32/win32.hpp>

namespace Si
{
	namespace win32
	{
		struct file_notification
		{
			DWORD action = 0;
			relative_path name;

			file_notification()
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
