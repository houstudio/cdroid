/*********************************************************************************
 * Port of AOSP coretests android.util.ListUtil — various useful stuff for
 * instrumentation-testing a ListView. The Instrumentation dependency maps to
 * the keynav key sender (sendCharacterSync / waitForIdleSync).
 * Source of truth:
 * frameworks/base/core/tests/coretests/src/android/util/ListUtil.java
 *********************************************************************************/
#ifndef KEYNAV_LISTUTIL_H
#define KEYNAV_LISTUTIL_H
#include <gtest/gtest.h>
#include <cdroid.h>
#include "keysender.h"

namespace keynav {

class ListUtil {
private:
    cdroid::ListView* mListView;
    cdroid::Window* mTarget; /* the activity window receiving key events */

public:
    ListUtil(cdroid::ListView* listView, cdroid::Window* target)
          : mListView(listView), mTarget(target) {}

    /* Set the selected position of the list view. */
    void setSelectedPosition(int pos) {
        mListView->post([this, pos]() { mListView->setSelection(pos); });
        pumpUntilIdle();
    }

    /* Get the top of the list. */
    int getListTop() const {
        return mListView->getListPaddingTop();
    }

    /* Get the bottom of the list. */
    int getListBottom() const {
        return mListView->getHeight() - mListView->getListPaddingBottom();
    }

    /* Arrow (up or down as appropriate) to the desired position in the list.
       AOSP throws IllegalStateException after 20 presses; here the caller
       asserts the returned position. */
    void arrowScrollToSelectedPosition(int desiredPos) {
        if (desiredPos > mListView->getSelectedItemPosition()) {
            arrowDownToSelectedPosition(desiredPos);
        } else {
            arrowUpToSelectedPosition(desiredPos);
        }
    }

private:
    void arrowDownToSelectedPosition(int position) {
        int maxDowns = 20;
        while (mListView->getSelectedItemPosition() < position && --maxDowns > 0) {
            sendKey(KeyEvent::KEYCODE_DPAD_DOWN, 0, mTarget);
        }
        EXPECT_EQ(position, mListView->getSelectedItemPosition())
                << "couldn't get to item after 20 downs";
    }

    void arrowUpToSelectedPosition(int position) {
        int maxUps = 20;
        while (mListView->getSelectedItemPosition() > position && --maxUps > 0) {
            sendKey(KeyEvent::KEYCODE_DPAD_UP, 0, mTarget);
        }
        EXPECT_EQ(position, mListView->getSelectedItemPosition())
                << "couldn't get to item after 20 ups";
    }
};

} // namespace keynav
#endif
