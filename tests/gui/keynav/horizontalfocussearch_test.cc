/*********************************************************************************
 * Port of AOSP coretests android.widget.focus.HorizontalFocusSearchTest, with the
 * HorizontalFocusSearch companion activity rebuilt programmatically instead of
 * setContentView. Source of truth:
 * frameworks/base/core/tests/coretests/src/android/widget/focus/HorizontalFocusSearch{,Test}.java
 *********************************************************************************/
#include <gtest/gtest.h>
#include <cdroid.h>
#include <view/focusfinder.h>
#include <guienvironment.h>
#include "keysender.h"

using namespace cdroid;

/* HorizontalFocusSearch: buttons of various heights and vertical placements in
   a horizontal layout, to exercise core focus searching. */
class HorizontalFocusSearchTest : public testing::Test {
protected:
    LinearLayout* mLayout;
    Button* mLeftTall;
    Button* mMidShort1Top;
    Button* mMidShort2Bottom;
    Button* mRightTall;

    void SetUp() override {
        App& app = App::getInstance();

        mLayout = new LinearLayout(&app);
        mLayout->setOrientation(LinearLayout::HORIZONTAL);
        mLayout->setLayoutParams(new ViewGroup::LayoutParams(
                ViewGroup::LayoutParams::MATCH_PARENT,
                ViewGroup::LayoutParams::MATCH_PARENT));

        mLeftTall = makeTall("left tall");
        mLayout->addView(mLeftTall);

        mMidShort1Top = addShort("mid(1) top", false);
        mMidShort2Bottom = addShort("mid(2) bottom", true);

        mRightTall = makeTall("right tall");
        mLayout->addView(mRightTall);

        /* setContentView + first traversal; View::layout takes (l, t, w, h). */
        ViewGroup* content = GUIEnvironment::content();
        const int w = content->getWidth() > 0 ? content->getWidth() : 1080;
        const int h = content->getHeight() > 0 ? content->getHeight() : 1920;
        content->addView(mLayout);
        mLayout->measure(MeasureSpec::makeMeasureSpec(w, MeasureSpec::EXACTLY),
                         MeasureSpec::makeMeasureSpec(h, MeasureSpec::EXACTLY));
        mLayout->layout(0, 0, w, h);
        pumpUntilIdle();
    }

    Button* makeTall(const std::string& label) {
        App& app = App::getInstance();
        Button* button = new Button(&app);
        button->setText(label);
        button->setLayoutParams(new LinearLayout::LayoutParams(
                ViewGroup::LayoutParams::WRAP_CONTENT,
                ViewGroup::LayoutParams::MATCH_PARENT));
        return button;
    }

    /* A short button taking just under half the column vertically (weight
       490 vs the filler's 510). */
    Button* addShort(const std::string& label, bool atBottom) {
        App& app = App::getInstance();
        Button* button = new Button(&app);
        button->setText(label);
        button->setLayoutParams(new LinearLayout::LayoutParams(
                ViewGroup::LayoutParams::WRAP_CONTENT,
                0 /* height */, 490));

        TextView* filler = new TextView(&app);
        filler->setText("filler");
        filler->setLayoutParams(new LinearLayout::LayoutParams(
                ViewGroup::LayoutParams::WRAP_CONTENT,
                0 /* height */, 510));

        LinearLayout* ll = new LinearLayout(&app);
        ll->setOrientation(LinearLayout::VERTICAL);
        ll->setLayoutParams(new LinearLayout::LayoutParams(
                ViewGroup::LayoutParams::WRAP_CONTENT,
                ViewGroup::LayoutParams::MATCH_PARENT));

        if (atBottom) {
            ll->addView(filler);
            ll->addView(button);
            mLayout->addView(ll);
        } else {
            ll->addView(button);
            ll->addView(filler);
            mLayout->addView(ll);
        }
        return button;
    }

    View* findNextFocus(View* focused, int direction) {
        return FocusFinder::getInstance().findNextFocus(mLayout, focused, direction);
    }
};

TEST_F(HorizontalFocusSearchTest, testPreconditions) {
    ASSERT_NE(nullptr, mLayout);
    ASSERT_NE(nullptr, mLeftTall);
    ASSERT_NE(nullptr, mMidShort1Top);
    ASSERT_NE(nullptr, mMidShort2Bottom);
    ASSERT_NE(nullptr, mRightTall);
}

TEST_F(HorizontalFocusSearchTest, testSearchFromLeftButton) {
    EXPECT_EQ(nullptr, findNextFocus(mLeftTall, View::FOCUS_UP));
    EXPECT_EQ(nullptr, findNextFocus(mLeftTall, View::FOCUS_DOWN));
    EXPECT_EQ(nullptr, findNextFocus(mLeftTall, View::FOCUS_LEFT));
    EXPECT_EQ(mMidShort1Top, findNextFocus(mLeftTall, View::FOCUS_RIGHT));
}

/* TODO_ prefix in AOSP = disabled upstream; DISABLED_ matches gtest. */
TEST_F(HorizontalFocusSearchTest, DISABLED_TODO_testSearchFromMiddleLeftButton) {
    EXPECT_EQ(nullptr, findNextFocus(mMidShort1Top, View::FOCUS_UP));
    EXPECT_EQ(mMidShort2Bottom, findNextFocus(mMidShort1Top, View::FOCUS_DOWN));
    EXPECT_EQ(mLeftTall, findNextFocus(mMidShort1Top, View::FOCUS_LEFT));
    EXPECT_EQ(mMidShort2Bottom, findNextFocus(mMidShort1Top, View::FOCUS_RIGHT));
}

/* TODO_ prefix in AOSP = disabled upstream; DISABLED_ matches gtest. */
TEST_F(HorizontalFocusSearchTest, DISABLED_TODO_testSearchFromMiddleRightButton) {
    EXPECT_EQ(mMidShort1Top, findNextFocus(mMidShort2Bottom, View::FOCUS_UP));
    EXPECT_EQ(nullptr, findNextFocus(mMidShort2Bottom, View::FOCUS_DOWN));
    EXPECT_EQ(mMidShort1Top, findNextFocus(mMidShort2Bottom, View::FOCUS_LEFT));
    EXPECT_EQ(mRightTall, findNextFocus(mMidShort2Bottom, View::FOCUS_RIGHT));
}

TEST_F(HorizontalFocusSearchTest, testSearchFromRightButton) {
    EXPECT_EQ(nullptr, findNextFocus(mRightTall, View::FOCUS_UP));
    EXPECT_EQ(nullptr, findNextFocus(mRightTall, View::FOCUS_DOWN));
    EXPECT_EQ(mMidShort2Bottom, findNextFocus(mRightTall, View::FOCUS_LEFT));
    EXPECT_EQ(nullptr, findNextFocus(mRightTall, View::FOCUS_RIGHT));
}
