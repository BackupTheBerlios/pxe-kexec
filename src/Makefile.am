MAINTAINERCLEANFILES    = Makefile.in
INCLUDES                = -I$(top_builddir) -I$(top_srcdir)

EXTRA_DIST		= pxe-kexec.8 pxe-kexec.pod
sbin_PROGRAMS           = pxe-kexec
pxe_kexec_SOURCES  	= global.h 				\
			  main.cc 				\
			  debug.h debug.cc 			\
			  kexec.h kexec.cc 			\
			  process.h process.cc 			\
			  stringutil.h stringutil.cc 		\
			  optionparser.h optionparser.cc 	\
			  io.h io.cc 				\
			  pxeparser.h pxeparser.cc 		\
			  pxekexec.h pxekexec.cc 		\
			  networkhelper.h networkhelper.cc 	\
			  downloader.h downloader.cc
man_MANS		= pxe-kexec.8


pxe-kexec.8: pxe-kexec.pod
	pod2man pxe-kexec.pod           \
                --center="PXE-Kexec"    \
                --release=$(VERSION)    \
                > pxe-kexec.8

