/*
 * (c) 2009, Bernhard Walle <bernhard@bwalle.de>
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
#include <iostream>
#include <fstream>
#include <vector>

#include "linuxdb.h"
#include "stringutil.h"

using std::cerr;
using std::endl;
using std::string;
using std::vector;
using std::ifstream;

/* LinuxDistDetector {{{ */

/* ---------------------------------------------------------------------------------------------- */
string LinuxDistDetector::distTypeToString(DistType type)
{
    switch (type) {
        case DT_UNKNOWN:
            return "Unknown";
        case DT_UBUNTU:
            return "Ubuntu";
        case DT_SUSE:
            return "SUSE";
        case DT_DEBIAN:
            return "Debian";
        case DT_REDHAT:
            return "Red Hat";
        default:
            return "(invalid)";
    }
}

/* ---------------------------------------------------------------------------------------------- */
LinuxDistDetector *LinuxDistDetector::getDetector()
    throw ()
{
    LinuxDistDetector *detector;

    // try the LSB one first
    detector = new LSBLinuxDistDetector();
    if (detector->detect()) {
        return detector;
    }
    delete detector;

    // and then SUSE
    detector = new SUSELinuxDistDetector();
    if (detector->detect()) {
        return detector;
    }
    delete detector;

    return NULL;
}

/* }}} */
/* AbstractLinuxDistDetector {{{ */

/* ---------------------------------------------------------------------------------------------- */
AbstractLinuxDistDetector::AbstractLinuxDistDetector()
    throw ()
    : m_type(DT_UNKNOWN)
{
}

/* ---------------------------------------------------------------------------------------------- */
LinuxDistDetector::DistType AbstractLinuxDistDetector::getType() const
    throw ()
{
    return m_type;
}

/* ---------------------------------------------------------------------------------------------- */
string AbstractLinuxDistDetector::getTypeAsString() const
    throw ()
{
    return distTypeToString(getType());
}

/* ---------------------------------------------------------------------------------------------- */
string AbstractLinuxDistDetector::getDistribution() const
    throw ()
{
    return m_distribution;
}

/* ---------------------------------------------------------------------------------------------- */
string AbstractLinuxDistDetector::getRelease() const
    throw ()
{
    return m_release;
}

/* ---------------------------------------------------------------------------------------------- */
string AbstractLinuxDistDetector::getCodename() const
    throw ()
{
    return m_codename;
}

/* ---------------------------------------------------------------------------------------------- */
string AbstractLinuxDistDetector::getDescription() const
    throw ()
{
    return m_description;
}

/* ---------------------------------------------------------------------------------------------- */
void AbstractLinuxDistDetector::setType(LinuxDistDetector::DistType type)
    throw ()
{
    m_type = type;
}

/* ---------------------------------------------------------------------------------------------- */
void AbstractLinuxDistDetector::setDistribution(const string &distribution)
    throw ()
{
    m_distribution = distribution;
}

/* ---------------------------------------------------------------------------------------------- */
void AbstractLinuxDistDetector::setRelease(const string &release)
    throw ()
{
    m_release = release;
}

/* ---------------------------------------------------------------------------------------------- */
void AbstractLinuxDistDetector::setCodename(const string &codename)
    throw ()
{
    m_codename = codename;
}

/* ---------------------------------------------------------------------------------------------- */
void AbstractLinuxDistDetector::setDescription(const string &description)
    throw ()
{
    m_description = description;
}

/* }}} */
/* LSBLinuxDistDetector {{{ */

/* ---------------------------------------------------------------------------------------------- */
bool LSBLinuxDistDetector::detect()
    throw ()
{
    const string LSB_FILENAME("/etc/lsb-release");
    ifstream fin(LSB_FILENAME.c_str());
    if (!fin.is_open()) {
        return false;
    }

    string line;
    while (getline(fin, line)) {
        // ignore comment lines
        if (startsWith(line, "#")) {
            continue;
        }

        vector<string> keyval = stringsplit(line, "=");
        if (keyval.size() != 2) {
            cerr << "Invalid line in '"<< LSB_FILENAME << "': " << line << endl;
        }
        string key = keyval[0];
        string value = strip(keyval[1], "\"");

        if (key == "DISTRIB_ID") {
            setDistribution(value);
        } else if (key == "DISTRIB_RELEASE") {
            setRelease(value);
        } else if (key == "DISTRIB_CODENAME") {
            setCodename(value);
        } else if (key == "DISTRIB_DESCRIPTION") {
            setDescription(value);
        }
    }

    // set the type
    if (getDistribution() == "Ubuntu") {
        setType(DT_UBUNTU);
    }

    return true;
}

/* }}} */
/* SUSELinuxDistDetector {{{ */

/* ---------------------------------------------------------------------------------------------- */
bool SUSELinuxDistDetector::detect()
    throw ()
{
    const string SUSE_FILENAME("/etc/SuSE-release");
    ifstream fin(SUSE_FILENAME.c_str());
    if (!fin.is_open()) {
        return false;
    }

    string line;
    bool first_line = true;
    while (getline(fin, line)) {
        // ignore comment lines
        if (startsWith(line, "#")) {
            continue;
        }

        if (first_line) {
            string::size_type first_digit = line.find_first_of("0123456789");
            if (first_digit != string::npos && first_digit > 1) {
                string first_part = line.substr(0, first_digit);
                first_part = stripr(first_part);
                setDistribution(first_part);
            } else {
                // that's an error
                setDistribution(line);
            }
            setDescription(line);

            first_line = false;
        } else  if (startsWith(line, "VERSION = ")) {
            string version = getRest(line, "VERSION = ");
            setRelease(version);
        }
    }

    setType(DT_SUSE);

    return true;
}

/* }}} */

// :tabSize=4:indentSize=4:noTabs=true:mode=c++:folding=explicit:collapseFolds=1:maxLineLen=100:
