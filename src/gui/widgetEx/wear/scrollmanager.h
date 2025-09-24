#ifndef __WEAR_SCROLL_MANAGER_H__
#define __WEAR_SCROLL_MANAGER_H__
#include <widgetEx/recyclerview/recyclerview.h>

namespace cdroid{
class ScrollManager {
    // One second in milliseconds.
private:
    static constexpr int ONE_SEC_IN_MS = 1000;
    static constexpr float VELOCITY_MULTIPLIER = 1.5f;
    static constexpr float FLING_EDGE_RATIO = 1.5f;

    float mMinRadiusFraction;
    float mMinRadiusFractionSquared;

    float mScrollDegreesPerScreen;
    float mScrollRadiansPerScreen;

    float mScreenRadiusPx;
    float mScreenRadiusPxSquared;
    float mScrollPixelsPerRadian;
    float mLastAngleRadians;

    bool mDown;
    bool mScrolling;
    RecyclerView* mRecyclerView;
    VelocityTracker* mVelocityTracker;
private:
    static float normalizeAngleRadians(float angleRadians);
public:
    ScrollManager();
    ~ScrollManager();
    void setRecyclerView(RecyclerView* recyclerView, int width, int height);

    void clearRecyclerView();

    bool onTouchEvent(MotionEvent& event);

    void setScrollDegreesPerScreen(float degreesPerScreen);

    void setBezelWidth(float fraction);

    float getScrollDegreesPerScreen() const;

    float getBezelWidth() const;
};
}/*endof namespace*/
#endif/*__WEAR_SCROLL_MANAGER_H__*/
