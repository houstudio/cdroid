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

const TimeInterpolator* LayoutTransition::ACCEL_DECEL_INTERPOLATOR         = AccelerateDecelerateInterpolator::Instance;
const TimeInterpolator* LayoutTransition::DECEL_INTERPOLATOR               = DecelerateInterpolator::Instance;
const TimeInterpolator* LayoutTransition::sAppearingInterpolator           = ACCEL_DECEL_INTERPOLATOR;
const TimeInterpolator* LayoutTransition::sDisappearingInterpolator        = ACCEL_DECEL_INTERPOLATOR;
const TimeInterpolator* LayoutTransition::sChangingAppearingInterpolator   = DECEL_INTERPOLATOR;
const TimeInterpolator* LayoutTransition::sChangingDisappearingInterpolator= DECEL_INTERPOLATOR;
const TimeInterpolator* LayoutTransition::sChangingInterpolator            = DECEL_INTERPOLATOR;

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
    // Cancel in-flight animators before teardown (AOSP relies on GC): their
    // end listeners delete the animators and drain the maps, so nothing stays
    // registered with the AnimationHandler against freed targets/`this`.
    if (mCleanupObserver && mCleanupObserver->isAlive() && mPreDrawCleanup) {
        mCleanupObserver->removeOnPreDrawListener(*mPreDrawCleanup);
    }
    if (mCleanupParent && mAttachStateCleanup) {
        mCleanupParent->removeOnAttachStateChangeListener(*mAttachStateCleanup);
    }
    std::vector<Animator*> inFlight;
    for (auto& it : currentChangingAnimations)     inFlight.push_back(it.second);
    for (auto& it : currentAppearingAnimations)    inFlight.push_back(it.second);
    for (auto& it : currentDisappearingAnimations) inFlight.push_back(it.second);
    for (Animator* remover : mPendingAnimRemovers) inFlight.push_back(remover);
    for (Animator* anim : inFlight) anim->cancel();
    // Sweep whatever never started (cancel() no-ops on those).
    for (auto& it : pendingAnimations)             delete it.second;
    for (auto& it : currentChangingAnimations)     delete it.second;
    for (auto& it : currentAppearingAnimations)    delete it.second;
    for (auto& it : currentDisappearingAnimations) delete it.second;
    pendingAnimations.clear();
    currentChangingAnimations.clear();
    currentAppearingAnimations.clear();
    currentDisappearingAnimations.clear();
    mPendingAnimRemovers.clear();
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

Animator* LayoutTransition::getAnimator(int transitionType) const {
    switch (transitionType) {
    case CHANGE_APPEARING:
        return mChangingAppearingAnim;
    case CHANGE_DISAPPEARING:
        return mChangingDisappearingAnim;
    case CHANGING:
        return mChangingAnim;
    case APPEARING:
        return mAppearingAnim;
    case DISAPPEARING:
        return mDisappearingAnim;
    }
    return nullptr;
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

void LayoutTransition::setInterpolator(int transitionType,const TimeInterpolator* interpolator) {
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

const TimeInterpolator* LayoutTransition::getInterpolator(int transitionType)const{
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
            // AOSP guards with `parentParent instanceof ViewGroup` (java:786-792): at the
            // top of the hierarchy getParent() is null (or a non-ViewGroup), and walking into
            // setupChangeAnimation with a null parent leaves the layout listener capturing it —
            // the next layout pass dereferences it in requestTransitionStart. Missing here,
            // this crashed the sample on the first Add click.
            if (ViewGroup* parentParent = dynamic_cast<ViewGroup*>(tempParent->getParent())) {
                setupChangeAnimation(parentParent, changeReason, parentAnimator, duration, tempParent);
                tempParent = parentParent;
            } else {
                tempParent = nullptr;
            }
        }
    }

    // This is the cleanup step. When we get this rendering event, we know that all of
    // the appropriate animations have been set up and run. Now we can clear out the
    // layout listeners. They capture themselves through shared_ptrs so the callbacks
    // remove the registered copies (a by-value self-capture would hold the empty
    // pre-assignment functor and never match on removal).
    if (!mPreDrawCleanup) {
        auto onPreDrawListener = std::make_shared<ViewTreeObserver::OnPreDrawListener>();
        auto onAttachStateListener = std::make_shared<View::OnAttachStateChangeListener>();
        auto sweep = [this](){
            for (auto it:layoutChangeListenerMap){
                View*view = it.first;
                view->removeOnLayoutChangeListener(it.second);
            }
            layoutChangeListenerMap.clear();
        };
        auto detach = [this,parent,onPreDrawListener,onAttachStateListener](){
            parent->getViewTreeObserver()->removeOnPreDrawListener(*onPreDrawListener);
            parent->removeOnAttachStateChangeListener(*onAttachStateListener);
            mPreDrawCleanup.reset();
            mAttachStateCleanup.reset();
            mCleanupParent = nullptr;
            mCleanupObserver = nullptr;
        };
        *onPreDrawListener = [sweep,detach](){
            detach();
            sweep();
            return true;
        };
        onAttachStateListener->onViewAttachedToWindow = [](View& v){};
        onAttachStateListener->onViewDetachedFromWindow=[sweep,detach](View& v){
            detach();
            sweep();
        };
        mCleanupParent = parent;
        mCleanupObserver = observer;
        mPreDrawCleanup = onPreDrawListener;
        mAttachStateCleanup = onAttachStateListener;
        observer->addOnPreDrawListener(*onPreDrawListener);
        parent->addOnAttachStateChangeListener(*onAttachStateListener);
    }
}

void LayoutTransition::setAnimateParentHierarchy(bool animateParentHierarchy) {
    mAnimateParentHierarchy = animateParentHierarchy;
}

// AOSP skips change animations whose every holder has equal start/end
// keyframe values; missing keyframes count as differing (path-based sets).
static bool keyframeEndpointsDiffer(PropertyValuesHolder* pvh){
    std::vector<Keyframe*>& kfs = pvh->getKeyframes()->getKeyframes();
    if (kfs.size() < 2) return true;
    const Keyframe* first = kfs.front();
    const Keyframe* last  = kfs.back();
    if (!first->hasValue() || !last->hasValue()) return true;
    const AnimateValue va = first->getValue();
    const AnimateValue vb = last->getValue();
    if (va.index() != vb.index()) return true;
    switch (va.index()) {
    case 0: return GET_VARIANT(va,int)   != GET_VARIANT(vb,int);
    case 1: return GET_VARIANT(va,float) != GET_VARIANT(vb,float);
    default: return true;
    }
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
    mPendingAnimRemovers.push_back(pendingAnimRemover);
    Animator::AnimatorListener al;
    al.onAnimationEnd=[this,child](Animator&anim,bool){
        auto itr = std::find(mPendingAnimRemovers.begin(), mPendingAnimRemovers.end(), &anim);
        if (itr != mPendingAnimRemovers.end()) mPendingAnimRemovers.erase(itr);
        auto it = pendingAnimations.find(child);
        delete &anim;
        if(it != pendingAnimations.end()){
            delete it->second;
            pendingAnimations.erase(it);
            // The layout listener still registered on the child captured that
            // now-deleted animator — the pending window is over, so detach it.
            // (AOSP leaves the listener in place; GC makes that harmless there.
            // If the animation was promoted to currentChangingAnimations, the
            // entry is already erased and this block stays out of the way.)
            auto itl = layoutChangeListenerMap.find(child);
            if (itl != layoutChangeListenerMap.end()) {
                child->removeOnLayoutChangeListener(itl->second);
                layoutChangeListenerMap.erase(itl);
            }
        }
    };
    pendingAnimRemover->addListener(al);
    pendingAnimRemover->start();

    View::OnLayoutChangeListener listener;
    listener = [this,anim,parent,child,changeReason,duration](View& v, int left, int top, int width, int height,
                    int oldLeft, int oldTop, int oldWidth, int oldHeight){

        anim->setupEndValues();
        if (dynamic_cast<ValueAnimator*>(anim)) {
            bool valuesDiffer = false;
            ValueAnimator* valueAnim = (ValueAnimator*)anim;
            std::vector<PropertyValuesHolder*> oldValues = valueAnim->getValues();
            for (PropertyValuesHolder* pvh : oldValues) {
                if (keyframeEndpointsDiffer(pvh)) {
                    valuesDiffer = true;
                }
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
        // AOSP removes the pending entry without deleting it: ownership of the
        // promoted animator moves to currentChangingAnimations (deleted on
        // animation end); the pendingAnimRemover no longer finds it.
        it = pendingAnimations.find(child);
        if (it!=pendingAnimations.end()) {
            pendingAnimations.erase(it);
        }
        // Cache the animation in case we need to cancel it later
        currentChangingAnimations.insert({child,anim});//put(child, anim);

        parent->requestTransitionStart(this);

        // this only removes listeners whose views changed - must clear the
        // other listeners later
        auto itl = layoutChangeListenerMap.find(child);
        if (itl != layoutChangeListenerMap.end()) {
            child->removeOnLayoutChangeListener(itl->second);
            layoutChangeListenerMap.erase(itl);
        }
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

    al.onAnimationCancel = [this,child](Animator& animator) {
        auto it = layoutChangeListenerMap.find(child);
        if (it != layoutChangeListenerMap.end()) {
            child->removeOnLayoutChangeListener(it->second);
            layoutChangeListenerMap.erase(it);
        }
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
            delete it->second;
            currentAppearingAnimations.erase(it);
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
            delete it->second;
            currentDisappearingAnimations.erase(it);
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
    auto it = std::find(mListeners.begin(),mListeners.end(),listener);
    if(it==mListeners.end()){
        mListeners.push_back(listener);
    }
}

void LayoutTransition::removeTransitionListener(const TransitionListener& listener){
    auto it = std::find(mListeners.begin(),mListeners.end(),listener);
    if(it!=mListeners.end()){
        mListeners.erase(it);
    }
}

}//endof namespace
