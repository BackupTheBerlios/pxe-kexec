/*
 * Copyright (c) 2008-2009, Bernhard Walle <bernhard@bwalle.de>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the <organization> nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY <copyright holder> ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <copyright holder> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <string>
#include <iostream>
#include <cerrno>
#include <cstring>
#include <cstdlib>
#include <algorithm>
#include <vector>

#include "bwconfig.h"

#if HAVE_LIBREADLINE
# include <readline/readline.h>
# include <readline/history.h>
#endif

#include "completion.h"
#include "stringutil.h"

using std::string;
using std::getline;
using std::cin;
using std::cout;
using std::endl;
using std::strerror;
using std::free;
using std::min;
using std::vector;

/* class definitions {{{ */

#if HAVE_LIBREADLINE
class ReadlineLineReader : public AbstractLineReader {
    public:
        ReadlineLineReader(const string &prompt);

    public:
        string readLine(const char *prompt = NULL);
        void readHistory(const std::string &file)
            throw (IOError);
        void writeHistory(const std::string &file)
            throw (IOError);
        bool haveHistory() const;
        bool canEditLine() const;
        string editLine(const char *oldLine);

        bool haveCompletion() const;
        void setCompletor(Completor *comp);

    private:
        Completor *m_completor;
};
#endif

/**
 * @brief Simple line reader without history and completion
 *
 * This line reader implementation is simple: You cannot use the cursor keys, it has
 * no history and also no completion.
 *
 * @author Bernhard Walle <bernhard@bwalle.de>
 */
class SimpleLineReader : public AbstractLineReader {
    public:
        /**
         * @brief Constructor
         *
         * Creates a new instance of a SimpleLineReader.
         *
         * @param[in] prompt the prompt of the line reader
         */
        SimpleLineReader(const string &prompt);

    public:
        /**
         * @copydoc LineReader::readLine()
         */
        string readLine(const char *prompt = NULL);
};

/* }}} */
/* LineReader {{{ */

/* ---------------------------------------------------------------------------------------------- */
LineReader *LineReader::defaultLineReader(const string &prompt)
{
#if HAVE_LIBREADLINE
    return new ReadlineLineReader(prompt);
#else
    return new SimpleLineReader(prompt);
#endif
}

/* }}} */
/* AbstractLineReader {{{ */

/* ---------------------------------------------------------------------------------------------- */
AbstractLineReader::AbstractLineReader(const string &prompt)
    : m_prompt(prompt), m_eof(false)
{}

/* ---------------------------------------------------------------------------------------------- */
string AbstractLineReader::getPrompt() const
{
    return m_prompt;
}

/* ---------------------------------------------------------------------------------------------- */
void AbstractLineReader::setEof(bool eof)
{
    m_eof = eof;
}

/* ---------------------------------------------------------------------------------------------- */
bool AbstractLineReader::eof() const
{
    return m_eof;
}

/* ---------------------------------------------------------------------------------------------- */
void AbstractLineReader::readHistory(const std::string &file)
    throw (IOError)
{}

/* ---------------------------------------------------------------------------------------------- */
void AbstractLineReader::writeHistory(const std::string &file)
    throw (IOError)
{}

/* ---------------------------------------------------------------------------------------------- */
bool AbstractLineReader::haveHistory() const
{
    return false;
}

/* ---------------------------------------------------------------------------------------------- */
bool AbstractLineReader::canEditLine() const
{
    return false;
}

/* ---------------------------------------------------------------------------------------------- */
string AbstractLineReader::editLine(const char *line)
{
    return line;
}

/* ---------------------------------------------------------------------------------------------- */
bool AbstractLineReader::haveCompletion() const
{
    return false;
}

/* ---------------------------------------------------------------------------------------------- */
void AbstractLineReader::setCompletor(Completor *comp)
{}

/* }}} */
/* SimpleLineReader {{{ */

/* ---------------------------------------------------------------------------------------------- */
SimpleLineReader::SimpleLineReader(const string &prompt)
    : AbstractLineReader(prompt)
{}

/* ---------------------------------------------------------------------------------------------- */
string SimpleLineReader::readLine(const char *prompt)
{
    string ret;
    if (!prompt)
        cout << getPrompt();
    else
        cout << prompt;
    getline(cin, ret, '\n');
    if (cout.eof())
        setEof(true);
    return ret;
}

/* }}} */
/* ReadlineLineReader {{{ */

#if HAVE_LIBREADLINE

/* completion stuff, not really object oriented :-( */

/* ---------------------------------------------------------------------------------------------- */
Completor *g_current_completor;

/* ---------------------------------------------------------------------------------------------- */
char **readline_line_reader_complete(const char *text, int start, int end)
{
    vector<string> completions = g_current_completor->complete(
            text, string(rl_line_buffer), start, end);

    if (completions.size() == 0)
        return NULL;

    // the first entry is the string which replaces text, so it must be
    // the largest string which is common to each entry in completions
    string replacement = completions[0];
    for (vector<string>::const_iterator it = completions.begin();
            it != completions.end(); ++it) {
        size_t len = min(it->size(), replacement.size());
        for (unsigned int mismatch = 0; mismatch < len; ++mismatch) {
            if ((*it)[mismatch] != replacement[mismatch]) {
                replacement = replacement.substr(0, mismatch);
                break;
            }
        }
    }

    completions.insert(completions.begin(), replacement);

    return stringvector_to_array(completions);
}

/* ---------------------------------------------------------------------------------------------- */
ReadlineLineReader::ReadlineLineReader(const string &prompt)
    : AbstractLineReader(prompt)
{}

/* ---------------------------------------------------------------------------------------------- */
string ReadlineLineReader::readLine(const char *prompt)
{
    char *line_read;
    string ret;

    line_read = readline(prompt ? prompt : getPrompt().c_str());
    if (!line_read)
        setEof(true);
    else if (*line_read) {
        if (!prompt)
            add_history(line_read);
        ret = string(line_read);
        free(line_read);
    }

    return ret;
}

/* ---------------------------------------------------------------------------------------------- */
bool ReadlineLineReader::canEditLine() const
{
    return true;
}

/* ---------------------------------------------------------------------------------------------- */
static string buffer;
static int my_rl_pre_input_hook()
{
    rl_replace_line(buffer.c_str(), 0);
    rl_redisplay();
    return 0;
}

/* ---------------------------------------------------------------------------------------------- */
string ReadlineLineReader::editLine(const char *line)
{
    char *line_read;
    string ret;

    buffer = string(line);
    rl_pre_input_hook = my_rl_pre_input_hook;
    line_read = readline(getPrompt().c_str());
    rl_pre_input_hook = NULL;
    if (!line_read)
        setEof(true);
    else if (*line_read) {
            add_history(line_read);
        ret = string(line_read);
        free(line_read);
    }

    return ret;
}

/* ---------------------------------------------------------------------------------------------- */
void ReadlineLineReader::readHistory(const std::string &file)
    throw (IOError)
{
    int ret = read_history(file.c_str());
    if (ret < 0)
        throw IOError(string("Reading readline history failed: ")
                + strerror(errno));
}

/* ---------------------------------------------------------------------------------------------- */
void ReadlineLineReader::writeHistory(const std::string &file)
    throw (IOError)
{
    int ret = write_history(file.c_str());
    if (ret < 0)
        throw IOError(string("Writing readline history failed: ")
                + strerror(errno));
}

/* ---------------------------------------------------------------------------------------------- */
bool ReadlineLineReader::haveHistory() const
{
    return true;
}

/* ---------------------------------------------------------------------------------------------- */
bool ReadlineLineReader::haveCompletion() const
{
    return true;
}

/* ---------------------------------------------------------------------------------------------- */
void ReadlineLineReader::setCompletor(Completor *comp)
{
    m_completor = comp;
    g_current_completor = m_completor;

    if (m_completor)
        rl_attempted_completion_function = readline_line_reader_complete;
    else
        rl_attempted_completion_function = NULL;
}

/* }}} */

#endif

// :tabSize=4:indentSize=4:noTabs=true:mode=c++:folding=explicit:collapseFolds=1:maxLineLen=100:
