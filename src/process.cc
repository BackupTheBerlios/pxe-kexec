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
#include <vector>
#include <string>
#include <sstream>
#include <cstdlib>

#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "process.h"
#include "debug.h"
#include "stringutil.h"

using std::string;
using std::exit;
using std::vector;
using std::stringstream;

/* -------------------------------------------------------------------------- */
bool Process::isInPath(const string &program)
{
    const char *path = getenv("PATH");
    if (!path) {
        return false;
    }

    vector<string> paths = stringsplit(path, ":");
    for (vector<string>::const_iterator it = paths.begin();
            it != paths.end(); ++it) {
        string current_path = *it;
        string binary = current_path + "/" + program;

        Debug::debug()->dbg("Checking for program '%s'\n", binary.c_str());

        // check if executable
        if (access(binary.c_str(), X_OK)) {
            return true;
        }
    }

    return false;
}

/* -------------------------------------------------------------------------- */
Process::Process(const string &name)
{
    m_name = name;
}

/* -------------------------------------------------------------------------- */
void Process::addArg(const std::string &arg)
{
    m_args.push_back(arg);
}

/* -------------------------------------------------------------------------- */
vector<string> Process::getArgs() const
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
    stringstream ss;
    ss << "Executing " << m_name << "[";
    for (unsigned int i = 0; i < m_args.size(); i++) {
        ss << "'" << m_args[i] << "'";
        if (i != m_args.size() - 1)
            ss << ", ";
    }
    ss << "]";
    Debug::debug()->dbg(ss.str().c_str());

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


// vim: set sw=4 ts=4 fdm=marker et:
