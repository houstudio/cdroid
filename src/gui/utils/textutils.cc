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
#include <utils/textutils.h>
#include <string.h>
#include <cdtypes.h>
#include <cdlog.h>
#include <memory>
#ifdef USE_ICONV
#include <iconv.h>
#endif
namespace cdroid{
static int utf8ToUnicodeChar (unsigned char *ch, wchar_t *unicode);

#ifdef USE_ICONV
static int convert(const char*from_charset,const char*to_charset,const char*inbuf,size_t inlen,char*outbuf,size_t outlen){
    char **pin = (char**)&inbuf;
    char **pout = &outbuf;
    size_t outlen0=outlen;
    iconv_t cd = iconv_open(to_charset,from_charset);
    if (cd==0) return -1;
    memset(outbuf,0,outlen);
    iconv(cd, pin, &inlen,pout, &outlen);
    iconv_close(cd);
    return outlen0-outlen;
}
#endif

const char* TextUtils::UCSWCHAR(){
    union { unsigned short bom; unsigned char byte; } endian_test;
    endian_test.bom = 0xFFFE;
    if(sizeof(wchar_t) == 4 && endian_test.byte == 0xFE)
        return "UCS-4LE";
    else if(sizeof(wchar_t) == 2 && endian_test.byte == 0xFE)
        return "UCS-2LE";
    else if(sizeof(wchar_t) == 4 && endian_test.byte != 0xFE)
         return "UCS-4BE";
    else if(sizeof(wchar_t) == 2 && endian_test.byte != 0xFE)
        return "UCS-2BE";
    else
        return "ASCII";
}

const char*TextUtils::UCS16(){
    union { unsigned short bom; unsigned char byte; } endian_test;
    endian_test.bom = 0xFFFE;
    if(endian_test.byte == 0xFE)
        return "UCS-2LE";
    return "UCS-2BE";
}

const std::string TextUtils::utf16_utf8(const unsigned short*utf16,int len){
    std::unique_ptr<char[]>out(new char[len*4]);
    #ifdef USE_ICONV
    int rc = convert(UCS16(),"UTF-8",(const char*)utf16,len*2,out,len*4);
    LOGD_IF(rc==-1,"convert error");
    #else
    char*pout = out.get();
    for(int i=0;i<len;i++){
        int n=UCS2UTF(utf16[i],pout,4);
        pout+=n;
    }
    *pout = 0;
    #endif
    std::string u8s(out.get());
    return u8s; 
}

const std::wstring TextUtils::utf8tounicode(const std::string&utf8){
    size_t u8len = utf8.size() + 8;
    std::unique_ptr<wchar_t[]> out(new wchar_t[u8len]);
    #ifdef USE_ICONV
    int rc = convert("UTF-8",UCSWCHAR(),utf8.c_str(),utf8.size(),(char*)out,sizeof(wchar_t)*u8len);
    LOGD_IF(rc==-1,"convert error");
    #else
    wchar_t*pout = out.get();
    for(int i=0;i< utf8.length() ;){
        int n=UTF2UCS((utf8.c_str()+i),pout++);
        i+=n;
    }
    *pout = L'\0';
    #endif
    std::wstring u32s(out.get());
    return u32s;
}

const std::u16string TextUtils::utf8_utf16(const std::string&utf8){
    size_t u8len = utf8.size()+8;
    std::unique_ptr<char16_t[]> out(new char16_t[u8len]);
    #ifdef USE_ICONV
    int rc = convert("UTF-8",UCSWCHAR(),utf8.c_str(),utf8.size(),(char*)out,sizeof(wchar_t)*u8len);
    LOGD_IF(rc==-1,"convert error");
    #else
    char16_t*pout = out.get();
    for(int i = 0;i < utf8.length() ;){
	wchar_t oc;
        int n=UTF2UCS((utf8.c_str()+i),&oc);
	*pout++= oc;
        i += n;
    }
    *pout = 0;
    #endif
    std::u16string u16s(out.get());//,wcslen(out));
    return u16s;
}

const std::string TextUtils::unicode2utf8(const std::wstring&u32s){
    const int u8len = u32s.length()*4+8;
    std::unique_ptr<char[]>out(new char[u8len]);
    #ifdef USE_ICONV
    int rc = convert(UCSWCHAR(),"UTF-8",(char*)u32s.c_str(),u32s.length()*sizeof(wchar_t),out,u8len);
    LOGD_IF(rc==-1,"convert error");
    #else
    //static int ucs4ToUtf8 (unsigned char *s, wchar_t uc, int n)
    char*pout = out.get();
    for(int i = 0 ;i < u32s.length() ;i++){
        int n = UCS2UTF(u32s[i],pout,4);
        pout+=n;
    }
    *pout=0;
    #endif
    std::string u8s(out.get());
    return u8s;
}

bool TextUtils::isEmpty(const std::string&text){
    return text.empty();
}

bool TextUtils::startWith(const std::string&str,const std::string&head){
    return str.compare(0, head.size(), head) == 0;
}

bool TextUtils::endWith(const std::string&str,const std::string&tail){
    return str.size()>=tail.size()&&(str.compare(str.size() - tail.size(), tail.size(), tail) == 0);
}

std::string& TextUtils::trim(std::string&s){
    if (s.empty()){  
        return s;  
    }  
    s.erase(0,s.find_first_not_of(" \t\r\n"));  
    s.erase(s.find_last_not_of(" \t\r\n") + 1);  
    return s;
}

std::string TextUtils::join(const std::string&sep,const std::vector<std::string>&parts){
    std::ostringstream oss;
    for(size_t i=0;i<parts.size();i++){
        oss<<parts[i];
        if(i!=parts.size()-1)oss<<sep;
    }
    return oss.str();
}

std::string& TextUtils::replace(std::string&src,const std::string&old_value,const std::string&new_value){
    for (std::string::size_type pos(0); pos != std::string::npos; pos += new_value.length()) {
        if ((pos = src.find(old_value, pos)) != std::string::npos) {
             src.replace(pos, old_value.length(), new_value);
        }
        else break;
    }
    return src;
}

long TextUtils::strtol(const std::string&value){
    if(value.empty())return 0;
    if(value[0]=='#')return std::strtoul(value.c_str()+1,nullptr,16);
    if((value[0]=='0')&&((value[1]=='x')||(value[1]=='X')))
        return std::strtoul(value.c_str()+2,nullptr,16);
    return std::strtoul(value.c_str(),nullptr,10);
}

std::vector<std::string> TextUtils::split(const std::string& s,const std::string& delim){
    std::vector<std::string> elems;
    size_t pos = 0;
    size_t len = s.length();
    size_t delim_len = delim.length();
    if (delim_len == 0) return elems;
    while (pos < len){
        int find_pos = s.find(delim, pos);
        if (find_pos < 0){
            elems.push_back(s.substr(pos, len - pos));
            break;
        }
        elems.push_back(s.substr(pos, find_pos - pos));
        pos = find_pos + delim_len;
    }
    return elems;
}

std::vector<std::string> TextUtils::split(const std::string& s,int delim){
    std::vector<std::string> elems;
    size_t pos = 0;
    size_t len = s.length();
    while (pos < len){
        int find_pos = s.find(delim, pos);
        if (find_pos < 0){
            elems.push_back(s.substr(pos, len - pos));
            break;
        }
        elems.push_back(s.substr(pos, find_pos - pos));
        pos = find_pos + 1;
    }
    return elems;
}

int TextUtils::UTF2UCS(const char*utf,wchar_t*unicode){
    const unsigned char *p = (const unsigned char*)utf;
    int e = 0, n = 0;
    if(!p || !unicode) return 0;

    if(*p >= 0xfc) {          /* 6:<11111100> */
        e = (p[0] & 0x01) << 30;
        e |= (p[1] & 0x3f) << 24;
        e |= (p[2] & 0x3f) << 18;
        e |= (p[3] & 0x3f) << 12;
        e |= (p[4] & 0x3f) << 6;
        e |= (p[5] & 0x3f);
        n = 6;
    } else if(*p >= 0xf8) {  /* 5:<11111000> */
        e = (p[0] & 0x03) << 24;
        e |= (p[1] & 0x3f) << 18;
        e |= (p[2] & 0x3f) << 12;
        e |= (p[3] & 0x3f) << 6;
        e |= (p[4] & 0x3f);
        n = 5;
    } else if(*p >= 0xf0) {  /* 4:<11110000> */
        e = (p[0] & 0x07) << 18;
        e |= (p[1] & 0x3f) << 12;
	e |= (p[2] & 0x3f) << 6;
        e |= (p[3] & 0x3f);
        n = 4;
    } else if(*p >= 0xe0) {  /* 3:<11100000> */
        e = (p[0] & 0x0f) << 12;
        e |= (p[1] & 0x3f) << 6;
        e |= (p[2] & 0x3f);
        n = 3;
    } else if(*p >= 0xc0) {  /* 2:<11000000> */
        e = (p[0] & 0x1f) << 6;
        e |= (p[1] & 0x3f);
        n = 2;
    } else {
        e = p[0];
        n = 1;
    }
    *unicode = e;
    /* Return bytes count of this utf-8 character */
    return n;
}

int TextUtils::UCS2UTF(wchar_t wc,char*oututf,int outlen){
    int count=0;
    unsigned char*s=(unsigned char*)oututf;
    if (wc < 0x80)         count = 1;
    else if (wc < 0x800)   count = 2;
    else if (wc < 0x10000) count = 3;
    else if (wc < 0x110000)count = 4;
    else  return -1;

    if(oututf==nullptr)return count;
    if(outlen<count)return -1;

    switch (count){ /* note: code falls through cases! */
    case 4: s[3] = 0x80 | (wc & 0x3f); wc = wc >> 6; wc |= 0x10000;
    case 3: s[2] = 0x80 | (wc & 0x3f); wc = wc >> 6; wc |= 0x800;
    case 2: s[1] = 0x80 | (wc & 0x3f); wc = wc >> 6; wc |= 0xc0;
    case 1: s[0] = wc;
    }
    return count;
}

void TextUtils::stringAppendV(std::string& dst, const char* format, va_list ap) {
    // First try with a small fixed size buffer
    char space[1024];// __attribute__((__uninitialized__));

    // It's possible for methods that use a va_list to invalidate
    // the data in it upon use.  The fix is to make a copy
    // of the structure before using it and use that copy instead.
    va_list backup_ap;
    va_copy(backup_ap, ap);
    int result = vsnprintf(space, sizeof(space), format, backup_ap);
    va_end(backup_ap);

    if (result < static_cast<int>(sizeof(space))) {
        if (result >= 0) {
            // Normal case -- everything fit.
            dst.append(space, result);
            return;
        }

        if (result < 0) {
            // Just an error.
            return;
        }
    }

    // Increase the buffer size to the size requested by vsnprintf,
    // plus one for the closing \0.
    int length = result + 1;
    char* buf = new char[length];

    // Restore the va_list before we use it again
    va_copy(backup_ap, ap);
    result = vsnprintf(buf, length, format, backup_ap);
    va_end(backup_ap);

    if (result >= 0 && result < length) {
        // It fit
        dst.append(buf, result);
    }
    delete[] buf;
}


std::string TextUtils::stringPrintf(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    std::string result;
    stringAppendV(result, fmt, ap);
    va_end(ap);
    return result;
}

void TextUtils::stringAppendF(std::string& dst, const char* format, ...) {
    va_list ap;
    va_start(ap, format);
    stringAppendV(dst, format, ap);
    va_end(ap);
}

std::string TextUtils::formatTime(const std::string& fmt, int64_t seconds){
    bool has_ss = false;
    int32_t h24 = seconds / 3600;
    int32_t m   = (seconds % 3600) / 60;
    int32_t s = seconds % 60;
    for (const char* p = fmt.c_str(); *p; ++p)
        if (*p == 's' && *(p+1) == 's') { has_ss = true; break; }

    const bool blink_colon = !has_ss && ((s & 1) == 0);

    std::string out;
    out.reserve(fmt.length() + 32);

    for (const char* p = fmt.c_str(); *p; ++p) {
        char c = *p;

        if (c == ':' && !has_ss) {
            out += (blink_colon ? ':' : ' ');
            continue;
        }

        /* 24h*/
        if (c == 'H' && *(p+1) != 'H') {
            char b[3]; std::snprintf(b, 3, "%u", h24 % 24); out += b; continue;
        }
        if (c == 'H' && *(p+1) == 'H') {
            char b[3]; std::snprintf(b, 3, "%02u", h24 % 24); out += b; ++p; continue;
        }

        /* 12h*/
        if (c == 'h' && *(p+1) != 'h') {
            uint32_t h12 = h24 % 12; if (h12 == 0) h12 = 12;
            char b[16]; std::snprintf(b, 3, "%d", h12); out += b; continue;
        }
        if (c == 'h' && *(p+1) == 'h') {
            uint32_t h12 = h24%12; if (h12 == 0) h12 = 12;
            char b[16]; std::snprintf(b, 3, "%02d", h12); out += b; ++p; continue;
        }

        /* minute */
        if (c == 'm' && *(p+1) != 'm') {
            char b[8]; std::snprintf(b, 3, "%u", m % 60); out += b; continue;
        }
        if (c == 'm' && *(p+1) == 'm') {
            char b[8]; std::snprintf(b, 3, "%02u", m % 60); out += b; ++p; continue;
        }

        /* second */
        if (c == 's' && *(p+1) != 's') {
            char b[8]; std::snprintf(b, 3, "%u", s % 60); out += b; continue;
        }
        if (c == 's' && *(p+1) == 's') {
            char b[8]; std::snprintf(b, 3, "%02u", s % 60); out += b; ++p; continue;
        }

        /* AM/PM */
        if (c == 'A') { out += (h24 < 12 ? "AM" : "PM"); continue; }
        if (c == 'a') { out += (h24 < 12 ? "am" : "pm"); continue; }

        out += c;
    }
    return out;
}

}//namespace
