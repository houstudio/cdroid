#include <widgetEx/wear/dismissableframelayout.h>
#include <widgetEx/wear/backbuttondismisscontroller.h>
namespace cdroid{

DismissibleFrameLayout::DismissibleFrameLayout(Context* context,const AttributeSet& attrs)
    :FrameLayout(context, attrs/*, defStyle, defStyleRes*/){

    mContext = context;
    setSwipeDismissible(true);//WearableNavigationHelper::isSwipeToDismissEnabled(context));
    setBackButtonDismissible(false);
    mDismissListener.onDismissStarted=[this](){
        performDismissStartedCallbacks();
    };
    mDismissListener.onDismissCanceled=[this](){
        performDismissCanceledCallbacks();
    };
    mDismissListener.onDismissed =[this](){
        performDismissFinishedCallbacks();
    };
}

void DismissibleFrameLayout::registerCallback(const Callback& callback) {
    mCallbacks.push_back(callback);
}

void DismissibleFrameLayout::unregisterCallback(const Callback& callback) {
    auto it = std::find(mCallbacks.begin(),mCallbacks.end(),callback);
    if (it==mCallbacks.end()) {
        throw std::runtime_error("removeCallback called with nonexistent callback");
    }
}

void DismissibleFrameLayout::setSwipeDismissible(bool swipeDismissible) {
    if (swipeDismissible) {
        if (mSwipeDismissController == nullptr) {
            mSwipeDismissController = new SwipeDismissController(mContext, this);
            mSwipeDismissController->setOnDismissListener(mDismissListener);
        }
    } else if (mSwipeDismissController != nullptr) {
        mSwipeDismissController->setOnDismissListener({});//nullptr);
        mSwipeDismissController = nullptr;
    }
}

/** Returns true if the frame layout can be dismissed by swipe gestures. */
bool DismissibleFrameLayout::isDismissableBySwipe() const{
    return mSwipeDismissController != nullptr;
}

void DismissibleFrameLayout::setBackButtonDismissible(bool backButtonDismissible) {
    if (backButtonDismissible) {
        if (mBackButtonDismissController == nullptr) {
            mBackButtonDismissController = new BackButtonDismissController(mContext, this);
            mBackButtonDismissController->setOnDismissListener(mDismissListener);
        }
    } else if (mBackButtonDismissController != nullptr) {
        mBackButtonDismissController->disable(this);
        mBackButtonDismissController = nullptr;
    }
}

/** Returns true if the frame layout would be dismissed with back button click */
bool DismissibleFrameLayout::isDismissableByBackButton()  const{
    return mBackButtonDismissController != nullptr;
}

SwipeDismissController* DismissibleFrameLayout::getSwipeDismissController() const{
    return mSwipeDismissController;
}

void DismissibleFrameLayout::performDismissFinishedCallbacks() {
    for (int i = mCallbacks.size() - 1; i >= 0; i--) {
        mCallbacks.at(i).onDismissFinished(*this);
    }
}

void DismissibleFrameLayout::performDismissStartedCallbacks() {
    for (int i = mCallbacks.size() - 1; i >= 0; i--) {
        mCallbacks.at(i).onDismissStarted(*this);
    }
}

void DismissibleFrameLayout::performDismissCanceledCallbacks() {
    for (int i = mCallbacks.size() - 1; i >= 0; i--) {
        mCallbacks.at(i).onDismissCanceled(*this);
    }
}

void DismissibleFrameLayout::requestDisallowInterceptTouchEvent(bool disallowIntercept) {
    if (mSwipeDismissController != nullptr) {
        mSwipeDismissController->requestDisallowInterceptTouchEvent(disallowIntercept);
    } else {
        FrameLayout::requestDisallowInterceptTouchEvent(disallowIntercept);
    }
}

bool DismissibleFrameLayout::onInterceptTouchEvent(MotionEvent& ev) {
    if (mSwipeDismissController != nullptr) {
        return mSwipeDismissController->onInterceptTouchEvent(ev);
    }
    return FrameLayout::onInterceptTouchEvent(ev);
}

bool DismissibleFrameLayout::canScrollHorizontally(int direction) {
    if (mSwipeDismissController != nullptr) {
        return mSwipeDismissController->canScrollHorizontally(direction);
    }
    return FrameLayout::canScrollHorizontally(direction);
}

bool DismissibleFrameLayout::onTouchEvent(MotionEvent& ev) {
    if ((mSwipeDismissController != nullptr)
            && mSwipeDismissController->onTouchEvent(ev)) {
        return true;
    }
    return FrameLayout::onTouchEvent(ev);
}
}

