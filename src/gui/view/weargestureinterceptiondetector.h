/*********************************************************************************
 * Copyright (C) [2026] [houzh@msn.com]
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *********************************************************************************/
#ifndef __WEAR_GESTURE_INTERCEPTION_DETECTOR_H__
#define __WEAR_GESTURE_INTERCEPTION_DETECTOR_H__
// com.android.internal.policy.WearGestureInterceptionDetector (android-35) — the
// DecorView-installed detector behind the wear system swipe-to-dismiss. The
// Window (CDROID's decor) feeds it motion events; on interception the decor
// consumes the stream and drives the visual dismissal itself (AOSP hands that
// half to SystemUI through ViewRootImpl/IWindowSession).
#include <core/context.h>

namespace cdroid {

class MotionEvent;
class View;

class WearGestureInterceptionDetector {
private:
    View* mInstalledDecorView;
    float mTouchSlop;
    float mSwipingStartThreshold;
    bool mSwiping = false;

    float mDownX = 0.f;
    float mDownY = 0.f;
    int mActivePointerId = -1;
    bool mDiscardIntercept = false;

    int getIndexForValidPointer(MotionEvent& ev);
    void updateSwiping(MotionEvent& ev);
    void updateDiscardIntercept(MotionEvent& ev, int pointerIndex);
    void resetMembers();
    bool canScroll(View* v, bool checkSelf, bool checkLeft, float x, float y);
public:
    WearGestureInterceptionDetector(Context* context, View* installedDecorView);

    bool isIntercepting() const;
    bool onInterceptTouchEvent(MotionEvent& ev);
};

} // namespace cdroid
#endif /*__WEAR_GESTURE_INTERCEPTION_DETECTOR_H__*/
