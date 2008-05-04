#
# This Makefile is only to produce the manpage.
# I don't want to produce the manpage with cmake since
# then the additional dependency pod2man is required.
# Since it doesn't make any difference whether the manpage
# is produced on my system or on any other system, I
# distribute the manpage with the sources.
#

POD2MAN = pod2man

all: pxe-kexec.8

pxe-kexec.8: pxe-kexec.pod
	$(POD2MAN) pxe-kexec.pod        \
		--center="PXE-Kexec"    \
		--release=$(VERSION)    \
		> pxe-kexec.8

# vim: set ts=8 sw=8 noet:
