#ifndef SILICIUM_LINUX_DYNAMIC_LIBRARY_IMPL_HPP
#define SILICIUM_LINUX_DYNAMIC_LIBRARY_IMPL_HPP

#include <silicium/c_string.hpp>
#include <boost/system/error_code.hpp>
#include <dlfcn.h>

namespace Si
{
	namespace linux
	{
		struct dynamic_library_impl
		{
			SILICIUM_USE_RESULT
			static void *open(c_string file, boost::system::error_code &ec)
			{
				void * const handle = dlopen(file.c_str(), RTLD_LAZY);
				if (handle)
				{
					ec = boost::system::error_code();
				}
				else
				{
					ec = boost::system::error_code(errno, boost::system::system_category());
				}
				return handle;
			}

			static void close(void *handle)
			{
				dlclose(handle);
			}

			SILICIUM_USE_RESULT
			static void *find_symbol(void *handle, c_string const &name)
			{
				return dlsym(handle, name.c_str());
			}
		};
	}
}

#endif
