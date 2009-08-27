/*
 * (c) 2008-2009, Bernhard Walle <bernhard@bwalle.de>
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
#include <iostream>

#include "kexec.h"
#include "process.h"
#include "console.h"
#include "global.h"

using std::string;
using std::cerr;
using std::endl;

/* ---------------------------------------------------------------------------------------------- */
void Kexec::setKernel(const string &filename)
{
	m_kernel = filename;
}

/* ---------------------------------------------------------------------------------------------- */
string Kexec::getKernel() const
{
    return m_kernel;
}

/* ---------------------------------------------------------------------------------------------- */
void Kexec::setInitrd(const string &filename)
{
    m_initrd = filename;
}

/* ---------------------------------------------------------------------------------------------- */
string Kexec::getInitrd() const
{
    return m_initrd;
}

/* ---------------------------------------------------------------------------------------------- */
void Kexec::setAppend(const string &append)
{
    m_append = append;
}

/* ---------------------------------------------------------------------------------------------- */
void Kexec::addAppend(const string &append)
{
    m_append += " " + append;
}

/* ---------------------------------------------------------------------------------------------- */
string Kexec::getAppend() const
{
    return m_append;
}

/* ---------------------------------------------------------------------------------------------- */
bool Kexec::reboot() const
{
    Process p("reboot");
    return p.execute() == 0;
}

/* ---------------------------------------------------------------------------------------------- */
bool Kexec::load()
{
    Process p("kexec");

    p.addArg("-l");
    p.addArg(m_kernel);
    p.addArg("--initrd=" + m_initrd);
    p.addArg("--append=" + m_append);

    return p.execute() == 0;
}

/* ---------------------------------------------------------------------------------------------- */
bool Kexec::execute()
{
    Process p("kexec");

    p.addArg("-e");

    // only returns when != 0 ...
    return p.execute() == 0;
}

/* ---------------------------------------------------------------------------------------------- */
bool Kexec::prepareConsole()
{
    try {
        if (!Console::isRealTerminal())
            Console::changeVirtualTerminal(1, true);
    } catch (const ApplicationError &e) {
        cerr << "Failed to change virtual terminal: " << e.what() << endl;
        return false;
    }

    return true;
}

// :tabSize=4:indentSize=4:noTabs=true:mode=c++:folding=explicit:collapseFolds=1:maxLineLen=100:
