/*********************************************************************************
 * Port of AOSP coretests android.util.InternalSelectionView — see header.
 * The onDraw rendering is reduced to flat row rectangles (the visual output is
 * never asserted); all measurement / focus / key logic is a faithful
 * translation of the Java original.
 *********************************************************************************/
#include "internalselectionview.h"

using namespace cdroid;

namespace keynav {

static const uint32_t COLOR_RED = 0xFFFF0000;

InternalSelectionView::InternalSelectionView(Context* context, int numRows, const std::string& label)
      : View(context) {
    mNumRows = numRows;
    mLabel = label;
    setFocusable(true);
}

int InternalSelectionView::measureWidth(int measureSpec) const {
    const int specMode = MeasureSpec::getMode(measureSpec);
    const int specSize = MeasureSpec::getSize(measureSpec);

    const int desiredWidth = 300 + mPaddingLeft + mPaddingRight;
    if (specMode == MeasureSpec::EXACTLY) {
        return specSize;
    } else if (specMode == MeasureSpec::AT_MOST) {
        return desiredWidth < specSize ? desiredWidth : specSize;
    } else {
        return desiredWidth;
    }
}

int InternalSelectionView::measureHeight(int measureSpec) const {
    const int specMode = MeasureSpec::getMode(measureSpec);
    const int specSize = MeasureSpec::getSize(measureSpec);

    const int desiredHeight = mDesiredHeight >= 0 ?
            mDesiredHeight :
            mNumRows * mEstimatedPixelHeight + mPaddingTop + mPaddingBottom;
    if (specMode == MeasureSpec::EXACTLY) {
        return specSize;
    } else if (specMode == MeasureSpec::AT_MOST) {
        return desiredHeight < specSize ? desiredHeight : specSize;
    } else {
        return desiredHeight;
    }
}

void InternalSelectionView::onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
    setMeasuredDimension(measureWidth(widthMeasureSpec), measureHeight(heightMeasureSpec));
}

void InternalSelectionView::onDraw(Canvas& canvas) {
    /* Simplified rendering (AOSP draws two rects + row number per row): one
       flat rect per row, the selected row highlighted when focused. */
    int rectTop = mPaddingTop;
    const int rectLeft = mPaddingLeft;
    const int rectRight = getWidth() - mPaddingRight;
    for (int i = 0; i < mNumRows; i++) {
        const int rowHeight = getRowHeight(i);
        if (i == mSelectedRow && hasFocus()) {
            canvas.set_color(COLOR_RED);
        } else {
            canvas.set_color((uint32_t)0x20200000 /* translucent black */);
        }
        canvas.rectangle(rectLeft, rectTop, rectRight - rectLeft, rowHeight);
        canvas.fill();
        rectTop += rowHeight;
    }
}

int InternalSelectionView::getRowHeight(int row) const {
    const int availableHeight = getHeight() - mPaddingTop - mPaddingBottom;
    const int desiredRowHeight = availableHeight / mNumRows;
    if (row < mNumRows - 1) {
        return desiredRowHeight;
    } else {
        const int residualHeight = availableHeight % mNumRows;
        return desiredRowHeight + residualHeight;
    }
}

void InternalSelectionView::getRectForRow(Rect& rect, int row) const {
    const int rowHeight = getRowHeight(row);
    const int top = mPaddingTop + row * rowHeight;
    rect.set(mPaddingLeft, top, getWidth() - mPaddingRight - mPaddingLeft, rowHeight);
}

void InternalSelectionView::ensureRectVisible() {
    Rect tempRect;
    getRectForRow(tempRect, mSelectedRow);
    requestRectangleOnScreen(tempRect);
}

bool InternalSelectionView::onKeyDown(int keyCode, KeyEvent& event) {
    switch (event.getKeyCode()) {
    case KeyEvent::KEYCODE_DPAD_UP:
        if (mSelectedRow > 0) {
            mSelectedRow--;
            invalidate();
            ensureRectVisible();
            return true;
        }
        break;
    case KeyEvent::KEYCODE_DPAD_DOWN:
        if (mSelectedRow < (mNumRows - 1)) {
            mSelectedRow++;
            invalidate();
            ensureRectVisible();
            return true;
        }
        break;
    }
    return false;
}

void InternalSelectionView::getFocusedRect(Rect& r) {
    getRectForRow(r, mSelectedRow);
}

void InternalSelectionView::onFocusChanged(bool focused, int direction, Rect* previouslyFocusedRect) {
    View::onFocusChanged(focused, direction, previouslyFocusedRect);

    if (focused) {
        switch (direction) {
        case View::FOCUS_DOWN:
            mSelectedRow = 0;
            break;
        case View::FOCUS_UP:
            mSelectedRow = mNumRows - 1;
            break;
        case View::FOCUS_LEFT:  /* fall through */
        case View::FOCUS_RIGHT:
            /* set the row that is closest to the rect */
            if (previouslyFocusedRect != nullptr) {
                const int y = previouslyFocusedRect->top + (previouslyFocusedRect->height / 2);
                const int yPerRow = getHeight() / mNumRows;
                mSelectedRow = y / yPerRow;
            } else {
                mSelectedRow = 0;
            }
            break;
        default:
            /* can't gleam any useful information about what internal
               selection should be... */
            return;
        }
        invalidate();
    }
}

} // namespace keynav
