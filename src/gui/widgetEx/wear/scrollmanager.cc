#include <widgetEx/wear/scrollmanager.h>
namespace cdroid{
/* 
    private static constexpr int ONE_SEC_IN_MS = 1000;
    private static constexpr float VELOCITY_MULTIPLIER = 1.5f;
    private static constexpr float FLING_EDGE_RATIO = 1.5f;

    private float mMinRadiusFraction = 0.0f;
    private float mMinRadiusFractionSquared = mMinRadiusFraction * mMinRadiusFraction;
    private float mScrollDegreesPerScreen = 180;
    private float mScrollRadiansPerScreen = (float) Math.toRadians(mScrollDegreesPerScreen);
    private float mScreenRadiusPx;
    private float mScreenRadiusPxSquared;
    private float mScrollPixelsPerRadian;

    private bool mDown;
    private bool mScrolling;
    private float mLastAngleRadians;

    private RecyclerView* mRecyclerView;
    VelocityTracker* mVelocityTracker;
*/

ScrollManager::ScrollManager(){
    mMinRadiusFraction = 0.0f;
    mMinRadiusFractionSquared = mMinRadiusFraction * mMinRadiusFraction;

    mScrollDegreesPerScreen = 180;
    mScrollRadiansPerScreen = MathUtils::toRadians(mScrollDegreesPerScreen);
 
    mDown = false;
    mScrolling =false;
    mRecyclerView = nullptr;
    mVelocityTracker = nullptr;
}

ScrollManager::~ScrollManager(){
    if(mVelocityTracker){
        mVelocityTracker->recycle();
    }
}

void ScrollManager::setRecyclerView(RecyclerView* recyclerView, int width, int height) {
    mRecyclerView = recyclerView;
    mScreenRadiusPx = std::max(width, height) / 2.f;
    mScreenRadiusPxSquared = mScreenRadiusPx * mScreenRadiusPx;
    mScrollPixelsPerRadian = height / mScrollRadiansPerScreen;
    mScrollRadiansPerScreen = (float) std::toRadians(mScrollDegreesPerScreen)
    mVelocityTracker = VelocityTracker::obtain();
}

/** Remove the binding with a {@link RecyclerView} */
void ScrollManager::clearRecyclerView() {
    mRecyclerView = nullptr;
}

bool ScrollManager::onTouchEvent(MotionEvent& event) {
    float deltaX = event.getRawX() - mScreenRadiusPx;
    float deltaY = event.getRawY() - mScreenRadiusPx;
    float radiusSquared = deltaX * deltaX + deltaY * deltaY;
    MotionEvent* vtev = MotionEvent::obtain(event);
    mVelocityTracker->addMovement(vtev);
    vtev->recycle();

    switch (event.getActionMasked()) {
    case MotionEvent::ACTION_DOWN:
        if (radiusSquared / mScreenRadiusPxSquared > mMinRadiusFractionSquared) {
            mDown = true;
            return true; // Consume the event.
        }
        break;

    case MotionEvent::ACTION_MOVE:
        if (mScrolling) {
            float angleRadians = (float) std::atan2(deltaY, deltaX);
            float deltaRadians = angleRadians - mLastAngleRadians;
            deltaRadians = normalizeAngleRadians(deltaRadians);
            int scrollPixels = std::round(deltaRadians * mScrollPixelsPerRadian);
            if (scrollPixels != 0) {
                mRecyclerView.scrollBy(0 /* x */, scrollPixels /* y */);
                // Recompute deltaRadians in terms of rounded scrollPixels.
                deltaRadians = scrollPixels / mScrollPixelsPerRadian;
                mLastAngleRadians += deltaRadians;
                mLastAngleRadians = normalizeAngleRadians(mLastAngleRadians);
            }
            // Always consume the event so that we never break the circular scrolling
            // gesture.
            return true;
        }

        if (mDown) {
            float deltaXFromCenter = event.getRawX() - mScreenRadiusPx;
            float deltaYFromCenter = event.getRawY() - mScreenRadiusPx;
            float distFromCenter = (float) std::hypot(deltaXFromCenter, deltaYFromCenter);
            if (distFromCenter != 0) {
                deltaXFromCenter /= distFromCenter;
                deltaYFromCenter /= distFromCenter;

                mScrolling = true;
                mRecyclerView.invalidate();
                mLastAngleRadians = (float) std::atan2(deltaYFromCenter, deltaXFromCenter);
                return true; // Consume the event.
            }
        } else {
            // Double check we're not missing an event we should really be handling.
            if (radiusSquared / mScreenRadiusPxSquared > mMinRadiusFractionSquared) {
                mDown = true;
                return true; // Consume the event.
            }
        }
        break;

    case MotionEvent::ACTION_UP:
        mDown = false;
        mScrolling = false;
        mVelocityTracker->computeCurrentVelocity(ONE_SEC_IN_MS,
                mRecyclerView->getMaxFlingVelocity());
        int velocityY = (int) mVelocityTracker->getYVelocity();
        if (event.getX() < FLING_EDGE_RATIO * mScreenRadiusPx) {
            velocityY = -velocityY;
        }
        mVelocityTracker->clear();
        if (std::abs(velocityY) > mRecyclerView->getMinFlingVelocity()) {
            return mRecyclerView->fling(0, (int) (VELOCITY_MULTIPLIER * velocityY));
        }
        break;

    case MotionEvent::ACTION_CANCEL:
        if (mDown) {
            mDown = false;
            mScrolling = false;
            mRecyclerView.invalidate();
            return true; // Consume the event.
        }
        break;
    }

    return false;
}

float ScrollManager::normalizeAngleRadians(float angleRadians) {
    if (angleRadians < -M_PI) {
        angleRadians = (float) (angleRadians + M_PI * 2.0);
    }
    if (angleRadians > M_PI) {
        angleRadians = (float) (angleRadians - M_PI * 2.0);
    }
    return angleRadians;
}

void ScrollManager::setScrollDegreesPerScreen(float degreesPerScreen) {
    mScrollDegreesPerScreen = degreesPerScreen;
    mScrollRadiansPerScreen = (float) MathUtils::toRadians(mScrollDegreesPerScreen);
}

void ScrollManager::setBezelWidth(float fraction) {
    mMinRadiusFraction = 1.f - fraction;
    mMinRadiusFractionSquared = mMinRadiusFraction * mMinRadiusFraction;
}

float ScrollManager::getScrollDegreesPerScreen() const{
    return mScrollDegreesPerScreen;
}

float ScrollManager::getBezelWidth() {
    return 1 - mMinRadiusFraction;
}
}/*endof namespace*/
