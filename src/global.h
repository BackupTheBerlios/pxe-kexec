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
#ifndef GLOBAL_H
#define GLOBAL_H

/**
 * @file global.h
 * @brief Global definitions
 *
 * This file contains everything that is global and didn't fit in another
 * file.
 *
 * @author Bernhard Walle <bernhard@bwalle.de>
 */

#include <vector>
#include <stdexcept>

/* typedefs {{{ */

/**
 * @brief A simple byte vector.
 */
typedef std::vector<unsigned char> ByteVector;

/**
 * @brief A simple string vector.
 */
typedef std::vector<std::string> StringVector;

/* }}} */
/* Macros {{{ */

/**
 * @brief Returns the size of a static array
 */
#define ARRAY_SIZE(a) \
    sizeof(a)/sizeof((a)[0])

/* }}} */
/* ParseError {{{ */

/**
 * @brief Error when parsing a file
 *
 * This exception class is thrown when parsing failed. Use the
 * std::runtime_error::what() method to retrieve the error message in your
 * exception handler.
 *
 * @author Bernhard Walle <bernhard@bwalle.de>
 */
class ParseError : public std::runtime_error {
    public:
        /**
         * @brief Constructor
         *
         * Creates a new ParseError.
         *
         * @param[in] string the error string
         */
        ParseError(const std::string& string)
            : std::runtime_error(string) {}
};

/* }}} */
/* ApplicationError {{{ */

/**
 * @brief General application error
 *
 * This exception class is thrown as general application error. Use the
 * std::runtime_error::what() method to retrieve the error message in your
 * exception handler.
 *
 * @author Bernhard Walle <bernhard@bwalle.de>
 */
class ApplicationError : public std::runtime_error {
    public:
        /**
         * @brief Constructor
         *
         * Creates a new ApplicationError.
         *
         * @param[in] string the error string
         */
        ApplicationError(const std::string& string)
            : std::runtime_error(string) {}
};

/* }}} */

#endif /* GLOBAL_H */

// :tabSize=4:indentSize=4:noTabs=true:mode=c++:folding=explicit:collapseFolds=1:maxLineLen=100:
