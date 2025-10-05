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
#include <utils/stringtokenizer.h>
//#include <utils/textutils.h>
namespace cdroid{

namespace Character{
static constexpr int MIN_HIGH_SURROGATE=0xD800;
static constexpr int MAX_LOW_SURROGATE =0xDBFF;

int charCount(int unicodeValue) {
    // 单字节字符
    if (unicodeValue >= 0 && unicodeValue <= 0x7F) {
        return 1;
    }// 双字节字符
    else if (unicodeValue >= 0x80 && unicodeValue <= 0x7FF) {
        return 2;
    }// 三字节字符
    else if (unicodeValue >= 0x800 && unicodeValue <= 0xFFFF) {
        return 3;
    }// 四字节字符
    else if (unicodeValue >= 0x10000 && unicodeValue <= 0x10FFFF) {
        return 4;
    }// 非法的Unicode值
    else {
        return -1;
    }
}

int codePointAt(const std::string&str,int i){
    const uint8_t startByte = str.at(i);
    int charLength = 0;

    if((startByte&0xC0)!=0x80)return -1;

    if ((startByte & 0x80) == 0) {  // 单字节字符
        charLength = 1;
    } else if ((startByte & 0xE0) == 0xC0) {  // 双字节字符
        charLength = 2;
    } else if ((startByte & 0xF0) == 0xE0) {  // 三字节字符
        charLength = 3;
    } else if ((startByte & 0xF8) == 0xF0) {  // 四字节字符
        charLength = 4;
    }

    int codePoint = (str[i] & (0xFF >> charLength)) << (6 * (charLength - 1));
    for (int j = 1; j < charLength; ++j) {
        if (i+j < str.length()) {
            codePoint |= (str[i+j] & 0x3F) << (6 * (charLength - 1 - j));
        } else {
            return -1; // 不完整的UTF-8编码
        }
    }
    return -1;
}
}

void StringTokenizer::setMaxDelimCodePoint() {
    if (mDelimiters.empty()) {
        maxDelimCodePoint = 0;
        return;
    }

    int m = 0;
    int c;
    int count = 0;
    for (int i = 0; i < mDelimiters.length(); i += Character::charCount(c)) {
        c = mDelimiters.at(i);
        if (c >= Character::MIN_HIGH_SURROGATE && c <= Character::MAX_LOW_SURROGATE) {
            c = Character::codePointAt(mDelimiters,i);//mDelimiters.codePointAt(i);
            mHasSurrogates = true;
        }
        if (m < c)
            m = c;
        count++;
    }
    maxDelimCodePoint = m;

    if (mHasSurrogates) {
        mDelimiterCodePoints.resize(count);
        for (int i = 0, j = 0; i < count; i++, j += Character::charCount(c)) {
            c = Character::codePointAt(mDelimiters,j);//mDelimiters.codePointAt(j);
            mDelimiterCodePoints[i] = c;
        }
    }
}

StringTokenizer::StringTokenizer(const std::string& str,const std::string& delim, bool returnDelims) {
    mCurrentPosition = 0;
    mNewPosition = -1;
    mDelimsChanged = false;
    mStr = str;
    mDelimiters = delim;
    mMaxPosition = mStr.length();
    mRetDelims = returnDelims;
    mHasSurrogates = false;
    setMaxDelimCodePoint();
}

StringTokenizer::StringTokenizer(const std::string& str,const std::string& delim)
    :StringTokenizer(str, delim, false){
}

StringTokenizer::StringTokenizer(const std::string& str)
   :StringTokenizer(str, " \t\n\r\f", false){
}

int StringTokenizer::skipDelimiters(int startPos)const{
    if (mDelimiters.empty())
        throw "new NullPointerException()";

    int position = startPos;
    while (!mRetDelims && position < mMaxPosition) {
        if (!mHasSurrogates) {
            char c = mStr.at(position);
            if ((c > maxDelimCodePoint) || (mDelimiters.find(c)!=std::string::npos))//indexOf(c) < 0))
                break;
            position++;
        } else {
            int c = Character::codePointAt(mStr,position);//mStr.codePointAt(position)
            if ((c > maxDelimCodePoint) || !isDelimiter(c)) {
                break;
            }
            position += Character::charCount(c);
        }
    }
    return position;
}

int StringTokenizer::scanToken(int startPos)const{
    int position = startPos;
    while (position < mMaxPosition) {
        if (!mHasSurrogates) {
            char c = mStr.at(position);
            if ((c <= maxDelimCodePoint) && (mDelimiters.find(c)>=0))//indexOf(c) >= 0))
                break;
            position++;
        } else {
            int c = Character::codePointAt(mStr,position);//mStr.codePointAt(position);
            if ((c <= maxDelimCodePoint) && isDelimiter(c))
                break;
            position += Character::charCount(c);
        }
    }
    if (mRetDelims && (startPos == position)) {
        if (!mHasSurrogates) {
            char c = mStr.at(position);//charAt(position);
            if ((c <= maxDelimCodePoint) && (mDelimiters.find(c)>=0))//indexOf(c) >= 0))
                position++;
        } else {
            int c = Character::codePointAt(mStr,position);//mStr.codePointAt(position);
            if ((c <= maxDelimCodePoint) && isDelimiter(c))
                position += Character::charCount(c);
        }
    }
    return position;
}

bool StringTokenizer::isDelimiter(int codePoint)const{
    for (int delimiterCodePoint : mDelimiterCodePoints) {
        if (delimiterCodePoint == codePoint) {
            return true;
        }
    }
    return false;
}

bool StringTokenizer::hasMoreTokens(){

    mNewPosition = skipDelimiters(mCurrentPosition);
    return (mNewPosition < mMaxPosition);
}

std::string StringTokenizer::nextToken() {

    mCurrentPosition = (mNewPosition >= 0 && !mDelimsChanged) ?
        mNewPosition : skipDelimiters(mCurrentPosition);

    /* Reset these anyway */
    mDelimsChanged = false;
    mNewPosition = -1;

    if (mCurrentPosition >= mMaxPosition)
        throw "new NoSuchElementException()";
    const int start = mCurrentPosition;
    mCurrentPosition = scanToken(mCurrentPosition);
    return mStr.substr(start, mCurrentPosition);
}

std::string StringTokenizer::nextToken(const std::string& delim) {
    mDelimiters = delim;

    /* delimiter string specified, so set the appropriate flag. */
    mDelimsChanged = true;

    setMaxDelimCodePoint();
    return nextToken();
}

bool StringTokenizer::hasMoreElements(){
    return hasMoreTokens();
}

std::string StringTokenizer::nextElement() {
    return nextToken();
}

int StringTokenizer::countTokens()const{
    int count = 0;
    int currpos = mCurrentPosition;
    while (currpos < mMaxPosition) {
        currpos = skipDelimiters(currpos);
        if (currpos >= mMaxPosition)
            break;
        currpos = scanToken(currpos);
        count++;
    }
    return count;
}
}
