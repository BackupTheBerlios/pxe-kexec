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

/**
 * @file Downloader.h
 * @brief CURL Download classes
 *
 * This files contains all files needed to download a file. The list of
 * supported protocol depends on the CURL version that is available on the
 * system.
 *
 * @author Bernhard Walle <bernhard.walle@gmx.de>
 */

#include <stdexcept>
#include <ostream>
#include <vector>

#include <curl/curl.h>

#include "global.h"

/* DownloadError {{{ */

/**
 * @brief Exception for download errors
 *
 * This class is the exception that is thrown on download errors. This
 * exception is derived from std::runtime_error, so you can use the
 * std::runtime_error::what() method to determine the cause of the failure.
 *
 * @author Bernhard Walle <bernhard.walle@gmx.de>
 */
class DownloadError : public std::runtime_error {
    public:
        /**
         * List of available potential errors.
         */
        enum DownloadErrorCode {
            DEC_UNKNOWN,                    /**< don't know an exact reason, default */
            DEC_CONNECTION_FAILED           /**< connection failed in CURL, maybe timeout */
        };

    public:
        /**
         * @brief Constructor
         *
         * Creates a new DownloadError from an error string.
         *
         * @param[in] string the error string
         */
        DownloadError(const std::string& string)
            : std::runtime_error(string), m_errorcode(DEC_UNKNOWN) {}

        /**
         * @brief Set an error code
         *
         * Additional to the error string supplied in
         * DownloadError::DownloadError(const std::string &), you can set an error
         * code here.
         *
         * @param[in] error the error code, see DownloadErrorCode for a list
         *            of available error codes
         */
        void setErrorcode(DownloadErrorCode error) { m_errorcode = error; }

        /**
         * @brief Return the error code
         *
         * Return the error code that has been set with setErrorcode().
         *
         * @return the error code, defaults to DownloadError::DEC_UNKNOWN if no error code
         *         has been set
         */
        DownloadErrorCode getErrorcode() const     { return m_errorcode;  }

    private:
        DownloadErrorCode m_errorcode;
};

/* }}} */
/* ProgressNotifier {{{ */

/**
 * @brief Interface to indicate the process of a dowload
 *
 * This is an interface that must be implemented in order to display the
 * progress of a download performed by Downloader to the user. The
 * implementation could be a simple status bar on console or update a GUI
 * progress bar. 
 *
 * Please note that in multi-thread environments, the synchronisation must be
 * done by the implementation of ProgressNotifier.
 *
 * @author Bernhard Walle <bernhard.walle@gmx.de>
 */
class ProgressNotifier {
    public:
        /**
         * @brief Destructor
         *
         * This is the destructor of the class.
         */
        virtual ~ProgressNotifier() {}

    public:
        /**
         * @brief Callback function for progress updates
         *
         * This is the callback function that is called on progress updates.
         * The interval in which the callback function is called cannot be
         * determined in advance. If your update is slow, you have to
         * implement some timing mechanisms that just does nothing if
         * progressed() is called too often.
         *
         * You can calculate the percentage of the progress by dividing @p now
         * by @p total and multiply the result with 100:
         *
         * @code
         * double percent = now / total * 100;
         * @endcode
         *
         * Please note that we cannot guarantee that the function is called on
         * 100 % because of rounding issues. Use the finished() callback below
         * to display the "100 %" and to perform cleanup actions.
         *
         * @param[in] total the total number as explained above
         * @param[in] now the current number as explained above
         * @return the return value has currently no meaning, any
         *         implementation should return true or 1.
         */
        virtual int progressed(double total, double now) = 0;

        /**
         * @brief Callback function for the finish action
         *
         * See progressed(double, double). Because we cannot guarantee that
         * the progressed() function is called with @c total == @c now, it's
         * guaranteed that after the last call of progressed() this method is
         * called exactly once. So any cleanup action like enabling the "Next"
         * button of a wizard after downloading has been completed can be
         * done here.
         */
        virtual void finished() = 0;
};

/* }}} */
/* Downloader {{{ */

/**
 * @brief Download a file with CURL
 *
 * This class can be used to download a file with all protocols supported by
 * CURL on the system. This is normally HTTP, FTP and TFTP (and many others
 * like HTTPS).
 *
 * Example to download a file:
 *
 * @code
 * std::ofstream os(filename.c_str(), ios::binary);
 * Downloader dl(os, 1000);
 * dl.setUrl("http://www.bla.org/fasel.txt");
 * dl.download();
 * os.close();
 * @endcode
 *
 *
 * @author Bernhard Walle <bernhard.walle@gmx.de>
 */
class Downloader {
    public:
        /**
         * @brief Constructor
         *
         * Creates a new instance of a Downloader.
         *
         * @todo is that timeout parameter really milliseconds
         * @param[out] output the stream where the output is written to
         * @param[in]  timeout the number of milliseconds after which the
         *             network connection should timeout
         * @exception DownloadError on CURL errors
         */
        Downloader(std::ostream &output, long timeout = 0) throw (DownloadError);

        /**
         * @brief Destructor
         *
         * Deletes a Downloader.
         */
        virtual ~Downloader();

    public:
        /**
         * @brief Sets the URL to download
         *
         * Set the URL to download. Example URLs:
         *
         *  - <tt>http://www.test.org/bla.txt</tt> for HTTP
         *  - <tt>ftp://fileserver.net/fasel.mp3</tt> for FTP
         *
         * See the documentation of your system's CURL implementation about
         * which protocols are supported.
         *
         * @param[in] url the URL as shown above
         * @throw DownloadError if the URL was invalid
         */
        void setUrl(const std::string &url) throw (DownloadError);

        /**
         * @brief Returns the URL
         *
         * Returns the URL that has been set with setUrl().
         *
         * @return the URL or the empty string if setUrl() was not called
         */
        std::string getUrl() const;

        /**
         * @brief Sets the progress notifier
         *
         * Sets a progress notification object. See the documentation about
         * ProgressNotifier how to implement such a download progress
         * notification properly. 
         *
         * The memory where @p notifier points to is not managed by the
         * Downloader. That means that you have to delete @p notifier yourself
         * after the download has been finished, i.e. after download()
         * returned and the ProgressNotifier::finished() function has been
         * called.
         *
         * @param[in] notifier a pointer to the progress notification object
         */
        void setProgress(ProgressNotifier *notifier);
        
        /**
         * @brief Performs the download
         *
         * This function actually performs the download. If the function
         * returns, the download was successful.
         *
         * @throw DownloadError if downloading fails for any reason
         */
        void download() throw (DownloadError);

    private:
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
