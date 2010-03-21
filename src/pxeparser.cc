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
#include <string>
#include <algorithm>

#include <strings.h>

#include "stringutil.h"
#include "pxeparser.h"
#include "debug.h"

/* PxeEntry {{{ */

/* ---------------------------------------------------------------------------------------------- */
PxeEntry::PxeEntry()
    : m_valid(false)
    , m_initrdParsed(false)
{}

/* ---------------------------------------------------------------------------------------------- */
PxeEntry::PxeEntry(const std::string &label)
    : m_valid(true)
    , m_label(label)
    , m_initrdParsed(false)
{}

/* ---------------------------------------------------------------------------------------------- */
bool PxeEntry::isValid() const
{
    return m_valid;
}

/* ---------------------------------------------------------------------------------------------- */
std::string PxeEntry::getLabel() const
{
    return m_label;
}

/* ---------------------------------------------------------------------------------------------- */
std::string PxeEntry::getKernel() const
{
    return m_kernel;
}

/* ---------------------------------------------------------------------------------------------- */
void PxeEntry::setKernel(const std::string &kernel)
{
    m_kernel = kernel;
}

/* ---------------------------------------------------------------------------------------------- */
std::string PxeEntry::getInitrd()
{
    // try to find out the initrd from the append line
    if (!m_initrdParsed) {
        std::string::size_type pos = m_append.find("initrd=");
        if (pos != std::string::npos) {
            std::string rest = m_append.substr(pos);
            std::string::size_type space = rest.find(" ");
            if (space != std::string::npos)
                rest = rest.substr(0, space);

            m_initrd = getRest(rest, "initrd=");
        }

        m_initrdParsed = true;
    }

    return m_initrd;
}

/* ---------------------------------------------------------------------------------------------- */
std::string PxeEntry::getAppend() const
{
    return m_append;
}

/* ---------------------------------------------------------------------------------------------- */
void PxeEntry::setAppend(const std::string &append)
{
    m_append = append;
    m_initrdParsed = false;
}

/* }}} */
/* PxeConfig {{{ */

/* ---------------------------------------------------------------------------------------------- */
std::string PxeConfig::getMessage() const
{
    return m_message;
}

/* ---------------------------------------------------------------------------------------------- */
void PxeConfig::addMessage(const std::string &msg)
{
    m_message += "\n" + msg;
}

/* ---------------------------------------------------------------------------------------------- */
std::string PxeConfig::getDefault() const
{
    return m_default;
}

/* ---------------------------------------------------------------------------------------------- */
void PxeConfig::setDefault(const std::string &def)
{
    m_default = def;
}

/* ---------------------------------------------------------------------------------------------- */
void PxeConfig::addEntry(PxeEntry entry)
{
    m_entries.push_back(entry);
}

/* ---------------------------------------------------------------------------------------------- */
std::vector<PxeEntry> PxeConfig::getEntries() const
{
    return m_entries;
}

/* ---------------------------------------------------------------------------------------------- */
std::vector<std::string> PxeConfig::getEntryNames() const
{
    std::vector<std::string> names;

    for (std::vector<PxeEntry>::const_iterator it = m_entries.begin();
            it != m_entries.end(); ++it) {
        names.push_back(it->getLabel());
    }

    return names;
}

/* ---------------------------------------------------------------------------------------------- */
bool compare_entry_name(PxeEntry entry, std::string label)
{
    return strcasecmp(entry.getLabel().c_str(), label.c_str()) == 0;
}

/* ---------------------------------------------------------------------------------------------- */
PxeEntry PxeConfig::getEntry(const std::string &label) const
{
    std::vector<PxeEntry>::const_iterator it =
        find_if(m_entries.begin(), m_entries.end(),
                std::bind2nd(std::ptr_fun(compare_entry_name), label));

    return it != m_entries.end() ? *it : PxeEntry();
}

/* }}} */
/* PxeParser {{{ */

/* ---------------------------------------------------------------------------------------------- */
PxeParser::PxeParser()
    : m_state(PS_GLOBAL)
{}

/* ---------------------------------------------------------------------------------------------- */
void PxeParser::feedLine(std::string line)
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

/* ---------------------------------------------------------------------------------------------- */
void PxeParser::parseStream(std::istream &stream)
    throw (ParseError)
{
    std::string line;

    while (std::getline(stream, line))
        feedLine(line);

    finishParsing();
}

/* ---------------------------------------------------------------------------------------------- */
void PxeParser::finishParsing()
{
    if (m_currentEntry.isValid())
        m_config.addEntry(m_currentEntry);
}

/* ---------------------------------------------------------------------------------------------- */
PxeConfig PxeParser::getConfig() const
{
    return m_config;
}

/* }}} */


// :tabSize=4:indentSize=4:noTabs=true:mode=c++:folding=explicit:collapseFolds=1:maxLineLen=100:
