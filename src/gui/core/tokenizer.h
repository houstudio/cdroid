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
#ifndef _UTILS_TOKENIZER_H
#define _UTILS_TOKENIZER_H

#include <string>
#include <iostream>

namespace cdroid {

/** A simple tokenizer for loading and parsing ASCII text files line by line.*/

class Tokenizer {
    Tokenizer(const std::string&name,char* buffer,bool ownBuffer, size_t length);
public:
    ~Tokenizer();

    /**
     * Opens a file and maps it into memory.
     *
     * Returns NO_ERROR and a tokenizer for the file, if successful.
     * Otherwise returns an error and sets outTokenizer to NULL.
     */
    static int fromStream(const std::string&name,std::istream& is, Tokenizer** outTokenizer);

    /**
     * Prepares to tokenize the contents of a string.
     *
     * Returns NO_ERROR and a tokenizer for the string, if successful.
     * Otherwise returns an error and sets outTokenizer to NULL.
     */
    static int fromContents(const std::string&name,const char* contents, Tokenizer** outTokenizer);

    /**
     * Returns true if at the end of the file.
     */
     bool isEof() const { return mCurrent == getEnd(); }

    /**
     * Returns true if at the end of the line or end of the file.
     */
     bool isEol() const { return isEof() || *mCurrent == '\n'; }

     const std::string& getName()const{return mName;} 
    /**
     * Gets a 1-based line number index for the current position.
     */
     int32_t getLineNumber() const { return mLineNumber; }

    /**
     * Formats a location string consisting of the filename and current line number.
     * Returns a string like "MyFile.txt:33".
     */
    std::string getLocation() const;

    /**
     * Gets the character at the current position.
     * Returns null at end of file.
     */
     char peekChar() const { return isEof() ? '\0' : *mCurrent; }

    /**
     * Gets the remainder of the current line as a string, excluding the newline character.
     */
    std::string peekRemainderOfLine() const;

    /**
     * Gets the character at the current position and advances past it.
     * Returns null at end of file.
     */
     char nextChar() { return isEof() ? '\0' : *(mCurrent++); }

    /**
     * Gets the next token on this line stopping at the specified delimiters
     * or the end of the line whichever comes first and advances past it.
     * Also stops at embedded nulls.
     * Returns the token or an empty string if the current character is a delimiter
     * or is at the end of the line.
     */
    std::string nextToken(const char* delimiters);

    /**
     * Advances to the next line.
     * Does nothing if already at the end of the file.
     */
    void nextLine();

    /**
     * Skips over the specified delimiters in the line.
     * Also skips embedded nulls.
     */
    void skipDelimiters(const char* delimiters);

private:
    Tokenizer(const Tokenizer& other); // not copyable
    std::string mName;
    char* mBuffer;
    bool mOwnBuffer;
    size_t mLength;
    const char* mCurrent;
    int32_t mLineNumber;

    const char* getEnd() const { return mBuffer + mLength; }
};

} // namespace cdroid

#endif // _UTILS_TOKENIZER_H
