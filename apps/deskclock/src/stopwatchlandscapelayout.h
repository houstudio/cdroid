#ifndef __DESKCLOCK_STOPWATCHLANDSCAPELAYOUT_H__
#define __DESKCLOCK_STOPWATCHLANDSCAPELAYOUT_H__
/*********************************************************************************
 * Port of com.android.deskclock.stopwatch.StopwatchLandscapeLayout — laps list
 * docked to one side, stopwatch circle centered in the remaining space.
 *********************************************************************************/
#include <view/viewgroup.h>

namespace cdroid {
namespace deskclock {
namespace stopwatch {

class StopwatchLandscapeLayout : public ViewGroup {
private:
    View* mLapsListView = nullptr;
    View* mStopwatchView = nullptr;

public:
    StopwatchLandscapeLayout(Context* context, const AttributeSet* attrs);

    void onFinishInflate() override;

protected:
    void onMeasure(int widthMeasureSpec, int heightMeasureSpec) override;
    void onLayout(bool changed, int left, int top, int right, int bottom) override;
};

} // namespace stopwatch
} // namespace deskclock
} // namespace cdroid

#endif // __DESKCLOCK_STOPWATCHLANDSCAPELAYOUT_H__
