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
#ifndef STRINGUTIL_H
#define STRINGUTIL_H

#include <string>
#include <vector>

std::string strip(std::string a);
std::string stripr(std::string a);
std::string stripl(std::string a);
bool startsWith(const std::string &str, const std::string &start);
std::string getRest(const std::string &str, const std::string &prefix);
char **stringvector_to_array(const std::vector<std::string> &vec);


#endif /* STRINGUTIL_H */

// vim: set sw=4 ts=4 fdm=marker et:

