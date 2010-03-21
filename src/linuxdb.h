/*
 * (c) 2009-2010, Bernhard Walle <bernhard@bwalle.de>
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
 * @file linuxdb.h
 * @brief Linux Distribution Database
 *
 * This file contains an implementation of a Linux distribution detector.
 * We use that for the whitelist to warn the user about distributions
 * that don't call kexec as part of their shutdown process.
 *
 * @author Bernhard Walle <bernhard@bwalle.de>
 */

#ifndef LINUXDB_H
#define LINUXDB_H

#include <string>

/* LinuxDistDetector {{{ */

/**
 * @brief Linux Distribution Detector
 *
 * This is an interface that gives you information about the Linux distribution we are running
 * on. Use the static method LinuxDistDetector::getDetector() to get a Linux distribution detector
 * suitable for your system.
 *
 * Please note that the subclass doesn't tell you very much about the distribution. We have
 * a LSBDetector implementation that is used for Ubuntu, for example.
 *
 * For example use that class like following (the use of std::auto_ptr makes the code
 * block exception safe, but of course you can use the traditional @c delete):
 *
 * @code
 * std::auto_ptr<LinuxDistDetector> dt(LinuxDistDetector::getDetector());
 * dt.detect();
 * if (dt.getType() != LinuxDistDetector::DT_REDHAT) {
 *     std::cerr << "SUSE is not supported. Buy Red Hat or pay $500,000." << std::endl;
 *     return -1;
 * }
 * @endcode
 *
 * The function names are
 *
 * @author Bernhard Walle <bernhard@bwalle.de>
 */
class LinuxDistDetector
{
    public:

        /**
         * This enumeration groups Linux distributions together. For example, <i>openSUSE</i>
         * and <i>SUSE Linux Enterprise Server (SLES)</i> have a different distribution string
         * but have enough properties in common to identify them both as "SUSE".
         */
        enum DistType {
            DT_UNKNOWN,     /**< unknown distribution type */
            DT_SUSE,        /**< openSUSE, SUSE Linux Enterprise (Server/Desktop) and SUSE Linux */
            DT_DEBIAN,      /**< Debian GNU/Linux and some derivates, but not Ubuntu */
            DT_UBUNTU,      /**< Ubuntu */
            DT_REDHAT,      /**< Red Hat (the old ones), Red Hat Enterprise Linux and Fedora */
            DT_ARCH         /**< ARCH Linux */
        };

        /**
         * @brief Returns the string representation of a DistType
         *
         * @return the representation of a LinuxDistDetector::DistType as string
         */
        static std::string distTypeToString(DistType type);

    public:

        /**
         * @brief Returns a suitable detector
         *
         * This function should be called to create a suitable detector for that system.
         *
         * @return a detector or @c NULL if there is no suitable detector for the system
         */
        static LinuxDistDetector *getDetector()
        throw ();

        /**
         * @brief Destructor
         *
         * Deletes the LinuxDistDetector.
         */
        virtual ~LinuxDistDetector() {}

         /**
         * @brief Returns the type of the Linux distribution
         *
         * The type is one of the values in LinuxDistDetector::DistType.
         *
         * @return the type or LinuxDistDetector::DT_UNKNOWN if the type was not detected
         */
        virtual DistType getType() const
        throw () = 0;

        /**
         * @brief Returns the type as string
         *
         * Convenience method that just combines the LinuxDistDetector::getType() call with
         * LinuxDistDetector::distTypeToString().
         *
         * @return the dist type as string
         */
        virtual std::string getTypeAsString() const
        throw () = 0;

        /**
         * @brief Actually does the detection work
         *
         * You need to call that function before calling any other method like
         * getDistribution(). Otherwise, these functions return empty strings.
         *
         * @return @c true if we were able to detect something, @c false otherwise
         */
        virtual bool detect()
        throw () = 0;

        /**
         * @brief Returns the Linux distribution
         *
         * Returns the Linux distribution as string. If you just need to know on which type
         * of Linux we are running on, use getType(). For example, openSUSE and SUSE Linux
         * Enterprise returns a different strings here but share the type DistType::DT_SUSE.
         *
         * Example: <tt>LinuxDistDetector::DT_UBUNTU</tt>
         *
         * @return the Linux distribution that was detected or the empty string if
         *         LinuxDistDetector::detect() has not been executed before.
         */
        virtual std::string getDistribution() const
        throw () = 0;

        /**
         * @brief Returns the release of the Linux distribution
         *
         * This is a string like <tt>"9.04"</tt> for <i>Ubuntu 9.04</i> or <tt>"11.1"</tt> for
         * <i>openSUSE 11.1</i>.
         *
         * Example: <tt>9.04</tt>
         *
         * @return the release string or the empty string if LinuxDistDetector::detect() has
         *         not been executed before.
         */
        virtual std::string getRelease() const
        throw () = 0;

        /**
         * @brief Returns the code name of the Linux distributions
         *
         * If the distribution has no code name, an empty string is returned.
         *
         * Example: <tt>jaunty</tt>
         *
         * @return the code name or an empty string
         */
        virtual std::string getCodename() const
        throw () = 0;

        /**
         * @brief Returns the description for the Linux distribution
         *
         * Returns a description that contains both the release number and the distribution
         * type.
         *
         * Example: <tt>Ubuntu 9.04</tt>
         *
         * @return the description
         */
        virtual std::string getDescription() const
        throw () = 0;

};

/* }}} */
/* AbstractLinuxDistDetector {{{ */

/**
 * @brief Base implementation for LinuxDistDetector
 *
 * This is a base implementation for all LinuxDistDetector implementations. It provides
 * getter and setter methods.
 *
 * @author Bernhard Walle <bernhard@bwalle.de>
 */
class AbstractLinuxDistDetector : public LinuxDistDetector
{
    public:
        /**
         * @brief Constructor
         *
         * Creates a new instance of a AbstractLinuxDistDetector.
         */
        AbstractLinuxDistDetector()
        throw ();

        /**
         * @copydoc LinuxDistDetector::getType()
         */
        virtual DistType getType() const
        throw ();

        /**
         * @copydoc LinuxDistDetector::getTypeAsString()
         */
        virtual std::string getTypeAsString() const
        throw ();

        /**
         * @copydoc LinuxDistDetector::getDistribution()
         */
        virtual std::string getDistribution() const
        throw ();

        /**
         * @copydoc LinuxDistDetector::getRelease()
         */
        virtual std::string getRelease() const
        throw ();

        /**
         * @copydoc LinuxDistDetector::getCodename()
         */
        virtual std::string getCodename() const
        throw ();

        /**
         * @copydoc LinuxDistDetector::getDescription()
         */
        virtual std::string getDescription() const
        throw ();

    protected:

        /**
         * @brief Sets the distribution type
         *
         * @param[in] type the distribution type (see LinuxDistDetector::getType() for an example)
         */
        void setType(DistType type)
        throw ();

        /**
         * @brief Sets the distribution
         *
         * @param[in] distribution the distribution (see LinuxDistDetector::getDistribution()
         *            for an example)
         */
        void setDistribution(const std::string &distribution)
        throw ();

        /**
         * @brief Sets the release
         *
         * @param[in] release the release (see LinuxDistDetector::getRelease() for an example)
         */
        void setRelease(const std::string &release)
        throw ();

        /**
         * @brief Sets the codename
         *
         * @param[in] codename the code name (see LinuxDistDetector::getCodename() for an example)
         */
        void setCodename(const std::string &codename)
        throw ();

        /**
         * @brief Sets the description
         *
         * @param[in] description the code name (see LinuxDistDetector::getDescription() for an
         *            example)
         */
        void setDescription(const std::string &description)
        throw ();

    private:
        DistType m_type;
        std::string m_distribution;
        std::string m_release;
        std::string m_codename;
        std::string m_description;
};

/* }}} */
/* LSBLinuxDistDetector {{{ */

/**
 * @brief Detector for LSB distributions
 *
 * Reads <tt>/etc/lsb-release</tt> to detect the Linux distribution.
 *
 * @author Bernhard Walle <bernhard@bwalle.de>
 */
class LSBLinuxDistDetector : public AbstractLinuxDistDetector
{
    public:
        /**
         * @copydoc LinuxDistDetector::detect()
         */
        bool detect()
        throw ();
};

/* }}} */
/* SUSELinuxDistDetector {{{ */

/**
 * @brief Detector for SUSE distributions
 *
 * Reads <tt>/etc/SuSE-release</tt> to detect the Linux distribution.
 *
 * @author Bernhard Walle <bernhard@bwalle.de>
 */
class SUSELinuxDistDetector : public AbstractLinuxDistDetector
{
    public:
        /**
         * @copydoc LinuxDistDetector::detect()
         */
        bool detect()
        throw ();
};

/* }}} */
/* RedHatDistDetector {{{ */

/**
 * @brief Detector for the Red Hat distributions
 *
 * Red Hat includes also CentOS and Fedora. Reads <tt>/etc/redhat-release</tt>. On Fedora,
 * the file <tt>/etc/fedora-release</tt> is just a symbolic link to <tt>/etc/redhat-release</tt>.
 *
 * @author Bernhard Walle <bernhard@bwalle.de>
 */
class RedHatDistDetector : public AbstractLinuxDistDetector
{
    public:
        /**
         * @copydoc LinuxDistDetector::detect()
         */
        bool detect()
        throw ();
};

/* }}} */
/* ArchDistDetector {{{ */

/**
 * @brief Detector for Arch distributions
 *
 * Reads <tt>/etc/arch-release</tt> to detect the Linux distribution. This distribution
 * has no version information.
 *
 * @author Bernhard Walle <bernhard@bwalle.de>
 */
class ArchDistDetector : public AbstractLinuxDistDetector
{
    public:
        /**
         * @copydoc LinuxDistDetector::detect()
         */
        bool detect()
        throw ();
};

/* }}} */
/* DebianDistDetector {{{ */

/**
 * @brief Detector for Debian distributions
 *
 * Reads <tt>/etc/debian_version</tt> to detect the Linux distribution.
 *
 * @author Bernhard Walle <bernhard@bwalle.de>
 */
class DebianDistDetector : public AbstractLinuxDistDetector
{
    public:
        /**
         * @copydoc LinuxDistDetector::detect()
         */
        bool detect()
        throw ();
};

/* }}} */

#endif /* LINUXDB_H */

// :tabSize=4:indentSize=4:noTabs=true:mode=c++:folding=explicit:collapseFolds=1:maxLineLen=100:
