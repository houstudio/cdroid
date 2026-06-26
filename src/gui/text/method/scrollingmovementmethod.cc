/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 * (LGPL v2.1+) — ported from Android's android.text.method.ScrollingMovementMethod.
 *********************************************************************************/
#include <text/method/scrollingmovementmethod.h>
#include <widget/textview.h>
#include <view/motionevent.h>
#include <algorithm>

namespace cdroid {

MovementMethod* ScrollingMovementMethod::getInstance() {
    static ScrollingMovementMethod sInstance;
    return &sInstance;
}

bool ScrollingMovementMethod::onTouchEvent(TextView& widget, Spannable& buffer, MotionEvent& event) {
    // Minimal drag-scroll: Android routes through the Touch helper (not ported),
    // which also tracks initial scroll X/Y spans. Here we scroll by whole lines
    // proportional to the vertical drag delta — enough for read-only scrolling text.
    const int action = event.getActionMasked();
    if (action == MotionEvent::ACTION_DOWN) {
        mLastY = event.getY();
        return true;
    }
    if (action == MotionEvent::ACTION_MOVE) {
        const float spacing = std::max(1.f, (float)widget.getLineHeight());
        const int lines = (int)((mLastY - event.getY()) / spacing);
        if (lines > 0)      scrollDown(widget, buffer, lines);
        else if (lines < 0) scrollUp(widget, buffer, -lines);
        mLastY = event.getY();
        return true;
    }
    return false;
}

}  // namespace cdroid
