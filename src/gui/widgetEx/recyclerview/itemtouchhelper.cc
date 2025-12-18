/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *********************************************************************************/
#include <view/gesturedetector.h>
#include <widgetEx/recyclerview/itemtouchhelper.h>
#include <core/build.h>
#include <utils/mathutils.h>

namespace cdroid{

class DragScrollInterpolator:public Interpolator{
public:
    DragScrollInterpolator(){}
    float getInterpolation(float t)const override{
        return t * t * t * t * t;
    }
};

class DragViewScrollInterpolator:public Interpolator{
public:
    DragViewScrollInterpolator(){}
    float getInterpolation(float t)const override{
        t -= 1.0f;
        return t * t * t * t * t + 1.0f;
    }
};
static DragScrollInterpolator sDragScrollInterpolator;
static DragViewScrollInterpolator sDragViewScrollCapInterpolator;

ItemTouchHelper::ItemTouchHelper(Callback* callback) {
    mCallback = callback;
    mSelected = nullptr;
    mRecyclerView = nullptr;
    mOverdrawChild= nullptr;
    mVelocityTracker = nullptr;
    mSwipeEscapeVelocity = 120;
    mMaxSwipeVelocity = 800;
    mOverdrawChildPosition = -1;
    mGestureDetector = nullptr;
    mShouldReactToLongPress = true;

    mScrollRunnable = [this]() {
        if ((mSelected != nullptr) && scrollIfNecessary()) {
            if (mSelected != nullptr) { //it might be lost during scrolling
                moveIfNecessary(*mSelected);
            }
            mRecyclerView->removeCallbacks(mScrollRunnable);
            mRecyclerView->postOnAnimation(mScrollRunnable);
        }
    };
    mOnChildAttachStateChangeListener.onChildViewAttachedToWindow = std::bind(&ItemTouchHelper::onChildViewAttachedToWindow,this,std::placeholders::_1);
    mOnChildAttachStateChangeListener.onChildViewDetachedFromWindow = std::bind(&ItemTouchHelper::onChildViewDetachedFromWindow,this,std::placeholders::_1);

    mOnItemTouchListener.onInterceptTouchEvent = std::bind(&ItemTouchHelper::onInterceptTouchEvent,this,std::placeholders::_1,std::placeholders::_2);
    mOnItemTouchListener.onTouchEvent = std::bind(&ItemTouchHelper::onTouchEvent,this,std::placeholders::_1,std::placeholders::_2);
    mOnItemTouchListener.onRequestDisallowInterceptTouchEvent = std::bind(&ItemTouchHelper::onRequestDisallowInterceptTouchEvent,this,std::placeholders::_1);
}

ItemTouchHelper::~ItemTouchHelper(){
    if(mRecyclerView!=nullptr){
        destroyCallbacks();
    }
#if ENABLE(GESTURE)
    delete mGestureDetector;
#endif
}

bool ItemTouchHelper::onInterceptTouchEvent(RecyclerView& recyclerView,MotionEvent& event) {
#if ENABLE(GESTURE)
    mGestureDetector->onTouchEvent(event);
#endif
    LOGD_IF(_Debug,"intercept: x:%.f ,y:%.f",event.getX(),event.getY());
    const int action = event.getActionMasked();
    if (action == MotionEvent::ACTION_DOWN) {
        mActivePointerId = event.getPointerId(0);
        mInitialTouchX = event.getX();
        mInitialTouchY = event.getY();
        obtainVelocityTracker();
        if (mSelected == nullptr) {
            RecoverAnimation* animation = findAnimation(event);
            if (animation != nullptr) {
                mInitialTouchX -= animation->mX;
                mInitialTouchY -= animation->mY;
                endRecoverAnimation(*animation->mViewHolder, true);
                auto it = std::find(mPendingCleanup.begin(),mPendingCleanup.end(),animation->mViewHolder->itemView);
                if (it != mPendingCleanup.end()){//remove(animation->mViewHolder->itemView)) {
                    mPendingCleanup.erase(it);
                    mCallback->clearView(*mRecyclerView, *animation->mViewHolder);
                }
                select(animation->mViewHolder, animation->mActionState);
                updateDxDy(event, mSelectedFlags, 0);
            }
        }
    } else if (action == MotionEvent::ACTION_CANCEL || action == MotionEvent::ACTION_UP) {
        mActivePointerId = ACTIVE_POINTER_ID_NONE;
        select(nullptr, ACTION_STATE_IDLE);
    } else if (mActivePointerId != ACTIVE_POINTER_ID_NONE) {
        // in a non scroll orientation, if distance change is above threshold, we
        // can select the item
        const int index = event.findPointerIndex(mActivePointerId);
        LOGD_IF(_Debug,"pointer index %d",index);
        if (index >= 0) {
            checkSelectForSwipe(action, event, index);
        }
    }
    if (mVelocityTracker != nullptr) {
        mVelocityTracker->addMovement(event);
    }
    return mSelected != nullptr;
}

void ItemTouchHelper::onTouchEvent(RecyclerView& recyclerView,MotionEvent& event) {
#if ENABLE(GESTURE)
    mGestureDetector->onTouchEvent(event);
#endif
    LOGD_IF(_Debug,"on touch: x:%d,y:%d",mInitialTouchX,mInitialTouchY);
    if (mVelocityTracker != nullptr) {
        mVelocityTracker->addMovement(event);
    }
    if (mActivePointerId == ACTIVE_POINTER_ID_NONE) {
        return;
    }
    const int action = event.getActionMasked();
    const int activePointerIndex = event.findPointerIndex(mActivePointerId);
    if (activePointerIndex >= 0) {
        checkSelectForSwipe(action, event, activePointerIndex);
    }
    RecyclerView::ViewHolder* viewHolder = mSelected;
    if (viewHolder == nullptr) {
        return;
    }
    switch (action) {
    case MotionEvent::ACTION_MOVE:
        // Find the index of the active pointer and fetch its position
        if (activePointerIndex >= 0) {
            updateDxDy(event, mSelectedFlags, activePointerIndex);
            moveIfNecessary(*viewHolder);
            mRecyclerView->removeCallbacks(mScrollRunnable);
            mScrollRunnable();//.run();
            mRecyclerView->invalidate();
        }
        break;
    case MotionEvent::ACTION_CANCEL:
        if (mVelocityTracker != nullptr) {
            mVelocityTracker->clear();
        }
        // fall through
    case MotionEvent::ACTION_UP:
        select(nullptr, ACTION_STATE_IDLE);
        mActivePointerId = ACTIVE_POINTER_ID_NONE;
        break;
    case MotionEvent::ACTION_POINTER_UP: {
            const int pointerIndex = event.getActionIndex();
            const int pointerId = event.getPointerId(pointerIndex);
            if (pointerId == mActivePointerId) {
                // This was our active pointer going up. Choose a new
                // active pointer and adjust accordingly.
                const int newPointerIndex = pointerIndex == 0 ? 1 : 0;
                mActivePointerId = event.getPointerId(newPointerIndex);
                updateDxDy(event, mSelectedFlags, pointerIndex);
            }
            break;
        }
    }
}

void ItemTouchHelper::onRequestDisallowInterceptTouchEvent(bool disallowIntercept) {
    if (!disallowIntercept) {
        return;
    }
    select(nullptr, ACTION_STATE_IDLE);
}

bool ItemTouchHelper::hitTest(View& child, float x, float y, float left, float top) {
    return (x >= left)  && (x <= left + child.getWidth())
            && (y >= top) && (y <= top + child.getHeight());
}

void ItemTouchHelper::attachToRecyclerView(RecyclerView* recyclerView) {
    if (mRecyclerView == recyclerView) {
        return; // nothing to do
    }
    if (mRecyclerView != nullptr) {
        destroyCallbacks();
    }
    mRecyclerView = recyclerView;
    if (recyclerView != nullptr) {
        cdroid::Context*ctx = recyclerView->getContext();
        mSwipeEscapeVelocity = ctx->getDimension("cdroid:dimen/item_touch_helper_swipe_escape_velocity");
        mMaxSwipeVelocity = ctx->getDimension("cdroid:dimen/item_touch_helper_swipe_escape_max_velocity");
        setupCallbacks();
    }
}

void ItemTouchHelper::setupCallbacks() {
    ViewConfiguration& vc = ViewConfiguration::get(mRecyclerView->getContext());
    mSlop = vc.getScaledTouchSlop();
    mRecyclerView->addItemDecoration(this);
    mRecyclerView->addOnItemTouchListener(mOnItemTouchListener);
    mRecyclerView->addOnChildAttachStateChangeListener(mOnChildAttachStateChangeListener);
    startGestureDetection();
}

void ItemTouchHelper::destroyCallbacks() {
    mRecyclerView->removeItemDecoration(this);
    mRecyclerView->removeOnItemTouchListener(mOnItemTouchListener);
    mRecyclerView->removeOnChildAttachStateChangeListener(mOnChildAttachStateChangeListener);
    // clean all attached
    const int recoverAnimSize = (int)mRecoverAnimations.size();
    for (int i = recoverAnimSize - 1; i >= 0; i--) {
        RecoverAnimation* recoverAnimation = mRecoverAnimations.at(0);
        recoverAnimation->cancel();
        mCallback->clearView(*mRecyclerView, *recoverAnimation->mViewHolder);
        delete recoverAnimation;
    }
    mRecoverAnimations.clear();
    mOverdrawChild = nullptr;
    mOverdrawChildPosition = -1;
    releaseVelocityTracker();
    stopGestureDetection();
}

void ItemTouchHelper::startGestureDetection() {
#if ENABLE(GESTURE)
    mItemTouchHelperGestureListener.onDown = std::bind(&ItemTouchHelper::onGestureDown,this,std::placeholders::_1);
    mItemTouchHelperGestureListener.onLongPress = std::bind(&ItemTouchHelper::onGestureLongPress,this,std::placeholders::_1);
    mGestureDetector = new GestureDetector(mRecyclerView->getContext(),mItemTouchHelperGestureListener);
#endif
}

void ItemTouchHelper::stopGestureDetection() {
    if (mItemTouchHelperGestureListener.onDown||mItemTouchHelperGestureListener.onLongPress) {
        doNotReactToLongPress();
        mItemTouchHelperGestureListener ={};
    }
#if ENABLE(GESTURE)
    delete mGestureDetector;
    mGestureDetector = nullptr;
#endif
}

void ItemTouchHelper::getSelectedDxDy(float outPosition[2]) {
    if ((mSelectedFlags & (LEFT | RIGHT)) != 0) {
        outPosition[0] = mSelectedStartX + mDx - mSelected->itemView->getLeft();
    } else {
        outPosition[0] = mSelected->itemView->getTranslationX();
    }
    if ((mSelectedFlags & (UP | DOWN)) != 0) {
        outPosition[1] = mSelectedStartY + mDy - mSelected->itemView->getTop();
    } else {
        outPosition[1] = mSelected->itemView->getTranslationY();
    }
}

void ItemTouchHelper::onDrawOver(Canvas& c, RecyclerView& parent, RecyclerView::State& state) {
    float dx = 0, dy = 0;
    if (mSelected != nullptr) {
        getSelectedDxDy(mTmpPosition);
        dx = mTmpPosition[0];
        dy = mTmpPosition[1];
    }
    mCallback->onDrawOver(c, parent, mSelected, mRecoverAnimations, mActionState, dx, dy);
}

void ItemTouchHelper::onDraw(Canvas& c, RecyclerView& parent, RecyclerView::State& state) {
    // we don't know if RV changed something so we should invalidate this index.
    mOverdrawChildPosition = -1;
    float dx = 0, dy = 0;
    if (mSelected != nullptr) {
        getSelectedDxDy(mTmpPosition);
        dx = mTmpPosition[0];
        dy = mTmpPosition[1];
    }
    mCallback->onDraw(c, parent,mSelected, mRecoverAnimations, mActionState, dx, dy);
}

class MyRecoverAnimation:public ItemTouchHelper::RecoverAnimation{
private:
    std::function<void(Animator&,bool)>mOnAnimationEndListener;
public:
    Runnable mRunnable;
public:
    MyRecoverAnimation(RecyclerView::ViewHolder* viewHolder, int animationType,
        int actionState, float startDx, float startDy, float targetX, float targetY)
       :ItemTouchHelper::RecoverAnimation(viewHolder,animationType,actionState,startDx,startDy,targetX,targetY){
    }
    void setListener(std::function<void(Animator&,bool)>func){
         mOnAnimationEndListener =func;
    }
    void onAnimationEnd(Animator& animation,bool isReverse)override{
        RecoverAnimation::onAnimationEnd(animation,isReverse);
        mOnAnimationEndListener(animation,isReverse);
    }
};
void ItemTouchHelper::select(RecyclerView::ViewHolder* selected, int actionState) {
    if ( (selected == mSelected) && (actionState == mActionState) ) {
        return;
    }
    mDragScrollStartTimeInMs = LLONG_MIN;
    const int prevActionState = mActionState;
    // prevent duplicate animations
    endRecoverAnimation(*selected, true);
    mActionState = actionState;
    if (actionState == ACTION_STATE_DRAG) {
        FATAL_IF(selected == nullptr,"Must pass a ViewHolder when dragging");

        // we remove after animation is complete. this means we only elevate the last drag
        // child but that should perform good enough as it is very hard to start dragging a
        // new child before the previous one settles.
        mOverdrawChild = selected->itemView;
        addChildDrawingOrderCallback();
    }
    const int actionStateMask = (1 << (DIRECTION_FLAG_COUNT + DIRECTION_FLAG_COUNT * actionState)) - 1;
    bool preventLayout = false;

    if (mSelected != nullptr) {
        RecyclerView::ViewHolder* prevSelected = mSelected;
        if (prevSelected->itemView->getParent()) {
            const int swipeDir = prevActionState == ACTION_STATE_DRAG ? 0
                    : swipeIfNecessary(*prevSelected);
            releaseVelocityTracker();
            // find where we should animate to
            float targetTranslateX, targetTranslateY;
            int animationType;
            switch (swipeDir) {
            case LEFT:
            case RIGHT:
            case START:
            case END:
                targetTranslateY = 0;
                targetTranslateX = MathUtils::signum(mDx) * mRecyclerView->getWidth();
                break;
            case UP:
            case DOWN:
                targetTranslateX = 0;
                targetTranslateY = MathUtils::signum(mDy) * mRecyclerView->getHeight();
                break;
            default:
                targetTranslateX = 0;
                targetTranslateY = 0;
            }
            if (prevActionState == ACTION_STATE_DRAG) {
                animationType = ANIMATION_TYPE_DRAG;
            } else if (swipeDir > 0) {
                animationType = ANIMATION_TYPE_SWIPE_SUCCESS;
            } else {
                animationType = ANIMATION_TYPE_SWIPE_CANCEL;
            }
            getSelectedDxDy(mTmpPosition);
            const float currentTranslateX = mTmpPosition[0];
            const float currentTranslateY = mTmpPosition[1];
            MyRecoverAnimation* rca = new MyRecoverAnimation(prevSelected, animationType,
                      prevActionState, currentTranslateX, currentTranslateY,targetTranslateX, targetTranslateY);
            rca->setListener([this,rca,swipeDir,prevSelected](Animator& animation,bool isReverse) {
                    if (rca->mOverridden) return;
                    if (swipeDir <= 0) {
                        // this is a drag or failed swipe. recover immediately
                        mCallback->clearView(*mRecyclerView,*prevSelected);
                        // full cleanup will happen on onDrawOver
                    } else {
                        // wait until remove animation is complete.
                        mPendingCleanup.push_back(prevSelected->itemView);
                        rca->mIsPendingCleanup = true;
                        // Animation might be ended by other animators during a layout.
                        // We defer callback to avoid editing adapter during a layout.
                        postDispatchSwipe(rca, swipeDir);
                    }
                    // removed from the list after it is drawn for the last time
                    if (mOverdrawChild == prevSelected->itemView) {
                        removeChildDrawingOrderCallbackIfNecessary(prevSelected->itemView);
                    }
                });
            const long duration = mCallback->getAnimationDuration(*mRecyclerView, animationType,
                    targetTranslateX - currentTranslateX, targetTranslateY - currentTranslateY);
            rca->setDuration(duration);
            mRecoverAnimations.push_back(rca);
            rca->start();
            preventLayout = true;
        } else {
            removeChildDrawingOrderCallbackIfNecessary(prevSelected->itemView);
            mCallback->clearView(*mRecyclerView, *prevSelected);
        }
        mSelected = nullptr;
    }
    if (selected != nullptr) {
        mSelectedFlags = (mCallback->getAbsoluteMovementFlags(*mRecyclerView, *selected) & actionStateMask)
                        >> (mActionState * DIRECTION_FLAG_COUNT);
        mSelectedStartX = selected->itemView->getLeft();
        mSelectedStartY = selected->itemView->getTop();
        mSelected = selected;

        if (actionState == ACTION_STATE_DRAG) {
            mSelected->itemView->performHapticFeedback(HapticFeedbackConstants::LONG_PRESS);
        }
    }
    ViewGroup* rvParent = mRecyclerView->getParent();
    if (rvParent != nullptr) {
        rvParent->requestDisallowInterceptTouchEvent(mSelected != nullptr);
    }
    if (!preventLayout) {
        mRecyclerView->getLayoutManager()->requestSimpleAnimationsInNextLayout();
    }
    mCallback->onSelectedChanged(mSelected, mActionState);
    mRecyclerView->invalidate();
}

void ItemTouchHelper::postDispatchSwipe(RecoverAnimation* anim,int swipeDir) {
    // wait until animations are complete.
    MyRecoverAnimation*myAnim = dynamic_cast<MyRecoverAnimation*>(anim);
    myAnim->mRunnable = ([this,anim,swipeDir]() {
        if (mRecyclerView && mRecyclerView->isAttachedToWindow() && !anim->mOverridden
                && (anim->mViewHolder->getAbsoluteAdapterPosition() != RecyclerView::NO_POSITION) ) {
            RecyclerView::ItemAnimator* animator = mRecyclerView->getItemAnimator();
            // if animator is running or we have other active recover animations, we try
            // not to call onSwiped because DefaultItemAnimator is not good at merging
            // animations. Instead, we wait and batch.
            if ((animator == nullptr || !animator->isRunning(nullptr))
                    && !hasRunningRecoverAnim()) {
                mCallback->onSwiped(*anim->mViewHolder, swipeDir);
            } else {
                mRecyclerView->post(dynamic_cast<MyRecoverAnimation*>(anim)->mRunnable);
            }
        }
    });
    mRecyclerView->post(myAnim->mRunnable);
}

bool ItemTouchHelper::hasRunningRecoverAnim() const{
    const int size = mRecoverAnimations.size();
    for (int i = 0; i < size; i++) {
        if (!mRecoverAnimations.at(i)->mEnded) {
            return true;
        }
    }
    return false;
}

/**
 * If user drags the view to the edge, trigger a scroll if necessary.
 */
bool ItemTouchHelper::scrollIfNecessary() {
    if (mSelected == nullptr) {
        mDragScrollStartTimeInMs = LLONG_MIN;
        return false;
    }
    const int64_t now = SystemClock::currentTimeMillis();
    const int64_t scrollDuration = mDragScrollStartTimeInMs
            == LLONG_MIN ? 0 : now - mDragScrollStartTimeInMs;
    RecyclerView::LayoutManager* lm = mRecyclerView->getLayoutManager();
    Rect mTmpRect;
    int scrollX = 0;
    int scrollY = 0;
    lm->calculateItemDecorationsForChild(mSelected->itemView, mTmpRect);
    if (lm->canScrollHorizontally()) {
        int curX = (int) (mSelectedStartX + mDx);
        const int leftDiff = curX - mTmpRect.left - mRecyclerView->getPaddingLeft();
        if (mDx < 0 && leftDiff < 0) {
            scrollX = leftDiff;
        } else if (mDx > 0) {
            const int rightDiff = curX + mSelected->itemView->getWidth() + mTmpRect.width
                            - (mRecyclerView->getWidth() - mRecyclerView->getPaddingRight());
            if (rightDiff > 0) {
                scrollX = rightDiff;
            }
        }
    }
    if (lm->canScrollVertically()) {
        int curY = (int) (mSelectedStartY + mDy);
        const int topDiff = curY - mTmpRect.top - mRecyclerView->getPaddingTop();
        if ((mDy < 0) && (topDiff < 0)) {
            scrollY = topDiff;
        } else if (mDy > 0) {
            const int bottomDiff = curY + mSelected->itemView->getHeight() + mTmpRect.height
                    - (mRecyclerView->getHeight() - mRecyclerView->getPaddingBottom());
            if (bottomDiff > 0) {
                scrollY = bottomDiff;
            }
        }
    }
    if (scrollX != 0) {
        scrollX = mCallback->interpolateOutOfBoundsScroll(*mRecyclerView,
                mSelected->itemView->getWidth(), scrollX,
                mRecyclerView->getWidth(), scrollDuration);
    }
    if (scrollY != 0) {
        scrollY = mCallback->interpolateOutOfBoundsScroll(*mRecyclerView,
                mSelected->itemView->getHeight(), scrollY,
                mRecyclerView->getHeight(), scrollDuration);
    }
    if ((scrollX != 0) || (scrollY != 0)) {
        if (mDragScrollStartTimeInMs == LLONG_MIN) {
            mDragScrollStartTimeInMs = now;
        }
        mRecyclerView->scrollBy(scrollX, scrollY);
        return true;
    }
    mDragScrollStartTimeInMs = LLONG_MIN;//Long.MIN_VALUE;
    return false;
}

std::vector<RecyclerView::ViewHolder*> ItemTouchHelper::findSwapTargets(RecyclerView::ViewHolder* viewHolder) {
    mSwapTargets.clear();
    mDistances.clear();
    const int margin = mCallback->getBoundingBoxMargin();
    const int left = std::round(mSelectedStartX + mDx) - margin;
    const int top  = std::round(mSelectedStartY + mDy) - margin;
    const int right  = left + viewHolder->itemView->getWidth() + 2 * margin;
    const int bottom = top + viewHolder->itemView->getHeight() + 2 * margin;
    const int centerX = (left + right) / 2;
    const int centerY = (top + bottom) / 2;
    RecyclerView::LayoutManager* lm = mRecyclerView->getLayoutManager();
    const int childCount = lm->getChildCount();
    for (int i = 0; i < childCount; i++) {
        View* other = lm->getChildAt(i);
        if (other == viewHolder->itemView) {
            continue; //myself!
        }
        if ((other->getBottom() < top) || (other->getTop() > bottom)
                || (other->getRight() < left) || (other->getLeft() > right)) {
            continue;
        }
        RecyclerView::ViewHolder* otherVh = mRecyclerView->getChildViewHolder(other);
        if (mCallback->canDropOver(*mRecyclerView, *mSelected, *otherVh)) {
            // find the index to add
            const int dx = std::abs(centerX - (other->getLeft() + other->getRight()) / 2);
            const int dy = std::abs(centerY - (other->getTop() + other->getBottom()) / 2);
            const int dist = dx * dx + dy * dy;

            int pos = 0;
            const int cnt = mSwapTargets.size();
            for (int j = 0; j < cnt; j++) {
                if (dist > mDistances.at(j)) {
                    pos++;
                } else {
                    break;
                }
            }
            mSwapTargets.insert( mSwapTargets.begin()+pos,otherVh);//add(pos, otherVh);
            mDistances.insert(mDistances.begin()+pos,dist);//add(pos, dist);
        }
    }
    return mSwapTargets;
}

/**
 * Checks if we should swap w/ another view holder.
 */
void ItemTouchHelper::moveIfNecessary(RecyclerView::ViewHolder& viewHolder) {
    if (mRecyclerView->isLayoutRequested()) {
        return;
    }
    if (mActionState != ACTION_STATE_DRAG) {
        return;
    }

    const float threshold = mCallback->getMoveThreshold(viewHolder);
    const int x = (int) (mSelectedStartX + mDx);
    const int y = (int) (mSelectedStartY + mDy);
    if ((std::abs(y - viewHolder.itemView->getTop()) < viewHolder.itemView->getHeight() * threshold)
            && (std::abs(x - viewHolder.itemView->getLeft()) < viewHolder.itemView->getWidth() * threshold)) {
        return;
    }
    std::vector<RecyclerView::ViewHolder*> swapTargets = findSwapTargets(&viewHolder);
    if (swapTargets.size() == 0) {
        return;
    }
    // may swap.
    RecyclerView::ViewHolder* target = mCallback->chooseDropTarget(viewHolder, swapTargets, x, y);
    if (target == nullptr) {
        mSwapTargets.clear();
        mDistances.clear();
        return;
    }
    const int toPosition = target->getAbsoluteAdapterPosition();
    const int fromPosition = viewHolder.getAbsoluteAdapterPosition();
    if (mCallback->onMove(*mRecyclerView, viewHolder, *target)) {
        // keep target visible
        mCallback->onMoved(*mRecyclerView, viewHolder, fromPosition,
                *target, toPosition, x, y);
    }
}


void ItemTouchHelper::onChildViewAttachedToWindow(View& view) {
}

void ItemTouchHelper::onChildViewDetachedFromWindow(View& view) {
    removeChildDrawingOrderCallbackIfNecessary(&view);
    RecyclerView::ViewHolder* holder = mRecyclerView->getChildViewHolder(&view);
    if (holder == nullptr) {
        return;
    }
    if ((mSelected != nullptr) && (holder == mSelected)) {
        select(nullptr, ACTION_STATE_IDLE);
    } else {
        endRecoverAnimation(*holder, false); // this may push it into pending cleanup list.
        auto it = std::find(mPendingCleanup.begin(),mPendingCleanup.end(),holder->itemView);
        if (it!=mPendingCleanup.end()) {
            mCallback->clearView(*mRecyclerView, *holder);
        }
    }
}

/**
 * Returns the animation type or 0 if cannot be found.
 */
void ItemTouchHelper::endRecoverAnimation(RecyclerView::ViewHolder& viewHolder, bool overrided) {
    const size_t recoverAnimSize = mRecoverAnimations.size();
    for (int i = int(recoverAnimSize - 1); i >= 0; i--) {
        RecoverAnimation* anim = mRecoverAnimations.at(i);
        if (anim->mViewHolder == &viewHolder) {
            anim->mOverridden |= overrided;
            if (!anim->mEnded) {
                anim->cancel();
            }
            mRecoverAnimations.erase(mRecoverAnimations.begin()+i);//remove(i);
            delete anim;
            return;
        }
    }
}

void ItemTouchHelper::getItemOffsets(Rect& outRect, View& view, RecyclerView& parent,
        RecyclerView::State& state) {
    outRect.setEmpty();
}

void ItemTouchHelper::obtainVelocityTracker() {
    if (mVelocityTracker != nullptr) {
        mVelocityTracker->recycle();
    }
    mVelocityTracker = VelocityTracker::obtain();
}

void ItemTouchHelper::releaseVelocityTracker() {
    if (mVelocityTracker != nullptr) {
        mVelocityTracker->recycle();
        mVelocityTracker = nullptr;
    }
}

RecyclerView::ViewHolder* ItemTouchHelper::findSwipedView(MotionEvent& motionEvent) {
    RecyclerView::LayoutManager* lm = mRecyclerView->getLayoutManager();
    if (mActivePointerId == ACTIVE_POINTER_ID_NONE) {
        return nullptr;
    }
    const int pointerIndex = motionEvent.findPointerIndex(mActivePointerId);
    const float dx = motionEvent.getX(pointerIndex) - mInitialTouchX;
    const float dy = motionEvent.getY(pointerIndex) - mInitialTouchY;
    const float absDx = std::abs(dx);
    const float absDy = std::abs(dy);

    if ((absDx < mSlop) && (absDy < mSlop)) {
        return nullptr;
    }
    if ((absDx > absDy) && lm->canScrollHorizontally()) {
        return nullptr;
    } else if ((absDy > absDx) && lm->canScrollVertically()) {
        return nullptr;
    }
    View* child = findChildView(motionEvent);
    if (child == nullptr) {
        return nullptr;
    }
    return mRecyclerView->getChildViewHolder(child);
}

void ItemTouchHelper::checkSelectForSwipe(int action, MotionEvent& motionEvent, int pointerIndex) {
    if ((mSelected != nullptr) || (action != MotionEvent::ACTION_MOVE)
            || (mActionState == ACTION_STATE_DRAG) || !mCallback->isItemViewSwipeEnabled()) {
        return;
    }
    if (mRecyclerView->getScrollState() == RecyclerView::SCROLL_STATE_DRAGGING) {
        return;
    }
    RecyclerView::ViewHolder* vh = findSwipedView(motionEvent);
    if (vh == nullptr) {
        return;
    }
    const int movementFlags = mCallback->getAbsoluteMovementFlags(*mRecyclerView, *vh);

    const int swipeFlags = (movementFlags & ACTION_MODE_SWIPE_MASK)
            >> (DIRECTION_FLAG_COUNT * ACTION_STATE_SWIPE);

    if (swipeFlags == 0) {
        return;
    }

    // mDx and mDy are only set in allowed directions. We use custom x/y here instead of
    // updateDxDy to avoid swiping if user moves more in the other direction
    const float x = motionEvent.getX(pointerIndex);
    const float y = motionEvent.getY(pointerIndex);

    // Calculate the distance moved
    const float dx = x - mInitialTouchX;
    const float dy = y - mInitialTouchY;
    // swipe target is chose w/o applying flags so it does not really check if swiping in that
    // direction is allowed. This why here, we use mDx mDy to check slope value again.
    const float absDx = std::abs(dx);
    const float absDy = std::abs(dy);

    if (absDx < mSlop && absDy < mSlop) {
        return;
    }
    if (absDx > absDy) {
        if (dx < 0 && ((swipeFlags & LEFT) == 0)) {
            return;
        }
        if (dx > 0 && ((swipeFlags & RIGHT) == 0)) {
            return;
        }
    } else {
        if (dy < 0 && ((swipeFlags & UP) == 0)) {
            return;
        }
        if (dy > 0 && ((swipeFlags & DOWN) == 0)) {
            return;
        }
    }
    mDx = mDy = 0.f;
    mActivePointerId = motionEvent.getPointerId(0);
    select(vh, ACTION_STATE_SWIPE);
}

View* ItemTouchHelper::findChildView(MotionEvent& event) {
    // first check elevated views, if none, then call RV
    const float x = event.getX();
    const float y = event.getY();
    if (mSelected != nullptr) {
        View* selectedView = mSelected->itemView;
        if (hitTest(*selectedView, x, y, mSelectedStartX + mDx, mSelectedStartY + mDy)) {
            return selectedView;
        }
    }
    for (int i = int(mRecoverAnimations.size() - 1); i >= 0; i--) {
        RecoverAnimation* anim = mRecoverAnimations.at(i);
        View* view = anim->mViewHolder->itemView;
        if (hitTest(*view, x, y, anim->mX, anim->mY)) {
            return view;
        }
    }
    return mRecyclerView->findChildViewUnder(x, y);
}

void ItemTouchHelper::startDrag(RecyclerView::ViewHolder& viewHolder) {
    if (!mCallback->hasDragFlag(*mRecyclerView, viewHolder)) {
        LOGE("Start drag has been called but dragging is not enabled");
        return;
    }
    if (viewHolder.itemView->getParent() != mRecyclerView) {
        LOGE("Start drag has been called with a view holder which is not a child of "
               "the RecyclerView which is controlled by this ItemTouchHelper.");
        return;
    }
    obtainVelocityTracker();
    mDx = mDy = 0.f;
    select(&viewHolder, ACTION_STATE_DRAG);
}

void ItemTouchHelper::startSwipe(RecyclerView::ViewHolder& viewHolder) {
    if (!mCallback->hasSwipeFlag(*mRecyclerView, viewHolder)) {
        LOGE("Start swipe has been called but swiping is not enabled");
        return;
    }
    if (viewHolder.itemView->getParent() != mRecyclerView) {
        LOGE("Start swipe has been called with a view holder which is not a child of "
               "the RecyclerView controlled by this ItemTouchHelper.");
        return;
    }
    obtainVelocityTracker();
    mDx = mDy = 0.f;
    select(&viewHolder, ACTION_STATE_SWIPE);
}

ItemTouchHelper::RecoverAnimation* ItemTouchHelper::findAnimation(MotionEvent& event) {
    View* target = findChildView(event);
    for (int i = int(mRecoverAnimations.size() - 1); i >= 0; i--) {
        RecoverAnimation* anim = mRecoverAnimations.at(i);
        if (anim->mViewHolder->itemView == target) {
            return anim;
        }
    }
    return nullptr;
}

void ItemTouchHelper::updateDxDy(MotionEvent& ev, int directionFlags, int pointerIndex) {
    const float x = ev.getX(pointerIndex);
    const float y = ev.getY(pointerIndex);

    // Calculate the distance moved
    mDx = x - mInitialTouchX;
    mDy = y - mInitialTouchY;
    if ((directionFlags & LEFT) == 0) {
        mDx = std::max(0.f, mDx);
    }
    if ((directionFlags & RIGHT) == 0) {
        mDx = std::min(0.f, mDx);
    }
    if ((directionFlags & UP) == 0) {
        mDy = std::max(0.f, mDy);
    }
    if ((directionFlags & DOWN) == 0) {
        mDy = std::min(0.f, mDy);
    }
}

int ItemTouchHelper::swipeIfNecessary(RecyclerView::ViewHolder& viewHolder) {
    if (mActionState == ACTION_STATE_DRAG) {
        return 0;
    }
    const int originalMovementFlags = mCallback->getMovementFlags(*mRecyclerView, viewHolder);
    const int absoluteMovementFlags = mCallback->convertToAbsoluteDirection(
            originalMovementFlags,mRecyclerView->getLayoutDirection());
    const int flags = (absoluteMovementFlags & ACTION_MODE_SWIPE_MASK) >> (ACTION_STATE_SWIPE * DIRECTION_FLAG_COUNT);
    if (flags == 0) {
        return 0;
    }
    const int originalFlags = (originalMovementFlags & ACTION_MODE_SWIPE_MASK) >> (ACTION_STATE_SWIPE * DIRECTION_FLAG_COUNT);
    int swipeDir;
    if (std::abs(mDx) > std::abs(mDy)) {
        if ((swipeDir = checkHorizontalSwipe(viewHolder, flags)) > 0) {
            // if swipe dir is not in original flags, it should be the relative direction
            if ((originalFlags & swipeDir) == 0) {
                // convert to relative
                return Callback::convertToRelativeDirection(swipeDir,
                        mRecyclerView->getLayoutDirection());
            }
            return swipeDir;
        }
        if ((swipeDir = checkVerticalSwipe(viewHolder, flags)) > 0) {
            return swipeDir;
        }
    } else {
        if ((swipeDir = checkVerticalSwipe(viewHolder, flags)) > 0) {
            return swipeDir;
        }
        if ((swipeDir = checkHorizontalSwipe(viewHolder, flags)) > 0) {
            // if swipe dir is not in original flags, it should be the relative direction
            if ((originalFlags & swipeDir) == 0) {
                // convert to relative
                return Callback::convertToRelativeDirection(swipeDir,
                         mRecyclerView->getLayoutDirection());
            }
            return swipeDir;
        }
    }
    return 0;
}

int ItemTouchHelper::checkHorizontalSwipe(RecyclerView::ViewHolder& viewHolder, int flags) {
    if ((flags & (LEFT | RIGHT)) != 0) {
        const int dirFlag = mDx > 0 ? RIGHT : LEFT;
        if (mVelocityTracker && mActivePointerId > -1) {
            mVelocityTracker->computeCurrentVelocity(PIXELS_PER_SECOND,
                    mCallback->getSwipeVelocityThreshold(mMaxSwipeVelocity));
            const float xVelocity = mVelocityTracker->getXVelocity(mActivePointerId);
            const float yVelocity = mVelocityTracker->getYVelocity(mActivePointerId);
            const int velDirFlag = xVelocity > 0.f ? RIGHT : LEFT;
            const float absXVelocity = std::abs(xVelocity);
            if ((velDirFlag & flags) != 0 && dirFlag == velDirFlag
                    && absXVelocity >= mCallback->getSwipeEscapeVelocity(mSwipeEscapeVelocity)
                    && absXVelocity > std::abs(yVelocity)) {
                return velDirFlag;
            }
        }

        const float threshold = mRecyclerView->getWidth() * mCallback->getSwipeThreshold(viewHolder);

        if ((flags & dirFlag) != 0 && std::abs(mDx) > threshold) {
            return dirFlag;
        }
    }
    return 0;
}

int ItemTouchHelper::checkVerticalSwipe(RecyclerView::ViewHolder& viewHolder, int flags) {
    if ((flags & (UP | DOWN)) != 0) {
        const int dirFlag = mDy > 0 ? DOWN : UP;
        if (mVelocityTracker && mActivePointerId > -1) {
            mVelocityTracker->computeCurrentVelocity(PIXELS_PER_SECOND,
                    mCallback->getSwipeVelocityThreshold(mMaxSwipeVelocity));
            const float xVelocity = mVelocityTracker->getXVelocity(mActivePointerId);
            const float yVelocity = mVelocityTracker->getYVelocity(mActivePointerId);
            const int velDirFlag = yVelocity > 0.f ? DOWN : UP;
            const float absYVelocity = std::abs(yVelocity);
            if ((velDirFlag & flags) != 0 && velDirFlag == dirFlag
                    && absYVelocity >= mCallback->getSwipeEscapeVelocity(mSwipeEscapeVelocity)
                    && absYVelocity > std::abs(xVelocity)) {
                return velDirFlag;
            }
        }

        const float threshold = mRecyclerView->getHeight() * mCallback->getSwipeThreshold(viewHolder);
        if ((flags & dirFlag) != 0 && std::abs(mDy) > threshold) {
            return dirFlag;
        }
    }
    return 0;
}

void ItemTouchHelper::addChildDrawingOrderCallback() {
    if (Build::VERSION::SDK_INT >= 21) {
        return; // we use elevation on Lollipop
    }
    if (mChildDrawingOrderCallback == nullptr) {
        mChildDrawingOrderCallback = [this](int childCount, int i) {
            if (mOverdrawChild == nullptr) {
                return i;
            }
            int childPosition = mOverdrawChildPosition;
            if (childPosition == -1) {
                childPosition = mRecyclerView->indexOfChild(mOverdrawChild);
                mOverdrawChildPosition = childPosition;
            }
            if (i == childCount - 1) {
                return childPosition;
            }
            return i < childPosition ? i : i + 1;
        };
    }
    mRecyclerView->setChildDrawingOrderCallback(mChildDrawingOrderCallback);
}

void ItemTouchHelper::removeChildDrawingOrderCallbackIfNecessary(View* view) {
    if (view == mOverdrawChild) {
        mOverdrawChild = nullptr;
        // only remove if we've added
        if (mChildDrawingOrderCallback != nullptr) {
            mRecyclerView->setChildDrawingOrderCallback({});
        }
    }
}
/*************************************************************************************************/
//public abstract static class ItemTouchHelper::Callback {

static NeverDestroyed<ItemTouchUIUtilImpl>INSTANCE;
ItemTouchUIUtil& ItemTouchHelper::Callback::getDefaultUIUtil() {
    return *INSTANCE;
}

int ItemTouchHelper::Callback::convertToRelativeDirection(int flags, int layoutDirection) {
    const int masked = flags & ABS_HORIZONTAL_DIR_FLAGS;
    if (masked == 0) {
        return flags; // does not have any abs flags, good.
    }
    flags &= ~masked; //remove left / right.
    if (layoutDirection == View::LAYOUT_DIRECTION_LTR) {
        // no change. just OR with 2 bits shifted mask and return
        flags |= masked << 2; // START is 2 bits after LEFT, END is 2 bits after RIGHT.
        return flags;
    } else {
        // add RIGHT flag as START
        flags |= ((masked << 1) & ~ABS_HORIZONTAL_DIR_FLAGS);
        // first clean RIGHT bit then add LEFT flag as END
        flags |= ((masked << 1) & ABS_HORIZONTAL_DIR_FLAGS) << 2;
    }
    return flags;
}

int ItemTouchHelper::Callback::makeMovementFlags(int dragFlags, int swipeFlags) {
    return makeFlag(ACTION_STATE_IDLE, swipeFlags | dragFlags)
            | makeFlag(ACTION_STATE_SWIPE, swipeFlags)
            | makeFlag(ACTION_STATE_DRAG, dragFlags);
}

int ItemTouchHelper::Callback::makeFlag(int actionState, int directions) {
    return directions << (actionState * DIRECTION_FLAG_COUNT);
}

int ItemTouchHelper::Callback::convertToAbsoluteDirection(int flags, int layoutDirection) {
    int masked = flags & RELATIVE_DIR_FLAGS;
    if (masked == 0) {
        return flags; // does not have any relative flags, good.
    }
    flags &= ~masked; //remove start / end
    if (layoutDirection == View::LAYOUT_DIRECTION_LTR) {
        // no change. just OR with 2 bits shifted mask and return
        flags |= masked >> 2; // START is 2 bits after LEFT, END is 2 bits after RIGHT.
        return flags;
    } else {
        // add START flag as RIGHT
        flags |= ((masked >> 1) & ~RELATIVE_DIR_FLAGS);
        // first clean start bit then add END flag as LEFT
        flags |= ((masked >> 1) & RELATIVE_DIR_FLAGS) >> 2;
    }
    return flags;
}

int ItemTouchHelper::Callback::getAbsoluteMovementFlags(RecyclerView& recyclerView, RecyclerView::ViewHolder& viewHolder) {
    const int flags = getMovementFlags(recyclerView, viewHolder);
    return convertToAbsoluteDirection(flags, recyclerView.getLayoutDirection());
}

bool ItemTouchHelper::Callback::hasDragFlag(RecyclerView& recyclerView, RecyclerView::ViewHolder& viewHolder) {
    const int flags = getAbsoluteMovementFlags(recyclerView, viewHolder);
    return (flags & ACTION_MODE_DRAG_MASK) != 0;
}

bool ItemTouchHelper::Callback::hasSwipeFlag(RecyclerView& recyclerView, RecyclerView::ViewHolder& viewHolder) {
    const int flags = getAbsoluteMovementFlags(recyclerView, viewHolder);
    return (flags & ACTION_MODE_SWIPE_MASK) != 0;
}

bool ItemTouchHelper::Callback::canDropOver(RecyclerView& recyclerView,RecyclerView::ViewHolder& current,RecyclerView::ViewHolder& target) {
    return true;
}

bool ItemTouchHelper::Callback::isLongPressDragEnabled() {
    return true;
}

bool ItemTouchHelper::Callback::isItemViewSwipeEnabled() {
    return true;
}

int ItemTouchHelper::Callback::getBoundingBoxMargin() {
    return 0;
}

float ItemTouchHelper::Callback::getSwipeThreshold(RecyclerView::ViewHolder& viewHolder) {
    return 0.5f;
}

float ItemTouchHelper::Callback::getMoveThreshold(RecyclerView::ViewHolder& viewHolder) {
    return 0.5f;
}

float ItemTouchHelper::Callback::getSwipeEscapeVelocity(float defaultValue) {
    return defaultValue;
}

float ItemTouchHelper::Callback::getSwipeVelocityThreshold(float defaultValue) {
    return defaultValue;
}

RecyclerView::ViewHolder* ItemTouchHelper::Callback::chooseDropTarget(RecyclerView::ViewHolder& selected,
        std::vector<RecyclerView::ViewHolder*>& dropTargets, int curX, int curY) {
    int right = curX + selected.itemView->getWidth();
    int bottom = curY + selected.itemView->getHeight();
    RecyclerView::ViewHolder* winner = nullptr;
    int winnerScore = -1;
    const int dx = curX - selected.itemView->getLeft();
    const int dy = curY - selected.itemView->getTop();
    const int targetsSize = dropTargets.size();
    for (int i = 0; i < targetsSize; i++) {
        RecyclerView::ViewHolder* target = dropTargets.at(i);
        if (dx > 0) {
            int diff = target->itemView->getRight() - right;
            if (diff < 0 && target->itemView->getRight() > selected.itemView->getRight()) {
                const int score = std::abs(diff);
                if (score > winnerScore) {
                    winnerScore = score;
                    winner = target;
                }
            }
        }
        if (dx < 0) {
            int diff = target->itemView->getLeft() - curX;
            if (diff > 0 && target->itemView->getLeft() < selected.itemView->getLeft()) {
                const int score = std::abs(diff);
                if (score > winnerScore) {
                    winnerScore = score;
                    winner = target;
                }
            }
        }
        if (dy < 0) {
            int diff = target->itemView->getTop() - curY;
            if (diff > 0 && target->itemView->getTop() < selected.itemView->getTop()) {
                const int score = std::abs(diff);
                if (score > winnerScore) {
                    winnerScore = score;
                    winner = target;
                }
            }
        }

        if (dy > 0) {
            int diff = target->itemView->getBottom() - bottom;
            if (diff < 0 && target->itemView->getBottom() > selected.itemView->getBottom()) {
                const int score = std::abs(diff);
                if (score > winnerScore) {
                    winnerScore = score;
                    winner = target;
                }
            }
        }
    }
    return winner;
}

void ItemTouchHelper::Callback::onSelectedChanged(RecyclerView::ViewHolder* viewHolder, int actionState) {
    if (viewHolder != nullptr) {
        getDefaultUIUtil().onSelected(*viewHolder->itemView);
    }
}

int ItemTouchHelper::Callback::getMaxDragScroll(RecyclerView& recyclerView) {
    if (mCachedMaxScrollSpeed == -1) {
        mCachedMaxScrollSpeed = 10;//recyclerView.getResources().getDimensionPixelSize(R.dimen.item_touch_helper_max_drag_scroll_per_frame);
    }
    return mCachedMaxScrollSpeed;
}

void ItemTouchHelper::Callback::onMoved(RecyclerView& recyclerView,RecyclerView::ViewHolder& viewHolder, int fromPos,
	RecyclerView::ViewHolder& target, int toPos, int x, int y) {
    RecyclerView::LayoutManager* layoutManager = recyclerView.getLayoutManager();
    if(layoutManager->prepareForDrop(viewHolder.itemView, target.itemView, x, y)){
        return;
    }

    // if layout manager cannot handle it, do some guesswork
    if (layoutManager->canScrollHorizontally()) {
        const int minLeft = layoutManager->getDecoratedLeft(target.itemView);
        if (minLeft <= recyclerView.getPaddingLeft()) {
            recyclerView.scrollToPosition(toPos);
        }
        const int maxRight = layoutManager->getDecoratedRight(target.itemView);
        if (maxRight >= recyclerView.getWidth() - recyclerView.getPaddingRight()) {
            recyclerView.scrollToPosition(toPos);
        }
    }

    if (layoutManager->canScrollVertically()) {
        const int minTop = layoutManager->getDecoratedTop(target.itemView);
        if (minTop <= recyclerView.getPaddingTop()) {
            recyclerView.scrollToPosition(toPos);
        }
        const int maxBottom = layoutManager->getDecoratedBottom(target.itemView);
        if (maxBottom >= recyclerView.getHeight() - recyclerView.getPaddingBottom()) {
            recyclerView.scrollToPosition(toPos);
        }
    }
}

void ItemTouchHelper::Callback::onDraw(Canvas& c, RecyclerView& parent, RecyclerView::ViewHolder* selected,
        std::vector<ItemTouchHelper::RecoverAnimation*>& recoverAnimationList,
        int actionState, float dX, float dY) {
    const int recoverAnimSize = recoverAnimationList.size();
    for (int i = 0; i < recoverAnimSize; i++) {
        ItemTouchHelper::RecoverAnimation* anim = recoverAnimationList.at(i);
        anim->update();
        c.save();
        onChildDraw(c, parent,*anim->mViewHolder, anim->mX, anim->mY, anim->mActionState, false);
        c.restore();
    }
    if (selected) {
        c.save();
        onChildDraw(c, parent, *selected, dX, dY, actionState, true);
        c.restore();
    }
}

void ItemTouchHelper::Callback::onDrawOver(Canvas& c, RecyclerView& parent, RecyclerView::ViewHolder* selected,
        std::vector<ItemTouchHelper::RecoverAnimation*>& recoverAnimationList,
        int actionState, float dX, float dY) {
    const int recoverAnimSize = recoverAnimationList.size();
    for (int i = 0; i < recoverAnimSize; i++) {
        ItemTouchHelper::RecoverAnimation* anim = recoverAnimationList.at(i);
        c.save();
        onChildDrawOver(c, parent, *anim->mViewHolder, anim->mX, anim->mY, anim->mActionState, false);
        c.restore();
    }
    if (selected) {
        c.save();
        onChildDrawOver(c, parent, *selected, dX, dY, actionState, true);
        c.restore();
    }
    bool hasRunningAnimation = false;
    for (int i = recoverAnimSize - 1; i >= 0; i--) {
        RecoverAnimation* anim = recoverAnimationList.at(i);
        if (anim->mEnded && !anim->mIsPendingCleanup) {
            recoverAnimationList.erase(recoverAnimationList.begin()+i);//remove(i);
            delete anim;
        } else if (!anim->mEnded) {
            hasRunningAnimation = true;
        }
    }
    if (hasRunningAnimation) {
        parent.invalidate();
    }
}

void ItemTouchHelper::Callback::clearView(RecyclerView& recyclerView,RecyclerView::ViewHolder& viewHolder) {
    getDefaultUIUtil().clearView(*viewHolder.itemView);
}

void ItemTouchHelper::Callback::onChildDraw(Canvas& c,RecyclerView& recyclerView,RecyclerView::ViewHolder& viewHolder,
        float dX, float dY, int actionState, bool isCurrentlyActive) {
    getDefaultUIUtil().onDraw(c, recyclerView, *viewHolder.itemView, dX, dY, actionState, isCurrentlyActive);
}

void ItemTouchHelper::Callback::onChildDrawOver(Canvas& c,RecyclerView& recyclerView, RecyclerView::ViewHolder& viewHolder,
        float dX, float dY, int actionState, bool isCurrentlyActive) {
    getDefaultUIUtil().onDrawOver(c, recyclerView, *viewHolder.itemView, dX, dY, actionState, isCurrentlyActive);
}

long ItemTouchHelper::Callback::getAnimationDuration(RecyclerView& recyclerView, int animationType,
        float animateDx, float animateDy) {
    RecyclerView::ItemAnimator* itemAnimator = recyclerView.getItemAnimator();
    if (itemAnimator == nullptr) {
        return animationType == ANIMATION_TYPE_DRAG ? DEFAULT_DRAG_ANIMATION_DURATION
                : DEFAULT_SWIPE_ANIMATION_DURATION;
    } else {
        return animationType == ANIMATION_TYPE_DRAG ? itemAnimator->getMoveDuration()
                : itemAnimator->getRemoveDuration();
    }
}

int ItemTouchHelper::Callback::interpolateOutOfBoundsScroll(RecyclerView& recyclerView,
        int viewSize, int viewSizeOutOfBounds, int totalSize, int64_t msSinceStartScroll) {
    const int maxScroll = getMaxDragScroll(recyclerView);
    const int absOutOfBounds = std::abs(viewSizeOutOfBounds);
    const int direction = (int) MathUtils::signum(viewSizeOutOfBounds);
    // might be negative if other direction
    float outOfBoundsRatio = std::min(1.f, 1.f * absOutOfBounds / viewSize);
    outOfBoundsRatio -=1.f;
    const int cappedScroll = (int) (direction * maxScroll* sDragViewScrollCapInterpolator.getInterpolation(outOfBoundsRatio));
    float timeRatio;
    if (msSinceStartScroll > DRAG_SCROLL_ACCELERATION_LIMIT_TIME_MS) {
        timeRatio = 1.f;
    } else {
        timeRatio = (float) msSinceStartScroll / DRAG_SCROLL_ACCELERATION_LIMIT_TIME_MS;
    }
    const int value = (int) (cappedScroll * timeRatio*sDragScrollInterpolator.getInterpolation(timeRatio));
    if (value == 0) {
        return viewSizeOutOfBounds > 0 ? 1 : -1;
    }
    return value;
}
/************************************************************************************************/
//public abstract static class ItemTouchHelper::SimpleCallback:public ItemTouchHelper::Callback

ItemTouchHelper::SimpleCallback::SimpleCallback(int dragDirs, int swipeDirs) {
    mDefaultSwipeDirs = swipeDirs;
    mDefaultDragDirs = dragDirs;
}

void ItemTouchHelper::SimpleCallback::setDefaultSwipeDirs(int defaultSwipeDirs) {
    mDefaultSwipeDirs = defaultSwipeDirs;
}

void ItemTouchHelper::SimpleCallback::setDefaultDragDirs(int defaultDragDirs) {
    mDefaultDragDirs = defaultDragDirs;
}

int ItemTouchHelper::SimpleCallback::getSwipeDirs(RecyclerView& recyclerView,RecyclerView::ViewHolder& viewHolder) {
    return mDefaultSwipeDirs;
}

int ItemTouchHelper::SimpleCallback::getDragDirs(RecyclerView& recyclerView,RecyclerView::ViewHolder& viewHolder) {
    return mDefaultDragDirs;
}

int ItemTouchHelper::SimpleCallback::getMovementFlags(RecyclerView& recyclerView,RecyclerView::ViewHolder& viewHolder) {
    return makeMovementFlags(getDragDirs(recyclerView, viewHolder),
            getSwipeDirs(recyclerView, viewHolder));
}

void ItemTouchHelper::doNotReactToLongPress() {
    mShouldReactToLongPress = false;
}

bool ItemTouchHelper::onGestureDown(MotionEvent& e) {
    return true;
}

void ItemTouchHelper::onGestureLongPress(MotionEvent& e) {
    if (!mShouldReactToLongPress) {
        return;
    }
    View* child = findChildView(e);
    if (child != nullptr) {
        RecyclerView::ViewHolder* vh = mRecyclerView->getChildViewHolder(child);
        if (vh != nullptr) {
            if (!mCallback->hasDragFlag(*mRecyclerView,*vh)) {
                return;
            }
            int pointerId = e.getPointerId(0);
            // Long press is deferred.
            // Check w/ active pointer id to avoid selecting after motion
            // event is canceled.
            if (pointerId == mActivePointerId) {
                const int index = e.findPointerIndex(mActivePointerId);
                const float x = e.getX(index);
                const float y = e.getY(index);
                mInitialTouchX = x;
                mInitialTouchY = y;
                mDx = mDy = 0.f;
                LOGD_IF(_Debug,"onlong press: x:%.f ,y:%.f",x,y);
                if (mCallback->isLongPressDragEnabled()) {
                    select(vh, ACTION_STATE_DRAG);
                }
            }
        }
    }
}

ItemTouchHelper::RecoverAnimation::RecoverAnimation(RecyclerView::ViewHolder* viewHolder, int animationType,
        int actionState, float startDx, float startDy, float targetX, float targetY) {
    mActionState = actionState;
    mAnimationType = animationType;
    mViewHolder = viewHolder;
    mStartDx = startDx;
    mStartDy = startDy;
    mTargetX = targetX;
    mTargetY = targetY;
    mOverridden = false;
    mEnded = false;
    mIsPendingCleanup = false;
    mValueAnimator = ValueAnimator::ofFloat({0.f, 1.f});
    ValueAnimator::AnimatorUpdateListener ls;
    ls/*onAnimationUpdate*/ = [this](ValueAnimator& animation) {
       setFraction(animation.getAnimatedFraction());
    };
    mValueAnimator->addUpdateListener(ls);
    mValueAnimator->setTarget(viewHolder->itemView);
    mAnimatorListener.onAnimationStart = std::bind(&RecoverAnimation::onAnimationStart,this,std::placeholders::_1,std::placeholders::_2);
    mAnimatorListener.onAnimationEnd   = std::bind(&RecoverAnimation::onAnimationEnd,this,std::placeholders::_1,std::placeholders::_2);
    mAnimatorListener.onAnimationCancel= std::bind(&RecoverAnimation::onAnimationCancel,this,std::placeholders::_1);
    mAnimatorListener.onAnimationRepeat= std::bind(&RecoverAnimation::onAnimationRepeat,this,std::placeholders::_1);
    mValueAnimator->addListener(mAnimatorListener);
    setFraction(0.f);
}

ItemTouchHelper::RecoverAnimation::~RecoverAnimation(){
    delete mValueAnimator;
}

void ItemTouchHelper::RecoverAnimation::setDuration(long duration) {
    mValueAnimator->setDuration(duration);
}

void ItemTouchHelper::RecoverAnimation::start() {
    mViewHolder->setIsRecyclable(false);
    mValueAnimator->start();
}

void ItemTouchHelper::RecoverAnimation::cancel() {
    mValueAnimator->cancel();
}

void ItemTouchHelper::RecoverAnimation::setFraction(float fraction) {
    mFraction = fraction;
}

void ItemTouchHelper::RecoverAnimation::update() {
    if (mStartDx == mTargetX) {
        mX = mViewHolder->itemView->getTranslationX();
    } else {
        mX = mStartDx + mFraction * (mTargetX - mStartDx);
    }
    if (mStartDy == mTargetY) {
        mY = mViewHolder->itemView->getTranslationY();
    } else {
        mY = mStartDy + mFraction * (mTargetY - mStartDy);
    }
}

void ItemTouchHelper::RecoverAnimation::onAnimationStart(Animator& animation,bool/*isReverse*/) {

}

void ItemTouchHelper::RecoverAnimation::onAnimationEnd(Animator& animation,bool isReverse) {
    if (!mEnded) {
        mViewHolder->setIsRecyclable(true);
    }
    mEnded = true;
}

void ItemTouchHelper::RecoverAnimation::onAnimationCancel(Animator& animation) {
    setFraction(1.f); //make sure we recover the view's state.
}

void ItemTouchHelper::RecoverAnimation::onAnimationRepeat(Animator& animation) {

}

/////////////////////////////////////////////////////////////////////////////////////////////////

}/*endof namespace*/

