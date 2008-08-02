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
#ifndef PXEPARSER_H
#define PXEPARSER_H

#include <string>
#include <vector>
#include <iostream>

#include "global.h"

/* PxeEntry {{{ */

class PxeEntry {
    public:
        PxeEntry();
        PxeEntry(const std::string &label);
        virtual ~PxeEntry() {}

    public:
        bool isValid() const;

        std::string getLabel() const;

        std::string getKernel() const;
        void setKernel(const std::string &kernel);

        std::string getAppend() const;
        std::string getInitrd(); // initrd is in append line
        void setAppend(const std::string &append);

    private:
        bool m_valid;
        std::string m_label;
        std::string m_kernel;
        std::string m_initrd;
        std::string m_append;
        bool m_initrdParsed;
};

/* }}} */
/* PxeConfig {{{ */

class PxeConfig {
    public:
        virtual ~PxeConfig() {}

    public:
        std::string getMessage() const;
        void addMessage(const std::string &msg);

        std::string getDefault() const;
        void setDefault(const std::string &def);

        void addEntry(PxeEntry entry);
        std::vector<PxeEntry> getEntries() const;
        std::vector<std::string> getEntryNames() const;
        PxeEntry getEntry(const std::string &label) const;

    private:
        std::string m_message;
        std::string m_default;
        std::string m_entry;
        std::vector<PxeEntry> m_entries;
};

/* }}} */
/* PxeParser {{{ */

class PxeParser {
    public:
        enum ParserState {
            PS_GLOBAL,
            PS_ENTRY
        };
        PxeParser();
        virtual ~PxeParser() {}

    public:
        void feedLine(std::string line)
            throw (ParseError);
        void finishParsing();
        void parseStream(std::istream &stream)
            throw (ParseError);

        PxeConfig getConfig() const;

    private:
        PxeConfig m_config;
        PxeEntry m_currentEntry;
        ParserState m_state;
};

/* }}} */

#endif /* PXEPARSER_H */

// vim: set sw=4 ts=4 fdm=marker et:
