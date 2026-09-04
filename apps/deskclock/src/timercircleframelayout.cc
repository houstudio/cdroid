#include <timercircleframelayout.h>

#include <algorithm>

#include <R.h>

using namespace ::deskclock;

namespace cdroid {
namespace deskclock {

TimerCircleFrameLayout::TimerCircleFrameLayout(Context* context, const AttributeSet* attrs)
    : FrameLayout(context, attrs) {
}

/**
 * Note: this method assumes the parent container will specify exact
 * (MeasureSpec.EXACTLY) width and height values. Upstream the percent layout
 * supplies those; the cdroid XML hands this container match_parent dimensions
 * instead (there is no percent layout port).
 */
void TimerCircleFrameLayout::onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
    const int paddingLeft = getPaddingLeft();
    const int paddingRight = getPaddingRight();
    const int paddingTop = getPaddingTop();
    const int paddingBottom = getPaddingBottom();

    // Fetch the exact sizes imposed by the parent container.
    const int width = MeasureSpec::getSize(widthMeasureSpec) - paddingLeft - paddingRight;
    const int height = MeasureSpec::getSize(heightMeasureSpec) - paddingTop - paddingBottom;
    const int smallestDimension = std::min(width, height);

    // Fetch the absolute maximum circle size allowed.
    const int maxSize = getContext()->getResources()
            .getDimensionPixelSize(R::dimen::max_timer_circle_size);
    const int size = std::min(smallestDimension, maxSize);

    // Set the size of this container.
    FrameLayout::onMeasure(
            MeasureSpec::makeMeasureSpec(size + paddingLeft + paddingRight, MeasureSpec::EXACTLY),
            MeasureSpec::makeMeasureSpec(size + paddingTop + paddingBottom, MeasureSpec::EXACTLY));
}

DECLARE_WIDGET3(cdroid::deskclock::TimerCircleFrameLayout, TimerCircleFrameLayout, 0);

} // namespace deskclock
} // namespace cdroid
