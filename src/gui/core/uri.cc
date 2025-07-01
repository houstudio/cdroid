/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <regex>
#include <vector>
#include <climits>
#include <core/uri.h>
#include <porting/cdlog.h>

namespace cdroid {
namespace {
    const std::string NOT_CACHED = "NOT VALID";
    const std::string EMPTY = "";
    const size_t NOT_FOUND = std::string::npos;
    const int NOT_CALCULATED = -2;
    const int PORT_NONE = -1;
    const char SCHEME_SEPARATOR = ':';
    const char SCHEME_FRAGMENT = '#';
    const char LEFT_SEPARATOR = '/';
    const char RIGHT_SEPARATOR = '\\';
    const char QUERY_FLAG = '?';
    const char USER_HOST_SEPARATOR = '@';
    const char PORT_SEPARATOR = ':';
    const size_t POS_INC = 1;
    const size_t POS_INC_MORE = 2;
    const size_t POS_INC_AGAIN = 3;
}; // namespace

Uri::Uri(const std::string& uriString)
{
    cachedSsi_ = NOT_FOUND;
    cachedFsi_ = NOT_FOUND;
    port_ = NOT_CALCULATED;

    if (uriString.empty()) {
        return;
    }

    uriString_ = uriString;
    scheme_ = NOT_CACHED;
    ssp_ = NOT_CACHED;
    authority_ = NOT_CACHED;
    host_ = NOT_CACHED;
    userInfo_ = NOT_CACHED;
    query_ = NOT_CACHED;
    path_ = NOT_CACHED;
    fragment_ = NOT_CACHED;

    if (!checkScheme()) {
        uriString_ = EMPTY;
        LOGE("URI Scheme wrong");
    }
}

Uri::~Uri()
{}

bool Uri::checkScheme()
{
    scheme_ = parseScheme();
    if (scheme_.empty()) {
        return true;
    }
    try {
        std::regex schemeRegex("[a-zA-Z][a-zA-Z|\\d|\\+|\\-|.]*$");
        if (!std::regex_match(scheme_, schemeRegex)) {
            return false;
        }
    } catch (std::regex_error &message) {
        LOGE("regex fail,message:%s", message.what());
        return false;
    }
    return true;
}

std::string Uri::getScheme()
{
    if (uriString_.empty()) {
        return EMPTY;
    }

    if (scheme_ == NOT_CACHED) {
        scheme_ = parseScheme();
    }
    return scheme_;
}

std::string Uri::parseScheme()
{
    size_t ssi = findSchemeSeparator();
    return (ssi == NOT_FOUND) ? EMPTY : uriString_.substr(0, ssi);
}

std::string Uri::getSchemeSpecificPart()
{
    if (uriString_.empty()) {
        return EMPTY;
    }

    return (ssp_ == NOT_CACHED) ? (ssp_ = parseSsp()) : ssp_;
}

std::string Uri::parseSsp()
{
    size_t ssi = findSchemeSeparator();
    size_t fsi = findFragmentSeparator();

    size_t start = (ssi == NOT_FOUND) ? 0 : (ssi + 1);
    size_t end = (fsi == NOT_FOUND) ? uriString_.size() : fsi;

    // Return everything between ssi and fsi.
    std::string ssp = EMPTY;
    if (end > start) {
        ssp = uriString_.substr(start, end - start);
    }

    return ssp;
}

std::string Uri::getAuthority()
{
    if (uriString_.empty()) {
        return EMPTY;
    }

    if (authority_ == NOT_CACHED) {
        authority_ = parseAuthority();
    }
    return authority_;
}

std::string Uri::parseAuthority()
{
    size_t ssi = findSchemeSeparator();
    if (ssi == NOT_FOUND) {
        return EMPTY;
    }

    size_t length = uriString_.length();
    // If "//" follows the scheme separator, we have an authority.
    if ((length > (ssi + POS_INC_MORE)) && (uriString_.at(ssi + POS_INC) == LEFT_SEPARATOR) &&
        (uriString_.at(ssi + POS_INC_MORE) == LEFT_SEPARATOR)) {
        // Look for the start of the path, query, or fragment, or the end of the string.
        size_t start = ssi + POS_INC_AGAIN;
        size_t end = start;

        while (end < length) {
            char ch = uriString_.at(end);
            if ((ch == LEFT_SEPARATOR) || (ch == RIGHT_SEPARATOR) || (ch == QUERY_FLAG) ||
                (ch == SCHEME_FRAGMENT)) {
                break;
            }

            end++;
        }

        return uriString_.substr(start, end - start);
    } else {
        return EMPTY;
    }
}

std::string Uri::getUserInfo()
{
    if (uriString_.empty()) {
        return EMPTY;
    }

    if (userInfo_ == NOT_CACHED) {
        userInfo_ = parseUserInfo();
    }
    return userInfo_;
}

std::string Uri::parseUserInfo()
{
    std::string authority = getAuthority();
    if (authority.empty()) {
        return EMPTY;
    }

    size_t end = authority.find_last_of(USER_HOST_SEPARATOR);
    return (end == NOT_FOUND) ? EMPTY : authority.substr(0, end);
}

std::string Uri::getHost()
{
    if (uriString_.empty()) {
        return EMPTY;
    }

    if (host_ == NOT_CACHED) {
        host_ = parseHost();
    }
    return host_;
}

std::string Uri::parseHost()
{
    std::string authority = getAuthority();
    if (authority.empty()) {
        return EMPTY;
    }

    // Parse out user info and then port.
    size_t userInfoSeparator = authority.find_last_of(USER_HOST_SEPARATOR);
    size_t start = (userInfoSeparator == NOT_FOUND) ? 0 : (userInfoSeparator + 1);
    size_t portSeparator = authority.find_first_of(PORT_SEPARATOR, start);
    size_t end = (portSeparator == NOT_FOUND) ? authority.size() : portSeparator;

    std::string host = EMPTY;
    if (start < end) {
        host = authority.substr(start, end - start);
    }

    return host;
}

int Uri::getPort()
{
    if (uriString_.empty()) {
        return PORT_NONE;
    }

    if (port_ == NOT_CALCULATED) {
        port_ = parsePort();
    }
    return port_;
}

static bool StrToInt(const std::string& str, int& value)
{
    if (str.empty() || (!isdigit(str.front()) && (str.front() != '-'))) {
        return false;
    }

    char* end = nullptr;
    errno = 0;
    auto addr = str.c_str();
    auto result = strtol(addr, &end, 10); /* 10 means decimal */
    if ((end == addr) || (end[0] != '\0') || (errno == ERANGE) ||
            (result > INT_MAX) || (result < INT_MIN)) {
        return false;
    }
    value = static_cast<int>(result);
    return true;
}

int Uri::parsePort()
{
    std::string authority = getAuthority();
    if (authority.empty()) {
        return PORT_NONE;
    }

    // Make sure we look for the port separtor *after* the user info separator.
    size_t userInfoSeparator = authority.find_last_of(USER_HOST_SEPARATOR);
    size_t start = (userInfoSeparator == NOT_FOUND) ? 0 : (userInfoSeparator + 1);
    size_t portSeparator = authority.find_first_of(PORT_SEPARATOR, start);
    if (portSeparator == NOT_FOUND) {
        return PORT_NONE;
    }

    start = portSeparator + 1;
    std::string portString = authority.substr(start);

    int value = PORT_NONE;
    return StrToInt(portString, value) ? value : PORT_NONE;
}

std::string Uri::getQuery()
{
    if (uriString_.empty()) {
        return EMPTY;
    }

    if (query_ == NOT_CACHED) {
        query_ = parseQuery();
    }
    return query_;
}

std::string Uri::parseQuery()
{
    size_t ssi = findSchemeSeparator();
    if (ssi == NOT_FOUND) {
        ssi = 0;
    }
    size_t qsi = uriString_.find_first_of(QUERY_FLAG, ssi);
    if (qsi == NOT_FOUND) {
        return EMPTY;
    }

    size_t start = qsi + 1;
    size_t fsi = findFragmentSeparator();
    if (fsi == NOT_FOUND) {
        return uriString_.substr(start);
    }

    if (fsi < qsi) {
        // Invalid.
        return EMPTY;
    }

    return uriString_.substr(start, fsi - start);
}

std::string Uri::getPath()
{
    if (uriString_.empty()) {
        return EMPTY;
    }

    if (path_ == NOT_CACHED) {
        path_ = parsePath();
    }
    return path_;
}

void Uri::getPathSegments(std::vector<std::string>& segments)
{
    if (uriString_.empty()) {
        return;
    }
    if (path_ == NOT_CACHED) {
        path_ = parsePath();
    }

    size_t previous = 0;
    size_t current;
    while ((current = path_.find(LEFT_SEPARATOR, previous)) != std::string::npos) {
        if (previous < current) {
            segments.emplace_back(path_.substr(previous, current - previous));
        }
        previous = current + POS_INC;
    }
    // Add in the final path segment.
    if (previous < path_.length()) {
        segments.emplace_back(path_.substr(previous));
    }
}

std::string Uri::parsePath()
{
    size_t ssi = findSchemeSeparator();
    // If the URI is absolute.
    if (ssi != NOT_FOUND) {
        // Is there anything after the ':'?
        if ((ssi + 1) == uriString_.length()) {
            // Opaque URI.
            return EMPTY;
        }

        // A '/' after the ':' means this is hierarchical.
        if (uriString_.at(ssi + 1) != LEFT_SEPARATOR) {
            // Opaque URI.
            return EMPTY;
        }
    } else {
        // All relative URIs are hierarchical.
    }

    return parsePath(ssi);
}

std::string Uri::parsePath(size_t ssi)
{
    size_t length = uriString_.length();

    // Find start of path.
    size_t pathStart = (ssi == NOT_FOUND) ? 0 : (ssi + POS_INC);
    if ((length > (pathStart + POS_INC)) && (uriString_.at(pathStart) == LEFT_SEPARATOR) &&
        (uriString_.at(pathStart + POS_INC) == LEFT_SEPARATOR)) {
        // Skip over authority to path.
        pathStart += POS_INC_MORE;

        while (pathStart < length) {
            char ch = uriString_.at(pathStart);
            if ((ch == QUERY_FLAG) || (ch == SCHEME_FRAGMENT)) {
                return EMPTY;
            }

            if ((ch == LEFT_SEPARATOR) || (ch == RIGHT_SEPARATOR)) {
                break;
            }

            pathStart++;
        }
    }

    // Find end of path.
    size_t pathEnd = pathStart;
    while (pathEnd < length) {
        char ch = uriString_.at(pathEnd);
        if ((ch == QUERY_FLAG) || (ch == SCHEME_FRAGMENT)) {
            break;
        }

        pathEnd++;
    }

    return uriString_.substr(pathStart, pathEnd - pathStart);
}

std::string Uri::getFragment()
{
    if (uriString_.empty()) {
        return EMPTY;
    }

    if (fragment_ == NOT_CACHED) {
        fragment_ = parseFragment();
    }
    return fragment_;
}

std::string Uri::parseFragment()
{
    size_t fsi = findFragmentSeparator();
    return (fsi == NOT_FOUND) ? EMPTY : uriString_.substr(fsi + 1);
}

size_t Uri::findSchemeSeparator()
{
    if (cachedSsi_ == NOT_FOUND) {
        cachedSsi_ = uriString_.find_first_of(SCHEME_SEPARATOR);
    }
    return cachedSsi_;
}

size_t Uri::findFragmentSeparator()
{
    if (cachedFsi_ == NOT_FOUND) {
        cachedFsi_ = uriString_.find_first_of(SCHEME_FRAGMENT, findSchemeSeparator());
    }
    return cachedFsi_;
}

bool Uri::isHierarchical()
{
    if (uriString_.empty()) {
        return false;
    }

    size_t ssi = findSchemeSeparator();
    if (ssi == NOT_FOUND) {
        // All relative URIs are hierarchical.
        return true;
    }

    if (uriString_.length() == (ssi + 1)) {
        // No ssp.
        return false;
    }

    // If the ssp starts with a '/', this is hierarchical.
    return (uriString_.at(ssi + 1) == LEFT_SEPARATOR);
}

bool Uri::isAbsolute()
{
    if (uriString_.empty()) {
        return false;
    }

    return !isRelative();
}

bool Uri::isRelative()
{
    if (uriString_.empty()) {
        return false;
    }

    // Note: We return true if the index is 0
    return findSchemeSeparator() == NOT_FOUND;
}

bool Uri::equals(const Uri& other) const
{
    return uriString_ == other.toString();
}

int Uri::compareTo(const Uri& other) const
{
    return uriString_.compare(other.toString());
}

std::string Uri::toString() const
{
    return uriString_;
}

bool Uri::operator==(const Uri& other) const
{
    return uriString_ == other.toString();
}
#if 0
bool Uri::marshalling(Parcel& parcel) const
{
    if (isAsciiString(uriString_)) {
        return parcel.WriteString16(Str8ToStr16(uriString_));
    }

    LOGE("uriString_ is not ascii string");
    return false;
}

Uri* Uri::unmarshalling(Parcel& parcel)
{
    return new Uri(Str16ToStr8(parcel.ReadString16()));
}
#endif

static int hexCharToInt(char c) {
    if (c >= '0' && c <= '9') {
        return c - '0';
    } else if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    } else if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    }
    return 0;
}

std::string Uri::decode(const std::string&encoded){
    std::ostringstream decoded;
    for (size_t i = 0; i < encoded.length(); ++i) {
        if (encoded[i] == '%' && i + 2 < encoded.length()) {
            // 处理百分号编码的字符
            int high = hexCharToInt(encoded[i + 1]);
            int low = hexCharToInt(encoded[i + 2]);
            decoded << static_cast<char>((high << 4) | low);
            i += 2;
        } else {
            // 普通字符直接添加
            decoded << encoded[i];
        }
    }
    return decoded.str();
}

} // namespace cdroid
