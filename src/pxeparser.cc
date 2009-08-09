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
#include <string>
#include <algorithm>

#include <strings.h>

#include "stringutil.h"
#include "pxeparser.h"
#include "debug.h"

using std::string;
using std::vector;
using std::ptr_fun;
using std::find_if;
using std::istream;

/* PxeEntry {{{ */

/* -------------------------------------------------------------------------- */
PxeEntry::PxeEntry()
    : m_valid(false)
    , m_initrdParsed(false)
{}

/* -------------------------------------------------------------------------- */
PxeEntry::PxeEntry(const std::string &label)
    : m_valid(true)
    , m_label(label)
    , m_initrdParsed(false)
{}

/* -------------------------------------------------------------------------- */
bool PxeEntry::isValid() const
{
    return m_valid;
}

/* -------------------------------------------------------------------------- */
string PxeEntry::getLabel() const
{
    return m_label;
}

/* -------------------------------------------------------------------------- */
string PxeEntry::getKernel() const
{
    return m_kernel;
}

/* -------------------------------------------------------------------------- */
void PxeEntry::setKernel(const std::string &kernel)
{
    m_kernel = kernel;
}

/* -------------------------------------------------------------------------- */
string PxeEntry::getInitrd()
{
    // try to find out the initrd from the append line
    if (!m_initrdParsed) {
        string::size_type pos = m_append.find("initrd=");
        if (pos != string::npos) {
            string rest = m_append.substr(pos);
            string::size_type space = rest.find(" ");
            if (space != string::npos)
                rest = rest.substr(0, space);

            m_initrd = getRest(rest, "initrd=");
        }

        m_initrdParsed = true;
    }

    return m_initrd;
}

/* -------------------------------------------------------------------------- */
string PxeEntry::getAppend() const
{
    return m_append;
}

/* -------------------------------------------------------------------------- */
void PxeEntry::setAppend(const std::string &append)
{
    m_append = append;
    m_initrdParsed = false;
}

/* }}} */
/* PxeConfig {{{ */

/* -------------------------------------------------------------------------- */
string PxeConfig::getMessage() const
{
    return m_message;
}

/* -------------------------------------------------------------------------- */
void PxeConfig::addMessage(const string &msg)
{
    m_message += "\n" + msg;
}

/* -------------------------------------------------------------------------- */
string PxeConfig::getDefault() const
{
    return m_default;
}

/* -------------------------------------------------------------------------- */
void PxeConfig::setDefault(const string &def)
{
    m_default = def;
}

/* -------------------------------------------------------------------------- */
void PxeConfig::addEntry(PxeEntry entry)
{
    m_entries.push_back(entry);
}

/* -------------------------------------------------------------------------- */
vector<PxeEntry> PxeConfig::getEntries() const
{
    return m_entries;
}

/* -------------------------------------------------------------------------- */
vector<string> PxeConfig::getEntryNames() const
{
    vector<string> names;

    for (vector<PxeEntry>::const_iterator it = m_entries.begin();
            it != m_entries.end(); ++it) {
        names.push_back(it->getLabel());
    }

    return names;
}

/* -------------------------------------------------------------------------- */
bool compare_entry_name(PxeEntry entry, string label)
{
    return strcasecmp(entry.getLabel().c_str(), label.c_str()) == 0;
}

/* -------------------------------------------------------------------------- */
PxeEntry PxeConfig::getEntry(const string &label) const
{
    vector<PxeEntry>::const_iterator it =
        find_if(m_entries.begin(), m_entries.end(),
                bind2nd(ptr_fun(compare_entry_name), label));

    return it != m_entries.end() ? *it : PxeEntry();
}

/* }}} */
/* PxeParser {{{ */

/* -------------------------------------------------------------------------- */
PxeParser::PxeParser()
    : m_state(PS_GLOBAL)
{}

/* -------------------------------------------------------------------------- */
void PxeParser::feedLine(string line)
    throw (ParseError)
{
    line = strip(line);

    // skip comments and empty lines
    if (line.size() == 0 || line[0] == '#')
        return;

    Debug::debug()->dbg("Line: %s (label:%d)\n", line.c_str(), startsWith(line, "label ", false) );

    switch (m_state) {
        case PS_GLOBAL:
            // parse "say"
            if (startsWith(line, "say ", false) ||
                    startsWith(line, "say\t", false)) {
                m_config.addMessage(stripr(getRest(line, "say")));
                return;
            }

            // parse "label"
            if (startsWith(line, "label ", false) ||
                    startsWith(line, "label\t", false)) {
                m_currentEntry = PxeEntry(strip(getRest(line, "label")));
                m_state = PS_ENTRY;
                return;
            }

            break;

        case PS_ENTRY:
            // parse "label"
            if (startsWith(line, "label ", false)) {
                Debug::debug()->trace("Adding entry with label=%s, "
                        "kernel=%s, initrd=%s, append=%s",
                        m_currentEntry.getLabel().c_str(),
                        m_currentEntry.getKernel().c_str(),
                        m_currentEntry.getInitrd().c_str(),
                        m_currentEntry.getAppend().c_str());
                m_config.addEntry(m_currentEntry);
                m_currentEntry = PxeEntry();
                m_state = PS_GLOBAL;
                feedLine(line);
                return;
            }

            // parse "kernel"
            if (startsWith(line, "kernel ", false)) {
                m_currentEntry.setKernel(strip(getRest(line, "kernel")));
                return;
            }

            // parse "append"
            if (startsWith(line, "append ", false)) {
                m_currentEntry.setAppend(strip(getRest(line, "append")));
                return;
            }

            break;
    }
}

/* -------------------------------------------------------------------------- */
void PxeParser::parseStream(istream &stream)
    throw (ParseError)
{
    string line;

    while (getline(stream, line))
        feedLine(line);

    finishParsing();
}

/* -------------------------------------------------------------------------- */
void PxeParser::finishParsing()
{
    if (m_currentEntry.isValid())
        m_config.addEntry(m_currentEntry);
}

/* -------------------------------------------------------------------------- */
PxeConfig PxeParser::getConfig() const
{
    return m_config;
}

/* }}} */

// vim: set sw=4 ts=4 fdm=marker et:

