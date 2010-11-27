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
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <cstdlib>

#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <libbw/debug.h>
#include <libbw/stringutil.h>

#include "process.h"

/* Process {{{ */

/* -------------------------------------------------------------------------- */
bool Process::m_dryRunMode = false;

/* -------------------------------------------------------------------------- */
void Process::enableDryRunMode()
    throw ()
{
    BW_DEBUG_DBG("enableDryRunMode");
    m_dryRunMode = true;
}

/* -------------------------------------------------------------------------- */
void Process::disableDryRunMode()
    throw ()
{
    BW_DEBUG_DBG("disableDryRunMode");
    m_dryRunMode = false;
}

/* -------------------------------------------------------------------------- */
bool Process::isInPath(const std::string &program)
{
    const char *path = getenv("PATH");
    if (!path) {
        return false;
    }

    std::vector<std::string> paths = bw::stringsplit(path, ":");
    for (std::vector<std::string>::const_iterator it = paths.begin();
            it != paths.end(); ++it) {
        std::string current_path = *it;
        std::string binary = current_path + "/" + program;

        BW_DEBUG_DBG("Checking for program '%s'\n", binary.c_str());

        // check if executable
        if (access(binary.c_str(), X_OK)) {
            return true;
        }
    }

    return false;
}

/* -------------------------------------------------------------------------- */
Process::Process(const std::string &name)
{
    m_name = name;
}

/* -------------------------------------------------------------------------- */
void Process::addArg(const std::string &arg)
{
    m_args.push_back(arg);
}

/* -------------------------------------------------------------------------- */
std::vector<std::string> Process::getArgs() const
{
    return m_args;
}

/* -------------------------------------------------------------------------- */
int Process::execute()
{
    char **vector = NULL;
    int returncode = -1;

    vector = new char*[m_args.size() + 2];

    // fill vector
    vector[0] = strdup(m_name.c_str());
    for (unsigned int i = 1; i <= m_args.size(); i++)
        vector[i] = strdup(m_args[i-1].c_str());
    vector[m_args.size()+1] = NULL;

    // debug string
    std::stringstream ss;
    ss << "Executing " << m_name << " ";
    for (unsigned int i = 0; i < m_args.size(); i++) {
        ss << "'" << m_args[i] << "' ";
    }
    BW_DEBUG_DBG(ss.str().c_str());

    if (m_dryRunMode) {
        std::cerr << "(dry run) " << ss.str() << std::endl;
        return 0;
    } else {
        // now fork and exec
        pid_t pid = fork();
        if (pid == 0) {
            // child, should run exec
            execvp(m_name.c_str(), vector);
            exit(-1);
        } else if (pid > 0) {
            int status;
            pid_t ret = waitpid(pid, &status, 0);
            if (ret < 0) {
                perror("Process::execute(): waitpid() failed");
                returncode = -1;
            }
            returncode = WEXITSTATUS(status);
        } else
            perror("Process::execute(): fork() failed");

        // free allocated C-strings
        for (unsigned int i = 0; i < m_args.size() + 2; i++)
            free(vector[i]);

        delete[] vector;

        return returncode;
    }
}

/* }}} */

// :tabSize=4:indentSize=4:noTabs=true:mode=c++:folding=explicit:collapseFolds=1:maxLineLen=100:
