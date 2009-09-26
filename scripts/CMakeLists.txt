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

# This is a bit unclean since we test if the directory exists on the host
# system. But well, for people that just install pxe-kexec on their
# machines manually, it works. Packagers can easily copy the script
# in their packaging script. But I don't want to clutter the system
# of people that are using dhcpcd.

IF (EXISTS /etc/dhcp3/dhclient-enter-hooks.d)
    INSTALL(
        PROGRAMS        dhclient-enter-hook-pxe-kexec.sh
        DESTINATION     /etc/dhcp3/dhclient-enter-hooks.d
        RENAME          pxe_kexec
    )
ENDIF (EXISTS /etc/dhcp3/dhclient-enter-hooks.d)
