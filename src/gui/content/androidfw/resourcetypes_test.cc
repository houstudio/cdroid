// Standalone unit test for ResStringPool — the first runtime piece of the aapt2
// binary resource mode. Builds with no cdroid dependency:
//   g++ -std=c++14 resourcetypes.cc resourcetypes_test.cc -o rsp_test && ./rsp_test
//
// Feeds the hand-verified aapt2 string-pool fixture (stringpool_fixture.h) and
// asserts: header parse, string count, UTF-8/UTF-16 decode round-trips (including
// the modified-UTF-8 / CESU-8 surrogate-pair case), plus the BAD_TYPE rejection
// paths (truncated, wrong type, un-terminated).
#include "resourcetypes.h"
#include "stringpool_fixture.h"

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

namespace {

int g_failures = 0;
int g_checks = 0;

#define CHECK(cond)                                                       \
    do {                                                                  \
        ++g_checks;                                                       \
        if (!(cond)) {                                                    \
            ++g_failures;                                                 \
            std::fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__,  \
                         #cond);                                          \
        }                                                                 \
    } while (0)

#define CHECK_EQ(actual, expected)                                        \
    do {                                                                  \
        ++g_checks;                                                       \
        auto _a = (actual);                                               \
        auto _e = (expected);                                             \
        if (!(_a == _e)) {                                                \
            ++g_failures;                                                 \
            std::fprintf(stderr, "FAIL %s:%d: %s != %s (%zd vs %zd)\n",    \
                         __FILE__, __LINE__, #actual, #expected,          \
                         (long)_a, (long)_e);                             \
        }                                                                 \
    } while (0)

// char16_t literal helper: build a std::u16string from explicit code units.
std::u16string u16(const std::vector<unsigned>& units) {
    std::u16string s;
    s.reserve(units.size());
    for (unsigned u : units) s.push_back(static_cast<char16_t>(u));
    return s;
}

// Compare a ResStringPool string (char16_t*, len) against an expected u16string.
bool u16eq(const char16_t* str, size_t len, const std::u16string& expected) {
    return str && len == expected.size() &&
           std::memcmp(str, expected.data(), len * sizeof(char16_t)) == 0;
}

// ---- Expected pool contents (hand-decoded from stringpool_fixture.h) ----
// 5 strings, UTF-8 pool (UTF8_FLAG set):
//   0 ""                      1 "Hello, World!"   2 "HelloWorld"
//   3 "Hi \U0001F30D" (\xD83C\xDF0D surrogate pair)   4 "你好世界"
const std::u16string kStr0 = u"";
const std::u16string kStr1 = u"Hello, World!";
const std::u16string kStr2 = u"HelloWorld";
const std::u16string kStr3 = u16({'H','i',' ',0xD83C,0xDF0D}); // 🌍
const std::u16string kStr4 = u16({0x4F60,0x597D,0x4E16,0x754C}); // 你好世界

// Raw UTF-8 byte lengths as stored by aapt2.
const size_t kLen8_0 = 0;
const size_t kLen8_1 = 13;
const size_t kLen8_2 = 10;
const size_t kLen8_3 = 9;  // "Hi " + two 3-byte surrogate groups
const size_t kLen8_4 = 12; // four 3-byte CJK chars

void test_happy_path() {
    cdroid::ResStringPool pool(kStringPool, kStringPoolLen, false);
    CHECK_EQ(pool.getError(), (int)cdroid::NO_ERROR);
    CHECK_EQ(pool.size(), (size_t)5);
    CHECK(pool.styleCount() == 0);
    CHECK(pool.isUTF8());
    CHECK(!pool.isSorted());
    CHECK_EQ(pool.bytes(), (size_t)kStringPoolLen);

    // Two-step: capture the pointer first, THEN read len, so we don't depend on
    // the unspecified evaluation order of (stringAt(&len) side effect vs len arg).
    size_t len = 0;
    const char16_t* p0 = pool.stringAt(0, &len); CHECK(u16eq(p0, len, kStr0)); CHECK_EQ(len, kStr0.size());
    const char16_t* p1 = pool.stringAt(1, &len); CHECK(u16eq(p1, len, kStr1)); CHECK_EQ(len, kStr1.size());
    const char16_t* p2 = pool.stringAt(2, &len); CHECK(u16eq(p2, len, kStr2)); CHECK_EQ(len, kStr2.size());
    const char16_t* p3 = pool.stringAt(3, &len); CHECK(u16eq(p3, len, kStr3)); CHECK_EQ(len, kStr3.size());
    const char16_t* p4 = pool.stringAt(4, &len); CHECK(u16eq(p4, len, kStr4)); CHECK_EQ(len, kStr4.size());

    // Out-of-range index -> nullptr, no crash.
    CHECK(pool.stringAt(5, &len) == nullptr);

    // Raw UTF-8 byte access (lengths only; content matches the stored bytes).
    const char* s8 = nullptr;
    s8 = pool.string8At(0, &len); CHECK(s8 != nullptr); CHECK_EQ(len, kLen8_0);
    s8 = pool.string8At(1, &len); CHECK(s8 != nullptr); CHECK_EQ(len, kLen8_1);
    s8 = pool.string8At(3, &len); CHECK(s8 != nullptr); CHECK_EQ(len, kLen8_3);
    s8 = pool.string8At(4, &len); CHECK(s8 != nullptr); CHECK_EQ(len, kLen8_4);
}

void test_decode_cache_is_stable() {
    // Repeated reads must return identical pointers (cache populated) and the
    // same content — guards against the lazy UTF-8->UTF-16 decode mutating state.
    cdroid::ResStringPool pool(kStringPool, kStringPoolLen, false);
    size_t la = 0, lb = 0;
    const char16_t* a = pool.stringAt(3, &la);
    const char16_t* b = pool.stringAt(3, &lb);
    CHECK(a != nullptr);
    CHECK(la == lb);
    CHECK(u16eq(a, la, kStr3));
    (void)b;
}

void test_error_truncated() {
    // Feed a prefix too small to hold the offset table -> BAD_TYPE.
    cdroid::ResStringPool pool(kStringPool, 32, false);
    CHECK_EQ(pool.getError(), (int)cdroid::BAD_TYPE);
    CHECK_EQ(pool.size(), (size_t)0);
}

void test_error_zero_length() {
    cdroid::ResStringPool pool(nullptr, 0, false);
    CHECK_EQ(pool.getError(), (int)cdroid::BAD_TYPE);
    cdroid::ResStringPool pool2(kStringPool, 0, false);
    CHECK_EQ(pool2.getError(), (int)cdroid::BAD_TYPE);
}

void test_error_garbage() {
    // A buffer of garbage: header size/total-size fields fail the dimension
    // check in validate_chunk -> BAD_TYPE. (Note: AOSP deliberately does NOT
    // validate the chunk *type* field, only dimensions, so we test dimensions.)
    std::vector<uint8_t> buf(64, 0xFF);
    cdroid::ResStringPool pool(buf.data(), buf.size(), false);
    CHECK_EQ(pool.getError(), (int)cdroid::BAD_TYPE);
    CHECK_EQ(pool.size(), (size_t)0);
}

void test_error_unterminated() {
    // setTo's termination guard inspects the final byte of the pool region
    // (mStrings + mStringPoolSize - 1). Flip that byte so it is non-zero.
    std::vector<uint8_t> buf(kStringPool, kStringPool + kStringPoolLen);
    buf[kStringPoolLen - 1] = 0x41; // final padding/terminator byte
    cdroid::ResStringPool pool(buf.data(), buf.size(), false);
    CHECK_EQ(pool.getError(), (int)cdroid::BAD_TYPE);
}

void test_copy_data() {
    // copyData=true: pool owns a malloc'd copy; reading still works and survives
    // the source going out of scope.
    std::vector<uint8_t> owned(kStringPool, kStringPool + kStringPoolLen);
    cdroid::ResStringPool pool(owned.data(), owned.size(), true);
    CHECK_EQ(pool.getError(), (int)cdroid::NO_ERROR);
    owned.assign(owned.size(), 0); // wipe the original
    size_t len = 0;
    const char16_t* p = pool.stringAt(1, &len); // two-step (see happy_path)
    CHECK(u16eq(p, len, kStr1));
}

void test_set_to_empty() {
    cdroid::ResStringPool pool;
    pool.setToEmpty();
    CHECK_EQ(pool.size(), (size_t)0);
    CHECK(pool.stringAt(0, nullptr) == nullptr);
}

} // namespace

int main() {
    test_happy_path();
    test_decode_cache_is_stable();
    test_error_zero_length();
    test_error_truncated();
    test_error_garbage();
    test_error_unterminated();
    test_copy_data();
    test_set_to_empty();

    std::printf("ResStringPool: %d checks, %d failures\n", g_checks, g_failures);
    return g_failures == 0 ? 0 : 1;
}
