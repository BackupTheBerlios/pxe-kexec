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
#ifndef KEXEC_H
#define KEXEC_H

class Kexec {

    public:
        void setKernel(const std::string &filename);
        std::string getKernel() const;

        void setInitrd(const std::string &filename);
        std::string getInitrd() const;

        void setAppend(const std::string &filename);
        void addAppend(const std::string &filename);
        std::string getAppend() const;

        bool load();
        bool prepareConsole();
        bool execute();

    private:
        std::string m_kernel;
        std::string m_initrd;
        std::string m_append;
};


#endif /* KEXEC_H */

// vim: set sw=4 ts=4 fdm=marker et:
