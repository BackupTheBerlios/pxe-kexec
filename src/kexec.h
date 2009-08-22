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
#ifndef KEXEC_H
#define KEXEC_H

/**
 * @file kexec.h
 * @brief Kexec implementation
 *
 * This file contains an interface to @c kexec.
 *
 * @author Bernhard Walle <bernhard@bwalle.de>
 */

/**
 * @brief Interface to @c kexec
 *
 * This class is the interface to @c kexec.
 *
 * @author Bernhard Walle <bernhard@bwalle.de>
 */
class Kexec {

    public:
        /**
         * @brief Sets the kernel
         *
         * Sets the kernel that should be kexec'd.
         *
         * @param[in] filename the kernel image
         */
        void setKernel(const std::string &filename);

        /**
         * @brief Returns the kernel
         *
         * Returns the path to the kernel image.
         *
         * @return the kernel image
         */
        std::string getKernel() const;

        /**
         * @brief Sets the initrd
         *
         * Sets the initrd that should be kexec'd.
         *
         * @param[in] filename the path to the initrd
         */
        void setInitrd(const std::string &filename);

        /**
         * @brief Returns the initrd
         *
         * Returns the path to the initrd.
         *
         * @return the path to the initrd
         */
        std::string getInitrd() const;

        /**
         * @brief Sets the append line
         *
         * Sets the kernel parameters.
         *
         * @param[in] filename the append line
         */
        void setAppend(const std::string &filename);

        /**
         * @brief Adds something to the append line
         *
         * Adds something to the append line (that is empty or has been set
         * with setAppend() previously).
         *
         * @param[in] filename the string that should be added to the append
         *            line
         */
        void addAppend(const std::string &filename);

        /**
         * @brief Returns the append line
         *
         * Returns the current append line.
         *
         * @return the append line
         */
        std::string getAppend() const;

        /**
         * @brief Loads the kernel
         *
         * Loads the kernel that has been specified with setKernel(), together
         * with the initrd that has been set with setInitrd() and the kernel
         * parameters (see setAppend() and addAppend()).
         *
         * @return @c true on success, @c false on failure
         */
        bool load();

        /**
         * @brief Prepares the console for kexec
         *
         * Switches to another virtual terminal. Waits until that switch has
         * been performed.
         *
         * @return @c true on success, @c false on failure
         */
        bool prepareConsole();

        /**
         * @brief Execute
         *
         * This function does the kexec. It never returns on success, it only
         * returns on failure.
         *
         * @return @c false on failure (does not return on success)
         */
        bool execute();

    private:
        std::string m_kernel;
        std::string m_initrd;
        std::string m_append;
};


#endif /* KEXEC_H */

// vim: set sw=4 ts=4 fdm=marker et:
