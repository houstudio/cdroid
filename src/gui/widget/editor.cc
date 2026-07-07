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
#include <text/method/worditerator.h>
#include <text/method/offsetmapping.h>
#include <text/method/movementmethod.h>
#include <text/selection.h>
#include <text/editable.h>
#include <text/spannablestringbuilder.h>
#include <text/layout.h>
#include <text/parcelablespan.h>
#include <view/keyevent.h>
#include <view/motionevent.h>
#include <view/view.h>
#include <view/viewconfiguration.h>
#include <core/inputdevice.h>
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
            LOGV("%p:%d blink cursor",mTextView,mTextView->getId());
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
        if (selEnd >= 0) return selEnd;   // bare caret — insert at its offset
    }
    return 0;   // no selection yet — caret at start
}

void Editor::replace() {
    // Android.Editor.replace() hides any active cursor/span controllers and
    // shows the suggestions popup for replacement. CDROID has no suggestions
    // popup yet, so preserve the shadow semantics by hiding controllers.
    hideCursorControllers();

    int middle = (mTextView->getSelectionStart() + mTextView->getSelectionEnd()) / 2;
    if (Spannable* e = editable()) Selection::setSelection(e, middle);
}
// =====================================================================================
//  Lifecycle
// =====================================================================================
void Editor::onAttachedToWindow() {
    // Android attaches controller listeners and spell-check helpers here.
    // CDROID does not yet implement those systems. Resume blinking the cursor
    // when the view is reattached, as Android does.
    if(mTextView->hasSelection()){
        //refreshTextActionMode();
    }
    mShowCursor = SystemClock::uptimeMillis();
    resumeBlink();
}

void Editor::onDetachedFromWindow() {
    // Android detaches controllers, hides popups, and cancels pending IME
    // callbacks. CDROID suspends the blink state and hides cursor controllers.
    suspendBlink();
    hideCursorControllers();
}

void Editor::onFocusChanged(bool focused, int /*direction*/, Rect* /*previouslyFocusedRect*/) {
    // Android.Editor.onFocusChanged preserves the last tap position, invokes
    // MovementMethod.onTakeFocus, and updates selection/highlight state.
    mShowCursor = SystemClock::uptimeMillis();

    if (focused) {
        Spannable* e = editable();
        const int selStart = mTextView->getSelectionStart();
        const int selEnd = mTextView->getSelectionEnd();
        // mSelectAllOnFocus lives on TextView now; Editor reaches it via friendship.
        const bool isFocusHighlighted = mSelectAllOnFocus && selStart == 0
                && selEnd == mTextView->length();

        mCreatedWithASelection = mFrozenWithFocus && mTextView->hasSelection()
                && !isFocusHighlighted;

        if (!mFrozenWithFocus || selStart < 0 || selEnd < 0) {
            const int lastTapPosition = getLastTapPosition();
            if (lastTapPosition >= 0 && e) {
                Selection::setSelection(e, lastTapPosition);
            }

            if (MovementMethod* mm = mTextView->getMovementMethod()) {
                if (e) mm->onTakeFocus(*mTextView, *e, /*direction*/ 0);
            }

            if (mSelectAllOnFocus) {
                mTextView->selectAllText();
            }

            mTouchFocusSelected = true;
        }

        mFrozenWithFocus = false;
        mSelectionMoved = false;
        mCursorVisible = true;
        resumeBlink();
    } else {
        mTextView->endBatchEdit();
        hideCursorControllers();
        suspendBlink();
        ensureNoSelectionIfNonSelectable();
        // Android also cancels selection/action-mode on focus loss. That is
        // still deferred in CDROID's current foundation pass.
    }
}

void Editor::onWindowFocusChanged(bool hasWindowFocus) {
    if (hasWindowFocus) {
        resumeBlink();
        // Android refreshes text action mode when the window returns focus.
        // CDROID defers action mode state until the selection/handle pass.
    } else {
        suspendBlink();
        hideCursorControllers();
        ensureNoSelectionIfNonSelectable();
        mTextView->endBatchEdit();
        // Android hides popups and controllers before parent focus loss.
    }
}

void Editor::ensureNoSelectionIfNonSelectable() {
    if (!mTextView->canSelectText() && mTextView->hasSelection()) {
        const int len = mTextView->length();
        Spannable* e = editable();
        if (e) Selection::setSelection(e, len, len);
    }
}

// =====================================================================================
//  Cursor blink + geometry / draw  (faithful ports of android.widget.Editor)
//  (isCursorVisible is inline in editor.h: mCursorVisible && isEditing().)
// =====================================================================================
bool Editor::shouldBlink() const {
    // Android.shouldBlink: visible, focused, the window is visible, and the
    // selection is a zero-length caret.
    if (!isCursorVisible() || !mTextView->isFocused()) return false;

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


void Editor::makeBlink() {
    // Android.makeBlink: stamp the blink start time, (un)cancel, then schedule (or
    // drop) the Blink runnable.
    if (shouldBlink()) {
        if(mTextView->getLayout()!=nullptr){
            mTextView->invalidateCursorPath();
        }
        mShowCursor = SystemClock::uptimeMillis();
        mBlinkCancelled = false;
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
            || (TextUtils::isEmpty(mTextView->mText)
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
    loadCursorDrawable();
    const int left = clampHorizontalPosition(mDrawableForCursor, horizontal);
    const int width = mDrawableForCursor->getIntrinsicWidth();
    const int y = top - mTempRect.top;
    const int h = (bottom + mTempRect.height) - y;
    mDrawableForCursor->setBounds(left, top-mTempRect.top, (width<0?2:width), h);
}

void Editor::updateCursorPosition() {
    loadCursorDrawable();
    if(mDrawableForCursor==nullptr){
        return ;
    }
    Layout* layout = mTextView->getLayout();
    // Ported from Android Editor.updateCursorPosition() (Editor.java:2428): use
    // selection START and map through OffsetMapping with the CURSOR strategy so a
    // length-altering transformation (e.g. password dots) positions the caret right.
    const int offset = mTextView->getSelectionStart();
    const int transformedOffset = mTextView->originalToTransformed(offset,
            OffsetMapping::MAP_STRATEGY_CURSOR);
    const int line = layout->getLineForOffset(transformedOffset);
    const int top = layout->getLineTop(line);
    const int bottom = layout->getLineBottomWithoutSpacing(line);   // includeLineSpacing=false
    const bool clamped = layout->shouldClampCursor(line);
    const float horizontal = layout->getPrimaryHorizontal(transformedOffset, clamped);

    updateCursorPosition(top,bottom,horizontal);
}

void Editor::invalidateCursorPath() {
    // Android.Editor delegates to TextView.invalidateCursorPath() (the Blink
    // runnable invalidates the caret region through this on each half-cycle).
    mTextView->invalidateCursorPath();
}

void Editor::invalidateCursor() {
    updateCursorPosition();
    // mCaretRect is gone; invalidate the whole view (the cursor drawable moved).
    // TODO(android): invalidate just the union of the old+new cursor rect, as Android
    //                does via the cursor Rect it tracks.
    mTextView->invalidate(true);
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
    // NOTE: do NOT call TextView::onDrawCaret here. That legacy path re-applies
    // mCaretRect bounds to the SAME drawable object (mDrawableForCursor aliases
    // TextView::mCursorDrawable) and redraws it — which fills the whole edit box
    // with the cursor color. Android's Editor.drawCursor draws the cursor once.
}

// =====================================================================================
//  Selection
// =====================================================================================
//  Selection — Android EditText owns setSelection/selectAll/extendSelection as
//  Selection:: convenience wrappers; Editor calls Selection:: directly.
// =====================================================================================

// =====================================================================================
//  Text-change hook
// =====================================================================================
void Editor::sendOnTextChanged(int /*start*/, int /*before*/, int /*after*/) {
    // The buffer changed: refresh cursor geometry, reset blink timing, and hide
    // active cursor controllers. Android also updates spell-check spans and
    // selection action mode helpers here; those subsystems are still deferred.
    invalidateTextDisplayList();
    //updateCursorPosition();
    makeBlink();
    hideCursorControllers();
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
            // The movement method updated the Selection spans but did NOT touch the
            // cursor drawable's bounds — reposition it before invalidating, otherwise
            // the caret is redrawn at its stale offset. makeBlink() also re-stamps
            // mShowCursor so shouldRenderCursor() is back in its ON phase; without it
            // the caret draws against a stale blink start time and intermittently
            // disappears (it only comes back once the cycle randomly lands ON again).
            makeBlink();
            invalidateCursor();
            return true;
        }
    }

    const int len = (int)editable->length();
    (void)len;

    // ENTER is handled here, not in the KeyListener: Android routes it through
    // TextView (onEditorAction / newline insertion gated on single-line), and the
    // QwertyKeyListener would otherwise insert '\n' unconditionally. Insert at the
    // caret (replacing any selection) for multi-line editors only.
    if (keyCode == KeyEvent::KEYCODE_ENTER) {
        if (!mTextView->isSingleLine()) {
            const int where = insertPosition();
            editable->insert(where, SpannedString(std::u16string(1, u'\n')));
            Selection::setSelection(editable, where + 1);
            makeBlink();
            invalidateCursor();
            return true;
        }
        return false;   // single-line: ENTER not handled (focus advance not ported)
    }

    // Editing keys (BACKSPACE/DEL delete + character typing) are routed through the
    // ported Android KeyListener stack (TextKeyListener → QwertyKeyListener /
    // DigitsKeyListener → BaseKeyListener → MetaKeyKeyListener) in
    // src/gui/text/method/. This replaces the former hand-written switch that stood
    // in for the KeyListener role. Selection/caret bookkeeping lives inside the
    // listener chain (and Selection spans), same as Android.
    if (KeyListener* kl = mTextView->getKeyListener()) {
        if (kl->onKeyDown(*mTextView, *editable, keyCode, event)) {
            makeBlink();
            // Reposition the cursor drawable to the new Selection, then invalidate.
            invalidateCursor();
            return true;
        }
    }

    return false;
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
    Selection::setSelection(editable, where + (int)u16.size());
    makeBlink();
    mTextView->invalidate(true);
    return (int)u16.size();
}

bool Editor::onTouchEvent(MotionEvent& event) {
    if (mTextView->getLayout() == nullptr) return false;

    const bool filterOutEvent = shouldFilterOutTouchEvent(event);
    mLastButtonState = event.getButtonState();
    if (filterOutEvent) {
        if (event.getActionMasked() == MotionEvent::ACTION_UP) {
            mIgnoreActionUpEvent = true;
        }
        return false;
    }

    const int action = event.getActionMasked();
    const float x = event.getX();
    const float y = event.getY();
    const int offset = mTextView->getOffsetForPosition(x, y);

    // Android's Editor.onTouchEvent also dispatches to the insertion/selection
    // controllers; CDROID has no handle controllers yet, so this is a simplified
    // gesture port that retains caret placement and drag selection.
    if (action == MotionEvent::ACTION_DOWN) {
        mTouchFocusSelected = false;
        mIgnoreActionUpEvent = false;
        mLastButtonState = event.getButtonState();
        mLastTouchOffset = offset;

        const int64_t now = (int64_t)event.getEventTime();
        const int64_t timeoutMS = (int64_t)ViewConfiguration::getDoubleTapTimeout();
        const float dx = x - mLastUpX, dy = y - mLastUpY;
        const float slop = (float)ViewConfiguration::getDoubleTapSlop();
        const bool inMultiTapWindow = mLastUpTime != 0
                && (now - mLastUpTime) <= timeoutMS
                && (dx * dx + dy * dy) <= slop * slop;
        mTapCount = inMultiTapWindow ? mTapCount + 1 : 1;
        if (mTapCount > 3) mTapCount = 1;
        if (mTapCount == 2) {
            selectCurrentWord();
        } else if (mTapCount >= 3) {
            Selection::selectAll(editable());
        } else {LOGD("selection from %d",offset);
            Selection::setSelection(editable(), offset);
        }
        makeBlink();
    } else if (action == MotionEvent::ACTION_MOVE) {
        Selection::extendSelection(editable(), offset);
    }
    return true;
}

bool Editor::shouldFilterOutTouchEvent(MotionEvent& event) const {
    if (!event.isFromSource(InputDevice::SOURCE_MOUSE)) {
        return false;
    }
    const bool primaryButtonStateChanged =
            ((mLastButtonState ^ event.getButtonState()) & MotionEvent::BUTTON_PRIMARY) != 0;
    const int action = event.getActionMasked();
    if ((action == MotionEvent::ACTION_DOWN || action == MotionEvent::ACTION_UP)
            && !primaryButtonStateChanged) {
        return true;
    }
    if (action == MotionEvent::ACTION_MOVE && !event.isButtonPressed(MotionEvent::BUTTON_PRIMARY)) {
        return true;
    }
    return false;
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

int Editor::getLastTapPosition() const {
    if (mLastTouchOffset >= 0 && mTextView && mLastTouchOffset <= mTextView->length()) {
        return mLastTouchOffset;
    }
    return -1;
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

    LOGD("selStart=%d selEnd=%d",selStart,selEnd);
    if (selStart == WordIterator::DONE || selEnd == WordIterator::DONE || selStart == selEnd) {
        return false; // not inside a word (whitespace/edge) — nothing to select
    }

    Selection::setSelection(e, selStart, selEnd);
    // Cursor update is driven by the Selection span change via spanChange.
    mTextView->invalidate(true);
    return true;
}

void Editor::onDraw(Canvas& canvas, Layout* layout, Path* highlight, Paint& highlightPaint, int cursorOffsetVertical){
    const int selectionStart = mTextView->getSelectionStart();
    const int selectionEnd = mTextView->getSelectionEnd();

    /*const InputMethodState ims = mInputMethodState;
    if (ims != null && ims.mBatchEditNesting == 0) {
        InputMethodManager imm = getInputMethodManager();
        if (imm != null) {
            if (imm.isActive(mTextView)) {
                if (ims.mContentChanged || ims.mSelectionModeChanged) {
                    // We are in extract mode and the content has changed
                    // in some way... just report complete new text to the
                    // input method.
                    reportExtractedText();
                }
            }
        }
    }

    if (mCorrectionHighlighter != null) {
        mCorrectionHighlighter->draw(canvas, cursorOffsetVertical);
    }*/

    if (highlight != nullptr && selectionStart == selectionEnd && mDrawableForCursor != nullptr) {
        drawCursor(canvas, cursorOffsetVertical);
        // Rely on the drawable entirely, do not draw the cursor line.
        // Has to be done after the IMM related code above which relies on the highlight.
        highlight = nullptr;
    }
    /*if (mSelectionActionModeHelper != nullptr) {
        mSelectionActionModeHelper.onDraw(canvas);
        if (mSelectionActionModeHelper.isDrawingHighlight()) {
            highlight = nullptr;
        }
    }*/
    /*if (mTextView.canHaveDisplayList() && canvas.isHardwareAccelerated()) {
        drawHardwareAccelerated(canvas, layout, highlight, highlightPaint,cursorOffsetVertical);
    } else */{
        layout->draw(canvas, highlight, &highlightPaint, cursorOffsetVertical);
    }
}
// =====================================================================================
//  Android public API forwarded from TextView
// =====================================================================================
bool Editor::isCursorVisible() const{
    return mCursorVisible&&mTextView->isTextEditable();
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

// =====================================================================================
//  Cursor controllers — deferred to the handles pass (intentional no-ops).
// =====================================================================================
void Editor::prepareCursorControllers() {
    // Android Editor builds and updates insertion/selection handle controllers
    // here. CDROID defers the actual controller implementation to a later pass,
    // but we can still keep the enable/disable state aligned with the text view
    // and cursor visibility.
    const bool enabled = mTextView->getLayout() != nullptr;
    mInsertionControllerEnabled = enabled && isCursorVisible();
    mSelectionControllerEnabled = enabled && mTextView->canSelectText();

    if (!mInsertionControllerEnabled || !mSelectionControllerEnabled) {
        hideCursorControllers();
    }
}

void Editor::hideCursorControllers() {
    // Android Editor hides any active cursor controllers when focus changes or
    // text mutations occur. No controller implementation is available yet, but
    // the state is still tracked.
    mInsertionControllerEnabled = false;
    mSelectionControllerEnabled = false;
}
void Editor::hideCursorAndSpanControllers() {
    // Android Editor hides any active cursor controllers and span controllers
    // when focus changes or text mutations occur. No controller implementation is
    // available yet, but the state is still tracked.
    mInsertionControllerEnabled = false;
    mSelectionControllerEnabled = false;
    //mSpanControllerEnabled = false;
}
}  // namespace cdroid
