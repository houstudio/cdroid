/*********************************************************************************
 * NavDeepLink tests — port of androidx.navigation.Navigation-common NavDeepLinkTest
 * (the subset CDROID implements; CDROID's NavDeepLink is a regex-based rewrite,
 * so tests for features it does not have are skipped, see below).
 *
 * Ported: exact match (+hyphen/plus/period escaping), {arg} path/query/fragment
 * extraction, typed extraction via NavArgument map, invalid-type parse -> null
 * args, missing required query param -> null args.
 *
 * Skipped (not implemented by the CDROID rewrite): scheme-optional matching
 * (deepLinkExactMatchNoScheme), ".*" wildcard/prefix matching, empty-string
 * {arg} match, URI percent-decoding of matched values, argument not crossing
 * '#', query-param default/nullable omission, repeated params -> arrays, and
 * the modern NavDeepLink(uri, action, mimetype) ctor tests (incl. mimeType /
 * action matching, see NavDeepLinkMimeTypeTest/NavDeepLinkActionTest).
 *********************************************************************************/
#include <gtest/gtest.h>
#include <navigation/navdeeplink.h>
#include <navigation/navargument.h>
#include <navigation/navtype.h>

using namespace cdroid;

namespace {
const std::string DEEP_LINK_EXACT_NO_SCHEME = "www.example.com";
const std::string DEEP_LINK_EXACT_HTTP  = "http://" + DEEP_LINK_EXACT_NO_SCHEME;
const std::string DEEP_LINK_EXACT_HTTPS = "https://" + DEEP_LINK_EXACT_NO_SCHEME;

// androidx.navigation.test helpers (intArgument()/stringArgument()).
NavArgument* intArgument()    { return NavArgument::Builder().setType(NavTypeKind::INT).build(); }
NavArgument* longArgument()   { return NavArgument::Builder().setType(NavTypeKind::LONG).build(); }
NavArgument* stringArgument() { return NavArgument::Builder().setType(NavTypeKind::STRING).build(); }
NavArgument* boolArgument()   { return NavArgument::Builder().setType(NavTypeKind::BOOL).build(); }

// RAII: getMatchingArguments returns an owned Bundle* and the map holds owned
// NavArgument*s (Builder::build) — release both when the test body ends.
struct OwnedArgs {
    std::map<std::string, NavArgument*> map;
    ~OwnedArgs(){
        for(auto& kv : map) delete kv.second;
    }
};
// Replace every {name} placeholder with value, like the Kotlin tests'
// deepLinkArgument.replace("{id}", id).
std::string withArg(const std::string& pattern, const std::string& name, const std::string& value){
    std::string out = pattern;
    const std::string token = "{" + name + "}";
    size_t pos;
    while((pos = out.find(token)) != std::string::npos) out.replace(pos, token.size(), value);
    return out;
}
} // namespace

TEST(NavDeepLink, ExactMatch) {
    NavDeepLink deepLink(DEEP_LINK_EXACT_HTTP);
    EXPECT_TRUE (deepLink.matches(DEEP_LINK_EXACT_HTTP));
    EXPECT_FALSE(deepLink.matches(DEEP_LINK_EXACT_HTTPS));
}

TEST(NavDeepLink, ExactMatchWithHyphens) {
    const std::string deepLinkString = "android-app://com.example";
    NavDeepLink deepLink(deepLinkString);
    EXPECT_TRUE(deepLink.matches(deepLinkString));
}

TEST(NavDeepLink, ExactMatchWithPlus) {
    const std::string deepLinkString = "android+app://com.example";
    NavDeepLink deepLink(deepLinkString);
    EXPECT_TRUE(deepLink.matches(deepLinkString));
}

TEST(NavDeepLink, ExactMatchWithPeriods) {
    const std::string deepLinkString = "android.app://com.example";
    NavDeepLink deepLink(deepLinkString);
    EXPECT_TRUE(deepLink.matches(deepLinkString));
}

TEST(NavDeepLink, ArgumentMatchWithoutArguments) {
    const std::string deepLinkArgument = DEEP_LINK_EXACT_HTTPS + "/users/{id}/posts";
    NavDeepLink deepLink(deepLinkArgument);
    OwnedArgs args; // empty map: placeholder returned as string
    Bundle* matchArgs = deepLink.getMatchingArguments(withArg(deepLinkArgument, "id", "2"), args.map);
    ASSERT_NE(matchArgs, nullptr);
    EXPECT_EQ(matchArgs->getString("id"), std::string("2"));
    delete matchArgs;
}

TEST(NavDeepLink, ArgumentMatch) {
    const std::string deepLinkArgument = DEEP_LINK_EXACT_HTTPS + "/users/{id}/posts";
    NavDeepLink deepLink(deepLinkArgument);
    OwnedArgs args;
    args.map["id"] = intArgument();
    Bundle* matchArgs = deepLink.getMatchingArguments(withArg(deepLinkArgument, "id", "2"), args.map);
    ASSERT_NE(matchArgs, nullptr);
    EXPECT_EQ(matchArgs->getInt("id"), 2);
    delete matchArgs;
}

TEST(NavDeepLink, ArgumentInvalidMatch) {
    const std::string deepLinkArgument = DEEP_LINK_EXACT_HTTPS + "/users/{id}/posts";
    NavDeepLink deepLink(deepLinkArgument);
    OwnedArgs args;
    args.map["id"] = intArgument();
    Bundle* matchArgs = deepLink.getMatchingArguments(withArg(deepLinkArgument, "id", "invalid"), args.map);
    EXPECT_EQ(matchArgs, nullptr);
}

TEST(NavDeepLink, ArgumentMatchWithQueryParams) {
    const std::string deepLinkArgument = DEEP_LINK_EXACT_HTTPS + "/users/{id}?myarg={myarg}";
    NavDeepLink deepLink(deepLinkArgument);
    OwnedArgs args;
    args.map["id"] = intArgument();
    args.map["myarg"] = stringArgument();
    std::string uri = withArg(withArg(deepLinkArgument, "id", "2"), "myarg", "test");
    Bundle* matchArgs = deepLink.getMatchingArguments(uri, args.map);
    ASSERT_NE(matchArgs, nullptr);
    EXPECT_EQ(matchArgs->getInt("id"), 2);
    EXPECT_EQ(matchArgs->getString("myarg"), std::string("test"));
    delete matchArgs;
}

// Ensure that arguments with multiple characters in the path get matched correctly.
TEST(NavDeepLink, MultiCharacterArgumentMatchWithQueryParams) {
    const std::string deepLinkArgument = DEEP_LINK_EXACT_HTTPS + "/users/{id}?myarg={myarg}";
    NavDeepLink deepLink(deepLinkArgument);
    OwnedArgs args;
    args.map["id"] = intArgument();
    args.map["myarg"] = stringArgument();
    std::string uri = withArg(withArg(deepLinkArgument, "id", "211"), "myarg", "test");
    Bundle* matchArgs = deepLink.getMatchingArguments(uri, args.map);
    ASSERT_NE(matchArgs, nullptr);
    EXPECT_EQ(matchArgs->getInt("id"), 211);
    EXPECT_EQ(matchArgs->getString("myarg"), std::string("test"));
    delete matchArgs;
}

// Ensure a question mark at the end of path params matches same as if there was none.
TEST(NavDeepLink, MultipleArgumentMatchQuestionMarkNoParams) {
    const std::string deepLinkArgument = DEEP_LINK_EXACT_HTTPS + "/users/{id}?";
    NavDeepLink deepLink(deepLinkArgument);
    OwnedArgs args;
    args.map["id"] = intArgument();
    Bundle* matchArgs = deepLink.getMatchingArguments(withArg(deepLinkArgument, "id", "211"), args.map);
    ASSERT_NE(matchArgs, nullptr);
    EXPECT_EQ(matchArgs->getInt("id"), 211);
    delete matchArgs;
}

TEST(NavDeepLink, MultipleArgumentMatch) {
    const std::string deepLinkArgument = DEEP_LINK_EXACT_HTTPS + "/users/{id}/posts/{postId}";
    NavDeepLink deepLink(deepLinkArgument);
    OwnedArgs args;
    args.map["id"] = intArgument();
    args.map["postId"] = longArgument();
    std::string uri = withArg(withArg(deepLinkArgument, "id", "2"), "postId", "42");
    Bundle* matchArgs = deepLink.getMatchingArguments(uri, args.map);
    ASSERT_NE(matchArgs, nullptr);
    EXPECT_EQ(matchArgs->getInt("id"), 2);
    EXPECT_EQ(matchArgs->getLong("postId"), 42L);
    delete matchArgs;
}

TEST(NavDeepLink, BooleanArgumentMatch) {
    const std::string deepLinkArgument = DEEP_LINK_EXACT_HTTPS + "/users/{active}";
    NavDeepLink deepLink(deepLinkArgument);
    OwnedArgs args;
    args.map["active"] = boolArgument();
    Bundle* matchArgs = deepLink.getMatchingArguments(withArg(deepLinkArgument, "active", "true"), args.map);
    ASSERT_NE(matchArgs, nullptr);
    EXPECT_TRUE(matchArgs->getBoolean("active"));
    delete matchArgs;
}

TEST(NavDeepLink, FragmentMatch) {
    const std::string deepLinkArgument = DEEP_LINK_EXACT_HTTPS + "/users#{frag}";
    NavDeepLink deepLink(deepLinkArgument);
    OwnedArgs args;
    args.map["frag"] = stringArgument();
    Bundle* matchArgs = deepLink.getMatchingArguments(DEEP_LINK_EXACT_HTTPS + "/users#testFrag", args.map);
    ASSERT_NE(matchArgs, nullptr);
    EXPECT_EQ(matchArgs->getString("frag"), std::string("testFrag"));
    delete matchArgs;
}

TEST(NavDeepLink, FragmentMatchWithQuery) {
    const std::string deepLinkArgument = DEEP_LINK_EXACT_HTTPS + "/users?id={id}#{frag}";
    NavDeepLink deepLink(deepLinkArgument);
    OwnedArgs args;
    args.map["id"] = intArgument();
    args.map["frag"] = stringArgument();
    Bundle* matchArgs = deepLink.getMatchingArguments(DEEP_LINK_EXACT_HTTPS + "/users?id=43#testFrag", args.map);
    ASSERT_NE(matchArgs, nullptr);
    EXPECT_EQ(matchArgs->getInt("id"), 43);
    EXPECT_EQ(matchArgs->getString("frag"), std::string("testFrag"));
    delete matchArgs;
}

TEST(NavDeepLink, MissingRequiredArgument) {
    const std::string deepLinkString = DEEP_LINK_EXACT_HTTPS + "/greeting?title={title}&text={text}";
    NavDeepLink deepLink(deepLinkString);
    OwnedArgs args;
    args.map["title"] = stringArgument();
    args.map["text"] = stringArgument();
    Bundle* matchArgs = deepLink.getMatchingArguments(DEEP_LINK_EXACT_HTTPS + "/greeting?title=No%20text", args.map);
    EXPECT_EQ(matchArgs, nullptr);
}

TEST(NavDeepLink, SingleQueryParamNoValue) {
    const std::string deepLinkArgument = DEEP_LINK_EXACT_HTTPS + "/users?{myarg}";
    NavDeepLink deepLink(deepLinkArgument);
    OwnedArgs args;
    args.map["myarg"] = stringArgument();
    Bundle* matchArgs = deepLink.getMatchingArguments(withArg(deepLinkArgument, "myarg", "name"), args.map);
    ASSERT_NE(matchArgs, nullptr);
    EXPECT_EQ(matchArgs->getString("myarg"), std::string("name"));
    delete matchArgs;
}

// androidx deepLinkPathEmptyInt: an empty {name} cannot satisfy a typed int arg —
// the pattern does not match at all, so args are null.
TEST(NavDeepLink, PathEmptyInt) {
    const std::string deepLinkArgument = DEEP_LINK_EXACT_HTTPS + "/{name}";
    NavDeepLink deepLink(deepLinkArgument);
    OwnedArgs args;
    args.map["name"] = intArgument();
    Bundle* matchArgs = deepLink.getMatchingArguments(withArg(deepLinkArgument, "name", ""), args.map);
    EXPECT_EQ(matchArgs, nullptr);
}
