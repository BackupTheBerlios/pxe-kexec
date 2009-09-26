#
# (c) 2009, Bernhard Walle <bernhard@bwalle.de>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.
#

PROJECT(pxe-kexec CXX C)

#
# Configuration
#

cmake_minimum_required(VERSION 2.4)
set (PACKAGE_STRING "pxe-kexec")
set (PACKAGE_VERSION "0.2.3")

include_directories("${PROJECT_BINARY_DIR}")

#
# Executable
#
FIND_PROGRAM(POD2MAN_EXECUTABLE pod2man)

#
# Directories
#

set(MANDIR "${CMAKE_INSTALL_PREFIX}/share/man" CACHE
        STRING "Place where the manual page is installed.")

#
# Libraries
#

set (CMAKE_LIBRARY_PATH /usr/lib64 ${CMAKE_LIBRARY_PATH})

# Readline

find_path(READLINE_INCLUDE_DIR readline/readline.h
    /usr/local/include /usr/include)
mark_as_advanced(READLINE_INCLUDE_DIR)

find_library(READLINE_LIBRARY readline
    /usr/lib64 /usr/lib)
mark_as_advanced(READLINE_LIBRARY)

if (READLINE_LIBRARY AND READLINE_INCLUDE_DIR)
    include_directories( ${READLINE_INCLUDE_DIR} )
    set (EXTRA_LIBS ${EXTRA_LIBS} ${READLINE_LIBRARY})
    set (HAVE_LIBREADLINE 1)
endif (READLINE_LIBRARY AND READLINE_INCLUDE_DIR)

if (NOT CURSES_LIBRARIES)
    find_package(Curses REQUIRED)
endif (NOT CURSES_LIBRARIES)

set (EXTRA_LIBS ${EXTRA_LIBS} ${CURSES_LIBRARIES})
include_directories( ${CURSES_INCLUDE_DIR} )


# CURL

find_package(CURL REQUIRED)
set (EXTRA_LIBS ${EXTRA_LIBS} ${CURL_LIBRARIES})
include_directories(${CURL_INCLUDE_DIRS})

# libbw

ADD_SUBDIRECTORY(ext/libbw)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/ext/libbw)
SET (EXTRA_LIBS ${EXTRA_LIBS} bw)

#
# Configure file
#

configure_file(
    "${PROJECT_SOURCE_DIR}/config.h.in"
    "${PROJECT_BINARY_DIR}/config.h"
)


#
# Subdirs
#

ADD_SUBDIRECTORY(src)


#
# Documentation
#


ADD_CUSTOM_TARGET(apidoc
    doxygen
    WORKING_DIRECTORY ${pxe-kexec_SOURCE_DIR}
)

# vim: set sw=4 ts=4 et:
