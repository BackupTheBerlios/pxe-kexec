/*
 * (c) 2008-2009, Bernhard Walle <bernhard@bwalle.de>
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

using std::string;
using std::vector;
using std::malloc;

/* ---------------------------------------------------------------------------------------------- */
string strip(string a, const string &chars_to_strip)
{
    if (a.length() == 0)
        return a;

    a.erase(0, a.find_first_not_of(chars_to_strip.c_str(), 0));
    a.erase(a.find_last_not_of(chars_to_strip.c_str())+1);

    return a;
}

/* ---------------------------------------------------------------------------------------------- */
string stripl(string a)
{
    if (a.length() == 0)
        return a;

    a.erase(0, a.find_first_not_of("\n \t", 0));

    return a;
}

/* ---------------------------------------------------------------------------------------------- */
string stripr(string a)
{
    if (a.length() == 0)
        return a;

    a.erase(a.find_last_not_of("\n \t")+1);

    return a;
}

/* ---------------------------------------------------------------------------------------------- */
bool startsWith(const string &str, const string &start, bool casesensitive)
{
    size_t len = start.size();
    if (str.size() < len) {
        return false;
    }

    if (casesensitive) {
        return str.substr(0, len) == start;
    } else {
        return strcasecmp(str.substr(0, len).c_str(), start.c_str()) == 0;
    }
}

/* ---------------------------------------------------------------------------------------------- */
string getRest(const string &str, const string &prefix)
{
    return str.substr(prefix.size(), str.size() - prefix.size());
}

/* ---------------------------------------------------------------------------------------------- */
char **stringvector_to_array(const vector<string> &vec)
{
    if (vec.size() == 0)
        return NULL;

    /* use malloc to be able to free the result with C free() */
    char **ret = (char **)malloc(sizeof(char *) * (vec.size()+1) );

    char **cur = ret;
    for (vector<string>::const_iterator it = vec.begin(); it != vec.end(); ++it)
        *cur++ = strdup((*it).c_str());
    *cur = NULL;

    return ret;
}

/* ---------------------------------------------------------------------------------------------- */
vector<string> stringsplit(const string &str, const string &pattern)
{
    vector<string> retval;
    string s = str;

    string::size_type pos;
    while ((pos = s.find(pattern)) != string::npos) {
        string item = s.substr(0, pos);
        retval.push_back(item);

        s = s.substr(pos + pattern.length());
    }

    if (s != "") {
        retval.push_back(s);
    }

    return retval;
}

// :tabSize=4:indentSize=4:noTabs=true:mode=c++:folding=explicit:collapseFolds=1:maxLineLen=100:
