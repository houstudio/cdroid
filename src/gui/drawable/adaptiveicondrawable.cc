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
#include <drawable/adaptiveicondrawable.h>
#include <drawable/pathparser.h>
namespace cdroid{
using namespace Cairo;

AdaptiveIconDrawable::AdaptiveIconDrawable()
    :AdaptiveIconDrawable((LayerState*) nullptr){
}

AdaptiveIconDrawable::AdaptiveIconDrawable(LayerState* state) {
    mLayerState = createConstantState(state);
    // config_icon_mask from context bound resource may have been chaged using
    // OverlayManager. Read that one first.
    /*Resources r = ActivityThread.currentActivityThread() == nullptr
            ? Resources.getSystem()
            : ActivityThread.currentActivityThread().getApplication().getResources();*/
    // TODO: either make sMask update only when config_icon_mask changes OR
    // get rid of it all-together in layoutlib
    /*sMask = PathParser::createPathFromPathData(r.getString(R.string.config_icon_mask));
    mMask = new Path(sMask);
    mMaskScaleOnly = new Path(mMask);*/
    //mMaskMatrix = new Matrix();
    //mCanvas = new Canvas();
    mTransparentRegion = Region::create();
}

AdaptiveIconDrawable::ChildDrawable*AdaptiveIconDrawable::createChildDrawable(Drawable* drawable) {
    ChildDrawable* layer = new ChildDrawable(mLayerState->mDensity);
    layer->mDrawable = drawable;
    layer->mDrawable->setCallback(this);
    mLayerState->mChildrenChangingConfigurations |= layer->mDrawable->getChangingConfigurations();
    return layer;
}

std::shared_ptr<AdaptiveIconDrawable::LayerState> AdaptiveIconDrawable::createConstantState(LayerState* state) {
    return std::make_shared<LayerState>(state, this);
}

AdaptiveIconDrawable::AdaptiveIconDrawable(Drawable* backgroundDrawable,Drawable* foregroundDrawable)
    :AdaptiveIconDrawable(backgroundDrawable, foregroundDrawable, nullptr){
}

AdaptiveIconDrawable::AdaptiveIconDrawable(Drawable* backgroundDrawable,
        Drawable* foregroundDrawable, Drawable* monochromeDrawable)
    :AdaptiveIconDrawable((LayerState*)nullptr){
    if (backgroundDrawable != nullptr) {
        addLayer(BACKGROUND_ID, createChildDrawable(backgroundDrawable));
    }
    if (foregroundDrawable != nullptr) {
        addLayer(FOREGROUND_ID, createChildDrawable(foregroundDrawable));
    }
    if (monochromeDrawable != nullptr) {
        addLayer(MONOCHROME_ID, createChildDrawable(monochromeDrawable));
    }
}

void AdaptiveIconDrawable::addLayer(int index,ChildDrawable* layer) {
    mLayerState->mChildren[index] = layer;
    mLayerState->invalidateCache();
}

void AdaptiveIconDrawable::inflate(XmlPullParser& parser,AttributeSet& attrs){
    Drawable::inflate(parser, attrs);

    auto state = mLayerState;
    if (state == nullptr) {
        return;
    }

    // The density may have changed since the last update. This will
    // apply scaling to any existing constant state properties.
    const int deviceDensity = Drawable::resolveDensity(0);
    state->setDensity(deviceDensity);
    state->mSrcDensityOverride = mSrcDensityOverride;
    //state->mSourceDrawableId = Resources.getAttributeSetSourceResId(attrs);

    auto& array = state->mChildren;
    for (int i = 0; i < array.size(); i++) {
        array[i]->setDensity(deviceDensity);
    }

    inflateLayers(parser, attrs);
}

float AdaptiveIconDrawable::getExtraInsetFraction() {
    return EXTRA_INSET_PERCENTAGE;
}

float AdaptiveIconDrawable::getExtraInsetPercentage() {
    return EXTRA_INSET_PERCENTAGE;
}

Path AdaptiveIconDrawable::getIconMask() {
    return mMask;
}

Drawable* AdaptiveIconDrawable::getForeground() {
    return mLayerState->mChildren[FOREGROUND_ID]->mDrawable;
}

Drawable* AdaptiveIconDrawable::getBackground() {
    return mLayerState->mChildren[BACKGROUND_ID]->mDrawable;
}

Drawable* AdaptiveIconDrawable::getMonochrome() {
    return mLayerState->mChildren[MONOCHROME_ID]->mDrawable;
}

void AdaptiveIconDrawable::onBoundsChange(const Rect& bounds) {
    if (bounds.empty()) {
        return;
    }
    updateLayerBounds(bounds);
}

void AdaptiveIconDrawable::updateLayerBounds(const Rect& bounds) {
    if (bounds.empty()) {
        return;
    }
    suspendChildInvalidation();
    updateLayerBoundsInternal(bounds);
    updateMaskBoundsInternal(bounds);
    resumeChildInvalidation();
}

void AdaptiveIconDrawable::updateLayerBoundsInternal(const Rect& bounds) {
    int cX = bounds.width / 2;
    int cY = bounds.height / 2;

    for (int i = 0, count = LayerState::N_CHILDREN; i < count; i++) {
        ChildDrawable* r = mLayerState->mChildren[i];
        Drawable* d = r->mDrawable;
        if (d == nullptr) {
            continue;
        }

        int insetWidth = (int) (bounds.width / (DEFAULT_VIEW_PORT_SCALE * 2));
        int insetHeight = (int) (bounds.height / (DEFAULT_VIEW_PORT_SCALE * 2));
        Rect outRect;
        outRect.set(cX - insetWidth, cY - insetHeight, cX + insetWidth, cY + insetHeight);
        d->setBounds(outRect);
    }
}

void AdaptiveIconDrawable::updateMaskBoundsInternal(const Rect& b) {
    // reset everything that depends on the view bounds
    mMaskMatrix.scale(b.width / MASK_SIZE, b.height / MASK_SIZE);
    //sMask.transform(mMaskMatrix, mMaskScaleOnly);

    mMaskMatrix.translate(b.left, b.top);
    //sMask.transform(mMaskMatrix, mMask);

    if ((mLayersBitmap == nullptr) || (mLayersBitmap->get_width() != b.width)
            || (mLayersBitmap->get_height() != b.height)) {
        mLayersBitmap = ImageSurface::create(Surface::Format::ARGB32,b.width, b.height);
    }

    //mPaint.setShader(null);
    //mTransparentRegion.setEmpty();
    //mLayersShader = null;
}

void AdaptiveIconDrawable::draw(Canvas& canvas) {
    if (mLayersBitmap == nullptr) {
        return;
    }
    /*if (mLayersShader == nullptr) {
        //mCanvas.setBitmap(mLayersBitmap);
        //mCanvas.drawColor(Color.BLACK);
        if (mLayerState->mChildren[BACKGROUND_ID]->mDrawable != nullptr) {
            mLayerState->mChildren[BACKGROUND_ID]->mDrawable->draw(*mCanvas);
        }
        if (mLayerState->mChildren[FOREGROUND_ID]->mDrawable != nullptr) {
            mLayerState->mChildren[FOREGROUND_ID]->mDrawable->draw(*mCanvas);
        }
        mLayersShader = new BitmapShader(mLayersBitmap, TileMode.CLAMP, TileMode.CLAMP);
        mPaint.setShader(mLayersShader);
    }
    if (mMaskScaleOnly != nullptr) {
        Rect bounds = getBounds();
        canvas.translate(bounds.left, bounds.top);
        //canvas.drawPath(mMaskScaleOnly, mPaint);
        canvas.translate(-bounds.left, -bounds.top);
    }*/
}

void AdaptiveIconDrawable::invalidateSelf() {
    //mLayersShader = nullptr;
    Drawable::invalidateSelf();
}

void AdaptiveIconDrawable::getOutline(Outline& outline) {
    //outline.setPath(mMask);
}

RefPtr<Region> AdaptiveIconDrawable::getSafeZone() {
    /*Path mask = getIconMask();
    mMaskMatrix.scale(SAFEZONE_SCALE, SAFEZONE_SCALE);//, getBounds().centerX(), getBounds().centerY());
    Path p = new Path();
    mask.transform(mMaskMatrix, p);*/
    auto safezoneRegion = Region::create((const RectangleInt&)mBounds);//getBounds());
    //safezoneRegion.setPath(p, safezoneRegion);
    return safezoneRegion;
}

Cairo::RefPtr<Cairo::Region> AdaptiveIconDrawable::getTransparentRegion() {
    if (mTransparentRegion->empty()) {
        //mMask.toggleInverseFillType();
        //mTransparentRegion.set(getBounds());
        //mTransparentRegion.setPath(mMask, mTransparentRegion);
        //mMask.toggleInverseFillType();
    }
    return mTransparentRegion;
}

/*void AdaptiveIconDrawable::applyTheme() {
    Drawable::applyTheme(t);

    auto state = mLayerState;
    if (state == nullptr) {
        return;
    }

    const int density = Drawable::resolveDensity(t.getResources(), 0);
    state->setDensity(density);

    for (int i = 0; i < LayerState::N_CHILDREN; i++) {
        ChildDrawable* layer = state->mChildren[i];
        layer->setDensity(density);

        if (layer->mThemeAttrs != nullptr) {
            TypedArray a = t.resolveAttributes(
                layer.mThemeAttrs, R.styleable.AdaptiveIconDrawableLayer);
            updateLayerFromTypedArray(layer, a);
            a.recycle();
        }

        Drawable* d = layer->mDrawable;
        if (d != nullptr && d->canApplyTheme()) {
            d->applyTheme(t);

            // Update cached mask of child changing configurations.
            state->mChildrenChangingConfigurations |= d->getChangingConfigurations();
        }
    }
}*/

int AdaptiveIconDrawable::getSourceDrawableResId() {
    return 0;//mLayerState == nullptr ? Resources.ID_NULL : mLayerState->mSourceDrawableId;
}

void AdaptiveIconDrawable::inflateLayers(XmlPullParser& parser,AttributeSet& attrs) {
    auto state = mLayerState;

    const int innerDepth = parser.getDepth() + 1;
    int type;
    int depth;
    int childIndex = 0;
    while ((type = parser.next()) != XmlPullParser::END_DOCUMENT
            && ((depth = parser.getDepth()) >= innerDepth || type != XmlPullParser::END_TAG)) {
        if (type != XmlPullParser::START_TAG) {
            continue;
        }

        if (depth > innerDepth) {
            continue;
        }
        std::string tagName = parser.getName();
        switch (tagName[0]) {
        case 'b'://"background":
            childIndex = BACKGROUND_ID;
            break;
        case 'f'://"foreground":
            childIndex = FOREGROUND_ID;
            break;
        case 'm'://"monochrome":
            childIndex = MONOCHROME_ID;
            break;
        default:
            continue;
        }

        ChildDrawable* layer = new ChildDrawable(state->mDensity);
        //final TypedArray a = obtainAttributes(r, theme, attrs,R.styleable.AdaptiveIconDrawableLayer);
        updateLayerFromTypedArray(layer, attrs);

        // If the layer doesn't have a drawable or unresolved theme
        // attribute for a drawable, attempt to parse one from the child
        // element. If multiple child elements exist, we'll only use the
        // first one.
        if (layer->mDrawable == nullptr && (layer->mThemeAttrs == nullptr)) {
            while ((type = parser.next()) == XmlPullParser::TEXT) {
            }
            if (type != XmlPullParser::START_TAG) {
                throw std::runtime_error(parser.getPositionDescription()+
                        ": <foreground> or <background> tag requires a 'drawable'"
                        "attribute or child tag defining a drawable");
            }

            // We found a child drawable. Take ownership.
            layer->mDrawable = Drawable::createFromXmlInnerForDensity(parser, attrs, mLayerState->mSrcDensityOverride);
            layer->mDrawable->setCallback(this);
            state->mChildrenChangingConfigurations |= layer->mDrawable->getChangingConfigurations();
        }
        addLayer(childIndex, layer);
    }
}

void AdaptiveIconDrawable::updateLayerFromTypedArray(ChildDrawable* layer,AttributeSet& a) {
    auto state = mLayerState;

    // Account for any configuration changes.
    //state->mChildrenChangingConfigurations |= a.getChangingConfigurations();

    // Extract the theme attributes, if any.
    //layer->mThemeAttrs = a.extractThemeAttrs();

    Drawable* dr = a.getDrawable("drawable");//a.getDrawableForDensity("drawable",state->mSrcDensityOverride);
    if (dr != nullptr) {
        if (layer->mDrawable != nullptr) {
            // It's possible that a drawable was already set, in which case
            // we should clear the callback. We may have also integrated the
            // drawable's changing configurations, but we don't have enough
            // information to revert that change.
            layer->mDrawable->setCallback(nullptr);
        }

        // Take ownership of the new drawable.
        layer->mDrawable = dr;
        layer->mDrawable->setCallback(this);
        state->mChildrenChangingConfigurations |= layer->mDrawable->getChangingConfigurations();
    }
}

bool AdaptiveIconDrawable::canApplyTheme() {
    return (mLayerState != nullptr && mLayerState->canApplyTheme()) || Drawable::canApplyTheme();
}

bool AdaptiveIconDrawable::isProjected() const{
    if (Drawable::isProjected()) {
        return true;
    }

    std::vector<ChildDrawable*>& layers = mLayerState->mChildren;
    for (int i = 0; i < LayerState::N_CHILDREN; i++) {
        if (layers[i]->mDrawable != nullptr && layers[i]->mDrawable->isProjected()) {
            return true;
        }
    }
    return false;
}

void AdaptiveIconDrawable::suspendChildInvalidation() {
    mSuspendChildInvalidation = true;
}

void AdaptiveIconDrawable::resumeChildInvalidation() {
    mSuspendChildInvalidation = false;

    if (mChildRequestedInvalidation) {
        mChildRequestedInvalidation = false;
        invalidateSelf();
    }
}

void AdaptiveIconDrawable::invalidateDrawable(Drawable& who) {
    if (mSuspendChildInvalidation) {
        mChildRequestedInvalidation = true;
    } else {
        invalidateSelf();
    }
}

void AdaptiveIconDrawable::scheduleDrawable(Drawable& who,const Runnable& what, int64_t when){
    scheduleSelf(what, when);
}

void AdaptiveIconDrawable::unscheduleDrawable(Drawable& who,const Runnable& what) {
    unscheduleSelf(what);
}

int AdaptiveIconDrawable::getChangingConfigurations() const{
    return Drawable::getChangingConfigurations() | mLayerState->getChangingConfigurations();
}

void AdaptiveIconDrawable::setHotspot(float x, float y) {
    for (int i = 0; i < LayerState::N_CHILDREN; i++) {
        Drawable* dr = mLayerState->mChildren[i]->mDrawable;
        if (dr != nullptr) {
            dr->setHotspot(x, y);
        }
    }
}

void AdaptiveIconDrawable::setHotspotBounds(int left, int top, int width, int height) {
    for (int i = 0; i < LayerState::N_CHILDREN; i++) {
        Drawable* dr = mLayerState->mChildren[i]->mDrawable;
        if (dr != nullptr) {
            dr->setHotspotBounds(left, top, width, height);
        }
    }

    mHotspotBounds.set(left, top, width, height);
}

void AdaptiveIconDrawable::getHotspotBounds(Rect& outRect) const{
    if (!mHotspotBounds.empty()) {
        outRect = mHotspotBounds;
    } else {
        Drawable::getHotspotBounds(outRect);
    }
}

bool AdaptiveIconDrawable::setVisible(bool visible, bool restart) {
    const bool changed = Drawable::setVisible(visible, restart);

    for (int i = 0; i < LayerState::N_CHILDREN; i++) {
        Drawable* dr = mLayerState->mChildren[i]->mDrawable;
        if (dr != nullptr) {
            dr->setVisible(visible, restart);
        }
    }

    return changed;
}

void AdaptiveIconDrawable::setDither(bool dither) {
    for (int i = 0; i < LayerState::N_CHILDREN; i++) {
        Drawable* dr = mLayerState->mChildren[i]->mDrawable;
        if (dr != nullptr) {
            dr->setDither(dither);
        }
    }
}

void AdaptiveIconDrawable::setAlpha(int alpha) {
    //mPaint.setAlpha(alpha);
    mAlpha = alpha;
}

int AdaptiveIconDrawable::getAlpha()const {
    return mAlpha;
}

void AdaptiveIconDrawable::setColorFilter(ColorFilter* colorFilter) {
    for (int i = 0; i < LayerState::N_CHILDREN; i++) {
        Drawable* dr = mLayerState->mChildren[i]->mDrawable;
        if (dr != nullptr) {
            dr->setColorFilter(colorFilter);
        }
    }
}

void AdaptiveIconDrawable::setTintList(const ColorStateList* tint) {
    for (int i = 0; i < LayerState::N_CHILDREN; i++) {
        Drawable* dr = mLayerState->mChildren[i]->mDrawable;
        if (dr != nullptr) {
            dr->setTintList(tint);
        }
    }
}

/*void AdaptiveIconDrawable::setTintBlendMode(BlendMode blendMode) {
    for (int i = 0; i < LayerState::N_CHILDREN; i++) {
        Drawable* dr = mLayerState->mChildren[i]->mDrawable;
        if (dr != nullptr) {
            dr->setTintBlendMode(blendMode);
        }
    }
}*/

void AdaptiveIconDrawable::setOpacity(int opacity) {
    mLayerState->mOpacityOverride = opacity;
}

int AdaptiveIconDrawable::getOpacity() {
    return PixelFormat::TRANSLUCENT;
}

void AdaptiveIconDrawable::setAutoMirrored(bool mirrored) {
    mLayerState->mAutoMirrored = mirrored;

    for (int i = 0; i < LayerState::N_CHILDREN; i++) {
        Drawable* dr = mLayerState->mChildren[i]->mDrawable;
        if (dr != nullptr) {
            dr->setAutoMirrored(mirrored);
        }
    }
}

bool AdaptiveIconDrawable::isAutoMirrored() const{
    return mLayerState->mAutoMirrored;
}

void AdaptiveIconDrawable::jumpToCurrentState() {
    for (int i = 0; i < LayerState::N_CHILDREN; i++) {
        Drawable* dr = mLayerState->mChildren[i]->mDrawable;
        if (dr != nullptr) {
            dr->jumpToCurrentState();
        }
    }
}

bool AdaptiveIconDrawable::isStateful() const{
    return mLayerState->isStateful();
}

bool AdaptiveIconDrawable::hasFocusStateSpecified() const{
    return mLayerState->hasFocusStateSpecified();
}

bool AdaptiveIconDrawable::onStateChange(const std::vector<int>& state) {
    bool changed = false;

    for (int i = 0; i < LayerState::N_CHILDREN; i++) {
        Drawable* dr = mLayerState->mChildren[i]->mDrawable;
        if (dr != nullptr && dr->isStateful() && dr->setState(state)) {
            changed = true;
        }
    }

    if (changed) {
        updateLayerBounds(getBounds());
    }

    return changed;
}

bool AdaptiveIconDrawable::onLevelChange(int level) {
    bool changed = false;

    for (int i = 0; i < LayerState::N_CHILDREN; i++) {
        Drawable* dr = mLayerState->mChildren[i]->mDrawable;
        if (dr != nullptr && dr->setLevel(level)) {
            changed = true;
        }
    }

    if (changed) {
        updateLayerBounds(getBounds());
    }

    return changed;
}

int AdaptiveIconDrawable::getIntrinsicWidth() {
    return (int)(getMaxIntrinsicWidth() * DEFAULT_VIEW_PORT_SCALE);
}

int AdaptiveIconDrawable::getMaxIntrinsicWidth() {
    int width = -1;
    for (int i = 0; i < LayerState::N_CHILDREN; i++) {
        ChildDrawable* r = mLayerState->mChildren[i];
        if (r->mDrawable == nullptr) {
            continue;
        }
        const int w = r->mDrawable->getIntrinsicWidth();
        if (w > width) {
            width = w;
        }
    }
    return width;
}

int AdaptiveIconDrawable::getIntrinsicHeight() {
    return (int)(getMaxIntrinsicHeight() * DEFAULT_VIEW_PORT_SCALE);
}

int AdaptiveIconDrawable::getMaxIntrinsicHeight() {
    int height = -1;
    for (int i = 0; i < LayerState::N_CHILDREN; i++) {
        ChildDrawable* r = mLayerState->mChildren[i];
        if (r->mDrawable == nullptr) {
            continue;
        }
        const int h = r->mDrawable->getIntrinsicHeight();
        if (h > height) {
            height = h;
        }
    }
    return height;
}

std::shared_ptr<Drawable::ConstantState> AdaptiveIconDrawable::getConstantState() {
    if (mLayerState->canConstantState()) {
        mLayerState->mChangingConfigurations = getChangingConfigurations();
        return mLayerState;
    }
    return nullptr;
}

Drawable* AdaptiveIconDrawable::mutate() {
    if (!mMutated && Drawable::mutate() == this) {
        mLayerState = createConstantState(mLayerState.get());
        for (int i = 0; i < LayerState::N_CHILDREN; i++) {
            Drawable* dr = mLayerState->mChildren[i]->mDrawable;
            if (dr != nullptr) {
                dr->mutate();
            }
        }
        mMutated = true;
    }
    return this;
}

void AdaptiveIconDrawable::clearMutated() {
    Drawable::clearMutated();
    for (int i = 0; i < LayerState::N_CHILDREN; i++) {
        Drawable* dr = mLayerState->mChildren[i]->mDrawable;
        if (dr != nullptr) {
            dr->clearMutated();
        }
    }
    mMutated = false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

//class ChildDrawable

AdaptiveIconDrawable::ChildDrawable::ChildDrawable(int density) {
    mDensity = density;
    mThemeAttrs = nullptr;
}

AdaptiveIconDrawable::ChildDrawable::ChildDrawable(ChildDrawable* orig, AdaptiveIconDrawable* owner) {

    Drawable* dr = orig->mDrawable;
    Drawable* clone;
    if (dr != nullptr) {
        auto cs = dr->getConstantState();
        if (cs == nullptr) {
            clone = dr;
        }/* else if (res != nullptr) {
            clone = cs->newDrawable(res);
        }*/ else {
            clone = cs->newDrawable();
        }
        clone->setCallback(owner);
        clone->setBounds(dr->getBounds());
        clone->setLevel(dr->getLevel());
    } else {
        clone = nullptr;
    }

    mDrawable = clone;
    mThemeAttrs = orig->mThemeAttrs;

    mDensity = Drawable::resolveDensity(orig->mDensity);
}

bool AdaptiveIconDrawable::ChildDrawable::canApplyTheme() const{
    return false;//(mThemeAttrs != nullptr)|| (mDrawable != nullptr && mDrawable->canApplyTheme());
}

void AdaptiveIconDrawable::ChildDrawable::setDensity(int targetDensity) {
    if (mDensity != targetDensity) {
        mDensity = targetDensity;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////

//static class LayerState extends ConstantState
AdaptiveIconDrawable::LayerState::LayerState(LayerState* orig, AdaptiveIconDrawable* owner) {
    mDensity = Drawable::resolveDensity(orig != nullptr ? orig->mDensity : 0);
    mChildren.resize(N_CHILDREN);// = new ChildDrawable[N_CHILDREN];
    if (orig != nullptr) {
        std::vector<ChildDrawable*>& origChildDrawable = orig->mChildren;

        mChangingConfigurations = orig->mChangingConfigurations;
        mChildrenChangingConfigurations = orig->mChildrenChangingConfigurations;
        mSourceDrawableId = orig->mSourceDrawableId;

        for (int i = 0; i < N_CHILDREN; i++) {
            ChildDrawable* orcd = origChildDrawable[i];
            mChildren[i] = new ChildDrawable(orcd, owner);
        }

        mCheckedOpacity = orig->mCheckedOpacity;
        mOpacity = orig->mOpacity;
        mCheckedStateful = orig->mCheckedStateful;
        mIsStateful = orig->mIsStateful;
        mAutoMirrored = orig->mAutoMirrored;
        //mThemeAttrs = orig->mThemeAttrs;
        mOpacityOverride = orig->mOpacityOverride;
        mSrcDensityOverride = orig->mSrcDensityOverride;
    } else {
        for (int i = 0; i < N_CHILDREN; i++) {
            mChildren[i] = new ChildDrawable(mDensity);
        }
    }
}

void AdaptiveIconDrawable::LayerState::setDensity(int targetDensity) {
    if (mDensity != targetDensity) {
        mDensity = targetDensity;
    }
}

bool AdaptiveIconDrawable::LayerState::canApplyTheme() const{
    /*if (mThemeAttrs != nullptr || Drawable::ConstantState::canApplyTheme()) {
        return true;
    }

    for (int i = 0; i < N_CHILDREN; i++) {
        ChildDrawable* layer = mChildren[i];
        if (layer->canApplyTheme()) {
            return true;
        }
    }*/
    return false;
}

Drawable* AdaptiveIconDrawable::LayerState::newDrawable() {
    return new AdaptiveIconDrawable(this);
}

int AdaptiveIconDrawable::LayerState::getChangingConfigurations() const{
    return mChangingConfigurations | mChildrenChangingConfigurations;
}

int AdaptiveIconDrawable::LayerState::getOpacity() {
    if (mCheckedOpacity) {
        return mOpacity;
    }

    // Seek to the first non-nullptr drawable.
    int firstIndex = -1;
    for (int i = 0; i < N_CHILDREN; i++) {
        if (mChildren[i]->mDrawable != nullptr) {
            firstIndex = i;
            break;
        }
    }

    int op;
    if (firstIndex >= 0) {
        op = mChildren[firstIndex]->mDrawable->getOpacity();
    } else {
        op = PixelFormat::TRANSPARENT;
    }

    // Merge all remaining non-nullptr drawables.
    for (int i = firstIndex + 1; i < N_CHILDREN; i++) {
        Drawable* dr = mChildren[i]->mDrawable;
        if (dr != nullptr) {
            op = Drawable::resolveOpacity(op, dr->getOpacity());
        }
    }

    mOpacity = op;
    mCheckedOpacity = true;
    return op;
}

bool AdaptiveIconDrawable::LayerState::isStateful() {
    if (mCheckedStateful) {
        return mIsStateful;
    }

    bool isStateful = false;
    for (int i = 0; i < N_CHILDREN; i++) {
        Drawable* dr = mChildren[i]->mDrawable;
        if (dr != nullptr && dr->isStateful()) {
            isStateful = true;
            break;
        }
    }

    mIsStateful = isStateful;
    mCheckedStateful = true;
    return isStateful;
}

bool AdaptiveIconDrawable::LayerState::hasFocusStateSpecified() const{
    for (int i = 0; i < N_CHILDREN; i++) {
        Drawable* dr = mChildren[i]->mDrawable;
        if (dr != nullptr && dr->hasFocusStateSpecified()) {
            return true;
        }
    }
    return false;
}

bool AdaptiveIconDrawable::LayerState::canConstantState() {
    for (int i = 0; i < N_CHILDREN; i++) {
        Drawable* dr = mChildren[i]->mDrawable;
        if (dr != nullptr && dr->getConstantState() == nullptr) {
            return false;
        }
    }

    // Don't cache the result, this method is not called very often.
    return true;
}

void AdaptiveIconDrawable::LayerState::invalidateCache() {
    mCheckedOpacity = false;
    mCheckedStateful = false;
}

}/*endof namespace*/
