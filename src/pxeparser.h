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
#ifndef PXEPARSER_H
#define PXEPARSER_H

/**
 * @file pxeparser.h
 * @brief Contains the PXE parser
 *
 * This file contains all classes related to PXE parsing.
 *
 * @author Bernhard Walle <bernhard@bwalle.de>
 */

#include <string>
#include <vector>
#include <iostream>

#include "global.h"

/* PxeEntry {{{ */

/**
 * @brief Entry in the PXE configuration
 *
 * This class represents an entry in a PXE configuration file.
 *
 * @author Bernhard Walle <bernhard@bwalle.de>
 */
class PxeEntry {
    public:
        /**
         * @brief Constructor
         *
         * Creates a new instance of a PxeEntry.
         */
        PxeEntry();

        /**
         * @brief Constructor
         *
         * Creates a new instance of a PxeEntry.
         *
         * @param[in] label the label for the PXE entry
         */
        PxeEntry(const std::string &label);

        /**
         * @brief Destructor
         *
         * Deletes a PxeEntry object.
         */
        virtual ~PxeEntry() {}

    public:
        /**
         * @brief Checks if the PXE entry is valid
         *
         * Checks if the PXE entry is valid, i.e. if it has a label. The
         * default Constructor creates invalid PxeEntry objects while the
         * PxeEntry(const std::string &) constructor creates valid PxeEntry
         * objects.
         *
         * @return @c true if the PxeEntry is valid, @c false otherwise
         */
        bool isValid() const;

        /**
         * @brief Returns the label
         *
         * Returns the label of the PxeEntry.
         *
         * @return the label
         */
        std::string getLabel() const;

        /**
         * @brief Returns the kernel of the PxeEntry
         *
         * Returns the kernel as specified in the PXE configuration.
         *
         * @return the kernel
         */
        std::string getKernel() const;

        /**
         * @brief Sets the kernel of the PxeEntry
         *
         * Sets the kernel of the PxeEntry.
         *
         * @param[in] kernel the kernel of the PXE entry
         */
        void setKernel(const std::string &kernel);

        /**
         * @brief Returns the append line
         *
         * Returns the append line of the PxeEntry.
         *
         * @return the append line
         */
        std::string getAppend() const;

        /**
         * @brief Returns the initrd
         *
         * Returns the initrd (from the append line).
         *
         * @return the initrd or the empty string if there is no
         *         <tt>initrd=</tt> parameter in the append line
         */
        std::string getInitrd();

        /**
         * @brief Sets the append line
         *
         * Sets the append line.
         *
         * @param[in] append the append line
         */
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

/**
 * @brief Represents the PXE configuration
 *
 * This is a whole PXE configuration. A PXE configuration has global
 * parameters like the message or the default entry. And it has a number of
 * PxeEntry entries.
 *
 * @author Bernhard Walle <bernhard@bwalle.de>
 */
class PxeConfig {
    public:
        /**
         * @brief Destructor
         *
         * Deletes a PxeConfig
         */
        virtual ~PxeConfig() {}

    public:
        /**
         * @brief Returns the PXE message
         *
         * Returns the whole PXE message.
         *
         * @return the PXE message
         */
        std::string getMessage() const;

        /**
         * @brief Adds a PXE string
         *
         * This function adds @p msg to the PXE message. In the PXE
         * configuration file, the @c SAY command is responsible for PXE
         * messages. Each @c SAY line results in a call to this function.
         *
         * @param[in] msg the string that should be added
         */
        void addMessage(const std::string &msg);

        /**
         * @brief Returns the default entry
         *
         * Returns the default PXE entry or the empty string if there is no
         * default.
         *
         * @return the default entry
         */
        std::string getDefault() const;

        /**
         * @brief Sets the default PXE entry
         *
         * Sets the default PXE entry to @p def.
         *
         * @param[in] def the default PXE entry string
         */
        void setDefault(const std::string &def);

        /**
         * @brief Add a PXE entry
         *
         * Adds a PxeEntry object to the list of entries.
         *
         * @param[in] entry the entry to add
         */
        void addEntry(PxeEntry entry);

        /**
         * @brief Returns a list of entries
         *
         * Returns a list of PXE entries.
         *
         * @return the vector of entries
         */
        std::vector<PxeEntry> getEntries() const;

        /**
         * @brief Returns a list of entry names
         *
         * Returns a vector of entry names.
         *
         * @return the list
         */
        std::vector<std::string> getEntryNames() const;

        /**
         * @brief Returns a named entry object
         *
         * Searches the entry with label @p label and returns the object on
         * success. Returns a invalid PxeEntry object on failure.
         *
         * @return a PxeEntry object
         */
        PxeEntry getEntry(const std::string &label) const;

    private:
        std::string m_message;
        std::string m_default;
        std::string m_entry;
        std::vector<PxeEntry> m_entries;
};

/* }}} */
/* PxeParser {{{ */

/**
 * @brief PXE configuration file parser
 *
 * Parses a PXE configuration file.
 *
 * @author Bernhard Walle <bernhard@bwalle.de>
 */
class PxeParser {
    public:
        /**
         * @brief Internal parser state
         */
        enum ParserState {
            PS_GLOBAL,          /**< global section */
            PS_ENTRY            /**< entries section */
        };

        /**
         * @brief Constructor
         *
         * Creates a new PxeParser.
         */
        PxeParser();

        /**
         * @brief Destructor
         *
         * Deletes PxeParser objects.
         */
        virtual ~PxeParser() {}

    public:
        /**
         * @brief Feeds the parser with a line
         *
         * Feeds the parser with @p line.
         *
         * @param[in] line the input line
         * @exception ParseError if parsing of @p line failed
         */
        void feedLine(std::string line)
            throw (ParseError);

        /**
         * @brief Finishes parsing
         *
         * Tells the parser that we're now done.
         */
        void finishParsing();

        /**
         * @brief Parses a stream
         *
         * Parses a complete stream, repeatedly calling feedLine().
         *
         * @param[in] stream the stream that should be parsed
         * @exception ParseError on a parser error
         */
        void parseStream(std::istream &stream)
            throw (ParseError);

        /**
         * @brief Returns the PXE config
         *
         * If parsing has been finished, returns the completely parsed PXE
         * configuration.
         *
         * @return the parsed PxeConfig or a invalid PxeConfig if parsing
         *         failed
         */
        PxeConfig getConfig() const;

    private:
        PxeConfig m_config;
        PxeEntry m_currentEntry;
        ParserState m_state;
};

/* }}} */

#endif /* PXEPARSER_H */

// :tabSize=4:indentSize=4:noTabs=true:mode=c++:folding=explicit:collapseFolds=1:maxLineLen=100:
