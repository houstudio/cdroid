/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
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
#include <widget/editor.h>
#include <widget/textview.h>
#include <text/method/movementmethod.h>
#include <text/selection.h>
#include <text/editable.h>
#include <text/spannablestringbuilder.h>
#include <text/worditerator.h>
#include <text/layout.h>
#include <text/parcelablespan.h>
#include <view/keyevent.h>
#include <view/motionevent.h>
#include <view/view.h>
#include <view/viewconfiguration.h>
#include <core/inputmethodmanager.h>
#include <core/canvas.h>
#include <core/systemclock.h>
#include <algorithm>

namespace cdroid {

namespace {
// Cursor blink period, in milliseconds (matches Android's Editor.BLINK).
constexpr int BLINK = 500;
}  // namespace

// =====================================================================================
//  Construction / destruction
// =====================================================================================
Editor::Editor(TextView* textView) : mTextView(textView) {
    // Body mirrors Android's Editor.Blink.run(): a cancelled blink is a no-op;
    // otherwise drop any pending duplicate, and while still blinking invalidate
    // the caret path and reschedule for the next half-cycle. The visible on/off
    // is decided at draw time by shouldRenderCursor() (time-based), not here.
    mBlink = [this]() {
        if (mBlinkCancelled) return;
        mTextView->removeCallbacks(mBlink);
        if (shouldBlink()) {
            if (mTextView->getLayout() != nullptr) mTextView->invalidateCursorPath();
            mTextView->postDelayed(mBlink, BLINK);
        }
    };
}

Editor::~Editor() {
    if (mTextView) mTextView->removeCallbacks(mBlink);
}

// =====================================================================================
//  Helpers
// =====================================================================================
Spannable* Editor::editable() const {
    // Editable is-a Spannable; the upcast is implicit and valid for a complete object.
    return mTextView->getEditableText();
}

bool Editor::isEditing() const {
    return editable() != nullptr;
}

int Editor::cursorOffset() const {
    Spannable* e = editable();
    if (e) {
        const int end = Selection::getSelectionEnd(e);
        if (end >= 0) return end;
    }
    return mTextView->getCaretPos();   // legacy fallback
}

int Editor::insertPosition() {
    Editable* ed = mTextView->getEditableText();
    Spannable* e = editable();
    if (ed && e) {
        const int selStart = Selection::getSelectionStart(e);
        const int selEnd = Selection::getSelectionEnd(e);
        // A real selection (start != end) is replaced by the typed text; a bare
        // caret has start == end and is left untouched.
        if (selStart >= 0 && selEnd >= 0 && selStart != selEnd) {
            const int lo = std::min(selStart, selEnd);
            const int hi = std::max(selStart, selEnd);
            ed->Delete(lo, hi);
            return lo;
        }
    }
    return cursorOffset();
}

void Editor::replace() {
    /*if (mSuggestionsPopupWindow == null) {
        mSuggestionsPopupWindow = new SuggestionsPopupWindow();
    }
    hideCursorAndSpanControllers();
    mSuggestionsPopupWindow.show();*/
    int middle = (mTextView->getSelectionStart() + mTextView->getSelectionEnd()) / 2;
    if (Spannable* e = editable()) Selection::setSelection(e, middle);
}
// =====================================================================================
//  Lifecycle
// =====================================================================================
void Editor::onAttachedToWindow() {
    makeBlink();
}

void Editor::onDetachedFromWindow() {
    if (mTextView) mTextView->removeCallbacks(mBlink);
}

void Editor::onFocusChanged(bool focused, int /*direction*/, Rect* /*previouslyFocusedRect*/) {
    if (focused) {
        mCursorVisible = true;
        makeBlink();
        // Android: TextView.mSelectAllOnFocus — select all when focus is gained.
        if (mSelectAllOnFocus && isEditing()) selectAll();
    } else {
        suspendBlink();
        hideCursorControllers();
    }
}

void Editor::onWindowFocusChanged(bool hasWindowFocus) {
    if (hasWindowFocus) {
        makeBlink();
    } else {
        suspendBlink();
    }
}

// =====================================================================================
//  Cursor blink + geometry / draw  (faithful ports of android.widget.Editor)
//  (isCursorVisible is inline in editor.h: mCursorVisible && isEditing().)
// =====================================================================================
bool Editor::shouldBlink() const {
    // Android.shouldBlink: visible, focused, the window is visible, and the
    // selection is a zero-length caret.
    if (!isCursorVisible() || !mTextView->isFocused()
            || mTextView->getWindowVisibility() != View::VISIBLE) return false;

    Spannable* e = editable();
    const int start = e ? Selection::getSelectionStart(e) : -1;
    if (start < 0) return false;
    const int end = e ? Selection::getSelectionEnd(e) : -1;
    if (end < 0) return false;
    return start == end;
}

bool Editor::shouldRenderCursor() const {
    // Android.shouldRenderCursor: on during the first half of each 2*BLINK cycle
    // since mShowCursor, or always when mRenderCursorRegardlessTiming.
    if (!isCursorVisible()) return false;
    if (mRenderCursorRegardlessTiming) return true;
    const int64_t showCursorDelta = SystemClock::uptimeMillis() - mShowCursor;
    return (showCursorDelta % (2 * BLINK)) < BLINK;
}

bool Editor::isBlinking() const {
    // Android.isBlinking: mBlink != null && !mBlink.mCancelled.
    return !mBlinkCancelled;
}

void Editor::makeBlink() {
    // Android.makeBlink: stamp the blink start time, (un)cancel, then schedule (or
    // drop) the Blink runnable.
    if (shouldBlink()) {
        mShowCursor = SystemClock::uptimeMillis();
        mBlinkCancelled = false;                 // mBlink.uncancel()
        mTextView->removeCallbacks(mBlink);
        mTextView->postDelayed(mBlink, BLINK);
    } else {
        mTextView->removeCallbacks(mBlink);
    }
}

void Editor::suspendBlink() {
    // Android.suspendBlink == mBlink.cancel(): remove the pending tick + mark cancelled.
    if (!mBlinkCancelled) {
        if (mTextView) mTextView->removeCallbacks(mBlink);
        mBlinkCancelled = true;
    }
}

void Editor::resumeBlink() {
    // Android.resumeBlink: mBlink.uncancel() then makeBlink().
    mBlinkCancelled = false;
    makeBlink();
}

void Editor::loadCursorDrawable() {
    // Android.loadCursorDrawable: lazily fetch the host's cursor drawable once.
    if (mDrawableForCursor == nullptr) {
        mDrawableForCursor = mTextView->getTextCursorDrawable();
    }
}

Drawable* Editor::getCursorDrawable() const {
    // Android.getCursorDrawable.
    return mDrawableForCursor;
}

int Editor::clampHorizontalPosition(Drawable* drawable, float horizontal) {
    // Android.clampHorizontalPosition, verbatim. NOTE: CDROID Rect is {left, top,
    // width, height} and Drawable::getPadding stores right/bottom padding in the
    // width/height fields (so mTempRect.width==Android.right, .height==.bottom).
    horizontal = std::max(0.5f, horizontal - 0.5f);

    int drawableWidth = 0;
    if (drawable != nullptr) {
        drawable->getPadding(mTempRect);
        drawableWidth = drawable->getIntrinsicWidth();
    } else {
        mTempRect.setEmpty();
    }

    const int scrollX = mTextView->getScrollX();
    const float horizontalDiff = horizontal - scrollX;
    const int viewClippedWidth = mTextView->getWidth()
            - mTextView->getCompoundPaddingLeft() - mTextView->getCompoundPaddingRight();

    int left;
    if (horizontalDiff >= (viewClippedWidth - 1.f)) {
        // at the rightmost position
        left = viewClippedWidth + scrollX - (drawableWidth - mTempRect.width);
    } else if (std::abs(horizontalDiff) <= 1.f
            || (mTextView->length() == 0   // TextUtils.isEmpty(mTextView.getText())
                && (TextView::VERY_WIDE - scrollX) <= (viewClippedWidth + 1.f)
                && horizontal <= 1.f)) {
        // at the leftmost position
        left = scrollX - mTempRect.left;
    } else {
        left = (int) horizontal - mTempRect.left;
    }
    return left;
}

void Editor::updateCursorPosition(int top, int bottom, float horizontal) {
    // Android.updateCursorPosition(top, bottom, horizontal): set the cursor
    // drawable's bounds, honoring its padding and clamping to the view edges.
    // Android setBounds(left, top, right, bottom); CDROID Drawable::setBounds is
    // (x, y, w, h) — so w = right - left (= intrinsic width), h as derived below.
    loadCursorDrawable();
    const int left = clampHorizontalPosition(mDrawableForCursor, horizontal);
    const int x = left;
    const int y = top - mTempRect.top;
    const int w = mDrawableForCursor->getIntrinsicWidth();           // (left + w) - left
    const int h = (bottom + mTempRect.height) - y;                   // bottom + pad.bottom - y
    mDrawableForCursor->setBounds(x, y, w, h);
}

void Editor::updateCursorPosition() {
    // Android.updateCursorPosition (no-arg): load the drawable, then place it at
    // the caret. CDROID draws the cursor in view space (the canvas is NOT
    // pre-translated to the text origin at this point, unlike Android), so the
    // drawable bounds are placed in text-local coords and mirrored into view space
    // (mCaretRect) for invalidation; drawCursor applies the text-origin translate.
    loadCursorDrawable();

    Layout* layout = mTextView->getLayout();
    if (layout == nullptr || !isEditing()) {
        mCaretRect.setEmpty();
        return;
    }
    const int offset = cursorOffset();
    const int line = layout->getLineForOffset(offset);
    const int top = layout->getLineTop(line);
    const int bottom = layout->getLineBottomWithoutSpacing(line);   // includeLineSpacing=false
    const bool clamped = layout->shouldClampCursor(line);
    const float horizontal = layout->getPrimaryHorizontal(offset, clamped);

    // View-space origin offset (compound padding + vertical offset) — applied by
    // drawCursor via canvas.translate for the drawable, and baked into mCaretRect
    // for the onDrawCaret fallback / invalidation.
    const int dx = mTextView->getCompoundPaddingLeft() + mTextView->getHorizontalOffsetForDrawables();
    const int dy = mTextView->getExtendedPaddingTop() + mTextView->getVerticalOffset(true);

    if (mDrawableForCursor != nullptr) {
        updateCursorPosition(top, bottom, horizontal);   // text-local drawable bounds
        const Rect& b = mDrawableForCursor->getBounds();
        mCaretRect.set(b.left + dx, b.top + dy, b.width, b.height);
    } else {
        // No drawable: fixed-thickness caret rect for TextView::onDrawCaret.
        constexpr int caretThickness = 2;
        mCaretRect.set(dx + (int) horizontal, dy + top, caretThickness, bottom - top);
    }
}

void Editor::invalidateCursorPath() {
    // Android.Editor delegates to TextView.invalidateCursorPath() (the Blink
    // runnable invalidates the caret region through this on each half-cycle).
    mTextView->invalidateCursorPath();
}

void Editor::invalidateCursor() {
    updateCursorPosition();
    if (mCaretRect.empty()) mTextView->invalidate(true);
    else mTextView->invalidate(mCaretRect);
}

void Editor::invalidateTextDisplayList() {
    mTextView->invalidate(true);
}

void Editor::drawCursor(Canvas& canvas, int cursorOffsetVertical) {
    // Mirrors the gate in Android.TextView.onDraw / Editor.onDraw: only the caret
    // (zero-width selection) is drawn here; a real selection is a highlight path.
    const bool translate = cursorOffsetVertical != 0;
    if (translate) canvas.translate(0, cursorOffsetVertical);
    if (mDrawableForCursor != nullptr) {
        mDrawableForCursor->draw(canvas);
    }
    if (translate) canvas.translate(0, -cursorOffsetVertical);

    // Back-compat fallback (CDROID's own wheel): no cursor drawable, so paint the
    // caret via TextView::onDrawCaret — kept so subclasses can still customize it.
    if (mCaretRect.empty()) return;
    mTextView->onDrawCaret(canvas, mCaretRect);
}

// =====================================================================================
//  Selection
// =====================================================================================
void Editor::setSelection(int index) {
    Spannable* e = editable();
    if (!e) return;
    Selection::setSelection(e, index);
    mTextView->setCaretPos(index);   // keep legacy caret state in sync
}

void Editor::setSelection(int start, int stop) {
    Spannable* e = editable();
    if (!e) return;
    Selection::setSelection(e, start, stop);
    mTextView->setCaretPos(stop);
}

void Editor::selectAll() {
    Spannable* e = editable();
    if (!e) return;
    Selection::selectAll(e);
    mTextView->setCaretPos(mTextView->length());
}

void Editor::extendSelection(int index) {
    Spannable* e = editable();
    if (!e) return;
    Selection::extendSelection(e, index);
    mTextView->setCaretPos(index);
}

// =====================================================================================
//  Text-change hook
// =====================================================================================
void Editor::sendOnTextChanged(int /*start*/, int /*before*/, int /*after*/) {
    // The buffer changed: refresh cursor geometry, keep blinking, and let the
    // (deferred) controllers know conditions may have changed.
    invalidateTextDisplayList();
    updateCursorPosition();
    makeBlink();
    prepareCursorControllers();
}

// =====================================================================================
//  Editing input — key handling (ported from EditText::onKeyDown)
// =====================================================================================
bool Editor::onKeyDown(int keyCode, KeyEvent& event) {
    Editable* editable = mTextView->getEditableText();
    if (editable == nullptr) return false;

    // Delegate navigation keys (arrows/page/home/end + ctrl/alt/shift modifiers)
    // to the movement method — Android's TextView.onKeyDown → mMovement.onKeyDown.
    // Editing keys (BACKSPACE/DEL/ENTER/typing) fall through to the switch below.
    if (MovementMethod* mm = mTextView->getMovementMethod()) {
        Spannable* sp = editable;   // Editable is-a Spannable; the local pointer
                                    // upcasts directly (avoids the shadowed editable()).
        if (sp != nullptr && mm->onKeyDown(*mTextView, *sp, keyCode, event)) {
            mTextView->invalidate(true);
            return true;
        }
    }

    Layout* layout = mTextView->getLayout();
    const int len = (int)editable->length();
    // Read the caret/selection from the Spannable (Selection spans) — the same
    // source the caret is rendered from. The old code read the legacy
    // getCaretPos() field, which drifted out of sync and made backspace delete
    // from the wrong end.
    int selStart = Selection::getSelectionStart(editable);
    int selEnd = Selection::getSelectionEnd(editable);
    if (selStart < 0) selStart = 0;
    if (selEnd < 0) selEnd = len;
    const int caret = selEnd;                     // == start when there is no selection
    const int lo = std::min(selStart, selEnd);
    const int hi = std::max(selStart, selEnd);
    const bool hasSelection = (selStart != selEnd);
    const int line = layout ? layout->getLineForOffset(caret) : 0;
    bool handled = false;

    switch (keyCode) {
    case KeyEvent::KEYCODE_DPAD_LEFT:
        if (hasSelection) setSelection(lo);        // collapse a selection to its start
        else if (caret > 0) setSelection(caret - 1);
        handled = true;
        break;
    case KeyEvent::KEYCODE_DPAD_RIGHT:
        if (hasSelection) setSelection(hi);        // collapse a selection to its end
        else if (caret < len) setSelection(caret + 1);
        handled = true;
        break;
    case KeyEvent::KEYCODE_DPAD_DOWN:
        handled = (!mTextView->isSingleLine()) && mTextView->moveCaret2Line(line + 1);
        break;
    case KeyEvent::KEYCODE_DPAD_UP:
        handled = (!mTextView->isSingleLine()) && mTextView->moveCaret2Line(line - 1);
        break;
    case KeyEvent::KEYCODE_BACKSPACE:
        if (hasSelection) {                        // delete the whole selection
            editable->Delete(lo, hi);
            setSelection(lo);
            handled = true;
        } else if (caret > 0 && caret <= len) {
            editable->Delete(caret - 1, caret);
            setSelection(caret - 1);
            handled = true;
        }
        break;
    case KeyEvent::KEYCODE_DEL:
        if (hasSelection) {                        // delete the whole selection
            editable->Delete(lo, hi);
            setSelection(lo);
            handled = true;
        } else if (caret < len) {
            editable->Delete(caret, caret + 1);
            handled = true;
        }
        break;
    case KeyEvent::KEYCODE_ENTER:
        if (!mTextView->isSingleLine()) {
            editable->append((char16_t)'\n');
            handled = true;
        }
        break;
    default: {
        // Key->char via the InputMethodManager (this is NOT an InputConnection).
        const wchar_t ch = InputMethodManager::getInstance().getCharacter(keyCode, event.getMetaState());
        if (ch != 0) {
            const int where = insertPosition();   // insert at caret (replaces any selection)
            editable->insert(where, SpannableStringBuilder(std::u16string(1, (char16_t)ch)));
            setSelection(where + 1);
            handled = true;
        }
        break;
    }
    }

    if (handled) {
        makeBlink();
        mTextView->invalidate(true);
    }
    return handled;
}

int Editor::commitText(const std::wstring& text) {
    Editable* editable = mTextView->getEditableText();
    if (editable == nullptr) return 0;
    // Insert at the caret (replacing any active selection) and advance the caret
    // past the inserted text.
    std::u16string u16;
    for (wchar_t ch : text) u16.push_back((char16_t)ch);
    const int where = insertPosition();
    editable->insert(where, SpannableStringBuilder(u16));
    setSelection(where + (int)u16.size());
    makeBlink();
    mTextView->invalidate(true);
    return (int)u16.size();
}

bool Editor::onTouchEvent(MotionEvent& event) {
    if (mTextView->getLayout() == nullptr) return false;

    const int action = event.getActionMasked();
    const float x = event.getX();
    const float y = event.getY();

    // Reuse TextView::getOffsetForPosition (Android public API) — single source
    // of truth for view-local (x,y) → buffer offset. It accounts for padding + scroll.
    const int offset = mTextView->getOffsetForPosition(x, y);

    if (action == MotionEvent::ACTION_DOWN) {
        mLastTouchOffset = offset;
        // Multi-tap: consecutive taps landing within DOUBLE_TAP_TIMEOUT +
        // DOUBLE_TAP_SLOP of the previous UP extend the sequence. Mirrors
        // Android's SelectionModifierCursorController tap-counting.
        const int64_t now = (int64_t)event.getEventTime();
        // event.getEventTime() is MICROSECONDS (SystemClock::uptimeMicros — see
        // inputdevice.cc); getDoubleTapTimeout() is milliseconds, so convert ms→µs.
        // (The previous *1000000 assumed nanoseconds → a 300-SECOND window, which
        // made every tap count as consecutive and broke double/triple-tap.)
        const int64_t timeoutUs = (int64_t)ViewConfiguration::getDoubleTapTimeout() * 1000LL;
        const float dx = x - mLastUpX, dy = y - mLastUpY;
        const float slop = (float)ViewConfiguration::getDoubleTapSlop();
        const bool inMultiTapWindow = mLastUpTime != 0
                && (now - mLastUpTime) <= timeoutUs
                && (dx * dx + dy * dy) <= slop * slop;
        mTapCount = inMultiTapWindow ? mTapCount + 1 : 1;
        if (mTapCount > 3) mTapCount = 1;   // a 4th tap cycles back to a single tap

        if (mTapCount == 2) {
            selectCurrentWord();            // double-tap → select word
        } else if (mTapCount >= 3) {
            selectAll();                    // triple-tap → select all
        } else {
            setSelection(offset);           // single tap → place caret
        }
        makeBlink();
    } else if (action == MotionEvent::ACTION_MOVE) {
        extendSelection(offset);   // drag to extend the selection
    }
    return true;
}

void Editor::onTouchUpEvent(MotionEvent& event) {
    // Record this tap (time + position) so the next ACTION_DOWN can recognize a
    // double-tap (UP→DOWN within DOUBLE_TAP_TIMEOUT + DOUBLE_TAP_SLOP). The
    // drag-handle / action-mode teardown Android also does here arrives with the
    // handles / action-mode passes.
    mLastUpTime = (int64_t)event.getEventTime();
    mLastUpX = event.getX();
    mLastUpY = event.getY();
}

bool Editor::selectCurrentWord() {
    Spannable* e = editable();
    if (e == nullptr) return false;

    const int offset = mLastTouchOffset;
    const int len = mTextView->length();
    if (offset < 0 || offset > len) return false;

    // Port of android.widget.Editor.selectCurrentWord(): find the word boundaries
    // around the touch offset via WordIterator, then set the selection. Deferred
    // (not in this foundation): URLSpan selection and the
    // needsToSelectAllToSelectWordOrParagraph / getCharClusterRange fallbacks.
    WordIterator wordIterator;
    wordIterator.setCharSequence(e, offset, offset);
    const int selStart = wordIterator.getBeginning(offset);
    const int selEnd = wordIterator.getEnd(offset);

    if (selStart == WordIterator::DONE || selEnd == WordIterator::DONE || selStart == selEnd) {
        return false;   // not inside a word (whitespace/edge) — nothing to select
    }

    Selection::setSelection(e, selStart, selEnd);
    mTextView->setCaretPos(selEnd);
    mTextView->invalidate(true);
    return true;
}

// =====================================================================================
//  Android public API forwarded from TextView
// =====================================================================================
void Editor::setCursorVisible(bool visible) {
    if (mCursorVisible == visible) return;
    mCursorVisible = visible;
    if (visible) {
        makeBlink();
    } else {
        if (mTextView) mTextView->removeCallbacks(mBlink);
        invalidateCursor();
    }
}

void Editor::setShowSoftInputOnFocus(bool show) {
    mShowSoftInputOnFocus = show;
}

void Editor::setSelectAllOnFocus(bool selectAll) {
    mSelectAllOnFocus = selectAll;
}

void Editor::beginBatchEdit() {
    // Android nests on mInputMethodState.mBatchEditNesting and, on the outermost
    // batch, defers IME updates. CDROID has no InputConnection yet, so just keep
    // the nesting count (faithful skeleton for the future IME pass).
    mBatchEditNesting++;
}

void Editor::endBatchEdit() {
    if (mBatchEditNesting > 0) mBatchEditNesting--;
    // On the outermost close, Android notifies the IME; nothing to do without one.
}

bool Editor::isSuggestionsEnabled() const {
    // Faithful port of the non-spell-check preconditions in Editor.isSuggestionsEnabled():
    // suggestions require an editable, cursor-visible, non-password field. (The actual
    // spell-check / SuggestionSpan machinery is still deferred.)
    return isEditing() && mCursorVisible && !mTextView->hasPasswordTransformationMethod();
}

// =====================================================================================
//  Cursor controllers — deferred to the handles pass (intentional no-ops).
// =====================================================================================
void Editor::prepareCursorControllers() {
    // Insertion/selection HandleView controllers land in a follow-up pass.
}

void Editor::hideCursorControllers() {
    // No controllers exist yet in the foundation.
}

}  // namespace cdroid
