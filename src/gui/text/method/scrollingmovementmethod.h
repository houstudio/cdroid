/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 * (LGPL v2.1+) — ported from Android's android.text.method.ScrollingMovementMethod.
 * Movement method for read-only scrollable text: movement actions scroll the view
 * instead of moving a caret. canSelectArbitrarily() == false.
 *********************************************************************************/
#ifndef __SCROLLING_MOVEMENT_METHOD_H__
#define __SCROLLING_MOVEMENT_METHOD_H__
#include <text/method/basemovementmethod.h>
namespace cdroid {

class ScrollingMovementMethod : public BaseMovementMethod {
public:
    static MovementMethod* getInstance();
    bool onTouchEvent(TextView& widget, Spannable& text, MotionEvent& event) override;

protected:
    bool left(TextView& w, Spannable& b)       override { return scrollLeft(w, b, 1); }
    bool right(TextView& w, Spannable& b)      override { return scrollRight(w, b, 1); }
    bool up(TextView& w, Spannable& b)         override { return scrollUp(w, b, 1); }
    bool down(TextView& w, Spannable& b)       override { return scrollDown(w, b, 1); }
    bool pageUp(TextView& w, Spannable& b)     override { return scrollPageUp(w, b); }
    bool pageDown(TextView& w, Spannable& b)   override { return scrollPageDown(w, b); }
    bool top(TextView& w, Spannable& b)        override { return scrollTop(w, b); }
    bool bottom(TextView& w, Spannable& b)     override { return scrollBottom(w, b); }
    bool lineStart(TextView& w, Spannable& b)  override { return scrollLineStart(w, b); }
    bool lineEnd(TextView& w, Spannable& b)    override { return scrollLineEnd(w, b); }
    bool home(TextView& w, Spannable& b)       override { return scrollLineStart(w, b); }
    bool end(TextView& w, Spannable& b)        override { return scrollLineEnd(w, b); }

private:
    float mLastY = 0;   // for minimal drag-scroll (Android uses the Touch helper)
};

}  // namespace cdroid
#endif  // __SCROLLING_MOVEMENT_METHOD_H__
