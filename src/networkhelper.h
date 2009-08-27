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
#ifndef NETWORKHELPER_H
#define NETWORKHELPER_H

/**
 * @file networkhelper.h
 * @brief Network helper classes
 *
 * This file contains classes for network interface descriptions on Linux.
 *
 * @author Bernhard Walle <bernhard@bwalle.de>
 */

#include <string>
#include <vector>

#include "global.h"

/* NetworkInterface {{{ */

/**
 * @brief Describes a network interface
 *
 * This class describes a Ethernet network interface. A network interface has
 * an IP address, a MAC address and a name (like @c eth0).
 *
 * @author Bernhard Walle <bernhard@bwalle.de>
 */
class NetworkInterface {

    public:
        /**
         * @brief Format of a MAC address
         *
         * This enumeration describes the format of a MAC address. This is a
         * bitmask, so the elements can be combined.
         */
        enum MacFormat {
            MF_UPPERCASE = 1<<0,    /**< the hexadecimal numbers are all in uppercase (A-F) */
            MF_LOWERCASE = 1<<1,    /**< the hexadecimal numbers are all in lowercase (a-f) */
            MF_DASH      = 1<<2,    /**< we use dashes (<tt>-</tt>) to separate pairs of numbers */
            MF_COLON     = 1<<3,    /**< we use colons (<tt>:</tt>) to separate pairs of numbers */
            MF_NOSEP     = 1<<4     /**< we don't separate the numbers with dashes or colons */
        };
        /**
         * @brief Format of an IP address
         *
         * This enumeration describes the format of a IP address
         */
        enum IpFormat {
            IF_HEX       = 1<<0,    /**< use 8 hex digits for an IP address, e.g. @c C0C60001 */
            IF_DOT       = 1<<1     /**< use the standard dotted decimal notation, e.g. <tt>192.168.0.1</tt> */
        };

    public:
        /**
         * @brief Default constructor
         *
         * Creates a new instance of NetworkInterface without a name.
         */
        NetworkInterface();

        /**
         * @brief Constructor
         *
         * Creates a new instance of NetworkInterface with the name @p name.
         *
         * @param[in] name the name of the network interface, e.g. @c eth0
         */
        NetworkInterface(const std::string &name);

        /**
         * @brief Destructor
         *
         * Deletes a NetworkInterface.
         */
        virtual ~NetworkInterface() {}

    public:
        /**
         * @brief Checks if the NetworkInterface is valid
         *
         * Checks if the current object is valid. A NetworkInterface is valid
         * if it has a name, i.e. it was constructed with
         * NetworkInterface(const std::string &).
         *
         * @return @c true if the interface is valid, @c false otherwise
         */
        bool isValid() const;

        /**
         * @brief Sets the name of the network interface.
         *
         * Sets the name of the network interface.
         *
         * @param[in] name the name, e.g. @c eth0.
         */
        void setName(const std::string &name);

        /**
         * @brief Returns the name of the network interface
         *
         * Returns the name of the network interface.
         *
         * @return the name, e.g. @c eth0
         */
        std::string getName() const;

        /**
         * @brief Checks if the network interface is "up"
         *
         * Checks if the current network interface is "up". See the
         * documentation of the @c ip or @c ifconfig command to get an idea
         * what "up"means in the context of a network interface.
         *
         * @return @c true if the network interface is up, @c false otherwise
         */
        bool isUp();

        /**
         * @brief Sets the "up" status of a network interface
         *
         * Sets the "up" status that can be checked with isUp().
         *
         * @param[in] up @c true if the network interface is up, @c false
         *            otherwise
         */
        void setUp(bool up);

        /**
         * @brief Retrieves the MAC address of the network interface
         *
         * Retrieves the MAC address of the network interface in the specified
         * @p format.  Example:
         *
         * @code
         * std::string mac = netif.getMac(NetworkInterface::MF_LOWERCASE | NetworkInterface::MF_DASH);
         * // mac is e.g. "af-fe-00-00-01-01"
         * mac = netif.getMac(NetworkInterface::MF_UPPERCASE | NetworkInterface::MF_NOSEP);
         * // mac is e.g. "AFFE00000101"
         * @endcode
         *
         * @param[in] format the format, see NetworkInterface::MacFormat
         * @return the MAC address of the network interface
         */
        std::string getMac(int format = MF_UPPERCASE | MF_COLON);

        /**
         * @brief Sets the MAC address
         *
         * Sets the MAC address.
         *
         * @param[in] data the MAC address in binary representation. This
         *            pointer must point to at least 6 valid bytes. There is
         *            no termination required.
         */
        void setMac(const char *data);

        /**
         * @brief Returns the IP address
         *
         * Returns the IP address of the network interface. Example:
         *
         * @code
         * std::string ip = netif.getIp(NetworkInterface::IF_HEX);
         * // ip is e.g. "C0C60001"
         * ip = netif.getIp(NetworkInterface::IF_DOT);
         * // ip == "192.168.0.1"
         * @endcode
         *
         * @return the IP address of the network interface in format @p format
         */
        std::string getIp(int format = IF_DOT);

        /**
         * @brief Sets the IP address
         *
         * Sets the IP address of the network interface.
         *
         * @param[in] ip the IP address as (at least) 32-bit integer
         */
        void setIp(int ip);

        /**
         * @brief Returns the IP address of the DHCP server
         *
         * Returns the IP address of the DHCP server. If the interface didn't
         * have its IP address from a DHCP server, the empty string is returned.
         *
         * @return the IP address in dotted decimal format (NetworkInterface::IF_DOT)
         */
        std::string getDHCPServerIP() const;

        /**
         * @brief Set the IP address of the DHCP server
         *
         * Sets the DHCP IP address of the DHCP server.
         *
         * @param[in] ip the IP address of the DHCP server in dotted decimal
         *            format (NetworkInterface::IF_DOT)
         */
        void setDHCPServerIP(const std::string &ip);

    private:
        bool m_isValid;
        bool m_up;
        std::string m_name;
        std::string m_dhcpServerIP;
        char m_mac[6];
        int m_ip;
};

/* }}} */
/* NetworkHelper {{{ */

/**
 * @brief Helper class for retrieving network interfaces
 *
 * This object is responsible to getrieve NetworkInterface objects from system
 * information.
 *
 * @author Bernhard Walle <bernhard@bwalle.de>
 * @bug only 30 network interfaces can currently be managed by a NetworkHelper
 */
class NetworkHelper {

    public:
        /**
         * @brief Constructor
         *
         * Creates a new instance of NetworkHelper.
         */
        NetworkHelper();

        /**
         * @brief Destructor
         *
         * Destroys a NetworkHelper.
         */
        virtual ~NetworkHelper() {}

    public:
        /**
         * @brief Returns the network interfaces of the system
         *
         * Returns all network interfaces.
         *
         * @return the network interfaces
         * @exception ApplicationError if detecting network interfaces failed
         */
        std::vector<NetworkInterface> getInterfaces()
            throw (ApplicationError);

        /**
         * @brief Returns the NetworkInterface object for a specific interface
         *
         * Returns the NetworkInterface object that corresponds to the
         * interface with name @p ifname. If there is no network interface
         * with name @p ifname, a NetworkInterface instance is returned whose
         * NetworkInterface::isValid() method returns @c false.
         *
         * @return the NetworkInterface
         */
        NetworkInterface getInterface(const std::string &ifname);

    protected:
        /**
         * @brief Detects interfaces
         *
         * Detects the interfaces that are present in the system. Fills m_interfaces.
         *
         * @throw ApplicationError on any error
         */
        void detectInterfaces()
            throw (ApplicationError);

        /**
         * @brief Detects DHCP servers
         *
         * Reads <tt>/var/lib/dhcpcd/dhcpcd-*</tt> to find a DHCP server.
         */
        void detectDHCPServers();

    private:
        bool m_ifDiscovered;
        std::vector<NetworkInterface> m_interfaces;
};

/* }}} */


#endif /* NETWORKHELPER_H */

// :tabSize=4:indentSize=4:noTabs=true:mode=c++:folding=explicit:collapseFolds=1:maxLineLen=100:
