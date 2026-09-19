/*********************************************************************************
 * Ported from AOSP coretests android.text.StaticLayoutBidiTest (Apache 2.0).
 * Quick check of native bidi implementation.
 *********************************************************************************/
#include <text/androidbidi.h>
#include <text/layout.h>
#include <gtest/gtest.h>
#include <string>
#include <vector>

using namespace cdroid;

namespace {

// Layout direction request/result constants (Layout).
constexpr int REQ_DL = 2;  // Layout::DIR_REQUEST_DEFAULT_LTR;
constexpr int REQ_DR = -2; // Layout::DIR_REQUEST_DEFAULT_RTL;
constexpr int REQ_L = 1;   // Layout::DIR_REQUEST_LTR;
constexpr int REQ_R = -1;  // Layout::DIR_REQUEST_RTL;
constexpr int L = Layout::DIR_LEFT_TO_RIGHT;
constexpr int R = Layout::DIR_RIGHT_TO_LEFT;

const std::u16string SP = u" ";
// Hebrew letters ALEF..DALET (U+05D0..U+05D3).
const std::u16string ALEF = { char16_t(0x05D0) };
const std::u16string BET = { char16_t(0x05D1) };
const std::u16string GIMEL = { char16_t(0x05D2) };
const std::u16string DALET = { char16_t(0x05D3) };

void expectNativeBidi(int dir, const std::u16string& text,
        const std::string& expectedLevels, int expectedDir) {
    const size_t n = text.size();
    std::vector<char16_t> chs(text.begin(), text.end());
    std::vector<uint8_t> chInfo(n);

    const int resultDir = AndroidBidi::bidi(dir, chs, chInfo);

    std::string resultLevels;
    for (size_t i = 0; i < n; ++i) {
        resultLevels.push_back((char)('0' + chInfo[i]));
    }
    ASSERT_EQ((size_t) n, resultLevels.size());
    EXPECT_EQ(expectedDir, resultDir) << "direction";
    EXPECT_EQ(expectedLevels, resultLevels) << "levels";
}

TEST(StaticLayoutBidiTest, testAllLtr) {
    expectNativeBidi(REQ_DL, u"a test", "000000", L);
}

TEST(StaticLayoutBidiTest, testLtrRtl) {
    expectNativeBidi(REQ_DL, u"abc " + ALEF + BET + GIMEL, "0000111", L);
}

TEST(StaticLayoutBidiTest, testAllRtl) {
    expectNativeBidi(REQ_DL, ALEF + SP + ALEF + BET + GIMEL + DALET, "111111", R);
}

TEST(StaticLayoutBidiTest, testRtlLtr) {
    expectNativeBidi(REQ_DL, ALEF + BET + GIMEL + u" abc", "1111222", R);
}

TEST(StaticLayoutBidiTest, testRAllLtr) {
    expectNativeBidi(REQ_R, u"a test", "222222", R);
}

TEST(StaticLayoutBidiTest, testRLtrRtl) {
    expectNativeBidi(REQ_R, u"abc " + ALEF + BET + GIMEL, "2221111", R);
}

TEST(StaticLayoutBidiTest, testLAllRtl) {
    expectNativeBidi(REQ_L, ALEF + SP + ALEF + BET + GIMEL + DALET, "111111", L);
}

TEST(StaticLayoutBidiTest, testLRtlLtr) {
    expectNativeBidi(REQ_DL, ALEF + BET + GIMEL + u" abc", "1111222", R);
}

TEST(StaticLayoutBidiTest, testNativeBidi) {
    expectNativeBidi(REQ_L, ALEF + BET + GIMEL + u" abc", "1110000", L);
}

} // namespace
