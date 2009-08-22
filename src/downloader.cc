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
#include <stdexcept>
#include <ostream>

#include <curl/curl.h>

#include "downloader.h"
#include "debug.h"

using std::string;
using std::ostream;

bool Downloader::m_firstCalled = true;

#ifdef _WIN32
#  define CURL_GLOBAL_FLAGS   CURL_GLOBAL_WIN32
#else
#  define CURL_GLOBAL_FLAGS   0
#endif

/* -------------------------------------------------------------------------- */
size_t Downloader::curl_write_callback(void *buffer, size_t size,
        size_t nmemb, void *userp)
{
    Downloader *downloader = reinterpret_cast<Downloader *>(userp);

    downloader->m_output.write((char *)buffer, size * nmemb);
    Debug::debug()->dbg("Writing %d*%d=%d bytes (%d)", size, nmemb, size*nmemb,
            int(downloader->m_output.good()));

    if (downloader->m_output.good())
        return size * nmemb;
    else
        return 0;
}


/* -------------------------------------------------------------------------- */
int Downloader::curl_progress_callback(void *clientp, double dltotal, double
        dlnow, double ultotal, double ulnow)
{
    Downloader *downloader = reinterpret_cast<Downloader *>(clientp);

    (void)ultotal;
    (void)ulnow;

    if (downloader->m_notifier)
        downloader->m_notifier->progressed(dltotal, dlnow);

    return 0;
}


/* -------------------------------------------------------------------------- */
Downloader::Downloader(ostream &output, long timeout) throw (DownloadError)
    : m_notifier(NULL), m_output(output)
{
    CURLcode err;

    // perform CURL initialisation only once
    if (m_firstCalled) {
        curl_global_init(CURL_GLOBAL_FLAGS);
        m_firstCalled = false;
    }

    m_curl = curl_easy_init();
    if (!m_curl)
        throw DownloadError("curl_easy_init returned NULL");

    // error buffer
    err = curl_easy_setopt(m_curl, CURLOPT_ERRORBUFFER, m_curl_errorstring);
    if (err != CURLE_OK)
        throw DownloadError("CURLOPT_ERRORBUFFER failed");

    // timeout
    if (timeout != 0) {
        err = curl_easy_setopt(m_curl, CURLOPT_TIMEOUT, 5);
        if (err != CURLE_OK)
            throw DownloadError("CURLOPT_TIMEOUT failed");

        // disable signals
        err = curl_easy_setopt(m_curl, CURLOPT_NOSIGNAL, 1);
        if (err != CURLE_OK)
            throw DownloadError("CURLOPT_NOSIGNAL failed");
    }

    // write function
    err = curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION,
            Downloader::curl_write_callback);
    if (err != CURLE_OK)
        throw DownloadError(string("CURL error: ") + m_curl_errorstring);

    // write data
    err = curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, this);
    if (err != CURLE_OK)
        throw DownloadError(string("CURL error: ") + m_curl_errorstring);
}

/* -------------------------------------------------------------------------- */
Downloader::~Downloader()
{
    if (m_curl)
        curl_easy_cleanup(m_curl);
}

/* -------------------------------------------------------------------------- */
void Downloader::setUrl(const string &url) throw (DownloadError)
{
    CURLcode err;

    m_url = url;

    Debug::debug()->dbg("Setting URL to %s", m_url.c_str());
    err = curl_easy_setopt(m_curl, CURLOPT_URL, url.c_str());
    if (err != CURLE_OK)
        throw DownloadError(string("CURL error: ") + m_curl_errorstring);
}

/* -------------------------------------------------------------------------- */
string Downloader::getUrl() const
{
    return m_url;
}

/* -------------------------------------------------------------------------- */
void Downloader::setProgress(ProgressNotifier *notifier)
{
    CURLcode err;
    m_notifier = notifier;

    if (notifier) {
        err = curl_easy_setopt(m_curl, CURLOPT_PROGRESSFUNCTION,
                Downloader::curl_progress_callback);
        if (err != CURLE_OK)
            throw DownloadError(string("CURL error: ") + m_curl_errorstring);

        err = curl_easy_setopt(m_curl, CURLOPT_PROGRESSDATA, this);
        if (err != CURLE_OK)
            throw DownloadError(string("CURL error: ") + m_curl_errorstring);

        err = curl_easy_setopt(m_curl, CURLOPT_NOPROGRESS, false);
        if (err != CURLE_OK)
            throw DownloadError(string("CURL error: ") + m_curl_errorstring);
    } else {
        curl_easy_setopt(m_curl, CURLOPT_PROGRESSDATA, NULL);
        curl_easy_setopt(m_curl, CURLOPT_PROGRESSFUNCTION, NULL);
        curl_easy_setopt(m_curl, CURLOPT_NOPROGRESS, true);
    }
}

/* -------------------------------------------------------------------------- */
void Downloader::download() throw (DownloadError)
{
    CURLcode err;

    Debug::debug()->dbg("Performing download");
    err = curl_easy_perform(m_curl);
    if (m_notifier)
        m_notifier->finished();

    if (err != CURLE_OK) {
        DownloadError error(string("CURL error: ") + m_curl_errorstring);

        // timeout
        if (err == CURLE_COULDNT_CONNECT)
            error.setErrorcode(DownloadError::DEC_CONNECTION_FAILED);

        throw error;
    }
}


// vim: set sw=4 ts=4 et:
