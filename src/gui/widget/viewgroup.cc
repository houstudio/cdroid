/*
 * Copyright (C) 2015 UI project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <widget/viewgroup.h>
#include <widget/measurespec.h>
#include <animation/layouttransition.h>
#include <animation/layoutanimationcontroller.h>
#include <cdlog.h>
#include <uievents.h>
#include <focusfinder.h>
#include <systemclock.h>

#define CHILD_LEFT_INDEX 0
#define CHILD_TOP_INDEX  1

namespace cdroid {
class TouchTarget{
private:
    static TouchTarget* sRecycleBin;
    static int sRecycledCount;
public:
    enum{
        ALL_POINTER_IDS= -1,
        MAX_RECYCLED   = 32
    };
    View*child;
    int pointerIdBits;
    TouchTarget*next;
public:
    TouchTarget(){
        child=nullptr;
        next=nullptr;
    }
    static TouchTarget*obtain(View*child,int pointerIdBits){
        TouchTarget*target;
        if(sRecycleBin==nullptr)
             target=new TouchTarget();
        else{
            target=sRecycleBin;
            sRecycleBin=target->next;
            sRecycledCount--;
            target->next=nullptr;
        }
        target->child=child;
        target->pointerIdBits=pointerIdBits;
        return target;
    }
    void recycle(){
        if (sRecycledCount < MAX_RECYCLED) {
            next = sRecycleBin;
            sRecycleBin = this;
            sRecycledCount += 1;
        } else {
            next = nullptr;
        }
        child = nullptr;
    }
};

TouchTarget*TouchTarget::sRecycleBin=nullptr;
int TouchTarget::sRecycledCount=0;
bool ViewGroup::DEBUG_DRAW = false;

ViewGroup::ViewGroup(Context*ctx,const AttributeSet& attrs):View(ctx,attrs){
    initGroup();
}

ViewGroup::ViewGroup(int w,int h)
  : ViewGroup(0,0,w,h) {
}

ViewGroup::ViewGroup(int x,int y,int w,int h)
:View(w,h){
    mLeft=x;
    mTop=y;
    initGroup();
}

void ViewGroup::initGroup(){
    mGroupFlags = FLAG_CLIP_CHILDREN; 
    mGroupFlags|= FLAG_CLIP_TO_PADDING;
    mGroupFlags|= FLAG_ANIMATION_DONE;
    mGroupFlags|= FLAG_ANIMATION_CACHE;
    mGroupFlags!= FLAG_ALWAYS_DRAWN_WITH_CACHE;
    mFocused = nullptr;
    mDefaultFocus = nullptr;
    mFocusedInCluster = nullptr;
    mFirstTouchTarget = nullptr;
    mOnHierarchyChangeListener = nullptr;
    mLayoutAnimationController = nullptr;
    mChildCountWithTransientState=0;
    mInvalidRgn=Region::create();
    mChildTransformation =nullptr;
    mInvalidationTransformation =nullptr;
    mTransition = nullptr;
    setDescendantFocusability(FOCUS_BEFORE_DESCENDANTS);
}

ViewGroup::~ViewGroup() {
    for_each(mChildren.begin(),mChildren.end(),[](View*v){
        delete v;
    });
    mChildren.clear();
    delete mChildTransformation;
    delete mInvalidationTransformation;
    delete mLayoutAnimationController;
}

void ViewGroup::cancelAndClearTouchTargets(MotionEvent* event){
    if (mFirstTouchTarget==nullptr)return;
 
    long now = SystemClock::uptimeMillis();
    bool syntheticEvent=(event==nullptr);
    if(event==nullptr){
	    event=MotionEvent::obtain(now, now,MotionEvent::ACTION_CANCEL, 0.0f, 0.0f, 0);
        event->setAction(MotionEvent::ACTION_CANCEL);
        //event.setSource(InputDevice.SOURCE_TOUCHSCREEN);
    }
    for (TouchTarget* target = mFirstTouchTarget; target ; target = target->next) {
        resetCancelNextUpFlag(target->child);
        dispatchTransformedTouchEvent(*event, true, target->child, target->pointerIdBits);
	    LOGD("send CANCEL to %p:%d",target->child,target->child->getId());
    }
    if(syntheticEvent)
        event->recycle();
    clearTouchTargets();
}

bool ViewGroup::dispatchTransformedTouchEvent(MotionEvent& event, bool cancel,
       View* child, int desiredPointerIdBits){
    bool handled;

    // Canceling motions is a special case.  We don't need to perform any transformations
    // or filtering.  The important part is the action, not the contents.
    int oldAction = event.getAction();
    if (cancel || oldAction == MotionEvent::ACTION_CANCEL) {
        event.setAction(MotionEvent::ACTION_CANCEL);
        if (child == nullptr) {
            handled = View::dispatchTouchEvent(event);
        } else {
            handled = child->dispatchTouchEvent(event);
        }
        event.setAction(oldAction);
        return handled;
    }

    // Calculate the number of pointers to deliver.
    int oldPointerIdBits = event.getPointerIdBits();
    int newPointerIdBits = oldPointerIdBits & desiredPointerIdBits;

    // If for some reason we ended up in an inconsistent state where it looks like we
    // might produce a motion event with no pointers in it, then drop the event.
    if (newPointerIdBits == 0) {
        return false;
    }

    // If the number of pointers is the same and we don't need to perform any fancy
    // irreversible transformations, then we can reuse the motion event for this
    // dispatch as long as we are careful to revert any changes we make.
    // Otherwise we need to make a copy.
    MotionEvent* transformedEvent;
    if (newPointerIdBits == oldPointerIdBits) {
        if (child == nullptr || child->hasIdentityMatrix()) {
            if (child == nullptr) {
                handled = View::dispatchTouchEvent(event);
            } else {
                float offsetX = mScrollX - child->mLeft;
                float offsetY = mScrollY - child->mTop;
                event.offsetLocation(offsetX, offsetY);

                handled = child->dispatchTouchEvent(event);

                event.offsetLocation(-offsetX, -offsetY);
            }
            return handled;
        }
        transformedEvent = MotionEvent::obtain(event);
    } else {
        transformedEvent = event.split(newPointerIdBits);
    }

    // Perform any necessary transformations and dispatch.
    if (child == nullptr) {
        handled = View::dispatchTouchEvent(*transformedEvent);
    } else {
        float offsetX = mScrollX - child->mLeft;
        float offsetY = mScrollY - child->mTop;
        transformedEvent->offsetLocation(offsetX, offsetY);
        /*if (! child->hasIdentityMatrix()) {
            transformedEvent.transform(child.getInverseMatrix());
        }*/

        handled = child->dispatchTouchEvent(*transformedEvent);
    }

    // Done. 
    transformedEvent->recycle();
    return handled;
}

TouchTarget* ViewGroup::getTouchTarget(View* child) {
    for (TouchTarget* target = mFirstTouchTarget; target ; target = target->next) {
        if (target->child == child) {
            return target;
        }
    }
    return nullptr;
}

TouchTarget* ViewGroup::addTouchTarget(View* child, int pointerIdBits) {
    TouchTarget* target = TouchTarget::obtain(child, pointerIdBits);
    target->next = mFirstTouchTarget;
    mFirstTouchTarget = target;
    return target;
}

void ViewGroup::resetTouchState() {
    clearTouchTargets();
    resetCancelNextUpFlag(this);
    mGroupFlags &= ~FLAG_DISALLOW_INTERCEPT;
    mNestedScrollAxes = SCROLL_AXIS_NONE;
}

bool ViewGroup::resetCancelNextUpFlag(View* view){
    if ((view->mPrivateFlags & PFLAG_CANCEL_NEXT_UP_EVENT) != 0) {
        view->mPrivateFlags &= ~PFLAG_CANCEL_NEXT_UP_EVENT;
        return true;
    }
    return false;
}

void ViewGroup::clearTouchTargets(){
    TouchTarget* target = mFirstTouchTarget;
    if (target != nullptr) {
        do {
            TouchTarget* next = target->next;
            target->recycle();
            target = next;
        } while (target != nullptr);
        mFirstTouchTarget = nullptr;
    }
}

void ViewGroup::removePointersFromTouchTargets(int pointerIdBits) {
    TouchTarget* predecessor = nullptr;
    TouchTarget* target = mFirstTouchTarget;
    while (target != nullptr) {
        TouchTarget* next = target->next;
        if ((target->pointerIdBits & pointerIdBits) != 0) {
            target->pointerIdBits &= ~pointerIdBits;
            if (target->pointerIdBits == 0) {
                if (predecessor == nullptr) mFirstTouchTarget = next;
                else    predecessor->next = next;
                target->recycle();
                target = next;
                continue;
            }
        }
        predecessor = target;
        target = next;
    }
}

void ViewGroup::cancelTouchTarget(View* view){
    TouchTarget* predecessor = nullptr;
    TouchTarget* target = mFirstTouchTarget;
    while (target != nullptr) {
        TouchTarget* next = target->next;
        if (target->child == view) {
            if (predecessor == nullptr) {
                mFirstTouchTarget = next;
            } else {
                predecessor->next = next;
            }
            target->recycle();
            long now = SystemClock::uptimeMillis();
            MotionEvent* event=MotionEvent::obtain(now, now, MotionEvent::ACTION_CANCEL, 0.0f, 0.0f, 0);
            event->setSource(InputEvent::SOURCE_TOUCHSCREEN);
            view->dispatchTouchEvent(*event);
            event->recycle();
            return;
        }
        predecessor = target;
        target = next;
    }
}
void ViewGroup::cancelHoverTarget(View*view){
}

bool ViewGroup::canViewReceivePointerEvents(View& child) {
    return (child.mViewFlags & VISIBILITY_MASK) == VISIBLE || child.getAnimation();
}
void ViewGroup::setOnHierarchyChangeListener(OnHierarchyChangeListener listener){
    mOnHierarchyChangeListener =listener;
}

void ViewGroup::onViewAdded(View*v){
}

void ViewGroup::onViewRemoved(View*v){
}

bool ViewGroup::hasTransientState(){
    return mChildCountWithTransientState > 0 || View::hasTransientState();
}

bool ViewGroup::dispatchGenericFocusedEvent(MotionEvent&event){
    if ((mPrivateFlags & (PFLAG_FOCUSED | PFLAG_HAS_BOUNDS))
            == (PFLAG_FOCUSED | PFLAG_HAS_BOUNDS)) {
        return View::dispatchGenericFocusedEvent(event);
    } else if (mFocused && (mFocused->mPrivateFlags & PFLAG_HAS_BOUNDS)
            == PFLAG_HAS_BOUNDS) {
        return mFocused->dispatchGenericMotionEvent(event);
    }
    return false;
}

bool ViewGroup::dispatchTransformedGenericPointerEvent(MotionEvent& event, View* child) {
    bool handled;
    /*if (!child->hasIdentityMatrix()) {
        MotionEvent transformedEvent = getTransformedMotionEvent(event, child);
        handled = child.dispatchGenericMotionEvent(transformedEvent);
        transformedEvent.recycle();
    } else */{
        float offsetX = mScrollX - child->mLeft;
        float offsetY = mScrollY - child->mTop;
        event.offsetLocation(offsetX, offsetY);
        handled = child->dispatchGenericMotionEvent(event);
        event.offsetLocation(-offsetX, -offsetY);
    }
    return handled;
}

bool ViewGroup::dispatchUnhandledMove(View* focused, int direction){
    return mFocused &&  mFocused->dispatchUnhandledMove(focused, direction);
}

void ViewGroup::childHasTransientStateChanged(View* child, bool childHasTransientState){
    bool oldHasTransientState = hasTransientState();
    if (childHasTransientState) {
        mChildCountWithTransientState++;
    } else {
        mChildCountWithTransientState--;
    }

    bool newHasTransientState = hasTransientState();
    if (mParent  && oldHasTransientState != newHasTransientState)
        mParent->childHasTransientStateChanged(this, newHasTransientState);
}
void ViewGroup::dispatchViewAdded(View*v){
    onViewAdded(v);
    if(mOnHierarchyChangeListener)
        mOnHierarchyChangeListener(*this,v,true);
}

void ViewGroup::dispatchViewRemoved(View*v){
    onViewRemoved(v);
    if(mOnHierarchyChangeListener)
        mOnHierarchyChangeListener(*this,v,false);
}

void ViewGroup::removeDetachedView(View* child, bool animate){
    //if (mTransition)mTransition.removeChild(this, child);

    if (child == mFocused) child->clearFocus();
    if (child == mDefaultFocus) clearDefaultFocus(child);
    if (child == mFocusedInCluster) clearFocusedInCluster(child);

    //child->clearAccessibilityFocus();

    cancelTouchTarget(child);
    cancelHoverTarget(child);

    if ((animate && child->getAnimation()) ||isViewTransitioning(child)){
        addDisappearingView(child);
    } else{// if (child->mAttachInfo != nullptr) {
        //child->dispatchDetachedFromWindow();
    }

    if (child->hasTransientState()) childHasTransientStateChanged(child, false);

    dispatchViewRemoved(child);
}
void ViewGroup::detachViewFromParent(View* child){
    removeFromArray(indexOfChild(child));
}
void ViewGroup::detachViewsFromParent(int start, int count){
    removeFromArray(start, count);
}

void ViewGroup::removeFromArray(int index){
    if (isViewTransitioning(mChildren[index])){
        mChildren[index]->mParent = nullptr;
    }
    if (index>=0&&index<mChildren.size()) {
        mChildren.erase(mChildren.begin()+index);
    } else {
        LOGE("IndexOutOfBounds %d",index);
    }
    if (mLastTouchDownIndex == index) {
        mLastTouchDownTime = 0;
        mLastTouchDownIndex = -1;
    } else if (mLastTouchDownIndex > index) {
        mLastTouchDownIndex--;
    }
}

void ViewGroup::removeFromArray(int start, int count){
    int childrenCount = mChildren.size();

    start = std::max(0, start);
    int end = std::min(childrenCount, start + count);

    if (start == end)  return;
    for (int i = start; i < end; i++) {
        mChildren[i]->mParent = nullptr;
        mChildren[i] = nullptr;
    }
    mChildren.erase(mChildren.begin()+start,mChildren.begin()+start+count);
}

void ViewGroup::detachAllViewsFromParent(){
    int count = mChildren.size();
    if (count <= 0) {
        return;
    }
    for (int i = count - 1; i >= 0; i--) {
        mChildren[i]->mParent = nullptr;
        mChildren[i] = nullptr;
    }
    mChildren.clear();
}

bool ViewGroup::addViewInLayout(View* child, int index,LayoutParams* params){
    return addViewInLayout(child, index, params, false);
}

bool ViewGroup::addViewInLayout(View* child, int index,LayoutParams* params,bool preventRequestLayout){
    if (child == nullptr) {
        LOGE("Cannot add a null child view to a ViewGroup");
	return false;
    }
    child->mParent = nullptr;
    addViewInner(child, index, params, preventRequestLayout);
    child->mPrivateFlags = (child->mPrivateFlags & ~PFLAG_DIRTY_MASK) | PFLAG_DRAWN;
    return true;
}

void ViewGroup::addDisappearingView(View* v) {
    mDisappearingChildren.push_back(v);
}

void ViewGroup::clearDisappearingChildren() {
    std::vector<View*> disappearingChildren = mDisappearingChildren;
    int count = disappearingChildren.size();
    for (int i = 0; i < count; i++) {
        View* view = disappearingChildren.at(i);
        //if (view.mAttachInfo != null)view.dispatchDetachedFromWindow();
        view->clearAnimation();
    }
    disappearingChildren.clear();
    invalidate();
}

void ViewGroup::startViewTransition(View* view){
    if (view->mParent == this)
        mTransitioningViews.push_back(view);
}

void ViewGroup::endViewTransition(View* view){
    auto it= std::find(mTransitioningViews.begin(),mTransitioningViews.end(),view);
    if(it!=mTransitioningViews.end())mTransitioningViews.erase(it);

    it= std::find(mDisappearingChildren.begin(),mDisappearingChildren.end(),view);
    if (it!=mDisappearingChildren.end()) {
        mDisappearingChildren.erase(it);

        it=std::find(mVisibilityChangingChildren.begin(),mVisibilityChangingChildren.end(),view);
        if (it!=mVisibilityChangingChildren.end()) {
            mVisibilityChangingChildren.erase(it);
        } else {
            //if (view.mAttachInfo != null) view.dispatchDetachedFromWindow();
            if (view->mParent) view->mParent = nullptr;
        }
       invalidate();
   }
}

void ViewGroup::finishAnimatingView(View* view, Animation* animation) {
    auto it=std::find(mDisappearingChildren.begin(),mDisappearingChildren.end(),view);
    if (it!=mDisappearingChildren.end()) {
        mDisappearingChildren.erase(it);
        //if (view.mAttachInfo != null) view->dispatchDetachedFromWindow();
        view->clearAnimation();
        mGroupFlags |= FLAG_INVALIDATE_REQUIRED;
    }

    if (animation && !animation->getFillAfter()) {
        view->clearAnimation();
    }

    if ((view->mPrivateFlags & PFLAG_ANIMATION_STARTED) == PFLAG_ANIMATION_STARTED) {
        view->onAnimationEnd();
        // Should be performed by onAnimationEnd() but this avoid an infinite loop,
        // so we'd rather be safe than sorry
        view->mPrivateFlags &= ~PFLAG_ANIMATION_STARTED;
        // Draw one more frame after the animation is done
        mGroupFlags |= FLAG_INVALIDATE_REQUIRED;
    }
}

void ViewGroup::dispatchInvalidateOnAnimation(View* view){
}

void ViewGroup::cancelInvalidate(View* view){
}

bool ViewGroup::isViewTransitioning(View* view){
    return  std::find(mTransitioningViews.begin(),mTransitioningViews.end(),view)!= mTransitioningViews.end();
}

void ViewGroup::onChildVisibilityChanged(View* child, int oldVisibility, int newVisibility){
    LOGV("view %p visibility %d->%d",child,oldVisibility, newVisibility);
    if (mTransition != nullptr) {
        if (newVisibility == VISIBLE) {
            mTransition->showChild(this, child, oldVisibility);
        } else {
            mTransition->hideChild(this, child, newVisibility);
            if ( isViewTransitioning(child) ){
                // Only track this on disappearing views - appearing views are already visible
                // and don't need special handling during drawChild()
                mVisibilityChangingChildren.push_back(child);
                addDisappearingView(child);
            }
        }
    }
    // in all cases, for drags
    /*if (newVisibility == VISIBLE && mCurrentDragStartEvent != null) {
        if (!mChildrenInterestedInDrag.contains(child)) {
            notifyChildOfDragStart(child);
        }
    }*/
}

void ViewGroup::attachViewToParent(View* child, int index, LayoutParams* params){
    child->mLayoutParams = params;

    if (index < 0) {
        index = getChildCount();//mChildrenCount;
    }

    addInArray(child, index);

    child->mParent = this;
    child->mPrivateFlags = (child->mPrivateFlags & ~PFLAG_DIRTY_MASK /*& ~PFLAG_DRAWING_CACHE_VALID*/)
         | PFLAG_DRAWN|PFLAG_INVALIDATED;
    this->mPrivateFlags |= PFLAG_INVALIDATED;
    if (child->hasFocus()) {
        requestChildFocus(child, child->findFocus());
    }
    //dispatchVisibilityAggregated(isAttachedToWindow() && getWindowVisibility() == VISIBLE&& isShown());
    //notifySubtreeAccessibilityStateChangedIfNeeded();
}

bool ViewGroup::isLayoutModeOptical()const{
    return mLayoutMode == LAYOUT_MODE_OPTICAL_BOUNDS;
}
int ViewGroup::getDescendantFocusability()const{
    return mGroupFlags&FLAG_MASK_FOCUSABILITY;
}

void ViewGroup::setDescendantFocusability(int focusability){
    mGroupFlags &= ~FLAG_MASK_FOCUSABILITY;
    mGroupFlags |= (focusability & FLAG_MASK_FOCUSABILITY);
}

bool ViewGroup::isChildrenDrawingOrderEnabled()const{
    return (mGroupFlags & FLAG_USE_CHILD_DRAWING_ORDER) == FLAG_USE_CHILD_DRAWING_ORDER;
}

void ViewGroup::setChildrenDrawingOrderEnabled(bool enabled) {
    setBooleanFlag(FLAG_USE_CHILD_DRAWING_ORDER, enabled);
}

void ViewGroup::setBooleanFlag(int flag, bool value) {
    if (value) {
        mGroupFlags |= flag;
    } else {
        mGroupFlags &= ~flag;
    }
}

bool ViewGroup::hasBooleanFlag(int flag)const{
    return (mGroupFlags & flag) == flag;
}

bool ViewGroup::getClipChildren() const{
    return ((mGroupFlags & FLAG_CLIP_CHILDREN) != 0);
}

void ViewGroup::setClipChildren(bool clipChildren){
    bool previousValue = (mGroupFlags & FLAG_CLIP_CHILDREN) == FLAG_CLIP_CHILDREN;
    if (clipChildren != previousValue){ 
        setBooleanFlag(FLAG_CLIP_CHILDREN, clipChildren);
	invalidate();
    }
}

void ViewGroup::setClipToPadding(bool clipToPadding) {
    if (hasBooleanFlag(FLAG_CLIP_TO_PADDING) != clipToPadding) {
        setBooleanFlag(FLAG_CLIP_TO_PADDING, clipToPadding);
        invalidate();
    }
}

bool ViewGroup::getClipToPadding()const{
   return hasBooleanFlag(FLAG_CLIP_TO_PADDING);
}

void ViewGroup::dispatchSetSelected(bool selected) {
    for (auto child:mChildren){
        child->setSelected(selected);
    }
}

void ViewGroup::dispatchSetActivated(bool activated) {
    for (auto child:mChildren){
        child->setActivated(activated);
    }
}

void ViewGroup::dispatchSetPressed(bool pressed) {
    for (auto child:mChildren){
        // Children that are clickable on their own should not
        // show a pressed state when their parent view does.
        // Clearing a pressed state always propagates.
        if (!pressed || (!child->isClickable() && !child->isLongClickable())) {
            child->setPressed(pressed);
        }
    }
}

int ViewGroup::getChildCount()const{
    return mChildren.size();
}

View*ViewGroup::getChildAt(int idx)const{
    if(idx<0||idx>=mChildren.size())return nullptr;
    return mChildren.at(idx);
}

int ViewGroup::indexOfChild(View* child)const{
    int count=mChildren.size();
    for(int i=0;i<count;i++)
        if(mChildren[i]==child)return i;
    return -1;
}

int ViewGroup::getChildDrawingOrder(int count,int i){
    return i;
}

bool ViewGroup::hasChildWithZ()const{
    for (auto child:mChildren) {
        if (child->getZ() != 0) return true;
    }
    return false;
}

int ViewGroup::buildOrderedChildList(std::vector<View*>&preSortedChildren) {
    const int childrenCount =  mChildren.size();
    if (childrenCount <= 1 || !hasChildWithZ()) return 0;

    preSortedChildren.clear();

    const bool customOrder = isChildrenDrawingOrderEnabled();
    for (int i = 0; i < childrenCount; i++) {
        // add next child (in child order) to end of list
        const int childIndex = getAndVerifyPreorderedIndex(childrenCount, i, customOrder);
        View* nextChild = mChildren[childIndex];
        const float currentZ = nextChild->getZ();

        // insert ahead of any Views with greater Z
        int insertIndex = i;
        while (insertIndex > 0 && preSortedChildren.at(insertIndex - 1)->getZ() > currentZ) {
            insertIndex--;
        }
        preSortedChildren.insert(preSortedChildren.begin()+insertIndex, nextChild);
    }
    return preSortedChildren.size();
}

int ViewGroup::getAndVerifyPreorderedIndex(int childrenCount, int i, bool customOrder){
    int childIndex;
    if (customOrder) {
        int childIndex1 = getChildDrawingOrder(childrenCount, i);
        LOGE_IF(childIndex1 >= childrenCount,"getChildDrawingOrder() returned invalid index %d (child count is %d)",childIndex1,childrenCount);
        childIndex = childIndex1;
    } else {
        childIndex = i;
    }
    return childIndex;
}

View* ViewGroup::getAndVerifyPreorderedView(const std::vector<View*>& preorderedList,const std::vector<View*>& children,int childIndex) {
    View* child;
    if (preorderedList.size()) {
        child = preorderedList.at(childIndex);
        LOGE_IF(child == nullptr,"Invalid preorderedList contained null child at index %d",childIndex);
    } else {
        child = children[childIndex];
    }
    return child;
}

bool ViewGroup::getChildStaticTransformation(View* child, Transformation* t){
    return false;
}

Transformation* ViewGroup::getChildTransformation(){
    if(mChildTransformation==nullptr)
        mChildTransformation=new Transformation();
    return mChildTransformation;
}

int ViewGroup::getChildMeasureSpec(int spec, int padding, int childDimension){
	int specMode = MeasureSpec::getMode(spec);
    int specSize = MeasureSpec::getSize(spec);

    int size = std::max(0, specSize - padding);

    int resultSize = 0;
    int resultMode = 0;

    switch (specMode) {
    // Parent has imposed an exact size on us
	case MeasureSpec::EXACTLY:
        if (childDimension >= 0) {
            resultSize = childDimension;
            resultMode = MeasureSpec::EXACTLY;
        } else if (childDimension == LayoutParams::MATCH_PARENT) {
            // Child wants to be our size. So be it.
            resultSize = size;
            resultMode = MeasureSpec::EXACTLY;
        } else if (childDimension == LayoutParams::WRAP_CONTENT) {
            // Child wants to determine its own size. It can't be
            // bigger than us.
            resultSize = size;
            resultMode = MeasureSpec::AT_MOST;
        }
        break;

    // Parent has imposed a maximum size on us
	case MeasureSpec::AT_MOST:
        if (childDimension >= 0) {
            // Child wants a specific size... so be it
            resultSize = childDimension;
            resultMode = MeasureSpec::EXACTLY;
        } else if (childDimension == LayoutParams::MATCH_PARENT) {
            // Child wants to be our size, but our size is not fixed.
            // Constrain child to not be bigger than us.
            resultSize = size;
            resultMode = MeasureSpec::AT_MOST;
        } else if (childDimension == LayoutParams::WRAP_CONTENT) {
            // Child wants to determine its own size. It can't be
            // bigger than us.
            resultSize = size;
            resultMode = MeasureSpec::AT_MOST;
        }
        break;

    // Parent asked to see how big we want to be
	case MeasureSpec::UNSPECIFIED:
        if (childDimension >= 0) {
            // Child wants a specific size... let him have it
            resultSize = childDimension;
            resultMode = MeasureSpec::EXACTLY;
        } else if (childDimension == LayoutParams::MATCH_PARENT) {
            // Child wants to be our size... find out how big it should
            // be
            resultSize =size;// View.sUseZeroUnspecifiedMeasureSpec ? 0 : size;
            resultMode = MeasureSpec::UNSPECIFIED;
        } else if (childDimension == LayoutParams::WRAP_CONTENT) {
            // Child wants to determine its own size.... find out how
            // big it should be
            resultSize = size;//View.sUseZeroUnspecifiedMeasureSpec ? 0 : size;
            resultMode = MeasureSpec::UNSPECIFIED;
        }
        break;
    }
    //noinspection ResourceType
    return MeasureSpec::makeMeasureSpec(resultSize, resultMode);
}

View&ViewGroup::addView(View* view){
    return addView(view,-1);
}

View& ViewGroup::addView(View* child, int index){
    LayoutParams* params = child->getLayoutParams();
    if (params == nullptr) {
        params = generateDefaultLayoutParams();
        LOGE_IF(params == nullptr,"generateDefaultLayoutParams() cannot return null");
    }
    return addView(child, index, params);
}

View& ViewGroup::addView(View* child, int width, int height){
    LayoutParams* params = generateDefaultLayoutParams();
    params->width = width;
    params->height = height;
    return addView(child, -1, params);
}

View& ViewGroup::addView(View* child, LayoutParams* params){
    return  addView(child, -1, params);
}

void ViewGroup::removeView(View* view){
    if(removeViewInternal(view)){
        requestLayout();
        invalidate();
    }
}

View&ViewGroup::addView(View* child, int index,LayoutParams* params){
    if(child==nullptr)
         throw "Cannot add a null child view to a ViewGroup";
    requestLayout();
    invalidate(true);
    return addViewInner(child, index, params, false);
}

void ViewGroup::addInArray(View* child, int index){
    if(index>=mChildren.size())
        mChildren.push_back(child);
    else
        mChildren.insert(mChildren.begin()+index,child);
}

void ViewGroup::cleanupLayoutState(View* child)const{
    child->mPrivateFlags &= ~PFLAG_FORCE_LAYOUT;
}

View& ViewGroup::addViewInner(View* child, int index,LayoutParams* params,bool preventRequestLayout){
    if (!checkLayoutParams(params)){
        LOGD("checkLayoutParams(%p) failed",params);
        params = generateLayoutParams(params);
    }

    if(child->mParent){
        throw "The specified child already has a parent. you must call removeView() on the child's parent first.";
    }
    if (!checkLayoutParams(params)) {
        params = generateLayoutParams(params);
    }
    if (preventRequestLayout) {
        child->mLayoutParams = params;
    } else {
        child->setLayoutParams(params);
    }

    if (index < 0) index =mChildren.size();

    addInArray(child, index);

    // tell our children
    if (preventRequestLayout) {
        child->setParent(this);
    } else {
        child->mParent = this;
        child->onAttached();
    }
    //if (child->hasUnhandledKeyListener()) incrementChildUnhandledKeyListeners();

    bool childHasFocus = child->hasFocus();
    if (childHasFocus) requestChildFocus(child, child->findFocus());

    /*AttachInfo ai = mAttachInfo;
    if (ai != null && (mGroupFlags & FLAG_PREVENT_DISPATCH_ATTACHED_TO_WINDOW) == 0) {
        bool lastKeepOn = ai.mKeepScreenOn;
        ai.mKeepScreenOn = false;
        child.dispatchAttachedToWindow(mAttachInfo, (mViewFlags&VISIBILITY_MASK));
        if (ai.mKeepScreenOn) {
            needGlobalAttributesUpdate(true);
        }
        ai.mKeepScreenOn = lastKeepOn;
    }*/

    if (child->isLayoutDirectionInherited()) child->resetRtlProperties();

    dispatchViewAdded(child);

    if ((child->mViewFlags & DUPLICATE_PARENT_STATE) == DUPLICATE_PARENT_STATE) {
        mGroupFlags |= FLAG_NOTIFY_CHILDREN_ON_DRAWABLE_STATE_CHANGE;
    }

    if (child->hasTransientState()) childHasTransientStateChanged(child, true);

    //if (child->getVisibility() !=GONE) notifySubtreeAccessibilityStateChangedIfNeeded();

    /*if (mTransientIndices != nullptr) {
        int transientCount = mTransientIndices.size();
        for (int i = 0; i < transientCount; ++i) {
            int oldIndex = mTransientIndices.get(i);
            if (index <= oldIndex) {
                mTransientIndices.set(i, oldIndex + 1);
            }
        }
    }

    if (mCurrentDragStartEvent != nullptr && child->getVisibility() == VISIBLE) {
        notifyChildOfDragStart(child);
    }*/

    if (child->hasDefaultFocus()) {
        // When adding a child that contains default focus, either during inflation or while
        // manually assembling the hierarchy, update the ancestor default-focus chain.
        setDefaultFocus(child);
    }
    //touchAccessibilityNodeProviderIfNeeded(child);
    return *child;
}

LayoutParams* ViewGroup::generateLayoutParams(const AttributeSet& attrs)const{
    return new LayoutParams(getContext(), attrs);
}

LayoutParams*ViewGroup::generateLayoutParams(const LayoutParams* p)const{
    return (LayoutParams*)p;
}

LayoutParams* ViewGroup::generateDefaultLayoutParams()const{
    return new LayoutParams(LayoutParams::WRAP_CONTENT,LayoutParams::WRAP_CONTENT);
}

bool ViewGroup::checkLayoutParams(const LayoutParams* p)const{
    return p!=nullptr;
}

void ViewGroup::onSetLayoutParams(View* child,LayoutParams* layoutParams){
    requestLayout();
}

void ViewGroup::measureChildren(int widthMeasureSpec, int heightMeasureSpec){
    for (auto child:mChildren) {
        if ((child->mViewFlags & VISIBILITY_MASK) != GONE) {
            measureChild(child, widthMeasureSpec, heightMeasureSpec);
        }
    }
}

void ViewGroup::measureChild(View* child, int parentWidthMeasureSpec,int parentHeightMeasureSpec){
    LayoutParams* lp = child->getLayoutParams();
    int childWidthMeasureSpec = getChildMeasureSpec(parentWidthMeasureSpec,
        mPaddingLeft + mPaddingRight, lp->width);
    int childHeightMeasureSpec = getChildMeasureSpec(parentHeightMeasureSpec,
        mPaddingTop + mPaddingBottom, lp->height);
    child->measure(childWidthMeasureSpec, childHeightMeasureSpec);
}

void ViewGroup::measureChildWithMargins(View* child,int parentWidthMeasureSpec, int widthUsed,
            int parentHeightMeasureSpec, int heightUsed){
	MarginLayoutParams* lp = (MarginLayoutParams*) child->getLayoutParams();

    int childWidthMeasureSpec = getChildMeasureSpec(parentWidthMeasureSpec,
        mPaddingLeft + mPaddingRight + lp->leftMargin + lp->rightMargin
                + widthUsed, lp->width);
    int childHeightMeasureSpec = getChildMeasureSpec(parentHeightMeasureSpec,
        mPaddingTop + mPaddingBottom + lp->topMargin + lp->bottomMargin
        + heightUsed, lp->height);
    child->measure(childWidthMeasureSpec, childHeightMeasureSpec);
}

void ViewGroup::removeViewAt(int index){
    removeViewInternal(index, getChildAt(index));
    requestLayout();
    invalidate();
}

void ViewGroup::removeViews(int start, int count){
    removeViewsInternal(start, count);
    requestLayout();
    invalidate();
}

void ViewGroup::removeViewInLayout(View* view){
    removeViewInternal(view);
}
void ViewGroup::removeViewsInLayout(int start,int count){
    removeViewsInternal(start, count);
}

void ViewGroup::removeAllViews(){
    removeAllViewsInLayout();
    requestLayout();
    invalidate();
}

void ViewGroup::removeAllViewsInLayout() {
    int count = mChildren.size();
    if (count <= 0) {
        return;
    }

    View* focused = mFocused;
    bool detach =mParent!=nullptr;// mAttachInfo != nullptr;
    bool bclearChildFocus = false;

    //needGlobalAttributesUpdate(false);
    for (int i = count - 1; i >= 0; i--) {
        View* view = mChildren[i];

        //if (mTransition) mTransition.removeChild(this, view);

        if (view == focused) {
            view->unFocus(nullptr);
            bclearChildFocus = true;
        }

        //view->clearAccessibilityFocus();

        cancelTouchTarget(view);
        cancelHoverTarget(view);

        /*if (view.getAnimation() != nullptr ||
            (mTransitioningViews != nullptr && mTransitioningViews.contains(view))) {
            addDisappearingView(view);
        } else if (detach) {
           view->dispatchDetachedFromWindow();
        }

        if (view->hasTransientState()) {
            childHasTransientStateChanged(view, false);
        }*/

        dispatchViewRemoved(view);

        view->mParent = nullptr;
        mChildren[i] = nullptr;
    }

    if (mDefaultFocus)  clearDefaultFocus(mDefaultFocus);

    if (mFocusedInCluster) clearFocusedInCluster(mFocusedInCluster);

    if (bclearChildFocus) {
        clearChildFocus(focused);
        /*if (!rootViewRequestFocus()) {
            notifyGlobalFocusCleared(focused);
        }*/
    }
    mChildren.clear();
}

bool ViewGroup::removeViewInternal(View* view){
    int index = indexOfChild(view);
    if (index >= 0) {
        removeViewInternal(index, view);
        return true;
    }
    return false;
}

void ViewGroup::removeViewInternal(int index, View* view){
    //if (mTransition != null)mTransition.removeChild(this, view);

    bool _clearChildFocus = false;
    if (view == mFocused) {
        view->unFocus(nullptr);
        _clearChildFocus = true;
    }
    if (view == mFocusedInCluster)clearFocusedInCluster(view);

    //view->clearAccessibilityFocus();

    cancelTouchTarget(view);
    cancelHoverTarget(view);

    if (view->getAnimation() ||isViewTransitioning(view)){
        addDisappearingView(view);
    } else {//if (view.mAttachInfo != null) {
       //view->dispatchDetachedFromWindow();
    }

    if (view->hasTransientState())childHasTransientStateChanged(view, false);

    //needGlobalAttributesUpdate(false);
    removeFromArray(index);

    //if (view->hasUnhandledKeyListener()) decrementChildUnhandledKeyListeners();

    if (view == mDefaultFocus) clearDefaultFocus(view);

    if (_clearChildFocus) {
        clearChildFocus(view);
        //if (!rootViewRequestFocus())notifyGlobalFocusCleared(this);
    }
    dispatchViewRemoved(view);

    //if (view->getVisibility() != View::GONE)notifySubtreeAccessibilityStateChangedIfNeeded();

    /*int transientCount = mTransientIndices == nullptr ? 0 : mTransientIndices.size();
    for (int i = 0; i < transientCount; ++i) {
        int oldIndex = mTransientIndices.get(i);
        if (index < oldIndex) {
            mTransientIndices.set(i, oldIndex - 1);
        }
    }*/
    //if (mCurrentDragStartEvent != nullptr)  mChildrenInterestedInDrag->remove(view);
}

void ViewGroup::removeViewsInternal(int start, int count){
    int end = start + count;

    if (start < 0 || count < 0 || end > mChildren.size()) {
        throw "IndexOutOfBoundsException";//new IndexOutOfBoundsException();
    }

    View* focused = mFocused;
    bool detach = true;//mAttachInfo != nullptr;
    bool _clearChildFocus = false;
    View* _clearDefaultFocus = nullptr;

    for (int i = start; i < end; i++) {
        View* view = mChildren[i];

        //if (mTransition != null) mTransition.removeChild(this, view);

        if (view == focused) {
            view->unFocus(nullptr);
            _clearChildFocus = true;
        }
        if (view == mDefaultFocus)_clearDefaultFocus = view;
        if (view == mFocusedInCluster) clearFocusedInCluster(view);

        //view->clearAccessibilityFocus();

        cancelTouchTarget(view);
        cancelHoverTarget(view);

        /*if (view->getAnimation() != nullptr ||
            (mTransitioningViews != nullptr && mTransitioningViews->contains(view))) {
            addDisappearingView(view);
        } else */if (detach) {
           //view->dispatchDetachedFromWindow();
        }

        if (view->hasTransientState()) childHasTransientStateChanged(view, false);

        //needGlobalAttributesUpdate(false);

        dispatchViewRemoved(view);
    }

    removeFromArray(start, count);

    if (_clearDefaultFocus != nullptr) {
        clearDefaultFocus(_clearDefaultFocus);
    }
    if (_clearChildFocus) {
        clearChildFocus(focused);
        //if (!rootViewRequestFocus()) notifyGlobalFocusCleared(focused);
    }
}

View* ViewGroup::findViewByPredicateTraversal(std::function<bool(const View*)>predicate,View* childToSkip)const{
    if (predicate(this)) {
        return (View*)this;
    }

    for (auto v:mChildren) {

        if (v != childToSkip && (v->mPrivateFlags & PFLAG_IS_ROOT_NAMESPACE) == 0) {
            v = v->findViewByPredicate(predicate);

            if (v != nullptr) {
                return  v;
            }
        }
    }
    return nullptr;

}

View*ViewGroup::findViewById(int id)const{
    for(auto v:mChildren){
       if(v->findViewById(id))return v;
    }
    return nullptr;
}

bool ViewGroup::drawChild(Canvas& canvas, View* child, long drawingTime){
    return child->draw(canvas,this,drawingTime);
}

void ViewGroup::onDebugDrawMargins(Canvas& canvas){
    for (View*c:mChildren){
        c->getLayoutParams()->onDebugDraw(*c, canvas);
    }
}

void ViewGroup::fillRect(Canvas& canvas,int x1, int y1, int x2, int y2) {
    if (x1 != x2 && y1 != y2) {
        canvas.rectangle(x1, y1,std::abs(x2-x1), std::abs(y2-y1));
    }
}

void ViewGroup::drawRect(Canvas& canvas,int x1, int y1, int x2, int y2){
    canvas.move_to(x1,y1);
    canvas.line_to(x2,y1);
    canvas.line_to(x2,y2);
    canvas.line_to(x1,y2);
    canvas.line_to(x1,y1);
}

void ViewGroup::drawCorner(Canvas& c,int x1, int y1, int dx, int dy, int lw) {
#define SIGN(x) ((x)>=0?1:-1)
    fillRect(c, x1, y1, x1 + dx, y1 + lw * SIGN(dy));
    fillRect(c, x1, y1, x1 + lw * SIGN(dx), y1 + dy);
}

void ViewGroup::drawRectCorners(Canvas& canvas, int x1, int y1, int x2, int y2,int lineLength, int lineWidth) {
    drawCorner(canvas, x1, y1, lineLength, lineLength, lineWidth);
    drawCorner(canvas, x1, y2, lineLength, -lineLength, lineWidth);
    drawCorner(canvas, x2, y1, -lineLength, lineLength, lineWidth);
    drawCorner(canvas, x2, y2, -lineLength, -lineLength, lineWidth);
}

void ViewGroup::onDebugDraw(Canvas& canvas){
    // Draw optical bounds
    canvas.set_color(Color::RED);
    canvas.set_line_width(.5f);
    for (View*c :mChildren){
        if (c->getVisibility() != View::GONE) {
            Insets insets = c->getOpticalInsets();
            drawRect(canvas,c->getLeft() + insets.left,c->getTop() + insets.top,
			   c->getRight() - insets.right - 1,c->getBottom() - insets.bottom - 1);
        }
    }
    canvas.stroke();

    // Draw margins
    canvas.set_color(0x70FF00FF);
    onDebugDrawMargins(canvas);//fill

    // Draw clip bounds
    //paint.setColor(DEBUG_CORNERS_COLOR);
    //paint.setStyle(Paint.Style.FILL);
    canvas.set_color(0xFF4080ff); 
    int lineLength = 8;//dipsToPixels(DEBUG_CORNERS_SIZE_DIP);
    int lineWidth = 1;//dipsToPixels(1);
    for (View* c:mChildren){
        if (c->getVisibility() != View::GONE) {
            drawRectCorners(canvas, c->getLeft(), c->getTop(), c->getRight(), c->getBottom(),lineLength, lineWidth);
        }
    }
    canvas.fill();

}

void ViewGroup::dispatchDraw(Canvas&canvas){
    const int childrenCount = mChildren.size();
    int flags = mGroupFlags;

    if ((flags & FLAG_RUN_ANIMATION) != 0 && canAnimate()) {
        bool buildCache = false;//!isHardwareAccelerated();
        for (int i=0;i<mChildren.size();i++){
            View* child=mChildren[i];
            if ((child->mViewFlags & VISIBILITY_MASK) == VISIBLE) {
                LayoutParams* params = child->getLayoutParams();
                attachLayoutAnimationParameters(child, params, i, childrenCount);
                bindLayoutAnimation(child);
            }
        }

        LayoutAnimationController* controller = mLayoutAnimationController;
        if (controller->willOverlap())  mGroupFlags |= FLAG_OPTIMIZE_INVALIDATE;

        controller->start();

        mGroupFlags &= ~FLAG_RUN_ANIMATION;
        mGroupFlags &= ~FLAG_ANIMATION_DONE;

        if (mAnimationListener.onAnimationStart) {
            mAnimationListener.onAnimationStart(*controller->getAnimation());
        }
    }

    int clipSaveCount = 0;
    bool clipToPadding = (flags & CLIP_TO_PADDING_MASK) == CLIP_TO_PADDING_MASK;
    if (clipToPadding) {
        canvas.save();
        clipSaveCount++;
        canvas.rectangle(mScrollX + mPaddingLeft, mScrollY + mPaddingTop,
                mWidth-mPaddingLeft - mPaddingRight, mHeight -mPaddingTop- mPaddingBottom);
    }

    // We will draw our child's animation, let's reset the flag
    mPrivateFlags &= ~PFLAG_DRAW_ANIMATION;
    mGroupFlags &= ~FLAG_INVALIDATE_REQUIRED;

    bool more = false;
    const long drawingTime = SystemClock::uptimeMillis();

    //if (usingRenderNodeProperties) canvas.insertReorderBarrier();
    const int transientCount = mTransientIndices.size();
    int transientIndex = transientCount != 0 ? 0 : -1;
    // Only use the preordered list if not HW accelerated, since the HW pipeline will do the
    // draw reordering internally
    std::vector<View*> preorderedList;
    buildOrderedChildList(preorderedList);
    const bool customOrder = preorderedList.size() && isChildrenDrawingOrderEnabled();
    for (int i = 0; i < childrenCount; i++) {
        while (transientIndex >= 0 && mTransientIndices.at(transientIndex) == i) {
            View* transientChild = mTransientViews.at(transientIndex);
            if ((transientChild->mViewFlags & VISIBILITY_MASK) == VISIBLE ||
                    transientChild->getAnimation() != nullptr) {
                more |= drawChild(canvas, transientChild, drawingTime);
            }
            transientIndex++;
            if (transientIndex >= transientCount) {
                transientIndex = -1;
            }
        }

        int childIndex = getAndVerifyPreorderedIndex(childrenCount, i, customOrder);
        View* child = getAndVerifyPreorderedView(preorderedList, mChildren, childIndex);
        if ((child->mViewFlags & VISIBILITY_MASK) == VISIBLE || child->getAnimation() != nullptr) {
            more |= drawChild(canvas, child, drawingTime);
        }
    }
    while (transientIndex >= 0) {
        // there may be additional transient views after the normal views
        View* transientChild = mTransientViews.at(transientIndex);
        if ((transientChild->mViewFlags & VISIBILITY_MASK) == VISIBLE ||
                transientChild->getAnimation() != nullptr) {
            more |= drawChild(canvas, transientChild, drawingTime);
        }
        transientIndex++;
        if (transientIndex >= transientCount) {
            break;
        }
    }
    preorderedList.clear();

    // Draw any disappearing views that have animations
    if (mDisappearingChildren.size())  {
        std::vector<View*>&disappearingChildren = mDisappearingChildren;
        int disappearingCount = disappearingChildren.size() - 1;
        // Go backwards -- we may delete as animations finish
        for (int i = disappearingCount; i >= 0; i--) {
            View* child = disappearingChildren.at(i);
            more |= drawChild(canvas, child, drawingTime);
        }
    }
    //if (usingRenderNodeProperties) canvas.insertInorderBarrier();
    
    if (DEBUG_DRAW)onDebugDraw(canvas);

    if (clipToPadding) {
        while(clipSaveCount--)canvas.restore();
    }

    // mGroupFlags might have been updated by drawChild()
    flags = mGroupFlags;

    if ((flags & FLAG_INVALIDATE_REQUIRED) == FLAG_INVALIDATE_REQUIRED) {
        invalidate(true);
    }

    if ((flags & FLAG_ANIMATION_DONE) == 0 && (flags & FLAG_NOTIFY_ANIMATION_LISTENER) == 0 &&
            mLayoutAnimationController->isDone() && !more) {
        // We want to erase the drawing cache and notify the listener after the
        // next frame is drawn because one extra invalidate() is caused by
        // drawChild() after the animation is over
        mGroupFlags |= FLAG_NOTIFY_ANIMATION_LISTENER;
        post([this](){notifyAnimationListener();});
    }
}

void ViewGroup::invalidateChild(View*child,Rect&dirty){

    const bool drawAnimation = (child->mPrivateFlags & PFLAG_DRAW_ANIMATION) != 0;

    const bool isOpaque = child->isOpaque() && !drawAnimation 
                 && child->getAnimation() == nullptr ;//&& childMatrix.isIdentity();

    int opaqueFlag = isOpaque ? PFLAG_DIRTY_OPAQUE : PFLAG_DIRTY;

    int location[2]={child->mLeft,child->mTop};

    //if (child->mLayerType != LAYER_TYPE_NONE)
    {
        mPrivateFlags |= PFLAG_INVALIDATED;
        mPrivateFlags &= ~PFLAG_DRAWING_CACHE_VALID;
    }

    ViewGroup*parent=this;
    View* view = parent;
    do {
        view=parent;
        if (drawAnimation) {
            if (view) {
                 view->mPrivateFlags |= PFLAG_DRAW_ANIMATION;
            }/* else if (parent instanceof ViewRootImpl) {
                 ((ViewRootImpl) parent).mIsAnimating = true;
            }*/
        }

        // If the parent is dirty opaque or not dirty, mark it dirty with the opaque
        // flag coming from the child that initiated the invalidate
        if (view) {
            if ((view->mViewFlags & FADING_EDGE_MASK) != 0 && view->getSolidColor() == 0) {
                  opaqueFlag = PFLAG_DIRTY;
            }
            if ((view->mPrivateFlags & PFLAG_DIRTY_MASK) != PFLAG_DIRTY) {
                view->mPrivateFlags = (view->mPrivateFlags & ~PFLAG_DIRTY_MASK) | opaqueFlag;
            }
        }

        parent = parent->invalidateChildInParent(location, dirty);
        /*if (view) { // Account for transform on current parent
            Matrix m = view.getMatrix();
            if (!m.isIdentity()) {
                RectF boundingRect = attachInfo.mTmpTransformRect;
                boundingRect.set(dirty);
                m.mapRect(boundingRect);
                dirty.set((int) Math.floor(boundingRect.left),
                        (int) Math.floor(boundingRect.top),
                        (int) Math.ceil(boundingRect.right),
                        (int) Math.ceil(boundingRect.bottom));
            }
        }*/
    } while (parent);

    //set invalidate region to rootview
    dynamic_cast<ViewGroup*>(view)->mInvalidRgn->do_union((const RectangleInt&)dirty);
}

ViewGroup*ViewGroup::invalidateChildInParent(int* location, Rect& dirty){
    if ((mPrivateFlags & (PFLAG_DRAWN | PFLAG_DRAWING_CACHE_VALID)) != 0) {//0x20 0x8000
        // either DRAWN, or DRAWING_CACHE_VALID
        if ((mGroupFlags & (FLAG_OPTIMIZE_INVALIDATE | FLAG_ANIMATION_DONE))
                != FLAG_OPTIMIZE_INVALIDATE) {
            dirty.offset(location[CHILD_LEFT_INDEX] - mScrollX,
                    location[CHILD_TOP_INDEX] - mScrollY);
            if ((mGroupFlags & FLAG_CLIP_CHILDREN) == 0) {
                //dirty.union(0, 0, mWidth,mHeight);
            }

            if ((mGroupFlags & FLAG_CLIP_CHILDREN) == FLAG_CLIP_CHILDREN) {
                if (!dirty.intersect(0, 0, mWidth, mHeight)) {
                    dirty.setEmpty();
                }
            }

            location[CHILD_LEFT_INDEX]= mLeft;
            location[CHILD_TOP_INDEX] = mTop;
        } else {
            if ((mGroupFlags & FLAG_CLIP_CHILDREN) == FLAG_CLIP_CHILDREN) {
                dirty.set(0, 0, mWidth, mHeight);
            } else {
                // in case the dirty rect extends outside the bounds of this container
                //dirty.union(0, 0, mWidth, mHeight);
            }
            location[CHILD_LEFT_INDEX]= mLeft;
            location[CHILD_TOP_INDEX] = mTop;

            mPrivateFlags &= ~PFLAG_DRAWN;
        }
        mPrivateFlags &= ~PFLAG_DRAWING_CACHE_VALID;
        //if (mLayerType != LAYER_TYPE_NONE) 
            mPrivateFlags |= PFLAG_INVALIDATED;

        return mParent;
    }
    return nullptr;
}

bool ViewGroup::shouldDelayChildPressedState(){
    return true;
}

void ViewGroup::unFocus(View* focused){
   if (mFocused == nullptr) {
         View::unFocus(focused);
   } else {
         mFocused->unFocus(focused);
         mFocused = nullptr;
   }
}

View*ViewGroup::findFocus(){
    if (isFocused()) {
        return this;
    }

    if (mFocused != nullptr) {
        return mFocused->findFocus();
    }
    return nullptr;
}

bool ViewGroup::restoreDefaultFocus(){
    if (mDefaultFocus != nullptr
            && getDescendantFocusability() != FOCUS_BLOCK_DESCENDANTS
            && (mDefaultFocus->mViewFlags & VISIBILITY_MASK) == VISIBLE
            && mDefaultFocus->restoreDefaultFocus()) {
        return true;
    }
    return View::restoreDefaultFocus();
}

bool ViewGroup::hasFocusable(bool allowAutoFocus, bool dispatchExplicit)const{
    if ((mViewFlags & VISIBILITY_MASK) != VISIBLE) {
        return false;
    }

    // Only use effective focusable value when allowed.
    if ((allowAutoFocus || getFocusable() != FOCUSABLE_AUTO) && isFocusable()) {
        return true;
    }

    // Determine whether we have a focused descendant.
    int descendantFocusability = getDescendantFocusability();
    if (descendantFocusability != FOCUS_BLOCK_DESCENDANTS) {
        return hasFocusableChild(dispatchExplicit);
    }
    return false;
}

bool ViewGroup::hasFocusableChild(bool dispatchExplicit)const{
    for (auto child:mChildren){
        // In case the subclass has overridden has[Explicit]Focusable, dispatch
        // to the expected one for each child even though we share logic here.
        if ((dispatchExplicit && child->hasExplicitFocusable())
                || (!dispatchExplicit && child->hasFocusable())) {
            return true;
        }
    }
    return false;
}

void ViewGroup::setDefaultFocus(View* child){
    if (mDefaultFocus != nullptr && mDefaultFocus->isFocusedByDefault()) {
        return;
    }
    mDefaultFocus = child;
    if (mParent) mParent->setDefaultFocus(this);
}

void ViewGroup::clearDefaultFocus(View* child){
    if (mDefaultFocus != child && mDefaultFocus != nullptr
                && mDefaultFocus->isFocusedByDefault()) {
        return;
    }

    mDefaultFocus = nullptr;
    // Search child siblings for default focusables.
    for (View*sibling:mChildren) {
        if (sibling->isFocusedByDefault()) {
            mDefaultFocus = sibling;
            return;
        } else if (mDefaultFocus == nullptr && sibling->hasDefaultFocus()) {
            mDefaultFocus = sibling;
        }
    }

    if (mParent)mParent->clearDefaultFocus(this);
}

bool ViewGroup::onRequestFocusInDescendants(int direction,Rect* previouslyFocusedRect){
    int index;
    int increment;
    int end;
    int count = mChildren.size();
    if ((direction & FOCUS_FORWARD) != 0) {
        index = 0;
        increment = 1;
        end = count;
    } else {
        index = count - 1;
        increment = -1;
        end = -1;
    }
    for (int i = index; i != end; i += increment) {
        View* child = mChildren[i];
        if ((child->mViewFlags & VISIBILITY_MASK) == VISIBLE) {
            if (child->requestFocus(direction, previouslyFocusedRect)) {
                return true;
            }
        }
    }
    return false;
}

bool ViewGroup::requestChildRectangleOnScreen(View* child,Rect& rectangle, bool immediate) {
    return false;
}

void ViewGroup::requestChildFocus(View*child,View*focused){
    if (getDescendantFocusability() == FOCUS_BLOCK_DESCENDANTS) {
       return;
    }
    // Unfocus us, if necessary
    View::unFocus(focused);

    // We had a previous notion of who had focus. Clear it.
    if (mFocused != child) {
        if (mFocused != nullptr)
            mFocused->unFocus(focused);
        mFocused = child;
    }
    if (mParent != nullptr) 
        mParent->requestChildFocus(this, focused);
}

void ViewGroup::clearChildFocus(View* child){
    if (mFocused == nullptr) {
         View::clearFocus();
    } else {
         View* focused = mFocused;
         mFocused = nullptr;
         focused->clearFocus();
    }
}

void ViewGroup::focusableViewAvailable(View*v){
    if (mParent 
             // shortcut: don't report a new focusable view if we block our descendants from
             // getting focus or if we're not visible
             && (getDescendantFocusability() != FOCUS_BLOCK_DESCENDANTS)
             && ((mViewFlags & VISIBILITY_MASK) == VISIBLE)
             && (isFocusableInTouchMode() || !shouldBlockFocusForTouchscreen())
             // shortcut: don't report a new focusable view if we already are focused
             // (and we don't prefer our descendants)
             //
             // note: knowing that mFocused is non-null is not a good enough reason
             // to break the traversal since in that case we'd actually have to find
             // the focused view and make sure it wasn't FOCUS_AFTER_DESCENDANTS and
             // an ancestor of v; this will get checked for at ViewAncestor
             && !(isFocused() && getDescendantFocusability() != FOCUS_AFTER_DESCENDANTS)) {
         mParent->focusableViewAvailable(v);
     }
}

bool ViewGroup::getTouchscreenBlocksFocus()const{ 
    return mGroupFlags & FLAG_TOUCHSCREEN_BLOCKS_FOCUS;
}

bool ViewGroup::shouldBlockFocusForTouchscreen()const{
    return getTouchscreenBlocksFocus()&&!( isKeyboardNavigationCluster()&& (hasFocus()||(findKeyboardNavigationCluster() != this)));
}

View* ViewGroup::focusSearch(View* focused, int direction)const{
    if (nullptr==mParent){//isRootNamespace()) {
        // root namespace means we should consider ourselves the top of the
        // tree for focus searching; otherwise we could be focus searching
        // into other tabs.  see LocalActivityManager and TabHost for more info.
        return FocusFinder::getInstance().findNextFocus((ViewGroup*)this, focused, direction);
    } else {
        return mParent->focusSearch(focused, direction);
    }
    return nullptr;
}

void ViewGroup::offsetDescendantRectToMyCoords(View* descendant,Rect& rect)const{
    offsetRectBetweenParentAndChild(descendant, rect, true, false);
}

void ViewGroup::offsetRectIntoDescendantCoords(View* descendant, Rect& rect)const{
    offsetRectBetweenParentAndChild(descendant, rect, false, false);
}

void ViewGroup::offsetRectBetweenParentAndChild(View* descendant,Rect&rect,bool offsetFromChildToParent, bool clipToBounds)const{
    // already in the same coord system :)
    if (descendant == this) return;

    View* theParent = descendant->getParent();

        // search and offset up to the parent
    while ((theParent != nullptr) && (theParent != this)) {

        if (offsetFromChildToParent) {
            rect.offset(descendant->getX() - descendant->getScrollX(),
                    descendant->getY() - descendant->getScrollY());
            if (clipToBounds) {
               View* p =  theParent;
               bool intersected = rect.intersect(0, 0,p->getWidth(),p->getHeight());
               if (!intersected)  rect.setEmpty();
            }
        } else {
            if (clipToBounds) {
                View* p =theParent;
                bool intersected = rect.intersect(0, 0,p->getWidth(),p->getHeight());
                if (!intersected) rect.setEmpty();
            }
            rect.offset(descendant->getScrollX() - descendant->getX(),
                        descendant->getScrollY() - descendant->getY());
       }

       descendant =theParent;
       theParent = descendant->getParent();
   }

   // now that we are up to this view, need to offset one more time
   // to get into our coordinate space
   if (theParent == this) {
        if (offsetFromChildToParent) {
            rect.offset(descendant->getX() - descendant->getScrollX(),
                    descendant->getY() - descendant->getScrollY());
        } else {
            rect.offset(descendant->getScrollX() - descendant->getX(),
                    descendant->getScrollY() - descendant->getY());
        }
   } else {
         LOGE("parameter must be a descendant of this view");
   }
}

void ViewGroup::offsetChildrenTopAndBottom(int offset){
    for (auto v:mChildren) {
        v->setPos(v->mLeft,v->mTop+offset);
    }
}

bool ViewGroup::canAnimate(){
    return mLayoutAnimationController!=nullptr;
}

void ViewGroup::startLayoutAnimation() {
    if (mLayoutAnimationController) {
        mGroupFlags |= FLAG_RUN_ANIMATION;
        requestLayout();
    }
}

void ViewGroup::scheduleLayoutAnimation() {
    mGroupFlags |= FLAG_RUN_ANIMATION;
}

void ViewGroup::setLayoutAnimation(LayoutAnimationController* controller) {
    mLayoutAnimationController = controller;
    if (mLayoutAnimationController) {
        mGroupFlags |= FLAG_RUN_ANIMATION;
    }
}

LayoutAnimationController*ViewGroup::getLayoutAnimation() {
    return mLayoutAnimationController;
}

void ViewGroup::setLayoutAnimationListener(Animation::AnimationListener animationListener){
    mAnimationListener = animationListener;
}

Animation::AnimationListener ViewGroup::getLayoutAnimationListener(){
    return mAnimationListener;
}

void ViewGroup::bindLayoutAnimation(View* child){
    Animation* a = mLayoutAnimationController->getAnimationForView(child);
    child->setAnimation(a);
}

void ViewGroup::attachLayoutAnimationParameters(View* child,LayoutParams* params, int index, int count) {
    LayoutAnimationController::AnimationParameters* animationParams =
               params->layoutAnimationParameters;
    if (animationParams == nullptr) {
        animationParams = new LayoutAnimationController::AnimationParameters();
        params->layoutAnimationParameters = animationParams;
    }

    animationParams->count = count;
    animationParams->index = index;
}

void ViewGroup::notifyAnimationListener(){
    mGroupFlags &= ~FLAG_NOTIFY_ANIMATION_LISTENER;
    mGroupFlags |= FLAG_ANIMATION_DONE;

    if (mAnimationListener.onAnimationEnd){
        post([this](){
            mAnimationListener.onAnimationEnd(*mLayoutAnimationController->getAnimation());
        });
    }
    invalidate(true);
}

void ViewGroup::addFocusables(std::vector<View*>& views, int direction, int focusableMode)const{
    int focusableCount = views.size();

    const int descendantFocusability = getDescendantFocusability();
    const bool blockFocusForTouchscreen = shouldBlockFocusForTouchscreen();
    const bool focusSelf = (isFocusableInTouchMode() || !blockFocusForTouchscreen);

    if (descendantFocusability == FOCUS_BLOCK_DESCENDANTS) {
        if (focusSelf) {
            View::addFocusables(views, direction, focusableMode);
        }
        return;
    }

    if (blockFocusForTouchscreen) {
        focusableMode |= FOCUSABLES_TOUCH_MODE;
    }

    if ((descendantFocusability == FOCUS_BEFORE_DESCENDANTS) && focusSelf) {
        View::addFocusables(views, direction, focusableMode);
    }

    std::vector<View*> children;
    for_each(mChildren.begin(),mChildren.end(),[&children](View*c){
        if(c->getVisibility()==View::VISIBLE)
           children.push_back(c);
    });
    //FocusFinder::sort(children, 0, children.size(), this, isLayoutRtl());
    for (int i = 0; i < children.size(); ++i) {
        children[i]->addFocusables(views, direction, focusableMode);
    }

    // When set to FOCUS_AFTER_DESCENDANTS, we only add ourselves if
    // there aren't any focusable descendants.  this is
    // to avoid the focus search finding layouts when a more precise search
    // among the focusable children would be more interesting.
    if ((descendantFocusability == FOCUS_AFTER_DESCENDANTS) && focusSelf
            && focusableCount == views.size()) {
        View::addFocusables(views, direction, focusableMode);
    }
}

void ViewGroup::clearFocusedInCluster(){
    View* top = findKeyboardNavigationCluster();
    ViewGroup* parent = this;
    do {
        parent->mFocusedInCluster = nullptr;
        if (parent == top)break;
        parent = parent->getParent();
    } while (parent);
}

void ViewGroup::clearFocusedInCluster(View* child) {
    if (mFocusedInCluster != child) {
        return;
    }
    clearFocusedInCluster();
}

void ViewGroup::addKeyboardNavigationClusters(std::vector<View*>&views,int drection)const{
}

void ViewGroup::transformPointToViewLocal(float point[2],View&child) {
     point[0] += mScrollX - child.getX();
     point[1] += mScrollY - child.getY();
     if (!child.hasIdentityMatrix()) {
         //child.getInverseMatrix().mapPoints(point);
     }
}

void ViewGroup::setTouchscreenBlocksFocusNoRefocus(bool touchscreenBlocksFocus) {
    if (touchscreenBlocksFocus) {
        mGroupFlags |= FLAG_TOUCHSCREEN_BLOCKS_FOCUS;
    } else {
        mGroupFlags &= ~FLAG_TOUCHSCREEN_BLOCKS_FOCUS;
    }
}

bool ViewGroup::isTransformedTouchPointInView(int x,int y, View& child,POINT*outLocalPoint) {
    float point[2] ={(float)x,(float)y};
    transformPointToViewLocal(point,child);
    const bool isInView=child.pointInView(point[0],point[1],0);
    if(isInView && outLocalPoint != nullptr) {
        outLocalPoint->set(x, y);
    }
    return isInView;
}

void ViewGroup::onSizeChanged(int w,int h,int ow,int oh){
}

bool ViewGroup::onStartNestedScroll(View* child, View* target, int nestedScrollAxes){
    return false;
}
void ViewGroup::onNestedScrollAccepted(View* child, View* target, int axes){
    mNestedScrollAxes = axes;
}

void ViewGroup::onStopNestedScroll(View* child){
    // Stop any recursive nested scrolling.
    stopNestedScroll();
    mNestedScrollAxes = 0;
}

void ViewGroup::onNestedScroll(View* target, int dxConsumed, int dyConsumed,int dxUnconsumed, int dyUnconsumed){
    // Re-dispatch up the tree by default
    dispatchNestedScroll(dxConsumed, dyConsumed, dxUnconsumed, dyUnconsumed, nullptr);
}

void ViewGroup::onNestedPreScroll(View* target, int dx, int dy, int*consumed){
    dispatchNestedPreScroll(dx, dy, consumed, nullptr);
}

int  ViewGroup::getNestedScrollAxes()const{
    return mNestedScrollAxes;
}

bool ViewGroup::onNestedFling(View* target, float velocityX, float velocityY, bool consumed){
    return true;
}

bool ViewGroup::onNestedPreFling(View* target, float velocityX, float velocityY){
    return true;
}

void ViewGroup::requestDisallowInterceptTouchEvent(bool disallowIntercept){
    if (disallowIntercept == ((mGroupFlags & FLAG_DISALLOW_INTERCEPT) != 0)) {
        // We're already in this state, assume our ancestors are too
        return;
    }

    if (disallowIntercept) {
        mGroupFlags |= FLAG_DISALLOW_INTERCEPT;
    } else {
        mGroupFlags &= ~FLAG_DISALLOW_INTERCEPT;
    }

    // Pass it up to our parent
    if (mParent) mParent->requestDisallowInterceptTouchEvent(disallowIntercept);
}

bool ViewGroup::onInterceptTouchEvent(MotionEvent& ev){
    if( (ev.getAction() == MotionEvent::ACTION_DOWN)
        && isOnScrollbarThumb(ev.getX(), ev.getY()) )
        return true;
    return false; 
}

bool ViewGroup::dispatchKeyEvent(KeyEvent&event){
    if ((mPrivateFlags & (PFLAG_FOCUSED | PFLAG_HAS_BOUNDS))
          == (PFLAG_FOCUSED | PFLAG_HAS_BOUNDS)) {
        if (View::dispatchKeyEvent(event)) {
            return true;
        }
    } else if (mFocused != nullptr && (mFocused->mPrivateFlags & PFLAG_HAS_BOUNDS)
            == PFLAG_HAS_BOUNDS) {
        if (mFocused->dispatchKeyEvent(event)) {
            return true;
        }
    }
    return View::dispatchKeyEvent(event);
}

bool ViewGroup::dispatchTouchEvent(MotionEvent&ev){
    const int action = ev.getAction();
    const int actionMasked=ev.getActionMasked();
    const float xf = ev.getX();
    const float yf = ev.getY();
    const float scrolledXFloat = xf + mScrollX;
    const float scrolledYFloat = yf + mScrollY;

    if (ev.isTargetAccessibilityFocus() && isAccessibilityFocusedViewOrHost()) {
         ev.setTargetAccessibilityFocus(false);
    }

    bool handled = false;
    if (actionMasked == MotionEvent::ACTION_DOWN) {
        cancelAndClearTouchTargets(&ev);
        resetTouchState();
    }
    // Check for interception.
    bool intercepted=false;
    if((actionMasked == MotionEvent::ACTION_DOWN)||mFirstTouchTarget){
        const bool disallowIntercept = (mGroupFlags & FLAG_DISALLOW_INTERCEPT) != 0;
        if (!disallowIntercept) {
            intercepted = onInterceptTouchEvent(ev);
            ev.setAction(action); // restore action in case it was changed
        }
    }else{
        intercepted=true;
    }
    
    if (intercepted || mFirstTouchTarget != nullptr) {
         ev.setTargetAccessibilityFocus(false);
    }
    // Check for cancelation.
    bool canceled = resetCancelNextUpFlag(this)|| actionMasked == MotionEvent::ACTION_CANCEL;

    bool split=false;
    TouchTarget* newTouchTarget = nullptr;
    bool alreadyDispatchedToNewTouchTarget = false;

    if(!canceled && !intercepted){
        int actionIndex = ev.getActionIndex(); // always 0 for down
        int idBitsToAssign = split ? 1 << ev.getPointerId(actionIndex): TouchTarget::ALL_POINTER_IDS;

        removePointersFromTouchTargets(idBitsToAssign);
        const int childrenCount = mChildren.size();
        if (newTouchTarget == nullptr && childrenCount){
            int x = ev.getX(actionIndex);
            int y = ev.getY(actionIndex);
            for(int i=childrenCount-1;i>=0;i--){
                View*child=mChildren[i];

		if (!canViewReceivePointerEvents(*child) || !isTransformedTouchPointInView(x, y,*child, nullptr)) {
                    ev.setTargetAccessibilityFocus(false);
                    continue;
                }
                newTouchTarget = getTouchTarget(child);
                if (newTouchTarget) {
                    // Child is already receiving touch within its bounds.
                    // Give it the new pointer in addition to the ones it is handling.
                    newTouchTarget->pointerIdBits |= idBitsToAssign;
                    break;
                }
                resetCancelNextUpFlag(child);
                if(dispatchTransformedTouchEvent(ev, false, child, idBitsToAssign)){
                    mLastTouchDownTime = ev.getDownTime();
                    mLastTouchDownIndex=i;
                    mLastTouchDownX = ev.getX() ;
                    mLastTouchDownY = ev.getY() ;
                    newTouchTarget=addTouchTarget(child,idBitsToAssign);
                    alreadyDispatchedToNewTouchTarget = true;
                    break;
                }
                ev.setTargetAccessibilityFocus(false);
            }
            if ( (newTouchTarget == nullptr) && mFirstTouchTarget) {
                // Did not find a child to receive the event.
               // Assign the pointer to the least recently added target.
                newTouchTarget = mFirstTouchTarget;
                while (newTouchTarget->next) {
                     newTouchTarget = newTouchTarget->next;
                }
                newTouchTarget->pointerIdBits |= idBitsToAssign;
            }
        }
    }

    // Dispatch to touch targets.
    if (mFirstTouchTarget == nullptr){
        handled = dispatchTransformedTouchEvent(ev, canceled, nullptr,TouchTarget::ALL_POINTER_IDS);
    }else{
        TouchTarget* predecessor = nullptr;
        TouchTarget* target = mFirstTouchTarget;
        while (target ) {
            TouchTarget* next = target->next;
            if (alreadyDispatchedToNewTouchTarget && target == newTouchTarget) {
                handled = true;
            } else {
                const bool cancelChild = resetCancelNextUpFlag(target->child)|| intercepted;
                if (dispatchTransformedTouchEvent(ev, cancelChild,
                        target->child, target->pointerIdBits)) {
                    handled = true;
                }
                if (cancelChild) {
                   if (predecessor == nullptr) mFirstTouchTarget = next;
                   else  predecessor->next = next;
                   target->recycle();
                   target = next;
                   continue;
                }
           }
           predecessor = target;
           target = next;
       }
    } 

    // Update list of touch targets for pointer up or cancel, if needed.
    if (canceled || actionMasked == MotionEvent::ACTION_UP
        || actionMasked == MotionEvent::ACTION_HOVER_MOVE) {
        resetTouchState();
    } else if (split && actionMasked == MotionEvent::ACTION_POINTER_UP) {
        int actionIndex = ev.getActionIndex();
        int idBitsToRemove = 1 << ev.getPointerId(actionIndex);
        removePointersFromTouchTargets(idBitsToRemove);
    }
    return handled;
}

bool ViewGroup::requestFocus(int direction,Rect*previouslyFocusedRect){
    int descendantFocusability = getDescendantFocusability();

    bool result,took;
    switch (descendantFocusability) {
    case FOCUS_BLOCK_DESCENDANTS:
        result = View::requestFocus(direction, previouslyFocusedRect);
        break;
    case FOCUS_BEFORE_DESCENDANTS:
        took = View::requestFocus(direction, previouslyFocusedRect);
        result = took ? took : onRequestFocusInDescendants(direction,
                       previouslyFocusedRect);
        break;
    case FOCUS_AFTER_DESCENDANTS:
        took = onRequestFocusInDescendants(direction, previouslyFocusedRect);
        result = took ? took : View::requestFocus(direction, previouslyFocusedRect);
        break;
    default:
        LOGW("descendant focusability must be one of FOCUS_BEFORE_DESCENDANTS,"
           " FOCUS_AFTER_DESCENDANTS, FOCUS_BLOCK_DESCENDANTS but is %x",descendantFocusability);
    }
    if (result && !isLayoutValid() && ((mPrivateFlags & PFLAG_WANTS_FOCUS) == 0)) {
        mPrivateFlags |= PFLAG_WANTS_FOCUS;
    }
    return result;
}

bool ViewGroup::restoreFocusNotInCluster(){
    if (mFocusedInCluster != nullptr) {
        // since clusters don't nest; we can assume that a non-null mFocusedInCluster
        // will refer to a view not-in a cluster.
        return restoreFocusInCluster(View::FOCUS_DOWN);
    }
    if (isKeyboardNavigationCluster() || (mViewFlags & VISIBILITY_MASK) != VISIBLE) {
        return false;
    }
    int descendentFocusability = getDescendantFocusability();
    if (descendentFocusability == FOCUS_BLOCK_DESCENDANTS) {
        return View::requestFocus(FOCUS_DOWN, nullptr);
    }
    if (descendentFocusability == FOCUS_BEFORE_DESCENDANTS
        && View::requestFocus(FOCUS_DOWN, nullptr)) {
        return true;
    }
    for (auto child:mChildren){
        if (!child->isKeyboardNavigationCluster()
                && child->restoreFocusNotInCluster()) {
            return true;
        }
    }
    if (descendentFocusability == FOCUS_AFTER_DESCENDANTS && !hasFocusableChild(false)) {
        return View::requestFocus(FOCUS_DOWN, nullptr);
    }
    return false;
}

View* ViewGroup::keyboardNavigationClusterSearch(View* currentCluster,int direction) {
    //checkThread();
    return FocusFinder::getInstance().findNextKeyboardNavigationCluster(
                getRootView()/*this*/, currentCluster, direction);
}

bool ViewGroup::performKeyboardGroupNavigation(int direction){
    View* focused = findFocus();
    if (focused == nullptr && restoreDefaultFocus()) {
        return true;
    }
    View*old=focused;
    View* cluster = focused == nullptr ? keyboardNavigationClusterSearch(nullptr, direction)
                    : focused->keyboardNavigationClusterSearch(nullptr, direction);

    LOGD("Focus changed %p:%d-->%p:%d",old,old?old->mID:-2,focused,focused?focused->mID:-2);
    // Since requestFocus only takes "real" focus directions (and therefore also
    // restoreFocusInCluster), convert forward/backward focus into FOCUS_DOWN.
    int realDirection = direction;
    if (direction == View::FOCUS_FORWARD || direction == View::FOCUS_BACKWARD) {
        realDirection = View::FOCUS_DOWN;
    }

    if (cluster && cluster->isRootNamespace()) {
        // the default cluster. Try to find a non-clustered view to focus.
        if (cluster->restoreFocusNotInCluster()) {
            //playSoundEffect(SoundEffectConstants.getContantForFocusDirection(direction));
            return true;
        }
        // otherwise skip to next actual cluster
        cluster = keyboardNavigationClusterSearch(nullptr, direction);
    }

    if (cluster && cluster->restoreFocusInCluster(realDirection)) {
        //playSoundEffect(SoundEffectConstants.getContantForFocusDirection(direction));
        return true;
    }
    return false;
}

static int isExcludedKeys(int key){
   return key==KEY_MENU||key==KEY_ESCAPE;//||key==KEY_EXIT;
}

void ViewGroup::drawableStateChanged(){
    View::drawableStateChanged();

    if ((mGroupFlags & FLAG_NOTIFY_CHILDREN_ON_DRAWABLE_STATE_CHANGE) != 0) {
        if ((mGroupFlags & FLAG_ADD_STATES_FROM_CHILDREN) != 0) {
            LOGE("addStateFromChildren cannot be enabled if a"
                        " child has duplicateParentState set to true");
        }
        for (auto child:mChildren){
            if ((child->mViewFlags & DUPLICATE_PARENT_STATE) != 0) {
                child->refreshDrawableState();
            }
        }
    }
}

void ViewGroup::jumpDrawablesToCurrentState(){
    View::jumpDrawablesToCurrentState();
    for (auto child:mChildren){
        child->jumpDrawablesToCurrentState();
    }
}

std::vector<int> ViewGroup::onCreateDrawableState()const{
    if ((mGroupFlags & FLAG_ADD_STATES_FROM_CHILDREN) == 0) {
        return View::onCreateDrawableState();
    }
    std::vector<int>state = View::onCreateDrawableState();
    const int N = getChildCount();
    for (int i = 0; i < N; i++) {
        std::vector<int> childState = getChildAt(i)->getDrawableState();
        state = mergeDrawableStates(state, childState);
    }
    return state;
}

void ViewGroup::setAddStatesFromChildren(bool addsStates) {
    if (addsStates) {
        mGroupFlags |= FLAG_ADD_STATES_FROM_CHILDREN;
    } else {
        mGroupFlags &= ~FLAG_ADD_STATES_FROM_CHILDREN;
    }
    refreshDrawableState();
}

bool ViewGroup::addStatesFromChildren() {
    return (mGroupFlags & FLAG_ADD_STATES_FROM_CHILDREN) != 0;
}

void ViewGroup::childDrawableStateChanged(View* child) {
    if ((mGroupFlags & FLAG_ADD_STATES_FROM_CHILDREN) != 0) {
        refreshDrawableState();
    }
}

}  // namespace ui
