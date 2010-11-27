/*
 * (c) 2008-2010, Bernhard Walle <bernhard@bwalle.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include <string>
#include <unistd.h>
#include <linux/kd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/vt.h>
#include <errno.h>
#include <cstring>

#include <libbw/stringutil.h>

#include "console.h"
#include "global.h"

/* ---------------------------------------------------------------------------------------------- */
bool Console::isRealTerminal()
{
    std::string terminal = ttyname(STDIN_FILENO);

    return bw::startsWith(terminal, "/dev/tty");
}

/* ---------------------------------------------------------------------------------------------- */
bool Console::isAConsole(int fd)
{
	char arg;

	arg = 0;
	return (ioctl(fd, KDGKBTYPE, &arg) == 0
		&& ((arg == KB_101) || (arg == KB_84)));
}

/* ---------------------------------------------------------------------------------------------- */
int Console::openConsoleFd()
{
    /* code taken from chvt.c in kbd package */

    int fd;
    const char *terminals[] = {
        "/dev/tty",
        "/dev/tty0",
        "/dev/vc/0",
        "/dev/console"
    };

    for (unsigned int i = 0; i < sizeof(terminals)/sizeof(terminals[0]); i++) {
        fd = open(terminals[i], O_RDWR);
        if (fd < 0 && errno == EACCES)
            fd = open(terminals[i], O_WRONLY);
        if (fd < 0 && errno == EACCES)
            fd = open(terminals[i], O_RDONLY);
        if (fd < 0)
            return -1;

        if (isAConsole(fd))
            return fd;
        else
            close(fd);
    }

    for (fd = 0; fd < 3; fd++)
        if (isAConsole(fd))
            return fd;

    return -1;
}

/* ---------------------------------------------------------------------------------------------- */
void Console::changeVirtualTerminal(int newTerminal, bool wait)
{
    int fd = openConsoleFd();

    if (fd < 0)
        throw ApplicationError("Unable to get console file descriptor");

    if (ioctl(fd, VT_ACTIVATE, newTerminal)) {
        close(fd);
        throw ApplicationError(strerror(errno));
    }

    if (wait) {
        if (ioctl(fd, VT_WAITACTIVE, newTerminal)) {
            close(fd);
            throw ApplicationError(strerror(errno));
        }
    }

    close(fd);
}

// :tabSize=4:indentSize=4:noTabs=true:mode=c++:folding=explicit:collapseFolds=1:maxLineLen=100:
