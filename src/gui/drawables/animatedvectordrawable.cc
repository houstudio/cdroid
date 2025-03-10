#if 0
#include <core/systemclock.h>
#include <drawables/animatedvectordrawable.h>
namespace cdroid{

AnimatedVectorDrawable::AnimatedVectorDrawable()
    :AnimatedVectorDrawable(nullptr){
}

AnimatedVectorDrawable::AnimatedVectorDrawable(std::shared_ptr<AnimatedVectorDrawableState> state){
    mAnimatedVectorState = std::make_shared<AnimatedVectorDrawableState>(state.get(), mCallback);
    mAnimatorSet = new VectorDrawableAnimatorRT(this);
    //mRes = res;
    /*private final Callback mCallback = new Callback() {
        public void invalidateDrawable(Drawable who) {
            invalidateSelf();
        }

        public void scheduleDrawable(Drawable who, Runnable what, long when) {
            scheduleSelf(what, when);
        }

        public void unscheduleDrawable(Drawable who, Runnable what) {
            unscheduleSelf(what);
        }
    };*/
}

Drawable* AnimatedVectorDrawable::mutate() {
    if (!mMutated && Drawable::mutate() == this) {
        mAnimatedVectorState = std::make_shared<AnimatedVectorDrawableState>(mAnimatedVectorState.get(), mCallback);
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
    if (!canvas.isHardwareAccelerated() && dynamic_cast<VectorDrawableAnimatorRT*>(mAnimatorSet)) {
        // If we have SW canvas and the RT animation is waiting to start, We need to fallback
        // to UI thread animation for AVD.
        if (!mAnimatorSet->isRunning() &&
                ((VectorDrawableAnimatorRT*) mAnimatorSet)->mPendingAnimationActions.size() > 0) {
            fallbackOntoUI();
        }
    }
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

void AnimatedVectorDrawable::inflate(Context*ctx,const std::string&resid){
    auto state = mAnimatedVectorState;
#if 0
    int eventType = parser.getEventType();
    float pathErrorScale = 1;
    const int innerDepth = parser.getDepth();

    // Parse everything until the end of the animated-vector element.
    while (eventType != XmlPullParser.END_DOCUMENT
            && (parser.getDepth() >= innerDepth || eventType != XmlPullParser.END_TAG)) {
        if (eventType == XmlPullParser.START_TAG) {
            std::string tagName = parser.getName();
            if (ANIMATED_VECTOR.equals(tagName)) {
                 TypedArray a = obtainAttributes(res, theme, attrs,
                        R.styleable.AnimatedVectorDrawable);
                int drawableRes = a.getResourceId(
                        R.styleable.AnimatedVectorDrawable_drawable, 0);
                if (drawableRes != 0) {
                    VectorDrawable* vectorDrawable = (VectorDrawable) res.getDrawable(
                            drawableRes, theme).mutate();
                    vectorDrawable.setAllowCaching(false);
                    vectorDrawable.setCallback(mCallback);
                    pathErrorScale = vectorDrawable.getPixelSize();
                    if (state.mVectorDrawable != null) {
                        state.mVectorDrawable.setCallback(null);
                    }
                    state.mVectorDrawable = vectorDrawable;
                }
                a.recycle();
            } else if (TARGET.equals(tagName)) {
                TypedArray a = obtainAttributes(res, theme, attrs,
                        R.styleable.AnimatedVectorDrawableTarget);
                std::string target = a.getString(
                        R.styleable.AnimatedVectorDrawableTarget_name);
                int animResId = a.getResourceId(
                        R.styleable.AnimatedVectorDrawableTarget_animation, 0);
                if (animResId != 0) {
                    if (theme != null) {
                        // The animator here could be ObjectAnimator or AnimatorSet.
                        Animator* animator = AnimatorInflater::loadAnimator(res, theme, animResId, pathErrorScale);
                        updateAnimatorProperty(animator, target, state.mVectorDrawable,
                                state.mShouldIgnoreInvalidAnim);
                        state.addTargetAnimator(target, animator);
                    } else {
                        // The animation may be theme-dependent. As a
                        // workaround until Animator has full support for
                        // applyTheme(), postpone loading the animator
                        // until we have a theme in applyTheme().
                        state.addPendingAnimator(animResId, pathErrorScale, target);

                    }
                }
                a.recycle();
            }
        }

        eventType = parser.next();
    }
#endif
    // If we don't have any pending animations, we don't need to hold a
    // reference to the resources.
    //mRes = state.mPendingAnims == null ? null : res;
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
            std::string propertyName = pvh->getPropertyName();
            Object* targetNameObj = (Object*)vectorDrawable->getTargetByName(targetName);
            Property* property = nullptr;
            if (dynamic_cast<VectorDrawable::VObject*>(targetNameObj)) {
                property = ((VectorDrawable::VObject*) targetNameObj)->getProperty(propertyName);
            } else if (dynamic_cast<VectorDrawable::VectorDrawableState*>(targetNameObj)) {
                property = ((VectorDrawable::VectorDrawableState*) targetNameObj)->getProperty(propertyName);
            }
            if (property != nullptr) {
                if (containsSameValueType(pvh, property)) {
                    pvh->setProperty(property);
                } else if (!ignoreInvalidAnim) {
                    /*throw new RuntimeException("Wrong valueType for Property: " + propertyName
                            + ".  Expected type: " + property.getType().toString() + ". Actual "
                            + "type defined in resources: " + pvh.getValueType().toString());*/

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
    Class type1 = holder.getValueType();
    Class type2 = property.getType();
    if (type1 == float.class || type1 == Float.class) {
        return type2 == float.class || type2 == Float.class;
    } else if (type1 == int.class || type1 == Integer.class) {
        return type2 == int.class || type2 == Integer.class;
    } else {
        return type1 == type2;
    }
}

void AnimatedVectorDrawable::forceAnimationOnUI() {
    if (mAnimatorSet instanceof VectorDrawableAnimatorRT) {
        VectorDrawableAnimatorRT* animator = (VectorDrawableAnimatorRT*) mAnimatorSet;
        if (animator->isRunning()) {
            throw std::runtime_error("Cannot force Animated Vector Drawable to"
                    " run on UI thread when the animation has started on RenderThread.");
        }
        fallbackOntoUI();
    }
}

void AnimatedVectorDrawable::fallbackOntoUI() {
    if (dynamic_cast<VectorDrawableAnimatorRT*>(mAnimatorSet)) {
        VectorDrawableAnimatorRT* oldAnim = (VectorDrawableAnimatorRT*) mAnimatorSet;
        mAnimatorSet = new VectorDrawableAnimatorUI(this);
        if (mAnimatorSetFromXml != nullptr) {
            mAnimatorSet->init(mAnimatorSetFromXml);
        }
        // Transfer the listener from RT animator to UI animator
        if (oldAnim.mListener != nullptr) {
            mAnimatorSet->setListener(oldAnim->mListener);
        }
        oldAnim->transferPendingActions(mAnimatorSet);
    }
}

bool AnimatedVectorDrawable::canApplyTheme() {
    return (mAnimatedVectorState != nullptr && mAnimatedVectorState->canApplyTheme())
            || Drawable::canApplyTheme();
}

void AnimatedVectorDrawable::applyTheme(Theme t) {
#if 0
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
#endif
}

////////////////////////////////////////////////////////////////////////////////////
//static class AnimatedVectorDrawableState:public Drawable::ConstantState
AnimatedVectorDrawable::AnimatedVectorDrawableState::AnimatedVectorDrawableState(const AnimatedVectorDrawableState* copy,Callback* owner) {
    mShouldIgnoreInvalidAnim = shouldIgnoreInvalidAnimation();
    if (copy != nullptr) {
        mChangingConfigurations = copy->mChangingConfigurations;

        if (copy->mVectorDrawable != nullptr) {
            auto cs = copy->mVectorDrawable->getConstantState();
            if (res != nullptr) {
                mVectorDrawable = (VectorDrawable*) cs->newDrawable(res);
            } else {
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

bool AnimatedVectorDrawable::AnimatedVectorDrawableState::canApplyTheme() {
    return (mVectorDrawable != nullptr && mVectorDrawable->canApplyTheme())
            || mPendingAnims != null || Drawable::canApplyTheme();
}

Drawable* AnimatedVectorDrawable::AnimatedVectorDrawableState::newDrawable() {
    return new AnimatedVectorDrawable(this, nullptr);
}

int AnimatedVectorDrawable::AnimatedVectorDrawableState::getChangingConfigurations() const{
    return mChangingConfigurations;
}

void AnimatedVectorDrawable::AnimatedVectorDrawableState::addPendingAnimator(int resId, float pathErrorScale, const std::string& target) {
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

    if (DBG_ANIMATION_VECTOR_DRAWABLE) {
        LOGV("add animator %p for target ",animator,targetName.c_str());
    }
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
        if (res != null) {
            inflatePendingAnimators(res, null);
        } else {
            LOGE("Failed to load animators. Either the AnimatedVectorDrawable"
                " must be created using a Resources object or applyTheme() must be"
                " called with a non-null Theme object.");
        }

        mPendingAnims.clear();// = null;
    }

    // Perform a deep copy of the constant state's animators.
    const int count = /*mAnimators == null ? 0 : */mAnimators.size();
    if (count > 0) {
        const Animator firstAnim = prepareLocalAnimator(0);
        const AnimatorSet.Builder builder = animatorSet.play(firstAnim);
        for (int i = 1; i < count; ++i) {
            Animator* nextAnim = prepareLocalAnimator(i);
            builder.with(nextAnim);
        }
    }
}

/**
 * Prepares a local animator for the given index within the constant
 * state's list of animators.
 *
 * @param index the index of the animator within the constant state
 */
Animator* AnimatedVectorDrawable::AnimatedVectorDrawableState::prepareLocalAnimator(int index) {
    Animator* animator = mAnimators.get(index);
    Animator* localAnimator = animator.clone();
    std::string targetName = mTargetNameMap.get(animator);
    Object* target = mVectorDrawable->getTargetByName(targetName);
    if (!mShouldIgnoreInvalidAnim) {
        if (target == null) {
            throw new IllegalStateException("Target with the name \"" + targetName
                    + "\" cannot be found in the VectorDrawable to be animated.");
        } else if (!(target instanceof VectorDrawable.VectorDrawableState)
                && !(target instanceof VectorDrawable.VObject)) {
            throw new UnsupportedOperationException("Target should be either VGroup, VPath,"
                    + " or ConstantState, " + target.getClass() + " is not supported");
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
void AnimatedVectorDrawable::AnimatedVectorDrawableState::inflatePendingAnimators(Resources res,Theme t) {
    std::vector<PendingAnimator*> pendingAnims = mPendingAnims;
    if (!pendingAnims.empty()){// != null) {
        mPendingAnims.clear();

        for (int i = 0, count = pendingAnims.size(); i < count; i++) {
            PendingAnimator* pendingAnimator = pendingAnims.at(i);
            Animator* animator = pendingAnimator->newInstance(res, t);
            updateAnimatorProperty(animator, pendingAnimator->target, mVectorDrawable,
                    mShouldIgnoreInvalidAnim);
            addTargetAnimator(pendingAnimator->target, animator);
        }
    }
};

/**
 * Basically a constant state for Animators until we actually implement
 * constant states for Animators.
 */
//static class AnimatedVectorDrawable::AnimatedVectorDrawableState::PendingAnimator;
AnimatedVectorDrawable::AnimatedVectorDrawableState::PendingAnimator::PendingAnimator(const std::String& animResId, float pathErrorScale, const std::string& target) {
    this.animResId = animResId;
    this.pathErrorScale = pathErrorScale;
    this.target = target;
}

Animator AnimatedVectorDrawable::AnimatedVectorDrawableState::PendingAnimator::newInstance(Theme theme) {
    return AnimatorInflater::loadAnimator(nullptr,animResId, pathErrorScale);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool AnimatedVectorDrawable::isRunning() {
    return mAnimatorSet.isRunning();
}
void AnimatedVectorDrawable::reset() {
    ensureAnimatorSet();
    /*if (DBG_ANIMATION_VECTOR_DRAWABLE) {
        LOGW("calling reset on AVD: " +
                ((VectorDrawable.VectorDrawableState) ((AnimatedVectorDrawableState)
                getConstantState()).mVectorDrawable.getConstantState()).mRootName
                + ", at: " + this);
    }*/
    mAnimatorSet.reset();
}

void AnimatedVectorDrawable::start() {
    ensureAnimatorSet();
    /*if (DBG_ANIMATION_VECTOR_DRAWABLE) {
        LOGD("calling start on AVD: %s at: %p"
                ((VectorDrawableState*) ((AnimatedVectorDrawableState*)getConstantState())->mVectorDrawable->getConstantState())->mRootName.c_str(), this);
    }*/
    mAnimatorSet->start();
}

void AnimatedVectorDrawable::ensureAnimatorSet() {
    if (mAnimatorSetFromXml == null) {
        // TODO: Skip the AnimatorSet creation and init the VectorDrawableAnimator directly
        // with a list of LocalAnimators.
        mAnimatorSetFromXml = new AnimatorSet();
        mAnimatedVectorState.prepareLocalAnimators(mAnimatorSetFromXml, mRes);
        mAnimatorSet.init(mAnimatorSetFromXml);
        mRes = null;
    }
}

void AnimatedVectorDrawable::stop() {
    /*if (DBG_ANIMATION_VECTOR_DRAWABLE) {
        LOGW(LOGTAG, "calling stop on AVD: %p at:%p"
                ((VectorDrawableState*) ((AnimatedVectorDrawableState*)
                        getConstantState())->mVectorDrawable->getConstantState())
                        ->mRootName.c_str(), this);
    }*/
    mAnimatorSet.end();
}

void AnimatedVectorDrawable::reverse() {
    ensureAnimatorSet();

    // Only reverse when all the animators can be reversed.
    if (!canReverse()) {
        LOGW("AnimatedVectorDrawable can't reverse()");
        return;
    }

    mAnimatorSet.reverse();
}

bool AnimatedVectorDrawable::canReverse() {
    return mAnimatorSet.canReverse();
}

void AnimatedVectorDrawable::registerAnimationCallback(AnimationCallback callback) {
    if (callback == null) {
        return;
    }

    // Add listener accordingly.
    /*if (mAnimationCallbacks == null) {
        mAnimationCallbacks = new ArrayList<>();
    }*/

    mAnimationCallbacks.add(callback);

    mAnimatorListener.onAnimationStart =[this](Animator& animation){
        auto tmpCallbacks = mAnimationCallbacks;
        int size = tmpCallbacks.size();
        for (int i = 0; i < size; i ++) {
            tmpCallbacks.at(i)->onAnimationStart(*this);
        }
    };

    mAnimatorListener.onAnimationEnd=[this](Animator& animation) {
        auto tmpCallbacks =mAnimationCallbacks;
        int size = tmpCallbacks.size();
        for (int i = 0; i < size; i ++) {
            tmpCallbacks.at(i)->onAnimationEnd(*this);
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

bool AnimatedVectorDrawable::unregisterAnimationCallback(AnimationCallback callback) {
    if (mAnimationCallbacks == null || callback == null) {
        // Nothing to be removed.
        return false;
    }
    bool removed = mAnimationCallbacks.remove(callback);

    //  When the last call back unregistered, remove the listener accordingly.
    if (mAnimationCallbacks.size() == 0) {
        removeAnimatorSetListener();
    }
    return removed;
}

void AnimatedVectorDrawable::clearAnimationCallbacks() {
    removeAnimatorSetListener();
    if (mAnimationCallbacks == null) {
        return;
    }

    mAnimationCallbacks.clear();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//private static class VectorDrawableAnimatorUI:public VectorDrawableAnimator {
// mSet is only initialized in init(). So we need to check whether it is null before any
// operation.
AnimatedVectorDrawable::VectorDrawableAnimatorUI::VectorDrawableAnimatorUI(AnimatedVectorDrawable* drawable) {
    mDrawable = drawable;
}

void AnimatedVectorDrawable::VectorDrawableAnimatorUI::init(AnimatorSet* set) {
    if (mSet != nullptr) {
        // Already initialized
        throw std:::logic_error("VectorDrawableAnimator cannot be re-initialized");
    }
    // Keep a deep copy of the set, such that set can be still be constantly representing
    // the static content from XML file.
    mSet = set->clone();
    mIsInfinite = mSet->getTotalDuration() == Animator::DURATION_INFINITE;

    // If there are listeners added before calling init(), now they should be setup.
    if (/*mListenerArray != null && */!mListenerArray.empty()) {
        for (int i = 0; i < mListenerArray.size(); i++) {
            mSet.addListener(mListenerArray.at(i));
        }
        mListenerArray.clear();
        //mListenerArray = null;
    }
}

// Although start(), reset() and reverse() should call init() already, it is better to
// protect these functions from NPE in any situation.
void AnimatedVectorDrawable::VectorDrawableAnimatorUI::start() {
    if (mSet == nullptr || mSet->isStarted()) {
        return;
    }
    mSet->start();
    invalidateOwningView();
}

void AnimatedVectorDrawable::VectorDrawableAnimatorUI::end() {
    if (mSet == nullptr) {
        return;
    }
    mSet.end();
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
        mSet.addListener(listener);
    }
}

void AnimatedVectorDrawable::VectorDrawableAnimatorUI::removeListener(const Animator::AnimatorListener& listener) {
    if (mSet == nullptr) {
        if (mListenerArray.empty()){// == null) {
            return;
        }
        mListenerArray.remove(listener);
    } else {
        mSet.removeListener(listener);
    }
}

void AnimatedVectorDrawable::VectorDrawableAnimatorUI::onDraw(Canvas& canvas) {
    if (mSet != nullptr && mSet->isStarted()) {
        invalidateOwningView();
    }
}

bool AnimatedVectorDrawable::VectorDrawableAnimatorUI::isStarted() {
    return mSet != nullptr && mSet->isStarted();
}

bool AnimatedVectorDrawable::VectorDrawableAnimatorUI::isRunning() {
    return mSet != nullptr && mSet->isRunning();
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

///////////////////////////////////////////////////////////////////////////////
///    public static class VectorDrawableAnimatorRT implements VectorDrawableAnimator {

AnimatedVectorDrawable::VectorDrawableAnimatorRT::VectorDrawableAnimatorRT(AnimatedVectorDrawable* drawable) {
    mDrawable = drawable;
    mSetPtr = nCreateAnimatorSet();
    // Increment ref count on native AnimatorSet, so it doesn't get released before Java
    // side is done using it.
    mSetRefBasePtr = new VirtualRefBasePtr(mSetPtr);
}

void AnimatedVectorDrawable::VectorDrawableAnimatorRT::init(AnimatorSet* set) {
    if (mInitialized) {
        // Already initialized
        throw std::runtime_error("VectorDrawableAnimator cannot be re-initialized");
    }
    parseAnimatorSet(set, 0);
    long vectorDrawableTreePtr = mDrawable->mAnimatedVectorState->mVectorDrawable.getNativeTree();
    nSetVectorDrawableTarget(mSetPtr, vectorDrawableTreePtr);
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

void AnimatedVectorDrawable::VectorDrawableAnimatorRT::parseAnimatorSet(AnimatorSet* set, long startTime) {
    auto animators = set->getChildAnimations();

    bool playTogether = set->shouldPlayTogether();
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
void AnimatedVectorDrawable::VectorDrawableAnimatorRT::createRTAnimator(ObjectAnimator* animator, long startTime) {
    std::vector<PropertyValuesHolder*> values = animator->getValues();
    Object* target = animator->getTarget();
    if (dynamic_cast<VectorDrawable::VGroup*>(target)) {
        createRTAnimatorForGroup(values, animator, (VectorDrawable::VGroup*) target,startTime);
    } else if (dynamic_cast<VectorDrawable::VPath*>(target)) {
        for (int i = 0; i < values.length; i++) {
            values[i].getPropertyValues(mTmpValues);
            if (mTmpValues.endValue instanceof PathParser::PathData &&
                    mTmpValues.propertyName.compare("pathData")==0) {
                createRTAnimatorForPath(animator, (VectorDrawable::VPath*) target,startTime);
            }  else if (dynamic_cast<VectorDrawable::VFullPath*>(target)) {
                createRTAnimatorForFullPath(animator, (VectorDrawable::VFullPath*) target,startTime);
            } else if (!mDrawable->mAnimatedVectorState->mShouldIgnoreInvalidAnim) {
                throw std::logic_error("ClipPath only supports PathData property");
            }
        }
    } else if (dynamic_cast<VectorDrawable::VectorDrawableState*>(target)) {
        createRTAnimatorForRootGroup(values, animator,(VectorDrawable::VectorDrawableState*) target, startTime);
    }
}

void AnimatedVectorDrawable::VectorDrawableAnimatorRT::createRTAnimatorForGroup(const std::vector<PropertyValuesHolder*>& values,
        ObjectAnimator* animator, VectorDrawable::VGroup* target,long startTime) {

    long nativePtr = target.getNativePtr();
    int propertyId;
    for (int i = 0; i < values.length; i++) {
        // TODO: We need to support the rare case in AVD where no start value is provided
        values[i].getPropertyValues(mTmpValues);
        propertyId = VectorDrawable::VGroup::getPropertyIndex(mTmpValues.propertyName);
        if (mTmpValues.type != Float.class && mTmpValues.type != float.class) {
            if (DBG_ANIMATION_VECTOR_DRAWABLE) {
                LOGE("Unsupported type: %d. Only float value is supported for Groups.",mTmpValues.type);
            }
            continue;
        }
        if (propertyId < 0) {
            if (DBG_ANIMATION_VECTOR_DRAWABLE) {
                LOGE("Unsupported property: for Vector Drawable Group",mTmpValues.propertyName);
            }
            continue;
        }
        long propertyPtr = nCreateGroupPropertyHolder(nativePtr, propertyId,
                (Float) mTmpValues.startValue, (Float) mTmpValues.endValue);
        if (mTmpValues.dataSource != null) {
            std::vector<float>dataPoints = createFloatDataPoints(mTmpValues.dataSource,
                    animator->getDuration());
            nSetPropertyHolderData(propertyPtr, dataPoints, dataPoints.length);
        }
        createNativeChildAnimator(propertyPtr, startTime, animator);
    }
}

void AnimatedVectorDrawable::VectorDrawableAnimatorRT::createRTAnimatorForPath( ObjectAnimator* animator, VectorDrawable::VPath* target,long startTime) {

    long nativePtr = target->getNativePtr();
    long startPathDataPtr = ((PathParser::PathData*) mTmpValues.startValue).getNativePtr();
    long endPathDataPtr = ((PathParser::PathData*) mTmpValues.endValue).getNativePtr();
    long propertyPtr = nCreatePathDataPropertyHolder(nativePtr, startPathDataPtr,endPathDataPtr);
    createNativeChildAnimator(propertyPtr, startTime, animator);
}

void AnimatedVectorDrawable::VectorDrawableAnimatorRT::createRTAnimatorForFullPath(ObjectAnimator* animator,VectorDrawable::VFullPath* target, long startTime) {

    int propertyId = target->getPropertyIndex(mTmpValues.propertyName);
    long propertyPtr;
    long nativePtr = target->getNativePtr();
    if (mTmpValues.type == Float.class || mTmpValues.type == float.class) {
        if (propertyId < 0) {
            if (mDrawable->mAnimatedVectorState->mShouldIgnoreInvalidAnim) {
                return;
            } else {
                /*throw new IllegalArgumentException("Property: " + mTmpValues.propertyName
                        + " is not supported for FullPath");*/
            }
        }
        propertyPtr = nCreatePathPropertyHolder(nativePtr, propertyId,
                (Float) mTmpValues.startValue, (Float) mTmpValues.endValue);
        if (mTmpValues.dataSource != null) {
            // Pass keyframe data to native, if any.
            std::vector<float> dataPoints = createFloatDataPoints(mTmpValues.dataSource,
                    animator->getDuration());
            //nSetPropertyHolderData(propertyPtr, dataPoints, dataPoints.size());
        }

    } else if (mTmpValues.type == Integer.class || mTmpValues.type == int.class) {
        propertyPtr = nCreatePathColorPropertyHolder(nativePtr, propertyId,
                (Integer) mTmpValues.startValue, (Integer) mTmpValues.endValue);
        if (mTmpValues.dataSource != null) {
            // Pass keyframe data to native, if any.
            std::vector<int> dataPoints = createIntDataPoints(mTmpValues.dataSource,
                    animator->getDuration());
            //nSetPropertyHolderData(propertyPtr, dataPoints, dataPoints.size());
        }
    } else {
        if (mDrawable.mAnimatedVectorState.mShouldIgnoreInvalidAnim) {
            return;
        } else {
            /*throw new UnsupportedOperationException("Unsupported type: " +
                    mTmpValues.type + ". Only float, int or PathData value is " +
                    "supported for Paths.");*/
        }
    }
    createNativeChildAnimator(propertyPtr, startTime, animator);
}

void AnimatedVectorDrawable::VectorDrawableAnimatorRT::createRTAnimatorForRootGroup(const std::vector<PropertyValuesHolder*>& values,
        ObjectAnimator* animator, VectorDrawable::VectorDrawableState* target,long startTime) {
    long nativePtr = target->getNativeRenderer();
    if (!animator->getPropertyName().compare("alpha")==0) {
        if (mDrawable->mAnimatedVectorState->mShouldIgnoreInvalidAnim) {
            return;
        } else {
            throw std::logic_error("Only alpha is supported for root group");
        }
    }
    Float startValue = null;
    Float endValue = null;
    for (int i = 0; i < values.length; i++) {
        values[i].getPropertyValues(mTmpValues);
        if (mTmpValues.propertyName.equals("alpha")) {
            startValue = (Float) mTmpValues.startValue;
            endValue = (Float) mTmpValues.endValue;
            break;
        }
    }
    if (startValue == null && endValue == null) {
        if (mDrawable->mAnimatedVectorState->mShouldIgnoreInvalidAnim) {
            return;
        } else {
            throw std::logic_error("No alpha values are specified");
        }
    }
    long propertyPtr = nCreateRootAlphaPropertyHolder(nativePtr, startValue, endValue);
    if (mTmpValues.dataSource != null) {
        // Pass keyframe data to native, if any.
        std::vector<float> dataPoints = createFloatDataPoints(mTmpValues.dataSource,animator->getDuration());
        nSetPropertyHolderData(propertyPtr, dataPoints, dataPoints.size());
    }
    createNativeChildAnimator(propertyPtr, startTime, animator);
}

/**
 * Calculate the amount of frames an animation will run based on duration.
 */
int AnimatedVectorDrawable::VectorDrawableAnimatorRT::getFrameCount(long duration) {
    long frameIntervalNanos = Choreographer::getInstance().getFrameIntervalNanos();
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
        PropertyValuesHolder.PropertyValues.DataSource dataSource, long duration) {
    int numAnimFrames = getFrameCount(duration);
    std::vector<float> values(numAnimFrames);
    float lastFrame = numAnimFrames - 1;
    for (int i = 0; i < numAnimFrames; i++) {
        float fraction = i / lastFrame;
        values[i] = (Float) dataSource.getValueAtFraction(fraction);
    }
    return values;
}

std::vector<int> AnimatedVectorDrawable::VectorDrawableAnimatorRT::createIntDataPoints(
        PropertyValuesHolder.PropertyValues.DataSource dataSource, long duration) {
    int numAnimFrames = getFrameCount(duration);
    std::vector<int>values(numAnimFrames);
    float lastFrame = numAnimFrames - 1;
    for (int i = 0; i < numAnimFrames; i++) {
        float fraction = i / lastFrame;
        values[i] = (Integer) dataSource.getValueAtFraction(fraction);
    }
    return values;
}

void AnimatedVectorDrawable::VectorDrawableAnimatorRT::createNativeChildAnimator(long propertyPtr, long extraDelay,ObjectAnimator* animator) {
    long duration = animator->getDuration();
    int repeatCount = animator->getRepeatCount();
    long startDelay = extraDelay + animator->getStartDelay();
    TimeInterpolator* interpolator = animator->getInterpolator();
    long nativeInterpolator = RenderNodeAnimatorSetHelper.createNativeInterpolator(interpolator, duration);

    startDelay *= ValueAnimator::getDurationScale();
    duration *= ValueAnimator::getDurationScale();

    mStartDelays.push_back(startDelay);
    //nAddAnimator(mSetPtr, propertyPtr, nativeInterpolator, startDelay, duration,repeatCount, animator->getRepeatMode());
}

/**
 * Holds a weak reference to the target that was last seen (through the DisplayListCanvas
 * in the last draw call), so that when animator set needs to start, we can add the animator
 * to the last seen RenderNode target and start right away.
 */
void AnimatedVectorDrawable::VectorDrawableAnimatorRT::recordLastSeenTarget(DisplayListCanvas canvas) {
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
}

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
    if (mLastSeenTarget != nullptr) {
        RenderNode* target = mLastSeenTarget;//.get();
        return useTarget(target);
    }
    return false;
}

bool AnimatedVectorDrawable::VectorDrawableAnimatorRT::useTarget(RenderNode* target) {
    if (target != null && target.isAttached()) {
        target.registerVectorDrawableAnimator(this);
        return true;
    }
    return false;
}

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
    //nStart(mSetPtr, this, ++mLastListenerId);
    invalidateOwningView();
    if (mListener.onAnimationStart != nullptr) {
        //mListener.onAnimationStart(null);
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
    //nEnd(mSetPtr);
    invalidateOwningView();
}

// This should only be called after animator has been added to the RenderNode target.
void AnimatedVectorDrawable::VectorDrawableAnimatorRT::resetAnimation() {
    //nReset(mSetPtr);
    invalidateOwningView();
}

// This should only be called after animator has been added to the RenderNode target.
void AnimatedVectorDrawable::VectorDrawableAnimatorRT::reverseAnimation() {
    mStarted = true;
    //nReverse(mSetPtr, this, ++mLastListenerId);
    invalidateOwningView();
    if (mListener.onAnimationStart != nullptr) {
        //mListener.onAnimationStart(nullptr);
    }
}

long AnimatedVectorDrawable::VectorDrawableAnimatorRT::getAnimatorNativePtr() {
    return mSetPtr;
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
    if (canvas.isHardwareAccelerated()) {
        recordLastSeenTarget((DisplayListCanvas) canvas);
    }
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
}/*endof namespace*/
#endif
