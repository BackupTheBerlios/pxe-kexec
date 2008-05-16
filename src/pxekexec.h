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
#ifndef PXEKEXEC_H
#define PXEKEXEC_H

#include <string>

#include "io.h"
#include "global.h"
#include "pxeparser.h"

class PxeKexec : public Completor {

    public:
        PxeKexec();
        virtual ~PxeKexec();

    public:
        bool parseCmdLine(int argc, char *argv[])
            throw (ApplicationError);
        void readPxeConfig()
            throw (ApplicationError);
        bool checkEnv();
        void displayMessage();
        bool chooseEntry();
        bool confirmBoot();
        void downloadStuff()
            throw (ApplicationError);
        void execute()
            throw (ApplicationError);
        void deleteKernels();
        std::vector<std::string> complete(const std::string &text,
                const std::string &full_text, size_t start_idx, ssize_t end_idx);

    private:
        std::string m_pxeHost;
        std::string m_networkInterface;
        PxeConfig   m_pxeConfig;
        PxeEntry    m_choice;
        std::string m_downloadedKernel;
        std::string m_downloadedInitrd;
        bool        m_noconfirm;
        bool        m_nodelete;
        std::string m_protocol;
        LineReader  *m_lineReader;
        bool        m_dryRun;
};


#endif /* PXEKEXEC_H */

// vim: set sw=4 ts=4 fdm=marker et:
