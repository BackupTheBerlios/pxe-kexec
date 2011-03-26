#!/bin/sh
set -e

NAME=pxe-kexec

VERSION=`grep PACKAGE_VERSION CMakeLists.txt |head -1|sed -e 's/.*"\(.*\)\".*/\1/g'`
ARCHIVE=$NAME-$VERSION.tar.bz2
git archive --format=tar --prefix=$NAME-$VERSION/ master | bzip2 > $ARCHIVE
echo "Archive '$ARCHIVE' ready!"
