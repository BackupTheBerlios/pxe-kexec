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
#ifndef PROCESS_H
#define PROCESS_H

#include <string>
#include <vector>

class Process {

    public:
        Process(const std::string &name);
        virtual ~Process() { }

    public:
        void addArg(const std::string &arg);
        std::vector<std::string> getArgs() const;

        int execute();

    private:
        std::string m_name;
        std::vector<std::string> m_args;
};

#endif /* PROCESS_H */

// vim: set sw=4 ts=4 fdm=marker et:
