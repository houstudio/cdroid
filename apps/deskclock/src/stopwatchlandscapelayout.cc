#include <stopwatchlandscapelayout.h>

#include <R.h>

#include <algorithm>

using namespace ::deskclock;

namespace cdroid {
namespace deskclock {
namespace stopwatch {

StopwatchLandscapeLayout::StopwatchLandscapeLayout(Context* context, const AttributeSet* attrs)
    : ViewGroup(context, attrs) {
}

void StopwatchLandscapeLayout::onFinishInflate() {
    ViewGroup::onFinishInflate();

    mLapsListView = findViewById(R::id::laps_list);
    mStopwatchView = findViewById(R::id::stopwatch_time_wrapper);
}

void StopwatchLandscapeLayout::onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
    const int height = View::MeasureSpec::getSize(heightMeasureSpec);
    const int width = View::MeasureSpec::getSize(widthMeasureSpec);
    const int halfWidth = width / 2;

    const int minWidthSpec = View::MeasureSpec::makeMeasureSpec(width, View::MeasureSpec::UNSPECIFIED);
    const int maxHeightSpec = View::MeasureSpec::makeMeasureSpec(height, View::MeasureSpec::AT_MOST);

    // First determine the width of the laps list.
    int lapsListWidth;
    if (mLapsListView != nullptr && mLapsListView->getVisibility() != View::GONE) {
        // Measure the intrinsic size of the laps list.
        mLapsListView->measure(minWidthSpec, maxHeightSpec);

        // Actual laps list width is the larger of half the container and its intrinsic width.
        lapsListWidth = std::max(mLapsListView->getMeasuredWidth(), halfWidth);
        const int lapsListWidthSpec =
                View::MeasureSpec::makeMeasureSpec(lapsListWidth, View::MeasureSpec::EXACTLY);
        mLapsListView->measure(lapsListWidthSpec, maxHeightSpec);
    } else {
        lapsListWidth = 0;
    }

    // Measure the stopwatch.
    const int stopwatchWidthSpec =
            View::MeasureSpec::makeMeasureSpec(width - lapsListWidth, View::MeasureSpec::AT_MOST);
    mStopwatchView->measure(stopwatchWidthSpec, maxHeightSpec);

    setMeasuredDimension(width, height);
}

void StopwatchLandscapeLayout::onLayout(bool /*changed*/, int /*left*/, int /*top*/,
                                        int /*right*/, int /*bottom*/) {
    const int left = getPaddingLeft();
    const int top = getPaddingTop();
    const int right = getWidth() - getPaddingRight();
    const int bottom = getHeight() - getPaddingBottom();
    const int width = right - left;
    const int height = bottom - top;
    const int halfHeight = height / 2;
    const bool isLTR = getLayoutDirection() == View::LAYOUT_DIRECTION_LTR;

    int lapsListWidth;
    if (mLapsListView != nullptr && mLapsListView->getVisibility() != View::GONE) {
        // Layout the laps list, centering it vertically.
        lapsListWidth = mLapsListView->getMeasuredWidth();
        const int lapsListHeight = mLapsListView->getMeasuredHeight();
        const int lapsListTop = top + halfHeight - lapsListHeight / 2;
        int lapsListLeft;
        int lapsListRight;
        if (isLTR) {
            lapsListLeft = right - lapsListWidth;
            lapsListRight = right;
        } else {
            lapsListLeft = left;
            lapsListRight = left + lapsListWidth;
        }
        mLapsListView->layout(lapsListLeft, lapsListTop, lapsListWidth, lapsListHeight);
    } else {
        lapsListWidth = 0;
    }

    // Layout the stopwatch, centering it horizontally and vertically.
    const int stopwatchWidth = mStopwatchView->getMeasuredWidth();
    const int stopwatchHeight = mStopwatchView->getMeasuredHeight();
    const int stopwatchTop = top + halfHeight - stopwatchHeight / 2;
    int stopwatchLeft;
    int stopwatchRight;
    if (isLTR) {
        stopwatchLeft = left + (width - lapsListWidth - stopwatchWidth) / 2;
        stopwatchRight = stopwatchLeft + stopwatchWidth;
    } else {
        stopwatchRight = right - (width - lapsListWidth - stopwatchWidth) / 2;
        stopwatchLeft = stopwatchRight - stopwatchWidth;
    }

    // View::layout takes (x, y, width, height) — pass the size, not the edges.
    mStopwatchView->layout(stopwatchLeft, stopwatchTop, stopwatchWidth, stopwatchHeight);
}

} // namespace stopwatch

typedef cdroid::deskclock::stopwatch::StopwatchLandscapeLayout StopwatchLandscapeLayout;
DECLARE_WIDGET2(StopwatchLandscapeLayout, "StopwatchLandscapeLayout");

} // namespace deskclock
} // namespace cdroid
