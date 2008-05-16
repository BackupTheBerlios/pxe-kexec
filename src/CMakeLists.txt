add_executable(pxe-kexec
        debug.cc
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
        pxekexec.cc)
target_link_libraries(pxe-kexec ${EXTRA_LIBS})

install (TARGETS pxe-kexec DESTINATION sbin)
install (FILES pxe-kexec.8 DESTINATION ${MANDIR}/man8)
