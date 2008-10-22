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
#include <string>
#include <vector>
#include <cstdlib>
#include <cstring>
#include <string.h>
#include "global.h"

using std::string;
using std::vector;
using std::malloc;

/* -------------------------------------------------------------------------- */
string strip(string a)
{
    if (a.length() == 0)
        return a;

    a.erase(0, a.find_first_not_of("\n \t", 0));
    a.erase(a.find_last_not_of("\n \t")+1);

    return a;
}

/* -------------------------------------------------------------------------- */
string stripl(string a)
{
    if (a.length() == 0)
        return a;

    a.erase(0, a.find_first_not_of("\n \t", 0));

    return a;
}

/* -------------------------------------------------------------------------- */
string stripr(string a)
{
    if (a.length() == 0)
        return a;

    a.erase(a.find_last_not_of("\n \t")+1);

    return a;
}

/* -------------------------------------------------------------------------- */
bool startsWith(const string &str, const string &start)
{
    size_t len = start.size();
    return (str.size() > len) && str.substr(0, len) == start;
}

/* -------------------------------------------------------------------------- */
string getRest(const string &str, const string &prefix)
{
    return str.substr(prefix.size(), str.size() - prefix.size());
}

/* -------------------------------------------------------------------------- */
char **stringvector_to_array(const StringVector &vec)
{
    if (vec.size() == 0)
        return NULL;

    /* use malloc to be able to free the result with C free() */
    char **ret = (char **)malloc(sizeof(char *) * (vec.size()+1) );

    char **cur = ret;
    for (StringVector::const_iterator it = vec.begin(); it != vec.end(); ++it)
        *cur++ = strdup((*it).c_str());
    *cur = NULL;

    return ret;
}

// vim: set sw=4 ts=4 fdm=marker et:
