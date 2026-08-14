/*
 * Copyright (C) 2007 The Android Open Source Project
 * Ported from android-36 android/net/Uri.java (line-by-line; class and method
 * names/signatures kept identical).
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

#ifndef __CDROID_URI_H__
#define __CDROID_URI_H__

#include <string>
#include <vector>
#include <memory>
#include "parcel.h"

namespace cdroid {

/**
 * Immutable URI reference. A URI reference includes a URI and a fragment, the
 * component of the URI following a '#'. Builds and parses URI references
 * which conform to RFC 2396.
 *
 * In the interest of performance, this class performs little to no
 * validation. Behavior is undefined for invalid input. This class is very
 * forgiving--in the face of invalid input, it will return garbage
 * rather than throw an exception unless otherwise specified.
 *
 * Java->C++ mapping notes (apply throughout):
 *  - Java String        -> std::string. A null String maps to the empty
 *    string for every public getter/parameter ("" == null), except where the
 *    null-vs-empty distinction is behavioral (Part.NULL vs Part.EMPTY), which
 *    is preserved internally via AbstractPart::isNull().
 *  - List<String>       -> std::vector<std::string>
 *  - Set<String>        -> std::vector<std::string> (insertion-ordered, the
 *    LinkedHashSet iteration order)
 *  - Java static final singleton fields (Uri.EMPTY, Part.NULL/EMPTY,
 *    PathPart.NULL/EMPTY, PathSegments.EMPTY) -> accessor functions returning
 *    the shared immutable instance ("NULL_": NULL is a macro in C/C++).
 *  - parse()/build()/... return Uri* (heap). Instances are immutable; callers
 *    own (delete) the result. getCanonicalUri()/normalizeScheme() may alias
 *    this — do not delete blindly.
 *  - AOSP package-private nested classes and members are public here (C++ has
 *    no package). Where AOSP relies on GC to share immutable Part instances
 *    between Uris (buildUpon), the internal (package-private-level) API uses
 *    std::shared_ptr<const Part>; the public API is unaffected.
 */
class Uri/*: public Parcelable, public Comparable<Uri>*/{
public:
    /* Nested classes, forward-declared so every member below can name them
       (AOSP declares them after the abstract API too). */
    class NotCachedHolder;
    class AbstractPart;
    class Part;
    class PathPart;
    class PathSegments;
    class PathSegmentsBuilder;
    class Builder;
    class AbstractHierarchicalUri;
    class StringUri;
    class OpaqueUri;
    class HierarchicalUri;

    /** Index of a component which was not found. */
    static constexpr int NOT_FOUND = -1;

    /** Placeholder value for an index which hasn't been calculated yet. */
    static constexpr int NOT_CALCULATED = -2;

    /**
     * The empty URI, equivalent to "".
     * AOSP: public static final Uri EMPTY = new HierarchicalUri(...).
     */
    static const Uri* EMPTY();

    /**
     * Prevents external subclassing. (AOSP: private Uri() {})
     */
protected:
    Uri();
public:
    virtual ~Uri();

    /**
     * Returns true if this URI is hierarchical like "http://google.com".
     * Absolute URIs are hierarchical if the scheme-specific part starts with
     * a '/'. Relative URIs are always hierarchical.
     */
    virtual bool isHierarchical() const = 0;

    /**
     * Returns true if this URI is opaque like "mailto:nobody@google.com". The
     * scheme-specific part of an opaque URI cannot start with a '/'.
     */
    virtual bool isOpaque() const;

    /**
     * Returns true if this URI is relative, i.e. if it doesn't contain an
     * explicit scheme.
     */
    virtual bool isRelative() const = 0;

    /**
     * Returns true if this URI is absolute, i.e. if it contains an
     * explicit scheme.
     */
    virtual bool isAbsolute() const;

    /** Gets the scheme of this URI. Example: "http". Empty if relative. */
    virtual std::string getScheme() const = 0;

    /**
     * Gets the scheme-specific part of this URI, i.e. everything between
     * the scheme separator ':' and the fragment separator '#'. If this is a
     * relative URI, this method returns the entire URI. Decodes escaped octets.
     * Example: "//www.google.com/search?q=android"
     */
    virtual std::string getSchemeSpecificPart() const = 0;

    /**
     * Gets the scheme-specific part of this URI, i.e. everything between
     * the scheme separator ':' and the fragment separator '#'. If this is a
     * relative URI, this method returns the entire URI. Leaves escaped octets
     * intact.
     */
    virtual std::string getEncodedSchemeSpecificPart() const = 0;

    /**
     * Gets the decoded authority part of this URI. For server addresses, the
     * authority is structured as follows: [ userinfo '@' ] host [ ':' port ]
     * Examples: "google.com", "bob@google.com:80". Empty if not present.
     */
    virtual std::string getAuthority() const = 0;

    /** Gets the encoded authority part of this URI. Empty if not present. */
    virtual std::string getEncodedAuthority() const = 0;

    /**
     * Gets the decoded user information from the authority. For example, if
     * the authority is "nobody@google.com", this method will return "nobody".
     */
    virtual std::string getUserInfo() const = 0;

    /** Gets the encoded user information from the authority. */
    virtual std::string getEncodedUserInfo() const = 0;

    /**
     * Gets the encoded host from the authority for this URI. For example, if
     * the authority is "bob@google.com", this method will return "google.com".
     */
    virtual std::string getHost() const = 0;

    /**
     * Gets the port from the authority for this URI. For example, if the
     * authority is "google.com:80", this method will return 80. -1 if
     * invalid or not present.
     */
    virtual int getPort() const = 0;

    /**
     * Gets the decoded path. Empty if this is not a hierarchical URI
     * (like "mailto:nobody@google.com") or the URI is invalid.
     */
    virtual std::string getPath() const = 0;

    /**
     * Gets the encoded path. Empty if this is not a hierarchical URI
     * (like "mailto:nobody@google.com") or the URI is invalid.
     */
    virtual std::string getEncodedPath() const = 0;

    /**
     * Gets the decoded query component from this URI. The query comes after
     * the query separator ('?') and before the fragment separator ('#').
     * "q=android" for "http://www.google.com/search?q=android".
     */
    virtual std::string getQuery() const = 0;

    /** Gets the encoded query component from this URI. */
    virtual std::string getEncodedQuery() const = 0;

    /** Gets the decoded fragment part of this URI, everything after the '#'. */
    virtual std::string getFragment() const = 0;

    /** Gets the encoded fragment part of this URI, everything after the '#'. */
    virtual std::string getEncodedFragment() const = 0;

    /**
     * Gets the decoded path segments.
     * @return decoded path segments, each without a leading or trailing '/'
     */
    virtual std::vector<std::string> getPathSegments() const = 0;

    /**
     * Gets the decoded last segment in the path. Empty if the path is empty.
     */
    virtual std::string getLastPathSegment() const = 0;

    /**
     * Compares this Uri to another object for equality. Returns true if the
     * encoded string representations of this Uri and the given Uri are
     * equal. Case counts. Paths are not normalized. If one Uri specifies a
     * default port explicitly and the other leaves it implicit, they will not
     * be considered equal.
     */
    bool equals(const Uri& other) const;
    bool operator==(const Uri& other) const;
    bool operator!=(const Uri& other) const;

    /**
     * Hashes the encoded string representation of this Uri consistently with
     * equals() (Java String.hashCode over the UTF-16 form).
     */
    int hashCode() const;

    /**
     * Compares the string representation of this Uri with that of another.
     */
    int compareTo(const Uri& other) const;

    /**
     * Returns the encoded string representation of this URI.
     * Example: "http://google.com/"
     */
    virtual std::string toString() const = 0;

    /**
     * Return a string representation of this URI that has common forms of PII
     * redacted, making it safer to use for logging purposes.
     * AOSP: @SystemApi String toSafeString()
     */
    std::string toSafeString() const;

    /**
     * Constructs a new builder, copying the attributes from this Uri.
     */
    virtual Builder buildUpon() const = 0;

    /**
     * Creates a Uri which parses the given encoded URI string.
     * @param uriString an RFC 2396-compliant, encoded URI
     * @return Uri for this given uri string (caller owns)
     */
    static Uri* parse(const std::string& uriString);

    /**
     * Creates a Uri from a file path. The URI has the form
     * "file://<absolute path>". Encodes path characters with the exception
     * of '/'.
     * Example: "file:///tmp/android.txt"
     * (AOSP: fromFile(File file); CDROID has no java.io.File — the absolute
     * path is passed directly.)
     */
    static Uri* fromFile(const std::string& absolutePath);

    /**
     * Creates an opaque Uri from the given components. Encodes the ssp
     * which means this method cannot be used to create hierarchical URIs.
     * @param scheme of the URI
     * @param ssp scheme-specific-part, everything between the scheme
     *            separator (':') and the fragment separator ('#'), which
     *            will get encoded
     * @param fragment fragment, everything after the '#', empty if undefined,
     *                 will get encoded
     * @return Uri composed of the given scheme, ssp, and fragment
     */
    static Uri* fromParts(const std::string& scheme, const std::string& ssp,
            const std::string& fragment);

    /**
     * Returns a list of the unique names of all query parameters. Iterating
     * over the list will return the names in order of their first occurrence.
     * @return a list of decoded names
     */
    std::vector<std::string> getQueryParameterNames() const;

    /**
     * Searches the query string for parameter values with the given key.
     * @param key which will be encoded
     * @return a list of decoded values
     */
    std::vector<std::string> getQueryParameters(const std::string& key) const;

    /**
     * Searches the query string for the first value with the given key.
     * @param key which will be encoded
     * @return the decoded value or empty string if no parameter is found
     */
    std::string getQueryParameter(const std::string& key) const;

    /**
     * Searches the query string for the first value with the given key and
     * interprets it as a boolean value. "false" and "0" are interpreted as
     * false, everything else is interpreted as true.
     * @param key which will be decoded
     * @param defaultValue the default value to return if there is no query
     *                     parameter for key
     * @return the boolean interpretation of the query parameter key
     */
    bool getBooleanQueryParameter(const std::string& key, bool defaultValue) const;

    /**
     * Return an equivalent URI with a lowercase scheme component.
     * This aligns the Uri with Android best practices for intent filtering.
     * For example, "HTTP://www.android.com" becomes "http://www.android.com"
     * @return normalized Uri (never null; may alias this)
     */
    const Uri* normalizeScheme() const;

    /**
     * Encodes characters in the given string as '%'-escaped octets using the
     * UTF-8 scheme. Leaves letters ("A-Z", "a-z"), numbers ("0-9"), and
     * unreserved characters ("_-!.~'()*") intact. Encodes all other
     * characters.
     */
    static std::string encode(const std::string& s);

    /**
     * Encodes characters in the given string as '%'-escaped octets using the
     * UTF-8 scheme. Leaves letters, numbers, and unreserved characters
     * intact. Encodes all other characters with the exception of those
     * specified in the allow argument.
     * @param allow set of additional characters to allow in the encoded
     *              form, "" if no characters should be skipped
     */
    static std::string encode(const std::string& s, const std::string& allow);

    /**
     * Encodes a value if it wasn't already encoded. (AOSP depends on the
     * Flags.encodeAppIntent() feature flag, which defaults to disabled.)
     */
    static std::string encodeIfNotEncoded(const std::string& value, const std::string& allow);

    /**
     * Decodes '%'-escaped octets in the given string using the UTF-8 scheme.
     * Replaces invalid octets with the unicode replacement character
     * ("\\uFFFD").
     */
    static std::string decode(const std::string& s);

    /**
     * Decodes a string if it was encoded, indicated by containing a %.
     * (AOSP depends on Flags.encodeAppIntent(), disabled by default.)
     */
    static std::string decodeIfNeeded(const std::string& value);

    /**
     * Creates a new Uri by appending an already-encoded path segment to a
     * base Uri.
     * @param baseUri Uri to append path segment to
     * @param pathSegment encoded path segment to append
     * @return a new Uri based on baseUri with the given segment appended to
     *         the path (caller owns)
     */
    static Uri* withAppendedPath(const Uri& baseUri, const std::string& pathSegment);

    /**
     * If this Uri is file://, then resolve and return its canonical path.
     * (AOSP @hide; the legacy-emulated-storage rewrite has no CDROID
     * equivalent and is omitted.)
     */
    const Uri* getCanonicalUri() const;

    /** Error message presented when a user tries to treat an opaque URI as hierarchical. */
    static const char* NOT_HIERARCHICAL;

    /** Identifies a null parcelled Uri. */
    static constexpr int NULL_TYPE_ID = 0;

    // AOSP: Parcelable plumbing (CDROID has no Parcelable base; Parcel's
    // readString/writeString stand in for readString8/writeString8).
    virtual int describeContents() const;
    virtual void writeToParcel(Parcel& parcel, int flags) const;
    /** AOSP: static writeToParcel(Parcel out, Uri uri). */
    static void writeToParcel(Parcel& out, const Uri* uri);
    /** AOSP: Parcelable.Creator<Uri> CREATOR — createFromParcel(Parcel). */
    static Uri* readFromParcel(Parcel& in);

    /**********************************************************************
     * NotCachedHolder: holds the NOT_CACHED sentinel string compared by
     * identity in AOSP. Here the sentinel role is carried by the per-field
     * cache flags in AbstractPart / the Uri subclasses (see uri.cc).
     **********************************************************************/
    class NotCachedHolder {
    public:
        static const std::string NOT_CACHED;
    };

    /**********************************************************************
     * Support for part implementations. (AOSP: package-private
     * AbstractPart/Part/PathPart.)
     **********************************************************************/
    class AbstractPart {
    public:
        virtual ~AbstractPart();
        virtual std::string getEncoded() const = 0;
        /*final*/ std::string getDecoded() const;
        /* Java: getEncoded() != null — the Part.NULL vs Part.EMPTY split. */
        bool isNull() const { return mNull; }
    protected:
        /* encoded/decoded: nullptr = Java null, &NotCachedHolder::NOT_CACHED =
           the NOT_CACHED identity sentinel (AOSP compares by identity), else
           the value (copied). */
        AbstractPart(const std::string* encoded, const std::string* decoded);
        /* Java: volatile String encoded/decoded with null and NOT_CACHED states. */
        mutable std::string mEncoded;
        mutable std::string mDecoded;
        mutable bool mEncodedCached = false;
        mutable bool mDecodedCached = false;
        bool mNull = false;
    };

    /**
     * Immutable wrapper of encoded and decoded versions of a URI part. Lazily
     * creates the encoded or decoded version from the other.
     */
    class Part : public AbstractPart {
    public:
        /** A part with null values. AOSP: static final Part NULL. */
        static std::shared_ptr<const Part> NULL_();
        /** A part with empty strings for values. AOSP: static final Part EMPTY. */
        static std::shared_ptr<const Part> EMPTY_();

        /**
         * Returns given part or NULL_() if the given part is null.
         */
        static std::shared_ptr<const Part> nonNull(const std::shared_ptr<const Part>& part);

        /** Creates a part from the encoded string (nullptr = Java null). */
        static std::shared_ptr<const Part> fromEncoded(const std::string* encoded);

        /** Creates a part from the decoded string (nullptr = Java null). */
        static std::shared_ptr<const Part> fromDecoded(const std::string* decoded);

        /**
         * Creates a part from the encoded and decoded strings.
         * Either may be nullptr (Java null).
         */
        static std::shared_ptr<const Part> from(const std::string* encoded,
                const std::string* decoded);

        virtual bool isEmpty() const;
        virtual std::string getEncoded() const;
    protected:
        Part(const std::string* encoded, const std::string* decoded);
    private:
        class EmptyPart;
        friend class EmptyPart;
    };

    /**
     * Immutable wrapper of encoded and decoded versions of a path part.
     * Lazily creates the encoded or decoded version from the other.
     */
    class PathPart : public AbstractPart {
    public:
        /** A part with null values. AOSP: static final PathPart NULL. */
        static std::shared_ptr<const PathPart> NULL_();
        /** A part with empty strings for values. AOSP: static final PathPart EMPTY. */
        static std::shared_ptr<const PathPart> EMPTY_();

        virtual std::string getEncoded() const;

        /**
         * Gets the individual path segments. Parses them if necessary.
         * @return parsed path segments (EMPTY when not hierarchical)
         */
        std::shared_ptr<const PathSegments> getPathSegments() const;

        static std::shared_ptr<const PathPart> appendEncodedSegment(
                const std::shared_ptr<const PathPart>& oldPart, const std::string& newSegment);

        static std::shared_ptr<const PathPart> appendDecodedSegment(
                const std::shared_ptr<const PathPart>& oldPart, const std::string& decoded);

        /** Creates a path from the encoded string (nullptr = Java null). */
        static std::shared_ptr<const PathPart> fromEncoded(const std::string* encoded);

        /** Creates a path from the decoded string (nullptr = Java null). */
        static std::shared_ptr<const PathPart> fromDecoded(const std::string* decoded);

        /**
         * Creates a path from the encoded and decoded strings.
         */
        static std::shared_ptr<const PathPart> from(const std::string* encoded,
                const std::string* decoded);

        /**
         * Prepends path values with "/" if they're present, not empty, and
         * they don't already start with "/".
         */
        static std::shared_ptr<const PathPart> makeAbsolute(
                const std::shared_ptr<const PathPart>& oldPart);

    protected:
        PathPart(const std::string* encoded, const std::string* decoded);
    private:
        /* Cached path segments. */
        mutable std::shared_ptr<const PathSegments> mPathSegments;
    };

    /**********************************************************************
     * Wrapper for path segment array. (AOSP: PathSegments extends
     * AbstractList<String>.)
     **********************************************************************/
    class PathSegments {
    public:
        static std::shared_ptr<const PathSegments> EMPTY();

        const std::string& get(int index) const;
        int size() const;

        PathSegments();

        /* AOSP: final String[] segments (package-visible). */
        std::vector<std::string> mSegments;
    };

    /**
     * Builds PathSegments.
     */
    class PathSegmentsBuilder {
    public:
        void add(const std::string& segment);
        std::shared_ptr<const PathSegments> build();
    private:
        std::vector<std::string> mSegments;
    };

    /**********************************************************************
     * Helper class for building or manipulating URI references. Not safe for
     * concurrent use.
     *
     * An absolute hierarchical URI reference follows the pattern:
     *   <scheme>://<authority><absolute path>?<query>#<fragment>
     *
     * Relative URI references (which are always hierarchical) follow one of
     * two patterns: <relative or absolute path>?<query>#<fragment> or
     * //<authority><absolute path>?<query>#<fragment>
     *
     * An opaque URI follows this pattern: <scheme>:<opaque part>#<fragment>
     *
     * Use Uri::buildUpon() to obtain a builder representing an existing URI.
     **********************************************************************/
    class Builder {
    public:
        Builder();

        /**
         * Sets the scheme.
         * @param scheme name or "" if this is a relative Uri
         */
        Builder& scheme(const std::string& scheme);
        Builder& opaquePart(const std::shared_ptr<const Part>& opaquePart);
        /** Encodes and sets the given opaque scheme-specific-part. */
        Builder& opaquePart(const std::string& opaquePart);
        /** Sets the previously encoded opaque scheme-specific-part. */
        Builder& encodedOpaquePart(const std::string& opaquePart);
        Builder& authority(const std::shared_ptr<const Part>& authority);
        /** Encodes and sets the authority. */
        Builder& authority(const std::string& authority);
        /** Sets the previously encoded authority. */
        Builder& encodedAuthority(const std::string& authority);
        Builder& path(const std::shared_ptr<const PathPart>& path);
        /**
         * Sets the path. Leaves '/' characters intact but encodes others as
         * necessary.
         *
         * If the path is not empty and doesn't start with a '/', and if you
         * specify a scheme and/or authority, the builder will prepend the
         * given path with a '/'.
         */
        Builder& path(const std::string& path);
        /** Sets the previously encoded path. */
        Builder& encodedPath(const std::string& path);
        /** Encodes the given segment and appends it to the path. */
        Builder& appendPath(const std::string& newSegment);
        /** Appends the given segment to the path. */
        Builder& appendEncodedPath(const std::string& newSegment);
        Builder& query(const std::shared_ptr<const Part>& query);
        /** Encodes and sets the query. */
        Builder& query(const std::string& query);
        /** Sets the previously encoded query. */
        Builder& encodedQuery(const std::string& query);
        Builder& fragment(const std::shared_ptr<const Part>& fragment);
        /** Encodes and sets the fragment. */
        Builder& fragment(const std::string& fragment);
        /** Sets the previously encoded fragment. */
        Builder& encodedFragment(const std::string& fragment);

        /**
         * Encodes the key and value and then appends the parameter to the
         * query string.
         * @param key which will be encoded
         * @param value which will be encoded
         */
        Builder& appendQueryParameter(const std::string& key, const std::string& value);

        /**
         * Clears the previously set query.
         */
        Builder& clearQuery();

        /**
         * Constructs a Uri with the current attributes.
         * @return a new Uri (caller owns)
         */
        Uri* build() const;

        bool hasSchemeOrAuthority() const;

        std::string toString() const;

    private:
        std::string mScheme;        // "" == null (relative)
        std::shared_ptr<const Part> mOpaquePart;
        std::shared_ptr<const Part> mAuthority;
        std::shared_ptr<const PathPart> mPath;
        std::shared_ptr<const Part> mQuery;
        std::shared_ptr<const Part> mFragment;
    };

private:
    static bool isAllowed(char c, const std::string& allow);
    static bool isEncoded(const std::string& value, const std::string& allow);

protected:
    /* AOSP toSafeString() distinguishes Java-null authority/path from ""
       (e.g. "http://" has an empty-but-present authority). The public getters
       return std::string where "" == null, so these part-aware hooks carry
       the distinction. */
    virtual bool isAuthorityNull() const;
    virtual bool isPathNull() const;
};

/* The Uri subclasses are declared inside Uri (AOSP nests them too) but
   defined here, after Uri is complete: C++ forbids a nested class definition
   from inheriting its still-incomplete enclosing class (AOSP:
   "private static class StringUri extends AbstractHierarchicalUri"). Inside
   these definitions, unqualified Part/Builder/... still resolve from Uri's
   scope, so the bodies are AOSP-shaped.
*/
    /**********************************************************************
     * Support for hierarchical URIs. (AOSP: private abstract
     * AbstractHierarchicalUri extends Uri.)
     **********************************************************************/
    class Uri::AbstractHierarchicalUri : public Uri {
    public:
        virtual std::string getLastPathSegment() const;
        virtual std::string getEncodedUserInfo() const;
        virtual std::string getUserInfo() const;
        virtual std::string getHost() const;
        virtual int getPort() const;
    private:
        std::shared_ptr<const Part> getUserInfoPart() const;
        std::string parseUserInfo() const;
        std::string parseHost() const;
        int parsePort() const;
        static int findPortSeparator(const std::string& authority);

        mutable std::shared_ptr<const Part> mUserInfo;
        mutable bool mUserInfoCached = false;
        mutable std::string mHost;
        mutable bool mHostCached = false;
        mutable int mPort = NOT_CALCULATED;
    };

    /**
     * An implementation which wraps a String URI. This URI can be opaque or
     * hierarchical, but we extend AbstractHierarchicalUri in case we need
     * the hierarchical functionality. (AOSP: private static class StringUri.)
     */
    class Uri::StringUri : public Uri::AbstractHierarchicalUri {
    public:
        /** Used in parcelling. */
        static constexpr int TYPE_ID = 1;

        explicit StringUri(const std::string& uriString);

        static Uri* readFrom(Parcel& parcel);

        virtual int describeContents() const;
        virtual void writeToParcel(Parcel& parcel, int flags) const;

        /** Finds the first ':'. Returns -1 if none found. */
        int findSchemeSeparator() const;
        /** Finds the first '#'. Returns -1 if none found. */
        int findFragmentSeparator() const;

        virtual bool isHierarchical() const;
        virtual bool isRelative() const;

        virtual std::string getScheme() const;
        std::string parseScheme() const;

        std::shared_ptr<const Part> getSsp() const;
        virtual std::string getEncodedSchemeSpecificPart() const;
        virtual std::string getSchemeSpecificPart() const;
        std::string parseSsp() const;

        std::shared_ptr<const Part> getAuthorityPart() const;
        virtual std::string getEncodedAuthority() const;
        virtual std::string getAuthority() const;

        std::shared_ptr<const PathPart> getPathPart() const;
        virtual std::string getPath() const;
        virtual std::string getEncodedPath() const;
        virtual std::vector<std::string> getPathSegments() const;
        std::string parsePath() const;

        std::shared_ptr<const Part> getQueryPart() const;
        virtual std::string getEncodedQuery() const;
        std::string parseQuery() const;
        virtual std::string getQuery() const;

        std::shared_ptr<const Part> getFragmentPart() const;
        virtual std::string getEncodedFragment() const;
        std::string parseFragment() const;
        virtual std::string getFragment() const;

        virtual std::string toString() const;

        /**
         * Parses an authority out of the given URI string.
         * @param uriString URI string
         * @param ssi scheme separator index, -1 for a relative URI
         * @return the authority or empty if none is found
         */
        static std::string parseAuthority(const std::string& uriString, int ssi);

        /**
         * Parses a path out of this given URI string.
         * @param uriString URI string
         * @param ssi scheme separator index, -1 for a relative URI
         * @return the path
         */
        static std::string parsePath(const std::string& uriString, int ssi);

        virtual Builder buildUpon() const;

    protected:
        virtual bool isAuthorityNull() const;
        virtual bool isPathNull() const;

    private:
        /** URI string representation. */
        const std::string mUriString;

        /** Cached scheme separator index. */
        mutable int mCachedSsi = NOT_CALCULATED;
        /** Cached fragment separator index. */
        mutable int mCachedFsi = NOT_CALCULATED;
        mutable bool mSchemeCached = false;
        mutable std::string mScheme;
        mutable std::shared_ptr<const Part> mSsp;
        mutable std::shared_ptr<const Part> mAuthority;
        mutable std::shared_ptr<const PathPart> mPath;
        mutable std::shared_ptr<const Part> mQuery;
        mutable std::shared_ptr<const Part> mFragment;
    };

    /**
     * Opaque URI. (AOSP: private static class OpaqueUri extends Uri.)
     */
    class Uri::OpaqueUri : public Uri {
    public:
        /** Used in parcelling. */
        static constexpr int TYPE_ID = 2;

        OpaqueUri(const std::string& scheme, const std::shared_ptr<const Part>& ssp,
                const std::shared_ptr<const Part>& fragment);

        static Uri* readFrom(Parcel& parcel);

        virtual int describeContents() const;
        virtual void writeToParcel(Parcel& parcel, int flags) const;

        virtual bool isHierarchical() const;
        virtual bool isRelative() const;

        virtual std::string getScheme() const;

        virtual std::string getEncodedSchemeSpecificPart() const;
        virtual std::string getSchemeSpecificPart() const;

        virtual std::string getAuthority() const;
        virtual std::string getEncodedAuthority() const;

        virtual std::string getPath() const;
        virtual std::string getEncodedPath() const;

        virtual std::string getQuery() const;
        virtual std::string getEncodedQuery() const;

        virtual std::string getFragment() const;
        virtual std::string getEncodedFragment() const;

        virtual std::vector<std::string> getPathSegments() const;
        virtual std::string getLastPathSegment() const;

        virtual std::string getUserInfo() const;
        virtual std::string getEncodedUserInfo() const;

        virtual std::string getHost() const;
        virtual int getPort() const;

        virtual std::string toString() const;

        virtual Builder buildUpon() const;

    private:
        const std::string mScheme;  // "" == null
        const std::shared_ptr<const Part> mSsp;
        const std::shared_ptr<const Part> mFragment;
        mutable bool mCachedString = false;
        mutable std::string mCachedStringValue;
    };

    /**
     * Hierarchical Uri. (AOSP: private static class HierarchicalUri extends
     * AbstractHierarchicalUri.)
     */
    class Uri::HierarchicalUri : public Uri::AbstractHierarchicalUri {
    public:
        /** Used in parcelling. */
        static constexpr int TYPE_ID = 3;

        HierarchicalUri(const std::string& scheme, const std::shared_ptr<const Part>& authority,
                const std::shared_ptr<const PathPart>& path,
                const std::shared_ptr<const Part>& query,
                const std::shared_ptr<const Part>& fragment);

        static Uri* readFrom(Parcel& parcel);

        virtual int describeContents() const;
        virtual void writeToParcel(Parcel& parcel, int flags) const;

        virtual bool isHierarchical() const;
        virtual bool isRelative() const;

        virtual std::string getScheme() const;

        std::shared_ptr<const Part> getSsp() const;
        virtual std::string getEncodedSchemeSpecificPart() const;
        virtual std::string getSchemeSpecificPart() const;

        /**
         * Creates the encoded scheme-specific part from its sub parts.
         */
        std::string makeSchemeSpecificPart() const;
        void appendSspTo(std::string& builder) const;

        virtual std::string getAuthority() const;
        virtual std::string getEncodedAuthority() const;

        virtual std::string getEncodedPath() const;
        virtual std::string getPath() const;

        virtual std::string getQuery() const;
        virtual std::string getEncodedQuery() const;

        virtual std::string getFragment() const;
        virtual std::string getEncodedFragment() const;

        virtual std::vector<std::string> getPathSegments() const;

        virtual std::string toString() const;
        std::string makeUriString() const;

        virtual Builder buildUpon() const;

    protected:
        virtual bool isAuthorityNull() const;
        virtual bool isPathNull() const;

    private:
        std::shared_ptr<const PathPart> generatePath(const std::shared_ptr<const PathPart>& originalPath) const;

        const std::string mScheme;  // "" == null
        const std::shared_ptr<const Part> mAuthority;
        const std::shared_ptr<const PathPart> mPath;
        const std::shared_ptr<const Part> mQuery;
        const std::shared_ptr<const Part> mFragment;
        mutable std::shared_ptr<const Part> mSsp;
        mutable bool mUriStringCached = false;
        mutable std::string mUriString;
    };


} // namespace cdroid
#endif /* __CDROID_URI_H__ */
