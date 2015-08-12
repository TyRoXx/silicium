#ifndef SILICIUM_LINUX_DYNAMIC_LIBRARY_IMPL_HPP
#define SILICIUM_LINUX_DYNAMIC_LIBRARY_IMPL_HPP

#include <silicium/c_string.hpp>
#include <silicium/get_last_error.hpp>
#include <dlfcn.h>
#include <boost/throw_exception.hpp>

namespace Si
{
	namespace linux
	{
		struct dynamic_library_impl
		{
			SILICIUM_USE_RESULT
			static void *open(c_string file)
			{
				void * const handle = dlopen(file.c_str(), RTLD_LAZY);
				if (!handle)
				{
					boost::throw_exception(std::runtime_error(dlerror()));
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
