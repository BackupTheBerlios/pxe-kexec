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

add_executable(pxe-kexec
        kexec.cc
        pxeparser.cc
        downloader.cc
        main.cc
        process.cc
        networkhelper.cc
        console.cc
        pxekexec.cc
        linuxdb.cc
        ext/rpmvercmp.c)
target_link_libraries(pxe-kexec ${EXTRA_LIBS})

INSTALL(
    TARGETS
        pxe-kexec
    DESTINATION
        sbin
)

INSTALL(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/pxe-kexec.8.gz
    DESTINATION
        ${MANDIR}/man8
)


ADD_CUSTOM_COMMAND(
    OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/pxe-kexec.8.gz
    COMMAND
        ${POD2MAN_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/pxe-kexec.pod
        --center="PXE-Kexec"
        --release=${PACKAGE_VERSION}
        ${CMAKE_CURRENT_BINARY_DIR}/pxe-kexec.8 &&
        rm -f ${CMAKE_CURRENT_SOURCE_DIR}/pxe-kexec.8.xml &&
        gzip -f ${CMAKE_CURRENT_BINARY_DIR}/pxe-kexec.8
    DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/pxe-kexec.pod
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}
)

#
# always build the manpages before building the program
ADD_DEPENDENCIES(
    pxe-kexec
    manpages
)

#
# we need to make a target out of the two ADD_CUSTOM_COMMAND() calls
# because otherwise cmake ignores the manpages
ADD_CUSTOM_TARGET(
    manpages
    DEPENDS
        ${CMAKE_CURRENT_BINARY_DIR}/pxe-kexec.8.gz
)

