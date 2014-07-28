#include <reactive/win32/directory_changes.hpp>
#include <reactive/exchange.hpp>
#include <Windows.h>

namespace rx
{
	namespace win32
	{
		directory_changes::directory_changes(boost::filesystem::path const &watched)
			: watch_file(CreateFileW(
				watched.c_str(),
				FILE_LIST_DIRECTORY,
				FILE_SHARE_WRITE | FILE_SHARE_READ | FILE_SHARE_DELETE,
				NULL,
				OPEN_EXISTING,
				FILE_FLAG_BACKUP_SEMANTICS,
				NULL
			))
		{
			if (watch_file.get() == INVALID_HANDLE_VALUE)
			{
				throw boost::system::system_error(::GetLastError(), boost::system::native_ecat);
			}
		}

		void directory_changes::async_get_one(observer<element_type> &receiver)
		{
		}

		void directory_changes::cancel()
		{
		}
	}
}
