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
#include <widgetEx/recyclerview/snaphelper.h>
#include <widgetEx/recyclerview/linearsmoothscroller.h>
#include <widgetEx/recyclerview/orientationhelper.h>

namespace cdroid{

SnapHelper::SnapHelper(){
    mScrolled = false;
    mRecyclerView = nullptr;
    mGravityScroller = nullptr;
    mScrollListener.onScrollStateChanged=[this](RecyclerView& recyclerView, int newState) {
        //super is null;super.onScrollStateChanged(recyclerView, newState);
        if (newState == RecyclerView::SCROLL_STATE_IDLE && mScrolled) {
            mScrolled = false;
            snapToTargetExistingView();
        }
    };

    mScrollListener.onScrolled=[this](RecyclerView& recyclerView, int dx, int dy) {
        if ((dx != 0) || (dy != 0)) {
            mScrolled = true;
        }
    };
}

SnapHelper::~SnapHelper(){
    delete mGravityScroller;
}

bool SnapHelper::onFling(int velocityX, int velocityY) {
    RecyclerView::LayoutManager* layoutManager = mRecyclerView->getLayoutManager();
    if (layoutManager == nullptr) {
        return false;
    }
    RecyclerView::Adapter* adapter = mRecyclerView->getAdapter();
    if (adapter == nullptr) {
        return false;
    }
    const int minFlingVelocity = mRecyclerView->getMinFlingVelocity();
    return (std::abs(velocityY) > minFlingVelocity || std::abs(velocityX) > minFlingVelocity)
            && snapFromFling(*layoutManager, velocityX, velocityY);
}

void SnapHelper::attachToRecyclerView(RecyclerView* recyclerView){
    if (mRecyclerView == recyclerView) {
        return; // nothing to do
    }
    if (mRecyclerView != nullptr) {
        destroyCallbacks();
    }
    mRecyclerView = recyclerView;
    if (mRecyclerView != nullptr) {
        setupCallbacks();
        mGravityScroller = new Scroller(mRecyclerView->getContext(), DecelerateInterpolator::Instance);
        snapToTargetExistingView();
    }
}

void SnapHelper::setupCallbacks(){
    if (mRecyclerView->getOnFlingListener() != nullptr) {
        FATAL("An instance of OnFlingListener already set.");
    }
    mRecyclerView->addOnScrollListener(mScrollListener);
    RecyclerView::OnFlingListener ls = [this](int velocityX,int velocityY){
        return onFling(velocityX,velocityY);
    };
    mRecyclerView->setOnFlingListener(ls);
}

void SnapHelper::destroyCallbacks() {
    mRecyclerView->removeOnScrollListener(mScrollListener);
    mRecyclerView->setOnFlingListener(nullptr);
}

void SnapHelper::calculateScrollDistance(int velocityX, int velocityY,int snapDistance[2]) {
    mGravityScroller->fling(0, 0, velocityX, velocityY, INT_MIN, INT_MAX, INT_MIN, INT_MAX);
    snapDistance[0] = mGravityScroller->getFinalX();
    snapDistance[1] = mGravityScroller->getFinalY();
}

bool SnapHelper::snapFromFling(RecyclerView::LayoutManager& layoutManager, int velocityX,int velocityY) {
    PointF scrollVector;
    if(!layoutManager.computeScrollVectorForPosition(0,scrollVector)){
        return false;
    }

    const int targetPosition = findTargetSnapPosition(layoutManager, velocityX, velocityY);
    if (targetPosition == RecyclerView::NO_POSITION) {
        return false;
    }

    RecyclerView::SmoothScroller* smoothScroller = createScroller(layoutManager);
    if (smoothScroller == nullptr) {
        return false;
    }

    smoothScroller->setTargetPosition(targetPosition);
    layoutManager.startSmoothScroll(smoothScroller);
    return true;
}

void SnapHelper::snapToTargetExistingView() {
    if (mRecyclerView == nullptr) {
        return;
    }
    RecyclerView::LayoutManager* layoutManager = mRecyclerView->getLayoutManager();
    if (layoutManager == nullptr) {
        return;
    }
    View* snapView = findSnapView(*layoutManager);
    if (snapView == nullptr) {
        return;
    }
    int snapDistance[2];
    calculateDistanceToFinalSnap(*layoutManager, *snapView,snapDistance);
    if (snapDistance[0] || snapDistance[1]) {
        mRecyclerView->smoothScrollBy(snapDistance[0], snapDistance[1]);
    }
}

RecyclerView::SmoothScroller* SnapHelper::createScroller(RecyclerView::LayoutManager& layoutManager) {
    return SnapHelper::createSnapScroller(layoutManager);
}


class MyLinearSmoothScroller:public LinearSmoothScroller{
private:
    SnapHelper* mSnapHelper;
    RecyclerView* mRecyclerView;
protected:
    void onTargetFound(View* targetView, RecyclerView::State& state, RecyclerView::SmoothScroller::Action& action) {
        if (mRecyclerView == nullptr) {
            // The associated RecyclerView has been removed so there is no action to take.
            return;
        }
        int snapDistances[2]={0,0};
        RecyclerView::LayoutManager*layoutManager = mRecyclerView->getLayoutManager();
        mSnapHelper->calculateDistanceToFinalSnap(*layoutManager,*targetView,snapDistances);
        const int dx = snapDistances[0];
        const int dy = snapDistances[1];
        const int time = calculateTimeForDeceleration(std::max(std::abs(dx), std::abs(dy)));
        if (time > 0) {
            action.update(dx, dy, time, mDecelerateInterpolator);
        }
    }

    float calculateSpeedPerPixel(DisplayMetrics& displayMetrics) {
        return SnapHelper::MILLISECONDS_PER_INCH / displayMetrics.densityDpi;
    }
public:
    MyLinearSmoothScroller(SnapHelper*snap,RecyclerView*rv)
	:LinearSmoothScroller(rv->getContext()){
	mSnapHelper = snap;
        mRecyclerView = rv;
    }
};

LinearSmoothScroller* SnapHelper::createSnapScroller(RecyclerView::LayoutManager& layoutManager) {
    PointF scrollVector;
    if(!layoutManager.computeScrollVectorForPosition(0,scrollVector)){
        return nullptr;
    }
    return new MyLinearSmoothScroller(this,mRecyclerView);
}

}/*endof namespace*/
