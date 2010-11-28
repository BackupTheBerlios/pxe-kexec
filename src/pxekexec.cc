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
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <ctime>
#include <cmath>
#include <cstring>
#include <cstring>
#include <memory>
#include <cstdlib>

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <libbw/debug.h>
#include <libbw/completion.h>
#include <libbw/stringutil.h>
#include <libbw/optionparser.h>

#include "pxekexec.h"
#include "networkhelper.h"
#include "downloader.h"
#include "pxeparser.h"
#include "kexec.h"
#include "config.h"
#include "process.h"
#include "linuxdb.h"
#include "ext/rpmvercmp.h"

/* SimpleNotifier definition {{{ */

/**
 * @brief Simple notifier implementation
 *
 * Implements the ProgressNotifier interface, providing a simple progress
 * bar on console.
 *
 * @author Bernhard Walle <bernhard@bwalle.de>
 */
class SimpleNotifier : public ProgressNotifier {

    public:
        /**
         * @brief Default Constructor
         *
         * Creates a new instance of SimpleNotifier.
         */
        SimpleNotifier();

        /**
         * @brief Destructor
         *
         * Deletes a SimpleNotifier
         */
        virtual ~SimpleNotifier() { }

    public:
        /**
         * @copydoc ProgressNotifier::progressed(double, double)
         */
        int progressed(double total, double now);

        /**
         * @copydoc ProgressNotifier::finished()
         */
        void finished();

    private:
        struct timeval m_lastDot;
};

#define CONNECTION_TIMEOUT 10

/* }}} */
/* SimpleNotifier implementation {{{ */

/* ---------------------------------------------------------------------------------------------- */
SimpleNotifier::SimpleNotifier()
{
    m_lastDot.tv_sec = 0;
    m_lastDot.tv_usec = 0;
}

/* ---------------------------------------------------------------------------------------------- */
static unsigned long long difftime_timeval(struct timeval a, struct timeval b)
{
    return (b.tv_sec - a.tv_sec) * 1000000 + (b.tv_usec - a.tv_usec);
}

/* ---------------------------------------------------------------------------------------------- */
int SimpleNotifier::progressed(double total, double now)
{
    struct timeval current_time;

    (void)total;
    (void)now;

    gettimeofday(&current_time, NULL);

    if (difftime_timeval(m_lastDot, current_time) < 100000)
        return true;

    std::cout << ".";
    std::cout.flush();

    m_lastDot = current_time;

    return true;
}

/* ---------------------------------------------------------------------------------------------- */
void SimpleNotifier::finished()
{
    std::cout << std::endl;
}

/* }}} */
/* PxeKexec {{{ */

/* ---------------------------------------------------------------------------------------------- */
PxeKexec::PxeKexec()
    : m_noconfirm(false)
    , m_nodelete(false)
    , m_quiet(false)
    , m_protocol("tftp")
    , m_dryRun(false)
    , m_force(false)
    , m_ignoreWhitelist(false)
    , m_detectDistOnly(false)
    , m_loadOnly(false)
{
    m_lineReader = bw::LineReader::defaultLineReader("> ");
}

/* ---------------------------------------------------------------------------------------------- */
PxeKexec::~PxeKexec()
{
    deleteKernels();
    delete m_lineReader;
}

/* ---------------------------------------------------------------------------------------------- */
bool PxeKexec::parseCmdLine(int argc, char *argv[])
    throw (ApplicationError)
{
    // repare the parser
    bw::OptionParser op;
    op.addOption(bw::Option("help",                'h', bw::OT_FLAG,
                            "Shows this help output"));
    op.addOption(bw::Option("version",             'v', bw::OT_FLAG,
                            "Shows the version and exits"));
    op.addOption(bw::Option("label",               'l', bw::OT_STRING,
                            "Boot the specified label without prompting"));
    op.addOption(bw::Option("interface",           'i', bw::OT_STRING,
                            "Use the specified network interface"));
    op.addOption(bw::Option("noconfirm",           'n', bw::OT_FLAG,
                            "Don't confirm the execution"));
    op.addOption(bw::Option("quiet",               'q', bw::OT_FLAG,
                            "Don't display PXE messages"));
    op.addOption(bw::Option("nodelete",            'd', bw::OT_FLAG,
                            "Dont't delete the downloaded files"));
    op.addOption(bw::Option("load-only",           'L', bw::OT_FLAG,
                            "Only load the kernel, don't reboot or 'kexec -e'"));
    op.addOption(bw::Option("force",               'f', bw::OT_FLAG,
                            "Immediately reboot without shutdown(8)"));
    op.addOption(bw::Option("ftp",                 'F', bw::OT_FLAG,
                            "Use FTP instead of TFTP"));
    op.addOption(bw::Option("dry-run",             'Y', bw::OT_FLAG,
                            "Don't run the final kexec -e"));
    op.addOption(bw::Option("debug",               'D', bw::OT_FLAG,
                            "Enable debugging output"));
    op.addOption(bw::Option("ignore-whitelist",    'w', bw::OT_FLAG,
                            "Ignore whitelist of Linux distributions that support "
                            "kexec in their reboot scripts"));
    op.addOption(bw::Option("print-distribution",  'p', bw::OT_FLAG,
                            "Only print the detected Linux distribution and exit"));

    // do the parsing
    bool ret = op.parse(argc, argv);
    if (!ret)
        throw ApplicationError("Parsing command line options failed");

    // evaluate options
    if (op.getValue("help").getFlag()) {
        op.printHelp(std::cerr, PACKAGE_STRING " " PACKAGE_VERSION);
        return false;
    }
    if (op.getValue("version").getFlag()) {
        printVersion();
        return false;
    }

    if (op.getValue("print-distribution").getFlag())
        m_detectDistOnly = true;
    if (op.getValue("debug").getFlag())
        bw::Debug::debug()->setLevel(bw::Debug::DL_TRACE);
    if (op.getValue("noconfirm").getFlag())
        m_noconfirm = true;
    if (op.getValue("nodelete").getFlag())
        m_nodelete = true;
    if (op.getValue("force").getFlag())
        m_force = true;
    if (op.getValue("load-only").getFlag())
        m_loadOnly = true;
    if (op.getValue("ignore-whitelist").getFlag())
    	m_ignoreWhitelist = true;
    if (op.getValue("label").getType() != bw::OT_INVALID) {
        m_preChoice = op.getValue("label").getString();
        m_quiet = true;
    }
    if (op.getValue("interface").getType() != bw::OT_INVALID)
        m_networkInterface = op.getValue("interface").getString();
    if (op.getValue("ftp").getType() != bw::OT_INVALID)
        m_protocol = "ftp";
    if (op.getValue("dry-run").getType() != bw::OT_INVALID) {
        Process::enableDryRunMode();
        m_dryRun = true;
    }
    if (op.getValue("quiet").getType() != bw::OT_INVALID)
        m_quiet = true;

    std::vector<std::string> args = op.getArgs();
    if (args.size() > 1)
        throw ApplicationError("Too many arguments.");
    if (args.size() == 1)
        m_pxeHost = args[0];

    BW_DEBUG_DBG("load_only=%d, force=%d, nodelete=%d",
                 int(m_loadOnly), int(m_force), int(m_nodelete));

    return true;
}

/* ---------------------------------------------------------------------------------------------- */
void PxeKexec::readPxeConfig()
    throw (ApplicationError)
{
    NetworkHelper nh;
    NetworkInterface netif;

    if (m_networkInterface.size() > 0) {
        netif = nh.getInterface(m_networkInterface);
        if (!netif.isValid())
            throw ApplicationError("Specified network interface does not exist.");
    } else {
        std::vector<NetworkInterface> ifs = nh.getInterfaces();
        if (ifs.size() < 1)
            throw ApplicationError("No network interfaces found");

        // use the first interface that is up
        netif = ifs[0];
    }
    BW_DEBUG_TRACE("Using interface '%s'", netif.getName().c_str());

    std::string pxe_mac = std::string("01-") + netif.getMac(NetworkInterface::MF_LOWERCASE |
            NetworkInterface::MF_DASH);
    std::string pxe_ip = netif.getIp(NetworkInterface::IF_HEX);

    // get PXE host
    if (m_pxeHost.size() == 0)
        m_pxeHost = netif.getDHCPServerIP();
    if (m_pxeHost.size() == 0)
        throw ApplicationError("No TFTP server specified and also no "
                "DHCP server in the DHCP info file\n(/var/lib/dhcpcd/dhcpcd-<if>.info).");

    char names[10][25];

    strcpy(names[0], pxe_mac.c_str());
    for (int i = 1; i < 9; i++)
        strcpy(names[i], pxe_ip.substr(0, 8-(i-1)).c_str());
    strcpy(names[9], "default");

    std::stringstream ss;
    SimpleNotifier notifier;
    for (int i = 0; i < 10; i++) {
        std::string url = m_protocol + "://" + m_pxeHost + "/pxelinux.cfg/" + names[i];

        BW_DEBUG_TRACE("Trying to retrieve %s", url.c_str());
        if (!m_quiet)
            std::cout << "Trying " << "pxelinux.cfg/" << names[i] << " ";

        try {
            Downloader dl(ss, CONNECTION_TIMEOUT);
            dl.setUrl(url);
            if (!m_quiet)
                dl.setProgress(&notifier);
            dl.download();
            break;
        } catch (const DownloadError &err) {
            BW_DEBUG_TRACE("DownloadError: %s", err.what());

            if (err.getErrorcode() == DownloadError::DEC_CONNECTION_FAILED) {
                std::cerr << "Connection to " << m_pxeHost << " with protocol " << m_protocol
                          << " failed. Aborting." << std::endl;
                if (m_protocol == "tftp") {
                    std::cerr << std::endl;
                    std::cerr << "HINT: This failure could be because of Firewall settings. "
                                 "Check your Firewall " << std::endl;
                    std::cerr << "configuration. If your FTP server has the same directory layout "
                              << "as your TFTP" << std::endl;
                    std::cerr << "server, you may also consider the '-F' option." << std::endl;
                    std::cerr << std::endl;
                }
                break;
            }
        }
    }

    if (ss.str().size() == 0)
        throw ApplicationError("No PXE configuration found.");

    PxeParser parser;
    try {
        parser.parseStream(ss);
        m_pxeConfig = parser.getConfig();
    } catch (const ParseError &pe) {
        throw ApplicationError(std::string("Parsing PXE config file failed: ") + pe.what());
    }
}

/* ---------------------------------------------------------------------------------------------- */
bool PxeKexec::checkEnv()
{
    if (!Process::isInPath("kexec")) {
        std::cerr << "Error: kexec-tools are not installed." << std::endl;
        return false;
    }
    if (geteuid() != 0) {
        std::cerr << "You have to be root to successfully use that program." << std::endl;
        return false;
    }

    //
    // check distribution
    //
    if (!m_ignoreWhitelist) {

        LinuxDistDetector *detector_ptr = LinuxDistDetector::getDetector();
        if (!detector_ptr) {
            std::cerr << "Could not detect Linux distribution." << std::endl;
            std::cerr << "Please read pxe-kexec(8) for more information. You "
                      << "can overwrite the check with '--ignore-whitelist'." << std::endl;
            return false;
        }
        std::auto_ptr<LinuxDistDetector> detector(detector_ptr);

        LinuxDistDetector::DistType type = detector->getType();
        std::string dist = detector->getDistribution();
        const char *detected_version = detector->getRelease().c_str();

        bool suitable_dist = false;
        if ((dist == "Ubuntu"       && rpmvercmp(detected_version, "9.04") >= 0)  ||
            (dist == "openSUSE"     && rpmvercmp(detected_version, "11.0") >= 0)  ||
            (dist == "Debian"       && rpmvercmp(detected_version, "5.0")  >= 0)  ||
            (dist == "Fedora"       && rpmvercmp(detected_version, "11")   >= 0)  ||
            ((dist == "CentOS" ||
              bw::startsWith(dist, "Red Hat Enterprise Linux"))
                                    && rpmvercmp(detected_version, "5.3") >= 0) ||
            (type == LinuxDistDetector::DT_ARCH)) {
            suitable_dist = true;
        }

        if (!suitable_dist) {
            std::cerr << "You Linux distribution '" << dist << "' in version '" << detected_version
                      << "'" << " doesn't have kexec" << std::endl;
            std::cerr << "in the reboot script. Please read pxe-kexec(8) for more information. You"
                      << std::endl
                      << "can overwrite the check with '--ignore-whitelist'." << std::endl;
            return false;
        }
    }

    return true;
}

/* ---------------------------------------------------------------------------------------------- */
void PxeKexec::displayMessage()
{
    if (!m_quiet) {
        std::cout << "\e[2J\e[0;0H";
        std::cout << m_pxeConfig.getMessage() << std::endl << std::endl << std::endl;
    }
}

/* ---------------------------------------------------------------------------------------------- */
bool PxeKexec::chooseEntry()
{
    std::string choice;

    if (m_preChoice.size() != 0)
        m_choice = m_pxeConfig.getEntry(m_preChoice);

    m_lineReader->setCompletor(this);
    while (!m_lineReader->eof() && !m_choice.isValid()) {
        choice = bw::strip(m_lineReader->readLine());
        if (choice.size() == 0 || choice == "quit" || choice == "exit") {
            m_lineReader->setCompletor(NULL);
            return false;
        }
        m_choice = m_pxeConfig.getEntry(choice);
        if (!m_choice.isValid())
            std::cout << "Entry " << choice << " does not exist." << std::endl;
    }

    m_lineReader->setCompletor(NULL);
    return m_choice.isValid();
}

/* ---------------------------------------------------------------------------------------------- */
StringVector PxeKexec::complete(const std::string   &text,
                                const std::string   &full_text,
                                size_t              start_idx,
                                ssize_t             end_idx)
{
    StringVector names = m_pxeConfig.getEntryNames();
    StringVector result;

    (void)full_text;
    (void)start_idx;
    (void)end_idx;

    for (StringVector::const_iterator it = names.begin(); it != names.end(); ++it) {
        std::string name = *it;
        if (strncasecmp(name.c_str(), text.c_str(), text.size()) == 0)
            result.push_back(name);
    }

    return result;
}

/* ---------------------------------------------------------------------------------------------- */
bool PxeKexec::confirmBoot()
{
begin:
    if (!(m_noconfirm && m_quiet)) {
        std::cout << "Booting following entry:" << std::endl;
        std::cout << "Kernel   : " << m_choice.getKernel() << std::endl;
        std::cout << "Initrd   : " << m_choice.getInitrd() << std::endl;
        std::cout << "Append   : " << m_choice.getAppend() << std::endl;
        std::cout << std::endl;
    }

    if (m_noconfirm)
        return true;

    std::cout << "Continue? [Y/n/e] ";
again:
    std::cout.flush();

    std::string ret;
    std::getline(std::cin, ret, '\n');
    if (ret.size() == 0 || ret[0] == '\n')
        return true;

    if (ret[0] == 'e' || ret[0] == 'E') {
        m_choice.setAppend(m_lineReader->editLine(m_choice.getAppend().c_str()));
        std::cout << std::endl << std::endl;
        goto begin;
    }

    switch (rpmatch(ret.c_str())) {
    case 0: /* no */
        return false;

    case 1: /* yes */
        return true;

    default: /* invalid */
        std::cout << "Invalid input. Try again [Y/n/e] ";
        goto again;
    }

    return false;
}

/* ---------------------------------------------------------------------------------------------- */
void PxeKexec::downloadStuff()
    throw (ApplicationError)
{
    std::string tmpdir = std::getenv("TMPDIR") ? std::getenv("TMPDIR") : "/tmp";
    tmpdir += "/";

    std::string kernel = tmpdir + "pxe-kexec-kernel";
    std::string initrd = tmpdir + "pxe-kexec-initrd";

    struct stat statbuf;
    while (stat(kernel.c_str(), &statbuf) >= 0)
        kernel += "_";

    while (stat(initrd.c_str(), &statbuf) >= 0)
        initrd += "_";

    SimpleNotifier notifier;
    std::string url;
    try {
        std::cout << "Downloading kernel ";
        std::ofstream os(kernel.c_str(), std::ios::binary);
        Downloader dl(os);
        url = m_protocol + "://" + m_pxeHost + "/" + m_choice.getKernel();
        dl.setUrl(url);
        dl.setProgress(&notifier);
        dl.download();
        os.close();
        m_downloadedKernel = kernel;
    } catch (const DownloadError &err) {
        throw ApplicationError("Downloading kernel "+ url +" failed: " + std::string(err.what()));
    }

    if (m_choice.getInitrd().size() > 0) {
        try {
            std::cout << "Downloading initrd ";
            std::ofstream os(initrd.c_str(), std::ios::binary);
            Downloader dl(os);
            url = m_protocol + "://" + m_pxeHost + "/" + m_choice.getInitrd();
            dl.setUrl(url);
            dl.setProgress(&notifier);
            dl.download();
            os.close();
            m_downloadedInitrd = initrd;
        } catch (const DownloadError &err) {
            throw ApplicationError("Downloading initrd "+url + " failed: " + std::string(err.what()));
        }
    }
}

/* ---------------------------------------------------------------------------------------------- */
void PxeKexec::deleteKernels()
{
    if (m_nodelete)
        return;

    // delete kernel and initrd since they have been loaded
    if (m_downloadedInitrd.size() > 0) {
        if (remove(m_downloadedKernel.c_str()) != 0)
            BW_DEBUG_INFO("Removal of %s failed.", m_downloadedKernel.c_str());
    }

    if (m_downloadedInitrd.size() > 0) {
        if (remove(m_downloadedInitrd.c_str()) != 0)
            BW_DEBUG_INFO("Removal of %s failed.", m_downloadedInitrd.c_str());
    }

}

/* ---------------------------------------------------------------------------------------------- */
void PxeKexec::execute()
    throw (ApplicationError)
{
    Kexec ke;

    if (m_downloadedKernel.size() == 0)
        throw ApplicationError("No kernel downloaded.");
    ke.setKernel(m_downloadedKernel);

    if (m_downloadedInitrd.size() > 0)
        ke.setInitrd(m_downloadedInitrd);

    ke.setAppend(m_choice.getAppend());
    bool loaded = ke.load();
    deleteKernels();

    if (!loaded)
        throw ApplicationError("Loading kernel failed.");

    if (m_loadOnly) {
        std::cerr << "Kernel loaded" << std::endl;
    } else {
        if (m_force) {
            /* try to change VT */
            if (m_dryRun) {
                std::cerr << "Switching to virtual terminal 0 in real world" << std::endl;
            } else {
                ke.prepareConsole();
            }

            // this never returns on success
            if (!ke.execute()) {
                throw ApplicationError("Executing kernel failed");
            }
        } else {
            std::cerr << "Initiating reboot" << std::endl;
            ke.reboot();
        }
    }
}

/* ---------------------------------------------------------------------------------------------- */
bool PxeKexec::getPrintLinuxDistributionOnly() const
{
    return m_detectDistOnly;
}

/* ---------------------------------------------------------------------------------------------- */
void PxeKexec::printLinuxDistribution()
{
    LinuxDistDetector *detector_ptr = LinuxDistDetector::getDetector();
    if (!detector_ptr) {
        std::cout << "Could not detect Linux distribution" << std::endl;
        return;
    }
    std::auto_ptr<LinuxDistDetector> detector(detector_ptr);

    std::cout << "Type        : " << detector->getTypeAsString() << std::endl;
    std::cout << "Name        : " << detector->getDistribution() << std::endl;
    std::cout << "Release     : " << detector->getRelease()      << std::endl;
    std::cout << "Codename    : " << detector->getCodename()     << std::endl;
    std::cout << "Description : " << detector->getDescription()  << std::endl;
}

/* ---------------------------------------------------------------------------------------------- */
void PxeKexec::printVersion()
{
    std::cerr << PACKAGE_STRING << " " << PACKAGE_VERSION << std::endl;
    std::cerr << "Compiled "
#if HAVE_LIBREADLINE
             << "with"
#else
             << "without"
#endif
             << " readline support" << std::endl;
}

/* }}} */

// :tabSize=4:indentSize=4:noTabs=true:mode=c++:folding=explicit:collapseFolds=1:maxLineLen=100:
