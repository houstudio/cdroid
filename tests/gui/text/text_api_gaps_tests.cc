// AOSP coretests android.text families whose production APIs are not ported
// to cdroid yet. Each AOSP test is kept as a GTEST_SKIP so the gap stays
// visible in the suite output instead of silently vanishing.
//
//  - AndroidCharacterTest (2 tests): AndroidCharacter.getDirectionalities
//    (native table walk) is not ported; Character.getDirectionality IS.
//  - BidiFormatterTest (testMarkAfter/testMarkBefore): BidiFormatter
//    (unicodeWrap/mark-based isolation) is not ported.
//  - EmojiTest (3 tests): android.text.Emoji is not ported.
//  - VariationParserTest (2 tests): font VariationParser is not ported.
//
// Originals: frameworks/base/core/tests/coretests/src/android/text/
//   {AndroidCharacter,BidiFormatter,Emoji,VariationParser}Test.java (Apache 2.0)
#include <gtest/gtest.h>

TEST(AndroidCharacterTest, testGetDirectionalities_nonSupplementaryCharacters) {
    GTEST_SKIP() << "AndroidCharacter.getDirectionalities not ported";
}

TEST(AndroidCharacterTest, testGetDirectionalities_supplementaryCharacters) {
    GTEST_SKIP() << "AndroidCharacter.getDirectionalities not ported";
}

TEST(BidiFormatterTest, testMarkAfter) {
    GTEST_SKIP() << "BidiFormatter not ported";
}

TEST(BidiFormatterTest, testMarkBefore) {
    GTEST_SKIP() << "BidiFormatter not ported";
}

TEST(EmojiTest, testIsNewEmoji_Emoji) {
    GTEST_SKIP() << "android.text.Emoji not ported";
}

TEST(EmojiTest, testisEmojiModifierBase_LegacyCompat) {
    GTEST_SKIP() << "android.text.Emoji not ported";
}

TEST(EmojiTest, testisEmojiModifierBase) {
    GTEST_SKIP() << "android.text.Emoji not ported";
}

TEST(VariationParserTest, testFromFontVariationSetting_InvalidStyleValue) {
    GTEST_SKIP() << "font VariationParser not ported";
}

TEST(VariationParserTest, testOpenTypeTagValue) {
    GTEST_SKIP() << "font VariationParser not ported";
}
