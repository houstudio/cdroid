/*********************************************************************************
 * Port of AOSP coretests android.util.InternalSelectionView (framework test
 * util). A view with a known number of selectable rows, maintaining a notion
 * of the selected row; the height divides evenly among rows (the last row
 * absorbs the remainder).
 *
 * Being a good citizen w.r.t. internal selection:
 * 1) calls View::requestRectangleOnScreen each time the selection changes due
 *    to internal navigation;
 * 2) implements View::getFocusedRect by filling in the rectangle of the
 *    currently selected row;
 * 3) overrides View::onFocusChanged and sets the selection according to the
 *    previously focused rectangle.
 *
 * Source of truth:
 * frameworks/base/core/tests/coretests/src/android/util/InternalSelectionView.java
 *********************************************************************************/
#ifndef KEYNAV_INTERNALSELECTIONVIEW_H
#define KEYNAV_INTERNALSELECTIONVIEW_H
#include <cdroid.h>

namespace keynav {

class InternalSelectionView : public cdroid::View {
private:
    int mNumRows = 5;
    int mSelectedRow = 0;
    static const int mEstimatedPixelHeight = 10;

    int mDesiredHeight = -1; /* null in AOSP */
    std::string mLabel;

protected:
    void onMeasure(int widthMeasureSpec, int heightMeasureSpec) override;
    void onDraw(cdroid::Canvas& canvas) override;
    void onFocusChanged(bool focused, int direction, cdroid::Rect* previouslyFocusedRect) override;

public:
    InternalSelectionView(cdroid::Context* context, int numRows, const std::string& label);

    int getNumRows() const { return mNumRows; }
    int getSelectedRow() const { return mSelectedRow; }
    void setDesiredHeight(int desiredHeight) { mDesiredHeight = desiredHeight; }
    const std::string& getLabel() const { return mLabel; }

    void getRectForRow(cdroid::Rect& rect, int row) const;
    void getFocusedRect(cdroid::Rect& r) override;
    bool onKeyDown(int keyCode, cdroid::KeyEvent& event) override;

private:
    int getRowHeight(int row) const;
    int measureWidth(int measureSpec) const;
    int measureHeight(int measureSpec) const;
    void ensureRectVisible();
};

} // namespace keynav
#endif
