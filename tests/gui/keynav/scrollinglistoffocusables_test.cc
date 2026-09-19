/*********************************************************************************
 * Port of AOSP coretests android.widget.focus.ScrollingThroughListOfFocusablesTest
 * (+ ListOfInternalSelectionViews companion activity), in a private Window.
 * Source of truth:
 * frameworks/base/core/tests/coretests/src/android/widget/focus/ScrollingThroughListOfFocusablesTest.java
 * frameworks/base/core/tests/coretests/src/android/widget/focus/ListOfInternalSelectionViews.java
 *********************************************************************************/
#include <gtest/gtest.h>
#include <cdroid.h>
#include <guienvironment.h>
#include "keysender.h"
#include "internalselectionview.h"

using namespace cdroid;
using keynav::InternalSelectionView;

/* ListOfInternalSelectionViews: a ListView of InternalSelectionViews whose
   geometry is configured by numItems / numRowsPerItem / itemScreenHeightFactor
   (AOSP passes them via the launch intent's bundle). */
class ListOfInternalSelectionViews {
private:
    Window* mWindow;
    ListView* mListView;

    int mScreenHeight;
    int mNumItems;
    int mNumRowsPerItem;
    double mItemScreenSizeFactor;

    class MyAdapter : public Adapter {
    private:
        ListOfInternalSelectionViews* mActivity;
    public:
        MyAdapter(ListOfInternalSelectionViews* activity) : mActivity(activity) {}

        int getCount() const override { return mActivity->mNumItems; }
        void* getItem(int position) const override { return nullptr; }
        long getItemId(int position) const override { return position; }

        View* getView(int position, View* /*convertView*/, ViewGroup* parent) override {
            InternalSelectionView* item = new InternalSelectionView(
                    parent->getContext(), mActivity->mNumRowsPerItem,
                    mActivity->getLabelForPosition(position));
            item->setDesiredHeight((int)(mActivity->mScreenHeight * mActivity->mItemScreenSizeFactor));
            return item;
        }
    };

public:
    ListOfInternalSelectionViews(int numItems, int numRowsPerItem, double itemScreenHeightFactor)
          : mNumItems(numItems), mNumRowsPerItem(numItems > 0 ? numRowsPerItem : 4),
            mItemScreenSizeFactor(itemScreenHeightFactor > 0 ? itemScreenHeightFactor : 5.0 / 4) {
        App& app = App::getInstance();
        mWindow = new Window(&app, 0, 0, -1, -1);
        mScreenHeight = GUIEnvironment::stage()->getHeight();

        mListView = new ListView(&app);
        mListView->setLayoutParams(new ViewGroup::LayoutParams(
                ViewGroup::LayoutParams::MATCH_PARENT,
                ViewGroup::LayoutParams::MATCH_PARENT));
        mListView->setDrawSelectorOnTop(false);
        mListView->setAdapter(new MyAdapter(this));
        mListView->setItemsCanFocus(true);

        mWindow->addView(mListView);
        /* AOSP ordering: the window grants focus BEFORE the first traversal
           (ViewRootImpl assigns focus at traversal start, layout runs after),
           so ListView::layoutChildren's "selected item takes focus when
           itemsCanFocus" handoff fires on the first layout and the item
           starts focused at row 0 (the first assert of every test expects
           that). Request focus before emulating the first traversal with
           the manual measure/layout below. */
        mListView->requestFocus();
        const int w = mWindow->getWidth() > 0 ? mWindow->getWidth() : 1080;
        mListView->measure(MeasureSpec::makeMeasureSpec(w, MeasureSpec::EXACTLY),
                           MeasureSpec::makeMeasureSpec(mScreenHeight, MeasureSpec::EXACTLY));
        mListView->layout(0, 0, w, mScreenHeight);
        pumpUntilIdle();
    }

    ListView* getListView() const { return mListView; }
    Window* getWindow() const { return mWindow; }
    int getScreenHeight() const { return mScreenHeight; }
    int getNumRowsPerItem() const { return mNumRowsPerItem; }
    int getNumItems() const { return mNumItems; }

    std::string getLabelForPosition(int position) const {
        return "position " + std::to_string(position);
    }

    InternalSelectionView* getSelectedView() const {
        return (InternalSelectionView*)getListView()->getSelectedView();
    }
};

class ScrollingThroughListOfFocusablesTest : public testing::Test {
protected:
    Rect mTempRect;

    ListOfInternalSelectionViews* mActivity;
    ListView* mListView;

    int mNumItems = 4;
    int mNumRowsPerItem = 5;
    double mScreenHeightFactor = 5.0 / 4;

    void SetUp() override {
        mActivity = new ListOfInternalSelectionViews(mNumItems, mNumRowsPerItem, mScreenHeightFactor);
        mListView = mActivity->getListView();
        /* Make sure we have some fading edge regardless of ListView style. */
        mListView->setVerticalFadingEdgeEnabled(true);
        mListView->setFadingEdgeLength(10);
        ensureNotInTouchMode();

        /* focus the listview */
        mListView->requestFocus();
        pumpUntilIdle();
    }

    void assertInternallySelectedRowOnScreen(InternalSelectionView* internalFocused, int row) {
        ASSERT_EQ(row, internalFocused->getSelectedRow()) << "expecting selected row";

        internalFocused->getRectForRow(mTempRect, row);
        mListView->offsetDescendantRectToMyCoords(internalFocused, mTempRect);

        ASSERT_GE(mTempRect.top, 0) << "top of row " << row << " should be on screen";
        ASSERT_LT(mTempRect.bottom(), mActivity->getScreenHeight())
                << "bottom of row " << row << " should be on screen";
    }

    void ensureNotInTouchMode() {
        /* If in touch mode inject a DPAD down event to exit that mode. */
        if (mListView->isInTouchMode()) {
            keynav::sendKey(KeyEvent::KEYCODE_DPAD_DOWN, 0, mActivity->getWindow());
            pumpUntilIdle();
        }
    }
};

TEST_F(ScrollingThroughListOfFocusablesTest, testPreconditions) {
    ASSERT_NE(nullptr, mActivity);
    ASSERT_NE(nullptr, mListView);
    ASSERT_EQ(mNumItems, mActivity->getNumItems());
    ASSERT_EQ(mNumRowsPerItem, mActivity->getNumRowsPerItem());
}

TEST_F(ScrollingThroughListOfFocusablesTest, testScrollingDownInFirstItem) {
    for (int i = 0; i < mNumRowsPerItem; i++) {
        ASSERT_EQ(0, mListView->getSelectedItemPosition());

        InternalSelectionView* view = mActivity->getSelectedView();

        assertInternallySelectedRowOnScreen(view, i);

        /* move to next row */
        if (i < mNumRowsPerItem - 1) {
            keynav::sendKey(KeyEvent::KEYCODE_DPAD_DOWN, 0, mActivity->getWindow());
            pumpUntilIdle();
        }
    }

    {
        ASSERT_EQ(0, mListView->getSelectedItemPosition());
        InternalSelectionView* view = (InternalSelectionView*)mListView->getSelectedView();

        /* 1 pixel tolerance in case height / 4 is not an even number */
        const int bottomFadingEdgeTop =
                mListView->getBottom() - mListView->getVerticalFadingEdgeLength();
        EXPECT_EQ(bottomFadingEdgeTop, view->getBottom())
                << "bottom of view should be just above fading edge";
    }

    /* make sure fading edge is the expected view */
    {
        EXPECT_EQ(2, mListView->getChildCount())
                << "should be a second view visible due to the fading edge";
        InternalSelectionView* peekingChild = (InternalSelectionView*)mListView->getChildAt(1);
        ASSERT_NE(nullptr, peekingChild);
        EXPECT_EQ(mActivity->getLabelForPosition(1), peekingChild->getLabel())
                << "wrong value for peeking list item";
    }
}

TEST_F(ScrollingThroughListOfFocusablesTest, testScrollingToSecondItem) {
    for (int i = 0; i < mNumRowsPerItem; i++) {
        keynav::sendKey(KeyEvent::KEYCODE_DPAD_DOWN, 0, mActivity->getWindow());
        pumpUntilIdle();
    }

    EXPECT_EQ(1, mListView->getSelectedItemPosition())
            << "should have moved to second item";
}

TEST_F(ScrollingThroughListOfFocusablesTest, testNoFadingEdgeAtBottomOfLastItem) {
    /* move down to last item */
    for (int i = 0; i < mNumItems; i++) {
        for (int j = 0; j < mNumRowsPerItem; j++) {
            if (i < mNumItems - 1 || j < mNumRowsPerItem - 1) {
                keynav::sendKey(KeyEvent::KEYCODE_DPAD_DOWN, 0, mActivity->getWindow());
                pumpUntilIdle();
            }
        }
    }

    ASSERT_EQ(mNumItems - 1, mListView->getSelectedItemPosition());
    InternalSelectionView* view = mActivity->getSelectedView();
    ASSERT_EQ(mNumRowsPerItem - 1, view->getSelectedRow());

    view->getRectForRow(mTempRect, mNumRowsPerItem - 1);
    mListView->offsetDescendantRectToMyCoords(view, mTempRect);

    EXPECT_LT(mListView->getBottom() - mListView->getVerticalFadingEdgeLength(), mTempRect.bottom())
            << "bottom of last row of last item should be at the bottom of the "
            "list view (no fading edge)";
}

TEST_F(ScrollingThroughListOfFocusablesTest, testNavigatingUpThroughInternalSelection) {
    /* get to bottom of second item */
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < mNumRowsPerItem; j++) {
            if (i < 1 || j < mNumRowsPerItem - 1) {
                keynav::sendKey(KeyEvent::KEYCODE_DPAD_DOWN, 0, mActivity->getWindow());
                pumpUntilIdle();
            }
        }
    }

    /* (make sure we are at last row of second item) */
    {
        ASSERT_EQ(1, mListView->getSelectedItemPosition());
        InternalSelectionView* view = mActivity->getSelectedView();
        ASSERT_EQ(mNumRowsPerItem - 1, view->getSelectedRow());
    }

    /* go back up to the top of the second item */
    for (int i = mNumRowsPerItem - 1; i >= 0; i--) {
        ASSERT_EQ(1, mListView->getSelectedItemPosition());
        InternalSelectionView* view = mActivity->getSelectedView();

        assertInternallySelectedRowOnScreen(view, i);

        /* move up to next row */
        if (i > 0) {
            keynav::sendKey(KeyEvent::KEYCODE_DPAD_UP, 0, mActivity->getWindow());
            pumpUntilIdle();
        }
    }

    /* now we are at top row, should have caused scrolling, and fading edge... */
    {
        ASSERT_EQ(1, mListView->getSelectedItemPosition());
        InternalSelectionView* view = mActivity->getSelectedView();
        ASSERT_EQ(0, view->getSelectedRow());

        view->getDrawingRect(mTempRect);
        mListView->offsetDescendantRectToMyCoords(view, mTempRect);
        EXPECT_EQ(mListView->getVerticalFadingEdgeLength(), view->getTop())
                << "top of selected row should be just below top vertical fading edge";
    }

    /* make sure fading edge is the view we expect */
    {
        InternalSelectionView* view = (InternalSelectionView*)mListView->getChildAt(0);
        EXPECT_EQ(mActivity->getLabelForPosition(0), view->getLabel());
    }
}
