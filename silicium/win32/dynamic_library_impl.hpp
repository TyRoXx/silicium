#ifndef SILICIUM_WIN32_DYNAMIC_LIBRARY_IMPL_HPP
#define SILICIUM_WIN32_DYNAMIC_LIBRARY_IMPL_HPP

#include <silicium/win32/win32.hpp>
#include <boost/filesystem/path.hpp>

namespace Si
{
#ifdef _WIN32
	namespace win32
	{
		struct dynamic_library_impl
		{
			static void *open(boost::filesystem::path const &file, boost::system::error_code &ec)
			{
				HMODULE const handle = LoadLibraryW(file.c_str());
				if (handle)
				{
					ec = boost::system::error_code();
				}
				else
				{
					ec = boost::system::error_code(GetLastError(), boost::system::system_category());
				}
				return handle;
			}

			static void close(void *handle)
			{
				FreeLibrary(static_cast<HMODULE>(handle));
			}

			static void *find_symbol(void *handle, std::string const &name)
			{
				return GetProcAddress(static_cast<HMODULE>(handle), name.c_str());
			}
		};
	}
#endif
}

#endif
