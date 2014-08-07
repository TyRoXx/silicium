# - Try to find libREACTIVE
# Once done this will define
#  REACTIVE_FOUND
#  REACTIVE_INCLUDE_DIRS
#  REACTIVE_LIBRARIES

find_path(REACTIVE_INCLUDE_DIR "reactive/config.hpp" HINTS "/usr/local/include" "/usr/include")
find_library(REACTIVE_LIBRARY "libreactive.a" HINTS "/usr/local/lib" "/usr/lib")

set(REACTIVE_LIBRARIES ${REACTIVE_LIBRARY})
set(REACTIVE_INCLUDE_DIRS ${REACTIVE_INCLUDE_DIR} ${REACTIVE_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set LIBREACTIVE_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(LibREACTIVE  DEFAULT_MSG
                                  REACTIVE_LIBRARY REACTIVE_INCLUDE_DIR)

mark_as_advanced(REACTIVE_INCLUDE_DIR REACTIVE_LIBRARY)
