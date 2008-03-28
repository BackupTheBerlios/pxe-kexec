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
#include <string>

#include "kexec.h"
#include "process.h"

using std::string;

/* -------------------------------------------------------------------------- */
void Kexec::setKernel(const string &filename)
{
	m_kernel = filename;
}

/* -------------------------------------------------------------------------- */
string Kexec::getKernel() const
{
    return m_kernel;
}

/* -------------------------------------------------------------------------- */
void Kexec::setInitrd(const string &filename)
{
    m_initrd = filename;
}

/* -------------------------------------------------------------------------- */
string Kexec::getInitrd() const
{
    return m_initrd;
}

/* -------------------------------------------------------------------------- */
void Kexec::setAppend(const string &append)
{
    m_append = append;
}

/* -------------------------------------------------------------------------- */
void Kexec::addAppend(const string &append)
{
    m_append += " " + append;
}

/* -------------------------------------------------------------------------- */
string Kexec::getAppend() const
{
    return m_append;
}

/* -------------------------------------------------------------------------- */
bool Kexec::load()
{
    Process p("kexec");

    p.addArg("-l");
    p.addArg(m_kernel);
    p.addArg("--initrd=" + m_initrd);
    p.addArg("--append=" + m_append);

    return p.execute() == 0;
}

/* -------------------------------------------------------------------------- */
bool Kexec::execute()
{
    Process p("kexec");
    
    p.addArg("-e");

    // only returns when != 0 ...
    return p.execute() == 0;
}

// vim: set sw=4 ts=4 fdm=marker et:
