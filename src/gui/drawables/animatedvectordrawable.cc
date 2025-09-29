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
#include <core/systemclock.h>
#include <porting/cdlog.h>
#include <animation/animatorinflater.h>
#include <drawables/animatedvectordrawable.h>
#ifdef ENABLE_VECTOR_RENDER_THREAD
#include <drawables/propertyvaluesanimatorset.h>
#endif
namespace cdroid{

AnimatedVectorDrawable::AnimatedVectorDrawable()
    :AnimatedVectorDrawable(nullptr){
}

class MyCallback:public Drawable::Callback{
private:
    AnimatedVectorDrawable*avd;
public:
    MyCallback(AnimatedVectorDrawable*a):avd(a){}
    void invalidateDrawable(Drawable& who)override{
        avd->invalidateSelf();
    }
    void scheduleDrawable(Drawable& who, Runnable& what, int64_t when)override{
        avd->scheduleSelf(what, when);
    }
    void unscheduleDrawable(Drawable& who, Runnable& what)override{
        avd->unscheduleSelf(what);
    }
};

AnimatedVectorDrawable::AnimatedVectorDrawable(std::shared_ptr<AnimatedVectorDrawableState> state){
    mMutated = false;
    mAnimatorSetFromXml = nullptr;
    mAnimatedVectorState = std::make_shared<AnimatedVectorDrawableState>(state, mCallback);
    mAnimatorSet = new VectorDrawableAnimatorUI(this);
    //mAnimatorSet = new VectorDrawableAnimatorRT(this);
    mCallback = new MyCallback(this);
}

AnimatedVectorDrawable::~AnimatedVectorDrawable(){
    delete mCallback;
    delete mAnimatorSet;
    delete mAnimatorSetFromXml;
}

AnimatedVectorDrawable* AnimatedVectorDrawable::mutate() {
    if (!mMutated && Drawable::mutate() == this) {
        mAnimatedVectorState = std::make_shared<AnimatedVectorDrawableState>(mAnimatedVectorState, mCallback);
        mMutated = true;
    }
    return this;
}

void AnimatedVectorDrawable::clearMutated() {
    Drawable::clearMutated();
    if (mAnimatedVectorState->mVectorDrawable != nullptr) {
        mAnimatedVectorState->mVectorDrawable->clearMutated();
    }
    mMutated = false;
}

bool AnimatedVectorDrawable::shouldIgnoreInvalidAnimation() {
    /*Application app = ActivityThread.currentApplication();
    if (app == null || app.getApplicationInfo() == null) {
        return true;
    }
    if (app.getApplicationInfo().targetSdkVersion < Build::VERSION_CODES::N) {
        return true;
    }*/
    return false;
}

std::shared_ptr<Drawable::ConstantState> AnimatedVectorDrawable::getConstantState() {
    mAnimatedVectorState->mChangingConfigurations = getChangingConfigurations();
    return mAnimatedVectorState;
}

int AnimatedVectorDrawable::getChangingConfigurations() const{
    return Drawable::getChangingConfigurations() | mAnimatedVectorState->getChangingConfigurations();
}

void AnimatedVectorDrawable::draw(Canvas& canvas) {
#ifdef ENABLE_VECTOR_RENDER_THREAD
    if (/*!canvas.isHardwareAccelerated() &&*/ dynamic_cast<VectorDrawableAnimatorRT*>(mAnimatorSet)) {
        // If we have SW canvas and the RT animation is waiting to start, We need to fallback
        // to UI thread animation for AVD.
        if (!mAnimatorSet->isRunning() &&
                ((VectorDrawableAnimatorRT*) mAnimatorSet)->mPendingAnimationActions.size() > 0) {
            fallbackOntoUI();
        }
    }
#endif
    mAnimatorSet->onDraw(canvas);
    mAnimatedVectorState->mVectorDrawable->draw(canvas);
}

void AnimatedVectorDrawable::onBoundsChange(const Rect& bounds) {
    mAnimatedVectorState->mVectorDrawable->setBounds(bounds);
}

bool AnimatedVectorDrawable::onStateChange(const std::vector<int>& state) {
    return mAnimatedVectorState->mVectorDrawable->setState(state);
}

bool AnimatedVectorDrawable::onLevelChange(int level) {
    return mAnimatedVectorState->mVectorDrawable->setLevel(level);
}

bool AnimatedVectorDrawable::onLayoutDirectionChanged(int layoutDirection) {
    return mAnimatedVectorState->mVectorDrawable->setLayoutDirection(layoutDirection);
}

int AnimatedVectorDrawable::getAlpha() const{
    return mAnimatedVectorState->mVectorDrawable->getAlpha();
}

void AnimatedVectorDrawable::setAlpha(int alpha) {
    mAnimatedVectorState->mVectorDrawable->setAlpha(alpha);
}

void AnimatedVectorDrawable::setColorFilter(ColorFilter* colorFilter) {
    mAnimatedVectorState->mVectorDrawable->setColorFilter(colorFilter);
}

ColorFilter* AnimatedVectorDrawable::getColorFilter() {
    return mAnimatedVectorState->mVectorDrawable->getColorFilter();
}

void AnimatedVectorDrawable::setTintList(const ColorStateList* tint) {
    mAnimatedVectorState->mVectorDrawable->setTintList(tint);
}

void AnimatedVectorDrawable::setHotspot(float x, float y) {
    mAnimatedVectorState->mVectorDrawable->setHotspot(x, y);
}

void AnimatedVectorDrawable::setHotspotBounds(int left, int top, int right, int bottom) {
    mAnimatedVectorState->mVectorDrawable->setHotspotBounds(left, top, right, bottom);
}

void AnimatedVectorDrawable::setTintMode(int tintMode) {
    mAnimatedVectorState->mVectorDrawable->setTintMode(tintMode);
}

bool AnimatedVectorDrawable::setVisible(bool visible, bool restart) {
    if (mAnimatorSet->isInfinite() && mAnimatorSet->isStarted()) {
        if (visible) {
            // Resume the infinite animation when the drawable becomes visible again.
            mAnimatorSet->resume();
        } else {
            // Pause the infinite animation once the drawable is no longer visible.
            mAnimatorSet->pause();
        }
    }
    mAnimatedVectorState->mVectorDrawable->setVisible(visible, restart);
    return Drawable::setVisible(visible, restart);
}

bool AnimatedVectorDrawable::isStateful() const{
    return mAnimatedVectorState->mVectorDrawable->isStateful();
}

int AnimatedVectorDrawable::getOpacity() {
    return PixelFormat::TRANSLUCENT;
}

int AnimatedVectorDrawable::getIntrinsicWidth() {
    return mAnimatedVectorState->mVectorDrawable->getIntrinsicWidth();
}

int AnimatedVectorDrawable::getIntrinsicHeight() {
    return mAnimatedVectorState->mVectorDrawable->getIntrinsicHeight();
}

void AnimatedVectorDrawable::getOutline(Outline& outline) {
    mAnimatedVectorState->mVectorDrawable->getOutline(outline);
}

Insets AnimatedVectorDrawable::getOpticalInsets() {
    return mAnimatedVectorState->mVectorDrawable->getOpticalInsets();
}

void AnimatedVectorDrawable::inflate(XmlPullParser&parser,const AttributeSet&attrs){
    auto state = mAnimatedVectorState;
    int eventType= parser.getEventType();//XmlPullParser::START_TAG;
    float pathErrorScale = 1;
    const int innerDepth = parser.getDepth()+1;
    Context*ctx = attrs.getContext();
    state->mContext = ctx;
    // Parse everything until the end of the animated-vector element.
    while ( (parser.getDepth() >= innerDepth || eventType != XmlPullParser::END_TAG)) {
        if (eventType == XmlPullParser::START_TAG) {
            const std::string tagName = parser.getName();
            if (tagName.compare(ANIMATED_VECTOR)==0) {
                std::string drawableRes = attrs.getString("drawable");
                LOGV("drawable=%s",drawableRes.c_str());
                if (!drawableRes.empty()) {
                    VectorDrawable* vectorDrawable = (VectorDrawable*) ctx->getDrawable(drawableRes)->mutate();
                    vectorDrawable->setAllowCaching(false);
                    vectorDrawable->setCallback(mCallback);
                    pathErrorScale = vectorDrawable->getPixelSize();
                    if (state->mVectorDrawable != nullptr) {
                        state->mVectorDrawable->setCallback(nullptr);
                    }
                    state->mVectorDrawable = vectorDrawable;
                }
            } else if (tagName.compare(TARGET)==0) {
                const std::string target = attrs.getString("name");
                const std::string animResId = attrs.getString("animation");
                if (!animResId.empty()) {
                    if (true/*theme != nullptr*/) {
                        // The animator here could be ObjectAnimator or AnimatorSet.
                        Animator* animator = AnimatorInflater::loadAnimator(ctx, animResId, pathErrorScale);
                        updateAnimatorProperty(animator, target, state->mVectorDrawable,state->mShouldIgnoreInvalidAnim);
                        state->addTargetAnimator(target, animator);
                        LOGV("%s -> %s %p",target.c_str(),animResId.c_str(),animator);
                    } else {
                        // The animation may be theme-dependent. As a
                        // workaround until Animator has full support for
                        // applyTheme(), postpone loading the animator
                        // until we have a theme in applyTheme().
                        state->addPendingAnimator(animResId, pathErrorScale, target);
                    }
                }
            }
        }
        eventType =parser.next();
    }
    // If we don't have any pending animations, we don't need to hold a
    // reference to the resources.
}

void AnimatedVectorDrawable::updateAnimatorProperty(Animator* animator, const std::string& targetName,VectorDrawable* vectorDrawable, bool ignoreInvalidAnim) {
    if (dynamic_cast<ObjectAnimator*>(animator)) {
        // Change the property of the Animator from using reflection based on the property
        // name to a Property object that wraps the setter and getter for modifying that
        // specific property for a given object. By replacing the reflection with a direct call,
        // we can largely reduce the time it takes for a animator to modify a VD property.
        std::vector<PropertyValuesHolder*> holders = ((ObjectAnimator*) animator)->getValues();
        for (int i = 0; i < holders.size(); i++) {
            PropertyValuesHolder* pvh = holders[i];
            const std::string propertyName = pvh->getPropertyName();
            void* targetNameObj = vectorDrawable->getTargetByName(targetName);
            const Property* property = nullptr;
            /*if (dynamic_cast<VectorDrawable::VObject*>(targetNameObj)) {
                property = ((VectorDrawable::VObject*) targetNameObj)->getProperty(propertyName);
            }*/if (targetNameObj==vectorDrawable->getConstantState().get()){
                //dynamic_cast<VectorDrawable::VectorDrawableState*>(targetNameObj)) {
                property = ((VectorDrawable::VectorDrawableState*) targetNameObj)->getProperty(propertyName);
            }else {//if (dynamic_cast<VectorDrawable::VObject*>(targetNameObj)){
                property = ((VectorDrawable::VObject*) targetNameObj)->getProperty(propertyName);
            }
            if (property != nullptr) {
                LOGV("pvh=%p %s.%s",pvh,targetName.c_str(),propertyName.c_str());
                if (containsSameValueType(pvh, property)) {
                    pvh->setProperty((Property*)property);
                } else if (!ignoreInvalidAnim) {
                    LOGE("Wrong valueType for Property:%s .  Expected type: %d . Actual type defined in resources:%d",
                            propertyName.c_str(),property->getType(),pvh->getValueType());
                }
            }
        }
    } else if (dynamic_cast<AnimatorSet*>(animator)) {
        for (Animator* anim : ((AnimatorSet*) animator)->getChildAnimations()) {
            updateAnimatorProperty(anim, targetName, vectorDrawable, ignoreInvalidAnim);
        }
    }
}

bool AnimatedVectorDrawable::containsSameValueType(const PropertyValuesHolder* holder,const Property* property) {
    const int type1 = holder->getValueType();
    const int type2 = property->getType();
    if (type1 == Property::FLOAT_TYPE) {
        return type2 == Property::FLOAT_TYPE;
    } else if (type1 == Property::INT_TYPE) {
        return (type2 == Property::INT_TYPE)||(type2==Property::COLOR_TYPE);
    } else {
        return type1 == type2;
    }
}

void AnimatedVectorDrawable::forceAnimationOnUI() {
#ifdef ENABLE_VECTOR_RENDER_THREAD
    if (dynamic_cast<VectorDrawableAnimatorRT*>(mAnimatorSet)) {
        VectorDrawableAnimatorRT* animator = (VectorDrawableAnimatorRT*) mAnimatorSet;
        if (animator->isRunning()) {
            throw std::runtime_error("Cannot force Animated Vector Drawable to"
                    " run on UI thread when the animation has started on RenderThread.");
        }
        fallbackOntoUI();
    }
#endif
}

void AnimatedVectorDrawable::fallbackOntoUI() {
#ifdef ENABLE_VECTOR_RENDER_THREAD
    if (dynamic_cast<VectorDrawableAnimatorRT*>(mAnimatorSet)) {
        VectorDrawableAnimatorRT* oldAnim = (VectorDrawableAnimatorRT*) mAnimatorSet;
        mAnimatorSet = new VectorDrawableAnimatorUI(this);
        if (mAnimatorSetFromXml != nullptr) {
            mAnimatorSet->init(mAnimatorSetFromXml);
        }
        // Transfer the listener from RT animator to UI animator
        if (oldAnim->mListener.onAnimationStart!=nullptr||oldAnim->mListener.onAnimationEnd != nullptr) {
            mAnimatorSet->setListener(oldAnim->mListener);
        }
        oldAnim->transferPendingActions(mAnimatorSet);
    }
#endif
}

bool AnimatedVectorDrawable::canApplyTheme() {
    return (mAnimatedVectorState != nullptr && mAnimatedVectorState->canApplyTheme())
            || Drawable::canApplyTheme();
}

#if 0
void AnimatedVectorDrawable::applyTheme(Theme t) {
    Drawable::applyTheme(t);

    VectorDrawable* vectorDrawable = mAnimatedVectorState->mVectorDrawable;
    if (vectorDrawable != nullptr && vectorDrawable->canApplyTheme()) {
        vectorDrawable->applyTheme(t);
    }

    if (t != null) {
        mAnimatedVectorState->inflatePendingAnimators(t.getResources(), t);
    }

    // If we don't have any pending animations, we don't need to hold a
    // reference to the resources.
    if (mAnimatedVectorState->mPendingAnims.empty()) {
        //mRes = null;
    }
}
#endif

////////////////////////////////////////////////////////////////////////////////////
//static class AnimatedVectorDrawableState:public Drawable::ConstantState
AnimatedVectorDrawable::AnimatedVectorDrawableState::AnimatedVectorDrawableState(std::shared_ptr<AnimatedVectorDrawableState> copy,Callback* owner) {
    mShouldIgnoreInvalidAnim = AnimatedVectorDrawable::shouldIgnoreInvalidAnimation();
    mChangingConfigurations =0;
    mContext = nullptr;

    if (copy != nullptr) {
        mChangingConfigurations = copy->mChangingConfigurations;

        if (copy->mVectorDrawable != nullptr) {
            auto cs = copy->mVectorDrawable->getConstantState();
            /*if (res != nullptr) {
                mVectorDrawable = (VectorDrawable*) cs->newDrawable(res);
            } else*/ {
                mVectorDrawable = (VectorDrawable*) cs->newDrawable();
            }
            mVectorDrawable = (VectorDrawable*) mVectorDrawable->mutate();
            mVectorDrawable->setCallback(owner);
            mVectorDrawable->setLayoutDirection(copy->mVectorDrawable->getLayoutDirection());
            mVectorDrawable->setBounds(copy->mVectorDrawable->getBounds());
            mVectorDrawable->setAllowCaching(false);
        }

        if (!copy->mAnimators.empty()){// != null) {
            mAnimators = copy->mAnimators;//new ArrayList<>(copy->mAnimators);
        }

        if (!copy->mTargetNameMap.empty()){// != null) {
            mTargetNameMap = copy->mTargetNameMap;//new ArrayMap<>(copy->mTargetNameMap);
        }

        if (copy->mPendingAnims.empty()){// != null) {
            mPendingAnims = copy->mPendingAnims;//new ArrayList<>(copy->mPendingAnims);
        }
    } else {
        mVectorDrawable = new VectorDrawable();
    }
}

AnimatedVectorDrawable::AnimatedVectorDrawableState::~AnimatedVectorDrawableState(){
    delete  mVectorDrawable;
}

bool AnimatedVectorDrawable::AnimatedVectorDrawableState::canApplyTheme() {
    return (mVectorDrawable != nullptr && mVectorDrawable->canApplyTheme())
            || mPendingAnims.size() ;//|| Drawable::canApplyTheme();
}

Drawable* AnimatedVectorDrawable::AnimatedVectorDrawableState::newDrawable() {
    return new AnimatedVectorDrawable(shared_from_this());//, nullptr);
}

int AnimatedVectorDrawable::AnimatedVectorDrawableState::getChangingConfigurations() const{
    return mChangingConfigurations;
}

void AnimatedVectorDrawable::AnimatedVectorDrawableState::addPendingAnimator(const std::string& resId, float pathErrorScale, const std::string& target) {
    /*if (mPendingAnims == null) {
        mPendingAnims = new ArrayList<>(1);
    }*/
    mPendingAnims.push_back(new PendingAnimator(resId, pathErrorScale, target));
}

void AnimatedVectorDrawable::AnimatedVectorDrawableState::addTargetAnimator(const std::string& targetName, Animator* animator) {
    /*if (mAnimators == null) {
        mAnimators = new ArrayList<>(1);
        mTargetNameMap = new ArrayMap<>(1);
    }*/
    mAnimators.push_back(animator);
    mTargetNameMap.emplace(animator, targetName);

    LOGV_IF(DBG_ANIMATION_VECTOR_DRAWABLE,"add animator %p for target ",animator,targetName.c_str());
}

/**
 * Prepares a local set of mutable animators based on the constant
 * state.
 * <p>
 * If there are any pending uninflated animators, attempts to inflate
 * them immediately against the provided resources object.
 *
 * @param animatorSet the animator set to which the animators should
 *                    be added
 * @param res the resources against which to inflate any pending
 *            animators, or {@code null} if not available
 */
void AnimatedVectorDrawable::AnimatedVectorDrawableState::prepareLocalAnimators(AnimatorSet* animatorSet) {
    // Check for uninflated animators. We can remove this after we add
    // support for Animator.applyTheme(). See comments in inflate().
    if (!mPendingAnims.empty()){// != nullptr) {
        // Attempt to load animators without applying a theme.
        if (true/*res != null*/) {
            inflatePendingAnimators(/*res, null*/);
        } else {
            LOGE("Failed to load animators. Either the AnimatedVectorDrawable must be created using "
                "a Resources object or applyTheme() must be called with a non-null Theme object.");
        }

        mPendingAnims.clear();// = null;
    }

    // Perform a deep copy of the constant state's animators.
    const size_t count = mAnimators.size();
    if (count > 0) {
        Animator* firstAnim = prepareLocalAnimator(0);
        AnimatorSet::Builder* builder = animatorSet->play(firstAnim);
        for (size_t i = 1; i < count; ++i) {
            Animator* nextAnim = prepareLocalAnimator(i);
            builder->with(nextAnim);
        }
        delete builder;
    }
}

/**
 * Prepares a local animator for the given index within the constant
 * state's list of animators.
 *
 * @param index the index of the animator within the constant state
 */
Animator* AnimatedVectorDrawable::AnimatedVectorDrawableState::prepareLocalAnimator(int index) {
    Animator* animator = mAnimators.at(index);
    Animator* localAnimator = animator->clone();
    auto it = mTargetNameMap.find(animator);
    std::string targetName = it->second;
    void* target = mVectorDrawable->getTargetByName(targetName);
    if (!mShouldIgnoreInvalidAnim) {
        if (target == nullptr) {
            LOGE("Target with the name %s cannot be found in the VectorDrawable to be animated.",targetName.c_str());
        } else if ((target!= mVectorDrawable->getConstantState().get())&&false){
            /*((dynamic_cast<VectorDrawable::VectorDrawableState*>(target))
              && !(dynamic_cast<VectorDrawable::VObject*>(target)))*/
            LOGE("Target should be either VGroup, VPath or ConstantState, is not supported");
        }
    }
    localAnimator->setTarget(target);
    return localAnimator;
}

/**
 * Inflates pending animators, if any, against a theme. Clears the list of
 * pending animators.
 *
 * @param t the theme against which to inflate the animators
 */
void AnimatedVectorDrawable::AnimatedVectorDrawableState::inflatePendingAnimators(/*Resources res,Theme t*/) {
    std::vector<PendingAnimator*> pendingAnims = mPendingAnims;
    if (!pendingAnims.empty()){// != null) {
        mPendingAnims.clear();

        for (int i = 0, count = pendingAnims.size(); i < count; i++) {
            PendingAnimator* pendingAnimator = pendingAnims.at(i);
            Animator* animator = pendingAnimator->newInstance(mContext);
            updateAnimatorProperty(animator, pendingAnimator->target, mVectorDrawable,mShouldIgnoreInvalidAnim);
            addTargetAnimator(pendingAnimator->target, animator);
        }
    }
};

/**
 * Basically a constant state for Animators until we actually implement
 * constant states for Animators.
 */
//static class AnimatedVectorDrawable::AnimatedVectorDrawableState::PendingAnimator;
AnimatedVectorDrawable::AnimatedVectorDrawableState::PendingAnimator::PendingAnimator(const std::string& animResId, float pathErrorScale, const std::string& target) {
    this->animResId = animResId;
    this->pathErrorScale = pathErrorScale;
    this->target = target;
}

Animator* AnimatedVectorDrawable::AnimatedVectorDrawableState::PendingAnimator::newInstance(Context*ctx) {
    return AnimatorInflater::loadAnimator(ctx,animResId, pathErrorScale);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool AnimatedVectorDrawable::isRunning() {
    return mAnimatorSet->isRunning();
}
void AnimatedVectorDrawable::reset() {
    ensureAnimatorSet();
    /*if (DBG_ANIMATION_VECTOR_DRAWABLE) {
        LOGW("calling reset on AVD: " +
                ((VectorDrawable.VectorDrawableState) ((AnimatedVectorDrawableState)
                getConstantState()).mVectorDrawable.getConstantState()).mRootName
                + ", at: " + this);
    }*/
    mAnimatorSet->reset();
}

void AnimatedVectorDrawable::start() {
    ensureAnimatorSet();
    if (DBG_ANIMATION_VECTOR_DRAWABLE) {
        auto vds = mAnimatedVectorState->mVectorDrawable->getConstantState();
        LOGD("calling start on AVD: %s at: %p",((VectorDrawable::VectorDrawableState*) vds.get())->mRootName.c_str(), this);
    }
    mAnimatorSet->start();
}

void AnimatedVectorDrawable::ensureAnimatorSet() {
    if (mAnimatorSetFromXml == nullptr) {
        // TODO: Skip the AnimatorSet creation and init the VectorDrawableAnimator directly
        // with a list of LocalAnimators.
        mAnimatorSetFromXml = new AnimatorSet();
        mAnimatedVectorState->prepareLocalAnimators(mAnimatorSetFromXml/*,mRes*/);
        mAnimatorSet->init(mAnimatorSetFromXml);
        //mRes = nullptr;
    }
}

void AnimatedVectorDrawable::stop() {
    /*if (DBG_ANIMATION_VECTOR_DRAWABLE) {
        LOGW(LOGTAG, "calling stop on AVD: %p at:%p"
                ((VectorDrawableState*) ((AnimatedVectorDrawableState*)
                        getConstantState())->mVectorDrawable->getConstantState())
                        ->mRootName.c_str(), this);
    }*/
    mAnimatorSet->end();
}

void AnimatedVectorDrawable::reverse() {
    ensureAnimatorSet();

    // Only reverse when all the animators can be reversed.
    if (!canReverse()) {
        LOGW("AnimatedVectorDrawable can't reverse()");
        return;
    }

    mAnimatorSet->reverse();
}

bool AnimatedVectorDrawable::canReverse() {
    return mAnimatorSet->canReverse();
}

void AnimatedVectorDrawable::registerAnimationCallback(const Animatable2::AnimationCallback& callback) {
    if ((callback.onAnimationStart == nullptr)&&(callback.onAnimationEnd==nullptr)) {
        return;
    }

    // Add listener accordingly.
    /*if (mAnimationCallbacks == null) {
        mAnimationCallbacks = new ArrayList<>();
    }*/

    mAnimationCallbacks.push_back(callback);

    mAnimatorListener.onAnimationStart =[this](Animator& animation,bool reverse){
        auto tmpCallbacks = mAnimationCallbacks;
        int size = tmpCallbacks.size();
        for (int i = 0; i < size; i ++) {
            tmpCallbacks.at(i).onAnimationStart(*this);
        }
    };

    mAnimatorListener.onAnimationEnd=[this](Animator& animation,bool reverse) {
        auto tmpCallbacks =mAnimationCallbacks;
        int size = tmpCallbacks.size();
        for (int i = 0; i < size; i ++) {
            tmpCallbacks.at(i).onAnimationEnd(*this);
        }
    };
    mAnimatorSet->setListener(mAnimatorListener);
}

// A helper function to clean up the animator listener in the mAnimatorSet.
void AnimatedVectorDrawable::removeAnimatorSetListener() {
    if (mAnimatorListener.onAnimationStart||mAnimatorListener.onAnimationEnd) {
        mAnimatorSet->removeListener(mAnimatorListener);
        mAnimatorListener = {};
    }
}

bool AnimatedVectorDrawable::unregisterAnimationCallback(const Animatable2::AnimationCallback& callback) {
    if (mAnimationCallbacks.empty() ||((callback.onAnimationStart == nullptr)&&(callback.onAnimationEnd==nullptr))) {
        // Nothing to be removed.
        return false;
    }
    auto it =std::find(mAnimationCallbacks.begin(),mAnimationCallbacks.end(),callback);
    bool removed = it!=mAnimationCallbacks.end();
    if(removed)mAnimationCallbacks.erase(it);

    //  When the last call back unregistered, remove the listener accordingly.
    if (mAnimationCallbacks.size() == 0) {
        removeAnimatorSetListener();
    }
    return removed;
}

void AnimatedVectorDrawable::clearAnimationCallbacks() {
    removeAnimatorSetListener();
    if (mAnimationCallbacks.empty()) {
        return;
    }

    mAnimationCallbacks.clear();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//class VectorDrawableAnimatorUI:public VectorDrawableAnimator
// mSet is only initialized in init(). So we need to check whether it is null before any operation.
AnimatedVectorDrawable::VectorDrawableAnimatorUI::VectorDrawableAnimatorUI(AnimatedVectorDrawable* drawable) {
    mDrawable = drawable;
    mSet = nullptr;
    LOGD("VectorDrawableAnimatorUI %p on vector %p",this,mDrawable);
}

AnimatedVectorDrawable::VectorDrawableAnimatorUI::~VectorDrawableAnimatorUI(){
    delete mSet;
}

void AnimatedVectorDrawable::VectorDrawableAnimatorUI::init(AnimatorSet* set) {
    if (mSet != nullptr) {
        // Already initialized
        throw std::logic_error("VectorDrawableAnimator cannot be re-initialized");
    }
    // Keep a deep copy of the set, such that set can be still be constantly representing
    // the static content from XML file.
    mSet = set->clone();
    mIsInfinite = mSet->getTotalDuration() == Animator::DURATION_INFINITE;

    // If there are listeners added before calling init(), now they should be setup.
    if (/*mListenerArray != null && */!mListenerArray.empty()) {
        for (int i = 0; i < mListenerArray.size(); i++) {
            mSet->addListener(mListenerArray.at(i));
        }
        mListenerArray.clear();
    }
}

// Although start(), reset() and reverse() should call init() already, it is better to
// protect these functions from NPE in any situation.
void AnimatedVectorDrawable::VectorDrawableAnimatorUI::start() {
    if ((mSet == nullptr) || mSet->isStarted()) {
        return;
    }
    mSet->start();
    invalidateOwningView();
}

void AnimatedVectorDrawable::VectorDrawableAnimatorUI::end() {
    if (mSet == nullptr) {
        return;
    }
    mSet->end();
}

void AnimatedVectorDrawable::VectorDrawableAnimatorUI::reset() {
    if (mSet == nullptr) {
        return;
    }
    start();
    mSet->cancel();
}

void AnimatedVectorDrawable::VectorDrawableAnimatorUI::reverse() {
    if (mSet == nullptr) {
        return;
    }
    mSet->reverse();
    invalidateOwningView();
}

bool AnimatedVectorDrawable::VectorDrawableAnimatorUI::canReverse() {
    return mSet != nullptr && mSet->canReverse();
}

void AnimatedVectorDrawable::VectorDrawableAnimatorUI::setListener(const Animator::AnimatorListener& listener) {
    if (mSet == nullptr) {
        /*if (mListenerArray == null) {
            mListenerArray = new ArrayList<AnimatorListener>();
        }*/
        mListenerArray.push_back(listener);
    } else {
        mSet->addListener(listener);
    }
}

void AnimatedVectorDrawable::VectorDrawableAnimatorUI::removeListener(const Animator::AnimatorListener& listener) {
    if (mSet == nullptr) {
        if (mListenerArray.empty()){// == null) {
            return;
        }
        auto it =std::find(mListenerArray.begin(),mListenerArray.end(),listener);
        mListenerArray.erase(it);
    } else {
        mSet->removeListener(listener);
    }
}

void AnimatedVectorDrawable::VectorDrawableAnimatorUI::onDraw(Canvas& canvas) {
    if ((mSet != nullptr) && mSet->isStarted()) {
        invalidateOwningView();
    }
}

bool AnimatedVectorDrawable::VectorDrawableAnimatorUI::isStarted() {
    return (mSet != nullptr) && mSet->isStarted();
}

bool AnimatedVectorDrawable::VectorDrawableAnimatorUI::isRunning() {
    return (mSet != nullptr) && mSet->isRunning();
}

bool AnimatedVectorDrawable::VectorDrawableAnimatorUI::isInfinite() {
    return mIsInfinite;
}

void AnimatedVectorDrawable::VectorDrawableAnimatorUI::pause() {
    if (mSet == nullptr) {
        return;
    }
    mSet->pause();
}

void AnimatedVectorDrawable::VectorDrawableAnimatorUI::resume() {
    if (mSet == nullptr) {
        return;
    }
    mSet->resume();
}

void AnimatedVectorDrawable::VectorDrawableAnimatorUI::invalidateOwningView() {
    mDrawable->invalidateSelf();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
///class VectorDrawableAnimatorRT implements VectorDrawableAnimator
#ifdef ENABLE_VECTOR_RENDER_THREAD
AnimatedVectorDrawable::VectorDrawableAnimatorRT::VectorDrawableAnimatorRT(AnimatedVectorDrawable* drawable) {
    mDrawable = drawable;
    mSetPtr = new hwui::PropertyValuesAnimatorSet();//nCreateAnimatorSet();//PropertyValuesAnimatorSet();
    // Increment ref count on native AnimatorSet, so it doesn't get released before Java
    // side is done using it.
    mSetRefBasePtr = (hwui::Tree*)mSetPtr;
}

AnimatedVectorDrawable::VectorDrawableAnimatorRT::~VectorDrawableAnimatorRT(){
    delete mSetPtr;
}

void AnimatedVectorDrawable::VectorDrawableAnimatorRT::init(AnimatorSet* set) {
    if (mInitialized) {
        // Already initialized
        throw std::runtime_error("VectorDrawableAnimator cannot be re-initialized");
    }
    parseAnimatorSet(set, 0);
    hwui::Tree*vectorDrawableTreePtr = (hwui::Tree*)mDrawable->mAnimatedVectorState->mVectorDrawable->getNativeTree();//hwui::Tree
    //TODO nSetVectorDrawableTarget(mSetPtr, vectorDrawableTreePtr);
    mInitialized = true;
    mIsInfinite = set->getTotalDuration() == Animator::DURATION_INFINITE;

    // Check reversible.
    mIsReversible = true;
    if (mContainsSequentialAnimators) {
        mIsReversible = false;
    } else {
        // Check if there's any start delay set on child
        for (int i = 0; i < mStartDelays.size(); i++) {
            if (mStartDelays.at(i) > 0) {
                mIsReversible = false;
                return;
            }
        }
    }
}

void AnimatedVectorDrawable::VectorDrawableAnimatorRT::parseAnimatorSet(AnimatorSet* set, int64_t startTime) {
    auto animators = set->getChildAnimations();

    const bool playTogether = set->shouldPlayTogether();
    // Convert AnimatorSet to VectorDrawableAnimatorRT
    for (int i = 0; i < animators.size(); i++) {
        Animator* animator = animators.at(i);
        // Here we only support ObjectAnimator
        if (dynamic_cast<AnimatorSet*>(animator)) {
            parseAnimatorSet((AnimatorSet*) animator, startTime);
        } else if (dynamic_cast<ObjectAnimator*>(animator)) {
            createRTAnimator((ObjectAnimator*) animator, startTime);
        } // ignore ValueAnimators and others because they don't directly modify VD
          // therefore will be useless to AVD.

        if (!playTogether) {
            // Assume not play together means play sequentially
            startTime += animator->getTotalDuration();
            mContainsSequentialAnimators = true;
        }
    }
}

// TODO: This method reads animation data from already parsed Animators. We need to move
// this step further up the chain in the parser to avoid the detour.
void AnimatedVectorDrawable::VectorDrawableAnimatorRT::createRTAnimator(ObjectAnimator* animator, int64_t startTime) {
    std::vector<PropertyValuesHolder*> values = animator->getValues();
    void* target = animator->getTarget();
    VectorDrawable::VObject*targetObj=(VectorDrawable::VObject*)target;
    if (target ==mDrawable->getConstantState().get()){
        createRTAnimatorForRootGroup(values, animator,(VectorDrawable::VectorDrawableState*) target, startTime);
    }else if(dynamic_cast<VectorDrawable::VGroup*>(targetObj)){//VGroup
        createRTAnimatorForGroup(values, animator, (VectorDrawable::VGroup*) target,startTime);
    }else if(dynamic_cast<VectorDrawable::VPath*>(targetObj)){
        for (int i = 0; i < values.size(); i++) {
            values[i]->getPropertyValues(mTmpValues);
            if (/*dynamic_cast<PathParser::PathData*>(mTmpValues.endValue) &&*/ mTmpValues.propertyName.compare("pathData")==0) {
                createRTAnimatorForPath(animator, (VectorDrawable::VPath*) target,startTime);
            }  else if (dynamic_cast<VectorDrawable::VFullPath*>(targetObj)){
                createRTAnimatorForFullPath(animator, (VectorDrawable::VFullPath*) target,startTime);
            } else if (!mDrawable->mAnimatedVectorState->mShouldIgnoreInvalidAnim) {
                throw std::logic_error("ClipPath only supports PathData property");
            }
        }
    }
}

void AnimatedVectorDrawable::VectorDrawableAnimatorRT::createRTAnimatorForGroup(const std::vector<PropertyValuesHolder*>& values,
        ObjectAnimator* animator, VectorDrawable::VGroup* target,int64_t startTime) {

    hwui::Group* nativePtr = (hwui::Group*)target->getNativePtr();//hwui::Group
    int propertyId;
    for (int i = 0; i < values.size(); i++) {
        // TODO: We need to support the rare case in AVD where no start value is provided
        values[i]->getPropertyValues(mTmpValues);
        propertyId = VectorDrawable::VGroup::getPropertyIndex(mTmpValues.propertyName);
        if (mTmpValues.type != Property::FLOAT_CLASS) {
            LOGE_IF(DBG_ANIMATION_VECTOR_DRAWABLE,"Unsupported type: %d. Only float value is supported for Groups.",mTmpValues.type);
            continue;
        }
        if (propertyId < 0) {
            LOGE_IF(DBG_ANIMATION_VECTOR_DRAWABLE,"Unsupported property: for Vector Drawable Group",mTmpValues.propertyName.c_str());
            continue;
        }
        //long propertyPtr = nCreateGroupPropertyHolder(nativePtr, propertyId,mTmpValues.startValue, mTmpValues.endValue);
        Property*prop = target->getProperty(mTmpValues.propertyName);
        auto propertyPtr = PropertyValuesHolder::ofFloat(prop,{GET_VARIANT(mTmpValues.startValue,float),GET_VARIANT(mTmpValues.endValue,float)});
        if (mTmpValues.dataSource != nullptr) {
            const std::vector<float>dataPoints = createFloatDataPoints(mTmpValues.dataSource,animator->getDuration());
            //nSetPropertyHolderData(propertyPtr, dataPoints, dataPoints.size());
            propertyPtr->setValues(dataPoints);
        }
        createNativeChildAnimator(propertyPtr, startTime, animator);
    }
}

void AnimatedVectorDrawable::VectorDrawableAnimatorRT::createRTAnimatorForPath( ObjectAnimator* animator, VectorDrawable::VPath* target,int64_t startTime) {

    hwui::Path* nativePtr = (hwui::Path*)target->getNativePtr();//hwui::Path
    std::shared_ptr<PathParser::PathData>startPathData = GET_VARIANT(mTmpValues.startValue,std::shared_ptr<PathParser::PathData>);
    std::shared_ptr<PathParser::PathData>endPathData = GET_VARIANT(mTmpValues.endValue,std::shared_ptr<PathParser::PathData>);
    hwui::PathData* startPathDataPtr = (hwui::PathData*)startPathData->getNativePtr();
    hwui::PathData* endPathDataPtr = (hwui::PathData*)endPathData->getNativePtr();
    //long propertyPtr = nCreatePathDataPropertyHolder(nativePtr, startPathDataPtr,endPathDataPtr);/*PathMorph*/
    PropertyValuesHolder *propertyPtr=nullptr;
    createNativeChildAnimator(propertyPtr, startTime, animator);
}

void AnimatedVectorDrawable::VectorDrawableAnimatorRT::createRTAnimatorForFullPath(ObjectAnimator* animator,VectorDrawable::VFullPath* target, int64_t startTime) {

    int propertyId = target->getPropertyIndex(mTmpValues.propertyName);
    Property*prop = target->getProperty(mTmpValues.propertyName);
    PropertyValuesHolder* propertyPtr = nullptr;
    hwui::FullPath* nativePtr = (hwui::FullPath*)target->getNativePtr();//hwui::FullPath
    if (mTmpValues.type == Property::FLOAT_CLASS) {
        if (propertyId < 0) {
            if (mDrawable->mAnimatedVectorState->mShouldIgnoreInvalidAnim) {
                return;
            } else {
                LOGE("Property: %s  is not supported for FullPath",mTmpValues.propertyName.c_str());
            }
        }
        //propertyPtr = nCreatePathPropertyHolder(nativePtr, propertyId,mTmpValues.startValue, mTmpValues.endValue);
        propertyPtr = PropertyValuesHolder::ofFloat(prop,{GET_VARIANT(mTmpValues.startValue,float),GET_VARIANT(mTmpValues.endValue,float)});
        if (mTmpValues.dataSource != nullptr) {
            // Pass keyframe data to native, if any.
            const std::vector<float> dataPoints = createFloatDataPoints(mTmpValues.dataSource,animator->getDuration());
            //nSetPropertyHolderData(propertyPtr, dataPoints, dataPoints.size());
            propertyPtr->setValues(dataPoints);
        }

    } else if (mTmpValues.type == Property::INT_CLASS) {
        //propertyPtr = nCreatePathColorPropertyHolder(nativePtr, propertyId,mTmpValues.startValue, mTmpValues.endValue);
        propertyPtr = PropertyValuesHolder::ofInt(prop,{(int)GET_VARIANT(mTmpValues.startValue,int32_t),GET_VARIANT(mTmpValues.endValue,int32_t)});
        if (mTmpValues.dataSource != nullptr) {
            // Pass keyframe data to native, if any.
            const std::vector<int> dataPoints = createIntDataPoints(mTmpValues.dataSource,animator->getDuration());
            //nSetPropertyHolderData(propertyPtr, dataPoints, dataPoints.size());
            propertyPtr->setValues(dataPoints);
        }
    } else {
        if (mDrawable->mAnimatedVectorState->mShouldIgnoreInvalidAnim) {
            return;
        } else {
            LOGE("Unsupported type:  Only float, int or PathData value is supported for Paths.");
        }
    }
    createNativeChildAnimator(propertyPtr, startTime, animator);
}

void AnimatedVectorDrawable::VectorDrawableAnimatorRT::createRTAnimatorForRootGroup(const std::vector<PropertyValuesHolder*>& values,
        ObjectAnimator* animator, VectorDrawable::VectorDrawableState* target,int64_t startTime) {
    long nativePtr = target->getNativeRenderer();
    if (!animator->getPropertyName().compare("alpha")==0) {
        if (mDrawable->mAnimatedVectorState->mShouldIgnoreInvalidAnim) {
            return;
        } else {
            throw std::logic_error("Only alpha is supported for root group");
        }
    }
    float startValue = INFINITY;//null;
    float endValue = INFINITY;//null;
    for (int i = 0; i < values.size(); i++) {
        values[i]->getPropertyValues(mTmpValues);
        if (mTmpValues.propertyName.compare("alpha")==0) {
            startValue = GET_VARIANT(mTmpValues.startValue,float);
            endValue = GET_VARIANT(mTmpValues.endValue,float);
            break;
        }
    }
    if (startValue == INFINITY && endValue == INFINITY) {
        if (mDrawable->mAnimatedVectorState->mShouldIgnoreInvalidAnim) {
            return;
        } else {
            throw std::logic_error("No alpha values are specified");
        }
    }
    //long propertyPtr = nCreateRootAlphaPropertyHolder(nativePtr, startValue, endValue);
    PropertyValuesHolder*propertyPtr = PropertyValuesHolder::ofFloat(nullptr,{startValue,endValue});
    if (mTmpValues.dataSource != nullptr) {
        // Pass keyframe data to native, if any.
        std::vector<float> dataPoints = createFloatDataPoints(mTmpValues.dataSource,animator->getDuration());
        //nSetPropertyHolderData(propertyPtr, dataPoints, dataPoints.size());
    }
    createNativeChildAnimator(propertyPtr, startTime, animator);
}

/**
 * Calculate the amount of frames an animation will run based on duration.
 */
int AnimatedVectorDrawable::VectorDrawableAnimatorRT::getFrameCount(int64_t duration) {
    int64_t frameIntervalNanos = Choreographer::getInstance().getFrameIntervalNanos();
    int animIntervalMs = (int) (frameIntervalNanos / SystemClock::NANOS_PER_MS);
    int numAnimFrames = (int) std::ceil(((double) duration) / animIntervalMs);
    // We need 2 frames of data minimum.
    numAnimFrames = std::max(2, numAnimFrames);
    if (numAnimFrames > MAX_SAMPLE_POINTS) {
        LOGW("AnimatedVectorDrawable", "Duration for the animation is too long :%d"
                ", the animation will subsample the keyframe or path data.",duration);
        numAnimFrames = MAX_SAMPLE_POINTS;
    }
    return numAnimFrames;
}

// These are the data points that define the value of the animating properties.
// e.g. translateX and translateY can animate along a Path, at any fraction in [0, 1]
// a point on the path corresponds to the values of translateX and translateY.
// TODO: (Optimization) We should pass the path down in native and chop it into segments
// in native.
std::vector<float> AnimatedVectorDrawable::VectorDrawableAnimatorRT::createFloatDataPoints(
        PropertyValuesHolder::PropertyValues::DataSource dataSource, int64_t duration) {
    int numAnimFrames = getFrameCount(duration);
    std::vector<float> values(numAnimFrames);
    float lastFrame = numAnimFrames - 1;
    for (int i = 0; i < numAnimFrames; i++) {
        float fraction = i / lastFrame;
        values[i] = GET_VARIANT(dataSource(fraction),float);
    }
    return values;
}

std::vector<int> AnimatedVectorDrawable::VectorDrawableAnimatorRT::createIntDataPoints(
        PropertyValuesHolder::PropertyValues::DataSource dataSource, int64_t duration) {
    int numAnimFrames = getFrameCount(duration);
    std::vector<int>values(numAnimFrames);
    float lastFrame = numAnimFrames - 1;
    for (int i = 0; i < numAnimFrames; i++) {
        float fraction = i / lastFrame;
        values[i] = GET_VARIANT(dataSource(fraction),int);
    }
    return values;
}

void AnimatedVectorDrawable::VectorDrawableAnimatorRT::createNativeChildAnimator(PropertyValuesHolder* holder, int64_t extraDelay,ObjectAnimator* animator) {
    int64_t duration = animator->getDuration();
    const int repeatCount = animator->getRepeatCount();
    int64_t startDelay = extraDelay + animator->getStartDelay();
    auto interpolator = animator->getInterpolator();

    startDelay *= ValueAnimator::getDurationScale();
    duration *= ValueAnimator::getDurationScale();

    mStartDelays.push_back(startDelay);
    //nAddAnimator(mSetPtr, propertyPtr, nativeInterpolator, startDelay, duration,repeatCount, animator->getRepeatMode());
    mSetPtr->addPropertyAnimator(holder, interpolator, startDelay, duration, repeatCount, animator->getRepeatMode());
}

/*void AnimatedVectorDrawable::VectorDrawableAnimatorRT::recordLastSeenTarget(DisplayListCanvas canvas) {
    RenderNode* node = RenderNodeAnimatorSetHelper.getTarget(canvas);
    mLastSeenTarget = new WeakReference<RenderNode>(node);
    // Add the animator to the list of animators on every draw
    if (mInitialized || mPendingAnimationActions.size() > 0) {
        if (useTarget(node)) {
            if (DBG_ANIMATION_VECTOR_DRAWABLE) {
                LOGD("Target is set in the next frame");
            }
            for (int i = 0; i < mPendingAnimationActions.size(); i++) {
                handlePendingAction(mPendingAnimationActions.at(i));
            }
            mPendingAnimationActions.clear();
        }
    }
}*/

void AnimatedVectorDrawable::VectorDrawableAnimatorRT::handlePendingAction(int pendingAnimationAction) {
    if (pendingAnimationAction == START_ANIMATION) {
        startAnimation();
    } else if (pendingAnimationAction == REVERSE_ANIMATION) {
        reverseAnimation();
    } else if (pendingAnimationAction == RESET_ANIMATION) {
        resetAnimation();
    } else if (pendingAnimationAction == END_ANIMATION) {
        endAnimation();
    } else {
        FATAL("Animation action %d is not supported",pendingAnimationAction);
    }
}

bool AnimatedVectorDrawable::VectorDrawableAnimatorRT::useLastSeenTarget() {
    /*if (mLastSeenTarget != nullptr) {
        RenderNode* target = mLastSeenTarget;//.get();
        return useTarget(target);
    }*/
    return false;
}

/*bool AnimatedVectorDrawable::VectorDrawableAnimatorRT::useTarget(RenderNode* target) {
    if (target != null && target.isAttached()) {
        target.registerVectorDrawableAnimator(this);
        return true;
    }
    return false;
}*/

void AnimatedVectorDrawable::VectorDrawableAnimatorRT::invalidateOwningView() {
    mDrawable->invalidateSelf();
}

void AnimatedVectorDrawable::VectorDrawableAnimatorRT::addPendingAction(int pendingAnimationAction) {
    invalidateOwningView();
    mPendingAnimationActions.push_back(pendingAnimationAction);
}

void AnimatedVectorDrawable::VectorDrawableAnimatorRT::start() {
    if (!mInitialized) {
        return;
    }
    LOGD("%p",this);

    if (useLastSeenTarget()) {
        if (DBG_ANIMATION_VECTOR_DRAWABLE) {
            LOGD("Target is set. Starting VDAnimatorSet from java");
        }
        startAnimation();
    } else {
        addPendingAction(START_ANIMATION);
    }
}

void AnimatedVectorDrawable::VectorDrawableAnimatorRT::end() {
    if (!mInitialized) {
        return;
    }

    if (useLastSeenTarget()) {
        endAnimation();
    } else {
        addPendingAction(END_ANIMATION);
    }
}

void AnimatedVectorDrawable::VectorDrawableAnimatorRT::reset() {
    if (!mInitialized) {
        return;
    }

    if (useLastSeenTarget()) {
        resetAnimation();
    } else {
        addPendingAction(RESET_ANIMATION);
    }
}

// Current (imperfect) Java AnimatorSet cannot be reversed when the set contains sequential
// animators or when the animator set has a start delay
void AnimatedVectorDrawable::VectorDrawableAnimatorRT::reverse() {
    if (!mIsReversible || !mInitialized) {
        return;
    }
    if (useLastSeenTarget()) {
        if (DBG_ANIMATION_VECTOR_DRAWABLE) {
            LOGD("Target is set. Reversing VDAnimatorSet from java");
        }
        reverseAnimation();
    } else {
        addPendingAction(REVERSE_ANIMATION);
    }
}

// This should only be called after animator has been added to the RenderNode target.
void AnimatedVectorDrawable::VectorDrawableAnimatorRT::startAnimation() {
    /*if (DBG_ANIMATION_VECTOR_DRAWABLE) {
        LOGD("starting animation on VD: %s" ,
                ((VectorDrawable::VectorDrawableState*) ((AnimatedVectorDrawableState*)
                        mDrawable->getConstantState().get())->mVectorDrawable->getConstantState())
                        ->mRootName.c_str());
    }*/
    mStarted = true;
    LOGD("%p",this);
    //mSetPtr->start();//nStart(mSetPtr, this, ++mLastListenerId);
    invalidateOwningView();
    if (mListener.onAnimationStart != nullptr) {
        //mListener.o/::nAnimationStart(null);
    }
}

// This should only be called after animator has been added to the RenderNode target.
void AnimatedVectorDrawable::VectorDrawableAnimatorRT::endAnimation() {
    /*if (DBG_ANIMATION_VECTOR_DRAWABLE) {
        LOGW("ending animation on VD: %%d",
                ((VectorDrawable::VectorDrawableState*) ((AnimatedVectorDrawableState*)
                        mDrawable->getConstantState().get())->mVectorDrawable->getConstantState())
                        ->mRootName.c_str());
    }*/
    LOGD("%p",this);
    mSetPtr->end();//nEnd(mSetPtr);
    invalidateOwningView();
}

// This should only be called after animator has been added to the RenderNode target.
void AnimatedVectorDrawable::VectorDrawableAnimatorRT::resetAnimation() {
    //nReset(mSetPtr);
    LOGD("%p",this);
    mSetPtr->reset();
    invalidateOwningView();
}

// This should only be called after animator has been added to the RenderNode target.
void AnimatedVectorDrawable::VectorDrawableAnimatorRT::reverseAnimation() {
    mStarted = true;
    LOGD("%p",this);
    //mSetPtr->reverse();//nReverse(mSetPtr, this, ++mLastListenerId);
    invalidateOwningView();
    if (mListener.onAnimationStart != nullptr) {
        //mListener.onAnimationStart(nullptr);
    }
}

long AnimatedVectorDrawable::VectorDrawableAnimatorRT::getAnimatorNativePtr() {
    return (long)mSetPtr;
}

bool AnimatedVectorDrawable::VectorDrawableAnimatorRT::canReverse() {
    return mIsReversible;
}

bool AnimatedVectorDrawable::VectorDrawableAnimatorRT::isStarted() {
    return mStarted;
}

bool AnimatedVectorDrawable::VectorDrawableAnimatorRT::isRunning() {
    if (!mInitialized) {
        return false;
    }
    return mStarted;
}

void AnimatedVectorDrawable::VectorDrawableAnimatorRT::setListener(const Animator::AnimatorListener& listener) {
    mListener = listener;
}

void AnimatedVectorDrawable::VectorDrawableAnimatorRT::removeListener(const Animator::AnimatorListener& listener) {
    mListener={};// = null;
}

void AnimatedVectorDrawable::VectorDrawableAnimatorRT::onDraw(Canvas& canvas) {
    /*if (canvas.isHardwareAccelerated()) {
        recordLastSeenTarget((DisplayListCanvas) canvas);
    }*/
}

bool AnimatedVectorDrawable::VectorDrawableAnimatorRT::isInfinite() {
    return mIsInfinite;
}

void AnimatedVectorDrawable::VectorDrawableAnimatorRT::pause() {
    // TODO: Implement pause for Animator On RT.
}

void AnimatedVectorDrawable::VectorDrawableAnimatorRT::resume() {
    // TODO: Implement resume for Animator On RT.
}

void AnimatedVectorDrawable::VectorDrawableAnimatorRT::onAnimationEnd(int listenerId) {
    if (listenerId != mLastListenerId) {
        return;
    }
    if (DBG_ANIMATION_VECTOR_DRAWABLE) {
        LOGD("on finished called from native");
    }
    mStarted = false;
    // Invalidate in the end of the animation to make sure the data in
    // RT thread is synced back to UI thread.
    invalidateOwningView();
    if (mListener.onAnimationEnd != nullptr) {
        //mListener.onAnimationEnd(nullptr);
    }
}

// onFinished: should be called from native
void AnimatedVectorDrawable::VectorDrawableAnimatorRT::callOnFinished(VectorDrawableAnimatorRT* set, int id) {
    set->onAnimationEnd(id);
}

void AnimatedVectorDrawable::VectorDrawableAnimatorRT::transferPendingActions(VectorDrawableAnimator* animatorSet) {
    for (int i = 0; i < mPendingAnimationActions.size(); i++) {
        int pendingAction = mPendingAnimationActions.at(i);
        if (pendingAction == START_ANIMATION) {
            animatorSet->start();
        } else if (pendingAction == END_ANIMATION) {
            animatorSet->end();
        } else if (pendingAction == REVERSE_ANIMATION) {
            animatorSet->reverse();
        } else if (pendingAction == RESET_ANIMATION) {
            animatorSet->reset();
        } else {
            FATAL("Animation action %d is not supported",pendingAction);
        }
    }
    mPendingAnimationActions.clear();
}
#endif
}/*endof namespace*/
