#include <animation/layouttransition.h>
#include <widget/viewgroup.h>
#include <animation/objectanimator.h>

namespace cdroid{

TimeInterpolator* LayoutTransition::ACCEL_DECEL_INTERPOLATOR         = new AccelerateDecelerateInterpolator();
TimeInterpolator* LayoutTransition::DECEL_INTERPOLATOR               = new DecelerateInterpolator();
TimeInterpolator* LayoutTransition::sAppearingInterpolator           = ACCEL_DECEL_INTERPOLATOR;
TimeInterpolator* LayoutTransition::sDisappearingInterpolator        = ACCEL_DECEL_INTERPOLATOR;
TimeInterpolator* LayoutTransition::sChangingAppearingInterpolator   = DECEL_INTERPOLATOR;
TimeInterpolator* LayoutTransition::sChangingDisappearingInterpolator= DECEL_INTERPOLATOR;
TimeInterpolator* LayoutTransition::sChangingInterpolator            = DECEL_INTERPOLATOR;

Animator* LayoutTransition::defaultChange   = nullptr ;
Animator* LayoutTransition::defaultChangeIn = nullptr ;
Animator* LayoutTransition::defaultChangeOut= nullptr ;

void LayoutTransition::setDuration(long duration) {
    mChangingAppearingDuration = duration;
    mChangingDisappearingDuration = duration;
    mChangingDuration = duration;
    mAppearingDuration = duration;
    mDisappearingDuration = duration;
}

void LayoutTransition::setDuration(int transitionType, long duration) {
    switch (transitionType) {
    case CHANGE_APPEARING:
        mChangingAppearingDuration = duration;
        break;
    case CHANGE_DISAPPEARING:
        mChangingDisappearingDuration = duration;
        break;
    case CHANGING:
        mChangingDuration = duration;
        break;
    case APPEARING:
        mAppearingDuration = duration;
        break;
    case DISAPPEARING:
        mDisappearingDuration = duration;
        break;
    }
}

long LayoutTransition::getDuration(int transitionType) {
    switch (transitionType) {
    case CHANGE_APPEARING:
        return mChangingAppearingDuration;
    case CHANGE_DISAPPEARING:
        return mChangingDisappearingDuration;
    case CHANGING:
        return mChangingDuration;
    case APPEARING:
        return mAppearingDuration;
    case DISAPPEARING:
        return mDisappearingDuration;
    }
    // shouldn't reach here
    return 0;
}

void LayoutTransition::enableTransitionType(int transitionType) {
    switch (transitionType) {
    case APPEARING:
        mTransitionTypes |= FLAG_APPEARING;
        break;
    case DISAPPEARING:
        mTransitionTypes |= FLAG_DISAPPEARING;
        break;
    case CHANGE_APPEARING:
        mTransitionTypes |= FLAG_CHANGE_APPEARING;
        break;
    case CHANGE_DISAPPEARING:
        mTransitionTypes |= FLAG_CHANGE_DISAPPEARING;
        break;
    case CHANGING:
        mTransitionTypes |= FLAG_CHANGING;
        break;
    }
}

void LayoutTransition::disableTransitionType(int transitionType){
    switch (transitionType) {
    case APPEARING:
        mTransitionTypes &= ~FLAG_APPEARING;
        break;
    case DISAPPEARING:
        mTransitionTypes &= ~FLAG_DISAPPEARING;
        break;
    case CHANGE_APPEARING:
        mTransitionTypes &= ~FLAG_CHANGE_APPEARING;
        break;
    case CHANGE_DISAPPEARING:
        mTransitionTypes &= ~FLAG_CHANGE_DISAPPEARING;
        break;
    case CHANGING:
        mTransitionTypes &= ~FLAG_CHANGING;
        break;
    }
}

bool LayoutTransition::isTransitionTypeEnabled(int transitionType){
    switch (transitionType) {
    case APPEARING:
        return (mTransitionTypes & FLAG_APPEARING) == FLAG_APPEARING;
    case DISAPPEARING:
        return (mTransitionTypes & FLAG_DISAPPEARING) == FLAG_DISAPPEARING;
    case CHANGE_APPEARING:
        return (mTransitionTypes & FLAG_CHANGE_APPEARING) == FLAG_CHANGE_APPEARING;
    case CHANGE_DISAPPEARING:
        return (mTransitionTypes & FLAG_CHANGE_DISAPPEARING) == FLAG_CHANGE_DISAPPEARING;
    case CHANGING:
        return (mTransitionTypes & FLAG_CHANGING) == FLAG_CHANGING;
    }
    return false;
}

void LayoutTransition::setStartDelay(int transitionType, long delay){
    switch (transitionType) {
    case CHANGE_APPEARING:
        mChangingAppearingDelay = delay;
        break;
    case CHANGE_DISAPPEARING:
        mChangingDisappearingDelay = delay;
        break;
    case CHANGING:
        mChangingDelay = delay;
        break;
    case APPEARING:
        mAppearingDelay = delay;
        break;
    case DISAPPEARING:
        mDisappearingDelay = delay;
        break;
    }
}

long LayoutTransition::getStartDelay(int transitionType) {
    switch (transitionType) {
    case CHANGE_APPEARING:
        return mChangingAppearingDelay;
    case CHANGE_DISAPPEARING:
        return mChangingDisappearingDelay;
    case CHANGING:
        return mChangingDelay;
    case APPEARING:
        return mAppearingDelay;
    case DISAPPEARING:
        return mDisappearingDelay;
    }
    // shouldn't reach here
    return 0;
}

void LayoutTransition::setStagger(int transitionType, long duration) {
    switch (transitionType) {
    case CHANGE_APPEARING:
        mChangingAppearingStagger = duration;
        break;
    case CHANGE_DISAPPEARING:
        mChangingDisappearingStagger = duration;
        break;
    case CHANGING:
        mChangingStagger = duration;
        break;
        // noop other cases
    }
}

long LayoutTransition::getStagger(int transitionType) {
    switch (transitionType) {
    case CHANGE_APPEARING:
        return mChangingAppearingStagger;
    case CHANGE_DISAPPEARING:
        return mChangingDisappearingStagger;
    case CHANGING:
        return mChangingStagger;
    }
    // shouldn't reach here
    return 0;
}

void LayoutTransition::setInterpolator(int transitionType, TimeInterpolator* interpolator) {
    switch (transitionType) {
    case CHANGE_APPEARING:
        mChangingAppearingInterpolator = interpolator;
        break;
    case CHANGE_DISAPPEARING:
        mChangingDisappearingInterpolator = interpolator;
        break;
    case CHANGING:
        mChangingInterpolator = interpolator;
        break;
    case APPEARING:
        mAppearingInterpolator = interpolator;
        break;
    case DISAPPEARING:
        mDisappearingInterpolator = interpolator;
        break;
    }
}

TimeInterpolator* LayoutTransition::getInterpolator(int transitionType) {
    switch (transitionType) {
    case CHANGE_APPEARING:
        return mChangingAppearingInterpolator;
    case CHANGE_DISAPPEARING:
        return mChangingDisappearingInterpolator;
    case CHANGING:
        return mChangingInterpolator;
    case APPEARING:
        return mAppearingInterpolator;
    case DISAPPEARING:
        return mDisappearingInterpolator;
    }
    // shouldn't reach here
    return nullptr;
}

void LayoutTransition::setAnimator(int transitionType, Animator* animator) {
    switch (transitionType) {
    case CHANGE_APPEARING:
        mChangingAppearingAnim = animator;
        break;
    case CHANGE_DISAPPEARING:
        mChangingDisappearingAnim = animator;
        break;
    case CHANGING:
        mChangingAnim = animator;
        break;
    case APPEARING:
        mAppearingAnim = animator;
        break;
    case DISAPPEARING:
        mDisappearingAnim = animator;
        break;
    }
}

Animator* LayoutTransition::getAnimator(int transitionType){
    switch (transitionType) {
    case CHANGE_APPEARING:
        return mChangingAppearingAnim;
    case CHANGE_DISAPPEARING:
        return mChangingDisappearingAnim;
    case CHANGING:
        return mChangingAnim;
        break;
    case APPEARING:
        return mAppearingAnim;
    case DISAPPEARING:
        return mDisappearingAnim;
    }
    return nullptr;
}

bool LayoutTransition::hasListeners()const{
   return mListeners.size()>0;
}

void LayoutTransition::runChangeTransition(ViewGroup* parent, View* newView, int changeReason){
    Animator* baseAnimator  = nullptr;
    Animator* parentAnimator= nullptr;
    long duration;
    switch (changeReason) {
    case APPEARING:
        baseAnimator = mChangingAppearingAnim;
        duration = mChangingAppearingDuration;
        parentAnimator = defaultChangeIn;
        break;
    case DISAPPEARING:
        baseAnimator = mChangingDisappearingAnim;
        duration = mChangingDisappearingDuration;
        parentAnimator = defaultChangeOut;
        break;
    case CHANGING:
        baseAnimator = mChangingAnim;
        duration = mChangingDuration;
        parentAnimator = defaultChange;
        break;
    default:
        // Shouldn't reach here
        duration = 0;
        break;
    }
    // If the animation is null, there's nothing to do
    if (baseAnimator == nullptr)  return;

    // reset the inter-animation delay, in case we use it later
    staggerDelay = 0;

    //ViewTreeObserver observer = parent.getViewTreeObserver();
    //if (!observer.isAlive())return;
    // If the observer's not in a good state, skip the transition
    int numChildren = parent->getChildCount();

    for (int i = 0; i < numChildren; ++i) {
        View* child = parent->getChildAt(i);

        // only animate the views not being added or removed
        if (child != newView) {
            setupChangeAnimation(parent, changeReason, baseAnimator, duration, child);
        }
    }
    if (mAnimateParentHierarchy) {
        ViewGroup* tempParent = parent;
        while (tempParent != nullptr) {
            ViewGroup* parentParent = tempParent->getParent();
            setupChangeAnimation(parentParent, changeReason, parentAnimator, duration, tempParent);
            tempParent = parentParent;

        }
    }

    // This is the cleanup step. When we get this rendering event, we know that all of
    // the appropriate animations have been set up and run. Now we can clear out the
    // layout listeners.
    /*CleanupCallback callback = new CleanupCallback(layoutChangeListenerMap, parent);
    observer.addOnPreDrawListener(callback);
    parent.addOnAttachStateChangeListener(callback);*/
}

void LayoutTransition::setupChangeAnimation(ViewGroup* parent, int changeReason, Animator* baseAnimator,long duration, View* child){
    if(layoutChangeListenerMap.find(child) ==layoutChangeListenerMap.end()){
        return;
    }
    if (child->getWidth() == 0 && child->getHeight() == 0) {
        return;
    }
    Animator* anim = baseAnimator;//->clone();
    auto ita=pendingAnimations.find(child);
    if (ita!=pendingAnimations.end()) {
        Animator*currentAnimator=ita->second;
        currentAnimator->cancel();
       pendingAnimations.erase(ita);
    }
    // Cache the animation in case we need to cancel it later
   pendingAnimations[child]=anim;
}

void LayoutTransition::startChangingAnimations(){
    for (auto  ita : currentChangingAnimations) {
        Animator*anim=ita.second;
        if(dynamic_cast<ObjectAnimator*>(anim)){
            ((ObjectAnimator*)anim)->setCurrentPlayTime(0);
        }
        anim->start();
    }
}
void LayoutTransition::endChangingAnimations(){ 
    for (auto ita : currentChangingAnimations) {
        Animator*anim=ita.second;
        anim->start();
        anim->cancel();//end
    }
    currentChangingAnimations.clear();
}

bool LayoutTransition::isChangingLayout() {
    return (currentChangingAnimations.size() > 0);
}

void LayoutTransition::layoutChange(ViewGroup *parent){
    if (parent->getWindowVisibility() != View::VISIBLE)  return;

    if ((mTransitionTypes & FLAG_CHANGING) == FLAG_CHANGING  && !isRunning()) {
        // This method is called for all calls to layout() in the container, including
        // those caused by add/remove/hide/show events, which will already have set up
        // transition animations. Avoid setting up CHANGING animations in this case; only
        // do so when there is not a transition already running on the container.
        runChangeTransition(parent, nullptr, CHANGING);
    }
}

bool LayoutTransition::isRunning() {
    return (currentChangingAnimations.size() > 0 || currentAppearingAnimations.size() > 0 ||
            currentDisappearingAnimations.size() > 0);
}

void LayoutTransition::cancel(){
    for (auto it:currentChangingAnimations) {
        it.second->cancel();
    }
    currentChangingAnimations.clear();
   
    for(auto it:currentAppearingAnimations)
        it.second->cancel();
    currentAppearingAnimations.clear();

    for(auto it:currentDisappearingAnimations)
        it.second->cancel();
    currentDisappearingAnimations.clear();
}

void LayoutTransition::cancel(int transitionType){
    switch (transitionType) {
    case CHANGE_APPEARING:
    case CHANGE_DISAPPEARING:
    case CHANGING:
        for (auto it : currentChangingAnimations)
            it.second->cancel();
        currentChangingAnimations.clear();
        break;
    case APPEARING:
        for (auto it:currentAppearingAnimations)
            it.second->cancel();
        currentAppearingAnimations.clear();
        break;
    case DISAPPEARING:
        for (auto it :currentDisappearingAnimations)
           it.second->cancel();
        currentDisappearingAnimations.clear();
        break;
    }
}

void LayoutTransition::runAppearingTransition(ViewGroup* parent,View* child){
    auto ita= currentDisappearingAnimations.find(child);
    if (ita!=currentDisappearingAnimations.end()){
        ita->second->cancel();
    }
    if (mAppearingAnim == nullptr) {
        for (auto l:mListeners)
            if(l.endTransition)l.endTransition(*this, parent, child, APPEARING);
        return;
    }
    Animator* anim = mAppearingAnim->clone();
    anim->setTarget(child);
    anim->setStartDelay(mAppearingDelay);
    anim->setDuration(mAppearingDuration);
    if (mAppearingInterpolator != sAppearingInterpolator) {
        anim->setInterpolator(mAppearingInterpolator);
    }
    if (dynamic_cast<ObjectAnimator*>(anim)) {
        ((ObjectAnimator*) anim)->setCurrentPlayTime(0);
    }
    AnimatorListenerAdapter lis;
    lis.onAnimationEnd=[&](Animator& anim,bool reverse){
        auto it=currentAppearingAnimations.find(child);
        if(it!=currentAppearingAnimations.end())
            currentAppearingAnimations.erase(it);
        for (auto l: mListeners)
            if(l.endTransition)l.endTransition(*this, parent, child, APPEARING);
    };
    anim->addListener(lis);

    currentAppearingAnimations[child]= anim;
    anim->start();
}

void LayoutTransition::runDisappearingTransition(ViewGroup* parent,View* child){
    auto it=currentAppearingAnimations.find(child);
    if (it!=currentAppearingAnimations.end()) {
        it->second->cancel();
    }
    if (mDisappearingAnim == nullptr) {
        for (TransitionListener l :mListeners)
            if(l.endTransition)l.endTransition(*this, parent, child, DISAPPEARING);
        return;
    }
    Animator* anim = mDisappearingAnim->clone();
    anim->setStartDelay(mDisappearingDelay);
    anim->setDuration(mDisappearingDuration);
    if (mDisappearingInterpolator != sDisappearingInterpolator) {
        anim->setInterpolator(mDisappearingInterpolator);
    }
    anim->setTarget(child);
    float preAnimAlpha = child->getAlpha();
    AnimatorListenerAdapter lis;
    lis.onAnimationEnd=[&](Animator& anim,bool reverse){
        auto it=currentDisappearingAnimations.find(child);
        if(it!=currentDisappearingAnimations.end())
            currentDisappearingAnimations.erase(it);
        child->setAlpha(preAnimAlpha);
        for (TransitionListener l :mListeners)
            if(l.endTransition)l.endTransition(*this, parent, child, DISAPPEARING);
    };
    anim->addListener(lis);

    if (dynamic_cast<ObjectAnimator*>(anim)) {
        ((ObjectAnimator*) anim)->setCurrentPlayTime(0);
    }
    currentDisappearingAnimations[child]= anim;
    anim->start();
}

void LayoutTransition::addChild(ViewGroup* parent, View* child, bool changesLayout){
    if ((mTransitionTypes & FLAG_APPEARING) == FLAG_APPEARING) {
        // Want disappearing animations to finish up before proceeding
        cancel(DISAPPEARING);
    }
    if (changesLayout && (mTransitionTypes & FLAG_CHANGE_APPEARING) == FLAG_CHANGE_APPEARING) {
        // Also, cancel changing animations so that we start fresh ones from current locations
        cancel(CHANGE_APPEARING);
        cancel(CHANGING);
    }
    if (hasListeners() && (mTransitionTypes & FLAG_APPEARING) == FLAG_APPEARING) {
        for (auto l:mListeners) {
            if(l.startTransition)l.startTransition(*this, parent, child, APPEARING);
        }
    }
    if (changesLayout && (mTransitionTypes & FLAG_CHANGE_APPEARING) == FLAG_CHANGE_APPEARING) {
        runChangeTransition(parent, child, APPEARING);
    }
    if ((mTransitionTypes & FLAG_APPEARING) == FLAG_APPEARING) {
        runAppearingTransition(parent, child);
    }
}

void LayoutTransition::removeChild(ViewGroup* parent, View* child, bool changesLayout){
    if (parent->getWindowVisibility() != View::VISIBLE) return;
   
    if ((mTransitionTypes & FLAG_DISAPPEARING) == FLAG_DISAPPEARING) {
        // Want appearing animations to finish up before proceeding
        cancel(APPEARING);
    }
    if (changesLayout &&  (mTransitionTypes & FLAG_CHANGE_DISAPPEARING) == FLAG_CHANGE_DISAPPEARING) {
        // Also, cancel changing animations so that we start fresh ones from current locations
        cancel(CHANGE_DISAPPEARING);
        cancel(CHANGING);
    }
    if (hasListeners() && (mTransitionTypes & FLAG_DISAPPEARING) == FLAG_DISAPPEARING) {
        for (auto l :mListeners) {
            if(l.startTransition)l.startTransition(*this, parent, child, DISAPPEARING);
        }
    }
    if (changesLayout && (mTransitionTypes & FLAG_CHANGE_DISAPPEARING) == FLAG_CHANGE_DISAPPEARING) {
        runChangeTransition(parent, child, DISAPPEARING);
    }
    if ((mTransitionTypes & FLAG_DISAPPEARING) == FLAG_DISAPPEARING) {
        runDisappearingTransition(parent, child);
    }
}

void LayoutTransition::addChild(ViewGroup* parent, View* child){
    addChild(parent, child, true);
}

void LayoutTransition::removeChild(ViewGroup* parent, View* child){
    removeChild(parent,child,true);
}

void LayoutTransition::hideChild(ViewGroup* parent, View* child){
    removeChild(parent, child, true);
}

void LayoutTransition::hideChild(ViewGroup* parent, View* child, int newVisibility){
    removeChild(parent, child, newVisibility == View::GONE);
}

void LayoutTransition::showChild(ViewGroup* parent, View* child, int oldVisibility){
    addChild(parent, child, oldVisibility == View::GONE);
}

void LayoutTransition::addTransitionListener(TransitionListener& listener){
    mListeners.push_back(listener);
}

void LayoutTransition::removeTransitionListener(TransitionListener& listener){
    for(auto it=mListeners.begin();it!=mListeners.end();it++){
        if(it->startTransition==listener.startTransition && it->endTransition==listener.endTransition){
            mListeners.erase(it);
            break;
        }
    }
}

}//endof namespace
