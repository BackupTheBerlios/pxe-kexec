#!/bin/sh

release=`grep PACKAGE_VERSION CMakeLists.txt |head -1|sed -e 's/.*"\(.*\)\".*/\1/g'`
hg archive -t tbz2 pxe-kexec-${release}.tar.bz2

