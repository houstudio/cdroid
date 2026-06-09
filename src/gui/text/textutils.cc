#include <cstring>
#include <sstream>
#include <cstdarg>
#include <core/predicate.h>
#include <text/character.h>
#include <text/spannablestring.h>
#include <text/textutils.h>
#include <unicode/uchar.h>
#include <text/measuredparagraph.h>
namespace cdroid{

const auto ObjectFilter =Predicate<const ParcelableSpan*>([](const ParcelableSpan* span){return dynamic_cast<const ParcelableSpan*>(span) != nullptr;});
const auto ReplacementSpanFilter=Predicate<const ParcelableSpan*>([](const ParcelableSpan* span){return dynamic_cast<const ReplacementSpan*>(span) != nullptr;});

std::string TextUtils::getEllipsisString(TextUtils::TruncateAt method) {
    static const char ELLIPSIS_NORMAL[] = u8"\u2026"; // HORIZONTAL ELLIPSIS (…)
    static const char ELLIPSIS_TWO_DOTS[] = u8"\u2025"; // TWO DOT LEADER (‥)
    return (method == TextUtils::TruncateAt::END_SMALL) ? ELLIPSIS_TWO_DOTS : ELLIPSIS_NORMAL;
}

void TextUtils::getChars(const CharSequence* s, int start, int end, char16_t* dest, int destoff) {
    /*Class<? extends CharSequence> c = s.getClass();
    if (c == String.class)
        ((String) s).getChars(start, end, dest, destoff);
    else if (c == StringBuffer.class)
        ((StringBuffer) s).getChars(start, end, dest, destoff);
    else if (c == StringBuilder.class)
        ((StringBuilder) s).getChars(start, end, dest, destoff);
    else if (s instanceof GetChars)
        ((GetChars) s).getChars(start, end, dest, destoff);
    else {
        for (int i = start; i < end; i++)
            dest[destoff++] = s.charAt(i);
    }*/
     s->getChars(start, end, dest, destoff);
}

int TextUtils::indexOf(const CharSequence* s, char16_t ch, int start) {
    return indexOf(s, ch, start, s->length());
}

int TextUtils::indexOf(const CharSequence* s, char16_t ch, int start, int end) {
    for (int i = start; i < end; i++)
        if (s->charAt(i) == ch)
            return i;
    return -1;
}

int TextUtils::lastIndexOf(const CharSequence* s, char16_t ch) {
    return lastIndexOf(s, ch, s->length() - 1);
}

int TextUtils::lastIndexOf(const CharSequence* s, char16_t ch, int last) {
    return lastIndexOf(s, ch, 0, last);
}

int TextUtils::lastIndexOf(const CharSequence* s, char16_t ch, int start, int last) {
    if (last < 0)
        return -1;
    if (last >= s->length())
        last = s->length() - 1;

    const int end = last + 1;

    for (int i = end - 1; i >= start; i--)
        if (s->charAt(i) == ch)
            return i;
    return -1;
}

int TextUtils::indexOf(const CharSequence* s, const CharSequence* needle) {
    return indexOf(s, needle, 0, s->length());
}

int TextUtils::indexOf(const CharSequence* s,const CharSequence* needle, int start) {
    return indexOf(s, needle, start, s->length());
}

int TextUtils::indexOf(const CharSequence* s,const CharSequence* needle, int start, int end) {
    int nlen = needle->length();
    if (nlen == 0)
        return start;
    char16_t c = needle->charAt(0);
    for (;;) {
        start = indexOf(s, c, start);
        if (start > end - nlen) {
            break;
        }
        if (start < 0) {
            return -1;
        }
        if (regionMatches(s, start, needle, 0, nlen)) {
            return start;
        }
        start++;
    }
    return -1;
}

bool TextUtils::regionMatches(const CharSequence* one, int toffset, const CharSequence* two, int ooffset, int len) {
    int tempLen = 2 * len;
    if (tempLen < len) {
        // Integer overflow; len is unreasonably large
        //throw new IndexOutOfBoundsException();
    }
    std::vector<char16_t> temp(tempLen);

    getChars(one, toffset, toffset + len, temp.data(), 0);
    getChars(two, ooffset, ooffset + len, temp.data(), len);

    bool match = true;
    for (int i = 0; i < len; i++) {
        if (temp[i] != temp[i + len]) {
            match = false;
            break;
        }
    }
    return match;
}

std::string TextUtils::substring(const CharSequence* source, int start, int end) {
    std::vector<char16_t> temp(end - start);
    getChars(source, start, end, temp.data(), 0);
    //String ret = new String(temp, 0, end - start);
    //return ret;
    return "";
}

std::vector<std::string> TextUtils::split(const std::string& text, const std::string& delim) {
    std::vector<std::string> elems;
    size_t pos = 0;
    size_t len = text.length();
    size_t delim_len = delim.length();
    if (delim_len == 0) return elems;
    while (pos < len){
        int find_pos = text.find(delim, pos);
        if (find_pos < 0){
            elems.push_back(text.substr(pos, len - pos));
            break;
        }
        elems.push_back(text.substr(pos, find_pos - pos));
        pos = find_pos + delim_len;
    }
    return elems;
}

std::vector<std::string> TextUtils::split(const std::string& s,int delim){
    std::vector<std::string> elems;
    size_t pos = 0;
    size_t len = s.length();
    while (pos < len){
        auto find_pos = s.find(delim, pos);
        if (find_pos != std::string::npos){
            elems.push_back(s.substr(pos, len - pos));
            break;
        }
        elems.push_back(s.substr(pos, find_pos - pos));
        pos = find_pos + 1;
    }
    return elems;
}

bool TextUtils::isEmpty(const CharSequence* str) {
    return str == nullptr || str->length() == 0;
}

bool TextUtils::isEmpty(const std::string&str){
    return str.empty();
}

std::wstring TextUtils::utf8tounicode(const std::string& utf8){
    size_t u8len = utf8.size() + 8;
    std::unique_ptr<wchar_t[]> out(new wchar_t[u8len]);
    wchar_t* pout = out.get();
    for(int i = 0; i < (int)utf8.length(); ){
        const unsigned char* p = (const unsigned char*)(utf8.c_str() + i);
        int e = 0, n = 0;
        if(*p >= 0xfc) {          /* 6 bytes */
            e = (p[0] & 0x01) << 30;
            e |= (p[1] & 0x3f) << 24;
            e |= (p[2] & 0x3f) << 18;
            e |= (p[3] & 0x3f) << 12;
            e |= (p[4] & 0x3f) << 6;
            e |= (p[5] & 0x3f);
            n = 6;
        } else if(*p >= 0xf8) {  /* 5 bytes */
            e = (p[0] & 0x03) << 24;
            e |= (p[1] & 0x3f) << 18;
            e |= (p[2] & 0x3f) << 12;
            e |= (p[3] & 0x3f) << 6;
            e |= (p[4] & 0x3f);
            n = 5;
        } else if(*p >= 0xf0) {  /* 4 bytes */
            e = (p[0] & 0x07) << 18;
            e |= (p[1] & 0x3f) << 12;
            e |= (p[2] & 0x3f) << 6;
            e |= (p[3] & 0x3f);
            n = 4;
        } else if(*p >= 0xe0) {  /* 3 bytes */
            e = (p[0] & 0x0f) << 12;
            e |= (p[1] & 0x3f) << 6;
            e |= (p[2] & 0x3f);
            n = 3;
        } else if(*p >= 0xc0) {  /* 2 bytes */
            e = (p[0] & 0x1f) << 6;
            e |= (p[1] & 0x3f);
            n = 2;
        } else {                 /* 1 byte */
            e = p[0];
            n = 1;
        }
        *pout++ = e;
        i += n;
    }
    *pout = L'\0';
    return std::wstring(out.get());
}

std::string TextUtils::unicode2utf8(const std::wstring& u32s){
    const int u8len = u32s.length() * 4 + 8;
    std::unique_ptr<char[]> out(new char[u8len]);
    char* pout = out.get();
    for(size_t i = 0; i < u32s.length(); i++){
        wchar_t wc = u32s[i];
        int count = 0;
        if (wc < 0x80)         count = 1;
        else if (wc < 0x800)   count = 2;
        else if (wc < 0x10000) count = 3;
        else if (wc < 0x110000) count = 4;
        else continue;
        
        unsigned char* s = (unsigned char*)pout;
        switch (count){
        case 4: s[3] = 0x80 | (wc & 0x3f); wc = wc >> 6; wc |= 0x10000;
        case 3: s[2] = 0x80 | (wc & 0x3f); wc = wc >> 6; wc |= 0x800;
        case 2: s[1] = 0x80 | (wc & 0x3f); wc = wc >> 6; wc |= 0xc0;
        case 1: s[0] = wc;
        }
        pout += count;
    }
    *pout = 0;
    return std::string(out.get());
}

std::string TextUtils::utf16_utf8(const std::u16string&utf16){
    return utf16_utf8((const uint16_t*)utf16.c_str(),(size_t)utf16.length());
}
std::string TextUtils::utf16_utf8(const uint16_t*utf16,size_t len){
    std::unique_ptr<char[]> out(new char[len * 2]);
    char* pout = out.get();
    for(int i = 0; i < len; i++){
        unsigned short wc = utf16[i];
        int count = 0;
        if (wc < 0x80)         count = 1;
        else if (wc < 0x800)   count = 2;
        else if (wc < 0x10000) count = 3;
        else continue;
        
        unsigned char* s = (unsigned char*)pout;
        switch (count){
        case 3: s[2] = 0x80 | (wc & 0x3f); wc = wc >> 6; wc |= 0x800;
        case 2: s[1] = 0x80 | (wc & 0x3f); wc = wc >> 6; wc |= 0xc0;
        case 1: s[0] = wc;
        }
        pout += count;
    }
    *pout = 0;
    return std::string(out.get());
}

const std::u16string TextUtils::utf8_utf16(const std::string& utf8){
    size_t u8len = utf8.size() + 8;
    std::unique_ptr<char16_t[]> out(new char16_t[u8len]);
    char16_t* pout = out.get();
    for(int i = 0; i < (int)utf8.length(); ){
        const unsigned char* p = (const unsigned char*)(utf8.c_str() + i);
        int e = 0, n = 0;
        if(*p >= 0xf0) {  /* 4 bytes - surrogate pair */
            e = (p[0] & 0x07) << 18;
            e |= (p[1] & 0x3f) << 12;
            e |= (p[2] & 0x3f) << 6;
            e |= (p[3] & 0x3f);
            n = 4;
            if (e >= 0x10000) {
                e -= 0x10000;
                *pout++ = (char16_t)(0xD800 | (e >> 10));
                *pout++ = (char16_t)(0xDC00 | (e & 0x3FF));
                i += n;
                continue;
            }
        } else if(*p >= 0xe0) {  /* 3 bytes */
            e = (p[0] & 0x0f) << 12;
            e |= (p[1] & 0x3f) << 6;
            e |= (p[2] & 0x3f);
            n = 3;
        } else if(*p >= 0xc0) {  /* 2 bytes */
            e = (p[0] & 0x1f) << 6;
            e |= (p[1] & 0x3f);
            n = 2;
        } else {                 /* 1 byte */
            e = p[0];
            n = 1;
        }
        *pout++ = (char16_t)e;
        i += n;
    }
    *pout = 0;
    return std::u16string(out.get());
}
std::string TextUtils::trim(std::string&s){
    if (s.empty()){
        return s;
    }
    s.erase(0,s.find_first_not_of(" \t\r\n"));
    s.erase(s.find_last_not_of(" \t\r\n") + 1);
    return s;
}

bool TextUtils::startWith(const std::string&str,const std::string&head){
    return str.compare(0, head.size(), head) == 0;
}

bool TextUtils::endWith(const std::string&str,const std::string&tail){
    return str.size()>=tail.size()&&(str.compare(str.size() - tail.size(), tail.size(), tail) == 0);
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
    uint32_t h24 = seconds / 3600;
    uint32_t m   = (seconds % 3600) / 60;
    uint32_t s = seconds % 60;
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
            char b[16]; std::snprintf(b, 3, "%u", h12); out += b; continue;
        }
        if (c == 'h' && *(p+1) == 'h') {
            uint32_t h12 = h24%12; if (h12 == 0) h12 = 12;
            char b[16]; std::snprintf(b, 3, "%02u", h12); out += b; ++p; continue;
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

int TextUtils::getTrimmedLength(const CharSequence* s) {
    int len = s->length();

    int start = 0;
    while (start < len && s->charAt(start) <= ' ') {
        start++;
    }
    int end = len;
    while (end > start && s->charAt(end - 1) <= ' ') {
        end--;
    }
    return end - start;
}

bool TextUtils::equals(const CharSequence* a, const CharSequence* b) {
    if (a == b) return true;
    int length;
    if (a != nullptr && b != nullptr && (length = a->length()) == b->length()) {
        for (int i = 0; i < length; i++) {
            if (a->charAt(i) != b->charAt(i)) return false;
        }
        return true;
    }
    return false;
}

int TextUtils::getOffsetBefore(const CharSequence* text, int offset) {
    if (offset == 0)
        return 0;
    if (offset == 1)
        return 0;
    char16_t c = text->charAt(offset - 1);
    if (c >= 0xDC00 && c <= 0xDFFF) {
        char c1 = text->charAt(offset - 2);
        if (c1 >= 0xD800 && c1 <= 0xDBFF)
            offset -= 2;
        else
            offset -= 1;
    } else {
        offset -= 1;
    }
    const Spanned* spanned = dynamic_cast<const Spanned*>(text);
    if (spanned != nullptr) {
        auto spans = spanned->getSpans(offset, offset, ReplacementSpanFilter);

        for (int i = 0; i < spans.size(); i++) {
            int start = spanned->getSpanStart(spans[i]);
            int end = spanned->getSpanEnd(spans[i]);

            if (start < offset && end > offset)
                offset = start;
        }
    }
    return offset;
}

int TextUtils::getOffsetAfter(const CharSequence* text, int offset) {
    int len = text->length();

    if (offset == len)
        return len;
    if (offset == len - 1)
        return len;

    char16_t c = text->charAt(offset);
    if (c >= 0xD800 && c <= 0xDBFF) {
        char c1 = text->charAt(offset + 1);
        if (c1 >= 0xDC00 && c1 <= 0xDFFF)
            offset += 2;
        else
            offset += 1;
    } else {
        offset += 1;
    }
    const Spanned* spanned = dynamic_cast<const Spanned*>(text);
    if (spanned != nullptr) {
        auto spans = spanned->getSpans(offset, offset, ReplacementSpanFilter);

        for (int i = 0; i < spans.size(); i++) {
            int start = spanned->getSpanStart(spans[i]);
            int end = spanned->getSpanEnd(spans[i]);
            if (start < offset && end > offset)
                offset = end;
        }
    }
    return offset;
}

void TextUtils::copySpansFrom(const Spanned* source, int start, int end, const SpanFilter& kind, Spannable* dest, int destoff) {
    auto spans = source->getSpans(start, end, !kind?ObjectFilter:kind);

    for (int i = 0; i < spans.size(); i++) {
        int st = source->getSpanStart(spans[i]);
        int en = source->getSpanEnd(spans[i]);
        int fl = source->getSpanFlags(spans[i]);
        if (st < start)
            st = start;
        if (en > end)
            en = end;
        dest->setSpan(spans[i], st - start + destoff, en - start + destoff, fl);
    }
}

CharSequence* TextUtils::ellipsize(CharSequence* text, TextPaint& paint, float avail, TruncateAt where,
        bool preserveLength, const EllipsizeCallback& callback, const TextDirectionHeuristic* textDir, const std::string& ellipsis) {

    const int len = text->length();

    MeasuredParagraph* mt = MeasuredParagraph::buildForMeasurement(&paint, text, 0, text->length(), textDir, mt);
    float width = mt->getWholeWidth();
    if (width <= avail) {
        if (callback != nullptr) {
            callback(0,0);//.ellipsized(0, 0);
        }

        return text;
    }
    // XXX assumes ellipsis string does not require shaping and
    // is unaffected by style
    float ellipsiswid = paint.measureText(ellipsis);
    avail -= ellipsiswid;

    int left = 0;
    int right = len;
    if (avail < 0) {
        // it all goes
    } else if (where == TruncateAt::START) {
        right = len - mt->breakText(len, false, avail);
    } else if (where == TruncateAt::END || where == TruncateAt::END_SMALL) {
        left = mt->breakText(len, true, avail);
    } else {
        right = len - mt->breakText(len, false, avail / 2);
        avail -= mt->measure(right, len);
        left = mt->breakText(right, true, avail);
    }

    if (callback != nullptr) {
        callback/*.ellipsized*/(left, right);
    }

    auto buf = mt->getChars();
    const Spanned* sp = dynamic_cast<const Spanned*>(text);

    const int removed = right - left;
    const int remaining = len - removed;
    /*if (preserveLength) {
        if (remaining > 0 && removed >= ellipsis.length()) {
            ellipsis.getChars(0, ellipsis.length(), buf, left);
            left += ellipsis.length();
        } // else skip the ellipsis
        for (int i = left; i < right; i++) {
            buf[i] = ELLIPSIS_FILLER;
        }
        String s = new String(buf, 0, len);
        if (sp == nullptr) {
            return new SpannedString(buf,0,len);
        }
        SpannableString* ss = new SpannableString(s);
        copySpansFrom(sp, 0, len, ObjectFilter, ss, 0);
        return ss;
    }*/

    if (remaining == 0) {
        return new SpannedString(u"");
    }

    if (sp == nullptr) {
        /*StringBuilder sb = new StringBuilder(remaining + ellipsis.length());
        sb.append(buf, 0, left);
        sb.append(ellipsis);
        sb.append(buf, right, len - right);
        return sb.toString();*/
        SpannableStringBuilder* sb=new SpannableStringBuilder();
        //sb->append(buf, 0, left);
        sb->append(ellipsis);
        //sb->append(buf, right, len - right);
        return sb;
    }

    SpannableStringBuilder* ssb = new SpannableStringBuilder();
    ssb->append(*text, 0, left);
    ssb->append(ellipsis);
    ssb->append(*text, right, len);
    return ssb;
}
#if 0
CharSequence* TextUtils::commaEllipsize(const CharSequence* text, TextPaint& p,
     float avail, const std::string& oneMore,const std::string& more, const TextDirectionHeuristic* textDir) {

    MeasuredParagraph* mt = nullptr;
    MeasuredParagraph* tempMt = nullptr;

    int len = text->length();
    mt = MeasuredParagraph::buildForMeasurement(p, text, 0, len, textDir, mt);
    const float width = mt->getWholeWidth();
    if (width <= avail) {
        return text;
    }

    auto& buf = mt->getChars();

    int commaCount = 0;
    for (int i = 0; i < len; i++) {
        if (buf[i] == ',') {
            commaCount++;
        }
    }

    int remaining = commaCount + 1;

    int ok = 0;
    std::string okFormat = "";

    int w = 0;
    int count = 0;
    auto& widths = mt->getWidths();

    for (int i = 0; i < len; i++) {
        w += widths[i];

        if (buf[i] == ',') {
            count++;

            std::string format;
            // XXX should not insert spaces, should be part of string
            // XXX should use plural rules and not assume English plurals
            if (--remaining == 1) {
                format = " " + oneMore;
            } else {
                format = " " + String.format(more, remaining);
            }

            // XXX this is probably ok, but need to look at it more
            tempMt = MeasuredParagraph::buildForMeasurement(
                    p, format, 0, format.length(), textDir, tempMt);
            float moreWid = tempMt->getWholeWidth();

            if (w + moreWid <= avail) {
                ok = i + 1;
                okFormat = format;
            }
        }
    }

    SpannableStringBuilder out = new SpannableStringBuilder(okFormat);
    out.insert(0, text, 0, ok);
    return out;
}
#endif
bool TextUtils::couldAffectRtl(char16_t c) {
    return (0x0590 <= c && c <= 0x08FF) ||  // RTL scripts
            c == 0x200E ||  // Bidi format character
            c == 0x200F ||  // Bidi format character
            (0x202A <= c && c <= 0x202E) ||  // Bidi format characters
            (0x2066 <= c && c <= 0x2069) ||  // Bidi format characters
            (0xD800 <= c && c <= 0xDFFF) ||  // Surrogate pairs
            (0xFB1D <= c && c <= 0xFDFF) ||  // Hebrew and Arabic presentation forms
            (0xFE70 <= c && c <= 0xFEFE);  // Arabic presentation forms
}

bool TextUtils::doesNotNeedBidi(const std::vector<char16_t>& text, int start, int len) {
    const int end = start + len;
    for (int i = start; i < end; i++) {
        if (couldAffectRtl(text[i])) {
            return false;
        }
    }
    return true;
}

std::string TextUtils::htmlEncode(const std::string& s) {
    std::ostringstream sb;
    char c;
    for (int i = 0; i < s.length(); i++) {
        c = s[i];
        switch (c) {
        case '<':
            sb<< "&lt;"; //$NON-NLS-1$
            break;
        case '>':
            sb<< "&gt;"; //$NON-NLS-1$
            break;
        case '&':
            sb<< "&amp;"; //$NON-NLS-1$
            break;
        case '\'':
            //http://www.w3.org/TR/xhtml1
            // The named character reference &apos; (the apostrophe, U+0027) was introduced in
            // XML 1.0 but does not appear in HTML. Authors should therefore use &#39; instead
            sb<< "&#39;"; //$NON-NLS-1$
            break;
        case '"':
            sb<< "&quot;"; //$NON-NLS-1$
            break;
        default:
            sb<< c;
        }
    }
    return sb.str();
}

CharSequence* TextUtils::concat(const std::vector<CharSequence*>&text) {
    /*if (text->size() == 0) {
        return "";
    }

    if (text.size() == 1) {
        return text[0];
    }

    bool spanned = false;
    for (CharSequence* piece : text) {
        if (dynamic_cast<Spanned*>(piece)) {
            spanned = true;
            break;
        }
    }

    if (spanned) {
        SpannableStringBuilder ssb;
        for (CharSequence* piece : text) {
            // If a piece is null, we append the string "null" for compatibility with the
            // behavior of StringBuilder and the behavior of the concat() method in earlier
            // versions of Android.
            ssb.append(piece == nullptr ? "null" : piece);
        }
        return new SpannedString(ssb);
    } else {
        std::ostringstream sb;
        for (CharSequence* piece : text) {
            sb<<piece;
        }
        return sb.str();
    }*/return nullptr;
}

bool TextUtils::isGraphic(const CharSequence* str) {
    const int len = str->length();
    for (int cp, i=0; i<len; i+=Character::charCount(cp)) {
        cp = Character::codePointAt(str, i);
        int gc = Character::getType(cp);
        if (gc != Character::CONTROL
                && gc != Character::FORMAT
                && gc != Character::SURROGATE
                && gc != Character::UNASSIGNED
                && gc != Character::LINE_SEPARATOR
                && gc != Character::PARAGRAPH_SEPARATOR
                && gc != Character::SPACE_SEPARATOR) {
            return true;
        }
    }
    return false;
}

bool TextUtils::isGraphic(char16_t c) {
    int gc =  Character::getType(c);
    return  gc != Character::CONTROL
            && gc != Character::FORMAT
            && gc != Character::SURROGATE
            && gc != Character::UNASSIGNED
            && gc != Character::LINE_SEPARATOR
            && gc != Character::PARAGRAPH_SEPARATOR
            && gc != Character::SPACE_SEPARATOR;
}

bool TextUtils::isDigitsOnly(const CharSequence* str) {
    const int len = str->length();
    for (int cp, i = 0; i < len; i += Character::charCount(cp)) {
        cp = Character::codePointAt(str, i);
        if (!!Character::isDigit(cp)) {
            return false;
        }
    }
    return true;
}

bool TextUtils::isPrintableAscii(char16_t c) {
    const int asciiFirst = 0x20;
    const int asciiLast = 0x7E;  // included
    return (asciiFirst <= c && c <= asciiLast) || c == '\r' || c == '\n';
}

bool TextUtils::isPrintableAsciiOnly(const CharSequence* str) {
    const int len = str->length();
    for (int i = 0; i < len; i++) {
        if (!isPrintableAscii(str->charAt(i))) {
            return false;
        }
    }
    return true;
}

int TextUtils::getCapsMode(const CharSequence* cs, int off, int reqModes) {
    if (off < 0) {
        return 0;
    }

    int i;
    char16_t c;
    int mode = 0;
#if 0
    if ((reqModes&CAP_MODE_CHARACTERS) != 0) {
        mode |= CAP_MODE_CHARACTERS;
    }
    if ((reqModes&(CAP_MODE_WORDS|CAP_MODE_SENTENCES)) == 0) {
        return mode;
    }

    // Back over allowed opening punctuation.

    for (i = off; i > 0; i--) {
        c = cs.charAt(i - 1);

        if (c != '"' && c != '\'' &&
            Character.getType(c) != Character.START_PUNCTUATION) {
            break;
        }
    }

    // Start of paragraph, with optional whitespace.

    int j = i;
    while (j > 0 && ((c = cs->charAt(j - 1)) == ' ' || c == '\t')) {
        j--;
    }
    if (j == 0 || cs->charAt(j - 1) == '\n') {
        return mode | CAP_MODE_WORDS;
    }

    // Or start of word if we are that style.

    if ((reqModes&CAP_MODE_SENTENCES) == 0) {
        if (i != j) mode |= CAP_MODE_WORDS;
        return mode;
    }

    // There must be a space if not the start of paragraph.

    if (i == j) {
        return mode;
    }

    // Back over allowed closing punctuation.

    for (; j > 0; j--) {
        c = cs->charAt(j - 1);

        if (c != '"' && c != '\'' &&
            Character.getType(c) != Character.END_PUNCTUATION) {
            break;
        }
    }

    if (j > 0) {
        c = cs->charAt(j - 1);

        if (c == '.' || c == '?' || c == '!') {
            // Do not capitalize if the word ends with a period but
            // also contains a period, in which case it is an abbreviation.

            if (c == '.') {
                for (int k = j - 2; k >= 0; k--) {
                    c = cs->charAt(k);

                    if (c == '.') {
                        return mode;
                    }

                    if (!Character.isLetter(c)) {
                        break;
                    }
                }
            }

            return mode | CAP_MODE_SENTENCES;
        }
    }
#endif
    return mode;
}

void TextUtils::removeEmptySpans(std::vector<ParcelableSpan*>& spans,const Spanned* spanned, const SpanFilter& klass) {
    auto it = spans.begin();
    while (it != spans.end()) {
        const int start = spanned->getSpanStart(*it);
        const int end = spanned->getSpanEnd(*it);
        if (start == end) {
            it = spans.erase(it);
        } else {
            ++it;
        }
    }
}

bool TextUtils::hasStyleSpan(const Spanned* spanned) {
    //Preconditions.checkArgument(spanned != null);
    /*SpanFilter[] styleClasses = {CharacterStyleFilter, ParagraphStyleFilter, UpdateAppearanceFilter};
    for (Class<?> clazz : styleClasses) {
        if (spanned->nextSpanTransition(-1, spanned->length(), clazz) < spanned->length()) {
            return true;
        }
    }*/
    return false;
}

const CharSequence* TextUtils::trimNoCopySpans(const CharSequence* charSequence) {
    if (charSequence != nullptr && dynamic_cast<const Spanned*>(charSequence)) {
        // SpannableStringBuilder copy constructor trims NoCopySpans.
        return new SpannableStringBuilder(charSequence);
    }
    return charSequence;
}

bool TextUtils::isNewline(int codePoint) {
    const int type =  u_charType(codePoint);//Character.getType(codePoint);
    return type == U_PARAGRAPH_SEPARATOR || type == U_LINE_SEPARATOR
            || codePoint == LINE_FEED_CODE_POINT;
}

bool TextUtils::isWhiteSpace(int codePoint) {
    if ((codePoint >= 0x1c && codePoint <= 0x20) || (codePoint >= 0x09 && codePoint <= 0x0d)) {
        return true;
    }
    if (codePoint < 0x1000) {
        return false;
    }
    // OGHAM SPACE MARK or MONGOLIAN VOWEL SEPARATOR?
    if (codePoint == 0x1680 || codePoint == 0x180e) {
        return true;
    }
    if (codePoint < 0x2000) {
        return false;
    }
    // Exclude General Punctuation's non-breaking spaces (which includes FIGURE SPACE).
    if (codePoint == 0x2007 || codePoint == 0x202f) {
        return false;
    }
    if (codePoint <= 0xffff) {
        // Other whitespace from General Punctuation...
        return codePoint <= 0x200a || codePoint == 0x2028 || codePoint == 0x2029 || codePoint == 0x205f ||
            codePoint == 0x3000; // ...or CJK Symbols and Punctuation?
    }
    //return Character.isWhitespace(codePoint) || codePoint == NBSP_CODE_POINT;
    return u_isWhitespace(codePoint)||codePoint == NBSP_CODE_POINT;
}
}/*endof namespace*/
