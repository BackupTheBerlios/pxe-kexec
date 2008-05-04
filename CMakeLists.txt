PROJECT(pxe-kexec CXX)

#
# Configuration
#

set (PACKAGE_STRING "pxe-kexec")
set (PACKAGE_VERSION "0.1.2")

configure_file(
    "${PROJECT_SOURCE_DIR}/config.h.in"
    "${PROJECT_BINARY_DIR}/config.h"
)

include_directories("${PROJECT_BINARY_DIR}")

#
# Directories
#

set(MANDIR "${CMAKE_INSTALL_PREFIX}/share/man" CACHE
        STRING "Place where the manual page is installed.")

#
# Libraries
#

# Readline

find_path(READLINE_INCLUDE_DIR readline/readline.h
    /usr/local/include /usr/include)
mark_as_advanced(READLINE_INCLUDE_DIR)

find_library(READLINE_LIBRARY readline
    /usr/lib64 /usr/lib)
mark_as_advanced(READLINE_LIBRARY)

if (READLINE_LIBRARY AND READLINE_INCLUDE_DIR)
    link_directories( ${READLINE_LIBRARY} ${CURL_LIBRARIES} )
    include_directories( ${READLINE_INCLUDE_DIR} ${CURL_INCLUDE_DIRS} )
    set (EXTRA_LIBS ${EXTRA_LIBS} ${READLINE_LIBRARY})
endif (READLINE_LIBRARY AND READLINE_INCLUDE_DIR)

# CURL

set (CMAKE_LIBRARY_PATH /usr/lib64 ${CMAKE_LIBRARY_PATH})
find_package(CURL REQUIRED)
set (EXTRA_LIBS ${EXTRA_LIBS} ${CURL_LIBRARIES})


#
# Subdirs
#

ADD_SUBDIRECTORY(src)

#
# Source Packages
#

SET(CPACK_GENERATOR "TGZ;TBZ2") # This line is need for a CMake (Version 2.4.7) Bug - Fixed in CVS
SET(CPACK_SOURCE_GENERATOR "TGZ;TBZ2")
SET(CPACK_SOURCE_PACKAGE_FILE_NAME "${PACKAGE_STRING}-${PACKAGE_VERSION}")
SET(CPACK_SOURCE_IGNORE_FILES "${CPACK_SOURCE_IGNORE_FILES};/.git;build")
INCLUDE(CPack)

# vim: set sw=4 ts=4 et:
