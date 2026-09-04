#ifndef __DESKCLOCK_TIMERCIRCLEVIEW_H__
#define __DESKCLOCK_TIMERCIRCLEVIEW_H__
/*********************************************************************************
 * Port of com.android.deskclock.timer.TimerCircleView — draws timer progress
 * as a circle: white = remaining, accent = completed, plus a progress dot.
 *********************************************************************************/
#include <view/view.h>

#include <timer.h>

namespace cdroid {
namespace deskclock {

class TimerCircleView : public View {
private:
    /** The size of the dot indicating the progress through the timer. */
    float mDotRadius;

    /** An amount to subtract from the true radius to account for drawing thicknesses. */
    float mRadiusOffset;

    /** The color indicating the remaining portion of the timer. */
    int mRemainderColor;

    /** The color indicating the completed portion of the timer. */
    int mCompletedColor;

    /** The size of the stroke that paints the timer circle. */
    float mStrokeSize;

    const data::Timer* mTimer = nullptr;

public:
    TimerCircleView(Context* context, const AttributeSet* attrs);

    void update(const data::Timer& timer);

protected:
    void onDraw(Canvas& canvas) override;
};

} // namespace deskclock
} // namespace cdroid

#endif // __DESKCLOCK_TIMERCIRCLEVIEW_H__
