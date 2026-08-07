// CDROID-specific BadgeDrawable get/set coverage. NOT an AOSP CTS port — BadgeDrawable is an
// androidx Material drawable (com.google.android.material.badge), not a framework class, so there
// is no AOSP CTS test for it. These cases exercise the round-trip of the observable properties via
// the public API, constructed through BadgeDrawable::create(Context*) (default state). No AOSP
// reference anchor — pure round-trip coverage.
#include <gtest/gtest.h>
#include <drawable/badgedrawable.h>
#include <core/app.h>
#include <memory>
#include <string>

using namespace cdroid;

class BadgeDrawableTest : public testing::Test {
protected:
    std::unique_ptr<BadgeDrawable> badge;
    void SetUp() override {
        badge.reset(BadgeDrawable::create(&App::getInstance()));
        ASSERT_NE(nullptr, badge);
    }
};

TEST_F(BadgeDrawableTest, testNumber) {
    badge->setNumber(42);
    EXPECT_EQ(42, badge->getNumber());
    EXPECT_TRUE(badge->hasNumber());
    badge->clearNumber();
    EXPECT_FALSE(badge->hasNumber());
}

TEST_F(BadgeDrawableTest, testMaxCharacterCount) {
    badge->setMaxCharacterCount(3);
    EXPECT_EQ(3, badge->getMaxCharacterCount());
}

TEST_F(BadgeDrawableTest, testMaxNumber) {
    badge->setMaxNumber(999);
    EXPECT_EQ(999, badge->getMaxNumber());
}

TEST_F(BadgeDrawableTest, testColors) {
    badge->setBackgroundColor(0xFF112233);
    EXPECT_EQ(0xFF112233, badge->getBackgroundColor());
    badge->setBadgeTextColor(0xFF445566);
    EXPECT_EQ(0xFF445566, badge->getBadgeTextColor());
}

TEST_F(BadgeDrawableTest, testBadgeGravity) {
    badge->setBadgeGravity(BadgeDrawable::BOTTOM_END);
    EXPECT_EQ((int)BadgeDrawable::BOTTOM_END, badge->getBadgeGravity());
}

TEST_F(BadgeDrawableTest, testAlpha) {
    badge->setAlpha(128);
    EXPECT_EQ(128, badge->getAlpha());
}

TEST_F(BadgeDrawableTest, testOffsets) {
    badge->setHorizontalOffset(8);
    EXPECT_EQ(8, badge->getHorizontalOffset());
    badge->setVerticalOffset(12);
    EXPECT_EQ(12, badge->getVerticalOffset());
}

TEST_F(BadgeDrawableTest, testText) {
    badge->setText("new");
    EXPECT_EQ(std::string("new"), badge->getText());
    EXPECT_TRUE(badge->hasText());
    badge->clearText();
    EXPECT_FALSE(badge->hasText());
}
