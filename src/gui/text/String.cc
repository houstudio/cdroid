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
#include <text/String.h>
#include <text/textutils.h>
#include <cstring>
#include <string>
#include <sstream>

namespace cdroid {

const String String::EMPTY;

// ----------------------------------------------------------------------------
// construction / assignment
// ----------------------------------------------------------------------------

String::String() = default;

String::String(const String&) = default;
String& String::operator=(const String&) = default;

String::String(const std::u16string& s) : mValue(s) {}
String::String(const char16_t* s) : mValue(s ? s : std::u16string()) {}
String::String(const char16_t* s, int length) {
    if (s && length > 0) mValue.assign(s, length);
}

String::String(const std::string& utf8) : mValue(TextUtils::utf8_utf16(utf8)) {}
String::String(const char* utf8) {
    if (utf8) mValue = TextUtils::utf8_utf16(std::string(utf8));
}

String& String::operator=(const std::string& utf8) {
    mValue = TextUtils::utf8_utf16(utf8);
    mUtf8Valid = false;
    mHash = 0;
    return *this;
}

String& String::operator=(const char* utf8) {
    mValue = utf8 ? TextUtils::utf8_utf16(std::string(utf8)) : std::u16string();
    mUtf8Valid = false;
    mHash = 0;
    return *this;
}

// ----------------------------------------------------------------------------
// CharSequence
// ----------------------------------------------------------------------------

size_t String::length() const { return mValue.size(); }

int String::charAt(int index) const {
    if (index < 0 || index >= (int)mValue.size()) return 0; // Android throws; relax here
    return (int)mValue[(size_t)index];
}

String* String::subSequence(int start, int end) const {
    return new String(substring(start, end));
}

void String::getChars(int start, int end, char16_t* dest, int destPos) const {
    if (start < 0 || end < start || start > (int)mValue.size() || end > (int)mValue.size() || !dest)
        return;
    const int n = end - start;
    if (n > 0) std::memcpy(dest + destPos, mValue.data() + start, (size_t)n * sizeof(char16_t));
}

std::u16string String::toUTF16() const { return mValue; }

std::string String::toUTF8() const {
    if (!mUtf8Valid) {
        mUtf8 = TextUtils::utf16_utf8(mValue);
        mUtf8Valid = true;
    }
    return mUtf8;
}

String* String::toString() const {
    // Owned copy, so ownership is uniform with every other CharSequence
    // implementor (caller deletes). Java's String.toString() returns `this`,
    // but a borrowed self-pointer here would make ownership inconsistent across
    // implementors and risk double-free.
    return new String(*this);
}

// ----------------------------------------------------------------------------
// ParcelableSpan
// ----------------------------------------------------------------------------

String* String::clone() const { return new String(*this); }

// ----------------------------------------------------------------------------
// std::string (UTF-8) interop
// ----------------------------------------------------------------------------

std::string String::str() const { return toUTF8(); }

const char* String::c_str() const {
    (void)toUTF8(); // ensure mUtf8 cache is valid
    return mUtf8.c_str();
}

const std::u16string& String::u16str() const { return mValue; }

// ----------------------------------------------------------------------------
// common operations
// ----------------------------------------------------------------------------

bool String::isEmpty() const { return mValue.empty(); }

bool String::equals(const String& other) const { return mValue == other.mValue; }
bool String::equals(const std::string& utf8) const { return toUTF8() == utf8; }
bool String::equals(const char* utf8) const { return utf8 && toUTF8() == utf8; }

bool String::operator==(const String& other) const { return mValue == other.mValue; }
bool String::operator==(const char* utf8) const { return equals(utf8); }
bool String::operator==(const std::string& utf8) const { return equals(utf8); }
bool String::operator!=(const String& other) const { return mValue != other.mValue; }

String String::operator+(const String& other) const { return concat(other); }

String String::concat(const String& other) const {
    String r;
    r.mValue.reserve(mValue.size() + other.mValue.size());
    r.mValue = mValue;
    r.mValue += other.mValue;
    return r;
}

int String::compareTo(const String& other) const { return mValue.compare(other.mValue); }
bool String::operator<(const String& other) const { return mValue < other.mValue; }
bool String::operator>(const String& other) const { return mValue > other.mValue; }

int String::indexOf(char16_t ch, int fromIndex) const {
    if (fromIndex < 0) fromIndex = 0;
    for (size_t i = (size_t)fromIndex; i < mValue.size(); i++) {
        if (mValue[i] == ch) return (int)i;
    }
    return -1;
}

int String::indexOf(const String& sub, int fromIndex) const {
    if (fromIndex < 0) fromIndex = 0;
    if (sub.isEmpty()) return std::min((size_t)fromIndex, mValue.size()) == mValue.size() ? (int)mValue.size() : (int)std::min((size_t)fromIndex, mValue.size());
    const char16_t needle = 0;
    (void)needle;
    if ((size_t)fromIndex > mValue.size()) return -1;
    size_t pos = mValue.find(sub.mValue, (size_t)fromIndex);
    return pos == std::u16string::npos ? -1 : (int)pos;
}

int String::lastIndexOf(char16_t ch, int fromIndex) const {
    int end = (fromIndex < 0 || fromIndex >= (int)mValue.size()) ? (int)mValue.size() - 1 : fromIndex;
    for (int i = end; i >= 0; i--) {
        if (mValue[(size_t)i] == ch) return i;
    }
    return -1;
}

String String::substring(int beginIndex) const {
    if (beginIndex < 0) beginIndex = 0;
    if ((size_t)beginIndex > mValue.size()) beginIndex = (int)mValue.size();
    String r;
    r.mValue = mValue.substr((size_t)beginIndex);
    return r;
}

String String::substring(int beginIndex, int endIndex) const {
    if (beginIndex < 0) beginIndex = 0;
    if (endIndex < beginIndex) endIndex = beginIndex;
    if ((size_t)endIndex > mValue.size()) endIndex = (int)mValue.size();
    if ((size_t)beginIndex > mValue.size()) beginIndex = (int)mValue.size();
    String r;
    r.mValue = mValue.substr((size_t)beginIndex, (size_t)(endIndex - beginIndex));
    return r;
}

bool String::startsWith(const String& prefix) const {
    return mValue.size() >= prefix.mValue.size() &&
           mValue.compare(0, prefix.mValue.size(), prefix.mValue) == 0;
}

bool String::endsWith(const String& suffix) const {
    return mValue.size() >= suffix.mValue.size() &&
           mValue.compare(mValue.size() - suffix.mValue.size(), suffix.mValue.size(), suffix.mValue) == 0;
}

bool String::contains(const String& sub) const { return mValue.find(sub.mValue) != std::u16string::npos; }

String String::replace(char16_t oldChar, char16_t newChar) const {
    String r(*this);
    for (auto& c : r.mValue) if (c == oldChar) c = newChar;
    return r;
}

String String::toLowerCase() const {
    // ASCII fast path; non-ASCII left unchanged (full ICU case mapping lives in
    // Character/CaseMapper -- wire there if needed).
    String r(*this);
    for (auto& c : r.mValue) if (c >= u'A' && c <= u'Z') c = (char16_t)(c - u'A' + u'a');
    return r;
}

String String::toUpperCase() const {
    String r(*this);
    for (auto& c : r.mValue) if (c >= u'a' && c <= u'z') c = (char16_t)(c - u'a' + u'A');
    return r;
}

String String::trim() const {
    // Java's trim() strips code points <= U+0020 from both ends.
    size_t start = 0, end = mValue.size();
    while (start < end && mValue[start] <= u' ') start++;
    while (end > start && mValue[end - 1] <= u' ') end--;
    String r;
    r.mValue.assign(mValue, start, end - start);
    return r;
}

int String::hashCode() const {
    if (mHash == 0 && !mValue.empty()) {
        int h = 0;
        for (char16_t c : mValue) h = 31 * h + (int)c;
        mHash = h;
    }
    return mHash;
}

// ----------------------------------------------------------------------------
// valueOf
// ----------------------------------------------------------------------------

String String::valueOf(char16_t c) { return String(std::u16string(1, c)); }
String String::valueOf(int i)      { return String(std::to_string(i)); }
String String::valueOf(int64_t l)  { return String(std::to_string(l)); }
String String::valueOf(float f)    { return String(std::to_string(f)); }
String String::valueOf(double d)   { return String(std::to_string(d)); }
String String::valueOf(bool b)     { return String(std::string(b ? "true" : "false")); }
String String::valueOf(const std::string& utf8) { return String(utf8); }
String String::valueOf(const char* utf8) { return String(utf8); }
String String::valueOf(const std::u16string& u16) { return String(u16); }

} // namespace cdroid
