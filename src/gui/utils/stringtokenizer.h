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
#ifndef __STRING_TOKENIZER_H__
#define __STRING_TOKENIZER_H__
#include <vector>
#include <string>
namespace cdroid{
class StringTokenizer{
private:
    int mCurrentPosition;
    int mNewPosition;
    int mMaxPosition;
    std::string mStr;
    std::string mDelimiters;
    bool mRetDelims;
    bool mDelimsChanged;
    int maxDelimCodePoint;
    bool mHasSurrogates = false;
    std::vector<int> mDelimiterCodePoints;
private:
    void setMaxDelimCodePoint();
    int skipDelimiters(int startPos)const;
    int scanToken(int startPos)const;
    bool isDelimiter(int codePoint)const;
public:
    StringTokenizer(const std::string& str,const std::string& delim, bool returnDelims);
    StringTokenizer(const std::string& str,const std::string& delim);
    StringTokenizer(const std::string& str);
    bool hasMoreTokens();
    std::string nextToken();
    std::string nextToken(const std::string& delim);
    bool hasMoreElements();
    std::string nextElement();
    int countTokens()const;
};
}/*endof namespace*/
#endif/*__STRING_TOKENIZER_H__*/
