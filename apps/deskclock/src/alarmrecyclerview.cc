#include <alarmrecyclerview.h>

namespace cdroid {
namespace deskclock {

AlarmRecyclerView::AlarmRecyclerView(Context* context, const AttributeSet* attrs)
    : RecyclerView(context, attrs) {
    mDisallowScrollListener.onInterceptTouchEvent = [](RecyclerView& rv, MotionEvent&) {
        // Disable scrolling/user action to prevent choppy animations.
        ItemAnimator* animator = rv.getItemAnimator();
        return animator != nullptr && animator->isRunning();
    };
    addOnItemTouchListener(mDisallowScrollListener);
}

void AlarmRecyclerView::onLayout(bool changed, int left, int top, int right, int bottom) {
    mIgnoreRequestLayout = true;
    RecyclerView::onLayout(changed, left, top, right, bottom);
    mIgnoreRequestLayout = false;
}

void AlarmRecyclerView::requestLayout() {
    ItemAnimator* animator = getItemAnimator();
    if (!mIgnoreRequestLayout && (animator == nullptr || !animator->isRunning())) {
        RecyclerView::requestLayout();
    }
}

} // namespace deskclock

typedef cdroid::deskclock::AlarmRecyclerView AlarmRecyclerView;
DECLARE_WIDGET2(AlarmRecyclerView, "AlarmRecyclerView");

} // namespace cdroid
