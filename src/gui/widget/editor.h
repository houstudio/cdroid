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
#include <text/paint.h>
#include <view/actionmode.h>
#include <widget/textview.h>
#include <text/method/keylistener.h>
namespace cdroid {
class Layout;
class Path;
class TextView;
class Spannable;
class Drawable;
class KeyEvent;
class MotionEvent;
class Canvas;
class TransformationMethod;
class Menu;
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
protected:
    friend class TextView;
    class CursorController;
    class InsertionPointCursorController;
    class TextViewPositionListener;
    class SpanController;
    class InputContentType;
    class TextActionModeCallback;
private:
    Drawable* mDrawableForCursor = nullptr;
    Drawable* mSelectHandleLeft = nullptr;
    Drawable* mSelectHandleRight = nullptr;
    Drawable* mSelectHandleCenter = nullptr;
    TextView* mTextView;
    KeyListener* mKeyListener = nullptr;
    SpanController* mSpanController = nullptr;// owned; attached as a borrowed NoCopySpan span
    InputContentType* mInputContentType =nullptr;
    ActionMode* mTextActionMode=nullptr;
    Runnable mBlink;
    bool mBlinkCancelled = false;
    bool mCursorVisible = true;
    bool mIgnoreActionUpEvent = false;
    bool mRenderCursorRegardlessTiming = false;
    bool mFrozenWithFocus = false;
    bool mCreatedWithASelection = false;
    bool mTouchFocusSelected = false;
    bool mSelectionMoved = false;
    bool mSelectAllOnFocus= false;
    bool mTextIsSelectable= false;
    bool mInsertionControllerEnabled = false;
    bool mSelectionControllerEnabled = false;
    bool mShowSoftInputOnFocus = true;
    int64_t mShowCursor = 0;
    Rect mTempRect;
    int  mBatchEditNesting = 0;

    int  mInputType = 0;   // Android: Editor field (EditorInfo.TYPE_NULL == 0)
    int  mLastButtonState = 0;
    int  mLastTouchOffset = 0;
    int  mTapCount = 0;
    int64_t mLastUpTime = 0;
    float mLastUpX = -1.f;
    float mLastUpY = -1.f;
private:
    Spannable* editable() const;
    int insertPosition();

    void updateCursorPosition(int top, int bottom, float horizontal);
    void loadCursorDrawable();
    int clampHorizontalPosition(Drawable* drawable, float horizontal);

    bool isCursorVisible() const;
    void beginBatchEdit();
    void endBatchEdit();
    bool shouldFilterOutTouchEvent(MotionEvent& event) const;
    int getLastTapPosition() const;
    void ensureNoSelectionIfNonSelectable();
    void adjustInputType(bool password, bool passwordInputType,
        bool webPasswordInputType, bool numberPasswordInputType);
    void createInputContentTypeIfNeeded();
protected:
    ActionMode*getTextActionMode()const{return mTextActionMode;}
    void stopTextActionMode();
    // Text selection ActionMode (对齐 AOSP Editor.startSelectionActionMode /
    // startInsertionActionMode / startActionModeInternal)。本轮: 不含 Smart Selection、
    // 选择手柄、custom callback。copy/cut/paste 因 clipboard 未移植而留桩。
    bool startSelectionActionMode();
    bool startInsertionActionMode();
    bool startActionModeInternal(int actionMode);
    void populateTextActionModeMenu(Menu& menu, bool hasSelection);
    void getTextActionModeContentRect(Rect& out, bool hasSelection);
    void invalidateTextActionMode();
public:
    explicit Editor(TextView* textView);
    ~Editor();

    void replace();
    /* Mirror of AOSP Editor.setTransformationMethod: forward to the host
     * TextView's setTransformationMethodInternal so an EditText (which owns an
     * Editor) actually applies the transformation. AOSP also has an
     * mInsertModeController branch (handwriting insert mode) -- out of scope for
     * CDROID, so this is the simplified "no controller" path. */
    void setTransformationMethod(TransformationMethod* method);
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
    void sendUpdateSelection();   // Android: InputMethodManager.updateSelection (IME — deferred)
    bool hasInsertionController() const{
        return mInsertionControllerEnabled;
    }
    bool hasSelectionController() const{
        return mSelectionControllerEnabled;
    }
    void drawCursor(Canvas& canvas, int cursorOffsetVertical = 0);
    Drawable* getCursorDrawable() const;

    void sendOnTextChanged(int start, int before, int after);

    bool onKeyDown(int keyCode, KeyEvent& event);
    bool onTouchEvent(MotionEvent& event);
    void onTouchUpEvent(MotionEvent& event);
    int  commitText(const std::wstring& text);
    bool selectCurrentWord();
    bool performLongClick(bool handled);

    bool ignoreActionUpEvent() const { return mIgnoreActionUpEvent; }

    void prepareCursorControllers();
    void hideCursorControllers();
    void hideCursorAndSpanControllers();
    void addSpanWatchers(Spannable& text);
    void stopTextActionModeWithPreservingSelection();
    void onDraw(Canvas& canvas, Layout* layout, Path* highlight, Paint& highlightPaint,int cursorOffsetVertical);
};
class Editor::InputContentType {
public:
    int imeOptions = 0;//EditorInfo.IME_NULL;
    std::string privateImeOptions;
    CharSequence* imeActionLabel;
    int imeActionId;
    //Bundle extras;
    TextView::OnEditorActionListener onEditorActionListener;
    bool enterDown;
    //LocaleList imeHintLocales;
};
class Editor::CursorController {
public:
    virtual ~CursorController() = default;
    virtual void show() {}
    virtual void hide() {}
    virtual void updatePosition(int x, int y) {}
    virtual bool isActive() const { return false; }
};
class Editor::InsertionPointCursorController:public CursorController{
private:
    bool mIsDraggingCursor;
    bool mIsTouchSnappedToHandleDuringDrag;
    int mPrevLineDuringDrag;
private:
    void positionCursorDuringDrag(MotionEvent& event);
    int getLineDuringDrag(MotionEvent& event);
    void startCursorDrag(MotionEvent& event);
    void performCursorDrag(MotionEvent& event);
    void endCursorDrag(MotionEvent& event);
public:
    void onTouchEvent(MotionEvent&);
    void show();
    void hide();
    void onTouchModeChanged(bool isInTouchMode);
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
