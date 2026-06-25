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
#include <text/selection.h>
#include <text/editable.h>
#include <text/layout.h>
#include <text/parcelablespan.h>
#include <view/keyevent.h>
#include <view/motionevent.h>
#include <view/view.h>
#include <core/inputmethodmanager.h>
#include <core/canvas.h>
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
    // The blink runnable toggles visibility, invalidates the caret region, and
    // reschedules itself while the caret should keep blinking.
    mBlink = [this]() {
        mBlinkVisible = !mBlinkVisible;
        invalidateCursor();
        if (shouldBlink()) mTextView->postDelayed(mBlink, BLINK);
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
//  Cursor blink + geometry / draw
// =====================================================================================
bool Editor::shouldBlink() const {
    if (!mCursorVisible || mBlinkSuspended) return false;
    if (!isEditing()) return false;
    return mTextView->isFocused();
}

void Editor::makeBlink() {
    if (!shouldBlink()) {
        mTextView->removeCallbacks(mBlink);
        return;
    }
    mBlinkVisible = true;
    mTextView->removeCallbacks(mBlink);   // avoid duplicate scheduling
    mTextView->postDelayed(mBlink, BLINK);
}

void Editor::suspendBlink() {
    mBlinkSuspended = true;
    mBlinkVisible = true;                 // draw solid while typing / briefly
    if (mTextView) mTextView->removeCallbacks(mBlink);
    invalidateCursor();
}

void Editor::resumeBlink() {
    mBlinkSuspended = false;
    makeBlink();
}

void Editor::updateCursorPosition() {
    Layout* layout = mTextView->getLayout();
    if (layout == nullptr || !isEditing()) {
        mCaretRect.setEmpty();
        return;
    }
    const int offset = cursorOffset();
    const int line = layout->getLineForOffset(offset);
    const int top = layout->getLineTop(line);
    const int bottom = layout->getLineBottom(line);
    const float horizontal = layout->getPrimaryHorizontal(offset);

    // Match the coordinate space TextView::onDraw uses for the caret (see the
    // mCaretRect.offset(...) line there): the canvas is pre-scrolled by the
    // framework, so we position in untranslated view space at
    // (compoundPaddingLeft + drawable hoff, extendedPaddingTop + vertical offset).
    // getHorizontalOffsetForDrawables()/getVerticalOffset() are TextView-private
    // (friend access).
    const int hoff = mTextView->getHorizontalOffsetForDrawables();
    const int x = mTextView->getCompoundPaddingLeft() + hoff + (int)horizontal;
    const int y = mTextView->getExtendedPaddingTop() + mTextView->getVerticalOffset(true) + top;

    constexpr int caretThickness = 2;
    mCaretRect.set(x, y, caretThickness, bottom - top);
}

void Editor::invalidateCursor() {
    updateCursorPosition();
    if (mCaretRect.empty()) mTextView->invalidate(true);
    else mTextView->invalidate(mCaretRect);
}

void Editor::invalidateTextDisplayList() {
    mTextView->invalidate(true);
}

void Editor::drawCursor(Canvas& canvas) {
    if (!mCursorVisible || !isEditing() || !mBlinkVisible) return;
    updateCursorPosition();
    if (mCaretRect.empty()) return;
    // Delegate the actual caret painting to TextView's onDrawCaret hook so that
    // subclasses can customize the caret appearance (mirrors Android, where the
    // cursor drawable is drawn here but apps can supply their own).
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

    Layout* layout = mTextView->getLayout();
    const int caret = mTextView->getCaretPos();
    const int line = layout ? layout->getLineForOffset(caret) : 0;
    bool handled = false;

    switch (keyCode) {
    case KeyEvent::KEYCODE_DPAD_LEFT:
        if (caret > 0) { setSelection(caret - 1); handled = true; }
        break;
    case KeyEvent::KEYCODE_DPAD_RIGHT:
        if (caret < (int)editable->length()) { setSelection(caret + 1); handled = true; }
        break;
    case KeyEvent::KEYCODE_DPAD_DOWN:
        handled = (!mTextView->isSingleLine()) && mTextView->moveCaret2Line(line + 1);
        break;
    case KeyEvent::KEYCODE_DPAD_UP:
        handled = (!mTextView->isSingleLine()) && mTextView->moveCaret2Line(line - 1);
        break;
    case KeyEvent::KEYCODE_BACKSPACE:
        if (editable->length() && caret > 0 && caret <= (int)editable->length()) {
            editable->Delete(caret - 1, caret);
            setSelection(caret - 1);
            handled = true;
        } else {
            mTextView->setCaretPos((int)editable->length() - 1);
        }
        break;
    case KeyEvent::KEYCODE_DEL:
        if (caret < (int)editable->length()) {
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
            editable->append((char16_t)ch);   // foundation: append (matches prior EditText behavior)
            mTextView->setCaretPos((int)editable->length());
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
    // Foundation: append at end (preserves prior EditText semantics). A later pass
    // will insert at the caret and advance it.
    for (wchar_t ch : text) editable->append((char16_t)ch);
    mTextView->setCaretPos((int)editable->length());
    makeBlink();
    mTextView->invalidate(true);
    return (int)text.size();
}

bool Editor::onTouchEvent(MotionEvent& event) {
    Layout* layout = mTextView->getLayout();
    if (layout == nullptr) return false;

    const int action = event.getActionMasked();
    const float x = event.getX();
    const float y = event.getY();

    // Convert view coordinates to text-buffer coordinates (inverse of the
    // transform applied in TextView::onDraw). getVerticalOffset() is private
    // to TextView (friend access).
    const int vpad = mTextView->getExtendedPaddingTop() + mTextView->getVerticalOffset(true);
    int line = layout->getLineForVertical((int)y - vpad + mTextView->getScrollY());
    if (line < 0) line = 0;
    if (line >= layout->getLineCount()) line = layout->getLineCount() - 1;

    const float horiz = x - mTextView->getCompoundPaddingLeft() + mTextView->getScrollX();
    const int offset = layout->getOffsetForHorizontal(line, horiz);

    if (action == MotionEvent::ACTION_DOWN) {
        setSelection(offset);
        makeBlink();
    } else if (action == MotionEvent::ACTION_MOVE) {
        extendSelection(offset);   // drag to extend the selection
    }
    return true;
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
