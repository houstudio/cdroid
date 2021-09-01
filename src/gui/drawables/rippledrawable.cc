#include <drawables/rippledrawable.h>
namespace cdroid{

RippleDrawable::RippleState::RippleState(LayerState* orig, RippleDrawable* owner)
    :LayerDrawable::LayerState(orig,owner){
    //mTouchThemeAttrs = orig->mTouchThemeAttrs;
    if(dynamic_cast<RippleState*>(orig)){
        RippleState* origs = (RippleState*) orig;
        mColor = origs->mColor;
        mMaxRadius = origs->mMaxRadius;
        if (orig->mDensity != mDensity) {
            applyDensityScaling(orig->mDensity, mDensity);
        }
    }
}

void RippleDrawable::RippleState::onDensityChanged(int sourceDensity, int targetDensity){
    LayerDrawable::LayerState::onDensityChanged(sourceDensity, targetDensity);
    applyDensityScaling(sourceDensity, targetDensity);
}

void RippleDrawable::RippleState::applyDensityScaling(int sourceDensity, int targetDensity) {
    if (mMaxRadius != RADIUS_AUTO) {
        mMaxRadius = Drawable::scaleFromDensity(
                mMaxRadius, sourceDensity, targetDensity, true);
    }
}

Drawable* RippleDrawable::RippleState::newDrawable() {
    return new RippleDrawable(std::dynamic_pointer_cast<RippleState>(shared_from_this()));//, nullptr);
}

int RippleDrawable::RippleState::getChangingConfigurations()const{
    return LayerDrawable::LayerState::getChangingConfigurations()
        |(mColor != nullptr ? mColor->getChangingConfigurations() : 0);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

RippleDrawable::RippleDrawable(std::shared_ptr<RippleState> state) {
    mState.reset(new RippleState(state.get(), this));
    mLayerState = mState;
    mDensity = 160;//Drawable::resolveDensity(res, mState.mDensity);

    if (mState->mChildren.size()) {
        ensurePadding();
        refreshPadding();
    }
    updateLocalState();
}

RippleDrawable::RippleDrawable(const ColorStateList& color,Drawable* content,Drawable* mask)
   :RippleDrawable(std::make_shared<RippleState>(nullptr,nullptr)){

}

void RippleDrawable::jumpToCurrentState(){
    LayerDrawable::jumpToCurrentState();

    if (mRipple) mRipple->end();

    if (mBackground) mBackground->jumpToFinal();

    cancelExitingRipples();
}

void RippleDrawable::cancelExitingRipples(){
    for (auto ripple:mExitingRipples) {
        ripple->end();
        delete ripple;
    }
    mExitingRipples.clear();
    // Always draw an additional "clean" frame after canceling animations.
    invalidateSelf(false);
}

int RippleDrawable::getOpacity(){
    return TRANSLUCENT;
}

bool RippleDrawable::onStateChange(const std::vector<int>&stateSet){
    const bool changed = LayerDrawable::onStateChange(stateSet);

    bool enabled = false;
    bool pressed = false;
    bool focused = false;
    bool hovered = false;

    for (int state : stateSet) {
        if (state == StateSet::ENABLED) {
            enabled = true;
        } else if (state == StateSet::FOCUSED) {
            focused = true;
        } else if (state == StateSet::PRESSED) {
            pressed = true;
        } else if (state == StateSet::HOVERED) {
            hovered = true;
        }
    }

    setRippleActive(enabled && pressed);
    setBackgroundActive(hovered, focused, pressed);
    return changed;
}

void RippleDrawable::setRippleActive(bool active){
    if (mRippleActive != active) {
        mRippleActive = active;
        if (active) {
            tryRippleEnter();
        } else {
            tryRippleExit();
        }
    }
}

void RippleDrawable::setBackgroundActive(bool hovered, bool focused, bool pressed){
    if (mBackground == nullptr && (hovered || focused)) {
        mBackground = new RippleBackground(this, mHotspotBounds, isBounded());
        mBackground->setup(mState->mMaxRadius, mDensity);
    }
    if (mBackground) {
        mBackground->setState(focused, hovered, pressed);
    }
}

void RippleDrawable::onBoundsChange(const Rect& bounds){
    LayerDrawable::onBoundsChange(bounds);

    if (!mOverrideBounds) {
        mHotspotBounds=bounds;
        //onHotspotBoundsChanged();
    }

    for (auto ripple:mExitingRipples) {
        ripple->onBoundsChange();
    }

    if (mBackground) mBackground->onBoundsChange();

    if (mRipple) mRipple->onBoundsChange();

    invalidateSelf();
}

bool RippleDrawable::setVisible(bool visible, bool restart){
    bool changed = LayerDrawable::setVisible(visible, restart);

    if (!visible) {
        clearHotspots();
    } else if (changed) {
        // If we just became visible, ensure the background and ripple
        // visibilities are consistent with their internal states.
        if (mRippleActive) {
            tryRippleEnter();
        }

        // Skip animations, just show the correct final states.
        jumpToCurrentState();
    }
    return changed;
}

bool RippleDrawable::isProjected(){
    if (isBounded())return false;

    // Otherwise, if the maximum radius is contained entirely within the
    // bounds then we don't need to project. This is sort of a hack to
    // prevent check box ripples from being projected across the edges of
    // scroll views. It does not impact rendering performance, and it can
    // be removed once we have better handling of projection in scrollable
    // views.
    const int radius = mState->mMaxRadius;
    const Rect drawableBounds = getBounds();
    const Rect hotspotBounds = mHotspotBounds;
    if (radius != RADIUS_AUTO
            && radius <= hotspotBounds.width / 2
            && radius <= hotspotBounds.height / 2
            && (drawableBounds==hotspotBounds
                    || drawableBounds.contains(hotspotBounds))) {
        return false;
    }
    return true;
}

bool RippleDrawable::isBounded()const{
    return getNumberOfLayers() > 0;
}


bool RippleDrawable::isStateful()const{
    return true;
}

bool RippleDrawable::hasFocusStateSpecified()const{
    return true;
}

void RippleDrawable::setColor(ColorStateList* color){
    mState->mColor = color;
    invalidateSelf(false);
}

void RippleDrawable::setRadius(int radius) {
    mState->mMaxRadius = radius;
    invalidateSelf(false);
}

int RippleDrawable::getRadius()const{
    return mState->mMaxRadius;
}

bool RippleDrawable::setDrawableByLayerId(int id, Drawable* drawable){
    if (LayerDrawable::setDrawableByLayerId(id, drawable)) {
        if (id == 0/*R.id.mask*/) {
            mMask = drawable;
            mHasValidMask = false;
        }
        return true;
    }
    return false;
}

void RippleDrawable::setPaddingMode(int mode) {
    LayerDrawable::setPaddingMode(mode);
}

bool RippleDrawable::canApplyTheme() {
    return false;//(mState && mState->canApplyTheme()) || LayerDrawable::canApplyTheme();
}

void RippleDrawable::tryRippleEnter(){
    if (mExitingRipples.size() >= MAX_RIPPLES) {
        // This should never happen unless the user is tapping like a maniac
        // or there is a bug that's preventing ripples from being removed.
        return;
    }

    if (mRipple == nullptr) {
        float x;
        float y;
        if (mHasPending) {
            mHasPending = false;
            x = mPendingX;
            y = mPendingY;
        } else {
            x = mHotspotBounds.centerX();
            y = mHotspotBounds.centerY();
        }

        mRipple = new RippleForeground(this, mHotspotBounds, x, y, mForceSoftware);
    }

    mRipple->setup(mState->mMaxRadius, mDensity);
    mRipple->enter();
}

void RippleDrawable::tryRippleExit(){
    if (mRipple != nullptr) {
        mExitingRipples.push_back(mRipple);//[mExitingRipplesCount++] = mRipple;
        mRipple->exit();
        mRipple = nullptr;
    }
}

void RippleDrawable::clearHotspots(){
}

void RippleDrawable::draw(Canvas& canvas){

}

void RippleDrawable::invalidateSelf() {
    invalidateSelf(true);
}

void RippleDrawable::invalidateSelf(bool invalidateMask) {
    LayerDrawable::invalidateSelf();
    if (invalidateMask) {
        // Force the mask to update on the next draw().
        mHasValidMask = false;
    }
}

Drawable*RippleDrawable::inflate(Context*ctx,const AttributeSet&atts){
    RippleDrawable*rd=nullptr;
    return rd;
}

void RippleDrawable::updateLocalState() {
    // Initialize from constant state.
    mMask = findDrawableByLayerId(0);//R.id.mask);
}
}
