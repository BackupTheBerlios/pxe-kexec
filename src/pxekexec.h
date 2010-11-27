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
#ifndef PXEKEXEC_H
#define PXEKEXEC_H

/**
 * @file pxekexec.h
 * @brief Main class
 *
 * This file contains the main class which performs all tasks and is called
 * from main().
 *
 * @author Bernhard Walle <bernhard@bwalle.de>
 */

#include <string>

#include <libbw/completion.h>
#include "global.h"
#include "pxeparser.h"

/* PxeKexec {{{ */

/**
 * @brief Main class
 *
 * This is the main class of the program. It contains all functions needed and
 * is called from main().
 *
 * @author Bernhard Walle <bernhard@bwalle.de>
 */
class PxeKexec : public bw::Completor {

    public:
        /**
         * @brief Constructor
         *
         * Creates a new instance of PxeKexec().
         */
        PxeKexec();

        /**
         * @brief Destructor
         *
         * Destroys PxeKexec().
         */
        virtual ~PxeKexec();

    public:
        /**
         * @brief Parses the command line
         *
         * This function is called from main() to parse the program's command line.
         * This function does not modify the contents of @p argv.
         *
         * @param[in] argc the number of arguments
         * @param[in] argv the arguments
         * @return @c true if the application should continue, @c false if the program
         *         should be terminated with @c EXIT_SUCCESS
         * @throw ApplicationError if an error occured
         */
        bool parseCmdLine(int argc, char *argv[])
            throw (ApplicationError);

        /**
         * @brief Read the PXE configuration
         *
         * This function needs to be called after parseCmdLine() and it downloads
         * and reads the PXE configuration.
         *
         * @throw ApplicationError if any error occurred
         */
        void readPxeConfig()
            throw (ApplicationError);

        /**
         * @brief Checks the environment
         *
         * Checks if the environment is sane, especially if @c kexec is there.
         *
         * @return @c true if the environment is ok, @c false otherwise
         */
        bool checkEnv();

        /**
         * @brief Displays the PXE message
         *
         * Displays all messages that are specified with the @c SAY command in
         * the PXE configuration. Needs to be called after readPxeConfig().
         */
        void displayMessage();

        /**
         * @brief Displays a prompt and chooses the entry
         *
         * This method is used to choose the entry for the PXE boot. Normally
         * it is called after displayMessage(), but it can also be called
         * after readPxeConfig().
         *
         * @return @c true if the user has chosen a valid entry, @c false
         * otherwise
         */
        bool chooseEntry();

        /**
         * @brief Asks the user to confirm if we should boot
         *
         * Asks the user to confirm if we really should continue.
         *
         * @return @c true if the user wants to continue, @c false otherwise
         */
        bool confirmBoot();

        /**
         * @brief Download kernel and initrd
         *
         * Called after confirmBoot(), downloads kernel and initrd needed for
         * the next step.
         *
         * @throw ApplicationError if downloading failed
         */
        void downloadStuff()
            throw (ApplicationError);

        /**
         * @brief Executes kexec
         *
         * Runs kexec. Has to be called after downloadStuff().
         *
         * @throw ApplicationError if something failed
         */
        void execute()
            throw (ApplicationError);

        /**
         * @brief Delete kernel and initrd
         *
         * This function has to be called after loading kernel and initrd.
         * If deletion failed, only a debugging message is printed.
         */
        void deleteKernels();

        /**
         * @brief Complete
         *
         * This is the completor needed in chooseEntry().
         *
         * @todo better document
         * @param[in] text the current text
         * @param[in] full_text the full text
         * @param[in] start_idx the start index
         * @param[in] end_idx the end index
         * @return the list of possible compltions
         */
        std::vector<std::string> complete(const std::string     &text,
                                          const std::string     &full_text,
                                          size_t                start_idx,
                                          ssize_t               end_idx);

        /**
         * @brief Checks if we should only print the Linux distribution and exit
         *
         * @return the value of the detectDistOnly flag
         */
        bool getPrintLinuxDistributionOnly() const;

        /**
         * @brief Prints the Linux distribution
         */
        void printLinuxDistribution();

        /**
         * @brief Prints the version to stdout
         */
        void printVersion();

    private:
        std::string    m_pxeHost;
        std::string    m_networkInterface;
        PxeConfig      m_pxeConfig;
        PxeEntry       m_choice;
        std::string    m_downloadedKernel;
        std::string    m_downloadedInitrd;
        bool           m_noconfirm;
        bool           m_nodelete;
        bool           m_quiet;
        std::string    m_preChoice;
        std::string    m_protocol;
        bw::LineReader *m_lineReader;
        bool           m_dryRun;
        bool           m_force;
        bool           m_ignoreWhitelist;
        bool           m_detectDistOnly;
        bool           m_loadOnly;
};

/* }}} */

#endif /* PXEKEXEC_H */

// :tabSize=4:indentSize=4:noTabs=true:mode=c++:folding=explicit:collapseFolds=1:maxLineLen=100:
