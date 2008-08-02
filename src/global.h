/*
 * (c) 2008, Bernhard Walle <bwalle@suse.de>
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
#ifndef GLOBAL_H
#define GLOBAL_H

#include <vector>
#include <stdexcept>

/* typedefs {{{ */

typedef std::vector<unsigned char> ByteVector;
typedef std::vector<std::string> StringVector;

/* }}} */
/* ParseError {{{ */

class ParseError : public std::runtime_error {
    public:
        ParseError(const std::string& string)
            : std::runtime_error(string) {}
};

/* }}} */
/* ApplicationError {{{ */

class ApplicationError : public std::runtime_error {
    public:
        ApplicationError(const std::string& string)
            : std::runtime_error(string) {}
};

/* }}} */
/* IOError {{{ */

class IOError : public std::runtime_error {
    public:
        IOError(const std::string& string)
            : std::runtime_error(string) {}
};

/* }}} */

#endif /* GLOBAL_H */

// vim: set sw=4 ts=4 fdm=marker et:

