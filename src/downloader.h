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
#ifndef DOWNLOADER_H
#define DOWNLOADER_H

#include <stdexcept>
#include <ostream>
#include <vector>

#include <curl/curl.h>

#include "global.h"

/* DownloadError {{{ */

class DownloadError : public std::runtime_error {
    public:
        enum DownloadErrorCode {
            DEC_UNKNOWN,                    // don't know an exact reason, default
            DEC_CONNECTION_FAILED           // connection failed in CURL, maybe timeout
        };

    public:
        DownloadError(const std::string& string)
            : std::runtime_error(string), m_errorcode(DEC_UNKNOWN) {}

        void setErrorcode(DownloadErrorCode error) { m_errorcode = error; }
        DownloadErrorCode getErrorcode() const     { return m_errorcode;  }

    private:
        DownloadErrorCode m_errorcode;
};

/* }}} */
/* ProgressNotifier {{{ */

class ProgressNotifier {
    public:
        virtual ~ProgressNotifier() {}

    public:
        virtual int progressed(double total, double now) = 0;
        virtual void finished() = 0;
};

/* }}} */
/* Downloader {{{ */

class Downloader {
    public:
        Downloader(std::ostream &output, long timeout = 0) throw (DownloadError);
        virtual ~Downloader();

    public:
        void setUrl(const std::string &url) throw (DownloadError);
        std::string getUrl() const;

        void setProgress(ProgressNotifier *notifier);
        void download() throw (DownloadError);

    protected:
        static int curl_progress_callback(void *clientp, double dltotal,
                double dlnow, double ultotal, double ulnow);
        static size_t curl_write_callback(void *buffer, size_t size,
                size_t nmemb, void *userp);

    private:
        ProgressNotifier  *m_notifier;
        std::string       m_url;
        CURL              *m_curl;
        char              m_curl_errorstring[CURL_ERROR_SIZE];
        std::ostream      &m_output;
        static bool       m_firstCalled;
};

/* }}} */

#endif /* DOWNLOADER_H */

// vim: set sw=4 ts=4 fdm=marker et:
