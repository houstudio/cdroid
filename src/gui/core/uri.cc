/*
 * Copyright (C) 2007 The Android Open Source Project
 * Ported from android-36 android/net/Uri.java and frameworks/base
 * android/net/UriCodec.java (line-by-line).
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <core/uri.h>
#include <text/textutils.h>   // TextUtils::utf8_utf16 (Java String.hashCode)
#include <porting/cdlog.h>
#include <algorithm>
#include <climits>
#include <cstdlib>
#include <cstring>

namespace cdroid {

namespace {

/* UriCodec: Interprets a char as hex digits, returning a number from -1
   (invalid char) to 15 ('f'). */
int hexCharToValue(char c) {
    if ('0' <= c && c <= '9') return c - '0';
    if ('a' <= c && c <= 'f') return 10 + c - 'a';
    if ('A' <= c && c <= 'F') return 10 + c - 'A';
    return -1;
}

/* CharsetDecoder(REPLACE with U+FFFD) over the accumulated escape bytes:
   valid UTF-8 sequences pass through, each malformed sequence becomes one
   U+FFFD (UTF-8 EF BF BD). */
void appendUtf8Replacing(std::string& out, const std::string& bytes) {
    static const char REPLACEMENT[] = "\xEF\xBF\xBD";
    size_t i = 0;
    while (i < bytes.size()) {
        const unsigned char b = (unsigned char)bytes[i];
        if (b < 0x80) { out += (char)b; i++; continue; }
        size_t len;
        uint32_t v;
        if ((b & 0xE0) == 0xC0) { len = 2; v = b & 0x1Fu; }
        else if ((b & 0xF0) == 0xE0) { len = 3; v = b & 0x0Fu; }
        else if ((b & 0xF8) == 0xF0) { len = 4; v = b & 0x07u; }
        else { out += REPLACEMENT; i++; continue; }   // invalid lead byte
        bool ok = (i + len <= bytes.size());
        for (size_t k = 1; ok && k < len; k++) {
            const unsigned char cc = (unsigned char)bytes[i + k];
            if ((cc & 0xC0) != 0x80) { ok = false; break; }
            v = (v << 6) | (cc & 0x3Fu);
        }
        if (!ok || (len == 2 && v < 0x80) || (len == 3 && v < 0x800)
                || (len == 4 && v < 0x10000) || v > 0x10FFFF
                || (v >= 0xD800 && v <= 0xDFFF)) {
            out += REPLACEMENT;
            i++;                                       // resync after the bad lead byte
            continue;
        }
        out.append(bytes, i, len);
        i += len;
    }
}

/* UriCodec.decode(s, convertPlus, UTF_8, throwOnFailure=false). Note the
   quirk the CTS locks in: an invalid escape appends U+FFFD and then still
   buffers the partially-parsed hexValue ("%p" -> U+FFFD + NUL). */
std::string uriCodecDecode(const std::string& s, bool convertPlus) {
    static const char INVALID_INPUT_CHARACTER[] = "\xEF\xBF\xBD";  // U+FFFD
    std::string builder;
    builder.reserve(s.size());
    std::string byteBuffer;
    auto flushDecodingByteAccumulator = [&]() {
        if (byteBuffer.empty()) return;
        appendUtf8Replacing(builder, byteBuffer);
        byteBuffer.clear();
    };
    size_t i = 0;
    while (i < s.size()) {
        char c = s[i];
        i++;
        switch (c) {
        case '+':
            flushDecodingByteAccumulator();
            builder += convertPlus ? ' ' : '+';
            break;
        case '%': {
            // Expect two characters representing a number in hex.
            unsigned int hexValue = 0;   // Java byte; put() takes the low 8 bits
            for (int j = 0; j < 2; j++) {
                if (i >= s.size()) {
                    // Unexpected end of input.
                    flushDecodingByteAccumulator();
                    builder += INVALID_INPUT_CHARACTER;
                    return builder;
                }
                c = s[i];
                i++;
                const int newDigit = hexCharToValue(c);
                if (newDigit < 0) {
                    flushDecodingByteAccumulator();
                    builder += INVALID_INPUT_CHARACTER;
                    break;
                }
                hexValue = (hexValue * 0x10 + newDigit) & 0xFF;
            }
            byteBuffer += (char)hexValue;
            break;
        }
        default:
            flushDecodingByteAccumulator();
            builder += c;
        }
    }
    flushDecodingByteAccumulator();
    return builder;
}

/* java.net.URLEncoder.encode(s, "UTF-8"): keeps alphanumerics and ".-*_",
   space becomes '+', everything else is %xx (lowercase hex). Used by
   getQueryParameters() exactly as AOSP uses it. */
std::string urlEncoderEncode(const std::string& s) {
    static const char* HEX = "0123456789abcdef";
    std::string out;
    out.reserve(s.size());
    for (size_t k = 0; k < s.size(); k++) {
        const unsigned char c = (unsigned char)s[k];
        if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')
                || c == '.' || c == '-' || c == '*' || c == '_') {
            out += (char)c;
        } else if (c == ' ') {
            out += '+';
        } else {
            out += '%';
            out += HEX[(c >> 4) & 0xF];
            out += HEX[c & 0xF];
        }
    }
    return out;
}

bool equalsIgnoreCase(const std::string& a, const char* b) {
    const size_t len = strlen(b);
    if (a.size() != len) return false;
    for (size_t k = 0; k < len; k++) {
        if (tolower((unsigned char)a[k]) != tolower((unsigned char)b[k])) return false;
    }
    return true;
}

std::string toLowerCase(const std::string& s) {
    std::string out = s;
    for (auto& c : out) c = (char)tolower((unsigned char)c);
    return out;
}

/* Java String.hashCode() over the UTF-16 form. */
int javaStringHashCode(const std::string& s) {
    const std::u16string u16 = TextUtils::utf8_utf16(s);
    uint32_t h = 0;
    for (char16_t c : u16) h = 31u * h + (uint32_t)c;
    return (int)h;
}

} // anonymous namespace

const std::string Uri::NotCachedHolder::NOT_CACHED = "NOT CACHED";
const char* Uri::NOT_HIERARCHICAL = "This isn't a hierarchical URI.";

const Uri* Uri::EMPTY() {
    // AOSP: public static final Uri EMPTY = new HierarchicalUri(null,
    // Part.NULL, PathPart.EMPTY, Part.NULL, Part.NULL). Immutable singleton,
    // deliberately never freed (no-GC environment).
    static HierarchicalUri* instance = nullptr;
    if (instance == nullptr) {
        instance = new HierarchicalUri(std::string(), Part::NULL_(),
                PathPart::EMPTY_(), Part::NULL_(), Part::NULL_());
    }
    return instance;
}

Uri::Uri() {}

Uri::~Uri() {}

bool Uri::isOpaque() const {
    return !isHierarchical();
}

bool Uri::isAbsolute() const {
    return !isRelative();
}

bool Uri::equals(const Uri& other) const {
    return toString() == other.toString();
}

bool Uri::operator==(const Uri& other) const {
    return equals(other);
}

bool Uri::operator!=(const Uri& other) const {
    return !equals(other);
}

int Uri::hashCode() const {
    return javaStringHashCode(toString());
}

int Uri::compareTo(const Uri& other) const {
    return toString().compare(other.toString());
}

std::string Uri::toSafeString() const {
    const std::string scheme = getScheme();
    const std::string ssp = getSchemeSpecificPart();
    std::string builder;
    builder.reserve(64);

    if (!scheme.empty()) {
        builder += scheme;
        builder += ':';
        if (equalsIgnoreCase(scheme, "tel") || equalsIgnoreCase(scheme, "sip")
                || equalsIgnoreCase(scheme, "sms") || equalsIgnoreCase(scheme, "smsto")
                || equalsIgnoreCase(scheme, "mailto") || equalsIgnoreCase(scheme, "nfc")) {
            for (size_t k = 0; k < ssp.size(); k++) {
                const char c = ssp[k];
                if (c == '-' || c == '@' || c == '.') {
                    builder += c;
                } else {
                    builder += 'x';
                }
            }
        } else {
            // For other schemes, let's be conservative about
            // the data we include -- only the host and port, not the query params, path or
            // fragment, because those can often have sensitive info.
            const std::string host = getHost();
            const int port = getPort();
            const std::string path = getPath();
            const std::string authority = getAuthority();
            if (!isAuthorityNull()) builder += "//";
            builder += host;
            if (port != -1) { builder += ':'; builder += std::to_string(port); }
            if (!isAuthorityNull() || !isPathNull()) builder += "/...";
        }
    }
    return builder;
}

bool Uri::isAuthorityNull() const {
    // Default for subclasses without a stored authority part: "" == null.
    return getEncodedAuthority().empty();
}

bool Uri::isPathNull() const {
    return getEncodedPath().empty();
}

Uri* Uri::parse(const std::string& uriString) {
    return new StringUri(uriString);
}

Uri* Uri::fromFile(const std::string& absolutePath) {
    const std::string path(absolutePath);
    auto pathPart = PathPart::fromDecoded(&path);
    return new HierarchicalUri("file", Part::EMPTY_(), pathPart,
            Part::NULL_(), Part::NULL_());
}

Uri* Uri::fromParts(const std::string& scheme, const std::string& ssp,
        const std::string& fragment) {
    const std::string sspCopy(ssp);
    const std::string fragmentCopy(fragment);
    return new OpaqueUri(scheme, Part::fromDecoded(&sspCopy),
            Part::fromDecoded(&fragmentCopy));
}

std::vector<std::string> Uri::getQueryParameterNames() const {
    if (isOpaque()) {
        LOGW("%s", NOT_HIERARCHICAL);
        return {};
    }

    const std::string query = getEncodedQuery();
    if (query.empty()) {
        return {};
    }

    // LinkedHashSet: unique names in order of first occurrence.
    std::vector<std::string> names;
    int start = 0;
    do {
        const int next = (int)query.find('&', start);
        const int end = (next == NOT_FOUND) ? (int)query.size() : next;

        int separator = (int)query.find('=', start);
        if (separator > end || separator == NOT_FOUND) {
            separator = end;
        }

        const std::string name = decode(query.substr(start, separator - start));
        if (std::find(names.begin(), names.end(), name) == names.end()) {
            names.push_back(name);
        }

        // Move start to end of name.
        start = end + 1;
    } while (start < (int)query.size());

    return names;
}

std::vector<std::string> Uri::getQueryParameters(const std::string& key) const {
    if (isOpaque()) {
        LOGW("%s", NOT_HIERARCHICAL);
        return {};
    }

    const std::string query = getEncodedQuery();
    if (query.empty()) {
        return {};
    }

    const std::string encodedKey = urlEncoderEncode(key);
    std::vector<std::string> values;

    int start = 0;
    do {
        const int nextAmpersand = (int)query.find('&', start);
        const int end = nextAmpersand != NOT_FOUND ? nextAmpersand : (int)query.size();

        int separator = (int)query.find('=', start);
        if (separator > end || separator == NOT_FOUND) {
            separator = end;
        }

        if (separator - start == (int)encodedKey.size()
                && query.compare(start, encodedKey.size(), encodedKey) == 0) {
            if (separator == end) {
                values.push_back("");
            } else {
                values.push_back(decode(query.substr(separator + 1, end - separator - 1)));
            }
        }

        // Move start to end of name.
        if (nextAmpersand != NOT_FOUND) {
            start = nextAmpersand + 1;
        } else {
            break;
        }
    } while (true);

    return values;
}

std::string Uri::getQueryParameter(const std::string& key) const {
    if (isOpaque()) {
        LOGW("%s", NOT_HIERARCHICAL);
        return "";
    }

    const std::string query = getEncodedQuery();
    if (query.empty()) {
        return "";
    }

    const std::string encodedKey = encode(key, "");
    const size_t length = query.size();
    size_t start = 0;
    do {
        const size_t nextAmpersand = query.find('&', start);
        const size_t end = nextAmpersand != std::string::npos ? nextAmpersand : length;

        size_t separator = query.find('=', start);
        if (separator > end || separator == std::string::npos) {
            separator = end;
        }

        if (separator - start == encodedKey.size()
                && query.compare(start, encodedKey.size(), encodedKey) == 0) {
            if (separator == end) {
                return "";
            } else {
                const std::string encodedValue = query.substr(separator + 1, end - separator - 1);
                // AOSP: UriCodec.decode(..., convertPlus=true, ...)
                return uriCodecDecode(encodedValue, true);
            }
        }

        // Move start to end of name.
        if (nextAmpersand != std::string::npos) {
            start = nextAmpersand + 1;
        } else {
            break;
        }
    } while (true);
    return "";
}

bool Uri::getBooleanQueryParameter(const std::string& key, bool defaultValue) const {
    std::string flag = getQueryParameter(key);
    if (flag.empty()) {
        // AOSP: flag == null means no parameter at all.
        return defaultValue;
    }
    flag = toLowerCase(flag);
    return !("false" == flag) && !("0" == flag);
}

const Uri* Uri::normalizeScheme() const {
    const std::string scheme = getScheme();
    if (scheme.empty()) return this;       // give up
    const std::string lowerScheme = toLowerCase(scheme);
    if (scheme == lowerScheme) return this; // no change

    return buildUpon().scheme(lowerScheme).build();
}

std::string Uri::encode(const std::string& s) {
    return encode(s, "");
}

std::string Uri::encode(const std::string& s, const std::string& allow) {
    static const char* HEX_DIGITS = "0123456789ABCDEF";
    if (s.empty()) return s;

    std::string encoded;
    const size_t oldLength = s.size();

    // This loop alternates between copying over allowed characters and
    // encoding in chunks. This results in fewer method calls and
    // allocations than encoding one character at a time.
    size_t current = 0;
    while (current < oldLength) {
        // Start in "copying" mode where we copy over allowed chars.

        // Find the next character which needs to be encoded.
        size_t nextToEncode = current;
        while (nextToEncode < oldLength && isAllowed(s[nextToEncode], allow)) {
            nextToEncode++;
        }

        // If there's nothing more to encode...
        if (nextToEncode == oldLength) {
            if (current == 0) {
                // We didn't need to encode anything!
                return s;
            } else {
                // Presumably, we've already done some encoding.
                encoded.append(s, current, oldLength - current);
                return encoded;
            }
        }

        if (nextToEncode > current) {
            // Append allowed characters leading up to this point.
            encoded.append(s, current, nextToEncode - current);
        }

        // Switch to "encoding" mode.

        // Find the next allowed character.
        current = nextToEncode;
        size_t nextAllowed = current + 1;
        while (nextAllowed < oldLength && !isAllowed(s[nextAllowed], allow)) {
            nextAllowed++;
        }

        // Convert the substring to bytes and encode the bytes as
        // '%'-escaped octets. (std::string already carries UTF-8 bytes.)
        for (size_t k = current; k < nextAllowed; k++) {
            const unsigned char b = (unsigned char)s[k];
            encoded += '%';
            encoded += HEX_DIGITS[(b >> 4) & 0xF];
            encoded += HEX_DIGITS[b & 0xF];
        }

        current = nextAllowed;
    }

    return encoded;
}

bool Uri::isAllowed(char c, const std::string& allow) {
    const unsigned char uc = (unsigned char)c;
    return (uc >= 'A' && uc <= 'Z')
            || (uc >= 'a' && uc <= 'z')
            || (uc >= '0' && uc <= '9')
            || std::string("_-!.~'()*").find(c) != std::string::npos
            || (!allow.empty() && allow.find(c) != std::string::npos);
}

std::string Uri::encodeIfNotEncoded(const std::string& value, const std::string& allow) {
    // AOSP: gated on Flags.encodeAppIntent() (disabled by default).
    if (isEncoded(value, allow)) return value;
    return encode(value, allow);
}

bool Uri::isEncoded(const std::string& value, const std::string& allow) {
    for (size_t index = 0; index < value.size(); index++) {
        const char c = value[index];

        // Allow % because that's the prefix for an encoded character.
        if (!isAllowed(c, allow) && c != '%') {
            return false;
        }
    }
    return true;
}

std::string Uri::decode(const std::string& s) {
    // AOSP: UriCodec.decode(s, convertPlus=false, UTF-8, throwOnFailure=false)
    return uriCodecDecode(s, false);
}

std::string Uri::decodeIfNeeded(const std::string& value) {
    // AOSP: gated on Flags.encodeAppIntent() (disabled by default).
    return value;
}

Uri* Uri::withAppendedPath(const Uri& baseUri, const std::string& pathSegment) {
    Builder builder = baseUri.buildUpon();
    builder = builder.appendEncodedPath(pathSegment);
    return builder.build();
}

const Uri* Uri::getCanonicalUri() const {
    if (getScheme() == "file") {
        char resolved[PATH_MAX];
        const std::string path = getPath();
        const char* canonicalPath = realpath(path.c_str(), resolved);
        if (canonicalPath == nullptr) {
            return this;   // AOSP: IOException -> this
        }
        const std::string canonical(canonicalPath);
        // AOSP continues with an emulated-storage legacy-path rewrite that has
        // no CDROID equivalent.
        return buildUpon().path(canonical).build();
    }
    return this;
}

int Uri::describeContents() const {
    return 0;
}

void Uri::writeToParcel(Parcel& parcel, int /*flags*/) const {
    // Every subclass overrides this (AOSP implements Parcelable per
    // StringUri/OpaqueUri/HierarchicalUri); unreachable through the base.
    LOGW("Uri::writeToParcel called on the abstract base");
    parcel.writeInt(NULL_TYPE_ID);
}

void Uri::writeToParcel(Parcel& out, const Uri* uri) {
    if (uri == nullptr) {
        out.writeInt(NULL_TYPE_ID);
    } else {
        uri->writeToParcel(out, 0);
    }
}

Uri* Uri::readFromParcel(Parcel& in) {
    const int type = in.readInt();
    switch (type) {
        case NULL_TYPE_ID: return nullptr;
        case StringUri::TYPE_ID: return StringUri::readFrom(in);
        case OpaqueUri::TYPE_ID: return OpaqueUri::readFrom(in);
        case HierarchicalUri::TYPE_ID: return HierarchicalUri::readFrom(in);
    }
    LOGW("Unknown URI type: %d", type);
    return nullptr;
}

/**********************************************************************
 * AbstractPart / Part / EmptyPart
 **********************************************************************/

Uri::AbstractPart::AbstractPart(const std::string* encoded, const std::string* decoded) {
    const std::string* const NC = &NotCachedHolder::NOT_CACHED;
    if (encoded != NC) {
        // this.encoded = encoded (may be Java null); this.decoded = NOT_CACHED.
        mNull = (encoded == nullptr);
        if (encoded != nullptr) mEncoded = *encoded;
        mEncodedCached = true;
        mDecodedCached = false;
    } else if (decoded != NC) {
        // this.encoded = NOT_CACHED; this.decoded = decoded.
        mNull = false;
        mDecoded = *decoded;
        mDecodedCached = true;
        mEncodedCached = false;
    } else {
        LOGW("Uri: Neither encoded nor decoded part provided");
    }
}

Uri::AbstractPart::~AbstractPart() {}

std::string Uri::AbstractPart::getDecoded() const {
    if (!mDecodedCached) {
        mDecoded = decode(mEncoded);
        mDecodedCached = true;
    }
    return mDecoded;
}

Uri::Part::Part(const std::string* encoded, const std::string* decoded)
    : AbstractPart(encoded, decoded) {}

bool Uri::Part::isEmpty() const {
    return false;
}

std::string Uri::Part::getEncoded() const {
    if (!mEncodedCached) {
        mEncoded = encode(mDecoded);
        mEncodedCached = true;
    }
    return mEncoded;
}

// Defined before the singleton accessors: they construct EmptyPart via new
// (protected ctor accessible from Part's static members, unlike make_shared).
class Uri::Part::EmptyPart : public Part {
public:
    explicit EmptyPart(const std::string* value)
        : Part(value, value) {
        if (value != nullptr && !value->empty()) {
            LOGW("Expected empty value, got: %s", value->c_str());
        }
        // Avoid having to re-calculate the non-canonical value.
        // (The Part ctor already stored it into both fields.)
    }
    virtual bool isEmpty() const { return true; }
};

std::shared_ptr<const Uri::Part> Uri::Part::NULL_() {
    static std::shared_ptr<const Part> inst(new EmptyPart((const std::string*)nullptr));
    return inst;
}

std::shared_ptr<const Uri::Part> Uri::Part::EMPTY_() {
    static const std::string kEmpty("");
    static std::shared_ptr<const Part> inst(new EmptyPart(&kEmpty));
    return inst;
}

std::shared_ptr<const Uri::Part> Uri::Part::nonNull(
        const std::shared_ptr<const Part>& part) {
    return part ? part : NULL_();
}

std::shared_ptr<const Uri::Part> Uri::Part::fromEncoded(const std::string* encoded) {
    return from(encoded, &NotCachedHolder::NOT_CACHED);
}

std::shared_ptr<const Uri::Part> Uri::Part::fromDecoded(const std::string* decoded) {
    return from(&NotCachedHolder::NOT_CACHED, decoded);
}

std::shared_ptr<const Uri::Part> Uri::Part::from(const std::string* encoded,
        const std::string* decoded) {
    // We have to check both encoded and decoded in case one is NOT_CACHED.
    const std::string* const NC = &NotCachedHolder::NOT_CACHED;

    if (encoded == nullptr) {
        return NULL_();
    }
    if (encoded != NC && encoded->empty()) {
        return EMPTY_();
    }

    if (decoded == nullptr) {
        return NULL_();
    }
    if (decoded != NC && decoded->empty()) {
        return EMPTY_();
    }

    return std::shared_ptr<const Part>(new Part(encoded, decoded));
}

/**********************************************************************
 * PathPart
 **********************************************************************/

Uri::PathPart::PathPart(const std::string* encoded, const std::string* decoded)
    : AbstractPart(encoded, decoded) {}

std::shared_ptr<const Uri::PathPart> Uri::PathPart::NULL_() {
    static std::shared_ptr<const PathPart> inst(
            new PathPart((const std::string*)nullptr, (const std::string*)nullptr));
    return inst;
}

std::shared_ptr<const Uri::PathPart> Uri::PathPart::EMPTY_() {
    static const std::string kEmpty("");
    static std::shared_ptr<const PathPart> inst(new PathPart(&kEmpty, &kEmpty));
    return inst;
}

std::string Uri::PathPart::getEncoded() const {
    if (!mEncodedCached) {
        // Don't encode '/'.
        mEncoded = encode(mDecoded, "/");
        mEncodedCached = true;
    }
    return mEncoded;
}

std::shared_ptr<const Uri::PathSegments> Uri::PathPart::getPathSegments() const {
    if (mPathSegments) {
        return mPathSegments;
    }

    const std::string path = getEncoded();
    if (isNull()) {
        mPathSegments = PathSegments::EMPTY();
        return mPathSegments;
    }

    PathSegmentsBuilder segmentBuilder;

    int previous = 0;
    size_t current;
    while ((current = path.find('/', previous)) != std::string::npos) {
        // This check keeps us from adding a segment if the path starts
        // '/' and an empty segment for "//".
        if (previous < (int)current) {
            const std::string decodedSegment
                    = decode(path.substr(previous, current - previous));
            segmentBuilder.add(decodedSegment);
        }
        previous = (int)current + 1;
    }

    // Add in the final path segment.
    if (previous < (int)path.size()) {
        segmentBuilder.add(decode(path.substr(previous)));
    }

    mPathSegments = segmentBuilder.build();
    return mPathSegments;
}

std::shared_ptr<const Uri::PathPart> Uri::PathPart::appendEncodedSegment(
        const std::shared_ptr<const PathPart>& oldPart, const std::string& newSegment) {
    // If there is no old path, should we make the new path relative
    // or absolute? I pick absolute.
    if (!oldPart) {
        // No old path.
        const std::string s = "/" + newSegment;
        return fromEncoded(&s);
    }

    std::string oldPath = oldPart->getEncoded();
    if (oldPath.empty()) {
        oldPath = "";
    }

    std::string newPath;
    if (oldPath.empty()) {
        // No old path.
        newPath = "/" + newSegment;
    } else if (oldPath[oldPath.size() - 1] == '/') {
        newPath = oldPath + newSegment;
    } else {
        newPath = oldPath + "/" + newSegment;
    }

    return fromEncoded(&newPath);
}

std::shared_ptr<const Uri::PathPart> Uri::PathPart::appendDecodedSegment(
        const std::shared_ptr<const PathPart>& oldPart, const std::string& decoded) {
    const std::string encoded = encode(decoded);

    // TODO: Should we reuse old PathSegments? Probably not.
    return appendEncodedSegment(oldPart, encoded);
}

std::shared_ptr<const Uri::PathPart> Uri::PathPart::fromEncoded(const std::string* encoded) {
    return from(encoded, &NotCachedHolder::NOT_CACHED);
}

std::shared_ptr<const Uri::PathPart> Uri::PathPart::fromDecoded(const std::string* decoded) {
    return from(&NotCachedHolder::NOT_CACHED, decoded);
}

std::shared_ptr<const Uri::PathPart> Uri::PathPart::from(const std::string* encoded,
        const std::string* decoded) {
    if (encoded == nullptr) {
        return NULL_();
    }

    if (encoded->empty()) {
        return EMPTY_();
    }

    return std::shared_ptr<const PathPart>(new PathPart(encoded, decoded));
}

std::shared_ptr<const Uri::PathPart> Uri::PathPart::makeAbsolute(
        const std::shared_ptr<const PathPart>& oldPart) {
    if (!oldPart) {
        // Unreachable via the AOSP paths (they never pass null here).
        return NULL_();
    }

    const bool encodedCached = oldPart->mEncodedCached;

    // We don't care which version we use, and we don't want to force
    // unnecessary encoding/decoding.
    const std::string& oldPath = encodedCached ? oldPart->mEncoded : oldPart->mDecoded;

    if (oldPath.empty() || oldPath.compare(0, 1, "/") == 0) {
        return oldPart;
    }

    // Prepend encoded string if present.
    std::string encodedPrefixed;
    const std::string* newEncoded;
    if (encodedCached) {
        encodedPrefixed = "/" + oldPart->mEncoded;
        newEncoded = &encodedPrefixed;
    } else {
        newEncoded = &NotCachedHolder::NOT_CACHED;
    }

    // Prepend decoded string if present.
    std::string decodedPrefixed;
    const bool decodedCached = oldPart->mDecodedCached;
    const std::string* newDecoded;
    if (decodedCached) {
        decodedPrefixed = "/" + oldPart->mDecoded;
        newDecoded = &decodedPrefixed;
    } else {
        newDecoded = &NotCachedHolder::NOT_CACHED;
    }

    return std::shared_ptr<const PathPart>(new PathPart(newEncoded, newDecoded));
}

/**********************************************************************
 * PathSegments / PathSegmentsBuilder
 **********************************************************************/

Uri::PathSegments::PathSegments() {}

std::shared_ptr<const Uri::PathSegments> Uri::PathSegments::EMPTY() {
    static std::shared_ptr<const PathSegments> inst = std::make_shared<PathSegments>();
    return inst;
}

const std::string& Uri::PathSegments::get(int index) const {
    if ((size_t)index >= mSegments.size()) {
        // AOSP throws IndexOutOfBoundsException; CDROID logs and returns "".
        LOGW("PathSegments: index %d out of bounds (size %zu)", index, mSegments.size());
        static const std::string kEmpty;
        return kEmpty;
    }
    return mSegments[index];
}

int Uri::PathSegments::size() const {
    return (int)mSegments.size();
}

void Uri::PathSegmentsBuilder::add(const std::string& segment) {
    mSegments.push_back(segment);
}

std::shared_ptr<const Uri::PathSegments> Uri::PathSegmentsBuilder::build() {
    // Makes sure this doesn't get reused (AOSP nulls the array; the shared
    // instance moves into the result).
    auto built = std::make_shared<PathSegments>();
    built->mSegments = std::move(mSegments);
    return built->mSegments.empty() ? PathSegments::EMPTY() : built;
}

/**********************************************************************
 * Builder
 **********************************************************************/

Uri::Builder::Builder() {}

Uri::Builder& Uri::Builder::scheme(const std::string& scheme) {
    if (!scheme.empty()) {
        std::string s = scheme;
        size_t pos;
        while ((pos = s.find("://")) != std::string::npos) {
            s.erase(pos, 3);
        }
        mScheme = s;
    } else {
        mScheme.clear();
    }
    return *this;
}

Uri::Builder& Uri::Builder::opaquePart(const std::shared_ptr<const Part>& opaquePart) {
    mOpaquePart = opaquePart;
    return *this;
}

Uri::Builder& Uri::Builder::opaquePart(const std::string& opaquePart) {
    const std::string s(opaquePart);
    /* qualified: the parameter shadows the member overload in C++ */
    return Builder::opaquePart(Part::fromDecoded(&s));
}

Uri::Builder& Uri::Builder::encodedOpaquePart(const std::string& opaquePart) {
    const std::string s(opaquePart);
    return Builder::opaquePart(Part::fromEncoded(&s));
}

Uri::Builder& Uri::Builder::authority(const std::shared_ptr<const Part>& authority) {
    // This URI will be hierarchical.
    mOpaquePart = nullptr;

    mAuthority = authority;
    return *this;
}

Uri::Builder& Uri::Builder::authority(const std::string& authority) {
    const std::string s(authority);
    return this->authority(Part::fromDecoded(&s));
}

Uri::Builder& Uri::Builder::encodedAuthority(const std::string& authority) {
    const std::string s(authority);
    return this->authority(Part::fromEncoded(&s));
}

Uri::Builder& Uri::Builder::path(const std::shared_ptr<const PathPart>& path) {
    // This URI will be hierarchical.
    mOpaquePart = nullptr;

    mPath = path;
    return *this;
}

Uri::Builder& Uri::Builder::path(const std::string& path) {
    const std::string s(path);
    return this->path(PathPart::fromDecoded(&s));
}

Uri::Builder& Uri::Builder::encodedPath(const std::string& path) {
    const std::string s(path);
    return this->path(PathPart::fromEncoded(&s));
}

Uri::Builder& Uri::Builder::appendPath(const std::string& newSegment) {
    return path(PathPart::appendDecodedSegment(mPath, newSegment));
}

Uri::Builder& Uri::Builder::appendEncodedPath(const std::string& newSegment) {
    return path(PathPart::appendEncodedSegment(mPath, newSegment));
}

Uri::Builder& Uri::Builder::query(const std::shared_ptr<const Part>& query) {
    // This URI will be hierarchical.
    mOpaquePart = nullptr;

    mQuery = query;
    return *this;
}

Uri::Builder& Uri::Builder::query(const std::string& query) {
    const std::string s(query);
    return this->query(Part::fromDecoded(&s));
}

Uri::Builder& Uri::Builder::encodedQuery(const std::string& query) {
    const std::string s(query);
    return this->query(Part::fromEncoded(&s));
}

Uri::Builder& Uri::Builder::fragment(const std::shared_ptr<const Part>& fragment) {
    mFragment = fragment;
    return *this;
}

Uri::Builder& Uri::Builder::fragment(const std::string& fragment) {
    const std::string s(fragment);
    return this->fragment(Part::fromDecoded(&s));
}

Uri::Builder& Uri::Builder::encodedFragment(const std::string& fragment) {
    const std::string s(fragment);
    return this->fragment(Part::fromEncoded(&s));
}

Uri::Builder& Uri::Builder::appendQueryParameter(const std::string& key, const std::string& value) {
    // This URI will be hierarchical.
    mOpaquePart = nullptr;

    const std::string encodedParameter = encode(key, "") + "=" + encode(value, "");

    if (!mQuery) {
        mQuery = Part::fromEncoded(&encodedParameter);
        return *this;
    }

    const std::string oldQuery = mQuery->getEncoded();
    if (oldQuery.empty()) {
        mQuery = Part::fromEncoded(&encodedParameter);
    } else {
        const std::string combined = oldQuery + "&" + encodedParameter;
        mQuery = Part::fromEncoded(&combined);
    }

    return *this;
}

Uri::Builder& Uri::Builder::clearQuery() {
    return query(std::shared_ptr<const Part>());
}

Uri* Uri::Builder::build() const {
    if (mOpaquePart) {
        if (mScheme.empty()) {
            // AOSP throws UnsupportedOperationException("An opaque URI must
            // have a scheme."); CDROID logs and returns null.
            LOGW("An opaque URI must have a scheme.");
            return nullptr;
        }

        return new OpaqueUri(mScheme, mOpaquePart, mFragment);
    } else {
        // Hierarchical URIs should not return null for getPath().
        std::shared_ptr<const PathPart> path = mPath;
        if (!path || path == PathPart::NULL_()) {
            path = PathPart::EMPTY_();
        } else {
            // If we have a scheme and/or authority, the path must
            // be absolute. Prepend it with a '/' if necessary.
            if (hasSchemeOrAuthority()) {
                path = PathPart::makeAbsolute(path);
            }
        }

        return new HierarchicalUri(mScheme, mAuthority, path, mQuery, mFragment);
    }
}

bool Uri::Builder::hasSchemeOrAuthority() const {
    return !mScheme.empty()
            || (mAuthority && mAuthority != Part::NULL_());
}

std::string Uri::Builder::toString() const {
    Uri* uri = build();
    const std::string s = uri ? uri->toString() : std::string();
    delete uri;
    return s;
}

/**********************************************************************
 * AbstractHierarchicalUri
 **********************************************************************/

std::string Uri::AbstractHierarchicalUri::getLastPathSegment() const {
    // TODO: If we haven't parsed all of the segments already, just
    // grab the last one directly so we only allocate one string.
    const std::vector<std::string> segments = getPathSegments();
    const int size = (int)segments.size();
    if (size == 0) {
        return "";
    }
    return segments[size - 1];
}

std::shared_ptr<const Uri::Part> Uri::AbstractHierarchicalUri::getUserInfoPart() const {
    if (!mUserInfoCached) {
        // AOSP parseUserInfo() returns null when there is no authority or no
        // '@' in it; "" for "@host".
        const std::string authority = getEncodedAuthority();
        const size_t end = authority.rfind('@');
        if (authority.empty() || end == std::string::npos) {
            mUserInfo = Part::NULL_();
        } else {
            const std::string parsed = authority.substr(0, end);
            mUserInfo = Part::fromEncoded(&parsed);
        }
        mUserInfoCached = true;
    }
    return mUserInfo;
}

std::string Uri::AbstractHierarchicalUri::getEncodedUserInfo() const {
    return getUserInfoPart()->getEncoded();
}

std::string Uri::AbstractHierarchicalUri::parseUserInfo() const {
    const std::string authority = getEncodedAuthority();
    if (authority.empty()) {
        return "";
    }

    const int end = (int)authority.rfind('@');
    return end == NOT_FOUND ? "" : authority.substr(0, end);
}

std::string Uri::AbstractHierarchicalUri::getUserInfo() const {
    return getUserInfoPart()->getDecoded();
}

std::string Uri::AbstractHierarchicalUri::getHost() const {
    if (!mHostCached) {
        mHost = parseHost();
        mHostCached = true;
    }
    return mHost;
}

std::string Uri::AbstractHierarchicalUri::parseHost() const {
    const std::string authority = getEncodedAuthority();
    if (authority.empty()) {
        return "";
    }

    // Parse out user info and then port.
    const int userInfoSeparator = (int)authority.rfind('@');
    const int portSeparator = findPortSeparator(authority);

    const std::string encodedHost = portSeparator == NOT_FOUND
            ? authority.substr(userInfoSeparator + 1)
            : authority.substr(userInfoSeparator + 1,
                    portSeparator - userInfoSeparator - 1);

    return decode(encodedHost);
}

int Uri::AbstractHierarchicalUri::getPort() const {
    if (mPort == NOT_CALCULATED) {
        mPort = parsePort();
    }
    return mPort;
}

int Uri::AbstractHierarchicalUri::parsePort() const {
    const std::string authority = getEncodedAuthority();
    const int portSeparator = findPortSeparator(authority);
    if (portSeparator == NOT_FOUND) {
        return -1;
    }

    const std::string portString = decode(authority.substr(portSeparator + 1));
    // AOSP catches NumberFormatException and returns -1.
    char* end = nullptr;
    const long v = strtol(portString.c_str(), &end, 10);
    if (end == portString.c_str() || *end != '\0' || v < INT_MIN || v > INT_MAX) {
        LOGW("Error parsing port string: %s", portString.c_str());
        return -1;
    }
    return (int)v;
}

int Uri::AbstractHierarchicalUri::findPortSeparator(const std::string& authority) {
    if (authority.empty()) {
        return NOT_FOUND;
    }

    // Reverse search for the ':' character that breaks as soon as a char
    // that is neither a colon nor an ascii digit is encountered.
    for (int i = (int)authority.size() - 1; i >= 0; --i) {
        const char character = authority[i];
        if (':' == character) return i;
        // Character.isDigit would include non-ascii digits
        if (character < '0' || character > '9') return NOT_FOUND;
    }
    return NOT_FOUND;
}

/**********************************************************************
 * StringUri
 **********************************************************************/

Uri::StringUri::StringUri(const std::string& uriString)
    : mUriString(uriString) {}

Uri* Uri::StringUri::readFrom(Parcel& parcel) {
    return new StringUri(parcel.readString());
}

int Uri::StringUri::describeContents() const {
    return 0;
}

void Uri::StringUri::writeToParcel(Parcel& parcel, int /*flags*/) const {
    parcel.writeInt(TYPE_ID);
    parcel.writeString(mUriString);
}

int Uri::StringUri::findSchemeSeparator() const {
    if (mCachedSsi == NOT_CALCULATED) {
        const size_t pos = mUriString.find(':');
        mCachedSsi = (pos == std::string::npos) ? NOT_FOUND : (int)pos;
    }
    return mCachedSsi;
}

int Uri::StringUri::findFragmentSeparator() const {
    if (mCachedFsi == NOT_CALCULATED) {
        const int ssi = findSchemeSeparator();
        const size_t pos = mUriString.find('#', ssi < 0 ? 0 : (size_t)ssi);
        mCachedFsi = (pos == std::string::npos) ? NOT_FOUND : (int)pos;
    }
    return mCachedFsi;
}

bool Uri::StringUri::isHierarchical() const {
    const int ssi = findSchemeSeparator();

    if (ssi == NOT_FOUND) {
        // All relative URIs are hierarchical.
        return true;
    }

    if ((int)mUriString.size() == ssi + 1) {
        // No ssp.
        return false;
    }

    // If the ssp starts with a '/', this is hierarchical.
    return mUriString[ssi + 1] == '/';
}

bool Uri::StringUri::isRelative() const {
    // Note: We return true if the index is 0
    return findSchemeSeparator() == NOT_FOUND;
}

std::string Uri::StringUri::getScheme() const {
    if (!mSchemeCached) {
        mScheme = parseScheme();
        mSchemeCached = true;
    }
    return mScheme;
}

std::string Uri::StringUri::parseScheme() const {
    const int ssi = findSchemeSeparator();
    return ssi == NOT_FOUND ? "" : mUriString.substr(0, ssi);
}

std::shared_ptr<const Uri::Part> Uri::StringUri::getSsp() const {
    if (!mSsp) {
        const std::string parsed = parseSsp();
        mSsp = Part::fromEncoded(&parsed);
    }
    return mSsp;
}

std::string Uri::StringUri::getEncodedSchemeSpecificPart() const {
    return getSsp()->getEncoded();
}

std::string Uri::StringUri::getSchemeSpecificPart() const {
    return getSsp()->getDecoded();
}

std::string Uri::StringUri::parseSsp() const {
    const int ssi = findSchemeSeparator();
    const int fsi = findFragmentSeparator();

    // Return everything between ssi and fsi.
    return fsi == NOT_FOUND
            ? mUriString.substr(ssi + 1)
            : mUriString.substr(ssi + 1, fsi - ssi - 1);
}

std::shared_ptr<const Uri::Part> Uri::StringUri::getAuthorityPart() const {
    if (!mAuthority) {
        // AOSP: Part.fromEncoded(parseAuthority(uriString, ssi)), where
        // parseAuthority returns null (not "") when there is no "//" authority
        // — the Part.NULL vs Part.EMPTY split toSafeString() depends on.
        const int ssi = findSchemeSeparator();
        const bool hasAuthority = (int)mUriString.size() > ssi + 2
                && mUriString[ssi + 1] == '/'
                && mUriString[ssi + 2] == '/';
        if (hasAuthority) {
            const std::string encodedAuthority = parseAuthority(mUriString, ssi);
            mAuthority = Part::fromEncoded(&encodedAuthority);
        } else {
            mAuthority = Part::NULL_();
        }
    }
    return mAuthority;
}

std::string Uri::StringUri::getEncodedAuthority() const {
    return getAuthorityPart()->getEncoded();
}

std::string Uri::StringUri::getAuthority() const {
    return getAuthorityPart()->getDecoded();
}

std::shared_ptr<const Uri::PathPart> Uri::StringUri::getPathPart() const {
    if (!mPath) {
        // AOSP parsePath() returns null for opaque URIs (and scheme-only),
        // "" for an empty path before '?'/'#' — preserve the split.
        if (isHierarchical()) {
            const std::string parsed = parsePath();
            mPath = PathPart::fromEncoded(&parsed);
        } else {
            mPath = PathPart::NULL_();
        }
    }
    return mPath;
}

std::string Uri::StringUri::getPath() const {
    return getPathPart()->getDecoded();
}

std::string Uri::StringUri::getEncodedPath() const {
    return getPathPart()->getEncoded();
}

std::vector<std::string> Uri::StringUri::getPathSegments() const {
    return getPathPart()->getPathSegments()->mSegments;
}

std::string Uri::StringUri::parsePath() const {
    const int ssi = findSchemeSeparator();

    // If the URI is absolute.
    if (ssi > -1) {
        // Is there anything after the ':'?
        const bool schemeOnly = ssi + 1 == (int)mUriString.size();
        if (schemeOnly) {
            // Opaque URI.
            return "";
        }

        // A '/' after the ':' means this is hierarchical.
        if (mUriString[ssi + 1] != '/') {
            // Opaque URI.
            return "";
        }
    } else {
        // All relative URIs are hierarchical.
    }

    return parsePath(mUriString, ssi);
}

std::shared_ptr<const Uri::Part> Uri::StringUri::getQueryPart() const {
    if (!mQuery) {
        // AOSP parseQuery() returns null when there is no '?' (or an invalid
        // '?' after the fragment separator).
        const int ssi = findSchemeSeparator();
        const size_t qsi = mUriString.find('?', ssi < 0 ? 0 : (size_t)ssi);
        if (qsi == std::string::npos) {
            mQuery = Part::NULL_();
        } else {
            const int fsi = findFragmentSeparator();
            if (fsi != NOT_FOUND && fsi < (int)qsi) {
                // Invalid.
                mQuery = Part::NULL_();
            } else {
                const std::string parsed = mUriString.substr(
                        qsi + 1, fsi == NOT_FOUND ? std::string::npos : (size_t)fsi - qsi - 1);
                mQuery = Part::fromEncoded(&parsed);
            }
        }
    }
    return mQuery;
}

std::string Uri::StringUri::getEncodedQuery() const {
    return getQueryPart()->getEncoded();
}

std::string Uri::StringUri::parseQuery() const {
    // It doesn't make sense to cache this index. We only ever
    // calculate it once.
    const int ssi = findSchemeSeparator();
    const size_t qsi = mUriString.find('?', ssi < 0 ? 0 : (size_t)ssi);
    if (qsi == std::string::npos) {
        return "";
    }

    const int fsi = findFragmentSeparator();

    if (fsi == NOT_FOUND) {
        return mUriString.substr(qsi + 1);
    }

    if (fsi < (int)qsi) {
        // Invalid.
        return "";
    }

    return mUriString.substr(qsi + 1, fsi - (int)qsi - 1);
}

std::string Uri::StringUri::getQuery() const {
    return getQueryPart()->getDecoded();
}

std::shared_ptr<const Uri::Part> Uri::StringUri::getFragmentPart() const {
    if (!mFragment) {
        // AOSP parseFragment() returns null when there is no '#'.
        if (findFragmentSeparator() == NOT_FOUND) {
            mFragment = Part::NULL_();
        } else {
            const std::string parsed = parseFragment();
            mFragment = Part::fromEncoded(&parsed);
        }
    }
    return mFragment;
}

std::string Uri::StringUri::getEncodedFragment() const {
    return getFragmentPart()->getEncoded();
}

std::string Uri::StringUri::parseFragment() const {
    const int fsi = findFragmentSeparator();
    return fsi == NOT_FOUND ? "" : mUriString.substr(fsi + 1);
}

std::string Uri::StringUri::getFragment() const {
    return getFragmentPart()->getDecoded();
}

std::string Uri::StringUri::toString() const {
    return mUriString;
}

std::string Uri::StringUri::parseAuthority(const std::string& uriString, int ssi) {
    const int length = (int)uriString.size();

    // If "//" follows the scheme separator, we have an authority.
    if (length > ssi + 2
            && uriString[ssi + 1] == '/'
            && uriString[ssi + 2] == '/') {
        // We have an authority.

        // Look for the start of the path, query, or fragment, or the
        // end of the string.
        int end = ssi + 3;
        while (end < length) {
            const char c = uriString[end];
            if (c == '/'      // Start of path
                    || c == '\\'  // Start of path (per URL spec, '\' as '/')
                    || c == '?'   // Start of query
                    || c == '#') {  // Start of fragment
                break;
            }
            end++;
        }

        return uriString.substr(ssi + 3, end - ssi - 3);
    } else {
        return "";
    }
}

std::string Uri::StringUri::parsePath(const std::string& uriString, int ssi) {
    const int length = (int)uriString.size();

    // Find start of path.
    int pathStart;
    if (length > ssi + 2
            && uriString[ssi + 1] == '/'
            && uriString[ssi + 2] == '/') {
        // Skip over authority to path.
        pathStart = ssi + 3;
        bool emptyPath = false;
        while (pathStart < length) {
            const char c = uriString[pathStart];
            if (c == '?') {        // Start of query
            } else if (c == '#') { // Start of fragment
            } else if (c == '/' || c == '\\') {  // Start of path!
                break;
            } else {
                pathStart++;
                continue;
            }
            emptyPath = true;      // Return "" (empty path).
            break;
        }
        if (emptyPath) return "";
    } else {
        // Path starts immediately after scheme separator.
        pathStart = ssi + 1;
    }

    // Find end of path.
    int pathEnd = pathStart;
    while (pathEnd < length) {
        const char c = uriString[pathEnd];
        if (c == '?' || c == '#') break;   // Start of query / fragment
        pathEnd++;
    }

    return uriString.substr(pathStart, pathEnd - pathStart);
}

Uri::Builder Uri::StringUri::buildUpon() const {
    if (isHierarchical()) {
        Builder b;
        b.scheme(getScheme())
         .authority(getAuthorityPart())
         .path(getPathPart())
         .query(getQueryPart())
         .fragment(getFragmentPart());
        return b;
    } else {
        Builder b;
        b.scheme(getScheme())
         .opaquePart(getSsp())
         .fragment(getFragmentPart());
        return b;
    }
}

bool Uri::StringUri::isAuthorityNull() const {
    return getAuthorityPart()->isNull();
}

bool Uri::StringUri::isPathNull() const {
    return getPathPart()->isNull();
}

/**********************************************************************
 * OpaqueUri
 **********************************************************************/

Uri::OpaqueUri::OpaqueUri(const std::string& scheme,
        const std::shared_ptr<const Part>& ssp,
        const std::shared_ptr<const Part>& fragment)
    : mScheme(scheme),
      mSsp(ssp),
      mFragment(fragment ? fragment : Part::NULL_()) {}

Uri* Uri::OpaqueUri::readFrom(Parcel& parcel) {
    // AOSP re-parses the written string through a StringUri.
    const StringUri stringUri(parcel.readString());
    const std::string scheme = stringUri.parseScheme();
    const std::string ssp = stringUri.parseSsp();
    const std::string fragment = stringUri.parseFragment();
    return new OpaqueUri(scheme, Part::fromEncoded(&ssp), Part::fromEncoded(&fragment));
}

int Uri::OpaqueUri::describeContents() const {
    return 0;
}

void Uri::OpaqueUri::writeToParcel(Parcel& parcel, int /*flags*/) const {
    parcel.writeInt(TYPE_ID);
    parcel.writeString(toString());
}

bool Uri::OpaqueUri::isHierarchical() const {
    return false;
}

bool Uri::OpaqueUri::isRelative() const {
    return mScheme.empty();
}

std::string Uri::OpaqueUri::getScheme() const {
    return mScheme;
}

std::string Uri::OpaqueUri::getEncodedSchemeSpecificPart() const {
    return mSsp->getEncoded();
}

std::string Uri::OpaqueUri::getSchemeSpecificPart() const {
    return mSsp->getDecoded();
}

std::string Uri::OpaqueUri::getAuthority() const {
    return "";
}

std::string Uri::OpaqueUri::getEncodedAuthority() const {
    return "";
}

std::string Uri::OpaqueUri::getPath() const {
    return "";
}

std::string Uri::OpaqueUri::getEncodedPath() const {
    return "";
}

std::string Uri::OpaqueUri::getQuery() const {
    return "";
}

std::string Uri::OpaqueUri::getEncodedQuery() const {
    return "";
}

std::string Uri::OpaqueUri::getFragment() const {
    return mFragment->getDecoded();
}

std::string Uri::OpaqueUri::getEncodedFragment() const {
    return mFragment->getEncoded();
}

std::vector<std::string> Uri::OpaqueUri::getPathSegments() const {
    return {};
}

std::string Uri::OpaqueUri::getLastPathSegment() const {
    return "";
}

std::string Uri::OpaqueUri::getUserInfo() const {
    return "";
}

std::string Uri::OpaqueUri::getEncodedUserInfo() const {
    return "";
}

std::string Uri::OpaqueUri::getHost() const {
    return "";
}

int Uri::OpaqueUri::getPort() const {
    return -1;
}

std::string Uri::OpaqueUri::toString() const {
    if (mCachedString) {
        return mCachedStringValue;
    }

    std::string sb;
    sb.reserve(mScheme.size() + mSsp->getEncoded().size() + 8);

    sb += mScheme;
    sb += ':';
    sb += getEncodedSchemeSpecificPart();

    if (!mFragment->isEmpty()) {
        sb += '#';
        sb += mFragment->getEncoded();
    }

    mCachedStringValue = sb;
    mCachedString = true;
    return mCachedStringValue;
}

Uri::Builder Uri::OpaqueUri::buildUpon() const {
    Builder b;
    b.scheme(mScheme)
     .opaquePart(mSsp)
     .fragment(mFragment);
    return b;
}

/**********************************************************************
 * HierarchicalUri
 **********************************************************************/

Uri::HierarchicalUri::HierarchicalUri(const std::string& scheme,
        const std::shared_ptr<const Part>& authority,
        const std::shared_ptr<const PathPart>& path,
        const std::shared_ptr<const Part>& query,
        const std::shared_ptr<const Part>& fragment)
    : mScheme(scheme),
      mAuthority(Part::nonNull(authority)),
      mPath(generatePath(path)),
      mQuery(Part::nonNull(query)),
      mFragment(Part::nonNull(fragment)) {}

std::shared_ptr<const Uri::PathPart> Uri::HierarchicalUri::generatePath(
        const std::shared_ptr<const PathPart>& originalPath) const {
    // In RFC3986 the path should be determined based on whether there is a
    // scheme or authority present (https://www.rfc-editor.org/rfc/rfc3986.html#section-3.3).
    const bool hasSchemeOrAuthority = !mScheme.empty() || !mAuthority->isEmpty();
    const std::shared_ptr<const PathPart> newPath = hasSchemeOrAuthority
            ? PathPart::makeAbsolute(originalPath)
            : originalPath;
    return newPath ? newPath : PathPart::NULL_();
}

Uri* Uri::HierarchicalUri::readFrom(Parcel& parcel) {
    // AOSP: new HierarchicalUri(stringUri.getScheme(), stringUri.getAuthorityPart(),
    // stringUri.getPathPart(), stringUri.getQueryPart(), stringUri.getFragmentPart()).
    const StringUri stringUri(parcel.readString());
    return new HierarchicalUri(stringUri.parseScheme(), stringUri.getAuthorityPart(),
            stringUri.getPathPart(), stringUri.getQueryPart(), stringUri.getFragmentPart());
}

int Uri::HierarchicalUri::describeContents() const {
    return 0;
}

void Uri::HierarchicalUri::writeToParcel(Parcel& parcel, int /*flags*/) const {
    parcel.writeInt(TYPE_ID);
    parcel.writeString(toString());
}

bool Uri::HierarchicalUri::isHierarchical() const {
    return true;
}

bool Uri::HierarchicalUri::isRelative() const {
    return mScheme.empty();
}

std::string Uri::HierarchicalUri::getScheme() const {
    return mScheme;
}

std::shared_ptr<const Uri::Part> Uri::HierarchicalUri::getSsp() const {
    if (!mSsp) {
        const std::string ssp = makeSchemeSpecificPart();
        mSsp = Part::fromEncoded(&ssp);
    }
    return mSsp;
}

std::string Uri::HierarchicalUri::getEncodedSchemeSpecificPart() const {
    return getSsp()->getEncoded();
}

std::string Uri::HierarchicalUri::getSchemeSpecificPart() const {
    return getSsp()->getDecoded();
}

std::string Uri::HierarchicalUri::makeSchemeSpecificPart() const {
    std::string builder;
    appendSspTo(builder);
    return builder;
}

void Uri::HierarchicalUri::appendSspTo(std::string& builder) const {
    const std::string encodedAuthority = mAuthority->getEncoded();
    if (!mAuthority->isNull()) {
        // Even if the authority is "", we still want to append "//".
        builder += "//";
        builder += encodedAuthority;
    }

    const std::string encodedPath = mPath->getEncoded();
    if (!mPath->isNull()) {
        builder += encodedPath;
    }

    if (!mQuery->isEmpty()) {
        builder += '?';
        builder += mQuery->getEncoded();
    }
}

std::string Uri::HierarchicalUri::getAuthority() const {
    return mAuthority->getDecoded();
}

std::string Uri::HierarchicalUri::getEncodedAuthority() const {
    return mAuthority->getEncoded();
}

std::string Uri::HierarchicalUri::getEncodedPath() const {
    return mPath->getEncoded();
}

std::string Uri::HierarchicalUri::getPath() const {
    return mPath->getDecoded();
}

std::string Uri::HierarchicalUri::getQuery() const {
    return mQuery->getDecoded();
}

std::string Uri::HierarchicalUri::getEncodedQuery() const {
    return mQuery->getEncoded();
}

std::string Uri::HierarchicalUri::getFragment() const {
    return mFragment->getDecoded();
}

std::string Uri::HierarchicalUri::getEncodedFragment() const {
    return mFragment->getEncoded();
}

std::vector<std::string> Uri::HierarchicalUri::getPathSegments() const {
    return mPath->getPathSegments()->mSegments;
}

std::string Uri::HierarchicalUri::toString() const {
    if (mUriStringCached) {
        return mUriString;
    }
    mUriString = makeUriString();
    mUriStringCached = true;
    return mUriString;
}

std::string Uri::HierarchicalUri::makeUriString() const {
    std::string builder;

    if (!mScheme.empty()) {
        builder += mScheme;
        builder += ':';
    }

    appendSspTo(builder);

    if (!mFragment->isEmpty()) {
        builder += '#';
        builder += mFragment->getEncoded();
    }

    return builder;
}

Uri::Builder Uri::HierarchicalUri::buildUpon() const {
    Builder b;
    b.scheme(mScheme)
     .authority(mAuthority)
     .path(mPath)
     .query(mQuery)
     .fragment(mFragment);
    return b;
}

bool Uri::HierarchicalUri::isAuthorityNull() const {
    return mAuthority->isNull();
}

bool Uri::HierarchicalUri::isPathNull() const {
    return mPath->isNull();
}

} // namespace cdroid
