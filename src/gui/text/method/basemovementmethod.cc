/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 * (LGPL v2.1+) — ported from Android's android.text.method.BaseMovementMethod.
 *********************************************************************************/
#include <text/method/basemovementmethod.h>
#include <widget/textview.h>
#include <text/layout.h>
#include <view/keyevent.h>
#include <cmath>
#include <algorithm>

namespace cdroid {

namespace {
// CDROID pragmatic meta helpers (Android has KeyEvent.metaStateHas*).
inline bool noModifiers(int m) {
    return (m & (KeyEvent::META_SHIFT_MASK | KeyEvent::META_ALT_MASK | KeyEvent::META_CTRL_MASK)) == 0;
}
inline bool hasCtrl(int m) {
    return (m & KeyEvent::META_CTRL_ON) && !(m & (KeyEvent::META_SHIFT_MASK | KeyEvent::META_ALT_MASK));
}
inline bool hasAlt(int m) {
    return (m & KeyEvent::META_ALT_ON) && !(m & (KeyEvent::META_SHIFT_MASK | KeyEvent::META_CTRL_MASK));
}
}  // namespace

int BaseMovementMethod::getMovementMetaState(Spannable& /*buffer*/, KeyEvent& event) {
    // CDROID adaptation: read modifiers off the event (Android merges buffer meta
    // via MetaKeyKeyListener, which isn't ported). Movement ignores SHIFT.
    return event.getMetaState() & ~KeyEvent::META_SHIFT_MASK;
}

bool BaseMovementMethod::onKeyDown(TextView& widget, Spannable& text, int keyCode, KeyEvent& event) {
    // Android also calls MetaKeyKeyListener.adjustMetaAfterKeypress/resetLockedMeta
    // here; with no span-based meta store, there is nothing to adjust.
    return handleMovementKey(widget, text, keyCode, getMovementMetaState(text, event), event);
}

bool BaseMovementMethod::onKeyOther(TextView& widget, Spannable& text, KeyEvent& event) {
    const int keyCode = event.getKeyCode();
    if (keyCode != KeyEvent::KEYCODE_UNKNOWN && event.getAction() == KeyEvent::ACTION_MULTIPLE) {
        const int movementMetaState = getMovementMetaState(text, event);
        const int repeat = event.getRepeatCount();
        bool handled = false;
        for (int i = 0; i < repeat; i++) {
            if (!handleMovementKey(widget, text, keyCode, movementMetaState, event)) break;
            handled = true;
        }
        return handled;
    }
    return false;
}

bool BaseMovementMethod::handleMovementKey(TextView& widget, Spannable& buffer,
        int keyCode, int movementMetaState, KeyEvent& /*event*/) {
    switch (keyCode) {
    case KeyEvent::KEYCODE_DPAD_LEFT:
        if (noModifiers(movementMetaState))      return left(widget, buffer);
        else if (hasCtrl(movementMetaState))     return leftWord(widget, buffer);
        else if (hasAlt(movementMetaState))      return lineStart(widget, buffer);
        break;
    case KeyEvent::KEYCODE_DPAD_RIGHT:
        if (noModifiers(movementMetaState))      return right(widget, buffer);
        else if (hasCtrl(movementMetaState))     return rightWord(widget, buffer);
        else if (hasAlt(movementMetaState))      return lineEnd(widget, buffer);
        break;
    case KeyEvent::KEYCODE_DPAD_UP:
        if (noModifiers(movementMetaState))      return up(widget, buffer);
        else if (hasAlt(movementMetaState))      return top(widget, buffer);
        else if (hasCtrl(movementMetaState))     return previousParagraph(widget, buffer);
        break;
    case KeyEvent::KEYCODE_DPAD_DOWN:
        if (noModifiers(movementMetaState))      return down(widget, buffer);
        else if (hasAlt(movementMetaState))      return bottom(widget, buffer);
        else if (hasCtrl(movementMetaState))     return nextParagraph(widget, buffer);
        break;
    case KeyEvent::KEYCODE_PAGE_UP:
        if (noModifiers(movementMetaState))      return pageUp(widget, buffer);
        else if (hasAlt(movementMetaState))      return top(widget, buffer);
        break;
    case KeyEvent::KEYCODE_PAGE_DOWN:
        if (noModifiers(movementMetaState))      return pageDown(widget, buffer);
        else if (hasAlt(movementMetaState))      return bottom(widget, buffer);
        break;
    case KeyEvent::KEYCODE_MOVE_HOME:
        if (noModifiers(movementMetaState))      return home(widget, buffer);
        else if (hasCtrl(movementMetaState))     return top(widget, buffer);
        break;
    case KeyEvent::KEYCODE_MOVE_END:
        if (noModifiers(movementMetaState))      return end(widget, buffer);
        else if (hasCtrl(movementMetaState))     return bottom(widget, buffer);
        break;
    }
    return false;
}

// ---- scroll geometry helpers ----
static int getInnerWidth(TextView& w)  { return w.getWidth()  - w.getTotalPaddingLeft() - w.getTotalPaddingRight(); }
static int getInnerHeight(TextView& w) { return w.getHeight() - w.getTotalPaddingTop()  - w.getTotalPaddingBottom(); }
static int getCharacterWidth(TextView& w) { return (int)std::ceil(w.getLineHeight()); }
static int getTopLine(TextView& w)    { return w.getLayout()->getLineForVertical(w.getScrollY()); }
static int getBottomLine(TextView& w) { return w.getLayout()->getLineForVertical(w.getScrollY() + getInnerHeight(w)); }

bool BaseMovementMethod::scrollLeft(TextView& widget, Spannable& /*buffer*/, int amount) {
    Layout* layout = widget.getLayout();
    if (!layout) return false;
    int minScrollX = 0;   // Android: min of getLineLeft over visible lines; 0 is fine for LTR.
    int scrollX = widget.getScrollX();
    if (scrollX > minScrollX) {
        scrollX = std::max(scrollX - getCharacterWidth(widget) * amount, minScrollX);
        widget.scrollTo(scrollX, widget.getScrollY());
        return true;
    }
    return false;
}

bool BaseMovementMethod::scrollRight(TextView& widget, Spannable& /*buffer*/, int amount) {
    Layout* layout = widget.getLayout();
    if (!layout) return false;
    int maxScrollX = (int)std::ceil(layout->getLineRight(0)) - getInnerWidth(widget);
    int scrollX = widget.getScrollX();
    if (scrollX < maxScrollX) {
        scrollX = std::min(scrollX + getCharacterWidth(widget) * amount, maxScrollX);
        widget.scrollTo(scrollX, widget.getScrollY());
        return true;
    }
    return false;
}

bool BaseMovementMethod::scrollUp(TextView& widget, Spannable& /*buffer*/, int amount) {
    Layout* layout = widget.getLayout();
    if (!layout) return false;
    const int top = widget.getScrollY();
    int topLine = layout->getLineForVertical(top);
    if (layout->getLineTop(topLine) == top) topLine -= 1;   // partially visible → previous
    if (topLine >= 0) {
        topLine = std::max(topLine - amount + 1, 0);
        widget.scrollTo(widget.getScrollX(), layout->getLineTop(topLine));
        return true;
    }
    return false;
}

bool BaseMovementMethod::scrollDown(TextView& widget, Spannable& /*buffer*/, int amount) {
    Layout* layout = widget.getLayout();
    if (!layout) return false;
    const int innerHeight = getInnerHeight(widget);
    const int bottom = widget.getScrollY() + innerHeight;
    int bottomLine = layout->getLineForVertical(bottom);
    if (layout->getLineTop(bottomLine + 1) < bottom + 1) bottomLine += 1;
    const int limit = layout->getLineCount() - 1;
    if (bottomLine <= limit) {
        bottomLine = std::min(bottomLine + amount - 1, limit);
        widget.scrollTo(widget.getScrollX(), layout->getLineTop(bottomLine + 1) - innerHeight);
        return true;
    }
    return false;
}

bool BaseMovementMethod::scrollPageUp(TextView& widget, Spannable& buffer) {
    return scrollUp(widget, buffer, std::max(1, getInnerHeight(widget) / std::max(1, (int)std::ceil(widget.getLineHeight()))));
}

bool BaseMovementMethod::scrollPageDown(TextView& widget, Spannable& buffer) {
    return scrollDown(widget, buffer, std::max(1, getInnerHeight(widget) / std::max(1, (int)std::ceil(widget.getLineHeight()))));
}

bool BaseMovementMethod::scrollTop(TextView& widget, Spannable& /*buffer*/) {
    Layout* layout = widget.getLayout();
    if (!layout) return false;
    if (getTopLine(widget) >= 0) { widget.scrollTo(widget.getScrollX(), layout->getLineTop(0)); return true; }
    return false;
}

bool BaseMovementMethod::scrollBottom(TextView& widget, Spannable& /*buffer*/) {
    Layout* layout = widget.getLayout();
    if (!layout) return false;
    const int lineCount = layout->getLineCount();
    if (getBottomLine(widget) <= lineCount - 1) {
        widget.scrollTo(widget.getScrollX(), layout->getLineTop(lineCount) - getInnerHeight(widget));
        return true;
    }
    return false;
}

bool BaseMovementMethod::scrollLineStart(TextView& widget, Spannable& /*buffer*/) {
    if (widget.getScrollX() > 0) { widget.scrollTo(0, widget.getScrollY()); return true; }
    return false;
}

bool BaseMovementMethod::scrollLineEnd(TextView& widget, Spannable& /*buffer*/) {
    Layout* layout = widget.getLayout();
    if (!layout) return false;
    int maxScrollX = (int)std::ceil(layout->getLineRight(0)) - getInnerWidth(widget);
    if (widget.getScrollX() < maxScrollX) { widget.scrollTo(maxScrollX, widget.getScrollY()); return true; }
    return false;
}

}  // namespace cdroid
