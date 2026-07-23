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
#include <widget/R.h>
#include <widget/editor.h>
#include <widget/textview.h>
#include <text/method/worditerator.h>
#include <text/method/offsetmapping.h>
#include <text/method/movementmethod.h>
#include <text/selection.h>
#include <text/editable.h>
#include <text/spannablestringbuilder.h>
#include <text/spanwatcher.h>
#include <text/layout.h>
#include <text/inputtype.h>
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
#include <menu/menu.h>
#include <menu/menuitem.h>

namespace cdroid {

namespace {
// Cursor blink period, in milliseconds (matches Android's Editor.BLINK).
constexpr int BLINK = 500;
// ActionMode menu item order (Editor.java:194)
constexpr int ORDER_CUT = 4;
constexpr int ORDER_COPY = 5;
constexpr int ORDER_PASTE = 6;
constexpr int ORDER_SELECT_ALL = 8;
}  // namespace

// =====================================================================================
//  SpanController — port of Android's Editor.SpanController (a SpanWatcher).
//  Defined early so Editor's destructor sees a complete type (it deletes the
//  instance). Attached to the editable buffer over [0, length] by
//  addSpanWatchers. In Android it does two things: (1) on non-intermediate
//  SELECTION_START/END span changes it calls sendUpdateSelection() to push the
//  new selection to the IME; (2) on EasyEditSpan add/remove/change it shows/
//  hides the EasyEditPopupWindow. CDROID has neither an IME connection nor
//  EasyEditSpan/EasyEditPopupWindow, so only the selection→sendUpdateSelection
//  path is wired (and sendUpdateSelection is itself a deferred no-op). The
//  structure is kept for parity and so future IME / EasyEdit work drops in.
// =====================================================================================
class Editor::SpanController : public SpanWatcher {
public:
    explicit SpanController(Editor* editor) : mEditor(editor) {}

    // Android.Editor.SpanController.isNonIntermediateSelectionSpan: a START/END
    // selection marker that is not carrying SPAN_INTERMEDIATE.
    static bool isNonIntermediateSelectionSpan(Spannable& text, const ParcelableSpan* span) {
        const bool isSelection = (span == Selection::SELECTION_START || span == Selection::SELECTION_END);
        const bool intermediate = (text.getSpanFlags(span) & Spanned::SPAN_INTERMEDIATE) != 0;
        return isSelection && !intermediate;
    }

    void onSpanAdded(Spannable& text, const ParcelableSpan* what, int /*start*/, int /*end*/) override {
        if (isNonIntermediateSelectionSpan(text, what)) {
            mEditor->sendUpdateSelection();
        }
        // Android else-branch: EasyEditSpan → build/show EasyEditPopupWindow +
        // schedule a 3s hide. Deferred (no EasyEditSpan / EasyEditPopupWindow).
    }

    void onSpanRemoved(Spannable& text, const ParcelableSpan* what, int /*start*/, int /*end*/) override {
        if (isNonIntermediateSelectionSpan(text, what)) {
            mEditor->sendUpdateSelection();
        }
        // Android else-branch: if the removed span was the popup's EasyEditSpan, hide().
    }

    void onSpanChanged(Spannable& text, const ParcelableSpan* what,
            int /*ostart*/, int /*oend*/, int /*nstart*/, int /*nend*/) override {
        if (isNonIntermediateSelectionSpan(text, what)) {
            mEditor->sendUpdateSelection();
        }
        // Android else-branch: EasyEditSpan moved → sendEasySpanNotification(TEXT_MODIFIED)
        // + removeSpan. Deferred.
    }

    void hide() {
        // Android: dismiss the EasyEditPopupWindow + cancel the hide runnable.
        // No popup in CDROID → intentional no-op.
    }

private:
    Editor* mEditor;
};

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

void Editor::setTransformationMethod(TransformationMethod* method) {
    // AOSP Editor.setTransformationMethod also has an mInsertModeController
    // branch (handwriting "insert mode"). That whole feature is out of scope for
    // CDROID -- it rides on the @hide AI handwriting stack + ReplacementSpan
    // drawing, and we've decided not to pursue it -- so we always take the
    // equivalent of AOSP's "no controller" path: forward straight to the host's
    // setTransformationMethodInternal.
    mTextView->setTransformationMethodInternal(method, /*updateText*/ true);
}

Editor::~Editor() {
    if (mTextView) mTextView->removeCallbacks(mBlink);
    // SpanController is owned by Editor but attached to the editable buffer as a
    // borrowed (NoCopySpan) span. In TextView's destruction order mText (line 252)
    // is destroyed BEFORE mEditor (line 155), so by the time we get here the
    // buffer is already gone and we must NOT touch it (getEditableText() would
    // deref freed memory). That is safe: a NoCopySpan record is never deleted or
    // dereferenced by the Spannable's own cleanup, so the just-freed buffer left
    // a harmless dangling record, and we free the object here.
    delete mSpanController;
    mSpanController = nullptr;
    delete mInputContentType;
    mInputContentType = nullptr;
}

void Editor::stopTextActionMode() {
    if (mTextActionMode != nullptr) {
        mTextActionMode->finish();
        mTextActionMode = nullptr;   // finish() 经 onDestroyActionMode 也会置空, 此处双保险
    }
}

void Editor::stopTextActionModeWithPreservingSelection() {
    if (mTextActionMode != nullptr) {
        mRestartActionModeOnNextRefresh = true;
    }
    mPreserveSelection = true;
    stopTextActionMode();
    mPreserveSelection = false;
}

// =====================================================================================
//  TextActionModeCallback — port of AOSP Editor.TextActionModeCallback (Editor.java:4683).
//  现代 Android 只有一个 callback 类, 用 mode 枚举(SELECTION/INSERTION)区分。
//  CDROID 的 ActionMode::Callback 是含 std::function 的 struct, 故本类继承它、构造时
//  把 5 个 function 填成 lambda。lambda **全部值捕获 Editor\*** (不捕 this): 本对象在
//  startActionModeInternal 里栈构造、切片拷贝进 FloatingActionMode::mCallback 后即销毁,
//  故不能捕 this; Editor 寿命长于 ActionMode。
// =====================================================================================
class Editor::TextActionModeCallback : public ActionMode::Callback {
public:
    enum { SELECTION = 0, INSERTION = 1 };
    explicit TextActionModeCallback(Editor* editor, int mode) {
        const bool hasSelection = (mode == SELECTION) && editor->mTextView->hasSelection();
        onCreateActionMode = [editor, hasSelection](ActionMode& am, Menu& menu)->bool {
            am.setTitle("");
            am.setSubtitle("");
            am.setTitleOptionalHint(true);
            editor->populateTextActionModeMenu(menu, hasSelection);
            return true;
        };
        onPrepareActionMode = [](ActionMode&, Menu&)->bool { return true; };
        onActionItemClicked = [editor](ActionMode&, MenuItem& item)->bool {
            return editor->mTextView->onTextContextMenuItem(item.getItemId());
        };
        onDestroyActionMode = [editor](ActionMode&) {
            editor->mTextActionMode = nullptr;
            // 收起选区为光标 (对齐 AOSP onDestroyActionMode 的 !mPreserveSelection 分支)
            Spannable* e = editor->mTextView->getEditableText();
            if (e) {
                const int selEnd = Selection::getSelectionEnd(e);
                Selection::setSelection(e, selEnd);
            }
        };
        onGetContentRect = [editor, hasSelection](ActionMode&, View&, Rect& out) {
            editor->getTextActionModeContentRect(out, hasSelection);
        };
    }
};

// 对齐 AOSP Editor.startActionModeInternal (Editor.java:2602)
bool Editor::startActionModeInternal(int actionMode) {
    if (mTextActionMode != nullptr) {
        mTextActionMode->invalidate();
        return false;
    }
    TextActionModeCallback callback(this, actionMode);
    mTextActionMode = mTextView->startActionMode(callback, ActionMode::TYPE_FLOATING);
    return mTextActionMode != nullptr;
}

bool Editor::startSelectionActionMode() {
    return startActionModeInternal(TextActionModeCallback::SELECTION);
}

bool Editor::startInsertionActionMode() {
    return startActionModeInternal(TextActionModeCallback::INSERTION);
}

void Editor::invalidateTextActionMode() {
    if (mTextActionMode != nullptr) mTextActionMode->invalidate();
}

// 对齐 AOSP Editor.populateMenuWithItems (Editor.java:4751)。条件 add (不 setVisible)。
void Editor::populateTextActionModeMenu(Menu& menu, bool /*hasSelection*/) {
    if (mTextView->canCut()) menu.add(0, cdroid::R::id::cut,   ORDER_CUT,        "Cut");
    if (mTextView->canCopy())  menu.add(0, cdroid::R::id::copy,  ORDER_COPY,       "Copy");
    if (mTextView->canPaste()) menu.add(0, cdroid::R::id::paste, ORDER_PASTE,      "Paste");
    if (mTextView->canSelectAllText())
        menu.add(0, R::id::select_all, ORDER_SELECT_ALL, "Select all");
}

// 对齐 AOSP Editor.onGetContentRect (Editor.java:4886)。简化: 用 selStart/selEnd 的行
// top/bottom + primaryHorizontal 近似 selection rect (精确版用 getSelectionPath +
// Path::computeBounds, CDROID Path 暂无 computeBounds)。返回屏坐标 (FloatingToolbar 期望)。
void Editor::getTextActionModeContentRect(Rect& outRect, bool hasSelection) {
    Layout* layout = mTextView->getLayout();
    const int selStart = mTextView->getSelectionStart();
    const int selEnd = mTextView->getSelectionEnd();
    if (layout == nullptr || selStart < 0 || selEnd < 0) {
        int pos[2] = {0, 0};
        mTextView->getLocationOnScreen(pos);
        outRect.set(pos[0], pos[1], mTextView->getWidth(), mTextView->getHeight());
        return;
    }
    const int startLine = layout->getLineForOffset(selStart);
    const int endLine = layout->getLineForOffset(selEnd);
    int top = layout->getLineTop(startLine);
    int bottom = layout->getLineBottom(endLine);
    const float phStart = layout->getPrimaryHorizontal(selStart);
    const float phEnd = layout->getPrimaryHorizontal(selEnd);
    int left = (int)std::min(phStart, phEnd);
    int right = (int)std::max(phStart, phEnd);
    if (!hasSelection || right <= left) {
        left = 0;
        right = mTextView->getWidth();
    }
    const int hoff = mTextView->viewportToContentHorizontalOffset();
    const int voff = mTextView->viewportToContentVerticalOffset();
    left += hoff; right += hoff; top += voff; bottom += voff;
    int pos[2] = {0, 0};
    mTextView->getLocationOnScreen(pos);
    outRect.set(pos[0] + left, pos[1] + top, right - left, bottom - top);
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

int64_t Editor::getLastTouchOffsets()const{
    //SelectionModifierCursorController selectionController = getSelectionController();
    const int minOffset = 0;//selectionController.getMinTouchOffset();
    const int maxOffset = 0;//selectionController.getMaxTouchOffset();
    return TextUtils::packRangeInLong(minOffset, maxOffset);
}

void Editor::onFocusChanged(bool focused, int /*direction*/, Rect* /*previouslyFocusedRect*/) {
    // Android.Editor.onFocusChanged preserves the last tap position, invokes
    // MovementMethod.onTakeFocus, and updates selection/highlight state.
    mShowCursor = SystemClock::uptimeMillis();
    //ensureEndedBatchEdit();
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
        mTextView->onEndBatchEdit();
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

void Editor::createInputContentTypeIfNeeded() {
    if (mInputContentType == nullptr) {
        mInputContentType = new InputContentType();
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
    if(mTextView!=nullptr){
        mTextView->removeCallbacks(mBlink);
    }
    mBlinkCancelled = false;
    makeBlink();
}

void Editor::adjustInputType(bool password, bool passwordInputType,
        bool webPasswordInputType, bool numberPasswordInputType) {
    // mInputType has been set from inputType, possibly modified by mInputMethod.
    // Specialize mInputType to [web]password if we have a text class and the original input
    // type was a password.
    if ((mInputType & InputType::TYPE_MASK_CLASS) == InputType::TYPE_CLASS_TEXT) {
        if (password || passwordInputType) {
            mInputType = (mInputType & ~(InputType::TYPE_MASK_VARIATION))
                    | InputType::TYPE_TEXT_VARIATION_PASSWORD;
        }
        if (webPasswordInputType) {
            mInputType = (mInputType & ~(InputType::TYPE_MASK_VARIATION))
                    | InputType::TYPE_TEXT_VARIATION_WEB_PASSWORD;
        }
    } else if ((mInputType & InputType::TYPE_MASK_CLASS) == InputType::TYPE_CLASS_NUMBER) {
        if (numberPasswordInputType) {
            mInputType = (mInputType & ~(InputType::TYPE_MASK_VARIATION))
                    | InputType::TYPE_NUMBER_VARIATION_PASSWORD;
        }
    }
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
        const bool inMultiTapWindow = (mLastUpTime != 0)
                && ((now - mLastUpTime) <= timeoutMS)
                && ((dx * dx + dy * dy) <= (slop * slop));
        mTapCount = inMultiTapWindow ? mTapCount + 1 : 1;
        if (mTapCount > 3) mTapCount = 1;
        if (mTapCount == 2) {
            selectCurrentWord();
        } else if (mTapCount >= 3) {
            Selection::selectAll(editable());
        } else {
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

bool  Editor::needsToSelectAllToSelectWordOrParagraph() const{
    if (mTextView->hasPasswordTransformationMethod()) {
        // Always select all on a password field.
        // Cut/copy menu entries are not available for passwords, but being able to select all
        // is however useful to delete or paste to replace the entire content.
        return true;
    }

    const int inputType = mTextView->getInputType();
    const int klass = inputType & InputType::TYPE_MASK_CLASS;
    const int variation = inputType & InputType::TYPE_MASK_VARIATION;

    // Specific text field types: select the entire text for these
    if (klass == InputType::TYPE_CLASS_NUMBER
            || klass == InputType::TYPE_CLASS_PHONE
            || klass == InputType::TYPE_CLASS_DATETIME
            || variation == InputType::TYPE_TEXT_VARIATION_URI
            || variation == InputType::TYPE_TEXT_VARIATION_EMAIL_ADDRESS
            || variation == InputType::TYPE_TEXT_VARIATION_WEB_EMAIL_ADDRESS
            || variation == InputType::TYPE_TEXT_VARIATION_FILTER) {
        return true;
    }
    return false;
}

bool Editor::selectCurrentParagraph(){
    if (!mTextView->canSelectText()) {
        return false;
    }

    if (needsToSelectAllToSelectWordOrParagraph()) {
        return mTextView->selectAllText();
    }

    const int64_t lastTouchOffsets = getLastTouchOffsets();
    const int minLastTouchOffset = TextUtils::unpackRangeStartFromLong(lastTouchOffsets);
    const int maxLastTouchOffset = TextUtils::unpackRangeEndFromLong(lastTouchOffsets);

    const long paragraphsRange = getParagraphsRange(minLastTouchOffset, maxLastTouchOffset);
    const int start = TextUtils::unpackRangeStartFromLong(paragraphsRange);
    const int end = TextUtils::unpackRangeEndFromLong(paragraphsRange);
    if (start < end) {
        Selection::setSelection(dynamic_cast<Spannable*>(&mTextView->getText()), start, end);
        return true;
    }
    return false;
}

int64_t Editor::getParagraphsRange(int startOffset, int endOffset){
    const int startOffsetTransformed = mTextView->originalToTransformed(startOffset,
            OffsetMapping::MAP_STRATEGY_CURSOR);
    const int endOffsetTransformed = mTextView->originalToTransformed(endOffset,
            OffsetMapping::MAP_STRATEGY_CURSOR);
    const Layout* layout = mTextView->getLayout();
    if (layout == nullptr) {
        return TextUtils::packRangeInLong(-1, -1);
    }
    const CharSequence* text = layout->getText();
    int minLine = layout->getLineForOffset(startOffsetTransformed);
    // Search paragraph start.
    while (minLine > 0) {
        const int prevLineEndOffset = layout->getLineEnd(minLine - 1);
        if (text->charAt(prevLineEndOffset - 1) == '\n') {
            break;
        }
        minLine--;
    }
    int maxLine = layout->getLineForOffset(endOffsetTransformed);
    // Search paragraph end.
    while (maxLine < layout->getLineCount() - 1) {
        const int lineEndOffset = layout->getLineEnd(maxLine);
        if (text->charAt(lineEndOffset - 1) == '\n') {
            break;
        }
        maxLine++;
    }
    const int paragraphStart = mTextView->transformedToOriginal(layout->getLineStart(minLine),
            OffsetMapping::MAP_STRATEGY_CURSOR);
    const int paragraphEnd = mTextView->transformedToOriginal(layout->getLineEnd(maxLine),
            OffsetMapping::MAP_STRATEGY_CURSOR);
    return TextUtils::packRangeInLong(paragraphStart, paragraphEnd);

}
bool Editor::selectCurrentWord() {
    Spannable* e = editable();
    if(!mTextView->canSelectText()||e==nullptr){
        return false;
    }
    if(needsToSelectAllToSelectWordOrParagraph()){
        return mTextView->selectAllText();
    }

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
    // 选词后弹出文本选择 ActionMode (双击选词 → FloatingToolbar)。
    startSelectionActionMode();
    return true;
}

// 对齐 AOSP Editor.performLongClick (Editor.java:1483)。简化: 无 accessibility 分支、
// InsertionPointCursorController、选择手柄 drag、touchPositionIsInSelection, 取默认分支:
// 长按文字 → selectCurrentWord (成功则由 selectCurrentWord 弹 selection ActionMode);
// 选词失败(空白) → 插入 ActionMode。
bool Editor::performLongClick(bool handled) {
    if (handled) return handled;
    if (mTextView->length() <= 0) return handled;
    const bool selected = selectCurrentWord();
    if (!selected) {
        startInsertionActionMode();
    }
    return handled;
}

void Editor::onDraw(Canvas& canvas, Layout* layout, Path* highlight, Paint& highlightPaint, int cursorOffsetVertical){
    const int selectionStart = mTextView->getSelectionStart();
    const int selectionEnd = mTextView->getSelectionEnd();

    // Android Editor.onDraw reports the full text to the IME here when in extract
    // mode (imm.isActive(mTextView) + reportExtractedText()) and draws the
    // correction highlighter. Neither extract mode nor InputMethodState/
    // mCorrectionHighlighter is ported, and the in-process IMM needs no extracted
    // text (it commits directly into the buddy view), so this block stays deferred.

    if (highlight != nullptr && selectionStart == selectionEnd && mDrawableForCursor != nullptr) {
        drawCursor(canvas, cursorOffsetVertical);
        // Rely on the drawable entirely, do not draw the cursor line.
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
    // On the outermost close, Android notifies the IME of the new selection. The
    // downstream IMM call is a no-op in the in-process model, but issue it so the
    // edit/batch path mirrors Android and stays live for a future IME pass.
    if (mBatchEditNesting == 0) {
        sendUpdateSelection();
    }
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

void Editor::addSpanWatchers(Spannable& text) {
    // Port of Android Editor.addSpanWatchers. Android attaches two spans over
    // [0, textLength]: mKeyListener (BaseKeyListener implements SpanWatcher, used
    // to react to SuggestionSpan changes) and mSpanController. CDROID's KeyListener
    // is NOT a SpanWatcher/ParcelableSpan (the KeyListener→SpanWatcher wiring is a
    // deferred Phase-1 item, see text/method/textkeylistener.h), so only the
    // SpanController is attached here.
    const int textLength = (int)text.length();

    if (mSpanController == nullptr) {
        mSpanController = new SpanController(this);
    }
    text.setSpan(mSpanController, 0, textLength, Spanned::SPAN_INCLUSIVE_INCLUSIVE);
}

void Editor::sendUpdateSelection() {
    // Android: InputMethodManager.updateSelection(view, selStart, selEnd,
    // candidatesStart, candidatesEnd) pushes the editor's selection to the IME so
    // its candidates/composing region can track it. CDROID's in-process IME keeps
    // no such state (composing lives inside IMEWindow and committed text flows
    // straight into the buddy view), so the downstream call is a documented
    // no-op — but we still issue it so the SpanController path reads like Android
    // and the hook is live for a future InputConnection pass. The selection itself
    // stays authoritative via the Selection spans; the host TextView is notified
    // independently via spanChange -> onSelectionChanged.
    InputMethodManager* imm = mTextView->getInputMethodManager();
    if (imm != nullptr) {
        const int selStart = mTextView->getSelectionStart();
        const int selEnd   = mTextView->getSelectionEnd();
        imm->updateSelection(mTextView, selStart, selEnd, /*candidatesStart*/ -1,
                             /*candidatesEnd*/ -1);
    }
}

}  // namespace cdroid
