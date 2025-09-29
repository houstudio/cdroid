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
#include <view/viewgroup.h>
#include <animation/layouttransition.h>
#include <animation/objectanimator.h>

namespace cdroid{

TimeInterpolator* LayoutTransition::ACCEL_DECEL_INTERPOLATOR         = AccelerateDecelerateInterpolator::gAccelerateDecelerateInterpolator.get();
TimeInterpolator* LayoutTransition::DECEL_INTERPOLATOR               = DecelerateInterpolator::gDecelerateInterpolator.get();
TimeInterpolator* LayoutTransition::sAppearingInterpolator           = ACCEL_DECEL_INTERPOLATOR;
TimeInterpolator* LayoutTransition::sDisappearingInterpolator        = ACCEL_DECEL_INTERPOLATOR;
TimeInterpolator* LayoutTransition::sChangingAppearingInterpolator   = DECEL_INTERPOLATOR;
TimeInterpolator* LayoutTransition::sChangingDisappearingInterpolator= DECEL_INTERPOLATOR;
TimeInterpolator* LayoutTransition::sChangingInterpolator            = DECEL_INTERPOLATOR;

Animator* LayoutTransition::defaultChange   = nullptr ;
Animator* LayoutTransition::defaultChangeIn = nullptr ;
Animator* LayoutTransition::defaultChangeOut= nullptr ;
Animator* LayoutTransition::defaultFadeIn = nullptr;
Animator* LayoutTransition::defaultFadeOut= nullptr;

LayoutTransition::LayoutTransition() {
    if (defaultChangeIn == nullptr) {
        // "left" is just a placeholder; we'll put real properties/values in when needed
        PropertyValuesHolder* pvhLeft = PropertyValuesHolder::ofInt("left",{0, 1});
        PropertyValuesHolder* pvhTop = PropertyValuesHolder::ofInt("top", {0, 1});
        PropertyValuesHolder* pvhRight = PropertyValuesHolder::ofInt("right",{0, 1});
        PropertyValuesHolder* pvhBottom = PropertyValuesHolder::ofInt("bottom",{0, 1});
        PropertyValuesHolder* pvhScrollX = PropertyValuesHolder::ofInt("scrollX",{0, 1});
        PropertyValuesHolder* pvhScrollY = PropertyValuesHolder::ofInt("scrollY",{0, 1});
        defaultChangeIn = ObjectAnimator::ofPropertyValuesHolder(nullptr,
			{pvhLeft, pvhTop, pvhRight, pvhBottom, pvhScrollX, pvhScrollY});
        defaultChangeIn->setDuration(DEFAULT_DURATION);
        defaultChangeIn->setStartDelay(mChangingAppearingDelay);
        defaultChangeIn->setInterpolator(mChangingAppearingInterpolator);
        defaultChangeOut = defaultChangeIn->clone();
        defaultChangeOut->setStartDelay(mChangingDisappearingDelay);
        defaultChangeOut->setInterpolator(mChangingDisappearingInterpolator);
        defaultChange = defaultChangeIn->clone();
        defaultChange->setStartDelay(mChangingDelay);
        defaultChange->setInterpolator(mChangingInterpolator);

        defaultFadeIn = ObjectAnimator::ofFloat(nullptr,"alpha",{0.f, 1.f});
        defaultFadeIn->setDuration(DEFAULT_DURATION);
        defaultFadeIn->setStartDelay(mAppearingDelay);
        defaultFadeIn->setInterpolator(mAppearingInterpolator);
        defaultFadeOut = ObjectAnimator::ofFloat(nullptr,"alpha",{1.f, 0.f});
        defaultFadeOut->setDuration(DEFAULT_DURATION);
        defaultFadeOut->setStartDelay(mDisappearingDelay);
        defaultFadeOut->setInterpolator(mDisappearingInterpolator);
    }
    mChangingAppearingAnim = defaultChangeIn;
    mChangingDisappearingAnim = defaultChangeOut;
    mChangingAnim = defaultChange;
    mAppearingAnim = defaultFadeIn;
    mDisappearingAnim = defaultFadeOut;
    staggerDelay = 0;
    mTransitionTypes = FLAG_CHANGE_APPEARING | FLAG_CHANGE_DISAPPEARING | FLAG_APPEARING | FLAG_DISAPPEARING;
}

LayoutTransition::~LayoutTransition(){
    if(mChangingAppearingAnim!=defaultChangeIn)delete mChangingAppearingAnim;
    if(mChangingDisappearingAnim!=defaultChangeOut)delete mChangingDisappearingAnim;
    if(mChangingAnim!=defaultChange)delete mChangingAnim;
    if(mAppearingAnim!=defaultFadeIn)delete mAppearingAnim;
    if(mDisappearingAnim!=defaultFadeOut)delete mDisappearingAnim;
}

void LayoutTransition::setDuration(int64_t duration) {
    mChangingAppearingDuration = duration;
    mChangingDisappearingDuration = duration;
    mChangingDuration = duration;
    mAppearingDuration = duration;
    mDisappearingDuration = duration;
}

void LayoutTransition::setDuration(int transitionType, int64_t duration) {
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

int64_t LayoutTransition::getDuration(int transitionType) const{
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

bool LayoutTransition::isTransitionTypeEnabled(int transitionType)const{
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

void LayoutTransition::setStartDelay(int transitionType, int64_t delay){
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

int64_t LayoutTransition::getStartDelay(int transitionType)const{
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

void LayoutTransition::setStagger(int transitionType, int64_t duration) {
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

int64_t LayoutTransition::getStagger(int transitionType)const{
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
    int64_t duration;
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

    ViewTreeObserver* observer = parent->getViewTreeObserver();
    if (!observer->isAlive())return;
    // If the observer's not in a good state, skip the transition
    const int numChildren = parent->getChildCount();

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
    ViewTreeObserver::OnPreDrawListener onPreDrawListener;
    View::OnAttachStateChangeListener onAttachStateListener;
    onPreDrawListener=[this,parent,onPreDrawListener,onAttachStateListener](){
        parent->getViewTreeObserver()->removeOnPreDrawListener(onPreDrawListener);
        parent->removeOnAttachStateChangeListener(onAttachStateListener);
        for (auto it:layoutChangeListenerMap){
            View*view = it.first;
            view->removeOnLayoutChangeListener(it.second);
        }
        layoutChangeListenerMap.clear();
        return true;
    };
    onAttachStateListener.onViewAttachedToWindow = [](View& v){};
    onAttachStateListener.onViewDetachedFromWindow=[this,parent,onPreDrawListener,onAttachStateListener](View& v){
        parent->getViewTreeObserver()->removeOnPreDrawListener(onPreDrawListener);
        parent->removeOnAttachStateChangeListener(onAttachStateListener);
        for (auto it:layoutChangeListenerMap){
            View*view = it.first;
            view->removeOnLayoutChangeListener(it.second);
        }
        layoutChangeListenerMap.clear();
    };
    observer->addOnPreDrawListener(onPreDrawListener);
    parent->addOnAttachStateChangeListener(onAttachStateListener);
}

void LayoutTransition::setAnimateParentHierarchy(bool animateParentHierarchy) {
    mAnimateParentHierarchy = animateParentHierarchy;
}

void LayoutTransition::setupChangeAnimation(ViewGroup* parent, int changeReason, Animator* baseAnimator,int64_t duration, View* child){
    if(layoutChangeListenerMap.find(child) !=layoutChangeListenerMap.end()){
        return;
    }
    if ((child->getWidth() == 0) && (child->getHeight() == 0)) {
        return;
    }
    Animator* anim = baseAnimator->clone();
    anim->setTarget(child);
    anim->setupStartValues();
    auto ita = pendingAnimations.find(child);
    if (ita != pendingAnimations.end()) {
        Animator*currentAnimator=ita->second;
        currentAnimator->cancel();
        pendingAnimations.erase(ita);
        delete currentAnimator;
    }
    // Cache the animation in case we need to cancel it later
    pendingAnimations.insert({child,anim});
    // For the animations which don't get started, we have to have a means of
    // removing them from the cache, lest we leak them and their target objects.
    // We run an animator for the default duration+100 (an arbitrary time, but one
    // which should far surpass the delay between setting them up here and
    // handling layout events which start them.
    ValueAnimator* pendingAnimRemover = ValueAnimator::ofFloat({0.f, 1.f});
    pendingAnimRemover->setDuration(duration + 100);
    Animator::AnimatorListener al;
    al.onAnimationEnd=[this,child](Animator&anim,bool){
        auto it = pendingAnimations.find(child);
        delete &anim;
        if(it != pendingAnimations.end()){
            delete it->second;
            pendingAnimations.erase(it);
        }
    };
    pendingAnimRemover->addListener(al);
    pendingAnimRemover->start();

    View::OnLayoutChangeListener listener;
    listener = [this,anim,parent,child,changeReason,duration,listener](View& v, int left, int top, int width, int height,
                    int oldLeft, int oldTop, int oldWidth, int oldHeight){

        anim->setupEndValues();
        if (dynamic_cast<ValueAnimator*>(anim)) {
            bool valuesDiffer = false;
            ValueAnimator* valueAnim = (ValueAnimator*)anim;
            std::vector<PropertyValuesHolder*> oldValues = valueAnim->getValues();
            for (int i = 0; i < oldValues.size(); ++i) {
                PropertyValuesHolder* pvh = oldValues[i];
                /*if (dynamic_cast<KeyframeSet*>(pvh)) {
                    KeyframeSet keyframeSet = (KeyframeSet) pvh->mKeyframes;
                    if (keyframeSet.mFirstKeyframe == nullptr ||
                            keyframeSet.mLastKeyframe == nullptr ||
                            !keyframeSet.mFirstKeyframe.getValue().equals(
                                    keyframeSet.mLastKeyframe.getValue())) {
                        valuesDiffer = true;
                    }
                } else if (!pvh.mKeyframes.getValue(0).equals(pvh.mKeyframes.getValue(1))) {
                    valuesDiffer = true;
                }*/
            }
            if (!valuesDiffer) {
                return;
            }
        }

        int64_t startDelay = 0;
        switch (changeReason) {
        case APPEARING:
            startDelay = mChangingAppearingDelay + staggerDelay;
            staggerDelay += mChangingAppearingStagger;
            if (mChangingAppearingInterpolator != sChangingAppearingInterpolator) {
                anim->setInterpolator(mChangingAppearingInterpolator);
            }
            break;
        case DISAPPEARING:
            startDelay = mChangingDisappearingDelay + staggerDelay;
            staggerDelay += mChangingDisappearingStagger;
            if (mChangingDisappearingInterpolator !=
                    sChangingDisappearingInterpolator) {
                anim->setInterpolator(mChangingDisappearingInterpolator);
            }
            break;
        case CHANGING:
            startDelay = mChangingDelay + staggerDelay;
            staggerDelay += mChangingStagger;
            if (mChangingInterpolator != sChangingInterpolator) {
                anim->setInterpolator(mChangingInterpolator);
            }
            break;
        }
        anim->setStartDelay(startDelay);
        anim->setDuration(duration);

        //Animator* prevAnimation = currentChangingAnimations.get(child);
        auto it = currentChangingAnimations.find(child);
        if (it!=currentChangingAnimations.end()){//prevAnimation != nullptr) {
            Animator* prevAnimation = it->second;
            prevAnimation->cancel();
            //delete prevAnimation;
        }
        //Animator* pendingAnimation = pendingAnimations.get(child);
        it = pendingAnimations.find(child);
        if (it!=pendingAnimations.end()){//pendingAnimation != nullptr) {
            Animator*pendingAnimation = it->second;
            pendingAnimations.erase(it);//pendingAnimations->remove(child);
            delete pendingAnimation;
        }
        // Cache the animation in case we need to cancel it later
        currentChangingAnimations.insert({child,anim});//put(child, anim);

        parent->requestTransitionStart(this);

        // this only removes listeners whose views changed - must clear the
        // other listeners later
        child->removeOnLayoutChangeListener(listener);
        auto itc = layoutChangeListenerMap.find(child);
        layoutChangeListenerMap.erase(itc);//layoutChangeListenerMap.remove(child);
    };
  
    al.onAnimationStart = [this,parent,child,changeReason](Animator& animator,bool) {
        if (hasListeners()) {
            std::vector<TransitionListener> listeners =mListeners;
            for (TransitionListener& listener:listeners) {
                listener.startTransition(*this, parent, child, changeReason == APPEARING ?
                                CHANGE_APPEARING : changeReason == DISAPPEARING ?
                                CHANGE_DISAPPEARING : CHANGING);
            }
        }
    };

    al.onAnimationCancel = [this,child,listener](Animator& animator) {
        auto it = layoutChangeListenerMap.find(child);
        child->removeOnLayoutChangeListener(listener);
        layoutChangeListenerMap.erase(it);
    };

    al.onAnimationEnd = [this,parent,child,changeReason](Animator& animator,bool) {
        auto it = currentChangingAnimations.find(child);
        currentChangingAnimations.erase(it);
        delete &animator;
        if (hasListeners()) {
            std::vector<TransitionListener> listeners = mListeners;
            for (TransitionListener& listener:listeners) {
                listener.endTransition(*this, parent, child, changeReason == APPEARING ?
                                CHANGE_APPEARING : changeReason == DISAPPEARING ?
                                CHANGE_DISAPPEARING : CHANGING);
            }
        }
    };
    anim->addListener(al);
    child->addOnLayoutChangeListener(listener);
    layoutChangeListenerMap.insert({child,listener});
}

void LayoutTransition::startChangingAnimations(){
    std::unordered_map<View*,Animator*>currentAnimCopy = currentChangingAnimations;
    for (auto ita : currentAnimCopy) {
        Animator*anim = ita.second;
        if(dynamic_cast<ObjectAnimator*>(anim)){
            ((ObjectAnimator*)anim)->setCurrentPlayTime(0);
        }
        anim->start();
    }
}

void LayoutTransition::endChangingAnimations(){
    std::unordered_map<View*,Animator*>currentAnimCopy = currentChangingAnimations;
    for (auto ita : currentAnimCopy) {
        Animator*anim = ita.second;
        anim->start();
        anim->cancel();
    }
    currentChangingAnimations.clear();
}

bool LayoutTransition::isChangingLayout() const{
    return (currentChangingAnimations.size() > 0);
}

void LayoutTransition::layoutChange(ViewGroup *parent){
    if (parent->getWindowVisibility() != View::VISIBLE){
        return;
    }

    if (((mTransitionTypes & FLAG_CHANGING) == FLAG_CHANGING) && !isRunning()) {
        // This method is called for all calls to layout() in the container, including
        // those caused by add/remove/hide/show events, which will already have set up
        // transition animations. Avoid setting up CHANGING animations in this case; only
        // do so when there is not a transition already running on the container.
        runChangeTransition(parent, nullptr, CHANGING);
    }
}

bool LayoutTransition::isRunning()const {
    return (currentChangingAnimations.size() > 0) || (currentAppearingAnimations.size() > 0) ||
            (currentDisappearingAnimations.size() > 0);
}

void LayoutTransition::cancel(){
    std::unordered_map<View*,Animator*>currentAnimCopy = currentChangingAnimations;
    for (auto it:currentAnimCopy) {
        it.second->cancel();
    }
    currentChangingAnimations.clear();
   
    currentAnimCopy = currentAppearingAnimations;
    for(auto it:currentAnimCopy)
        it.second->end();
    currentAppearingAnimations.clear();

    currentAnimCopy = currentDisappearingAnimations;
    for(auto it:currentAnimCopy)
        it.second->end();
    currentDisappearingAnimations.clear();
}

void LayoutTransition::cancel(int transitionType){
    std::unordered_map<View*,Animator*>currentAnimCopy;
    switch (transitionType) {
    case CHANGE_APPEARING:
    case CHANGE_DISAPPEARING:
    case CHANGING:
        currentAnimCopy = currentChangingAnimations;
        for (auto it : currentAnimCopy)
            it.second->cancel();
        currentChangingAnimations.clear();
        break;
    case APPEARING:
        currentAnimCopy = currentAppearingAnimations;
        for (auto it:currentAnimCopy)
            it.second->end();
        currentAppearingAnimations.clear();
        break;
    case DISAPPEARING:
        currentAnimCopy = currentDisappearingAnimations;
        for (auto it :currentAnimCopy)
           it.second->end();
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
        std::vector<TransitionListener> listeners = mListeners;
        for (auto& l:listeners)
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
    lis.onAnimationEnd = [this,parent,child](Animator& anim,bool reverse){
        auto it = currentAppearingAnimations.find(child);
        if(it!=currentAppearingAnimations.end()){
            currentAppearingAnimations.erase(it);
            delete it->second;
        }
        std::vector<TransitionListener>listeners = mListeners;
        for (auto& l: listeners)
            if(l.endTransition)l.endTransition(*this, parent, child, APPEARING);
    };
    anim->addListener(lis);

    currentAppearingAnimations.insert({child,anim});
    anim->start();
}

void LayoutTransition::runDisappearingTransition(ViewGroup* parent,View* child){
    auto it=currentAppearingAnimations.find(child);
    if (it!=currentAppearingAnimations.end()) {
        it->second->cancel();
    }
    if (mDisappearingAnim == nullptr) {
        std::vector<TransitionListener>listeners = mListeners;
        for (TransitionListener& l :listeners)
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
    const float preAnimAlpha = child->getAlpha();
    AnimatorListenerAdapter lis;
    lis.onAnimationEnd = [this,parent,child,preAnimAlpha](Animator& anim,bool reverse){
        auto it = currentDisappearingAnimations.find(child);
        if(it!=currentDisappearingAnimations.end()){
            currentDisappearingAnimations.erase(it);
            delete it->second;
        }
        child->setAlpha(preAnimAlpha);
        std::vector<TransitionListener>listeners = mListeners;
        for (TransitionListener& l:listeners)
            if(l.endTransition)l.endTransition(*this, parent, child, DISAPPEARING);
    };
    anim->addListener(lis);

    if (dynamic_cast<ObjectAnimator*>(anim)) {
        ((ObjectAnimator*) anim)->setCurrentPlayTime(0);
    }
    currentDisappearingAnimations.insert({child,anim});
    anim->start();
}

void LayoutTransition::addChild(ViewGroup* parent, View* child, bool changesLayout){
    if ((mTransitionTypes & FLAG_APPEARING) == FLAG_APPEARING) {
        // Want disappearing animations to finish up before proceeding
        cancel(DISAPPEARING);
    }
    if (changesLayout && ((mTransitionTypes & FLAG_CHANGE_APPEARING) == FLAG_CHANGE_APPEARING)) {
        // Also, cancel changing animations so that we start fresh ones from current locations
        cancel(CHANGE_APPEARING);
        cancel(CHANGING);
    }
    if (hasListeners() && ((mTransitionTypes & FLAG_APPEARING) == FLAG_APPEARING)) {
        for (auto& l:mListeners) {
            if(l.startTransition)l.startTransition(*this, parent, child, APPEARING);
        }
    }
    if (changesLayout && ((mTransitionTypes & FLAG_CHANGE_APPEARING) == FLAG_CHANGE_APPEARING)) {
        runChangeTransition(parent, child, APPEARING);
    }
    if ((mTransitionTypes & FLAG_APPEARING) == FLAG_APPEARING) {
        runAppearingTransition(parent, child);
    }
}

void LayoutTransition::removeChild(ViewGroup* parent, View* child, bool changesLayout){
    if ((parent->getWindowVisibility() != View::VISIBLE)||(child==nullptr)) return;
   
    if ((mTransitionTypes & FLAG_DISAPPEARING) == FLAG_DISAPPEARING) {
        // Want appearing animations to finish up before proceeding
        cancel(APPEARING);
    }
    if (changesLayout && ((mTransitionTypes & FLAG_CHANGE_DISAPPEARING) == FLAG_CHANGE_DISAPPEARING)) {
        // Also, cancel changing animations so that we start fresh ones from current locations
        cancel(CHANGE_DISAPPEARING);
        cancel(CHANGING);
    }
    if (hasListeners() && ((mTransitionTypes & FLAG_DISAPPEARING) == FLAG_DISAPPEARING)) {
        for (auto& l :mListeners) {
            if(l.startTransition)l.startTransition(*this, parent, child, DISAPPEARING);
        }
    }
    if (changesLayout && ((mTransitionTypes & FLAG_CHANGE_DISAPPEARING) == FLAG_CHANGE_DISAPPEARING)) {
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

void LayoutTransition::addTransitionListener(const TransitionListener& listener){
    mListeners.push_back(listener);
}

void LayoutTransition::removeTransitionListener(const TransitionListener& listener){
    for(auto it = mListeners.begin();it!=mListeners.end();it++){
        if((it->startTransition==listener.startTransition) && (it->endTransition==listener.endTransition)){
            mListeners.erase(it);
            break;
        }
    }
}

}//endof namespace
