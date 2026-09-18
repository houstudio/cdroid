#ifndef __TIMER_CIRCLE_FRAME_LAYOUT_H__
#define __TIMER_CIRCLE_FRAME_LAYOUT_H__
// Port of com.android.deskclock.TimerCircleFrameLayout — a container that
// frames a timer circle of some sort. The circle is allowed to grow naturally
// according to its layout constraints up to the largest
// (R.dimen.max_timer_circle_size) allowable size.
#include <widget/framelayout.h>

namespace cdroid {
namespace deskclock {

class TimerCircleFrameLayout : public FrameLayout {
public:
    TimerCircleFrameLayout(Context* context, const AttributeSet* attrs);
protected:
    void onMeasure(int widthMeasureSpec, int heightMeasureSpec) override;
};

} // namespace deskclock
} // namespace cdroid

#endif /* __TIMER_CIRCLE_FRAME_LAYOUT_H__ */
