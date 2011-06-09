#!/bin/sh
set -e

NAME=pxe-kexec

VERSION=`grep PACKAGE_VERSION CMakeLists.txt |head -1|sed -e 's/.*"\(.*\)\".*/\1/g'`
ARCHIVE=$NAME-$VERSION.tar.bz2
hg archive -X dist.sh -t tbz2 $NAME-${VERSION}.tar.bz2
echo "Archive '$ARCHIVE' ready!"
