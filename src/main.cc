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

/**
 * @file main.cc
 * @brief Entry-point
 *
 * Defines the entry point of the application.
 *
 * @author Bernhard Walle <bernhard@bwalle.de>
 */

#include <iostream>
#include <string>
#include <stdexcept>
#include <cstdlib>
#include <cstdio>
#include <clocale>

#include "process.h"
#include "pxekexec.h"
#include "networkhelper.h"

/**
 * @brief The entry point of the application.
 *
 * This is the main() function.
 *
 * @param[in] argc the argument number
 * @param[in] argv the arguments to the program
 */
int main(int argc, char *argv[])
{
    setlocale(LC_ALL, "");

    try {
        PxeKexec pe;

        if (!pe.parseCmdLine(argc, argv))
            return EXIT_SUCCESS;

        if (pe.getPrintLinuxDistributionOnly()) {
            pe.printLinuxDistribution();
            return EXIT_SUCCESS;
        }

        if (!pe.checkEnv())
            return EXIT_FAILURE;

        pe.readPxeConfig();
        pe.displayMessage();
        if (!pe.chooseEntry())
            return EXIT_SUCCESS;
        if (!pe.confirmBoot())
            return EXIT_SUCCESS;
        pe.downloadStuff();
        pe.execute();

    } catch (const ApplicationError &ae) {
        std::cerr << ae.what() << std::endl;
        return EXIT_FAILURE;
    } catch (const std::runtime_error &re) {
        std::cerr << "Runtime error: " << re.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

// :tabSize=4:indentSize=4:noTabs=true:mode=c++:folding=explicit:collapseFolds=1:maxLineLen=100:
