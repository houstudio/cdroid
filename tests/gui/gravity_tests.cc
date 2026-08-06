// Ported from AOSP CTS GravityTest.java (android.view.Gravity).
//
// CDROID Rect adaptation: CTS uses android.graphics.Rect {left,top,right,bottom}; CDROID Rect is
// {left,top,width,height} with right()==left+width, bottom()==top+height. So every CTS
// `new Rect(l,t,r,b)` becomes `Rect(l, t, r-l, b-t)` (same region), and every assertion on
// `.right`/`.bottom` becomes `.right()`/`.bottom()`. LayoutDirection LTR/RTL come from
// gravity.h's enum (View.LAYOUT_DIRECTION_* map to LayoutDirection::LTR/RTL = 0/1).
//
// Original: cts/tests/tests/view/src/android/view/cts/GravityTest.java (Apache 2.0)
#include <gtest/gtest.h>
#include <view/gravity.h>
#include <core/rect.h>

using namespace cdroid;

class CtsGravityTest : public testing::Test {
protected:
    Rect mInRect;
    Rect mOutRect;
    void SetUp() override {
        mInRect = Rect{1, 2, 2, 2};   // CTS Rect(1,2,3,4)
        mOutRect = Rect{};
    }
    void applyGravity(int gravity, int w, int h, bool bRtl) {
        const int layoutDirection = bRtl ? LayoutDirection::RTL : LayoutDirection::LTR;
        Gravity::apply(gravity, w, h, mInRect, mOutRect, layoutDirection);
    }
};

TEST_F(CtsGravityTest, testConstructor) {
    Gravity g;  // CTS: new Gravity()
    (void)g;
}

TEST_F(CtsGravityTest, testApply) {
    mInRect = Rect{10, 20, 20, 20};  // CTS Rect(10,20,30,40)

    Gravity::apply(Gravity::TOP, 2, 3, mInRect, mOutRect);
    EXPECT_EQ(19, mOutRect.left);
    EXPECT_EQ(21, mOutRect.right());
    EXPECT_EQ(20, mOutRect.top);
    EXPECT_EQ(23, mOutRect.bottom());
    Gravity::apply(Gravity::TOP, 2, 3, mInRect, 5, 5, mOutRect);
    EXPECT_EQ(24, mOutRect.left);
    EXPECT_EQ(26, mOutRect.right());
    EXPECT_EQ(25, mOutRect.top);
    EXPECT_EQ(28, mOutRect.bottom());
    applyGravity(Gravity::TOP, 2, 3, false);
    EXPECT_EQ(19, mOutRect.left);
    EXPECT_EQ(21, mOutRect.right());
    EXPECT_EQ(20, mOutRect.top);
    EXPECT_EQ(23, mOutRect.bottom());
    applyGravity(Gravity::TOP, 2, 3, true);
    EXPECT_EQ(19, mOutRect.left);
    EXPECT_EQ(21, mOutRect.right());
    EXPECT_EQ(20, mOutRect.top);
    EXPECT_EQ(23, mOutRect.bottom());

    Gravity::apply(Gravity::BOTTOM, 2, 3, mInRect, mOutRect);
    EXPECT_EQ(19, mOutRect.left);
    EXPECT_EQ(21, mOutRect.right());
    EXPECT_EQ(37, mOutRect.top);
    EXPECT_EQ(40, mOutRect.bottom());
    Gravity::apply(Gravity::BOTTOM, 2, 3, mInRect, 5, 5, mOutRect);
    EXPECT_EQ(24, mOutRect.left);
    EXPECT_EQ(26, mOutRect.right());
    EXPECT_EQ(32, mOutRect.top);
    EXPECT_EQ(35, mOutRect.bottom());
    applyGravity(Gravity::BOTTOM, 2, 3, false);
    EXPECT_EQ(19, mOutRect.left);
    EXPECT_EQ(21, mOutRect.right());
    EXPECT_EQ(37, mOutRect.top);
    EXPECT_EQ(40, mOutRect.bottom());
    applyGravity(Gravity::BOTTOM, 2, 3, true);
    EXPECT_EQ(19, mOutRect.left);
    EXPECT_EQ(21, mOutRect.right());
    EXPECT_EQ(37, mOutRect.top);
    EXPECT_EQ(40, mOutRect.bottom());

    Gravity::apply(Gravity::LEFT, 2, 10, mInRect, mOutRect);
    EXPECT_EQ(10, mOutRect.left);
    EXPECT_EQ(12, mOutRect.right());
    EXPECT_EQ(25, mOutRect.top);
    EXPECT_EQ(35, mOutRect.bottom());
    Gravity::apply(Gravity::LEFT, 2, 10, mInRect, 5, 5, mOutRect);
    EXPECT_EQ(15, mOutRect.left);
    EXPECT_EQ(17, mOutRect.right());
    EXPECT_EQ(30, mOutRect.top);
    EXPECT_EQ(40, mOutRect.bottom());
    applyGravity(Gravity::LEFT, 2, 10, false);
    EXPECT_EQ(10, mOutRect.left);
    EXPECT_EQ(12, mOutRect.right());
    EXPECT_EQ(25, mOutRect.top);
    EXPECT_EQ(35, mOutRect.bottom());
    applyGravity(Gravity::LEFT, 2, 10, true);
    EXPECT_EQ(10, mOutRect.left);
    EXPECT_EQ(12, mOutRect.right());
    EXPECT_EQ(25, mOutRect.top);
    EXPECT_EQ(35, mOutRect.bottom());

    Gravity::apply(Gravity::START, 2, 10, mInRect, mOutRect);
    EXPECT_EQ(10, mOutRect.left);
    EXPECT_EQ(12, mOutRect.right());
    EXPECT_EQ(25, mOutRect.top);
    EXPECT_EQ(35, mOutRect.bottom());
    Gravity::apply(Gravity::START, 2, 10, mInRect, 5, 5, mOutRect);
    EXPECT_EQ(15, mOutRect.left);
    EXPECT_EQ(17, mOutRect.right());
    EXPECT_EQ(30, mOutRect.top);
    EXPECT_EQ(40, mOutRect.bottom());
    applyGravity(Gravity::START, 2, 10, false);
    EXPECT_EQ(10, mOutRect.left);
    EXPECT_EQ(12, mOutRect.right());
    EXPECT_EQ(25, mOutRect.top);
    EXPECT_EQ(35, mOutRect.bottom());
    applyGravity(Gravity::START, 2, 10, true);
    EXPECT_EQ(28, mOutRect.left);
    EXPECT_EQ(30, mOutRect.right());
    EXPECT_EQ(25, mOutRect.top);
    EXPECT_EQ(35, mOutRect.bottom());

    Gravity::apply(Gravity::RIGHT, 2, 10, mInRect, mOutRect);
    EXPECT_EQ(28, mOutRect.left);
    EXPECT_EQ(30, mOutRect.right());
    EXPECT_EQ(25, mOutRect.top);
    EXPECT_EQ(35, mOutRect.bottom());
    Gravity::apply(Gravity::RIGHT, 2, 10, mInRect, 5, 5, mOutRect);
    EXPECT_EQ(23, mOutRect.left);
    EXPECT_EQ(25, mOutRect.right());
    EXPECT_EQ(30, mOutRect.top);
    EXPECT_EQ(40, mOutRect.bottom());
    applyGravity(Gravity::RIGHT, 2, 10, false);
    EXPECT_EQ(28, mOutRect.left);
    EXPECT_EQ(30, mOutRect.right());
    EXPECT_EQ(25, mOutRect.top);
    EXPECT_EQ(35, mOutRect.bottom());
    applyGravity(Gravity::RIGHT, 2, 10, true);
    EXPECT_EQ(28, mOutRect.left);
    EXPECT_EQ(30, mOutRect.right());
    EXPECT_EQ(25, mOutRect.top);
    EXPECT_EQ(35, mOutRect.bottom());

    Gravity::apply(Gravity::END, 2, 10, mInRect, mOutRect);
    EXPECT_EQ(28, mOutRect.left);
    EXPECT_EQ(30, mOutRect.right());
    EXPECT_EQ(25, mOutRect.top);
    EXPECT_EQ(35, mOutRect.bottom());
    Gravity::apply(Gravity::END, 2, 10, mInRect, 5, 5, mOutRect);
    EXPECT_EQ(23, mOutRect.left);
    EXPECT_EQ(25, mOutRect.right());
    EXPECT_EQ(30, mOutRect.top);
    EXPECT_EQ(40, mOutRect.bottom());
    applyGravity(Gravity::END, 2, 10, false);
    EXPECT_EQ(28, mOutRect.left);
    EXPECT_EQ(30, mOutRect.right());
    EXPECT_EQ(25, mOutRect.top);
    EXPECT_EQ(35, mOutRect.bottom());
    applyGravity(Gravity::END, 2, 10, true);
    EXPECT_EQ(10, mOutRect.left);
    EXPECT_EQ(12, mOutRect.right());
    EXPECT_EQ(25, mOutRect.top);
    EXPECT_EQ(35, mOutRect.bottom());

    Gravity::apply(Gravity::CENTER_VERTICAL, 2, 10, mInRect, mOutRect);
    EXPECT_EQ(19, mOutRect.left);
    EXPECT_EQ(21, mOutRect.right());
    EXPECT_EQ(25, mOutRect.top);
    EXPECT_EQ(35, mOutRect.bottom());
    Gravity::apply(Gravity::CENTER_VERTICAL, 2, 10, mInRect, 5, 5, mOutRect);
    EXPECT_EQ(24, mOutRect.left);
    EXPECT_EQ(26, mOutRect.right());
    EXPECT_EQ(30, mOutRect.top);
    EXPECT_EQ(40, mOutRect.bottom());
    applyGravity(Gravity::CENTER_VERTICAL, 2, 10, false);
    EXPECT_EQ(19, mOutRect.left);
    EXPECT_EQ(21, mOutRect.right());
    EXPECT_EQ(25, mOutRect.top);
    EXPECT_EQ(35, mOutRect.bottom());
    applyGravity(Gravity::CENTER_VERTICAL, 2, 10, true);
    EXPECT_EQ(19, mOutRect.left);
    EXPECT_EQ(21, mOutRect.right());
    EXPECT_EQ(25, mOutRect.top);
    EXPECT_EQ(35, mOutRect.bottom());

    Gravity::apply(Gravity::FILL_VERTICAL, 2, 10, mInRect, mOutRect);
    EXPECT_EQ(19, mOutRect.left);
    EXPECT_EQ(21, mOutRect.right());
    EXPECT_EQ(20, mOutRect.top);
    EXPECT_EQ(40, mOutRect.bottom());
    Gravity::apply(Gravity::FILL_VERTICAL, 2, 10, mInRect, 5, 5, mOutRect);
    EXPECT_EQ(24, mOutRect.left);
    EXPECT_EQ(26, mOutRect.right());
    EXPECT_EQ(25, mOutRect.top);
    EXPECT_EQ(45, mOutRect.bottom());
    applyGravity(Gravity::FILL_VERTICAL, 2, 10, false);
    EXPECT_EQ(19, mOutRect.left);
    EXPECT_EQ(21, mOutRect.right());
    EXPECT_EQ(20, mOutRect.top);
    EXPECT_EQ(40, mOutRect.bottom());
    applyGravity(Gravity::FILL_VERTICAL, 2, 10, true);
    EXPECT_EQ(19, mOutRect.left);
    EXPECT_EQ(21, mOutRect.right());
    EXPECT_EQ(20, mOutRect.top);
    EXPECT_EQ(40, mOutRect.bottom());

    Gravity::apply(Gravity::CENTER_HORIZONTAL, 2, 10, mInRect, mOutRect);
    EXPECT_EQ(19, mOutRect.left);
    EXPECT_EQ(21, mOutRect.right());
    EXPECT_EQ(25, mOutRect.top);
    EXPECT_EQ(35, mOutRect.bottom());
    Gravity::apply(Gravity::CENTER_HORIZONTAL, 2, 10, mInRect, 5, 5, mOutRect);
    EXPECT_EQ(24, mOutRect.left);
    EXPECT_EQ(26, mOutRect.right());
    EXPECT_EQ(30, mOutRect.top);
    EXPECT_EQ(40, mOutRect.bottom());
    applyGravity(Gravity::CENTER_HORIZONTAL, 2, 10, false);
    EXPECT_EQ(19, mOutRect.left);
    EXPECT_EQ(21, mOutRect.right());
    EXPECT_EQ(25, mOutRect.top);
    EXPECT_EQ(35, mOutRect.bottom());
    applyGravity(Gravity::CENTER_HORIZONTAL, 2, 10, true);
    EXPECT_EQ(19, mOutRect.left);
    EXPECT_EQ(21, mOutRect.right());
    EXPECT_EQ(25, mOutRect.top);
    EXPECT_EQ(35, mOutRect.bottom());

    Gravity::apply(Gravity::FILL_HORIZONTAL, 2, 10, mInRect, mOutRect);
    EXPECT_EQ(10, mOutRect.left);
    EXPECT_EQ(30, mOutRect.right());
    EXPECT_EQ(25, mOutRect.top);
    EXPECT_EQ(35, mOutRect.bottom());
    Gravity::apply(Gravity::FILL_HORIZONTAL, 2, 10, mInRect, 5, 5, mOutRect);
    EXPECT_EQ(15, mOutRect.left);
    EXPECT_EQ(35, mOutRect.right());
    EXPECT_EQ(30, mOutRect.top);
    EXPECT_EQ(40, mOutRect.bottom());
    applyGravity(Gravity::FILL_HORIZONTAL, 2, 10, false);
    EXPECT_EQ(10, mOutRect.left);
    EXPECT_EQ(30, mOutRect.right());
    EXPECT_EQ(25, mOutRect.top);
    EXPECT_EQ(35, mOutRect.bottom());
    applyGravity(Gravity::FILL_HORIZONTAL, 2, 10, true);
    EXPECT_EQ(10, mOutRect.left);
    EXPECT_EQ(30, mOutRect.right());
    EXPECT_EQ(25, mOutRect.top);
    EXPECT_EQ(35, mOutRect.bottom());
}

TEST_F(CtsGravityTest, testIsVertical) {
    EXPECT_FALSE(Gravity::isVertical(-1));
    EXPECT_TRUE(Gravity::isVertical(Gravity::VERTICAL_GRAVITY_MASK));
    EXPECT_FALSE(Gravity::isVertical(Gravity::NO_GRAVITY));
}

TEST_F(CtsGravityTest, testIsHorizontal) {
    EXPECT_FALSE(Gravity::isHorizontal(-1));
    EXPECT_TRUE(Gravity::isHorizontal(Gravity::HORIZONTAL_GRAVITY_MASK));
    EXPECT_TRUE(Gravity::isHorizontal(Gravity::RELATIVE_HORIZONTAL_GRAVITY_MASK));
    EXPECT_FALSE(Gravity::isHorizontal(Gravity::NO_GRAVITY));
}

TEST_F(CtsGravityTest, testApplyDisplay) {
    Rect display = Rect{20, 30, 20, 20};     // CTS Rect(20,30,40,50)
    Rect inoutRect = Rect{10, 10, 20, 50};   // CTS Rect(10,10,30,60)
    Gravity::applyDisplay(Gravity::DISPLAY_CLIP_VERTICAL, display, inoutRect);
    EXPECT_EQ(20, inoutRect.left);
    EXPECT_EQ(40, inoutRect.right());
    EXPECT_EQ(30, inoutRect.top);
    EXPECT_EQ(50, inoutRect.bottom());

    display = Rect{20, 30, 20, 20};
    inoutRect = Rect{10, 10, 20, 50};
    Gravity::applyDisplay(Gravity::DISPLAY_CLIP_HORIZONTAL, display, inoutRect);
    EXPECT_EQ(20, inoutRect.left);
    EXPECT_EQ(30, inoutRect.right());
    EXPECT_EQ(30, inoutRect.top);
    EXPECT_EQ(50, inoutRect.bottom());
}

TEST_F(CtsGravityTest, testGetAbsoluteGravity) {
    auto verify = [](int expected, int initial, bool isRtl) {
        const int layoutDirection = isRtl ? LayoutDirection::RTL : LayoutDirection::LTR;
        EXPECT_EQ(expected, Gravity::getAbsoluteGravity(initial, layoutDirection));
    };

    verify(Gravity::LEFT, Gravity::LEFT, false);
    verify(Gravity::LEFT, Gravity::LEFT, true);
    verify(Gravity::RIGHT, Gravity::RIGHT, false);
    verify(Gravity::RIGHT, Gravity::RIGHT, true);
    verify(Gravity::TOP, Gravity::TOP, false);
    verify(Gravity::TOP, Gravity::TOP, true);
    verify(Gravity::BOTTOM, Gravity::BOTTOM, false);
    verify(Gravity::BOTTOM, Gravity::BOTTOM, true);
    verify(Gravity::CENTER_VERTICAL, Gravity::CENTER_VERTICAL, false);
    verify(Gravity::CENTER_VERTICAL, Gravity::CENTER_VERTICAL, true);
    verify(Gravity::CENTER_HORIZONTAL, Gravity::CENTER_HORIZONTAL, false);
    verify(Gravity::CENTER_HORIZONTAL, Gravity::CENTER_HORIZONTAL, true);
    verify(Gravity::CENTER, Gravity::CENTER, false);
    verify(Gravity::CENTER, Gravity::CENTER, true);
    verify(Gravity::FILL_VERTICAL, Gravity::FILL_VERTICAL, false);
    verify(Gravity::FILL_VERTICAL, Gravity::FILL_VERTICAL, true);
    verify(Gravity::FILL_HORIZONTAL, Gravity::FILL_HORIZONTAL, false);
    verify(Gravity::FILL_HORIZONTAL, Gravity::FILL_HORIZONTAL, true);
    verify(Gravity::FILL, Gravity::FILL, false);
    verify(Gravity::FILL, Gravity::FILL, true);
    verify(Gravity::CLIP_HORIZONTAL, Gravity::CLIP_HORIZONTAL, false);
    verify(Gravity::CLIP_HORIZONTAL, Gravity::CLIP_HORIZONTAL, true);
    verify(Gravity::CLIP_VERTICAL, Gravity::CLIP_VERTICAL, false);
    verify(Gravity::CLIP_VERTICAL, Gravity::CLIP_VERTICAL, true);
    verify(Gravity::LEFT, Gravity::START, false);
    verify(Gravity::RIGHT, Gravity::START, true);
    verify(Gravity::RIGHT, Gravity::END, false);
    verify(Gravity::LEFT, Gravity::END, true);
}
