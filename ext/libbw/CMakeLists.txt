# Copyright (c) 2009, Bernhard Walle <bernhard@bwalle.de>
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in the
#       documentation and/or other materials provided with the distribution.
#     * Neither the name of the <organization> nor the
#       names of its contributors may be used to endorse or promote products
#       derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY <copyright holder> ''AS IS'' AND ANY
# EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL <copyright holder> BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

SET (HAVE_LIBREADLINE 0)

# Readline

FIND_PATH(READLINE_INCLUDE_DIR readline/readline.h
    /usr/local/include /usr/include)
MARK_AS_ADVANCED(READLINE_INCLUDE_DIR)

FIND_LIBRARY(READLINE_LIBRARY readline
    /usr/lib64 /usr/lib)
MARK_AS_ADVANCED(READLINE_LIBRARY)

IF (READLINE_LIBRARY AND READLINE_INCLUDE_DIR)
    INCLUDE_DIRECTORIES( ${READLINE_INCLUDE_DIR} )
    SET (EXTRA_LIBS ${EXTRA_LIBS} ${READLINE_LIBRARY})
    SET (HAVE_LIBREADLINE 1)
ENDIF (READLINE_LIBRARY AND READLINE_INCLUDE_DIR)

IF (NOT CURSES_LIBRARIES)
    FIND_PACKAGE(Curses REQUIRED)
ENDIF (NOT CURSES_LIBRARIES)

SET (EXTRA_LIBS ${EXTRA_LIBS} ${CURSES_LIBRARIES})
INCLUDE_DIRECTORIES( ${CURSES_INCLUDE_DIR} )

# for bwconfig.h
INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_BINARY_DIR} )


ADD_LIBRARY(
    bw
    STATIC
    debug.cc
    stringutil.cc
    completion.cc
)

CONFIGURE_FILE(
    "${CMAKE_CURRENT_SOURCE_DIR}/bwconfig.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/bwconfig.h"
)


# vim: set sw=4 ts=4 et:
