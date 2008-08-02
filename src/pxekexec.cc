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
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <ctime>
#include <cmath>
#include <cstring>
#include <string.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

#include "pxekexec.h"
#include "optionparser.h"
#include "debug.h"
#include "networkhelper.h"
#include "downloader.h"
#include "pxeparser.h"
#include "io.h"
#include "kexec.h"
#include "config.h"
#include "stringutil.h"

using std::strcpy;
using std::time;
using std::cerr;
using std::endl;
using std::ios;
using std::cout;
using std::cin;
using std::vector;
using std::string;
using std::stringstream;
using std::ofstream;

/* SimpleNotifier definition {{{ */

class SimpleNotifier : public ProgressNotifier {

    public:
        SimpleNotifier();
        virtual ~SimpleNotifier() { }

    public:
        int progressed(double total, double now);
        void finished();

    private:
        struct timeval m_lastDot;
};

#define CONNECTION_TIMEOUT 10

/* }}} */
/* SimpleNotifier implementation {{{1 */

/* -------------------------------------------------------------------------- */
SimpleNotifier::SimpleNotifier()
{
    m_lastDot.tv_sec = 0;
    m_lastDot.tv_usec = 0;
}

/* -------------------------------------------------------------------------- */
static unsigned long long difftime_timeval(struct timeval a, struct timeval b)
{
    return (b.tv_sec - a.tv_sec) * 1000000 + (b.tv_usec - a.tv_usec);
}

/* -------------------------------------------------------------------------- */
int SimpleNotifier::progressed(double total, double now)
{
    struct timeval current_time;

    gettimeofday(&current_time, NULL);

    if (difftime_timeval(m_lastDot, current_time) < 100000)
        return true;

    cout << ".";
    cout.flush();

    m_lastDot = current_time;

    return true;
}

/* -------------------------------------------------------------------------- */
void SimpleNotifier::finished()
{
    cout << endl;
}

/* }}} */
/* PxeKexec {{{ */

/* -------------------------------------------------------------------------- */
PxeKexec::PxeKexec()
    : m_noconfirm(false), m_nodelete(false), m_quiet(false),
      m_protocol("tftp"), m_dryRun(false)
{
    m_lineReader = LineReader::defaultLineReader("> ");
}

/* -------------------------------------------------------------------------- */
PxeKexec::~PxeKexec()
{
    deleteKernels();
    delete m_lineReader;
}

/* -------------------------------------------------------------------------- */
bool PxeKexec::parseCmdLine(int argc, char *argv[])
    throw (ApplicationError)
{
    // repare the parser
    OptionParser op;
    op.addOption(Option("debug", 'D', OT_FLAG, "Enable debugging output"));
    op.addOption(Option("help", 'h', OT_FLAG, "Shows this help output"));
    op.addOption(Option("interface", 'i', OT_STRING, "Use the specified network interface"));
    op.addOption(Option("noconfirm", 'n', OT_FLAG, "Don't confirm the execution"));
    op.addOption(Option("quiet", 'q', OT_FLAG, "Don't display PXE messages"));
    op.addOption(Option("nodelete", 'd', OT_FLAG, "Dont't delete the downloaded files"));
    op.addOption(Option("ftp", 'F', OT_FLAG, "Use FTP instead of TFTP"));
    op.addOption(Option("dry-run", 'Y', OT_FLAG, "Don't run the final kexec -e"));

    // do the parsing
    bool ret = op.parse(argc, argv);
    if (!ret)
        throw ApplicationError("Parsing command line options failed");

    // evaluate options
    if (op.getValue("help").getFlag()) {
        op.printHelp(cerr, PACKAGE_STRING " " PACKAGE_VERSION);
        return false;
    }
    if (op.getValue("debug").getFlag())
        Debug::debug()->setLevel(Debug::DL_TRACE);
    if (op.getValue("noconfirm").getFlag())
        m_noconfirm = true;
    if (op.getValue("nodelete").getFlag())
        m_nodelete = true;
    if (op.getValue("interface").getType() != OT_INVALID)
        m_networkInterface = op.getValue("interface").getString();
    if (op.getValue("ftp").getType() != OT_INVALID)
        m_protocol = "ftp";
    if (op.getValue("dry-run").getType() != OT_INVALID)
        m_dryRun = true;
    if (op.getValue("quiet").getType() != OT_INVALID)
        m_quiet = true;

    vector<string> args = op.getArgs();
    if (args.size() > 2)
        throw ApplicationError("Too many arguments.");
    if ((args.size() == 1 || args.size() == 2) && args[0] != "-")
        m_pxeHost = args[0];
    if (args.size() == 2) {
        m_preChoice = args[1];
        m_quiet = true;
    }

    return true;
}

/* -------------------------------------------------------------------------- */
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
        vector<NetworkInterface> ifs = nh.getInterfaces();
        if (ifs.size() < 1)
            throw ApplicationError("No network interfaces found");

        // use the first interface that is up
        netif = ifs[0];
    }
    Debug::debug()->trace("Using interface '%s'", netif.getName().c_str());

    string pxe_mac = string("01-") + netif.getMac(NetworkInterface::MF_LOWERCASE |
            NetworkInterface::MF_DASH);
    string pxe_ip = netif.getIp(NetworkInterface::IF_HEX);

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

    stringstream ss;
    SimpleNotifier notifier;
    for (int i = 0; i < 10; i++) {
        string url = m_protocol + "://" + m_pxeHost + "/pxelinux.cfg/" + names[i];

        Debug::debug()->trace("Trying to retrieve %s", url.c_str());
        if (!m_quiet)
            cout << "Trying " << "pxelinux.cfg/" << names[i] << " ";

        try {
            Downloader dl(ss, CONNECTION_TIMEOUT);
            dl.setUrl(url);
            if (!m_quiet)
                dl.setProgress(&notifier);
            dl.download();
            break;
        } catch (const DownloadError &err) {
            Debug::debug()->trace("DownloadError: %s", err.what());

            if (err.getErrorcode() == DownloadError::DEC_CONNECTION_FAILED) {
                cerr << "Connection to " << m_pxeHost << " with protocol " << m_protocol
                     << " failed. Aborting." << endl;
                if (m_protocol == "tftp") {
                    cerr << endl;
                    cerr << "HINT: This failure could be because of Firewall settings. "
                            "Check your Firewall " << endl;
                    cerr << "configuration. If your FTP server has the same directory layout "
                         << "as your TFTP" << endl;
                    cerr << "server, you may also consider the '-F' option." << endl;
                    cerr << endl;
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
        throw ApplicationError(string("Parsing PXE config file failed: ") +
                pe.what());
    }
}

/* -------------------------------------------------------------------------- */
bool PxeKexec::checkEnv()
{
    int ret = system("kexec -h &>/dev/null");
    if (WEXITSTATUS(ret) != 0) {
        cout << "Error: kexec-tools are not installed." << endl;
        return false;
    }

    return true;
}

/* -------------------------------------------------------------------------- */
void PxeKexec::displayMessage()
{
    if (!m_quiet) {
        cout << "\e[2J\e[0;0H";
        cout << m_pxeConfig.getMessage() << endl << endl << endl;
    }
}

/* -------------------------------------------------------------------------- */
bool PxeKexec::chooseEntry()
{
    string choice;

    if (m_preChoice.size() != 0)
        m_choice = m_pxeConfig.getEntry(m_preChoice);

    m_lineReader->setCompletor(this);
    while (!m_lineReader->eof() && !m_choice.isValid()) {
        choice = strip(m_lineReader->readLine());
        if (choice.size() == 0 || choice == "quit" || choice == "exit") {
            m_lineReader->setCompletor(NULL);
            return false;
        }
        m_choice = m_pxeConfig.getEntry(choice);
        if (!m_choice.isValid())
            cout << "Entry " << choice << " does not exist." << endl;
    }

    m_lineReader->setCompletor(NULL);
    return m_choice.isValid();
}

/* -------------------------------------------------------------------------- */
StringVector PxeKexec::complete(const string &text, const string &full_text,
        size_t start_idx, ssize_t end_idx)
{
    StringVector names = m_pxeConfig.getEntryNames();
    StringVector result;

    for (StringVector::const_iterator it = names.begin();
            it != names.end(); ++it)
    {
        string name = *it;
        if (strncasecmp(name.c_str(), text.c_str(), text.size()) == 0)
            result.push_back(name);
    }

    return result;
}

/* -------------------------------------------------------------------------- */
bool PxeKexec::confirmBoot()
{
begin:
    if (!(m_noconfirm && m_quiet)) {
        cout << "Booting following entry:" << endl;
        cout << "Kernel   : " << m_choice.getKernel() << endl;
        cout << "Initrd   : " << m_choice.getInitrd() << endl;
        cout << "Append   : " << m_choice.getAppend() << endl;
        cout << endl;
    }

    if (m_noconfirm)
        return true;

    cout << "Continue? [Y/n/e] ";
again:
    cout.flush();

    string ret;
    getline(cin, ret, '\n');
    if (ret.size() == 0 || ret[0] == '\n')
        return true;

    if (ret[0] == 'e' || ret[0] == 'E') {
        m_choice.setAppend(m_lineReader->editLine(m_choice.getAppend().c_str()));
        cout << endl << endl;
        goto begin;
    }

    switch (rpmatch(ret.c_str())) {
    case 0: /* no */
        return false;

    case 1: /* yes */
        return true;

    default: /* invalid */
        cout << "Invalid input. Try again [Y/n/e] ";
        goto again;
    }

    return false;
}

/* -------------------------------------------------------------------------- */
void PxeKexec::downloadStuff()
    throw (ApplicationError)
{
    string tmpdir = getenv("TMPDIR") ? getenv("TMPDIR") : "/tmp";
    tmpdir += "/";

    string kernel = tmpdir + "pxe-kexec-kernel";
    string initrd = tmpdir + "pxe-kexec-initrd";

    struct stat statbuf;
    while (stat(kernel.c_str(), &statbuf) >= 0)
        kernel += "_";

    while (stat(initrd.c_str(), &statbuf) >= 0)
        initrd += "_";

    SimpleNotifier notifier;
    string url;
    try {
        cout << "Downloading kernel ";
        ofstream os(kernel.c_str(), ios::binary);
        Downloader dl(os);
        url = m_protocol + "://" + m_pxeHost + "/" + m_choice.getKernel();
        dl.setUrl(url);
        dl.setProgress(&notifier);
        dl.download();
        os.close();
        m_downloadedKernel = kernel;
    } catch (const DownloadError &err) {
        throw ApplicationError("Downloading kernel "+ url
                +" failed: " + string(err.what()));
    }

    if (m_choice.getInitrd().size() > 0) {
        try {
            cout << "Downloading initrd ";
            ofstream os(initrd.c_str(), ios::binary);
            Downloader dl(os);
            url = m_protocol + "://" + m_pxeHost + "/" + m_choice.getInitrd();
            dl.setUrl(url);
            dl.setProgress(&notifier);
            dl.download();
            os.close();
            m_downloadedInitrd = initrd;
        } catch (const DownloadError &err) {
            throw ApplicationError("Downloading initrd "+url +
                    " failed: " + string(err.what()));
        }
    }
}

/* -------------------------------------------------------------------------- */
void PxeKexec::deleteKernels()
{
    if (m_nodelete)
        return;

    // delete kernel and initrd since they have been loaded
    if (m_downloadedInitrd.size() > 0) {
        if (remove(m_downloadedKernel.c_str()) != 0)
            Debug::debug()->info("Removal of %s failed.",
                    m_downloadedKernel.c_str());
    }

    if (m_downloadedInitrd.size() > 0) {
        if (remove(m_downloadedInitrd.c_str()) != 0)
            Debug::debug()->info("Removal of %s failed.",
                    m_downloadedInitrd.c_str());
    }

}

/* -------------------------------------------------------------------------- */
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

    /* try to change VT */
    ke.prepareConsole();

    if (m_dryRun)
        return;

    if (!ke.execute())
        throw ApplicationError("Executing kernel failed");
}

/* }}} */

// vim: set sw=4 ts=4 fdm=marker et:
