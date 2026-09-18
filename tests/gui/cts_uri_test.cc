// AOSP CTS UriTest port (android.net.Uri, line-by-line expectations).
//
// Original: packages/modules/Connectivity/tests/cts/net/src/android/net/cts/UriTest.java
// (Apache 2.0). Ported against the android-36 Uri implementation in
// src/gui/core/uri.{h,cc}.
//
// Java->C++ adaptations (same conventions as uri.h):
//  - Java null String args/returns -> "" (Uri getters) or nullptr (test
//    helpers taking const char*, preserving the null-vs-"" split the
//    hierarchical-URI builder cases need).
//  - Uri::parse()/build() return Uri*; tests hold them in unique_ptr.
//    normalizeScheme()/getCanonicalUri() may alias `this` — the helper never
//    double-deletes.
//  - testParcelling is omitted: CDROID's Parcel is a stub (readInt() always
//    returns 0), so a roundtrip cannot assert anything.
//  - testToSafeString_customUri uses the SdkLevel.isAtLeastT() branch: the
//    pre-T full-string passthrough branch belongs to the legacy
//    implementation CDROID does not port.
//  - ContentUris (withAppendedId/parseId) is not ported; the two call sites
//    are emulated with the exact Builder expression ContentUris uses.
//  - PathSegments::get() out-of-bounds logs and returns "" instead of
//    throwing IndexOutOfBoundsException (checked via the internal class).

#include <gtest/gtest.h>
#include <core/uri.h>
#include <memory>
#include <string>
#include <vector>

using namespace cdroid;

namespace {

// Uri::normalizeScheme()/getCanonicalUri() may return `this`; delete only
// owned results.
struct UriRef {
    const Uri* uri;
    const Uri* owner;   // nullptr when uri aliases another Uri
    UriRef(const Uri* mayAlias, const Uri* self)
        : uri(mayAlias), owner(mayAlias == self ? nullptr : mayAlias) {}
    ~UriRef() { delete owner; }
};

const std::string U0102  = "\xC4\x82";        // Ā
const std::string U0840  = "\xE0\xA1\x80";    // ࡀ
const std::string UFFFD  = "\xEF\xBF\xBD";    // U+FFFD

} // namespace

TEST(CtsUriTest, testBuildUpon) {
    std::unique_ptr<Uri> u(Uri::parse("bob:lee")->buildUpon().scheme("robert").build());
    EXPECT_EQ("robert", u->getScheme());
    EXPECT_EQ("lee", u->getEncodedSchemeSpecificPart());
    EXPECT_EQ("lee", u->getSchemeSpecificPart());
    EXPECT_TRUE(u->getQuery().empty());      // AOSP: assertNull
    EXPECT_TRUE(u->getPath().empty());       // AOSP: assertNull
    EXPECT_TRUE(u->getAuthority().empty());  // AOSP: assertNull
    EXPECT_TRUE(u->getHost().empty());       // AOSP: assertNull

    std::unique_ptr<Uri> a(Uri::fromParts("foo", "bar", "tee"));
    std::unique_ptr<Uri> b(a->buildUpon().fragment("new").build());
    EXPECT_EQ("new", b->getFragment());
    EXPECT_EQ("bar", b->getSchemeSpecificPart());
    EXPECT_EQ("foo", b->getScheme());

    a.reset(Uri::Builder().scheme("foo").encodedOpaquePart("bar").fragment("tee").build());
    b.reset(a->buildUpon().fragment("new").build());
    EXPECT_EQ("new", b->getFragment());
    EXPECT_EQ("bar", b->getSchemeSpecificPart());
    EXPECT_EQ("foo", b->getScheme());

    a.reset(Uri::fromParts("scheme", "[2001:db8::dead:e1f]/foo", "bar"));
    b.reset(a->buildUpon().fragment("qux").build());
    EXPECT_EQ("qux", b->getFragment());
    EXPECT_EQ("[2001:db8::dead:e1f]/foo", b->getSchemeSpecificPart());
    EXPECT_EQ("scheme", b->getScheme());
}

TEST(CtsUriTest, testStringUri) {
    EXPECT_EQ("bob lee", Uri::parse("foo:bob%20lee")->getSchemeSpecificPart());
    EXPECT_EQ("bob%20lee", Uri::parse("foo:bob%20lee")->getEncodedSchemeSpecificPart());

    EXPECT_EQ("/bob%20lee", Uri::parse("foo:/bob%20lee")->getEncodedPath());
    EXPECT_TRUE(Uri::parse("foo:bob%20lee")->getPath().empty());   // AOSP: assertNull (opaque)

    EXPECT_EQ("bob%20lee", Uri::parse("foo:?bob%20lee")->getEncodedQuery());
    EXPECT_TRUE(Uri::parse("foo:bob%20lee")->getEncodedQuery().empty());
    EXPECT_TRUE(Uri::parse("foo:bar#?bob%20lee")->getQuery().empty());

    EXPECT_EQ("bob%20lee", Uri::parse("foo:#bob%20lee")->getEncodedFragment());

    std::unique_ptr<Uri> uri(Uri::parse("http://localhost:42"));
    EXPECT_EQ("localhost", uri->getHost());
    EXPECT_EQ(42, uri->getPort());

    uri.reset(Uri::parse("http://bob@localhost:42"));
    EXPECT_EQ("bob", uri->getUserInfo());
    EXPECT_EQ("localhost", uri->getHost());
    EXPECT_EQ(42, uri->getPort());

    uri.reset(Uri::parse("http://bob%20lee@localhost:42"));
    EXPECT_EQ("bob lee", uri->getUserInfo());
    EXPECT_EQ("bob%20lee", uri->getEncodedUserInfo());

    uri.reset(Uri::parse("http://localhost"));
    EXPECT_EQ("localhost", uri->getHost());
    EXPECT_EQ(-1, uri->getPort());

    uri.reset(Uri::parse("http://a:a@example.com:a@example2.com/path"));
    EXPECT_EQ("a:a@example.com:a@example2.com", uri->getAuthority());
    EXPECT_EQ("example2.com", uri->getHost());
    EXPECT_EQ(-1, uri->getPort());
    EXPECT_EQ("/path", uri->getPath());

    uri.reset(Uri::parse("http://a.foo.com\\.example.com/path"));
    EXPECT_EQ("a.foo.com", uri->getHost());
    EXPECT_EQ(-1, uri->getPort());
    EXPECT_EQ("\\.example.com/path", uri->getPath());

    uri.reset(Uri::parse("https://[2001:db8::dead:e1f]/foo"));
    EXPECT_EQ("[2001:db8::dead:e1f]", uri->getAuthority());
    EXPECT_TRUE(uri->getUserInfo().empty());
    EXPECT_EQ("[2001:db8::dead:e1f]", uri->getHost());
    EXPECT_EQ(-1, uri->getPort());
    EXPECT_EQ("/foo", uri->getPath());
    EXPECT_TRUE(uri->getFragment().empty());
    EXPECT_EQ("//[2001:db8::dead:e1f]/foo", uri->getSchemeSpecificPart());

    uri.reset(Uri::parse("https://[2001:db8::dead:e1f]/#foo"));
    EXPECT_EQ("[2001:db8::dead:e1f]", uri->getAuthority());
    EXPECT_TRUE(uri->getUserInfo().empty());
    EXPECT_EQ("[2001:db8::dead:e1f]", uri->getHost());
    EXPECT_EQ(-1, uri->getPort());
    EXPECT_EQ("/", uri->getPath());
    EXPECT_EQ("foo", uri->getFragment());
    EXPECT_EQ("//[2001:db8::dead:e1f]/", uri->getSchemeSpecificPart());

    uri.reset(Uri::parse("https://some:user@[2001:db8::dead:e1f]:1234/foo?corge=thud&corge=garp#bar"));
    EXPECT_EQ("some:user@[2001:db8::dead:e1f]:1234", uri->getAuthority());
    EXPECT_EQ("some:user", uri->getUserInfo());
    EXPECT_EQ("[2001:db8::dead:e1f]", uri->getHost());
    EXPECT_EQ(1234, uri->getPort());
    EXPECT_EQ("/foo", uri->getPath());
    EXPECT_EQ("bar", uri->getFragment());
    EXPECT_EQ("//some:user@[2001:db8::dead:e1f]:1234/foo?corge=thud&corge=garp",
            uri->getSchemeSpecificPart());
    EXPECT_EQ("corge=thud&corge=garp", uri->getQuery());
    EXPECT_EQ("thud", uri->getQueryParameter("corge"));
    EXPECT_EQ((std::vector<std::string>{"thud", "garp"}), uri->getQueryParameters("corge"));
}

TEST(CtsUriTest, testCompareTo) {
    std::unique_ptr<Uri> a(Uri::parse("foo:a"));
    std::unique_ptr<Uri> b(Uri::parse("foo:b"));
    std::unique_ptr<Uri> b2(Uri::parse("foo:b"));

    EXPECT_TRUE(a->compareTo(*b) < 0);
    EXPECT_TRUE(b->compareTo(*a) > 0);
    EXPECT_EQ(0, b->compareTo(*b2));
}

TEST(CtsUriTest, testEqualsAndHashCode) {
    std::unique_ptr<Uri> a(Uri::parse("http://crazybob.org/test/?foo=bar#tee"));

    std::unique_ptr<Uri> b(Uri::Builder()
            .scheme("http")
            .authority("crazybob.org")
            .path("/test/")
            .encodedQuery("foo=bar")
            .fragment("tee")
            .build());

    // Try alternate builder methods.
    std::unique_ptr<Uri> c(Uri::Builder()
            .scheme("http")
            .encodedAuthority("crazybob.org")
            .encodedPath("/test/")
            .encodedQuery("foo=bar")
            .encodedFragment("tee")
            .build());

    // AOSP: assertFalse(Uri.EMPTY.equals(null)) — no null Uri overload in C++.
    EXPECT_FALSE(*Uri::EMPTY() == *a);
    EXPECT_TRUE(*a == *b);
    EXPECT_TRUE(*b == *c);
    EXPECT_TRUE(*c == *a);
    EXPECT_EQ(a->hashCode(), b->hashCode());
    EXPECT_EQ(b->hashCode(), c->hashCode());
}

namespace {
void assertEncodeDecodeRoundtripExact(const std::string& s) {
    EXPECT_EQ(s, Uri::decode(Uri::encode(s, "")));
}
} // namespace

TEST(CtsUriTest, testEncodeAndDecode) {
    const std::string encoded = Uri::encode("Bob:/", "/");
    EXPECT_EQ(std::string::npos, encoded.find(':'));
    EXPECT_NE(std::string::npos, encoded.find('/'));
    assertEncodeDecodeRoundtripExact("");
    assertEncodeDecodeRoundtripExact("Bob");
    assertEncodeDecodeRoundtripExact(":Bob");
    assertEncodeDecodeRoundtripExact("::Bob");
    assertEncodeDecodeRoundtripExact("Bob::Lee");
    assertEncodeDecodeRoundtripExact("Bob:Lee");
    assertEncodeDecodeRoundtripExact("Bob::");
    assertEncodeDecodeRoundtripExact("Bob:");
    assertEncodeDecodeRoundtripExact("::Bob::");
    assertEncodeDecodeRoundtripExact("https:/some:user@[2001:db8::dead:e1f]:1234/foo#bar");
    // AOSP also roundtrips null (null == decode(encode(null))); "" above covers
    // the C++ null mapping.
}

TEST(CtsUriTest, testDecode_emptyString_returnsEmptyString) {
    EXPECT_EQ("", Uri::decode(""));
}

// AOSP: testDecode_null_returnsNull — C++ has no null strings; the "" case
// above covers the mapping.

TEST(CtsUriTest, testDecode_wrongHexDigit) {
    // %p in the end.
    EXPECT_EQ("ab/$" + U0102 + "%" + U0840 + UFFFD + std::string(1, '\0'),
            Uri::decode("ab%2f$%C4%82%25%e0%a1%80%p"));
}

TEST(CtsUriTest, testDecode_secondHexDigitWrong) {
    // %1p in the end.
    EXPECT_EQ("ab/$" + U0102 + "%" + U0840 + UFFFD + std::string(1, '\x01'),
            Uri::decode("ab%2f$%c4%82%25%e0%a1%80%1p"));
}

TEST(CtsUriTest, testDecode_endsWithPercent_appendsUnknownCharacter) {
    // % in the end.
    EXPECT_EQ("ab/$" + U0102 + "%" + U0840 + UFFFD,
            Uri::decode("ab%2f$%c4%82%25%e0%a1%80%"));
}

TEST(CtsUriTest, testDecode_plusNotConverted) {
    EXPECT_EQ("ab/$" + U0102 + "%+" + U0840,
            Uri::decode("ab%2f$%c4%82%25+%e0%a1%80"));
}

// Last character needs decoding (make sure we are flushing the buffer with chars to decode).
TEST(CtsUriTest, testDecode_lastCharacter) {
    EXPECT_EQ("ab/$" + U0102 + "%" + U0840,
            Uri::decode("ab%2f$%c4%82%25%e0%a1%80"));
}

// Check that a second row of encoded characters is decoded properly (internal buffers are
// reset properly).
TEST(CtsUriTest, testDecode_secondRowOfEncoded) {
    EXPECT_EQ("ab/$" + U0102 + "%" + U0840 + "aa" + U0840,
            Uri::decode("ab%2f$%c4%82%25%e0%a1%80aa%e0%a1%80"));
}

TEST(CtsUriTest, testFromFile) {
    std::unique_ptr<Uri> uri(Uri::fromFile("/tmp/bob"));
    EXPECT_EQ("file:///tmp/bob", uri->toString());
    // AOSP: fromFile(null) throws NPE — no null strings in C++.
}

TEST(CtsUriTest, testQueryParameters) {
    std::unique_ptr<Uri> uri(Uri::parse("content://user"));
    EXPECT_EQ("", uri->getQueryParameter("a"));   // AOSP: null

    uri.reset(uri->buildUpon().appendQueryParameter("a", "b").build());
    EXPECT_EQ("b", uri->getQueryParameter("a"));

    uri.reset(uri->buildUpon().appendQueryParameter("a", "b2").build());
    EXPECT_EQ((std::vector<std::string>{"b", "b2"}), uri->getQueryParameters("a"));

    uri.reset(uri->buildUpon().appendQueryParameter("c", "d").build());
    EXPECT_EQ((std::vector<std::string>{"b", "b2"}), uri->getQueryParameters("a"));
    EXPECT_EQ("d", uri->getQueryParameter("c"));
}

TEST(CtsUriTest, testPathOperations) {
    std::unique_ptr<Uri> uri(Uri::parse("content://user/a/b"));

    EXPECT_EQ(2u, uri->getPathSegments().size());
    EXPECT_EQ("a", uri->getPathSegments()[0]);
    EXPECT_EQ("b", uri->getPathSegments()[1]);
    EXPECT_EQ("b", uri->getLastPathSegment());

    std::unique_ptr<Uri> first(Uri::parse("content://user/a/b"));
    uri.reset(uri->buildUpon().appendPath("c").build());
    EXPECT_EQ(3u, uri->getPathSegments().size());
    EXPECT_EQ("c", uri->getPathSegments()[2]);
    EXPECT_EQ("c", uri->getLastPathSegment());
    EXPECT_EQ("content://user/a/b/c", uri->toString());

    // ContentUris.withAppendedId(uri, 100) == buildUpon().appendEncodedPath(id).build()
    uri.reset(uri->buildUpon().appendEncodedPath("100").build());
    EXPECT_EQ(4u, uri->getPathSegments().size());
    EXPECT_EQ("100", uri->getPathSegments()[3]);
    EXPECT_EQ("100", uri->getLastPathSegment());
    EXPECT_EQ(100, std::stol(uri->getLastPathSegment()));   // AOSP: ContentUris.parseId
    EXPECT_EQ("content://user/a/b/c/100", uri->toString());

    // Make sure the original URI is still intact.
    EXPECT_EQ(2u, first->getPathSegments().size());
    EXPECT_EQ("b", first->getLastPathSegment());

    // AOSP: first.getPathSegments().get(2) throws IndexOutOfBoundsException.
    // CDROID: the public API returns std::vector (bounds-checked by the
    // container itself); the internal PathSegments::get logs and returns "".
    const auto& segments = first->getPathSegments();
    EXPECT_GE(2u, segments.size());   // index 2 is out of bounds (size 2)
    EXPECT_EQ("", Uri::PathSegments::EMPTY()->get(0));

    EXPECT_EQ("", Uri::EMPTY()->getLastPathSegment());   // AOSP: null

    std::unique_ptr<Uri> withC(Uri::parse("foo:/a/b/")->buildUpon().appendPath("c").build());
    EXPECT_EQ("/a/b/c", withC->getPath());
}

namespace {
void testOpaqueUri(const Uri& uri) {
    EXPECT_EQ("mailto", uri.getScheme());
    EXPECT_EQ("nobody", uri.getSchemeSpecificPart());
    EXPECT_EQ("nobody", uri.getEncodedSchemeSpecificPart());

    EXPECT_TRUE(uri.getFragment().empty());   // AOSP: assertNull
    EXPECT_TRUE(uri.isAbsolute());
    EXPECT_TRUE(uri.isOpaque());
    EXPECT_FALSE(uri.isRelative());
    EXPECT_FALSE(uri.isHierarchical());

    EXPECT_TRUE(uri.getAuthority().empty());   // AOSP: assertNull
    EXPECT_TRUE(uri.getEncodedAuthority().empty());
    EXPECT_TRUE(uri.getPath().empty());
    EXPECT_TRUE(uri.getEncodedPath().empty());
    EXPECT_TRUE(uri.getUserInfo().empty());
    EXPECT_TRUE(uri.getEncodedUserInfo().empty());
    EXPECT_TRUE(uri.getQuery().empty());
    EXPECT_TRUE(uri.getEncodedQuery().empty());
    EXPECT_TRUE(uri.getHost().empty());
    EXPECT_EQ(-1, uri.getPort());

    EXPECT_TRUE(uri.getPathSegments().empty());
    EXPECT_TRUE(uri.getLastPathSegment().empty());

    EXPECT_EQ("mailto:nobody", uri.toString());

    std::unique_ptr<Uri> withFragment(uri.buildUpon().fragment("top").build());
    EXPECT_EQ("mailto:nobody#top", withFragment->toString());
}
} // namespace

TEST(CtsUriTest, testOpaqueUri) {
    std::unique_ptr<Uri> uri(Uri::parse("mailto:nobody"));
    testOpaqueUri(*uri);

    uri.reset(uri->buildUpon().build());
    testOpaqueUri(*uri);

    uri.reset(Uri::fromParts("mailto", "nobody", ""));
    testOpaqueUri(*uri);

    uri.reset(uri->buildUpon().build());
    testOpaqueUri(*uri);

    uri.reset(Uri::Builder().scheme("mailto").opaquePart("nobody").build());
    testOpaqueUri(*uri);

    uri.reset(uri->buildUpon().build());
    testOpaqueUri(*uri);
}

namespace {
void compareHierarchical(const std::string& uriString, const std::string& ssp,
        const Uri& uri, const char* scheme, const char* authority, const char* path,
        const char* query, const char* fragment) {
    EXPECT_EQ(scheme ? scheme : "", uri.getScheme());
    EXPECT_EQ(authority ? authority : "", uri.getAuthority());
    EXPECT_EQ(authority ? authority : "", uri.getEncodedAuthority());
    EXPECT_EQ(path ? path : "", uri.getPath());
    EXPECT_EQ(path ? path : "", uri.getEncodedPath());
    EXPECT_EQ(query ? query : "", uri.getQuery());
    EXPECT_EQ(query ? query : "", uri.getEncodedQuery());
    EXPECT_EQ(fragment ? fragment : "", uri.getFragment());
    EXPECT_EQ(fragment ? fragment : "", uri.getEncodedFragment());
    EXPECT_EQ(ssp, uri.getSchemeSpecificPart());

    if (scheme != nullptr) {
        EXPECT_TRUE(uri.isAbsolute());
        EXPECT_FALSE(uri.isRelative());
    } else {
        EXPECT_FALSE(uri.isAbsolute());
        EXPECT_TRUE(uri.isRelative());
    }

    EXPECT_FALSE(uri.isOpaque());
    EXPECT_TRUE(uri.isHierarchical());
    EXPECT_EQ(uriString, uri.toString());
}

void testHierarchical(const char* scheme, const char* authority,
        const char* path, const char* query, const char* fragment) {
    std::string sb;

    if (authority != nullptr) {
        sb += "//";
        sb += authority;
    }
    if (path != nullptr) {
        sb += path;
    }
    if (query != nullptr) {
        sb += '?';
        sb += query;
    }

    const std::string ssp = sb;

    if (scheme != nullptr) {
        sb.insert(0, std::string(scheme) + ":");
    }
    if (fragment != nullptr) {
        sb += '#';
        sb += fragment;
    }

    const std::string uriString = sb;

    std::unique_ptr<Uri> uri(Uri::parse(uriString));

    // Run these twice to test caching.
    compareHierarchical(uriString, ssp, *uri, scheme, authority, path, query, fragment);
    compareHierarchical(uriString, ssp, *uri, scheme, authority, path, query, fragment);

    // Test rebuilt version.
    uri.reset(uri->buildUpon().build());

    // Run these twice to test caching.
    compareHierarchical(uriString, ssp, *uri, scheme, authority, path, query, fragment);
    compareHierarchical(uriString, ssp, *uri, scheme, authority, path, query, fragment);

    // The decoded and encoded versions of the inputs are all the same.
    // We'll test the actual encoding decoding separately.

    // Test building with encoded versions.
    {
        Uri::Builder b;
        if (scheme) b.scheme(scheme);
        if (authority) b.encodedAuthority(authority);
        if (path) b.encodedPath(path);
        if (query) b.encodedQuery(query);
        if (fragment) b.encodedFragment(fragment);
        std::unique_ptr<Uri> built(b.build());

        compareHierarchical(uriString, ssp, *built, scheme, authority, path, query, fragment);
        compareHierarchical(uriString, ssp, *built, scheme, authority, path, query, fragment);
    }

    // Test building with decoded versions.
    {
        Uri::Builder b;
        if (scheme) b.scheme(scheme);
        if (authority) b.authority(authority);
        if (path) b.path(path);
        if (query) b.query(query);
        if (fragment) b.fragment(fragment);
        std::unique_ptr<Uri> built(b.build());

        compareHierarchical(uriString, ssp, *built, scheme, authority, path, query, fragment);
        compareHierarchical(uriString, ssp, *built, scheme, authority, path, query, fragment);

        // Rebuild.
        built.reset(built->buildUpon().build());

        compareHierarchical(uriString, ssp, *built, scheme, authority, path, query, fragment);
        compareHierarchical(uriString, ssp, *built, scheme, authority, path, query, fragment);
    }
}
} // namespace

TEST(CtsUriTest, testHierarchicalUris) {
    testHierarchical("http", "google.com", "/p1/p2", "query", "fragment");
    testHierarchical("file", nullptr, "/p1/p2", nullptr, nullptr);
    testHierarchical("content", "contact", "/p1/p2", nullptr, nullptr);
    testHierarchical("http", "google.com", "/p1/p2", nullptr, "fragment");
    testHierarchical("http", "google.com", "", nullptr, "fragment");
    testHierarchical("http", "google.com", "", "query", "fragment");
    testHierarchical("http", "google.com", "", "query", nullptr);
    testHierarchical("http", nullptr, "/", "query", nullptr);
}

namespace {
void checkNormalize(const std::string& expected, const std::string& original) {
    std::unique_ptr<Uri> o(Uri::parse(original));
    UriRef n(o->normalizeScheme(), o.get());
    EXPECT_EQ(expected, n.uri->toString());
}
} // namespace

TEST(CtsUriTest, testNormalizeScheme) {
    checkNormalize("", "");
    checkNormalize("http://www.android.com", "http://www.android.com");
    checkNormalize("http://USER@WWW.ANDROID.COM:100/ABOUT?foo=blah@bar=bleh#c",
            "HTTP://USER@WWW.ANDROID.COM:100/ABOUT?foo=blah@bar=bleh#c");
}

namespace {
void checkToSafeString(const std::string& expectedSafeString, const std::string& original) {
    std::unique_ptr<Uri> uri(Uri::parse(original));
    EXPECT_EQ(expectedSafeString, uri->toSafeString());
}
} // namespace

TEST(CtsUriTest, testToSafeString_tel) {
    checkToSafeString("tel:xxxxxx", "tel:Google");
    checkToSafeString("tel:xxxxxxxxxx", "tel:1234567890");
    checkToSafeString("tEl:xxx.xxx-xxxx", "tEl:123.456-7890");
}

TEST(CtsUriTest, testToSafeString_sip) {
    checkToSafeString("sip:xxxxxxx@xxxxxxx.xxxxxxxx", "sip:android@android.com:1234");
    checkToSafeString("sIp:xxxxxxx@xxxxxxx.xxx", "sIp:android@android.com");
}

TEST(CtsUriTest, testToSafeString_sms) {
    checkToSafeString("sms:xxxxxx", "sms:123abc");
    checkToSafeString("smS:xxx.xxx-xxxx", "smS:123.456-7890");
}

TEST(CtsUriTest, testToSafeString_smsto) {
    checkToSafeString("smsto:xxxxxx", "smsto:123abc");
    checkToSafeString("SMSTo:xxx.xxx-xxxx", "SMSTo:123.456-7890");
}

TEST(CtsUriTest, testToSafeString_mailto) {
    checkToSafeString("mailto:xxxxxxx@xxxxxxx.xxx", "mailto:android@android.com");
    checkToSafeString("Mailto:xxxxxxx@xxxxxxx.xxxxxxxxxx", "Mailto:android@android.com/secret");
}

TEST(CtsUriTest, testToSafeString_nfc) {
    checkToSafeString("nfc:xxxxxx", "nfc:123abc");
    checkToSafeString("nfc:xxx.xxx-xxxx", "nfc:123.456-7890");
    checkToSafeString("nfc:xxxxxxx@xxxxxxx.xxx", "nfc:android@android.com");
}

TEST(CtsUriTest, testToSafeString_http) {
    checkToSafeString("http://www.android.com/...", "http://www.android.com");
    checkToSafeString("HTTP://www.android.com/...", "HTTP://www.android.com");
    checkToSafeString("http://www.android.com/...", "http://www.android.com/");
    checkToSafeString("http://www.android.com/...", "http://www.android.com/secretUrl?param");
    checkToSafeString("http://www.android.com/...", "http://user:pwd@www.android.com/secretUrl?param");
    checkToSafeString("http://www.android.com/...", "http://user@www.android.com/secretUrl?param");
    checkToSafeString("http://www.android.com/...", "http://www.android.com/secretUrl?param");
    checkToSafeString("http:///...", "http:///path?param");
    checkToSafeString("http:///...", "http://");
    checkToSafeString("http://:12345/...", "http://:12345/");
}

TEST(CtsUriTest, testToSafeString_https) {
    checkToSafeString("https://www.android.com/...", "https://www.android.com/secretUrl?param");
    checkToSafeString("https://www.android.com:8443/...", "https://user:pwd@www.android.com:8443/secretUrl?param");
    checkToSafeString("https://www.android.com/...", "https://user:pwd@www.android.com");
    checkToSafeString("Https://www.android.com/...", "Https://user:pwd@www.android.com");
}

TEST(CtsUriTest, testToSafeString_ftp) {
    checkToSafeString("ftp://ftp.android.com/...", "ftp://ftp.android.com/");
    checkToSafeString("ftP://ftp.android.com/...", "ftP://anonymous@ftp.android.com/");
    checkToSafeString("ftp://ftp.android.com:2121/...", "ftp://root:love@ftp.android.com:2121/");
}

TEST(CtsUriTest, testToSafeString_rtsp) {
    checkToSafeString("rtsp://rtsp.android.com/...", "rtsp://rtsp.android.com/");
    checkToSafeString("rtsp://rtsp.android.com/...", "rtsp://rtsp.android.com/video.mov");
    checkToSafeString("rtsp://rtsp.android.com/...", "rtsp://rtsp.android.com/video.mov?param");
    checkToSafeString("RtsP://rtsp.android.com/...", "RtsP://anonymous@rtsp.android.com/");
    checkToSafeString("rtsp://rtsp.android.com:2121/...", "rtsp://username:password@rtsp.android.com:2121/");
}

// SdkLevel.isAtLeastT() branch of the AOSP testToSafeString_customUri.
TEST(CtsUriTest, testToSafeString_customUri) {
    checkToSafeString("other://ajkakjah/...", "other://ajkakjah/askdha/secret?secret");
    checkToSafeString("unsupported:", "unsupported:foo//bar");
    checkToSafeString("other://host:80/...", "other://user@host:80/secret/path/");
    checkToSafeString("content://contacts/...", "content://contacts/secret/path/name@foo.com");
    checkToSafeString("file:///...", "file:///path/to/secret.doc");
}
