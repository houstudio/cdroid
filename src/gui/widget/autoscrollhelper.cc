#include <widget/autoscrollhelper.h>
#include <widget/abslistview.h>
#include <utils/mathutils.h>
namespace cdroid{

AutoScrollHelper::AutoScrollHelper(View* target) {
    mTarget = target;

    //DisplayMetrics metrics = Resources.getSystem().getDisplayMetrics();
    //int maxVelocity = (int) (DEFAULT_MAXIMUM_VELOCITY_DIPS * metrics.density + 0.5f);
    //int minVelocity = (int) (DEFAULT_MINIMUM_VELOCITY_DIPS * metrics.density + 0.5f);
    int maxVelocity = DEFAULT_MAXIMUM_VELOCITY_DIPS*160;
    int minVelocity = DEFAULT_MINIMUM_VELOCITY_DIPS*160;
    setMaximumVelocity(maxVelocity, maxVelocity);
    setMinimumVelocity(minVelocity, minVelocity);

    setEdgeType(DEFAULT_EDGE_TYPE);
    setMaximumEdges(DEFAULT_MAXIMUM_EDGE, DEFAULT_MAXIMUM_EDGE);
    setRelativeEdges(DEFAULT_RELATIVE_EDGE, DEFAULT_RELATIVE_EDGE);
    setRelativeVelocity(DEFAULT_RELATIVE_VELOCITY, DEFAULT_RELATIVE_VELOCITY);
    setActivationDelay(DEFAULT_ACTIVATION_DELAY);
    setRampUpDuration(DEFAULT_RAMP_UP_DURATION);
    setRampDownDuration(DEFAULT_RAMP_DOWN_DURATION);

    mScroller =new ClampedScroller();
    mEdgeInterpolator = AccelerateInterpolator::Instance;
}

AutoScrollHelper::~AutoScrollHelper(){
    delete mScroller;
    //delete mEdgeInterpolator;
}

AutoScrollHelper& AutoScrollHelper::setEnabled(bool enabled) {
    if (mEnabled && !enabled) {
        requestStop();
    }
    mEnabled = enabled;
    return *this;
}

bool AutoScrollHelper::isEnabled()const{
    return mEnabled;
}

AutoScrollHelper& AutoScrollHelper::setExclusive(bool exclusive) {
    mExclusive = exclusive;
    return *this;
}

bool AutoScrollHelper::isExclusive()const{
    return mExclusive;
}

AutoScrollHelper& AutoScrollHelper::setMaximumVelocity(float horizontalMax, float verticalMax) {
    mMaximumVelocity[HORIZONTAL] = horizontalMax / 1000.f;
    mMaximumVelocity[VERTICAL] = verticalMax / 1000.f;
    return *this;
}

AutoScrollHelper& AutoScrollHelper::setMinimumVelocity(float horizontalMin, float verticalMin) {
    mMinimumVelocity[HORIZONTAL] = horizontalMin / 1000.f;
    mMinimumVelocity[VERTICAL] = verticalMin / 1000.f;
    return *this;
}

AutoScrollHelper& AutoScrollHelper::setRelativeVelocity(float horizontal, float vertical){
    mRelativeVelocity[HORIZONTAL] = horizontal / 1000.f;
    mRelativeVelocity[VERTICAL] = vertical / 1000.f;
    return*this;
}

AutoScrollHelper& AutoScrollHelper::setEdgeType(int type){
    mEdgeType =type;
    return *this;
}

AutoScrollHelper& AutoScrollHelper::setRelativeEdges(float horizontal, float vertical) {
    mRelativeEdges[HORIZONTAL] = horizontal;
    mRelativeEdges[VERTICAL] = vertical;
    return *this;
}

AutoScrollHelper& AutoScrollHelper::setMaximumEdges(float horizontalMax, float verticalMax) {
    mMaximumEdges[HORIZONTAL] = horizontalMax;
    mMaximumEdges[VERTICAL] = verticalMax;
    return *this;
}
   
AutoScrollHelper& AutoScrollHelper::setActivationDelay(int delayMillis) {
    mActivationDelay = delayMillis;
    return *this;
}
    
AutoScrollHelper& AutoScrollHelper::setRampUpDuration(int durationMillis) {
    mScroller->setRampUpDuration(durationMillis);
    return *this;
}
  
AutoScrollHelper& AutoScrollHelper::setRampDownDuration(int durationMillis) {
    mScroller->setRampDownDuration(durationMillis);
    return *this;
}

bool AutoScrollHelper::onTouch(View& v, MotionEvent& event) {
    if (!mEnabled) {
        return false;
    }
    float xTargetVelocity=.0,yTargetVelocity=.0;
    int action = event.getActionMasked();
    switch (action) {
    case MotionEvent::ACTION_DOWN:
         mNeedsCancel = true;
         mAlreadyDelayed = false;
         // $FALL-THROUGH$
    case MotionEvent::ACTION_MOVE:
         xTargetVelocity = computeTargetVelocity(
                        HORIZONTAL, event.getX(), v.getWidth(), mTarget->getWidth());
         yTargetVelocity = computeTargetVelocity(
                        VERTICAL, event.getY(), v.getHeight(), mTarget->getHeight());
         mScroller->setTargetVelocity(xTargetVelocity, yTargetVelocity);

         // If the auto scroller was not previously active, but it should
         // be, then update the state and start animations.
         if (!mAnimating && shouldAnimate()) {
             startAnimating();
         }
         break;
    case MotionEvent::ACTION_UP:
    case MotionEvent::ACTION_CANCEL:
         requestStop();
         break;
    }
    return mExclusive && mAnimating;
}

bool AutoScrollHelper::shouldAnimate() {
   const int verticalDirection = mScroller->getVerticalDirection();
   const int horizontalDirection = mScroller->getHorizontalDirection();

   return verticalDirection != 0 && canTargetScrollVertically(verticalDirection)
           || horizontalDirection != 0 && canTargetScrollHorizontally(horizontalDirection);
}


void AutoScrollHelper::startAnimating() {
    if (mRunnable == nullptr) {
        mRunnable = std::bind(&AutoScrollHelper::animationRun,this);
    }

    mAnimating = true;
    mNeedsReset = true;

    if (!mAlreadyDelayed && mActivationDelay > 0) {
        mTarget->postOnAnimationDelayed(mRunnable, mActivationDelay);
    } else {
        mRunnable();
    }

    // If we start animating again before the user lifts their finger, we
    // already know it's not a tap and don't need an activation delay.
    mAlreadyDelayed = true;
}

void AutoScrollHelper::requestStop() {
    if (mNeedsReset) {
        // The animation has been posted, but hasn't run yet. Manually
        // stopping animation will prevent it from running.
        mAnimating = false;
    } else {
        mScroller->requestStop();
    }
}

float AutoScrollHelper::computeTargetVelocity(int direction, float coordinate, float srcSize, float dstSize) {
    float relativeEdge = mRelativeEdges[direction];
    float maximumEdge = mMaximumEdges[direction];
    float value = getEdgeValue(relativeEdge, srcSize, maximumEdge, coordinate);
    if (value == 0) {
        // The edge in this direction is not activated.
       return 0;
    }

    float relativeVelocity = mRelativeVelocity[direction];
    float minimumVelocity = mMinimumVelocity[direction];
    float maximumVelocity = mMaximumVelocity[direction];
    float targetVelocity = relativeVelocity * dstSize;

    // Target velocity is adjusted for interpolated edge position, then
    // clamped to the minimum and maximum values. Later, this value will be
    // adjusted for time-based acceleration.
    if (value > 0) {
        return MathUtils::constrain(value * targetVelocity, minimumVelocity, maximumVelocity);
    } else {
        return -MathUtils::constrain(-value * targetVelocity, minimumVelocity, maximumVelocity);
    }
}

float AutoScrollHelper::getEdgeValue(float relativeValue, float size, float maxValue, float current) {
    // For now, leading and trailing edges are always the same size.
    const float edgeSize = MathUtils::constrain(relativeValue * size, NO_MIN, maxValue);
    const float valueLeading = constrainEdgeValue(current, edgeSize);
    const float valueTrailing = constrainEdgeValue(size - current, edgeSize);
    const float value = (valueTrailing - valueLeading);
    float interpolated;
    if (value < 0) {
        interpolated = -mEdgeInterpolator->getInterpolation(-value);
    } else if (value > 0) {
        interpolated = mEdgeInterpolator->getInterpolation(value);
    } else {
        return 0;
    }

    return MathUtils::constrain(interpolated, -1.f, 1.f);
}

float AutoScrollHelper::constrainEdgeValue(float current, float leading) {
    if (leading == 0) return 0;

    switch (mEdgeType) {
    case EDGE_TYPE_INSIDE:
    case EDGE_TYPE_INSIDE_EXTEND:
        if (current < leading) {
           if (current >= 0) {
               // Movement up to the edge is scaled.
               return 1.f - current / leading;
           } else if (mAnimating && (mEdgeType == EDGE_TYPE_INSIDE_EXTEND)) {
               // Movement beyond the edge is always maximum.
               return 1.f;
           }
        }
        break;
    case EDGE_TYPE_OUTSIDE:
        if (current < 0) {
           // Movement beyond the edge is scaled.
           return current / -leading;
        }
        break;
    }
    return 0;
}

void AutoScrollHelper::cancelTargetTouch() {
    const auto eventTime = SystemClock::uptimeMillis();
    MotionEvent* cancel = MotionEvent::obtain(
            eventTime, eventTime, MotionEvent::ACTION_CANCEL, 0, 0, 0);
    mTarget->onTouchEvent(*cancel);
    cancel->recycle();
}

void AutoScrollHelper::animationRun(){
    if (!mAnimating) {
        return;
    }
    if (mNeedsReset) {
        mNeedsReset = false;
        mScroller->start();
    }

    ClampedScroller* scroller = mScroller;
    if (scroller->isFinished() || !shouldAnimate()) {
        mAnimating = false;
        return;
    }

    if (mNeedsCancel) {
        mNeedsCancel = false;
        cancelTargetTouch();
    }

    scroller->computeScrollDelta();

    const int deltaX = scroller->getDeltaX();
    const int deltaY = scroller->getDeltaY();
    scrollTargetBy(deltaX,  deltaY);

    // Keep going until the scroller has permanently stopped.
    mTarget->postOnAnimation(mRunnable);
}
/////////////////////////////////////////////////////////////////////////////////////
AutoScrollHelper::ClampedScroller::ClampedScroller() {
    mStartTime = INT64_MIN;//Long.MIN_VALUE;
    mStopTime = -1;
    mDeltaTime = 0;
    mDeltaX = 0;
    mDeltaY = 0;
}

void AutoScrollHelper::ClampedScroller::setRampUpDuration(int durationMillis) {
    mRampUpDuration = durationMillis;
}

void AutoScrollHelper::ClampedScroller::setRampDownDuration(int durationMillis) {
    mRampDownDuration = durationMillis;
}

void AutoScrollHelper::ClampedScroller::start() {
    mStartTime = AnimationUtils::currentAnimationTimeMillis();
    mStopTime = -1;
    mDeltaTime = mStartTime;
    mStopValue = 0.5f;
    mDeltaX = 0;
    mDeltaY = 0;
}


void AutoScrollHelper::ClampedScroller::requestStop() {
    int64_t currentTime = AnimationUtils::currentAnimationTimeMillis();
    mEffectiveRampDown = MathUtils::constrain((int) (currentTime - mStartTime), 0, mRampDownDuration);
    mStopValue = getValueAt(currentTime);
    mStopTime = currentTime;
}

bool AutoScrollHelper::ClampedScroller::isFinished() {
    return mStopTime > 0
          && AnimationUtils::currentAnimationTimeMillis() > mStopTime + mEffectiveRampDown;
}

float AutoScrollHelper::ClampedScroller::getValueAt(int64_t currentTime) {
    if (currentTime < mStartTime) {
        return .0f;
    } else if (mStopTime < 0 || currentTime < mStopTime) {
        int64_t elapsedSinceStart = currentTime - mStartTime;
        return 0.5f * MathUtils::constrain(elapsedSinceStart / (float) mRampUpDuration, .0f, 1.f);
    } else {
        int64_t elapsedSinceEnd = currentTime - mStopTime;
        return (1.f - mStopValue) + mStopValue
                * MathUtils::constrain(elapsedSinceEnd / (float) mEffectiveRampDown, .0f, 1.f);
    }
}

float AutoScrollHelper::ClampedScroller::interpolateValue(float value) {
    return -4 * value * value + 4 * value;
}

void AutoScrollHelper::ClampedScroller::computeScrollDelta() {
    if (mDeltaTime == 0) {
        throw "Cannot compute scroll delta before calling start()";
    }

    const int64_t currentTime = AnimationUtils::currentAnimationTimeMillis();
    float value = getValueAt(currentTime);
    float scale = interpolateValue(value);
    int64_t elapsedSinceDelta = currentTime - mDeltaTime;

    mDeltaTime = currentTime;
    mDeltaX = (int) (elapsedSinceDelta * scale * mTargetVelocityX);
    mDeltaY = (int) (elapsedSinceDelta * scale * mTargetVelocityY);
}

void AutoScrollHelper::ClampedScroller::setTargetVelocity(float x, float y) {
    mTargetVelocityX = x;
    mTargetVelocityY = y;
}

int AutoScrollHelper::ClampedScroller::getHorizontalDirection()const{
    return (int) (mTargetVelocityX / std::abs(mTargetVelocityX));
}

int AutoScrollHelper::ClampedScroller::getVerticalDirection()const{
    return (int) (mTargetVelocityY / std::abs(mTargetVelocityY));
}

int AutoScrollHelper::ClampedScroller::getDeltaX()const {
    return mDeltaX;
}

int AutoScrollHelper::ClampedScroller::getDeltaY()const {
    return mDeltaY;
}

////////////////////////////////////////////////////////////////////////////////////////////
AbsListViewAutoScroller::AbsListViewAutoScroller(AbsListView* target):AutoScrollHelper(target){
    mTarget = target;
}

void AbsListViewAutoScroller::scrollTargetBy(int deltaX, int deltaY) {
    ((AbsListView*)mTarget)->scrollListBy(deltaY);
}

bool AbsListViewAutoScroller::canTargetScrollHorizontally(int direction) {
    // List do not scroll horizontally.
    return false;
}

bool AbsListViewAutoScroller::canTargetScrollVertically(int direction) {
    AbsListView* target = (AbsListView*)mTarget;
    int itemCount = target->getCount();
    if (itemCount == 0) {
        return false;
    }

    int childCount = target->getChildCount();
    int firstPosition = target->getFirstVisiblePosition();
    int lastPosition = firstPosition + childCount;

    if (direction > 0) {
        // Are we already showing the entire last item?
        if (lastPosition >= itemCount) {
            View* lastView = target->getChildAt(childCount - 1);
            if (lastView->getBottom() <= target->getHeight()) {
                return false;
            }
        }
    } else if (direction < 0) {
        // Are we already showing the entire first item?
        if (firstPosition <= 0) {
            View* firstView = target->getChildAt(0);
            if (firstView->getTop() >= 0) {
                return false;
            }
        }
    } else {
        // The behavior for direction 0 is undefined and we can return
        // whatever we want.
        return false;
    }
    return true;
}
}
