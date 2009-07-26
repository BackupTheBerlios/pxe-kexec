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
#ifndef NETWORKHELPER_H
#define NETWORKHELPER_H

#include <string>
#include <vector>

#include "global.h"

/* NetworkInterface {{{ */

class NetworkInterface {

    public:
        enum MacFormat {
            MF_UPPERCASE = 1<<0,
            MF_LOWERCASE = 1<<1,
            MF_DASH      = 1<<2,
            MF_COLON     = 1<<3,
            MF_NOSEP     = 1<<4
        };
        enum IpFormat {
            IF_HEX       = 1<<0,
            IF_DOT       = 1<<1
        };

    public:
        NetworkInterface();
        NetworkInterface(const std::string &name);
        virtual ~NetworkInterface() {}

    public:
        bool isValid() const;

        void setName(const std::string &name);
        std::string getName() const;

        bool isUp();
        void setUp(bool up);

        std::string getMac(int format = MF_UPPERCASE | MF_COLON);
        void setMac(const char *data);

        std::string getIp(int format = IF_DOT);
        void setIp(int ip);

        // IF_DOT format
        std::string getDHCPServerIP() const;
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

class NetworkHelper {

    public:
        NetworkHelper();
        virtual ~NetworkHelper() {}

    public:
        std::vector<NetworkInterface> getInterfaces()
            throw (ApplicationError);
        NetworkInterface getInterface(const std::string &ifname);

    protected:
        void detectInterfaces()
            throw (ApplicationError);
        void detectDHCPServers();

    private:
        bool m_ifDiscovered;
        std::vector<NetworkInterface> m_interfaces;
};

/* }}} */


#endif /* NETWORKHELPER_H */

// vim: set sw=4 ts=4 fdm=marker et:
