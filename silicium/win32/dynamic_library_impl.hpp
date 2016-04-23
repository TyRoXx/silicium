#ifndef SILICIUM_WIN32_DYNAMIC_LIBRARY_IMPL_HPP
#define SILICIUM_WIN32_DYNAMIC_LIBRARY_IMPL_HPP

#include <silicium/c_string.hpp>
#include <silicium/throw_last_error.hpp>
#include <boost/filesystem/path.hpp>

namespace Si
{
#ifdef _WIN32
    namespace win32
    {
        struct dynamic_library_impl
        {
            SILICIUM_USE_RESULT
            static void *open(native_path_string file)
            {
                HMODULE const handle = LoadLibraryW(file.c_str());
                if (!handle)
                {
                    throw_last_error();
                }
                return handle;
            }

            static void close(void *handle)
            {
                FreeLibrary(static_cast<HMODULE>(handle));
            }

            SILICIUM_USE_RESULT
            static void *find_symbol(void *handle, c_string name)
            {
                return GetProcAddress(
                    static_cast<HMODULE>(handle), name.c_str());
            }
        };
    }
#endif
}

#endif
