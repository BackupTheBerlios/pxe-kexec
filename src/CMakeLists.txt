add_executable(pxe-kexec
        kexec.cc
        optionparser.cc
        pxeparser.cc
        downloader.cc
        main.cc
        process.cc
        stringutil.cc
        io.cc
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

