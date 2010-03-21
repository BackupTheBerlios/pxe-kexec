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
#ifndef CONSOLE_H
#define CONSOLE_H

/**
 * @file console.h
 * @brief Terminal abstractions
 *
 * Some static helper functions for dealing with virtual terminals on Linux.
 *
 * @author Bernhard Walle <bernhard@bwalle.de>
 */

/**
 * @brief Terminal abstraction functions
 *
 * This class contains only static functions.
 *
 * @author Bernhard Walle <bernhard@bwalle.de>
 */
class Console {

    public:
        /**
         * @brief Checks if the current terminal is a VT
         *
         * Checks if the current terminal is a real Linux terminal of just a
         * virtual terminal like a X Window terminal emulator (@c xterm).
         *
         * @return @c true if the current terminal is a real terminal,
         *         @c false otherwise
         */
        static bool isRealTerminal();

        /**
         * @brief Switch to another virtual console
         *
         * Switch to another virtual Linux console. This function does the
         * same like when the user presses <tt>Alt-NUMBER</tt>.
         *
         * @param[in] newTerminal the number of the new terminal, starting
         *            from @c 1
         * @param[in] wait if set to @c true, then we wait until the switch is
         *            done
         */
        static void changeVirtualTerminal(int newTerminal, bool wait = true);

    protected:
        /**
         * @brief Opens the file descriptor of a console
         *
         * Tries to get a file descriptor from a real console. Tries that devices
         * in that order:
         *
         *  - @c /dev/tty"
         *  - @c /dev/tty0
         *  - @c /dev/vc/0
         *  - @c /dev/console
         *
         * @return the device descriptor of the console
         */
        static int openConsoleFd();

        /**
         * @brief Checks if a given file descriptor is a console
         *
         * Checks if the given @p fd is a console.
         *
         * @return @c true if it's a console, @c  false otherwise
         */
        static bool isAConsole(int fd);
};

#endif /* CONSOLE_H */

// :tabSize=4:indentSize=4:noTabs=true:mode=c++:folding=explicit:collapseFolds=1:maxLineLen=100:
