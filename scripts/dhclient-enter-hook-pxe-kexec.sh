#!/bin/sh
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

export LC_ALL=C

LOCATION=/var/lib/pxe-kexec
FILENAME="${LOCATION}/${interface}"
DATE=`date`

if ! [ -d "${LOCATION}" ] ; then
    mkdir "${LOCATION}"
fi

echo "# Generated by dhclient-enter-hook-pxe-kexec.sh at $DATE" > ${FILENAME}
echo "dhcp_server_identifier=${new_dhcp_server_identifier}" >> ${FILENAME}

# vim: set sw=4 ts=4 et:
