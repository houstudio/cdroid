/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *********************************************************************************/
#include <tokenizer.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <cdtypes.h>
#include <cdlog.h>
#include <errno.h>
#define DEBUG_TOKENIZER 0

namespace cdroid {

static inline bool isDelimiter(char ch, const char* delimiters) {
    return strchr(delimiters, ch) != NULL;
}

Tokenizer::Tokenizer(const std::string&name,char* buffer, bool ownBuffer, size_t length) :
        mName(name),mBuffer(buffer), mOwnBuffer(ownBuffer), 
        mLength(length), mCurrent(buffer), mLineNumber(1) {
}

Tokenizer::~Tokenizer() {
    if (mOwnBuffer) {
        delete[] mBuffer;
    }
}

int Tokenizer::fromStream(const std::string&name,std::istream&is, Tokenizer** outTokenizer) {
    size_t length;
    int result = 0;
    *outTokenizer = NULL;
    if (!is.good()) {
        LOGE("Error stream %s not ready!",name.c_str());
        return -1;
    }
    is.seekg(0,std::ios::end); 
    length = is.tellg();
    is.seekg(0,std::ios::beg);
    // Fall back to reading into a buffer since we can't mmap files in sysfs.
    // The length we obtained from stat is wrong too (it will always be 4096)
    // so we must trust that read will read the entire file.
    char*buffer = new char[length];
    if (!is.read(buffer, length)) {
        //result = -errno;
        LOGE("Error reading stream:%s!",name.c_str());
        delete[] buffer;
        return -1;
    }
    *outTokenizer = new Tokenizer(name, buffer,true, length);
    return result;
}

int Tokenizer::fromContents(const std::string&name,const char* contents, Tokenizer** outTokenizer) {
    *outTokenizer = new Tokenizer(name,const_cast<char*>(contents), false, strlen(contents));
    return 0;
}

std::string Tokenizer::getLocation() const {
    std::string result=mName;
    result +=":";
    result += std::to_string(mLineNumber);
    return result;
}

std::string Tokenizer::peekRemainderOfLine() const {
    const char* end = getEnd();
    const char* eol = mCurrent;
    while (eol != end) {
        char ch = *eol;
        if (ch == '\n') {
            break;
        }
        eol += 1;
    }
    return std::string(mCurrent, eol - mCurrent);
}

std::string Tokenizer::nextToken(const char* delimiters) {
#if DEBUG_TOKENIZER
    LOGD("nextToken");
#endif
    const char* end = getEnd();
    const char* tokenStart = mCurrent;
    while (mCurrent != end) {
        char ch = *mCurrent;
        if (ch == '\n' || isDelimiter(ch, delimiters)) {
            break;
        }
        mCurrent += 1;
    }
    return std::string(tokenStart, mCurrent - tokenStart);
}

void Tokenizer::nextLine() {
#if DEBUG_TOKENIZER
    LOGD("nextLine");
#endif
    const char* end = getEnd();
    while (mCurrent != end) {
        char ch = *(mCurrent++);
        if (ch == '\n') {
            mLineNumber += 1;
            break;
        }
    }
}

void Tokenizer::skipDelimiters(const char* delimiters) {
#if DEBUG_TOKENIZER
    LOGD("skipDelimiters");
#endif
    const char* end = getEnd();
    while (mCurrent != end) {
        char ch = *mCurrent;
        if (ch == '\n' || !isDelimiter(ch, delimiters)) {
            break;
        }
        mCurrent += 1;
    }
}

} // namespace cdroid
