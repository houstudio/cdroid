/*********************************************************************************
 * Port of AOSP coretests android.widget.focus.FocusChangeWithInterestingRectHintTest
 * (+ AdjacentVerticalRectLists companion activity), in a private Window.
 * FocusFinder.findNextFocus and View.requestFocus(direction, Rect) work
 * together to give a newly focused item a hint about the most interesting
 * rectangle of the previously focused view; three adjacent InternalSelectionViews
 * use that hint to keep the internal row while snaking back and forth.
 * Source of truth:
 * frameworks/base/core/tests/coretests/src/android/widget/focus/FocusChangeWithInterestingRectHintTest.java
 * frameworks/base/core/tests/coretests/src/android/widget/focus/AdjacentVerticalRectLists.java
 *********************************************************************************/
#include <gtest/gtest.h>
#include <cdroid.h>
#include <guienvironment.h>
#include "keysender.h"
#include "internalselectionview.h"

using namespace cdroid;
using keynav::InternalSelectionView;

/* AdjacentVerticalRectLists: horizontal LinearLayout, three equal-weight
   5-row InternalSelectionViews with 10px padding. */
class FocusChangeWithInterestingRectHintTest : public testing::Test {
protected:
    Window* mActivity;
    LinearLayout* mLayout;
    InternalSelectionView* mLeftColumn;
    InternalSelectionView* mMiddleColumn;
    InternalSelectionView* mRightColumn;

    void SetUp() override {
        App& app = App::getInstance();
        mActivity = new Window(&app, 0, 0, -1, -1);

        mLayout = new LinearLayout(&app);
        mLayout->setOrientation(LinearLayout::HORIZONTAL);
        mLayout->setLayoutParams(new ViewGroup::LayoutParams(
                ViewGroup::LayoutParams::MATCH_PARENT,
                ViewGroup::LayoutParams::MATCH_PARENT));

        mLeftColumn = addColumn("left column");
        mMiddleColumn = addColumn("middle column");
        mRightColumn = addColumn("right column");

        mActivity->addView(mLayout);
        const int w = mActivity->getWidth() > 0 ? mActivity->getWidth() : 1080;
        const int h = mActivity->getHeight() > 0 ? mActivity->getHeight() : 1920;
        mLayout->measure(MeasureSpec::makeMeasureSpec(w, MeasureSpec::EXACTLY),
                         MeasureSpec::makeMeasureSpec(h, MeasureSpec::EXACTLY));
        mLayout->layout(0, 0, w, h);
        pumpUntilIdle();

        /* AOSP: the fresh activity window grants focus to the first
           focusable (left column). */
        mLeftColumn->requestFocus();
        pumpUntilIdle();
    }

    InternalSelectionView* addColumn(const std::string& label) {
        App& app = App::getInstance();
        InternalSelectionView* column = new InternalSelectionView(&app, 5, label);
        column->setLayoutParams(new LinearLayout::LayoutParams(
                0 /* width */, ViewGroup::LayoutParams::MATCH_PARENT, 1));
        column->setPadding(10, 10, 10, 10);
        mLayout->addView(column);
        return column;
    }
};

TEST_F(FocusChangeWithInterestingRectHintTest, testPreconditions) {
    ASSERT_NE(nullptr, mLeftColumn);
    ASSERT_NE(nullptr, mMiddleColumn);
    ASSERT_NE(nullptr, mRightColumn);
    ASSERT_TRUE(mLeftColumn->hasFocus());
    ASSERT_GT(mLeftColumn->getNumRows(), 2);
    ASSERT_EQ(mLeftColumn->getNumRows(), mMiddleColumn->getNumRows());
    ASSERT_EQ(mMiddleColumn->getNumRows(), mRightColumn->getNumRows());
}

TEST_F(FocusChangeWithInterestingRectHintTest, testSnakeBackAndForth) {
    const int numRows = mLeftColumn->getNumRows();
    for (int row = 0; row < numRows; row++) {
        if ((row % 2) == 0) {
            ASSERT_EQ(row, mLeftColumn->getSelectedRow());

            keynav::sendKey(KeyEvent::KEYCODE_DPAD_RIGHT, 0, mActivity);
            ASSERT_TRUE(mMiddleColumn->hasFocus());
            ASSERT_EQ(row, mMiddleColumn->getSelectedRow());

            keynav::sendKey(KeyEvent::KEYCODE_DPAD_RIGHT, 0, mActivity);
            ASSERT_TRUE(mRightColumn->hasFocus());
            ASSERT_EQ(row, mRightColumn->getSelectedRow());

            if (row < numRows - 1) {
                keynav::sendKey(KeyEvent::KEYCODE_DPAD_DOWN, 0, mActivity);
                ASSERT_EQ(row + 1, mRightColumn->getSelectedRow());
            }
        } else {
            ASSERT_TRUE(mRightColumn->hasFocus());

            keynav::sendKey(KeyEvent::KEYCODE_DPAD_LEFT, 0, mActivity);
            ASSERT_TRUE(mMiddleColumn->hasFocus());
            ASSERT_EQ(row, mMiddleColumn->getSelectedRow());

            keynav::sendKey(KeyEvent::KEYCODE_DPAD_LEFT, 0, mActivity);
            ASSERT_TRUE(mLeftColumn->hasFocus());
            ASSERT_EQ(row, mLeftColumn->getSelectedRow());

            if (row < numRows - 1) {
                keynav::sendKey(KeyEvent::KEYCODE_DPAD_DOWN, 0, mActivity);
                ASSERT_EQ(row + 1, mLeftColumn->getSelectedRow());
            }
        }
    }
}
