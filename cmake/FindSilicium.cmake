# - Try to find libsilicium
# Once done this will define
#  SILICIUM_FOUND
#  SILICIUM_INCLUDE_DIRS
#  SILICIUM_LIBRARIES

find_path(SILICIUM_INCLUDE_DIR "silicium/version.hpp" HINTS "/usr/local/include" "/usr/include")
find_library(SILICIUM_LIBRARY "libsilicium.a" HINTS "/usr/local/lib" "/usr/lib")

set(SILICIUM_LIBRARIES ${SILICIUM_LIBRARY})
set(SILICIUM_INCLUDE_DIRS ${SILICIUM_INCLUDE_DIR} ${REACTIVE_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set LIBSILICIUM_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(LibSilicium  DEFAULT_MSG
                                  SILICIUM_LIBRARY SILICIUM_INCLUDE_DIR)

mark_as_advanced(SILICIUM_INCLUDE_DIR SILICIUM_LIBRARY)
