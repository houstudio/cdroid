#ifndef __DESKCLOCK_STOPWATCHCIRCLEVIEW_H__
#define __DESKCLOCK_STOPWATCHCIRCLEVIEW_H__
/*********************************************************************************
 * Port of com.android.deskclock.stopwatch.StopwatchCircleView — draws the
 * reference lap as a circle: white remainder arc + accent completed arc + dot.
 *********************************************************************************/
#include <view/view.h>

namespace cdroid {
namespace deskclock {
namespace stopwatch {

class StopwatchCircleView : public View {
private:
    /** The size of the dot indicating the user's position within the reference lap. */
    float mDotRadius;
    /** An amount to subtract from the true radius to account for drawing thicknesses. */
    float mRadiusOffset;
    /** Used to scale the width of the marker to make it similarly visible on all screens. */
    float mScreenDensity;
    /** The color indicating the remaining portion of the current lap. */
    int mRemainderColor;
    /** The color indicating the completed portion of the lap. */
    int mCompletedColor;
    /** The size of the stroke that paints the lap circle. */
    float mStrokeSize;
    /** The size of the stroke that paints the marker for the end of the prior lap. */
    float mMarkerStrokeSize;

public:
    StopwatchCircleView(Context* context, const AttributeSet* attrs);

    /** Start the animation if it is not currently running. */
    void update();

    void onDraw(Canvas& canvas) override;
};

} // namespace stopwatch
} // namespace deskclock
} // namespace cdroid

#endif // __DESKCLOCK_STOPWATCHCIRCLEVIEW_H__
