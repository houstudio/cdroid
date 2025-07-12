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
#ifndef __TEXT_UTILS_H__
#define __TEXT_UTILS_H__
#include <string>
#include <vector>
namespace cdroid{

class TextUtils{
public:
    static const char*UCSWCHAR();
    static const char*UCS16();
    static const std::string utf16_utf8(const unsigned short*utf16,int len);
    static const std::wstring utf8tounicode(const std::string&utf8);
    static const std::u16string utf8_utf16(const std::string&utf8);
    static const std::string unicode2utf8(const std::wstring&);
    static bool isEmpty(const std::string&);
    static bool startWith(const std::string&str,const std::string&head);
    static bool endWith(const std::string&str,const std::string&tail);
    static std::string join(const std::string&sep,const std::vector<std::string>&parts);
    static std::string& trim(std::string&);
    static std::string& replace(std::string&str,const std::string&sfind,const std::string&sreplace);
    static long strtol(const std::string&value);
    static std::vector<std::string> split(const std::string& s,const std::string& delim);
    static std::vector<std::string> split(const std::string& s,int delim);
    static int UCS2UTF(wchar_t wc,char*oututf,int outlen);
    static int UTF2UCS(const char*utf,wchar_t*unicode);
};
}
#endif

