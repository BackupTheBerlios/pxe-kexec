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
    set (EXTRA_LIBS ${EXTRA_LIBS} readline)
endif (READLINE_LIBRARY AND READLINE_INCLUDE_DIR)

# CURL

set (CMAKE_LIBRARY_PATH /usr/lib64 ${CMAKE_LIBRARY_PATH})
find_package(CURL REQUIRED)
set (EXTRA_LIBS ${EXTRA_LIBS} curl)


#
# Subdirs
#

ADD_SUBDIRECTORY(src)

# vim: set sw=4 ts=4 et:
