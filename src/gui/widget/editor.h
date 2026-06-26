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
public:
    friend class TextView;   // TextView owns an Editor and forwards its Android
                             // public API to these (private) Editor implementations.
    /** Interface implemented by the insertion/selection cursor controllers.
     *  Real implementations arrive with the handles pass; foundation ships no
     *  concrete controller, so the default methods are no-ops. */
    class CursorController {
    public:
        virtual ~CursorController() = default;
        virtual void show() {}
        virtual void hide() {}
        virtual void updatePosition(int x, int y) {}
        virtual bool isActive() const { return false; }
    };

    /** Listener notified when the host TextView's position/scroll in the window
     *  changes. Used by popup windows and (later) the CursorAnchorInfoNotifier. */
    class TextViewPositionListener {
    public:
        virtual ~TextViewPositionListener() = default;
        virtual void updatePosition(int xInWindow, int yInWindow,
                                    int oldXInWindow, int oldYInWindow,
                                    bool preX, bool preY) = 0;
    };

    explicit Editor(TextView* textView);
    ~Editor();

    // ----- lifecycle (wired from TextView hooks in Step B) -----
    void onAttachedToWindow();
    void onDetachedFromWindow();
    void onFocusChanged(bool focused, int direction, Rect* previouslyFocusedRect);
    void onWindowFocusChanged(bool hasWindowFocus);

    // ----- cursor blink + geometry / draw -----
    void makeBlink();
    void suspendBlink();
    void resumeBlink();
    bool shouldBlink() const;
    void updateCursorPosition();        // recompute mCaretRect from selection + Layout
    void invalidateCursor();            // invalidate the caret region on the TextView
    void invalidateTextDisplayList();   // foundation: just invalidate()
    void drawCursor(Canvas& canvas);    // called at the end of TextView::onDraw

    // ----- selection (delegate to cdroid::Selection on the editable) -----
    void setSelection(int index);
    void setSelection(int start, int stop);
    void selectAll();
    void extendSelection(int index);

    // ----- text-change hook (TextView::sendOnTextChanged forwards here) -----
    void sendOnTextChanged(int start, int before, int after);

    // ----- editing input (moved here from EditText in Step B) -----
    bool onKeyDown(int keyCode, KeyEvent& event);
    bool onTouchEvent(MotionEvent& event);   // tap -> caret, drag -> extend selection
    void onTouchUpEvent(MotionEvent& event); // touch-up: place caret, (later) hide handles
    int  commitText(const std::wstring& text);

    /** Selects the word at the last touch offset (double-tap gesture). Faithful
     *  port of android.widget.Editor.selectCurrentWord(), minus the URLSpan and
     *  needsToSelectAllToSelectWordOrParagraph branches (deferred). */
    bool selectCurrentWord();

    // Set while a drag handle consumes the touch stream so the host TextView's
    // ACTION_UP handling (soft-input etc.) is suppressed. Always false until the
    // handles pass lands.
    bool ignoreActionUpEvent() const { return mIgnoreActionUpEvent; }

    // ----- cursor controllers (deferred: no-ops until the handles pass) -----
    void prepareCursorControllers();
    void hideCursorControllers();
    void hide() { hideCursorControllers(); }

private:
    /** @return the TextView's editable as a Spannable, or nullptr when not editable. */
    Spannable* editable() const;
    /** True when the TextView currently has an editable buffer (i.e. is editing). */
    bool isEditing() const;
    /** Current caret offset: selection end if there is a real selection, else caret pos. */
    int cursorOffset() const;
    /** Insert point for new text: deletes any active selection first (so typing
     *  replaces the selection, as in Android), else returns the caret offset. */
    int insertPosition();

    // Internal implementation of TextView's Android public API (TextView is a
    // friend, so it calls these). Kept private so Editor's own public surface
    // doesn't duplicate the TextView-facing API.
    void setCursorVisible(bool visible);
    bool isCursorVisible() const { return mCursorVisible; }
    void setShowSoftInputOnFocus(bool show);          // Editor.mShowSoftInputOnFocus
    bool getShowSoftInputOnFocus() const { return mShowSoftInputOnFocus; }
    void setSelectAllOnFocus(bool selectAll);         // TextView.mSelectAllOnFocus
    bool isSelectAllOnFocus() const { return mSelectAllOnFocus; }
    void beginBatchEdit();
    void endBatchEdit();
    bool isSuggestionsEnabled() const;

    TextView* mTextView;

    // blink — represented as a Runnable posted via View::postDelayed (500ms),
    // toggling mBlinkVisible. A dedicated Blink inner class can replace this later.
    Runnable mBlink;
    bool mBlinkVisible = true;
    bool mBlinkSuspended = false;
    bool mCursorVisible = true;
    bool mIgnoreActionUpEvent = false;  // see ignoreActionUpEvent()
    bool mShowSoftInputOnFocus = true;  // Android: Editor.mShowSoftInputOnFocus
    bool mSelectAllOnFocus = false;     // Android: TextView.mSelectAllOnFocus (Editor hook)
    int  mBatchEditNesting = 0;         // Android: mInputMethodState.mBatchEditNesting

    // Multi-tap selection: the offset of the most recent ACTION_DOWN, plus the
    // time/position of the most recent ACTION_UP so consecutive taps landing
    // within DOUBLE_TAP_TIMEOUT + DOUBLE_TAP_SLOP extend the sequence. Count:
    // 1 = place caret, 2 = select word, 3 = select all (then it cycles).
    int   mLastTouchOffset = 0;
    int   mTapCount = 0;
    int64_t mLastUpTime = 0;            // nsecs; 0 means "no recent tap"
    float mLastUpX = -1.f;
    float mLastUpY = -1.f;

    Rect mCaretRect;                        // caret geometry, recomputed by updateCursorPosition()

    // cursor controllers — concrete instances arrive with the handles pass.
    bool mInsertionControllerEnabled = false;
    bool mSelectionControllerEnabled = false;
};

}  // namespace cdroid

#endif  // __EDITOR_H__
