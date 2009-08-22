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
#ifndef PROCESS_H
#define PROCESS_H

/**
 * @file process.h
 * @brief A process
 *
 * Defines a process that can be executed.
 *
 * @author Bernhard Walle <bernhard@bwalle.de>
 */

#include <string>
#include <vector>

/**
 * @brief Represents a process
 *
 * This class represents a process that can be executed. There's also a
 * static method called isInPath to check if a process is present via
 * <tt>$PATH</tt>.
 *
 * For example to execute <tt>ls -l</tt> one would write following code:
 *
 * @code
 * Process p("ls");
 * p.addArg("-l");
 * p.execute();
 * @endcode
 *
 * This seems to be a bit fiddly compared with just
 *
 * @code
 * system("ls -l");
 * @endcode
 *
 * but the big advantage is that there's no shell involved, so we don't need any
 * quoting.
 *
 * @author Bernhard Walle <bernhard@bwalle.de>
 */
class Process {

    public:
        /**
         * @brief Checks if a process is in path and executable
         *
         * Checks if a process in in system's <tt>$PATH</tt> and if we are
         * allowed to execute the process.
         *
         * @param[in] program the name of the process, e.g. @c ls
         * @return @c true if the process is in <tt>$PATH</tt> and if
         *         executing that program is ok, @c false otherwise
         */
        static bool isInPath(const std::string &program);

        /**
         * @brief Globally enables the dry-run mode
         *
         * If that static method is called, every Process::execute()
         * call does nothing besides printing a message what would be
         * executed.
         *
         * This function is not directly thread-safe. You have to serialize
         * all calls to Process::enableDryRunMode(),
         * Process::disableDryRunMode() and Process::execute().
         *
         * Use Process::disableDryRunMode() to disable dry-run mode.
         */
        static void enableDryRunMode()
        throw ();

        /**
         * @brief Globally disables the dry-run mode
         *
         * If that static method is called, every Process::execute() call
         * behaves normally. This is the opposite of Process::enableDryRunMode().
         */
        static void disableDryRunMode()
        throw ();

    public:
        /**
         * @brief Constructor
         *
         * Creates a new process without any arguments. Use addArg() to add
         * arguments.
         *
         * @param[in] name the name of the process, e.g. @c ls.
         */
        Process(const std::string &name);

        /**
         * @brief Destructor
         *
         * Currently that function does nothing.
         */
        virtual ~Process() { }

    public:
        /**
         * @brief Add an argument
         *
         * Adds an argument to a process. This method has to be called multiple
         * times if more than one argument should be added. No quoting is necessary
         * (and even not allowed) if @p arg contains spaces.
         *
         * @param[in] arg the argument to add
         */
        void addArg(const std::string &arg);

        /**
         * @brief Returns all arguments
         *
         * Returns a list of arguments currently set for that process.
         *
         * @return the list (vector) of arguments.
         */
        std::vector<std::string> getArgs() const;

        /**
         * @brief Executes the process.
         *
         * Executes the process with all arguments added with addArg().
         *
         * @return the process status code, that means 0 on success and any
         *         other value on failure. For readers familiar with Unix,
         *         we return only the @c WEXITSTATUS part of it.
         */
        int execute();

    private:
        std::string m_name;
        std::vector<std::string> m_args;
        static bool m_dryRunMode;
};

#endif /* PROCESS_H */

// vim: set sw=4 ts=4 fdm=marker et:
