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
#ifndef __EDITOR_H__
#define __EDITOR_H__

#include <string>
#include <core/rect.h>
#include <core/callbackbase.h>   // Runnable

namespace cdroid {

class TextView;
class Spannable;
class Drawable;
class KeyEvent;
class MotionEvent;
class Canvas;

/**
 * Helper class used by TextView to handle editable text views.
 *
 * This is a C++ port of Android's android.widget.Editor. It owns everything
 * related to the editing UX of a TextView: cursor blinking & geometry,
 * selection, the text-change reaction hook, touch/key editing, and (in later
 * passes) the insertion/selection handles and the floating selection action
 * mode.
 *
 * An Editor is NOT a View; it is a plain helper that holds a back-pointer to
 * its owning TextView. TextView is declared `friend class Editor;` in
 * textview.h so Editor can reach the TextView internals it needs (vertical
 * offset, cursor drawable, layout).
 *
 * Scope of this foundation pass: cursor blink/draw, selection (delegated to the
 * existing cdroid::Selection), TextWatcher-reaction hook, touch-to-place-caret,
 * key editing. Deferred: drag handles, floating action mode, suggestions /
 * spell-check, undo/redo, magnifier, text classification.
 */
class Editor {
private:
    friend class TextView;

    class CursorController;
    class TextViewPositionListener;

    Spannable* editable() const;
    bool isEditing() const;
    int cursorOffset() const;
    int insertPosition();

    void updateCursorPosition(int top, int bottom, float horizontal);
    void loadCursorDrawable();
    int clampHorizontalPosition(Drawable* drawable, float horizontal);

    void setCursorVisible(bool visible);
    bool isCursorVisible() const { return mCursorVisible && isEditing(); }
    void setShowSoftInputOnFocus(bool show);
    bool getShowSoftInputOnFocus() const { return mShowSoftInputOnFocus; }
    void setSelectAllOnFocus(bool selectAll);
    bool isSelectAllOnFocus() const { return mSelectAllOnFocus; }
    void setTextIsSelectable(bool selectable);
    bool isTextSelectable() const { return mTextIsSelectable; }
    void beginBatchEdit();
    void endBatchEdit();
    bool isSuggestionsEnabled() const;

    Drawable* mDrawableForCursor = nullptr;
    Drawable* mSelectHandleLeft = nullptr;
    Drawable* mSelectHandleRight = nullptr;
    Drawable* mSelectHandleCenter = nullptr;
    TextView* mTextView;

    Runnable mBlink;
    bool mBlinkCancelled = false;
    bool mCursorVisible = true;
    int64_t mShowCursor = 0;
    bool mRenderCursorRegardlessTiming = false;
    Rect mTempRect;
    bool mIgnoreActionUpEvent = false;
    bool mShowSoftInputOnFocus = true;
    bool mSelectAllOnFocus = false;
    int  mBatchEditNesting = 0;

    bool shouldFilterOutTouchEvent(MotionEvent& event) const;
    int getLastTapPosition() const;
    void ensureNoSelectionIfNonSelectable();

    int   mLastButtonState = 0;
    int   mLastTouchOffset = 0;
    int   mTapCount = 0;
    int64_t mLastUpTime = 0;
    float mLastUpX = -1.f;
    float mLastUpY = -1.f;

    bool mFrozenWithFocus = false;
    bool mCreatedWithASelection = false;
    bool mTouchFocusSelected = false;
    bool mSelectionMoved = false;
    bool mTextIsSelectable = false;

    Rect mCaretRect;
    bool mInsertionControllerEnabled = false;
    bool mSelectionControllerEnabled = false;

public:
    explicit Editor(TextView* textView);
    ~Editor();

    void replace();
    void onAttachedToWindow();
    void onDetachedFromWindow();
    void onFocusChanged(bool focused, int direction, Rect* previouslyFocusedRect);
    void onWindowFocusChanged(bool hasWindowFocus);

    void makeBlink();
    void suspendBlink();
    void resumeBlink();
    bool shouldBlink() const;
    bool shouldRenderCursor() const;
    void updateCursorPosition();
    void invalidateCursorPath();
    void invalidateCursor();
    void invalidateTextDisplayList();
    void drawCursor(Canvas& canvas, int cursorOffsetVertical = 0);
    Drawable* getCursorDrawable() const;
    bool isBlinking() const;

    void setSelection(int index);
    void setSelection(int start, int stop);
    void selectAll();
    void extendSelection(int index);

    void sendOnTextChanged(int start, int before, int after);

    bool onKeyDown(int keyCode, KeyEvent& event);
    bool onTouchEvent(MotionEvent& event);
    void onTouchUpEvent(MotionEvent& event);
    int  commitText(const std::wstring& text);
    bool selectCurrentWord();

    bool ignoreActionUpEvent() const { return mIgnoreActionUpEvent; }

    void prepareCursorControllers();
    void hideCursorControllers();
    void hide() { hideCursorControllers(); }
};

class Editor::CursorController {
public:
    virtual ~CursorController() = default;
    virtual void show() {}
    virtual void hide() {}
    virtual void updatePosition(int x, int y) {}
    virtual bool isActive() const { return false; }
};

class Editor::TextViewPositionListener {
public:
    virtual ~TextViewPositionListener() = default;
    virtual void updatePosition(int xInWindow, int yInWindow,
                                int oldXInWindow, int oldYInWindow,
                                bool preX, bool preY) = 0;
};

}  // namespace cdroid

#endif  // __EDITOR_H__
