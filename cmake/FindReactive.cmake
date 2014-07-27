# - Try to find libREACTIVE
# Once done this will define
#  LIBREACTIVE_FOUND
#  LIBREACTIVE_INCLUDE_DIRS
#  LIBREACTIVE_LIBRARIES

find_path(LIBREACTIVE_INCLUDE_DIR "reactive/config.hpp" HINTS "/usr/local/include" "/usr/include")
find_library(LIBREACTIVE_LIBRARY "libreactive.a" HINTS "/usr/local/lib" "/usr/lib")

set(LIBREACTIVE_LIBRARIES ${LIBREACTIVE_LIBRARY})
set(LIBREACTIVE_INCLUDE_DIRS ${LIBREACTIVE_INCLUDE_DIR} ${LIBREACTIVE_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set LIBREACTIVE_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(LibREACTIVE  DEFAULT_MSG
                                  LIBREACTIVE_LIBRARY LIBREACTIVE_INCLUDE_DIR)

mark_as_advanced(LIBREACTIVE_INCLUDE_DIR LIBREACTIVE_LIBRARY)
