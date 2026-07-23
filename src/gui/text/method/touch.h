/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * (LGPL v2.1+) — ported from Android's android.text.method.Touch.
 * Drag-to-scroll helper used by movement methods. A DragState is attached to the
 * Spannable buffer as a NoCopySpan (borrowed, never cloned) so it survives across
 * onTouchEvent calls.
 *********************************************************************************/
#ifndef __TOUCH_H__
#define __TOUCH_H__
#include <text/parcelablespan.h>   // NoCopySpan
namespace cdroid{

class TextView;
class Layout;
class MotionEvent;
class Spannable;

class Touch {
private:
    Touch() = default;
public:
    class DragState;   // NoCopySpan holding the in-progress drag

    /** Constrained scroll: clamps X to the visible horizontal region at Y. */
    static void scrollTo(TextView& widget, Layout& layout, int x, int y);

    /** Handles drag-to-scroll (manages DragState). Returns true if consumed. */
    static bool onTouchEvent(TextView& widget, Spannable& buffer, MotionEvent& event);

    static int getInitialScrollX(TextView& widget, Spannable& buffer);
    static int getInitialScrollY(TextView& widget, Spannable& buffer);
};

// Mirrors Android's Touch.DragState. NoCopySpan → borrowed; never cloned/sliced.
class Touch::DragState : public NoCopySpan {
public:
    float mX;
    float mY;
    int mScrollX;
    int mScrollY;
    bool mFarEnough = false;
    bool mUsed = false;
    DragState(float x, float y, int scrollX, int scrollY)
        : mX(x), mY(y), mScrollX(scrollX), mScrollY(scrollY) {}
};

}/*endof namespace*/
#endif/*__TOUCH_H__*/
