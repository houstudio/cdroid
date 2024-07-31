#ifndef __LINEAR_SMOOTH_SCROLLER_H__
#define __LINEAR_SMOOTH_SCROLLER_H__
#include <widgetEx/recyclerview/recyclerview.h>
namespace cdroid{
class LinearSmoothScroller:public RecyclerView::SmoothScroller {
private:
    static constexpr float MILLISECONDS_PER_INCH = 25.f;
    static constexpr int TARGET_SEEK_SCROLL_DISTANCE_PX = 10000;
    static constexpr float TARGET_SEEK_EXTRA_SCROLL_RATIO = 1.2f;
    float mMillisPerPixel;
    DisplayMetrics mDisplayMetrics;
public:
    static constexpr int SNAP_TO_START = -1;
    static constexpr int SNAP_TO_END = 1;
    static constexpr int SNAP_TO_ANY = 0;
protected:
    LinearInterpolator* mLinearInterpolator;
    DecelerateInterpolator* mDecelerateInterpolator;
    PointF mTargetVector;
    int mInterimTargetDx = 0;
    int mInterimTargetDy = 0;
    bool mHasCalculatedMillisPerPixel;
    bool mTargetVectorUsable;
private:
    float getSpeedPerPixel();
protected:
    float calculateSpeedPerPixel(const DisplayMetrics& displayMetrics) const;
public:
    LinearSmoothScroller(Context* context);
    ~LinearSmoothScroller();
    void onStart()override;
    void onTargetFound(View* targetView, RecyclerView::State& state, Action& action)override;
    void onSeekTargetStep(int dx, int dy, RecyclerView::State& state, Action& action)override;
    void onStop()override;
    float calculateSpeedPerPixel(DisplayMetrics& displayMetrics);
    int calculateTimeForDeceleration(int dx);
    int calculateTimeForScrolling(int dx);
    int getHorizontalSnapPreference();
    int getVerticalSnapPreference();
    void updateActionForInterimTarget(Action& action);
    int clampApplyScroll(int tmpDt, int dt);
    virtual int calculateDtToFit(int viewStart, int viewEnd, int boxStart, int boxEnd, int
        snapPreference);

    virtual int calculateDyToMakeVisible(View* view, int snapPreference);
    virtual int calculateDxToMakeVisible(View* view, int snapPreference);
};
}/*endof namespace*/
#endif/*__LINEAR_SMOOTH_SCROLLER_H__*/
