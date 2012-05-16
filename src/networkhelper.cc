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
#include <string>
#include <vector>
#include <cerrno>
#include <cstring>
#include <algorithm>
#include <fstream>

#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <unistd.h>

#include <libbw/stringutil.h>
#include <libbw/debug.h>

#include "networkhelper.h"

/* NetworkInterface {{{ */

/* ---------------------------------------------------------------------------------------------- */
NetworkInterface::NetworkInterface()
    : m_isValid(false)
{}

/* ---------------------------------------------------------------------------------------------- */
NetworkInterface::NetworkInterface(const std::string &name)
    : m_isValid(true)
    , m_up(false)
    , m_name(name)
{}

/* ---------------------------------------------------------------------------------------------- */
bool NetworkInterface::isValid() const
{
    return m_isValid;
}

/* ---------------------------------------------------------------------------------------------- */
std::string NetworkInterface::getName() const
{
    return m_name;
}

/* ---------------------------------------------------------------------------------------------- */
void NetworkInterface::setName(const std::string &name)
{
    m_name = name;
}

/* ---------------------------------------------------------------------------------------------- */
bool NetworkInterface::isUp()
{
    return m_up;
}

/* ---------------------------------------------------------------------------------------------- */
void NetworkInterface::setUp(bool up)
{
    m_up = up;
}

/* ---------------------------------------------------------------------------------------------- */
std::string NetworkInterface::getMac(int format)
{
    char buffer[32];
    char sep = format & MF_COLON ? ':' : '-';

    if (format & MF_UPPERCASE) {
        snprintf(buffer, 32, "%02hhX%c%02hhX%c%02hhX%c%02hhX%c%02hhX%c%02hhX",
                m_mac[0], sep, m_mac[1], sep, m_mac[2], sep,
                m_mac[3], sep, m_mac[4], sep, m_mac[5]);
    } else if (format & MF_LOWERCASE) {
        snprintf(buffer, 32, "%02hhx%c%02hhx%c%02hhx%c%02hhx%c%02hhx%c%02hhx",
                m_mac[0], sep, m_mac[1], sep, m_mac[2], sep,
                m_mac[3], sep, m_mac[4], sep, m_mac[5]);
    } else
        throw ApplicationError("NetworkInterface::getIp(): "
                "Invalid format specified");

    return std::string(buffer);
}

/* ---------------------------------------------------------------------------------------------- */
void NetworkInterface::setMac(const char *data)
{
    memmove(m_mac, data, 6);
}

/* ---------------------------------------------------------------------------------------------- */
std::string NetworkInterface::getIp(int format)
{
    char buffer[32];
    if (format & IF_HEX) {
        snprintf(buffer, 32, "%02X%02X%02X%02X",
                (m_ip & 0x000000ff) >> 0,
                (m_ip & 0x0000ff00) >> 8,
                (m_ip & 0x00ff0000) >> 16,
                (m_ip & 0xff000000) >> 24);
    } else if (format & IF_DOT) {
        snprintf(buffer, 32, "%d.%d.%d.%d",
                (m_ip & 0x000000ff) >> 0,
                (m_ip & 0x0000ff00) >> 8,
                (m_ip & 0x00ff0000) >> 16,
                (m_ip & 0xff000000) >> 24);
    } else
        throw ApplicationError("NetworkInterface::getIp(): "
                "Invalid format specified");

    return std::string(buffer);
}

/* ---------------------------------------------------------------------------------------------- */
void NetworkInterface::setIp(int addr)
{
    m_ip = addr;
}

/* ---------------------------------------------------------------------------------------------- */
void NetworkInterface::setDHCPServerIP(const std::string &ip)
{
    m_dhcpServerIP = ip;
}

/* ---------------------------------------------------------------------------------------------- */
std::string NetworkInterface::getDHCPServerIP() const
{
    return m_dhcpServerIP;
}

/* }}} */
/* NetworkHelper {{{ */

/* ---------------------------------------------------------------------------------------------- */
NetworkHelper::NetworkHelper()
    : m_ifDiscovered(false)
{}

/* ---------------------------------------------------------------------------------------------- */
bool compare_network_interface_name(NetworkInterface interf, std::string name)
{
    return interf.getName() == name;
}

/* ---------------------------------------------------------------------------------------------- */
NetworkInterface NetworkHelper::getInterface(const std::string &ifname)
{
    if (!m_ifDiscovered) {
        detectInterfaces();
        m_ifDiscovered = true;
    }

    std::vector<NetworkInterface>::iterator it =
        find_if(m_interfaces.begin(), m_interfaces.end(),
                bind2nd(ptr_fun(compare_network_interface_name), ifname));

    return it != m_interfaces.end() ? *it : NetworkInterface();
}


/* ---------------------------------------------------------------------------------------------- */
std::vector<NetworkInterface> NetworkHelper::getInterfaces()
    throw (ApplicationError)
{
    if (!m_ifDiscovered) {
        detectInterfaces();
        m_ifDiscovered = true;
    }

    return m_interfaces;
}

/* ---------------------------------------------------------------------------------------------- */
#define MAX_IFS 30
void NetworkHelper::detectInterfaces()
    throw (ApplicationError)
{
    int sockfd;
    struct ifreq *ifr;

    BW_DEBUG_TRACE("Detecting network interfaces");

    sockfd = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (!sockfd)
        throw ApplicationError(std::string("socket(PF_INET) failed: ") + std::strerror(errno));

    struct ifconf ifc;
    struct ifconf buffer[MAX_IFS];
    ifc.ifc_buf = reinterpret_cast<char *>(buffer);
    ifc.ifc_len = MAX_IFS * sizeof(struct ifreq);

    int ret = ioctl(sockfd, SIOCGIFCONF, &ifc);
    int error = errno;
    if (ret < 0) {
        close(sockfd);
        throw ApplicationError(std::string("ioctl(sockfd) failed: ") + std::strerror(error));
    }

    ifr = ifc.ifc_req;
    for (int n = 0; n < ifc.ifc_len; n += sizeof(struct ifreq), ifr++) {
        NetworkInterface interface(ifr->ifr_name);
        interface.setUp(true);

        BW_DEBUG_INFO("Found network interface %s", interface.getName().c_str());

        // get flags to see if it's a loopback interface
        ret = ioctl(sockfd, SIOCGIFFLAGS, ifr);
        if (ret < 0) {
            close(sockfd);
            throw ApplicationError(std::string("ioctl(SIOCGIFHWADDR) failed: ")
                                   + std::strerror(error));
        }
        if (ifr->ifr_flags & IFF_LOOPBACK) {
            BW_DEBUG_TRACE("Interface %s is loopback", interface.getName().c_str());
            continue;
        }

        // get MAC address
        ret = ioctl(sockfd, SIOCGIFHWADDR, ifr);
        if (ret < 0) {
            close(sockfd);
            throw ApplicationError(std::string("ioctl(SIOCGIFHWADDR) failed: ")
                    + std::strerror(error));
        }
        interface.setMac(ifr->ifr_hwaddr.sa_data);

        // get IP address
        ret = ioctl(sockfd, SIOCGIFADDR, ifr);
        if (ret < 0) {
            close(sockfd);
            throw ApplicationError(std::string("ioctl(SIOCGIFADDR) failed: ")
                    + std::strerror(error));
        }

        interface.setIp(((struct sockaddr_in *)&ifr->ifr_addr)->sin_addr.s_addr);

        m_interfaces.push_back(interface);
    }

    close(sockfd);

    detectDHCPServers();
}
#undef MAX_IFS

/* ---------------------------------------------------------------------------------------------- */
bool NetworkHelper::detectDHCPServers()
{
    return detectDHCPServerDhcpd() || detectDHCPServerDhclient();
}


/* ---------------------------------------------------------------------------------------------- */
bool NetworkHelper::detectDHCPServerDhcpd()
{
    for (std::vector<NetworkInterface>::iterator it = m_interfaces.begin();
            it != m_interfaces.end(); ++it) {

        NetworkInterface &interface = *it;

        std::string configfile = "/var/lib/dhcpcd/dhcpcd-" + interface.getName()
            + ".info";
        std::ifstream fin(configfile.c_str());

        std::string line;
        while (std::getline(fin, line)) {
            if (bw::startsWith(line, "DHCPSIADDR=")) {
                interface.setDHCPServerIP(bw::getRest(line, "DHCPSIADDR="));
                BW_DEBUG_DBG("Set DHCP IP address of interface %s to %s",
                             interface.getName().c_str(),
                             interface.getDHCPServerIP().c_str());
                return true;
            }
        }

        fin.close();
    }

    return false;
}

/* ---------------------------------------------------------------------------------------------- */
bool NetworkHelper::detectDHCPServerDhclient()
{
    // this is our
    const std::string DHCLIENT_PXE_KEXEC_CONFIGDIR("/var/lib/pxe-kexec/");

    for (std::vector<NetworkInterface>::iterator it = m_interfaces.begin();
            it != m_interfaces.end(); ++it) {

        NetworkInterface &interface = *it;

        std::string configfile = DHCLIENT_PXE_KEXEC_CONFIGDIR + interface.getName();
        std::ifstream fin(configfile.c_str());

        std::string line;
        while (std::getline(fin, line)) {
            if (bw::startsWith(line, "dhcp_server_identifier=")) {
                interface.setDHCPServerIP(bw::getRest(line, "dhcp_server_identifier="));
                BW_DEBUG_DBG("Set DHCP IP address of interface %s to %s",
                            interface.getName().c_str(),
                            interface.getDHCPServerIP().c_str());
                return true;
            }
        }

        fin.close();
    }

    return false;
}

/* }}} */

// :tabSize=4:indentSize=4:noTabs=true:mode=c++:folding=explicit:collapseFolds=1:maxLineLen=100:
