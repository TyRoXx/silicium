# - Try to find libsilicium
# Once done this will define
#  LIBSILICIUM_FOUND
#  LIBSILICIUM_INCLUDE_DIRS
#  LIBSILICIUM_LIBRARIES

find_path(LIBSILICIUM_INCLUDE_DIR "version.hpp" HINTS "/usr/local/include/silicium" "/usr/include/silicium")
find_library(LIBSILICIUM_LIBRARY "libsilicium.a" HINTS "/usr/local/lib" "/usr/lib")

set(LIBSILICIUM_LIBRARIES ${LIBSILICIUM_LIBRARY})
set(LIBSILICIUM_INCLUDE_DIRS ${LIBSILICIUM_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set LIBSILICIUM_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(LibSilicium  DEFAULT_MSG
                                  LIBSILICIUM_LIBRARY LIBSILICIUM_INCLUDE_DIR)

mark_as_advanced(LIBSILICIUM_INCLUDE_DIR LIBSILICIUM_LIBRARY )
