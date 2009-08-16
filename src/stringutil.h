/*
 * (c) 2008-2009, Bernhard Walle <bernhard.walle@gmx.de>
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
#ifndef STRINGUTIL_H
#define STRINGUTIL_H

/**
 * @file stringutil.h
 * @brief String helper functions
 *
 * This file contains global functions for working with strings.
 *
 * @author Bernhard Walle <bernhard.walle@gmx.de>
 */

#include <string>
#include <vector>

/**
 * @brief Removes spaces from a string
 *
 * Removes trailing and leading spaces, tabs and newlines.
 *
 * @param[in] a the string to strip (this string is not modified)
 * @return the stripped string
 */
std::string strip(std::string a);

/**
 * @brief Removes spaces from a string on the right side
 *
 * Removes leading spaces, tabs and newlines.
 *
 * @param[in] a the string to strip (this string is not modified)
 * @return the stripped string
 */
std::string stripr(std::string a);

/**
 * @brief Removes spaces from a string on the left side
 *
 * Removes trailing spaces, tabs and newlines.
 *
 * @param[in] a the string to strip  (this string is not modified)
 * @return the stripped string
 */
std::string stripl(std::string a);

/**
 * @brief Checks if a string starts with another string
 *
 * Checks if @p str starts with @p start. If @p casesensitive is @c true, the
 * matching is done case-sensitive.
 *
 * @param[in] str the string which is checked if it starts with @p start
 * @param[in] start the string which is taken to check if @p str starts with
 *            @p start
 * @param[in] casesensitive if @c true, the comparison is done case-sensitive
 * @return @c true if @p str starts with @p start, @c false otherwise
 */
bool startsWith(const std::string &str, const std::string &start, bool casesensitive = true);

/**
 * @brief Returns the rest
 *
 * If @p str starts with @p prefix, returns the rest, i.e @p str without @p
 * prefix. If @p str does not start with @p prefix, @p str is returned
 * unmodified.
 *
 * @param[in] str the base string
 * @param[in] prefix the prefix which is stripped
 * @return the rest string as described above
 */
std::string getRest(const std::string &str, const std::string &prefix);

/**
 * @brief Converts a string vector to a C array
 *
 * Converts a std::vector<std::string> to a char ** array. If the size of the
 * vector @p vec is 0, @c NULL is returned.
 *
 * @param[in] vec the vector to convert
 * @return the C array. The memory needs to be freed with free() after using
 *         the return value.
 */
char **stringvector_to_array(const std::vector<std::string> &vec);

/**
 * @brief Splits a string
 *
 * Splits a string in a vector. For example, if @p str is
 * <tt>"abra:kadabra"</tt> and if @p pattern is <tt>":"</tt> (both without
 * the quotes), then the resulting vector contains two elements, being
 * <tt>"abra"</tt> the first and <tt>"kadabra"</tt> the second.
 *
 * @param[in] str the string to split
 * @param[in] pattern the pattern which is used as separator
 * @return the result of the string split. If @p str does not contain @p
 *         pattern, the result is just @p str.
 */
std::vector<std::string> stringsplit(const std::string &str, const std::string &pattern);

#endif /* STRINGUTIL_H */

// vim: set sw=4 ts=4 fdm=marker et:

