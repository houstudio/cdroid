/*********************************************************************************
+ * Copyright (C) [2019] [houzh@msn.com]
+ *
+ * This library is free software; you can redistribute it and/or
+ * modify it under the terms of the GNU Lesser General Public
+ * License as published by the Free Software Foundation; either
+ * version 2.1 of the License, or (at your option) any later version.
+ *
+ * This library is distributed in the hope that it will be useful,
+ * but WITHOUT ANY WARRANTY; without even the implied warranty of
+ * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
+ * Lesser General Public License for more details.
+ *
+ * You should have received a copy of the GNU Lesser General Public
+ * License along with this library; if not, write to the Free Software
+ * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
+ *********************************************************************************/
#include <widget/adapterviewanimator.h>
#include <animation/animatorinflater.h>
namespace cdroid{

DECLARE_WIDGET(AdapterViewAnimator)

AdapterViewAnimator::AdapterViewAnimator(Context* context,const AttributeSet& attrs)
    :AdapterView(context,attrs){
    initViewAnimator();
    std::string res = attrs.getString("inAnimation");
    if(res.empty())
        setInAnimation(getDefaultInAnimation());
    else
        setInAnimation(context,res);
    res = attrs.getString("outAnimation");

    if(res.empty())
        setOutAnimation(getDefaultOutAnimation());
    else
        setOutAnimation(context,res);

    const bool flag = attrs.getBoolean("animateFirstView",true);
    setAnimateFirstView(flag);
    mLoopViews = attrs.getBoolean("loopViews",false);
    initViewAnimator();
}

AdapterViewAnimator::~AdapterViewAnimator(){
    delete mInAnimation;
    delete mOutAnimation;
    for(auto v:mViewsMap)
        delete v.second;
    mViewsMap.clear();
    if(mDataSetObserver){
        mAdapter->unregisterDataSetObserver(mDataSetObserver);
        delete mDataSetObserver;
    }
}

AdapterViewAnimator::ViewAndMetaData::ViewAndMetaData(View* view, int relativeIndex, int adapterPosition, long itemId) {
    this->view = view;
    this->relativeIndex = relativeIndex;
    this->adapterPosition = adapterPosition;
    this->itemId = itemId;
}

void AdapterViewAnimator::initViewAnimator(){
    mInAnimation = nullptr;
    mOutAnimation= nullptr;
    mDataSetObserver = nullptr;
    mPendingCheckForTap = [this](){
        if (mTouchMode == TOUCH_MODE_DOWN_IN_CURRENT_VIEW) {
            View* v = getCurrentView();
            showTapFeedback(v);
        }
    };
}

void AdapterViewAnimator::configureViewAnimator(int numVisibleViews, int activeOffset){
    if (activeOffset > numVisibleViews - 1) {
        // Throw an exception here.
    }
    mMaxNumActiveViews = numVisibleViews;
    mActiveOffset = activeOffset;
    mPreviousViews.clear();
    for(auto v:mViewsMap){
        delete v.second;
    }
    mViewsMap.clear();
    removeAllViewsInLayout();
    mCurrentWindowStart = 0;
    mCurrentWindowEnd = -1;
}

void AdapterViewAnimator::transformViewForTransition(int fromIndex, int toIndex, View* view, bool animate) {
    if (fromIndex == -1) {
        mInAnimation->setTarget(view);
        mInAnimation->start();
    } else if (toIndex == -1) {
        mOutAnimation->setTarget(view);
        mOutAnimation->start();
    }
}

ObjectAnimator* AdapterViewAnimator::getDefaultInAnimation() {
    ObjectAnimator* anim = ObjectAnimator::ofFloat(nullptr,"alpha",{0.0f, 1.0f});
    anim->setDuration(DEFAULT_ANIMATION_DURATION);
    return anim;
}

ObjectAnimator* AdapterViewAnimator::getDefaultOutAnimation() {
    ObjectAnimator* anim = ObjectAnimator::ofFloat(nullptr,"alpha",{1.0f, 0.0f});
    anim->setDuration(DEFAULT_ANIMATION_DURATION);
    return anim;
}

void AdapterViewAnimator::setDisplayedChild(int whichChild){
    setDisplayedChild(whichChild, true);
}

void AdapterViewAnimator::setDisplayedChild(int whichChild, bool animate) {
    if (mAdapter != nullptr) {
        mWhichChild = whichChild;
        if (whichChild >= getWindowSize()) {
            mWhichChild = mLoopViews ? 0 : getWindowSize() - 1;
        } else if (whichChild < 0) {
            mWhichChild = mLoopViews ? getWindowSize() - 1 : 0;
        }

        const bool hasFocus = getFocusedChild() != nullptr;
        // This will clear old focus if we had it
        showOnly(mWhichChild, animate);
        if (hasFocus) {
            // Try to retake focus if we had it
            requestFocus(FOCUS_FORWARD);
        }
    }
}

void AdapterViewAnimator::applyTransformForChildAtIndex(View* child, int relativeIndex) {
    //NOTHING
}


int AdapterViewAnimator::getDisplayedChild() {
    return mWhichChild;
}

void AdapterViewAnimator::showNext(){
    setDisplayedChild(mWhichChild + 1);
}

void AdapterViewAnimator::showPrevious(){
    setDisplayedChild(mWhichChild - 1);
}

int AdapterViewAnimator::modulo(int pos, int size) {
    if (size > 0) {
        return (size + (pos % size)) % size;
    } else {
        return 0;
    }
}

View* AdapterViewAnimator::getViewAtRelativeIndex(int relativeIndex){
    if ((relativeIndex >= 0) && (relativeIndex <= getNumActiveViews() - 1) && (mAdapter != nullptr)) {
        int i = modulo(mCurrentWindowStartUnbounded + relativeIndex, getWindowSize());
        auto it=mViewsMap.find(i);
        if (it!=mViewsMap.end()) {
            return it->second->view;
        }
    }
    return nullptr;
}

int AdapterViewAnimator::getNumActiveViews() {
    if (mAdapter != nullptr) {
        return std::min(getCount() + 1, mMaxNumActiveViews);
    } else {
        return mMaxNumActiveViews;
    }
}

int AdapterViewAnimator::getWindowSize() {
    if (mAdapter != nullptr) {
        const int adapterCount = getCount();
        if (adapterCount <= getNumActiveViews() && mLoopViews) {
            return adapterCount*mMaxNumActiveViews;
        } else {
            return adapterCount;
        }
    } else {
        return 0;
    }
}

AdapterViewAnimator::ViewAndMetaData* AdapterViewAnimator::getMetaDataForChild(View* child){
    for (auto it= mViewsMap.begin();it!=mViewsMap.end();it++) {
        if (it->second->view == child) {
            return it->second;
        }
    }
    return nullptr;
}

LayoutParams* AdapterViewAnimator::createOrReuseLayoutParams(View* v) {
    LayoutParams* currentLp = v->getLayoutParams();
    if (currentLp != nullptr) {
        return currentLp;
    }
    return new LayoutParams(0, 0);
}

void AdapterViewAnimator::refreshChildren() {
    if (mAdapter == nullptr) return;
    for (int i = mCurrentWindowStart; i <= mCurrentWindowEnd; i++) {
        const int index = modulo(i, getWindowSize());

        const int adapterCount = getCount();
        // get the fresh child from the adapter
        View* updatedChild = mAdapter->getView(modulo(i, adapterCount), nullptr, this);

        if (updatedChild->getImportantForAccessibility() == IMPORTANT_FOR_ACCESSIBILITY_AUTO) {
            updatedChild->setImportantForAccessibility(IMPORTANT_FOR_ACCESSIBILITY_YES);
        }
        auto it = mViewsMap.find(index);
        if (mViewsMap.end()!=it) {
            FrameLayout* fl = (FrameLayout*) it->second->view;
            // add the new child to the frame, if it exists
            if (updatedChild != nullptr) {
                // flush out the old child
                fl->removeAllViewsInLayout();
                fl->addView(updatedChild);
            }
        }
    }
}

FrameLayout* AdapterViewAnimator::getFrameForChild() {
    return new FrameLayout(mContext,AttributeSet(mContext,mContext->getPackageName()));
}

void AdapterViewAnimator::showOnly(int childIndex, bool animate) {
    if (mAdapter == nullptr) return;
    const int adapterCount = getCount();
    if (adapterCount == 0) return;

    for (int i = 0; i < mPreviousViews.size(); i++) {
        auto it = mViewsMap.find(mPreviousViews.at(i)); 
        View* viewToRemove = it->second->view;
        mViewsMap.erase(it);
        delete it->second;
        viewToRemove->clearAnimation();
        if (dynamic_cast<ViewGroup*>(viewToRemove)) {
            ViewGroup* vg = (ViewGroup*) viewToRemove;
            vg->removeAllViewsInLayout();
        }
        // applyTransformForChildAtIndex here just allows for any cleanup
        // associated with this view that may need to be done by a subclass
        applyTransformForChildAtIndex(viewToRemove, -1);

        removeViewInLayout(viewToRemove);
    }
    mPreviousViews.clear();
    int newWindowStartUnbounded = childIndex - mActiveOffset;
    int newWindowEndUnbounded = newWindowStartUnbounded + getNumActiveViews() - 1;
    int newWindowStart = std::max(0, newWindowStartUnbounded);
    int newWindowEnd = std::min(adapterCount - 1, newWindowEndUnbounded);

    if (mLoopViews) {
        newWindowStart = newWindowStartUnbounded;
        newWindowEnd = newWindowEndUnbounded;
    }
    const int rangeStart = modulo(newWindowStart, getWindowSize());
    const int rangeEnd = modulo(newWindowEnd, getWindowSize());

    bool wrap = false;
    if (rangeStart > rangeEnd) {
        wrap = true;
    }

    // This section clears out any items that are in our active views list
    // but are outside the effective bounds of our window (this is becomes an issue
    // at the extremities of the list, eg. where newWindowStartUnbounded < 0 or
    // newWindowEndUnbounded > adapterCount - 1
    for (auto vm : mViewsMap) {
        int index=vm.first;
        bool remove = false;
        if (!wrap && (index < rangeStart || index > rangeEnd)) {
            remove = true;
        } else if (wrap && (index > rangeEnd && index < rangeStart)) {
            remove = true;
        }

        if (remove) {
            View* previousView = vm.second->view;
            int oldRelativeIndex = vm.second->relativeIndex;

            mPreviousViews.push_back(index);
            transformViewForTransition(oldRelativeIndex, -1, previousView, animate);
        }
    }

    // If the window has changed
    if (!((newWindowStart == mCurrentWindowStart) && (newWindowEnd == mCurrentWindowEnd) &&
            (newWindowStartUnbounded == mCurrentWindowStartUnbounded))) {
        // Run through the indices in the new range
        for (int i = newWindowStart; i <= newWindowEnd; i++) {

            int index = modulo(i, getWindowSize());
            int oldRelativeIndex;
            auto it=mViewsMap.find(index);
            if (it!=mViewsMap.end()) {
                oldRelativeIndex = it->second->relativeIndex;
            } else {
                oldRelativeIndex = -1;
            }
            int newRelativeIndex = i - newWindowStartUnbounded;

            // If this item is in the current window, great, we just need to apply
            // the transform for it's new relative position in the window, and animate
            // between it's current and new relative positions
            const bool inOldRange = (it!=mViewsMap.end())&& (std::find(mPreviousViews.begin(),
                    mPreviousViews.end(),index)==mPreviousViews.end());

            if (inOldRange) {
                View* view = it->second->view;
                it->second->relativeIndex = newRelativeIndex;
                applyTransformForChildAtIndex(view, newRelativeIndex);
                transformViewForTransition(oldRelativeIndex, newRelativeIndex, view, animate);

                // Otherwise this view is new to the window
            } else {
                // Get the new view from the adapter, add it and apply any transform / animation
                const int adapterPosition = modulo(i, adapterCount);
                View* newView = mAdapter->getView(adapterPosition, nullptr, this);
                const long itemId = mAdapter->getItemId(adapterPosition);

                // We wrap the new view in a FrameLayout so as to respect the contract
                // with the adapter, that is, that we don't modify this view directly
                FrameLayout* fl = getFrameForChild();

                // If the view from the adapter is null, we still keep an empty frame in place
                if (newView != nullptr) {
                    fl->addView(newView);
                }
                mViewsMap.insert(std::pair<int,ViewAndMetaData*>(index, new ViewAndMetaData(fl, newRelativeIndex,
                        adapterPosition, itemId)));
                addChild(fl);
                applyTransformForChildAtIndex(fl, newRelativeIndex);
                transformViewForTransition(-1, newRelativeIndex, fl, animate);
            }
            mViewsMap[index]->view->bringToFront();
        }
        mCurrentWindowStart = newWindowStart;
        mCurrentWindowEnd = newWindowEnd;
        mCurrentWindowStartUnbounded = newWindowStartUnbounded;
        /*if (mRemoteViewsAdapter != nullptr) {
            int adapterStart = modulo(mCurrentWindowStart, adapterCount);
            int adapterEnd = modulo(mCurrentWindowEnd, adapterCount);
            mRemoteViewsAdapter.setVisibleRangeHint(adapterStart, adapterEnd);
        }*/
    }
    requestLayout();
    invalidate();
}

void AdapterViewAnimator::addChild(View* child) {
    addViewInLayout(child, -1, createOrReuseLayoutParams(child));

    // This code is used to obtain a reference width and height of a child in case we need
    // to decide our own size. TODO: Do we want to update the size of the child that we're
    // using for reference size? If so, when?
    if ((mReferenceChildWidth == -1) || (mReferenceChildHeight == -1)) {
        const int measureSpec = MeasureSpec::makeMeasureSpec(0, MeasureSpec::UNSPECIFIED);
        child->measure(measureSpec, measureSpec);
        mReferenceChildWidth = child->getMeasuredWidth();
        mReferenceChildHeight = child->getMeasuredHeight();
    }
}

void AdapterViewAnimator::showTapFeedback(View* v) {
    v->setPressed(true);
}

void AdapterViewAnimator::hideTapFeedback(View* v) {
    v->setPressed(false);
}

void AdapterViewAnimator::cancelHandleClick() {
    View* v = getCurrentView();
    if (v != nullptr) {
        hideTapFeedback(v);
    }
    mTouchMode = TOUCH_MODE_NONE;
}

bool AdapterViewAnimator::onTouchEvent(MotionEvent& ev){
    const int action = ev.getAction();
    bool handled = false;
    View* v = nullptr;
    switch (action) {
    case MotionEvent::ACTION_DOWN:
        if ((v=getCurrentView())!=nullptr) {
            if (isTransformedTouchPointInView(ev.getX(), ev.getY(), *v, nullptr)) {
                mTouchMode = TOUCH_MODE_DOWN_IN_CURRENT_VIEW;
                postDelayed(mPendingCheckForTap, ViewConfiguration::getTapTimeout());
            }
        }
        break;
    case MotionEvent::ACTION_MOVE: break;
    case MotionEvent::ACTION_POINTER_UP: break;
    case MotionEvent::ACTION_UP:
        if (mTouchMode == TOUCH_MODE_DOWN_IN_CURRENT_VIEW) {
            v = getCurrentView();
            ViewAndMetaData* viewData = getMetaDataForChild(v);
            if (v != nullptr) {
                if (isTransformedTouchPointInView(ev.getX(), ev.getY(), *v, nullptr)) {
                    removeCallbacks(mPendingCheckForTap);
                    showTapFeedback(v);
                    postDelayed([this,viewData,v](){
                        if (viewData != nullptr)
                            performItemClick(*v, viewData->adapterPosition,viewData->itemId);
                        else performItemClick(*v, 0, 0);
                    },ViewConfiguration::getPressedStateDuration());
                    handled = true;
                }
            }
        }
        mTouchMode = TOUCH_MODE_NONE;
        break;
    case MotionEvent::ACTION_CANCEL:
        v = getCurrentView();
        if (v != nullptr)  {
            hideTapFeedback(v);
        }
        mTouchMode = TOUCH_MODE_NONE;
    }
    return handled;
}

void AdapterViewAnimator::measureChildren() {
    const int count = getChildCount();
    const int childWidth = getMeasuredWidth() - mPaddingLeft - mPaddingRight;
    const int childHeight = getMeasuredHeight() - mPaddingTop - mPaddingBottom;

    for (int i = 0; i < count; i++) {
        View* child = getChildAt(i);
        child->measure(MeasureSpec::makeMeasureSpec(childWidth, MeasureSpec::EXACTLY),
                       MeasureSpec::makeMeasureSpec(childHeight, MeasureSpec::EXACTLY));
    }
}

void AdapterViewAnimator::onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
    int widthSpecSize = MeasureSpec::getSize(widthMeasureSpec);
    int heightSpecSize = MeasureSpec::getSize(heightMeasureSpec);
    const int widthSpecMode = MeasureSpec::getMode(widthMeasureSpec);
    const int heightSpecMode = MeasureSpec::getMode(heightMeasureSpec);

    const bool haveChildRefSize = (mReferenceChildWidth != -1 && mReferenceChildHeight != -1);

    // We need to deal with the case where our parent hasn't told us how
    // big we should be. In this case we try to use the desired size of the first
    // child added.
    if (heightSpecMode == MeasureSpec::UNSPECIFIED) {
        heightSpecSize = haveChildRefSize ? mReferenceChildHeight + mPaddingTop +
                mPaddingBottom : 0;
    } else if (heightSpecMode == MeasureSpec::AT_MOST) {
        if (haveChildRefSize) {
            const int height = mReferenceChildHeight + mPaddingTop + mPaddingBottom;
            if (height > heightSpecSize) {
                heightSpecSize |= MEASURED_STATE_TOO_SMALL;
            } else {
                heightSpecSize = height;
            }
        }
    }

    if (widthSpecMode == MeasureSpec::UNSPECIFIED) {
        widthSpecSize = haveChildRefSize ? mReferenceChildWidth + mPaddingLeft +
                mPaddingRight : 0;
    } else if (heightSpecMode == MeasureSpec::AT_MOST) {
        if (haveChildRefSize) {
            const int width = mReferenceChildWidth + mPaddingLeft + mPaddingRight;
            if (width > widthSpecSize) {
                widthSpecSize |= MEASURED_STATE_TOO_SMALL;
            } else {
                widthSpecSize = width;
            }
        }
    }

    setMeasuredDimension(widthSpecSize, heightSpecSize);
    measureChildren();
}

void AdapterViewAnimator::checkForAndHandleDataChanged() {
    const bool dataChanged = mDataChanged;
    if (dataChanged) {
        post([this](){
            handleDataChanged();
            // if the data changes, mWhichChild might be out of the bounds of the adapter
            // in this case, we reset mWhichChild to the beginning
            if (mWhichChild >= getWindowSize()) {
                mWhichChild = 0;

                showOnly(mWhichChild, false);
            } else if (mOldItemCount != getCount()) {
                showOnly(mWhichChild, false);
            }
            refreshChildren();
            requestLayout();
        });
    }
    mDataChanged = false;
}

void AdapterViewAnimator::onLayout(bool changed, int left, int top, int width, int height) {
    checkForAndHandleDataChanged();

    const int childCount = getChildCount();
    for (int i = 0; i < childCount; i++) {
        View* child = getChildAt(i);

        const int childWidth = child->getMeasuredWidth();
        const int childHeight= child->getMeasuredHeight();

        child->layout(mPaddingLeft, mPaddingTop, childWidth, childHeight);
    }
}

View* AdapterViewAnimator::getCurrentView(){
    return getViewAtRelativeIndex(mActiveOffset);
}

ObjectAnimator* AdapterViewAnimator::getInAnimation() {
    return mInAnimation;
}

ObjectAnimator* AdapterViewAnimator::getOutAnimation() {
    return mOutAnimation;
}

void AdapterViewAnimator::setInAnimation(ObjectAnimator* inAnimation) {
    mInAnimation=inAnimation;
}

void AdapterViewAnimator::setOutAnimation(ObjectAnimator* outAnimation) {
    mInAnimation=outAnimation;
}

void AdapterViewAnimator::setInAnimation(Context* context, const std::string&resourceID) {
    setInAnimation((ObjectAnimator*) AnimatorInflater::loadAnimator(context, resourceID));
}

void AdapterViewAnimator::setOutAnimation(Context* context, const std::string&resourceID) {
    setOutAnimation((ObjectAnimator*) AnimatorInflater::loadAnimator(context, resourceID));
}

void AdapterViewAnimator::setAnimateFirstView(bool animate) {
    mAnimateFirstTime = animate;
}

int AdapterViewAnimator::getBaseline() {
    return getCurrentView() ? getCurrentView()->getBaseline() : AdapterView::getBaseline();
}

Adapter* AdapterViewAnimator::getAdapter() {
    return mAdapter;
}

void AdapterViewAnimator::setAdapter(Adapter* adapter) {
    if (mAdapter != nullptr && mDataSetObserver != nullptr) {
        mAdapter->unregisterDataSetObserver(mDataSetObserver);
        delete mDataSetObserver;
    }

    mAdapter = adapter;
    checkFocus();

    if (mAdapter != nullptr) {
        mDataSetObserver = new AdapterDataSetObserver(this);
        mAdapter->registerDataSetObserver(mDataSetObserver);
        mItemCount = mAdapter->getCount();
    }
    setFocusable(true);
    mWhichChild = 0;
    showOnly(mWhichChild, false);
}

void AdapterViewAnimator::setSelection(int position) {
    setDisplayedChild(position);
}

View* AdapterViewAnimator::getSelectedView() {
    return getViewAtRelativeIndex(mActiveOffset);
}

/**
* This defers a notifyDataSetChanged on the pending RemoteViewsAdapter if it has not
* connected yet.*/
void AdapterViewAnimator::deferNotifyDataSetChanged() {
    mDeferNotifyDataSetChanged = true;
}

void AdapterViewAnimator::advance() {
    showNext();
}

}//endof namespace
