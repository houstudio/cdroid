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
#include <drawables/rippledrawable.h>
#include <widget/R.h>
namespace cdroid{

RippleDrawable::RippleState::RippleState(LayerState* orig, RippleDrawable* owner)
    :LayerDrawable::LayerState(orig,owner){
    //mTouchThemeAttrs = orig->mTouchThemeAttrs;
    mColor = nullptr;
    mMaxRadius = RADIUS_AUTO;
    if(dynamic_cast<RippleState*>(orig)){
        RippleState* origs = (RippleState*) orig;
        mColor = origs->mColor;
        mMaxRadius = origs->mMaxRadius;
        if (orig->mDensity != mDensity) {
            applyDensityScaling(orig->mDensity, mDensity);
        }
    }
}

RippleDrawable::RippleState::~RippleState(){
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

RippleDrawable* RippleDrawable::RippleState::newDrawable(){
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
    mDensity = Drawable::resolveDensity(mState->mDensity);
    mRipple  = nullptr;
    mBackground = nullptr;
    mRippleActive   = false;
    mOverrideBounds = false;
    if (mState->mChildren.size()) {
        ensurePadding();
        refreshPadding();
    }
    updateLocalState();
}

RippleDrawable::RippleDrawable():RippleDrawable(std::make_shared<RippleState>(nullptr,this)){
}

RippleDrawable::RippleDrawable(const ColorStateList* color,Drawable* content,Drawable* mask)
  :RippleDrawable(std::make_shared<RippleState>(nullptr,nullptr)){
    if(content)addLayer(content,{0},-1,0,0,0,0);
    if(mask)addLayer(mask,{0},cdroid::R::id::mask,0,0,0,0);
    setColor(color);
    ensurePadding();
    refreshPadding();
    updateLocalState();
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
    if ((mBackground == nullptr) && (hovered || focused)) {
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
        onHotspotBoundsChanged();
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

void RippleDrawable::setColor(const ColorStateList* color){
    if(mState->mColor!=color){
        mState->mColor = color;
        invalidateSelf(false);
    }
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
        if (id == cdroid::R::id::mask) {
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
        const float x = mHasPending ? mPendingX:mHotspotBounds.centerX();
        const float y = mHasPending ? mPendingY:mHotspotBounds.centerY();
        mHasPending = false;
        mRipple = new RippleForeground(this, mHotspotBounds, x, y, mForceSoftware);
    }

    mRipple->setup(mState->mMaxRadius, mDensity);
    mRipple->enter();
}

void RippleDrawable::tryRippleExit(){
    if (mRipple != nullptr) {
        mExitingRipples.push_back(mRipple);
        mRipple->exit();
        mRipple = nullptr;
    }
}

void RippleDrawable::clearHotspots(){
    if(mRipple){
        mRipple->end();
        mRipple =nullptr;
        mRippleActive =false;
    }
    if(mBackground)
        mBackground->setState(false,false,false);
    cancelExitingRipples();
}

void RippleDrawable::setHotspot(float x,float y){
    mHasPending=(mRipple==nullptr)||(mBackground ==nullptr);
    if (mHasPending){
         mPendingX =x;
         mPendingY = y;
    }
    if(mRipple)mRipple->move(x,y);
}

void RippleDrawable::setHotspotBounds(int left,int top,int w,int h){
    mOverrideBounds = true;
    mHotspotBounds.set(left,top,w,h);
    onHotspotBoundsChanged();
}

void RippleDrawable::getHotspotBounds(Rect&out)const{
    out = mHotspotBounds;
}

void RippleDrawable::onHotspotBoundsChanged(){
    for(auto ripple:mExitingRipples)
        ripple->onHotspotBoundsChanged();
    if(mRipple)mRipple->onHotspotBoundsChanged();
    if(mBackground)mBackground->onHotspotBoundsChanged();
}

void RippleDrawable::draw(Canvas& canvas){
    pruneRipples();

    // Clip to the dirty bounds, which will be the drawable bounds if we
    // have a mask or content and the ripple bounds if we're projecting.
    Rect bounds = getDirtyBounds();
    canvas.save();
    if (isBounded()) {
        canvas.rectangle(bounds.left,bounds.top,bounds.width,bounds.height);
        canvas.clip();
    }

    drawContent(canvas);
    drawBackgroundAndRipples(canvas);
    canvas.restore();
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

void RippleDrawable::pruneRipples() {
    for(auto it = mExitingRipples.begin();it!=mExitingRipples.end();){
        RippleForeground*fg = *it;
        if(fg->hasFinishedExit()){
            it = mExitingRipples.erase(it);
            delete fg;
        }else {
            it++;
        }
    }
}

int  RippleDrawable::getMaskType(){
    if ((mRipple == nullptr) && mExitingRipples.empty()
            && (mBackground == nullptr || !mBackground->isVisible())) {
        // We might need a mask later.
        return MASK_UNKNOWN;
    }

    if (mMask) {
        if (mMask->getOpacity() == OPAQUE) {
            // Clipping handles opaque explicit masks.
            return MASK_NONE;
        } else {
            return MASK_EXPLICIT;
        }
    }

    // Check for non-opaque, non-mask content.
    std::vector<ChildDrawable*>& array = mLayerState->mChildren;
    int count = array.size();
    for (int i = 0; i < count; i++) {
        if (array[i]->mDrawable->getOpacity() != OPAQUE) {
            return MASK_CONTENT;
        }
    }

    // Clipping handles opaque content.
    return MASK_NONE;
}

void RippleDrawable::drawContent(Canvas& canvas) {
    // Draw everything except the mask.
    std::vector<ChildDrawable*> &array = mLayerState->mChildren;
    const int count = (int)mLayerState->mChildren.size();
    for (int i = 0; i < count; i++) {
        if (array[i]->mId != cdroid::R::id::mask) {
            array[i]->mDrawable->draw(canvas);
        }
    }
}

void RippleDrawable::drawBackgroundAndRipples(Canvas& canvas) {
    RippleForeground* active = mRipple;
    if ((active == nullptr) && mExitingRipples.empty() && (mBackground == nullptr || !mBackground->isVisible())) {
        // Move along, nothing to draw here.
        return;
    }

    const float x = (float)mHotspotBounds.centerX();
    const float y = (float)mHotspotBounds.centerY();
    canvas.translate(x, y);
    int color = Color::MAGENTA;
    if(mState->mColor)
        color = mState->mColor->getColorForState(getState(),0xFF888888);
    canvas.set_color(color);

    if (mBackground  && mBackground->isVisible()) {
        mBackground->draw(canvas, 1.f);
    }
    for (auto ripple:mExitingRipples) {
        const int alpha = int(ripple->getOpacity()*0x80);
        color = (color&0x00FFFFFF) | (alpha<<24);
        canvas.set_color(color);
        ripple->draw(canvas,1.f);
    }

    if (active) active->draw(canvas,1.f);

    canvas.translate(-x, -y);
}

void RippleDrawable::drawMask(Canvas& canvas) {
    mMask->draw(canvas);

}

void RippleDrawable::drawSolid(Canvas& canvas){
    pruneRipples();

    // Clip to the dirty bounds, which will be the drawable bounds if we
    // have a mask or content and the ripple bounds if we're projecting.
    Rect bounds = getDirtyBounds();
    canvas.save();
    if (isBounded()) {
        canvas.rectangle(bounds);
        canvas.clip();
    }

    drawContent(canvas);
    drawBackgroundAndRipples(canvas);
    canvas.restore();
}

void RippleDrawable::exitPatternedBackgroundAnimation() {
    mTargetBackgroundOpacity = 0;
    if (mBackgroundAnimation) mBackgroundAnimation->cancel();
    // after cancel
    mRunBackgroundAnimation = true;
    invalidateSelf(false);
}

void RippleDrawable::startPatternedAnimation() {
    mAddRipple = true;
    invalidateSelf(false);
}

void RippleDrawable::exitPatternedAnimation() {
    mExitingAnimation = true;
    invalidateSelf(false);
}

void RippleDrawable::enterPatternedBackgroundAnimation(bool focused, bool hovered) {
    mBackgroundOpacity = 0;
    mTargetBackgroundOpacity = focused ? .6f : hovered ? .2f : 0.f;
    if (mBackgroundAnimation) mBackgroundAnimation->cancel();
    // after cancel
    mRunBackgroundAnimation = true;
    invalidateSelf(false);
}

void RippleDrawable::startBackgroundAnimation() {
    mRunBackgroundAnimation = false;
    /*if (Looper.myLooper() == null) {
        Log.w(TAG, "Thread doesn't have a looper. Skipping animation.");
        return;
    }*/
    mBackgroundAnimation = ValueAnimator::ofFloat({mBackgroundOpacity, mTargetBackgroundOpacity});
    /*mBackgroundAnimation->setInterpolator(LINEAR_INTERPOLATOR);
    mBackgroundAnimation->setDuration(BACKGROUND_OPACITY_DURATION);
    mBackgroundAnimation.addUpdateListener(update -> {
        mBackgroundOpacity = (float) update.getAnimatedValue();
        invalidateSelf(false);
    });*/
    mBackgroundAnimation->start();
}

void RippleDrawable::drawPatterned(Canvas& canvas) {
#if 0
    Rect bounds = mHotspotBounds;
    canvas.save();//Canvas.CLIP_SAVE_FLAG);
    bool useCanvasProps = !mForceSoftware;
    if (isBounded()) {
	canvas.rectangle(getDirtyBounds());
        canvas.clip();
    }
    float x, y, cx, cy, w, h;
    bool addRipple = mAddRipple;
    cx = bounds.centerX();
    cy = bounds.centerY();
    bool shouldExit = mExitingAnimation;
    mExitingAnimation = false;
    mAddRipple = false;
    if (mRunningAnimations.size() > 0 && !addRipple) {
        // update paint when view is invalidated
        updateRipplePaint();
    }
    drawContent(canvas);
    drawPatternedBackground(canvas, cx, cy);
    if (addRipple && mRunningAnimations.size() <= MAX_RIPPLES) {
        if (mHasPending) {
            x = mPendingX;
            y = mPendingY;
            mHasPending = false;
        } else {
            x = bounds.exactCenterX();
            y = bounds.exactCenterY();
        }
        h = bounds.height();
        w = bounds.width();
        RippleAnimationSession.AnimationProperties<Float, Paint> properties =
                createAnimationProperties(x, y, cx, cy, w, h);
        mRunningAnimations.add(new RippleAnimationSession(properties, !useCanvasProps)
                .setOnAnimationUpdated(() -> invalidateSelf(false))
                .setOnSessionEnd(session -> {
                    mRunningAnimations.remove(session);
                })
                .setForceSoftwareAnimation(!useCanvasProps)
                .enter(canvas));
    }
    if (shouldExit) {
        for (int i = 0; i < mRunningAnimations.size(); i++) {
            RippleAnimationSession s = mRunningAnimations.get(i);
            s.exit(canvas);
        }
    }
    for (int i = 0; i < mRunningAnimations.size(); i++) {
        RippleAnimationSession s = mRunningAnimations.get(i);
        if (!canvas.isHardwareAccelerated()) {
            Log.e(TAG, "The RippleDrawable.STYLE_PATTERNED animation is not supported for a "
                    + "non-hardware accelerated Canvas. Skipping animation.");
            break;
        } else if (useCanvasProps) {
            RippleAnimationSession.AnimationProperties<CanvasProperty<Float>,
                    CanvasProperty<Paint>>
                    p = s.getCanvasProperties();
            RecordingCanvas can = (RecordingCanvas) canvas;
            can.drawRipple(p.getX(), p.getY(), p.getMaxRadius(), p.getPaint(),
                    p.getProgress(), p.getNoisePhase(), p.getColor(), p.getShader());
        } else {
            RippleAnimationSession.AnimationProperties<Float, Paint> p =
                    s.getProperties();
            float radius = p.getMaxRadius();
            canvas.drawCircle(p.getX(), p.getY(), radius, p.getPaint());
        }
    }
    canvas.restoreToCount(saveCount);
#endif
}

Rect RippleDrawable::getDirtyBounds() const{
    if (!isBounded()) {
        Rect drawingBounds = mDrawingBounds;
        Rect dirtyBounds = mDirtyBounds;
        dirtyBounds = drawingBounds;
        drawingBounds.set(0,0,0,0);

        int cX = (int) mHotspotBounds.centerX();
        int cY = (int) mHotspotBounds.centerY();
        Rect rippleBounds;

        const std::vector<RippleForeground*>& activeRipples = mExitingRipples;
        const int N = (int)mExitingRipples.size();
        for (int i = 0; i < N; i++) {
            activeRipples[i]->getBounds(rippleBounds);
            rippleBounds.offset(cX, cY);
            drawingBounds.Union(rippleBounds);
        }

        if (mBackground) {
            mBackground->getBounds(rippleBounds);
            rippleBounds.offset(cX, cY);
            drawingBounds.Union(rippleBounds);
        }

        dirtyBounds.Union(drawingBounds);
        dirtyBounds.Union(LayerDrawable::getDirtyBounds());
        return dirtyBounds;
    } else {
        return getBounds();
    }
}

void RippleDrawable::inflate(XmlPullParser&parser,const AttributeSet&atts){
    
    // Force padding default to STACK before inflating.
    setPaddingMode(PADDING_MODE_STACK);
    LayerDrawable::inflate(parser,atts);
    mState->mColor = atts.getColorStateList("color");
    mState->mMaxRadius = atts.getDimensionPixelSize("radius", mState->mMaxRadius);
    updateLocalState();
}

void RippleDrawable::updateLocalState() {
    // Initialize from constant state.
    mMask = findDrawableByLayerId(cdroid::R::id::mask);
}
}
