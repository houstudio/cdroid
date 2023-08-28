#include <algorithm>
#include <view/viewtreeobserver.h>
namespace cdroid {

bool ViewTreeObserver::sIllegalOnDrawModificationIsFatal = false;

void ViewTreeObserver::InternalInsetsInfo::setTouchableInsets(int val) {
    mTouchableInsets = val;
}

void ViewTreeObserver::InternalInsetsInfo::reset() {
    contentInsets.set(0,0,0,0);//setEmpty();
    visibleInsets.set(0,0,0,0);//setEmpty();
    //touchableRegion.setEmpty();
    mTouchableInsets = TOUCHABLE_INSETS_FRAME;
}

bool ViewTreeObserver::InternalInsetsInfo::isEmpty()const{
    return contentInsets.empty()
           && visibleInsets.empty()
           && touchableRegion->empty()
           && mTouchableInsets == TOUCHABLE_INSETS_FRAME;
}

int ViewTreeObserver::InternalInsetsInfo::hashCode() const{
    int result = 0;//contentInsets.hashCode();
    /*result = 31 * result + visibleInsets.hashCode();
    result = 31 * result + touchableRegion.hashCode();
    result = 31 * result + mTouchableInsets;*/
    return result;
}

bool ViewTreeObserver::InternalInsetsInfo::equals(Object* o)const{
    if ((void*)this == (void*)o) return true;
    if (o == nullptr) return false;

    InternalInsetsInfo& other = *(InternalInsetsInfo*)o;
    return mTouchableInsets == other.mTouchableInsets &&
           contentInsets == other.contentInsets &&
           visibleInsets == other.visibleInsets ;//&&
           //touchableRegion.equals(other.touchableRegion);
}

void ViewTreeObserver::InternalInsetsInfo::set(const ViewTreeObserver::InternalInsetsInfo& other) {
    contentInsets = other.contentInsets;//.set(other.contentInsets);
    visibleInsets = other.visibleInsets;//set(other.visibleInsets);
    //touchableRegion.set(other.touchableRegion);
    mTouchableInsets = other.mTouchableInsets;
}


ViewTreeObserver::ViewTreeObserver(Context* context) {
    sIllegalOnDrawModificationIsFatal = true;
    //context.getApplicationInfo().targetSdkVersion >= Build.VERSION_CODES.O;
}

void ViewTreeObserver::merge(ViewTreeObserver& observer) {
    if(observer.mOnWindowAttachListeners.size()){
        mOnWindowAttachListeners.reserve(mOnWindowAttachListeners.size() + observer.mOnWindowAttachListeners.size());
        mOnWindowAttachListeners.insert(mOnWindowAttachListeners.end(),observer.mOnWindowAttachListeners.begin(),
		    observer.mOnWindowAttachListeners.end());
    }

    if(observer.mOnWindowFocusListeners.size()){
        mOnWindowFocusListeners.reserve(mOnWindowFocusListeners.size() + observer.mOnWindowFocusListeners.size());
        mOnWindowFocusListeners.insert(mOnWindowFocusListeners.end(),observer.mOnWindowFocusListeners.begin(),
		    observer.mOnWindowFocusListeners.end());
    }


    if (observer.mOnGlobalFocusListeners.size()) {
	mOnGlobalFocusListeners.reserve(mOnGlobalFocusListeners.size() + observer.mOnGlobalFocusListeners.size());
	mOnGlobalFocusListeners.insert(mOnGlobalFocusListeners.end(),observer.mOnGlobalFocusListeners.begin(),
			observer.mOnGlobalFocusListeners.end());
    }

    if (observer.mOnGlobalLayoutListeners.size()) {
	mOnGlobalLayoutListeners.reserve(mOnGlobalLayoutListeners.size() + observer.mOnGlobalLayoutListeners.size());
	mOnGlobalLayoutListeners.insert(mOnGlobalLayoutListeners.end(),observer.mOnGlobalLayoutListeners.begin(),
			observer.mOnGlobalLayoutListeners.end());
    }

    if (observer.mOnPreDrawListeners.size()) {
	mOnPreDrawListeners.reserve(mOnPreDrawListeners.size() + observer.mOnPreDrawListeners.size());
	mOnPreDrawListeners.insert(mOnPreDrawListeners.end(),observer.mOnPreDrawListeners.begin(),
			observer.mOnPreDrawListeners.end());
    }

    if (observer.mOnDrawListeners.size()) {
	mOnDrawListeners.reserve(mOnDrawListeners.size() + observer.mOnDrawListeners.size());
	mOnDrawListeners.insert(mOnDrawListeners.end(),observer.mOnDrawListeners.begin(),
			observer.mOnDrawListeners.end());
    }

    if (observer.mOnTouchModeChangeListeners.size()) {
	mOnTouchModeChangeListeners.reserve(mOnTouchModeChangeListeners.size() + observer.mOnTouchModeChangeListeners.size());
	mOnTouchModeChangeListeners.insert(mOnTouchModeChangeListeners.end(),observer.mOnTouchModeChangeListeners.begin(),
			observer.mOnTouchModeChangeListeners.end());
    }

    if (observer.mOnComputeInternalInsetsListeners.size()) {
	mOnComputeInternalInsetsListeners.reserve(mOnComputeInternalInsetsListeners.size() + observer.mOnComputeInternalInsetsListeners.size());
	mOnComputeInternalInsetsListeners.insert(mOnComputeInternalInsetsListeners.end(),observer.mOnComputeInternalInsetsListeners.begin(),
			observer.mOnComputeInternalInsetsListeners.end());
    }

    if (observer.mOnScrollChangedListeners.size()) {
	mOnScrollChangedListeners.reserve(mOnScrollChangedListeners.size() + observer.mOnScrollChangedListeners.size());
	mOnScrollChangedListeners.insert(mOnScrollChangedListeners.end(),observer.mOnScrollChangedListeners.begin(),
			observer.mOnScrollChangedListeners.end());
    }

    if (observer.mOnWindowShownListeners.size()) {
	mOnWindowShownListeners.reserve(mOnWindowShownListeners.size() + observer.mOnWindowShownListeners.size());
	mOnWindowShownListeners.insert(mOnWindowShownListeners.end(),observer.mOnWindowShownListeners.begin(),
			observer.mOnWindowShownListeners.end());
    }

    observer.kill();
}

void ViewTreeObserver::addOnWindowAttachListener(const OnWindowAttachListener& listener) {
    checkIsAlive();

    /*if (mOnWindowAttachListeners == nullptr) {
        mOnWindowAttachListeners  = new CopyOnWriteArrayList<OnWindowAttachListener>();
    }*/

    mOnWindowAttachListeners.push_back(listener);
}

void ViewTreeObserver::removeOnWindowAttachListener(const OnWindowAttachListener& victim) {
    checkIsAlive();
    for(auto it = mOnWindowAttachListeners.begin();it!=mOnWindowAttachListeners.end();it++){
        if(it->onWindowDetached==victim.onWindowDetached){
            mOnWindowAttachListeners.erase(it);
	    break;
	}
    }
}

void ViewTreeObserver::addOnWindowFocusChangeListener(OnWindowFocusChangeListener listener) {
    checkIsAlive();

    /*if (mOnWindowFocusListeners == nullptr) {
        mOnWindowFocusListeners  = new CopyOnWriteArrayList<OnWindowFocusChangeListener>();
    }*/

    mOnWindowFocusListeners.push_back(listener);
}

void ViewTreeObserver::removeOnWindowFocusChangeListener(OnWindowFocusChangeListener victim) {
    checkIsAlive();
    if (mOnWindowFocusListeners.empty()) {
        return;
    }
    auto it = std::find(mOnWindowFocusListeners.begin(),mOnWindowFocusListeners.end(),victim);
    if(it!=mOnWindowFocusListeners.end())
       mOnWindowFocusListeners.erase(it);
}

void ViewTreeObserver::addOnGlobalFocusChangeListener(OnGlobalFocusChangeListener listener) {
    checkIsAlive();

    /*if (mOnGlobalFocusListeners == nullptr) {
        mOnGlobalFocusListeners = new CopyOnWriteArrayList<OnGlobalFocusChangeListener>();
    }*/

    mOnGlobalFocusListeners.push_back(listener);
}

void ViewTreeObserver::removeOnGlobalFocusChangeListener(OnGlobalFocusChangeListener victim) {
    checkIsAlive();
    if (mOnGlobalFocusListeners.empty()) {
        return;
    }
    auto it = std::find(mOnGlobalFocusListeners.begin(),mOnGlobalFocusListeners.end(),victim);
    if(it != mOnGlobalFocusListeners.end())
        mOnGlobalFocusListeners.erase(it);
}

void ViewTreeObserver::addOnGlobalLayoutListener(OnGlobalLayoutListener listener) {
    checkIsAlive();

    /*if (mOnGlobalLayoutListeners == nullptr) {
        mOnGlobalLayoutListeners = new CopyOnWriteArray<OnGlobalLayoutListener>();
    }*/

    mOnGlobalLayoutListeners.push_back(listener);
}

//@Deprecated
void ViewTreeObserver::removeGlobalOnLayoutListener(OnGlobalLayoutListener victim) {
    removeOnGlobalLayoutListener(victim);
}

void ViewTreeObserver::removeOnGlobalLayoutListener(OnGlobalLayoutListener victim) {
    checkIsAlive();
    if (mOnGlobalLayoutListeners.empty()) {
        return;
    }
    auto it = std::find(mOnGlobalLayoutListeners.begin(),mOnGlobalLayoutListeners.end(),victim);
    if(it != mOnGlobalLayoutListeners.end())
        mOnGlobalLayoutListeners.erase(it);
}

void ViewTreeObserver::addOnPreDrawListener(OnPreDrawListener listener) {
    checkIsAlive();

    /*if (mOnPreDrawListeners == nullptr) {
        mOnPreDrawListeners = new CopyOnWriteArray<OnPreDrawListener>();
    }*/

    mOnPreDrawListeners.push_back(listener);
}

void ViewTreeObserver::removeOnPreDrawListener(OnPreDrawListener victim) {
    checkIsAlive();
    if (mOnPreDrawListeners.empty()) {
        return;
    }
    auto it = std::find(mOnPreDrawListeners.begin(),mOnPreDrawListeners.end(),victim);
    if(it!=mOnPreDrawListeners.end())
	mOnPreDrawListeners.erase(it);
}

void ViewTreeObserver::addOnWindowShownListener(OnWindowShownListener listener) {
    checkIsAlive();

    /*if (mOnWindowShownListeners == nullptr) {
        mOnWindowShownListeners = new CopyOnWriteArray<OnWindowShownListener>();
    }*/

    mOnWindowShownListeners.push_back(listener);
    if (mWindowShown) {
        listener();//.onWindowShown();
    }
}

void ViewTreeObserver::removeOnWindowShownListener(OnWindowShownListener victim) {
    checkIsAlive();
    if (mOnWindowShownListeners.empty()) {
        return;
    }
    auto it = std::find(mOnWindowShownListeners.begin(),mOnWindowShownListeners.end(),victim);
    if(it!=mOnWindowShownListeners.end())
        mOnWindowShownListeners.erase(it);
}

void ViewTreeObserver::addOnDrawListener(OnDrawListener listener) {
    checkIsAlive();

    /*if (mOnDrawListeners == nullptr) {
        mOnDrawListeners = new ArrayList<OnDrawListener>();
    }*/

    if (mInDispatchOnDraw) {
        /*IllegalStateException ex = new IllegalStateException(
            "Cannot call addOnDrawListener inside of onDraw");
        if (sIllegalOnDrawModificationIsFatal) {
            throw ex;
        } else {
            Log.e("ViewTreeObserver", ex.getMessage(), ex);
        }*/
    }
    mOnDrawListeners.push_back(listener);
}

void ViewTreeObserver::removeOnDrawListener(OnDrawListener victim) {
    checkIsAlive();
    if (mOnDrawListeners.size()) {
        return;
    }
    if (mInDispatchOnDraw) {
        /*IllegalStateException ex = new IllegalStateException(
        if (sIllegalOnDrawModificationIsFatal) {
            FATAL("Cannot call removeOnDrawListener inside of onDraw");
            
        } else {
            Log.e("ViewTreeObserver", ex.getMessage(), ex);
        }*/
    }
    auto it = std::find(mOnDrawListeners.begin(),mOnDrawListeners.end(),victim);
    if(it!=mOnDrawListeners.end())mOnDrawListeners.erase(it);
}

void ViewTreeObserver::addOnScrollChangedListener(OnScrollChangedListener listener) {
    checkIsAlive();

    /*if (mOnScrollChangedListeners == nullptr) {
        mOnScrollChangedListeners = new CopyOnWriteArray<OnScrollChangedListener>();
    }*/

    mOnScrollChangedListeners.push_back(listener);
}

void ViewTreeObserver::removeOnScrollChangedListener(OnScrollChangedListener victim) {
    checkIsAlive();
    if (mOnScrollChangedListeners.size()) {
        return;
    }
    auto it = std::find(mOnScrollChangedListeners.begin(),mOnScrollChangedListeners.end(),victim);
    if(it!=mOnScrollChangedListeners.end())
        mOnScrollChangedListeners.erase(it);
}

void ViewTreeObserver::addOnTouchModeChangeListener(OnTouchModeChangeListener listener) {
    checkIsAlive();

    /*if (mOnTouchModeChangeListeners == nullptr) {
        mOnTouchModeChangeListeners = new CopyOnWriteArrayList<OnTouchModeChangeListener>();
    }*/

    mOnTouchModeChangeListeners.push_back(listener);
}

void ViewTreeObserver::removeOnTouchModeChangeListener(OnTouchModeChangeListener victim) {
    checkIsAlive();
    if (mOnTouchModeChangeListeners.size()) {
        return;
    }
    auto it = std::find(mOnTouchModeChangeListeners.begin(),mOnTouchModeChangeListeners.end(),victim);
    if(it != mOnTouchModeChangeListeners.end())
       mOnTouchModeChangeListeners.erase(it);
}

void ViewTreeObserver::addOnComputeInternalInsetsListener(OnComputeInternalInsetsListener listener) {
    checkIsAlive();

    if (mOnComputeInternalInsetsListeners.size()) {
        //mOnComputeInternalInsetsListeners = new CopyOnWriteArray<OnComputeInternalInsetsListener>();
    }

    mOnComputeInternalInsetsListeners.push_back(listener);
}

void ViewTreeObserver::removeOnComputeInternalInsetsListener(OnComputeInternalInsetsListener victim) {
    checkIsAlive();
    if (mOnComputeInternalInsetsListeners.size()) {
        return;
    }
    auto it = std::find(mOnComputeInternalInsetsListeners.begin(),mOnComputeInternalInsetsListeners.end(),victim);
    if(it!=mOnComputeInternalInsetsListeners.end())
        mOnComputeInternalInsetsListeners.erase(it);
}

void ViewTreeObserver::addOnEnterAnimationCompleteListener(OnEnterAnimationCompleteListener listener) {
    checkIsAlive();
    if (mOnEnterAnimationCompleteListeners.size()) {
        //mOnEnterAnimationCompleteListeners =  new CopyOnWriteArrayList<OnEnterAnimationCompleteListener>();
    }
    mOnEnterAnimationCompleteListeners.push_back(listener);
}

void ViewTreeObserver::removeOnEnterAnimationCompleteListener(OnEnterAnimationCompleteListener listener) {
    checkIsAlive();
    if (mOnEnterAnimationCompleteListeners.size()) {
        return;
    }
    auto it =std::find(mOnEnterAnimationCompleteListeners.begin(),mOnEnterAnimationCompleteListeners.end(),listener);
    if(it != mOnEnterAnimationCompleteListeners.end())
        mOnEnterAnimationCompleteListeners.erase(it);
}

void ViewTreeObserver::checkIsAlive()const{
    if (!mAlive) {
        FATAL("This ViewTreeObserver is not alive, call getViewTreeObserver() again");
    }
}

bool ViewTreeObserver::isAlive()const{
    return mAlive;
}

void ViewTreeObserver::kill(){
    mAlive = false;
}

void ViewTreeObserver::dispatchOnWindowAttachedChange(bool attached) {
    auto listeners = mOnWindowAttachListeners;
    for (OnWindowAttachListener listener : listeners) {
        if (attached) listener.onWindowAttached();
        else listener.onWindowDetached();
    }
}

void ViewTreeObserver::dispatchOnWindowFocusChange(bool hasFocus) {
    auto listeners   = mOnWindowFocusListeners;
    for (OnWindowFocusChangeListener listener : listeners) {
        listener(hasFocus);//.onWindowFocusChanged(hasFocus);
    }
}

void ViewTreeObserver::dispatchOnGlobalFocusChange(View* oldFocus, View* newFocus) {
    auto listeners = mOnGlobalFocusListeners;
    for (OnGlobalFocusChangeListener listener : listeners) {
        listener(oldFocus,newFocus);//.onGlobalFocusChanged(oldFocus, newFocus);
    }
}

void ViewTreeObserver::dispatchOnGlobalLayout() {
    auto &listeners = mOnGlobalLayoutListeners;
    if ( listeners.size() > 0) {
        for (auto listener: mOnGlobalLayoutListeners) {
            listener();//access.get(i).onGlobalLayout();
        }
    }
}

bool ViewTreeObserver::hasOnPreDrawListeners() {
    return  mOnPreDrawListeners.size() > 0;
}

bool ViewTreeObserver::dispatchOnPreDraw() {
    bool cancelDraw = false;
    auto& listeners = mOnPreDrawListeners;
    if ( listeners.size() > 0) {
        for (auto listener:mOnPreDrawListeners) {
            cancelDraw |= !listener();//(access.get(i).onPreDraw());
        }
    }
    return cancelDraw;
}

void ViewTreeObserver::dispatchOnWindowShown() {
    mWindowShown = true;
    auto&listeners = mOnWindowShownListeners;
    if (listeners.size() > 0) {
        for (auto listener:mOnWindowShownListeners) {
            listener();//access.get(i).onWindowShown();
        }
    }
}

void ViewTreeObserver::dispatchOnDraw() {
    if (mOnDrawListeners.size()) {
        mInDispatchOnDraw = true;
        for (auto listener:mOnDrawListeners) {
            listener();//onDraw();
        }
        mInDispatchOnDraw = false;
    }
}

void ViewTreeObserver::dispatchOnTouchModeChanged(bool inTouchMode) {
    auto& listeners =   mOnTouchModeChangeListeners;
    for (OnTouchModeChangeListener listener : listeners) {
        listener(inTouchMode);//.onTouchModeChanged(inTouchMode);
    }
}

void ViewTreeObserver::dispatchOnScrollChanged() {
    auto&listeners = mOnScrollChangedListeners;
    for (auto listener:mOnScrollChangedListeners) {
         listener();//onScrollChanged();
    }
}

bool ViewTreeObserver::hasComputeInternalInsetsListeners() {
    auto listeners =  mOnComputeInternalInsetsListeners;
    return listeners.size() > 0;
}

void ViewTreeObserver::dispatchOnComputeInternalInsets(InternalInsetsInfo& inoutInfo) {
    auto  listeners = mOnComputeInternalInsetsListeners;
    for (auto listener:listeners){
        listener(inoutInfo);//onComputeInternalInsets(inoutInfo);
    }
}

void ViewTreeObserver::dispatchOnEnterAnimationComplete() {
    auto listeners = mOnEnterAnimationCompleteListeners;
    for (OnEnterAnimationCompleteListener listener : listeners) {
        listener();//.onEnterAnimationComplete();
    }
}
}//endof namespace
