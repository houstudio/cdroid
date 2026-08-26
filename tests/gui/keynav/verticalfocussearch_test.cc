/*********************************************************************************
 * Port of AOSP coretests android.widget.focus.VerticalFocusSearchTest, with the
 * VerticalFocusSearch companion activity rebuilt programmatically instead of
 * setContentView. Source of truth:
 * frameworks/base/core/tests/coretests/src/android/widget/focus/VerticalFocusSearch{,Test}.java
 *********************************************************************************/
#include <gtest/gtest.h>
#include <cdroid.h>
#include <view/focusfinder.h>
#include <guienvironment.h>
#include "keysender.h"

using namespace cdroid;

/* VerticalFocusSearch: a few buttons of various widths and horizontal
   placements in a vertical layout, to exercise core focus searching. */
class VerticalFocusSearchTest : public testing::Test {
protected:
    LinearLayout* mLayout;
    Button* mTopWide;
    Button* mMidSkinny1Left;
    Button* mMidSkinny2Right;
    Button* mBottomWide;

    void SetUp() override {
        App& app = App::getInstance();

        mLayout = new LinearLayout(&app);
        mLayout->setOrientation(LinearLayout::VERTICAL);
        mLayout->setHorizontalGravity(Gravity::START);
        mLayout->setLayoutParams(new ViewGroup::LayoutParams(
                ViewGroup::LayoutParams::MATCH_PARENT,
                ViewGroup::LayoutParams::MATCH_PARENT));

        mTopWide = makeWide("top wide");
        mLayout->addView(mTopWide);

        mMidSkinny1Left = addSkinny("mid skinny 1(L)", false);
        mMidSkinny2Right = addSkinny("mid skinny 2(R)", true);

        mBottomWide = makeWide("bottom wide");
        mLayout->addView(mBottomWide);

        /* setContentView: attach to the harness content pane, then run the
           first traversal explicitly (AOSP gets this from ViewRootImpl with
           the window size) so the 480/520 weights resolve into real
           positions. Note: View::layout takes (l, t, w, h) in CDROID. */
        ViewGroup* content = GUIEnvironment::content();
        const int w = content->getWidth() > 0 ? content->getWidth() : 1080;
        const int h = content->getHeight() > 0 ? content->getHeight() : 1920;
        content->addView(mLayout);
        mLayout->measure(MeasureSpec::makeMeasureSpec(w, MeasureSpec::EXACTLY),
                         MeasureSpec::makeMeasureSpec(h, MeasureSpec::EXACTLY));
        mLayout->layout(0, 0, w, h);
        pumpUntilIdle();
    }

    Button* makeWide(const std::string& label) {
        App& app = App::getInstance();
        Button* button = new Button(&app);
        button->setText(label);
        button->setLayoutParams(new LinearLayout::LayoutParams(
                ViewGroup::LayoutParams::MATCH_PARENT,
                ViewGroup::LayoutParams::WRAP_CONTENT));
        return button;
    }

    /* A skinny button taking just under half the row horizontally (weight
       480 vs the filler's 520). */
    Button* addSkinny(const std::string& label, bool atRight) {
        App& app = App::getInstance();
        Button* button = new Button(&app);
        button->setText(label);
        button->setLayoutParams(new LinearLayout::LayoutParams(
                0 /* width */, ViewGroup::LayoutParams::WRAP_CONTENT, 480));

        TextView* filler = new TextView(&app);
        filler->setText("filler");
        filler->setLayoutParams(new LinearLayout::LayoutParams(
                0 /* width */, ViewGroup::LayoutParams::WRAP_CONTENT, 520));

        LinearLayout* ll = new LinearLayout(&app);
        ll->setOrientation(LinearLayout::HORIZONTAL);
        ll->setLayoutParams(new LinearLayout::LayoutParams(
                ViewGroup::LayoutParams::MATCH_PARENT,
                ViewGroup::LayoutParams::WRAP_CONTENT));

        if (atRight) {
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

    /* The AOSP FocusSearchAlg indirection (old-vs-new impl switch) collapses to
       the new impl: FocusFinder.findNextFocus. */
    View* findNextFocus(View* focused, int direction) {
        return FocusFinder::getInstance().findNextFocus(mLayout, focused, direction);
    }
};

TEST_F(VerticalFocusSearchTest, testPreconditions) {
    ASSERT_NE(nullptr, mLayout);
    ASSERT_NE(nullptr, mTopWide);
    ASSERT_NE(nullptr, mMidSkinny1Left);
    ASSERT_NE(nullptr, mMidSkinny2Right);
    ASSERT_NE(nullptr, mBottomWide);
}

TEST_F(VerticalFocusSearchTest, testSearchFromTopButton) {
    EXPECT_EQ(nullptr, findNextFocus(mTopWide, View::FOCUS_UP));
    EXPECT_EQ(nullptr, findNextFocus(mTopWide, View::FOCUS_LEFT));
    EXPECT_EQ(nullptr, findNextFocus(mTopWide, View::FOCUS_RIGHT));
    EXPECT_EQ(mMidSkinny1Left, findNextFocus(mTopWide, View::FOCUS_DOWN));
}

/* NOTE: AOSP marks this class @Suppress ("until bug 1416545 is fixed"); the
   mid-row assertions below describe the desired, not current, algorithm
   behavior — the faithful FocusFinder picks mBottomWide for DOWN from
   mMidSkinny1Left (weighted distance favors the in-beam full-width row over
   the nearer out-of-beam skinny one). Kept as ported. */
TEST_F(VerticalFocusSearchTest, testSearchFromMidLeft) {
    EXPECT_EQ(nullptr, findNextFocus(mMidSkinny1Left, View::FOCUS_LEFT));
    EXPECT_EQ(mMidSkinny2Right, findNextFocus(mMidSkinny1Left, View::FOCUS_RIGHT));
    EXPECT_EQ(mTopWide, findNextFocus(mMidSkinny1Left, View::FOCUS_UP));
    EXPECT_EQ(mMidSkinny2Right, findNextFocus(mMidSkinny1Left, View::FOCUS_DOWN));
}

TEST_F(VerticalFocusSearchTest, testSearchFromMidRight) {
    EXPECT_EQ(mMidSkinny1Left, findNextFocus(mMidSkinny2Right, View::FOCUS_LEFT));
    EXPECT_EQ(nullptr, findNextFocus(mMidSkinny2Right, View::FOCUS_RIGHT));
    EXPECT_EQ(mMidSkinny1Left, findNextFocus(mMidSkinny2Right, View::FOCUS_UP));
    EXPECT_EQ(mBottomWide, findNextFocus(mMidSkinny2Right, View::FOCUS_DOWN));
}

TEST_F(VerticalFocusSearchTest, testSearchFromFromBottom) {
    EXPECT_EQ(nullptr, findNextFocus(mBottomWide, View::FOCUS_DOWN));
    EXPECT_EQ(nullptr, findNextFocus(mBottomWide, View::FOCUS_LEFT));
    EXPECT_EQ(nullptr, findNextFocus(mBottomWide, View::FOCUS_RIGHT));
    EXPECT_EQ(mMidSkinny2Right, findNextFocus(mBottomWide, View::FOCUS_UP));
}
