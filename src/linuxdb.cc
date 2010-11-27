/*
 * (c) 2009-2010, Bernhard Walle <bernhard@bwalle.de>
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
#include <cstring>

#include <libbw/stringutil.h>

#include "linuxdb.h"

/* LinuxDistDetector {{{ */

/* ---------------------------------------------------------------------------------------------- */
std::string LinuxDistDetector::distTypeToString(DistType type)
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
        case DT_ARCH:
            return "ARCH Linux";
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

    // then Debian
    detector = new DebianDistDetector();
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

    // Fedora, Red Hat or CentOS
    detector = new RedHatDistDetector();
    if (detector->detect()) {
        return detector;
    }
    delete detector;

    // and then ARCH
    detector = new ArchDistDetector();
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
std::string AbstractLinuxDistDetector::getTypeAsString() const
    throw ()
{
    return distTypeToString(getType());
}

/* ---------------------------------------------------------------------------------------------- */
std::string AbstractLinuxDistDetector::getDistribution() const
    throw ()
{
    return m_distribution;
}

/* ---------------------------------------------------------------------------------------------- */
std::string AbstractLinuxDistDetector::getRelease() const
    throw ()
{
    return m_release;
}

/* ---------------------------------------------------------------------------------------------- */
std::string AbstractLinuxDistDetector::getCodename() const
    throw ()
{
    return m_codename;
}

/* ---------------------------------------------------------------------------------------------- */
std::string AbstractLinuxDistDetector::getDescription() const
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
void AbstractLinuxDistDetector::setDistribution(const std::string &distribution)
    throw ()
{
    m_distribution = distribution;
}

/* ---------------------------------------------------------------------------------------------- */
void AbstractLinuxDistDetector::setRelease(const std::string &release)
    throw ()
{
    m_release = release;
}

/* ---------------------------------------------------------------------------------------------- */
void AbstractLinuxDistDetector::setCodename(const std::string &codename)
    throw ()
{
    m_codename = codename;
}

/* ---------------------------------------------------------------------------------------------- */
void AbstractLinuxDistDetector::setDescription(const std::string &description)
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
    const std::string LSB_FILENAME("/etc/lsb-release");
    std::ifstream fin(LSB_FILENAME.c_str());
    if (!fin.is_open()) {
        return false;
    }

    std::string line;
    while (std::getline(fin, line)) {
        // ignore comment lines
        if (bw::startsWith(line, "#"))
            continue;

        std::vector<std::string> keyval = bw::stringsplit(line, "=");
        if (keyval.size() != 2)
            std::cerr << "Invalid line in '"<< LSB_FILENAME << "': " << line << std::endl;
        std::string key = keyval[0];
        std::string value = bw::strip(keyval[1], "\"");

        if (key == "DISTRIB_ID")
            setDistribution(value);
        else if (key == "DISTRIB_RELEASE")
            setRelease(value);
        else if (key == "DISTRIB_CODENAME")
            setCodename(value);
        else if (key == "DISTRIB_DESCRIPTION")
            setDescription(value);
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
    const std::string SUSE_FILENAME("/etc/SuSE-release");
    std::ifstream fin(SUSE_FILENAME.c_str());
    if (!fin.is_open()) {
        return false;
    }

    std::string line;
    bool first_line = true;
    while (std::getline(fin, line)) {
        // ignore comment lines
        if (bw::startsWith(line, "#"))
            continue;

        if (first_line) {
            std::string::size_type first_digit = line.find_first_of("0123456789");
            if (first_digit != std::string::npos && first_digit > 1) {
                std::string first_part = line.substr(0, first_digit);
                first_part = bw::stripr(first_part);
                setDistribution(first_part);
            } else {
                // that's an error
                setDistribution(line);
            }
            setDescription(line);

            first_line = false;
        } else  if (bw::startsWith(line, "VERSION = ")) {
            std::string version = bw::getRest(line, "VERSION = ");
            setRelease(version);
        }
    }

    setType(DT_SUSE);

    return true;
}

/* }}} */
/* RedHatDistDetector {{{ */

/* ---------------------------------------------------------------------------------------------- */
bool RedHatDistDetector::detect()
    throw ()
{
    const std::string REDHAT_FILENAME("/etc/redhat-release");
    std::ifstream fin(REDHAT_FILENAME.c_str());
    if (!fin.is_open()) {
        return false;
    }

    // the file normally has only one line
    std::string line;
    if (!std::getline(fin, line)) {
        return false;
    }

    // search for the 'release' word
    std::string::size_type release_pos = line.find("release");
    if (release_pos == std::string::npos) {
        return false;
    }

    // and search for the '(' and ')'
    std::string::size_type open_bracket_pos = line.find("(");
    std::string::size_type closing_bracket_pos = line.find(")");
    if ((open_bracket_pos == std::string::npos) || (closing_bracket_pos == std::string::npos)) {
        return false;
    }
    if (open_bracket_pos < release_pos) {
        return false;
    }
    if (closing_bracket_pos < open_bracket_pos) {
        return false;
    }

    setDistribution(line.substr(0, release_pos - 1));
    setRelease(line.substr(release_pos + strlen("release") + 1,
                           open_bracket_pos - release_pos - strlen("release") - 2));
    setCodename(line.substr(open_bracket_pos + 1,
                            closing_bracket_pos - open_bracket_pos - 1));
    setDescription(line);

    setType(DT_REDHAT);

    return true;
}

/* }}} */
/* ArchDistDetector {{{ */

/* ---------------------------------------------------------------------------------------------- */
bool ArchDistDetector::detect()
    throw ()
{
    const std::string SUSE_FILENAME("/etc/arch-release");
    std::ifstream fin(SUSE_FILENAME.c_str());
    if (!fin.is_open()) {
        return false;
    }

    setType(DT_ARCH);
    setDistribution("ARCH Linux");
    setDescription("ARCH Linux");

    return true;
}

/* }}} */
/* DebianDistDetector {{{ */

/* ---------------------------------------------------------------------------------------------- */
bool DebianDistDetector::detect()
    throw ()
{
    const std::string DEBIAN_FILENAME("/etc/debian_version");
    std::ifstream fin(DEBIAN_FILENAME.c_str());
    if (!fin.is_open()) {
        return false;
    }

    // the file normally has only one line
    std::string line;
    if (!std::getline(fin, line)) {
        return false;
    }

    setType(DT_DEBIAN);
    setRelease(line);
    setDistribution("Debian");
    setDescription("Debian GNU/Linux " + line);

    return true;
}

/* }}} */

// :tabSize=4:indentSize=4:noTabs=true:mode=c++:folding=explicit:collapseFolds=1:maxLineLen=100:
