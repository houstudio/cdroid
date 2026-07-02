/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 * (LGPL v2.1+) — ported from Android's android.text.method.ArrowKeyMovementMethod.
 *********************************************************************************/
#include <text/method/arrowkeymovementmethod.h>
#include <widget/textview.h>
#include <text/selection.h>
#include <text/layout.h>
#include <text/worditerator.h>
#include <view/keyevent.h>
#include <view/motionevent.h>
#include <core/rect.h>
#include <algorithm>

namespace cdroid {

MovementMethod* ArrowKeyMovementMethod::getInstance() {
    static ArrowKeyMovementMethod sInstance;
    return &sInstance;
}

void ArrowKeyMovementMethod::initialize(TextView& /*widget*/, Spannable& text) {
    Selection::setSelection(&text, 0);
}

void ArrowKeyMovementMethod::onTakeFocus(TextView& view, Spannable& text, int direction) {
    // Ported from Android ArrowKeyMovementMethod.onTakeFocus: taking focus
    // forward/down with a layout leaves the caret where initialize() put it (0);
    // every other case (back/up, or forward/down before layout) moves to the end.
    if ((direction & (View::FOCUS_FORWARD | View::FOCUS_DOWN)) != 0) {
        if (view.getLayout() == nullptr) {
            // This shouldn't be null, but do something sensible if it is.
            Selection::setSelection(&text, (int)text.length());
        }
    } else {
        Selection::setSelection(&text, (int)text.length());
    }
}

bool ArrowKeyMovementMethod::onKeyDown(TextView& widget, Spannable& text, int keyCode, KeyEvent& event) {
    // CDROID adaptation: derive selection mode from this event's SHIFT modifier
    // (Android stores it in the buffer via MetaKeyKeyListener, not ported).
    mSelecting = (event.getMetaState() & KeyEvent::META_SHIFT_ON) != 0;
    return BaseMovementMethod::onKeyDown(widget, text, keyCode, event);
}

bool ArrowKeyMovementMethod::onTouchEvent(TextView& /*widget*/, Spannable& /*text*/, MotionEvent& /*event*/) {
    // The Android version handles tap/drag/selecting-mode touch via the Touch
    // helper + a LAST_TAP_DOWN span. In CDROID the Editor currently owns touch
    // (tap→caret, drag→extend, double/triple-tap). Defer wiring MovementMethod
    // touch until that path migrates; return false so the host keeps handling it.
    return false;
}

int ArrowKeyMovementMethod::getCurrentLineTop(Spannable& buffer, Layout* layout) {
    return layout->getLineTop(layout->getLineForOffset(Selection::getSelectionEnd(&buffer)));
}

int ArrowKeyMovementMethod::getPageHeight(TextView& widget) {
    // Android uses the global visible rect; getHeight() is a fine approximation.
    return widget.getHeight();
}

bool ArrowKeyMovementMethod::handleMovementKey(TextView& widget, Spannable& buffer, int keyCode,
        int movementMetaState, KeyEvent& event) {
    // Ported from Android ArrowKeyMovementMethod.handleMovementKey. Its only
    // special case is DPAD_CENTER while selecting → showContextMenu(); that path
    // needs MetaKeyKeyListener (META_SELECTING in the buffer) and View.showContextMenu,
    // neither of which is ported yet, so for now every key falls through to the
    // base decoder.
    switch (keyCode) {
        case KeyEvent::KEYCODE_DPAD_CENTER:
            if (KeyEvent::metaStateHasNoModifiers(movementMetaState)) {
                if (event.getAction() == KeyEvent::ACTION_DOWN
                        && event.getRepeatCount() == 0) {
                    // Android: && MetaKeyKeyListener.getMetaState(buffer,
                    //           META_SELECTING, event) != 0 → widget.showContextMenu();
                    // Deferred: META_SELECTING is not tracked (no MetaKeyKeyListener).
                }
            }
            break;
    }
    return BaseMovementMethod::handleMovementKey(widget, buffer, keyCode, movementMetaState, event);
}

bool ArrowKeyMovementMethod::leftWord(TextView& widget, Spannable& buffer) {
    // Ported from Android ArrowKeyMovementMethod.leftWord. Android reuses the
    // widget's cached WordIterator via getWordIterator(); CDROID has none, so
    // construct one for the default locale (Android's getWordIterator does the same
    // lazily).
    const int selectionEnd = widget.getSelectionEnd();
    WordIterator wordIterator;
    wordIterator.setCharSequence(&buffer, selectionEnd, selectionEnd);
    return Selection::moveToPreceding(&buffer, wordIterator, isSelecting());
}

bool ArrowKeyMovementMethod::rightWord(TextView& widget, Spannable& buffer) {
    const int selectionEnd = widget.getSelectionEnd();
    WordIterator wordIterator;
    wordIterator.setCharSequence(&buffer, selectionEnd, selectionEnd);
    return Selection::moveToFollowing(&buffer, wordIterator, isSelecting());
}

bool ArrowKeyMovementMethod::left(TextView& widget, Spannable& buffer) {
    if (widget.isOffsetMappingAvailable()) return false;
    Layout* layout = widget.getLayout();
    return layout && (isSelecting() ? Selection::extendLeft(&buffer, layout) : Selection::moveLeft(&buffer, layout));
}
bool ArrowKeyMovementMethod::right(TextView& widget, Spannable& buffer) {
    if (widget.isOffsetMappingAvailable()) return false;
    Layout* layout = widget.getLayout();
    return layout && (isSelecting() ? Selection::extendRight(&buffer, layout) : Selection::moveRight(&buffer, layout));
}
bool ArrowKeyMovementMethod::up(TextView& widget, Spannable& buffer) {
    if (widget.isOffsetMappingAvailable()) return false;
    Layout* layout = widget.getLayout();
    return layout && (isSelecting() ? Selection::extendUp(&buffer, layout) : Selection::moveUp(&buffer, layout));
}
bool ArrowKeyMovementMethod::down(TextView& widget, Spannable& buffer) {
    if (widget.isOffsetMappingAvailable()) return false;
    Layout* layout = widget.getLayout();
    return layout && (isSelecting() ? Selection::extendDown(&buffer, layout) : Selection::moveDown(&buffer, layout));
}

bool ArrowKeyMovementMethod::top(TextView&, Spannable& buffer) {
    if (isSelecting()) Selection::extendSelection(&buffer, 0);
    else               Selection::setSelection(&buffer, 0);
    return true;
}
bool ArrowKeyMovementMethod::bottom(TextView&, Spannable& buffer) {
    const int end = (int)buffer.length();
    if (isSelecting()) Selection::extendSelection(&buffer, end);
    else               Selection::setSelection(&buffer, end);
    return true;
}
bool ArrowKeyMovementMethod::lineStart(TextView& widget, Spannable& buffer) {
    if (widget.isOffsetMappingAvailable()) return false;
    Layout* layout = widget.getLayout();
    return layout && (isSelecting() ? Selection::extendToLeftEdge(&buffer, layout) : Selection::moveToLeftEdge(&buffer, layout));
}
bool ArrowKeyMovementMethod::lineEnd(TextView& widget, Spannable& buffer) {
    if (widget.isOffsetMappingAvailable()) return false;
    Layout* layout = widget.getLayout();
    return layout && (isSelecting() ? Selection::extendToRightEdge(&buffer, layout) : Selection::moveToRightEdge(&buffer, layout));
}
bool ArrowKeyMovementMethod::home(TextView& widget, Spannable& buffer)            { return lineStart(widget, buffer); }
bool ArrowKeyMovementMethod::end(TextView& widget, Spannable& buffer)             { return lineEnd(widget, buffer); }
bool ArrowKeyMovementMethod::previousParagraph(TextView& widget, Spannable& buffer) {
    if (widget.isOffsetMappingAvailable()) return false;
    if (widget.getLayout() == nullptr) return false;
    return isSelecting() ? Selection::extendToParagraphStart(&buffer) : Selection::moveToParagraphStart(&buffer, widget.getLayout());
}
bool ArrowKeyMovementMethod::nextParagraph(TextView& widget, Spannable& buffer) {
    if (widget.isOffsetMappingAvailable()) return false;
    if (widget.getLayout() == nullptr) return false;
    return isSelecting() ? Selection::extendToParagraphEnd(&buffer) : Selection::moveToParagraphEnd(&buffer, widget.getLayout());
}

bool ArrowKeyMovementMethod::pageUp(TextView& widget, Spannable& buffer) {
    if (widget.isOffsetMappingAvailable()) return false;
    Layout* layout = widget.getLayout();
    if (!layout) return false;
    const bool selecting = isSelecting();
    const int targetY = getCurrentLineTop(buffer, layout) - getPageHeight(widget);
    bool handled = false;
    for (;;) {
        const int prev = Selection::getSelectionEnd(&buffer);
        if (selecting) Selection::extendUp(&buffer, layout); else Selection::moveUp(&buffer, layout);
        if (Selection::getSelectionEnd(&buffer) == prev) break;
        handled = true;
        if (getCurrentLineTop(buffer, layout) <= targetY) break;
    }
    return handled;
}

bool ArrowKeyMovementMethod::pageDown(TextView& widget, Spannable& buffer) {
    if (widget.isOffsetMappingAvailable()) return false;
    Layout* layout = widget.getLayout();
    if (!layout) return false;
    const bool selecting = isSelecting();
    const int targetY = getCurrentLineTop(buffer, layout) + getPageHeight(widget);
    bool handled = false;
    for (;;) {
        const int prev = Selection::getSelectionEnd(&buffer);
        if (selecting) Selection::extendDown(&buffer, layout); else Selection::moveDown(&buffer, layout);
        if (Selection::getSelectionEnd(&buffer) == prev) break;
        handled = true;
        if (getCurrentLineTop(buffer, layout) >= targetY) break;
    }
    return handled;
}

}  // namespace cdroid
