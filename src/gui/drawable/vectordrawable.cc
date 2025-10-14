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
#include <stack>
#include <fstream>
#include <unordered_map>
#include <core/canvas.h>
#include <porting/cdlog.h>
#include <drawable/hwpathparser.h>
#include <drawable/vectordrawable.h>
#include <drawable/hwvectordrawable.h>
#include <drawable/drawableinflater.h>
namespace cdroid{

VectorDrawable::VectorDrawable()
    :VectorDrawable(std::make_shared<VectorDrawableState>(nullptr)){
}

VectorDrawable::VectorDrawable(std::shared_ptr<VectorDrawableState> state) {
    mMutated = false;
    mColorFilter= nullptr;
    mTintFilter = nullptr;
    mTargetDensity=0;
    mVectorState = state;
    updateLocalState();
}

VectorDrawable::~VectorDrawable(){
    delete mTintFilter;
    delete mColorFilter;
}

void VectorDrawable::updateLocalState() {
    const int density = Drawable::resolveDensity(mVectorState->mDensity);
    if (mTargetDensity != density) {
        mTargetDensity = density;
        mDpiScaledDirty = true;
    }

    mTintFilter = updateTintFilter(mTintFilter, mVectorState->mTint, mVectorState->mTintMode);
}

Drawable* VectorDrawable::mutate() {
    if (!mMutated && Drawable::mutate() == this) {
        auto ss=mVectorState;
        mVectorState = nullptr;
        mVectorState = std::make_shared<VectorDrawableState>(ss.get());
        mMutated = true;
    }
    return this;
}

void VectorDrawable::clearMutated() {
    Drawable::clearMutated();
    mMutated = false;
}

void* VectorDrawable::getTargetByName(const std::string& name) {
    auto it = mVectorState->mVGTargetsMap.find(name);
    return (it==mVectorState->mVGTargetsMap.end())?nullptr:it->second;
}

std::shared_ptr<Drawable::ConstantState> VectorDrawable::getConstantState() {
    mVectorState->mChangingConfigurations = getChangingConfigurations();
    return mVectorState;
}

void VectorDrawable::draw(Canvas& canvas) {
    // We will offset the bounds for drawBitmap, so copyBounds() here instead
    // of getBounds().
    Rect mTmpBounds;
    copyBounds(mTmpBounds);
    if (mTmpBounds.width <= 0 || mTmpBounds.height <= 0) {
        // Nothing to draw
        return;
    }

    // Color filters always override tint filters.
    ColorFilter* colorFilter = (mColorFilter == nullptr ? mTintFilter : mColorFilter);
    //long colorFilterNativeInstance = colorFilter == nullptr ? 0 :colorFilter.getNativeInstance();
    const bool canReuseCache = mVectorState->canReuseCache();
    /*int pixelCount = nDraw(mVectorState->getNativeRenderer(), canvas.getNativeCanvasWrapper(),
            colorFilterNativeInstance, mTmpBounds, needMirroring(), canReuseCache)*/;
    if(colorFilter){
        canvas.rectangle(mTmpBounds.left,mTmpBounds.top,mTmpBounds.width,mTmpBounds.height);
        canvas.clip();
        canvas.push_group();
    }
    const int pixelCount = mVectorState->mNativeTree->draw(canvas,nullptr,mTmpBounds,needMirroring(),canReuseCache);
    if(colorFilter){
        mTintFilter->apply(canvas,mBounds);
        canvas.pop_group_to_source();
        canvas.paint();
    }
    if (pixelCount == 0) {
        // Invalid canvas matrix or drawable bounds. This would not affect existing bitmap
        // cache, if any.
        return;
    }
    int deltaInBytes;
    // Track different bitmap cache based whether the canvas is hw accelerated. By doing so,
    // we don't over count bitmap cache allocation: if the input canvas is always of the same
    // type, only one bitmap cache is allocated.
    if (0/*canvas.isHardwareAccelerated()*/) {
        // Each pixel takes 4 bytes.
        deltaInBytes = (pixelCount - mVectorState->mLastHWCachePixelCount) * 4;
        mVectorState->mLastHWCachePixelCount = pixelCount;
    } else {
        // Each pixel takes 4 bytes.
        deltaInBytes = (pixelCount - mVectorState->mLastSWCachePixelCount) * 4;
        mVectorState->mLastSWCachePixelCount = pixelCount;
    }
}

int VectorDrawable::getAlpha() const{
    return (int) (mVectorState->getAlpha() * 255);
}

void VectorDrawable::setAlpha(int alpha) {
    if (mVectorState->setAlpha(float(alpha&0xFF) / 255.f)) {
        invalidateSelf();
    }
}

void VectorDrawable::setColorFilter(ColorFilter* colorFilter) {
    mColorFilter = colorFilter;
    invalidateSelf();
}

ColorFilter* VectorDrawable::getColorFilter() {
    return mColorFilter;
}

void VectorDrawable::setTintList(const ColorStateList* tint) {
    auto state = mVectorState;
    if (state->mTint != tint) {
        state->mTint =(ColorStateList*)tint;
        mTintFilter = updateTintFilter(mTintFilter, tint, state->mTintMode);
        invalidateSelf();
    }
}

void VectorDrawable::setTintMode(int tintMode) {
    auto state = mVectorState;
    if (state->mTintMode != tintMode) {
        state->mTintMode = tintMode;
        mTintFilter = updateTintFilter(mTintFilter, state->mTint, tintMode);
        invalidateSelf();
    }
}

bool VectorDrawable::isStateful() const{
    return Drawable::isStateful() || (mVectorState != nullptr && mVectorState->isStateful());
}

bool VectorDrawable::hasFocusStateSpecified()const {
    return (mVectorState != nullptr) && mVectorState->hasFocusStateSpecified();
}

bool VectorDrawable::onStateChange(const std::vector<int>&stateSet) {
    bool changed = false;

    // When the VD is stateful, we need to mutate the drawable such that we don't share the
    // cache bitmap with others. Such that the state change only affect this new cached bitmap.
    if (isStateful()) {
        mutate();
    }
    auto state = mVectorState;
    if (state->onStateChange(stateSet)) {
        changed = true;
        state->mCacheDirty = true;
    }
    if (state->mTint && state->mTintMode) {
        mTintFilter = updateTintFilter(mTintFilter, state->mTint, state->mTintMode);
        changed = true;
    }

    return changed;
}

int VectorDrawable::getOpacity() {
    // We can't tell whether the drawable is fully opaque unless we examine all the pixels,
    // but we could tell it is transparent if the root alpha is 0.
    return getAlpha() == 0 ? PixelFormat::TRANSPARENT : PixelFormat::TRANSLUCENT;
}

int VectorDrawable::getIntrinsicWidth(){
    if (mDpiScaledDirty) {
        computeVectorSize();
    }
    return mDpiScaledWidth;
}

int VectorDrawable::getIntrinsicHeight(){
    if (mDpiScaledDirty) {
        computeVectorSize();
    }
    return mDpiScaledHeight;
}

Insets VectorDrawable::getOpticalInsets() {
    if (mDpiScaledDirty) {
        computeVectorSize();
    }
    return mDpiScaledInsets;
}

/*
 * Update local dimensions to adjust for a target density that may differ
 * from the source density against which the constant state was loaded.
 */
void VectorDrawable::computeVectorSize() {
    const Insets& opticalInsets = mVectorState->mOpticalInsets;

    const int sourceDensity = mVectorState->mDensity;
    const int targetDensity = mTargetDensity;
    if (targetDensity != sourceDensity) {
        mDpiScaledWidth = Drawable::scaleFromDensity(mVectorState->mBaseWidth, sourceDensity,targetDensity, true);
        mDpiScaledHeight = Drawable::scaleFromDensity(mVectorState->mBaseHeight,sourceDensity,targetDensity, true);
        const int left = Drawable::scaleFromDensity(opticalInsets.left, sourceDensity, targetDensity, false);
        const int right = Drawable::scaleFromDensity(opticalInsets.right, sourceDensity, targetDensity, false);
        const int top = Drawable::scaleFromDensity(opticalInsets.top, sourceDensity, targetDensity, false);
        const int bottom = Drawable::scaleFromDensity(opticalInsets.bottom, sourceDensity, targetDensity, false);
        mDpiScaledInsets = Insets::of(left, top, right, bottom);
    } else {
        mDpiScaledWidth = mVectorState->mBaseWidth;
        mDpiScaledHeight = mVectorState->mBaseHeight;
        mDpiScaledInsets = opticalInsets;
    }

    mDpiScaledDirty = false;
}

bool VectorDrawable::canApplyTheme() {
    return (mVectorState != nullptr && mVectorState->canApplyTheme()) || Drawable::canApplyTheme();
}

#if 0
void VectorDrawable::applyTheme(Theme t) {
    super.applyTheme(t);

    final VectorDrawableState state = mVectorState;
    if (state == null) {
        return;
    }

    final bool changedDensity = mVectorState.setDensity(
            Drawable::resolveDensity(t.getResources(), 0));
    mDpiScaledDirty |= changedDensity;

    if (state.mThemeAttrs != null) {
        final TypedArray a = t.resolveAttributes(
                state.mThemeAttrs, R.styleable.VectorDrawable);
        try {
            state.mCacheDirty = true;
            updateStateFromTypedArray(a);
        } catch (XmlPullParserException e) {
            throw new RuntimeException(e);
        } finally {
            a.recycle();
        }

        // May have changed size.
        mDpiScaledDirty = true;
    }

    // Apply theme to contained color state list.
    if (state.mTint != null && state.mTint.canApplyTheme()) {
        state.mTint = state.mTint.obtainForTheme(t);
    }

    if (mVectorState != null && mVectorState.canApplyTheme()) {
        mVectorState.applyTheme(t);
    }

    // Update local properties.
    updateLocalState(t.getResources());
}
#endif

/**
 * The size of a pixel when scaled from the intrinsic dimension to the viewport dimension.
 * This is used to calculate the path animation accuracy.
 *
 * @hide
 */
float VectorDrawable::getPixelSize() {
    if ((mVectorState == nullptr) ||(mVectorState->mBaseWidth == 0) || (mVectorState->mBaseHeight == 0)
            || (mVectorState->mViewportHeight == 0) || (mVectorState->mViewportWidth == 0)) {
        return 1; // fall back to 1:1 pixel mapping.
    }
    const float intrinsicWidth = mVectorState->mBaseWidth;
    const float intrinsicHeight = mVectorState->mBaseHeight;
    const float viewportWidth = mVectorState->mViewportWidth;
    const float viewportHeight = mVectorState->mViewportHeight;
    const float scaleX = viewportWidth / intrinsicWidth;
    const float scaleY = viewportHeight / intrinsicHeight;
    return std::min(scaleX, scaleY);
}

VectorDrawable* VectorDrawable::create(Context*ctx, const std::string&rid) {
    VectorDrawable* drawable = (VectorDrawable*)DrawableInflater::loadDrawable(ctx,rid);//new VectorDrawable();
    //drawable->inflate(ctx,rid);
    return drawable;
}

void VectorDrawable::inflate(XmlPullParser&parser,const AttributeSet&atts){

    if (mVectorState->mRootGroup != nullptr || mVectorState->mNativeTree != nullptr) {
        // This VD has been used to display other VD resource content, clean up.
        if (mVectorState->mRootGroup != nullptr) {
            // Subtract the native allocation for all the nodes.
            // Remove child nodes' reference to tree
            mVectorState->mRootGroup->setTree(nullptr);
        }
        delete mVectorState->mRootGroup;//
        mVectorState->mRootGroup = new VGroup();
        if (mVectorState->mNativeTree != nullptr) {
            // Subtract the native allocation for the tree wrapper, which contains root node
            // as well as rendering related data.
            delete mVectorState->mNativeTree;//->release();
            mVectorState->mNativeTree = nullptr;
        }
        mVectorState->createNativeTree(mVectorState->mRootGroup);
    }

    auto state = mVectorState;
    mVectorState->setDensity(Drawable::resolveDensity(0));

    updateStateFromTypedArray(atts);
    mDpiScaledDirty = true;
    mVectorState->mCacheDirty = true;

    inflateChildElements(parser,atts);
    mVectorState->onTreeConstructionFinished();
    // Update local properties.
    updateLocalState();
}

void VectorDrawable::updateStateFromTypedArray(const AttributeSet&atts){
    auto state = mVectorState;

    // Account for any configuration changes.
    state->mChangingConfigurations = 0;//|= a.getChangingConfigurations();

    // Extract the theme attributes, if any.
    //state->mThemeAttrs = atts.extractThemeAttrs();

    const int tintMode = atts.getInt("tintMode", -1);
    if (tintMode != -1) {
        //state->mTintMode = Drawable::parseTintMode(tintMode, Mode::SRC_IN);
    }

    ColorStateList* tint = atts.getColorStateList("tint");
    if (tint != nullptr) {
        state->mTint = tint;
    }

    state->mAutoMirrored = atts.getBoolean("autoMirrored", state->mAutoMirrored);

    const float viewportWidth = atts.getFloat("viewportWidth", state->mViewportWidth);
    const float viewportHeight = atts.getFloat("viewportHeight", state->mViewportHeight);
    state->setViewportSize(viewportWidth, viewportHeight);

    if (state->mViewportWidth <= 0) {
        LOGE("<vector> tag requires viewportWidth > 0");
    } else if (state->mViewportHeight <= 0) {
        LOGE("<vector> tag requires viewportHeight > 0");
    }

    state->mBaseWidth = atts.getDimensionPixelSize("width", state->mBaseWidth);
    state->mBaseHeight = atts.getDimensionPixelSize("height", state->mBaseHeight);

    if (state->mBaseWidth <= 0) {
        LOGE("<vector> tag requires width > 0");
    } else if (state->mBaseHeight <= 0) {
        LOGE("<vector> tag requires height > 0");
    }

    const int insetLeft = atts.getDimensionPixelOffset("opticalInsetLeft", state->mOpticalInsets.left);
    const int insetTop = atts.getDimensionPixelOffset("opticalInsetTop", state->mOpticalInsets.top);
    const int insetRight = atts.getDimensionPixelOffset("opticalInsetRight", state->mOpticalInsets.right);
    const int insetBottom = atts.getDimensionPixelOffset("opticalInsetBottom", state->mOpticalInsets.bottom);
    state->mOpticalInsets = Insets::of(insetLeft, insetTop, insetRight, insetBottom);

    const float alphaInFloat = atts.getFloat("alpha", state->getAlpha());
    state->setAlpha(alphaInFloat);

    const std::string name = atts.getString("name");
    if (!name.empty()) {
        state->mRootName = name;
        state->mRootGroup->mGroupName = name;
        LOGD("%p rootName=%s",state->mRootGroup,name.c_str());
        state->mVGTargetsMap.emplace(name, state.get());
    }

}

void VectorDrawable::inflateChildElements(XmlPullParser&parser,const AttributeSet&atts){
    auto state = mVectorState;
    bool noPathTag = true;

    // Use a stack to help to build the group tree.
    // The top of the stack is always the current group.
    std::stack<VGroup*> groupStack;;
    groupStack.push(state->mRootGroup);

    int eventType;
    const int innerDepth = parser.getDepth()+1;
    // Parse everything until the end of the vector element.
    while (((eventType =parser.next())!= XmlPullParser::END_DOCUMENT)
            && (parser.getDepth() >= innerDepth || eventType != XmlPullParser::END_TAG)) {
        if (eventType == XmlPullParser::START_TAG) {
            const std::string tagName = parser.getName();
            VGroup* currentGroup = groupStack.top();

            if (tagName.compare(SHAPE_PATH)==0) {
                VFullPath* path = new VFullPath();
                path->inflate(parser,atts);
                currentGroup->addChild(path);
                if (!path->getPathName().empty()) {
                    state->mVGTargetsMap.emplace(path->getPathName(), path);
                }
                noPathTag = false;
                state->mChangingConfigurations |= path->mChangingConfigurations;
            } else if (tagName.compare(SHAPE_CLIP_PATH)==0) {
                VClipPath* path = new VClipPath();
                path->inflate(parser,atts);
                currentGroup->addChild(path);
                if (!path->getPathName().empty()) {
                    state->mVGTargetsMap.emplace(path->getPathName(), path);
                }
                state->mChangingConfigurations |= path->mChangingConfigurations;
            } else if (tagName.compare(SHAPE_GROUP)==0) {
                VGroup* newChildGroup = new VGroup();
                newChildGroup->inflate(parser,atts);
                currentGroup->addChild(newChildGroup);
                groupStack.push(newChildGroup);
                if (!newChildGroup->getGroupName().empty()) {
                    state->mVGTargetsMap.emplace(newChildGroup->getGroupName(), newChildGroup);
                }
                state->mChangingConfigurations |= newChildGroup->mChangingConfigurations;
            }
        } else if (eventType == XmlPullParser::END_TAG) {
            const std::string tagName = parser.getName();
            if (tagName.compare(SHAPE_GROUP)==0) {
                groupStack.pop();
            }
        }
    }

    if (noPathTag) {
        std::ostringstream tag;
        tag<<"no "<<SHAPE_PATH<<" defined";
        throw std::logic_error(tag.str());
    }
}

int VectorDrawable::getChangingConfigurations()const {
    return Drawable::getChangingConfigurations() | mVectorState->getChangingConfigurations();
}

void VectorDrawable::setAllowCaching(bool allowCaching) {
    //nSetAllowCaching(mVectorState->getNativeRenderer(), allowCaching);
    hwui::Tree*tree=(hwui::Tree*)mVectorState->getNativeRenderer();
    mVectorState->mNativeTree->setAllowCaching(allowCaching);
}

bool VectorDrawable::needMirroring() {
    return isAutoMirrored() && (getLayoutDirection() == LayoutDirection::RTL);
}

void VectorDrawable::setAutoMirrored(bool mirrored) {
    if (mVectorState->mAutoMirrored != mirrored) {
        mVectorState->mAutoMirrored = mirrored;
        invalidateSelf();
    }
}

bool VectorDrawable::isAutoMirrored()const{
    return mVectorState->mAutoMirrored;
}

long VectorDrawable::getNativeTree() {
    return mVectorState->getNativeRenderer();
}

void VectorDrawable::setAntiAlias(bool aa) {
    //nSetAntiAlias(mVectorState.mNativeTree.get(), aa);
    mVectorState->mNativeTree->setAntiAlias(aa);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////
///static class VectorDrawableState extends ConstantState {
namespace {
class PROP_ALPHA:public FloatProperty{
public:
    PROP_ALPHA():FloatProperty("alpha") {}
    void set(void*object,const AnimateValue& value)const {
        ((VectorDrawable::VectorDrawableState*)object)->setAlpha(GET_VARIANT(value,float));
    }
    AnimateValue get(void*object)const {
        return ((VectorDrawable::VectorDrawableState*)object)->getAlpha();
    }
};
class PROP_ALPHA DRAWABLESTATE_ALPHA;
}
const FloatProperty& VectorDrawable::VectorDrawableState::ALPHA= DRAWABLESTATE_ALPHA;

const Property* VectorDrawable::VectorDrawableState::getProperty(const std::string& propertyName) {
    if (ALPHA.getName().compare(propertyName)==0) {
        return &ALPHA;
    }
    return nullptr;
}

// If copy is not null, deep copy the given VectorDrawableState. Otherwise, create a
// native vector drawable tree with an empty root group.
VectorDrawable::VectorDrawableState::VectorDrawableState(const VectorDrawableState* copy) {
    mTint = nullptr;
    mNativeTree = nullptr;
    mCachedTint = nullptr;
    mCacheDirty = false;
    mRootGroup = nullptr;
    mCachedAutoMirrored = false;
    if (copy != nullptr) {
        //mThemeAttrs = copy->mThemeAttrs;
        mChangingConfigurations = copy->mChangingConfigurations;
        mTint = copy->mTint;
        mTintMode = copy->mTintMode;
        mAutoMirrored = copy->mAutoMirrored;
        mRootGroup = new VGroup(copy->mRootGroup, mVGTargetsMap);
        createNativeTreeFromCopy(copy, mRootGroup);

        mBaseWidth = copy->mBaseWidth;
        mBaseHeight = copy->mBaseHeight;
        setViewportSize(copy->mViewportWidth, copy->mViewportHeight);
        mOpticalInsets = copy->mOpticalInsets;

        mRootName = copy->mRootName;
        mDensity = copy->mDensity;
        if (!copy->mRootName.empty()) {
            //mVGTargetsMap.emplace(copy->mRootName, this);
        }
    } else {
        mRootGroup = new VGroup();
        createNativeTree(mRootGroup);
    }
    onTreeConstructionFinished();
}

void VectorDrawable::VectorDrawableState::createNativeTree(VGroup* rootGroup) {
   // mNativeTree = new VirtualRefBasePtr(nCreateTree(rootGroup->mNativePtr));
   mNativeTree = new hwui::Tree(rootGroup->mNativePtr);
}

// Create a new native tree with the given root group, and copy the properties from the
// given VectorDrawableState's native tree.
void VectorDrawable::VectorDrawableState::createNativeTreeFromCopy(const VectorDrawableState* copy, VGroup* rootGroup) {
    //mNativeTree = new VirtualRefBasePtr(nCreateTreeFromCopy(copy->mNativeTree.get(), rootGroup->mNativePtr));
    mNativeTree = new hwui::Tree(copy->mNativeTree, rootGroup->mNativePtr);
}

// This should be called every time after a new RootGroup and all its subtrees are created
// (i.e. in constructors of VectorDrawableState and in inflate).
void VectorDrawable::VectorDrawableState::onTreeConstructionFinished() {
    mRootGroup->setTree(mNativeTree);
}

long VectorDrawable::VectorDrawableState::getNativeRenderer() {
    if (mNativeTree == nullptr) {
        return 0;
    }
    return (long)mNativeTree;
}

bool VectorDrawable::VectorDrawableState::canReuseCache() {
    if (!mCacheDirty && (memcmp(mCachedThemeAttrs,mThemeAttrs,sizeof(int)*2)==0)
            && (mCachedTint == mTint) && (mCachedTintMode == mTintMode)
            && (mCachedAutoMirrored == mAutoMirrored) ) {
        return true;
    }
    updateCacheStates();
    return false;
}

void VectorDrawable::VectorDrawableState::updateCacheStates() {
    // Use shallow copy here and shallow comparison in canReuseCache(),
    // likely hit cache miss more, but practically not much difference.
    //mCachedThemeAttrs = mThemeAttrs;
    mCachedTint = mTint;
    //mCachedTintMode = mTintMode;
    mCachedAutoMirrored = mAutoMirrored;
    mCacheDirty = false;
}

void VectorDrawable::VectorDrawableState::applyTheme(Theme t) {
    mRootGroup->applyTheme(t);
}

bool VectorDrawable::VectorDrawableState::canApplyTheme() {
    return /*mThemeAttrs != nullptr
            || (mRootGroup != nullptr && mRootGroup->canApplyTheme())
            || (mTint != nullptr && mTint->canApplyTheme())
            || super.canApplyTheme();*/false;
}

Drawable* VectorDrawable::VectorDrawableState::newDrawable() {
    return new VectorDrawable(shared_from_this());
}

int VectorDrawable::VectorDrawableState::getChangingConfigurations() const{
    return mChangingConfigurations
            | (mTint != nullptr ? mTint->getChangingConfigurations() : 0);
}

bool VectorDrawable::VectorDrawableState::isStateful() const{
    return (mTint != nullptr && mTint->isStateful())
            || (mRootGroup != nullptr && mRootGroup->isStateful());
}

bool VectorDrawable::VectorDrawableState::hasFocusStateSpecified()const {
    return mTint != nullptr && mTint->hasFocusStateSpecified()
            || (mRootGroup != nullptr && mRootGroup->hasFocusStateSpecified());
}

void VectorDrawable::VectorDrawableState::setViewportSize(float viewportWidth, float viewportHeight) {
    mViewportWidth = viewportWidth;
    mViewportHeight = viewportHeight;
    mNativeTree->mutateStagingProperties()->setViewportSize(viewportWidth, viewportHeight);
}

bool VectorDrawable::VectorDrawableState::setDensity(int targetDensity) {
    if (mDensity != targetDensity) {
        const int sourceDensity = mDensity;
        mDensity = targetDensity;
        applyDensityScaling(sourceDensity, targetDensity);
        return true;
    }
    return false;
}

void VectorDrawable::VectorDrawableState::applyDensityScaling(int sourceDensity, int targetDensity) {
    mBaseWidth = Drawable::scaleFromDensity(mBaseWidth, sourceDensity, targetDensity, true);
    mBaseHeight = Drawable::scaleFromDensity(mBaseHeight, sourceDensity, targetDensity,true);

    const int insetLeft = Drawable::scaleFromDensity(mOpticalInsets.left, sourceDensity, targetDensity, false);
    const int insetTop = Drawable::scaleFromDensity(mOpticalInsets.top, sourceDensity, targetDensity, false);
    const int insetRight = Drawable::scaleFromDensity(mOpticalInsets.right, sourceDensity, targetDensity, false);
    const int insetBottom = Drawable::scaleFromDensity(mOpticalInsets.bottom, sourceDensity, targetDensity, false);
    mOpticalInsets = Insets::of(insetLeft, insetTop, insetRight, insetBottom);
}

bool VectorDrawable::VectorDrawableState::onStateChange(const std::vector<int>& stateSet) {
    return mRootGroup->onStateChange(stateSet);
}

VectorDrawable::VectorDrawableState::~VectorDrawableState(){
    int bitmapCacheSize = mLastHWCachePixelCount * 4 + mLastSWCachePixelCount * 4;
    delete mRootGroup;
    delete mNativeTree;
}

/**
 * setAlpha() and getAlpha() are used mostly for animation purpose. Return true if alpha
 * has changed.
 */
bool VectorDrawable::VectorDrawableState::setAlpha(float alpha) {
    hwui::Tree*tree = (hwui::Tree*)mNativeTree;
    return tree->mutateStagingProperties()->setRootAlpha(alpha);
}

float VectorDrawable::VectorDrawableState::getAlpha() {
    hwui::Tree*tree = (hwui::Tree*)mNativeTree;
    return tree->stagingProperties().getRootAlpha();
}

//////////////////////////////////////////////////////////////////////////////////////////////
//static class VGroup extends VObject {
#define DEFINE_FLOATPROPERTY(CLASS,PROPNAME, METHOD, PROJ)  \
namespace {                                                 \
class prop_##PROJ : public FloatProperty {                  \
public:                                                     \
    prop_##PROJ() : FloatProperty(PROPNAME) {}              \
    void set(void*obj,const AnimateValue& v)const override{ \
        ((CLASS*)obj)->set##METHOD(GET_VARIANT(v,float));   \
    }                                                       \
    AnimateValue get(void*obj)const override                \
            { return ((CLASS*)obj)->get##METHOD(); }        \
};                                                          \
static const prop_##PROJ INST_##PROJ;                       \
}const FloatProperty*const CLASS::PROJ = &INST_##PROJ;

#define DEFINE_INTPROPERTY(CLASS,PROPNAME, METHOD, PROJ)    \
namespace {                                                 \
class prop_##PROJ : public Property {                       \
public:                                                     \
    prop_##PROJ() : Property(PROPNAME,INT_TYPE) {}          \
    void set(void*obj,const AnimateValue& v)const override{ \
        ((CLASS*)obj)->set##METHOD(GET_VARIANT(v,int));     \
    }                                                       \
    AnimateValue get(void*obj)const override                \
            { return ((CLASS*)obj)->get##METHOD(); }        \
};                                                          \
static const prop_##PROJ INTPROP_##PROJ;                    \
}const Property*const CLASS::PROJ = &INTPROP_##PROJ;

DEFINE_FLOATPROPERTY(VectorDrawable::VGroup,"translateX",TranslateX,TRANSLATE_X);
DEFINE_FLOATPROPERTY(VectorDrawable::VGroup,"translateY",TranslateY,TRANSLATE_Y);
DEFINE_FLOATPROPERTY(VectorDrawable::VGroup,"scaleX",ScaleX,SCALE_X);
DEFINE_FLOATPROPERTY(VectorDrawable::VGroup,"scaleY",ScaleY,SCALE_Y);
DEFINE_FLOATPROPERTY(VectorDrawable::VGroup,"pivotX",PivotX,PIVOT_X);
DEFINE_FLOATPROPERTY(VectorDrawable::VGroup,"pivotY",PivotY,PIVOT_Y);
DEFINE_FLOATPROPERTY(VectorDrawable::VGroup,"rotation",Rotation,ROTATION);

std::unordered_map<std::string, int> VectorDrawable::VGroup::sPropertyIndexMap ={
    {"translateX", (int)TRANSLATE_X_INDEX},
    {"translateY", (int)TRANSLATE_Y_INDEX},
    {"scaleX", (int)SCALE_X_INDEX},
    {"scaleY", (int)SCALE_Y_INDEX},
    {"pivotX", (int)PIVOT_X_INDEX},
    {"pivotY", (int)PIVOT_Y_INDEX},
    {"rotation", (int)ROTATION_INDEX}
};

int VectorDrawable::VGroup::getPropertyIndex(const std::string& propertyName) {
    auto it = sPropertyIndexMap.find(propertyName);
    if (it!=sPropertyIndexMap.end()) {
        return it->second;
    } else {
        // property not found
        return -1;
    }
}


// Below are the Properties that wrap the setters to avoid reflection overhead in animations
const std::unordered_map<std::string, const Property*> VectorDrawable::VGroup::sPropertyMap ={
    {"translateX", TRANSLATE_X},
    {"translateY", TRANSLATE_Y},
    {"scaleX", SCALE_X},
    {"scaleY", SCALE_Y},
    {"pivotX", PIVOT_X},
    {"pivotY", PIVOT_Y},
    {"rotation", ROTATION}
};

VectorDrawable::VGroup::VGroup() {
    mIsStateful = false;
    //mNativePtr = nCreateGroup();
    mNativePtr = new hwui::Group();
}

VectorDrawable::VGroup::~VGroup(){
    for(auto child:mChildren){
        delete child;
    }
    delete mNativePtr;
}

// Temp array to store transform values obtained from native.
VectorDrawable::VGroup::VGroup(const VGroup* copy,std::unordered_map<std::string, void*>& targetsMap) {

    mIsStateful = copy->mIsStateful;
    //mThemeAttrs = copy->mThemeAttrs;
    mGroupName = copy->mGroupName;
    mChangingConfigurations = copy->mChangingConfigurations;
    if (!mGroupName.empty()) {
        targetsMap.emplace(mGroupName, this);
    }
    //nCreateGroup((long)copy->mNativePtr);
    mNativePtr = new hwui::Group(*copy->mNativePtr);

    const std::vector<VObject*> children = copy->mChildren;
    for (int i = 0; i < children.size(); i++) {
        VObject* copyChild = children.at(i);
        if (dynamic_cast<VGroup*>(copyChild)) {
            VGroup* copyGroup = (VGroup*) copyChild;
            addChild(new VGroup(copyGroup, targetsMap));
        } else {
            VPath* newPath;
            if (dynamic_cast<VFullPath*>(copyChild)) {
                newPath = new VFullPath((VFullPath*) copyChild);
            } else if (dynamic_cast<VClipPath*>(copyChild)) {
                newPath = new VClipPath((VClipPath*)copyChild);
            } else {
                LOGE("Unknown object in the tree!");
            }
            addChild(newPath);
            if (!newPath->mPathName.empty()) {
                targetsMap.emplace(newPath->mPathName, newPath);
            }
        }
    }
}

const Property* VectorDrawable::VGroup::getProperty(const std::string& propertyName) {
    auto it=sPropertyMap.find(propertyName);
    if (it!=sPropertyMap.end()) {
        return it->second;
    } else {
        // property not found
        return nullptr;
    }
}

std::string VectorDrawable::VGroup::getGroupName()const{
    return mGroupName;
}

void VectorDrawable::VGroup::addChild(VObject* child) {
    mIsStateful =false;
    //nAddChild(mNativePtr, child->getNativePtr());
    hwui::Group*group = mNativePtr;
    hwui::Node*childNode = (hwui::Node*)child->getNativePtr();
    group->addChild(childNode);
    mChildren.push_back(child);
    mIsStateful |= child->isStateful();
}

void VectorDrawable::VGroup::setTree(hwui::Tree* treeRoot) {
    VObject::setTree(treeRoot);
    for (int i = 0; i < mChildren.size(); i++) {
        mChildren.at(i)->setTree(treeRoot);
    }
}

long VectorDrawable::VGroup::getNativePtr() {
    return (long)mNativePtr;
}


void VectorDrawable::VGroup::inflate(XmlPullParser&parser,const AttributeSet&atts) {
    const auto properties=mNativePtr->stagingProperties();
    float rotate = atts.getFloat("rotation",properties->getRotation());
    float pivotX = atts.getFloat("pivotX",properties->getPivotX());
    float pivotY = atts.getFloat("pivotY",properties->getPivotY());
    float scaleX = atts.getFloat("scaleX",properties->getScaleX());
    float scaleY = atts.getFloat("scaleY",properties->getScaleY());
    float translateX = atts.getFloat("translateX",properties->getTranslateX());
    float translateY = atts.getFloat("translateY",properties->getTranslateY());
    mGroupName = atts.getString("name");
    if (!mGroupName.empty()) {
        //nSetName(mNativePtr, mGroupName);
        mNativePtr->setName(mGroupName.c_str());
    }
    //nUpdateGroupProperties(mNativePtr, rotate, pivotX, pivotY, scaleX, scaleY,translateX, translateY);
    mNativePtr->mutateStagingProperties()->updateProperties(rotate,pivotX,pivotY,scaleX,scaleY,translateX,translateY);
}

bool VectorDrawable::VGroup::onStateChange(const std::vector<int>& stateSet) {
    bool changed = false;

    std::vector<VObject*>& children = mChildren;
    for (int i = 0, count = children.size(); i < count; i++) {
        VObject* child = children.at(i);
        if (child->isStateful()) {
            changed |= child->onStateChange(stateSet);
        }
    }

    return changed;
}

bool VectorDrawable::VGroup::isStateful()const {
    return mIsStateful;
}

bool VectorDrawable::VGroup::hasFocusStateSpecified() const{
    bool result = false;

    const std::vector<VObject*>& children = mChildren;
    for (int i = 0, count = children.size(); i < count; i++) {
        VObject* child = children.at(i);
        if (child->isStateful()) {
            result |= child->hasFocusStateSpecified();
        }
    }

    return result;
}

bool VectorDrawable::VGroup::canApplyTheme() {
#ifndef __clang__
    if (mThemeAttrs != nullptr)
#endif
    {
        return true;
    }

    std::vector<VObject*> children = mChildren;
    for (int i = 0, count = children.size(); i < count; i++) {
        VObject* child = children.at(i);
        if (child->canApplyTheme()) {
            return true;
        }
    }

    return false;
}

void VectorDrawable::VGroup::applyTheme(Theme t) {
    /*if (mThemeAttrs != null) {
        final TypedArray a = t.resolveAttributes(mThemeAttrs,R.styleable.VectorDrawableGroup);
        updateStateFromTypedArray(a);
        a.recycle();
    }*/

    std::vector<VObject*>& children = mChildren;
    for (int i = 0, count = children.size(); i < count; i++) {
        VObject* child = children.at(i);
        if (child->canApplyTheme()) {
            child->applyTheme(t);

            // Applying a theme may have made the child stateful.
            mIsStateful |= child->isStateful();
        }
    }
}

/* Setters and Getters, used by animator from AnimatedVectorDrawable. */
float VectorDrawable::VGroup::getRotation() {
    return isTreeValid() ? mNativePtr->stagingProperties()->getRotation():0;
}

void VectorDrawable::VGroup::setRotation(float rotation) {
    if (isTreeValid()) {
        //nSetRotation(mNativePtr, rotation);
        mNativePtr->mutateStagingProperties()->setRotation(rotation);
    }
}

float VectorDrawable::VGroup::getPivotX() {
    return isTreeValid() ? mNativePtr->stagingProperties()->getPivotX():0;
}

void VectorDrawable::VGroup::setPivotX(float pivotX) {
    if (isTreeValid()) {
        //nSetPivotX(mNativePtr, pivotX);
        mNativePtr->mutateStagingProperties()->setPivotX(pivotX);
    }
}

float VectorDrawable::VGroup::getPivotY() {
    return isTreeValid() ? mNativePtr->stagingProperties()->getPivotY():0;
}

void VectorDrawable::VGroup::setPivotY(float pivotY) {
    if (isTreeValid()) {
        //nSetPivotY(mNativePtr, pivotY);
        mNativePtr->mutateStagingProperties()->setPivotY(pivotY);
    }
}

float VectorDrawable::VGroup::getScaleX() {
    return isTreeValid() ? mNativePtr->stagingProperties()->getScaleX():0;
}

void VectorDrawable::VGroup::setScaleX(float scaleX) {
    if (isTreeValid()) {
        //nSetScaleX(mNativePtr, scaleX);
        mNativePtr->mutateStagingProperties()->setScaleX(scaleX);
    }
}

float VectorDrawable::VGroup::getScaleY() {
    return isTreeValid() ? mNativePtr->stagingProperties()->getScaleY():0;
}

void VectorDrawable::VGroup::setScaleY(float scaleY) {
    if (isTreeValid()) {
        //nSetScaleY(mNativePtr, scaleY);
        mNativePtr->mutateStagingProperties()->setScaleY(scaleY);
    }
}

float VectorDrawable::VGroup::getTranslateX() {
    return isTreeValid() ? mNativePtr->stagingProperties()->getTranslateX():0;
}

void VectorDrawable::VGroup::setTranslateX(float translateX) {
    if (isTreeValid()) {
        //nSetTranslateX(mNativePtr, translateX);
        mNativePtr->mutateStagingProperties()->setTranslateX(translateX);
    }
}

float VectorDrawable::VGroup::getTranslateY() {
    return isTreeValid() ? mNativePtr->stagingProperties()->getTranslateY():0;
}

void VectorDrawable::VGroup::setTranslateY(float translateY) {
    if (isTreeValid()) {
        //nSetTranslateY(mNativePtr, translateY);
        mNativePtr->mutateStagingProperties()->setTranslateY(translateY);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Common Path information for clip path and normal path.
 *static abstract class VPath extends VObject*/
namespace {
class PROP_PATH_DATA:public Property{
public:
    PROP_PATH_DATA():Property("pathData",PATH_TYPE){}
    void set(void* object,const AnimateValue& data)const override{
        PathParser::PathData pathData =GET_VARIANT(data,PathParser::PathData);
        ((VectorDrawable::VPath*)object)->setPathData(&pathData);
    }
    AnimateValue get(void* object)const override{
        return *((VectorDrawable::VPath*)object)->getPathData();
    }
};
static class PROP_PATH_DATA VPATH_PATH_DATA;
}
const Property*const VectorDrawable::VPath::PATH_DATA= &VPATH_PATH_DATA;

const Property* VectorDrawable::VPath::getProperty(const std::string& propertyName) {
    if (PATH_DATA->getName().compare(propertyName)==0) {
        return PATH_DATA;
    }
    // property not found
    return nullptr;
}

VectorDrawable::VPath::VPath() {
    // Empty constructor.
}

VectorDrawable::VPath::VPath(const VPath* copy) {
    mPathName = copy->mPathName;
    mChangingConfigurations = copy->mChangingConfigurations;
    mPathData = (copy->mPathData == nullptr) ? nullptr : new PathParser::PathData(*copy->mPathData);
}

VectorDrawable::VPath::~VPath(){
    delete mPathData;
}

std::string VectorDrawable::VPath::getPathName() const{
    return mPathName;
}


/* Setters and Getters, used by animator from AnimatedVectorDrawable. */
PathParser::PathData* VectorDrawable::VPath::getPathData() {
    return mPathData;
}

// TODO: Move the PathEvaluator and this setter and the getter above into native.
void VectorDrawable::VPath::setPathData(const PathParser::PathData* pathData) {
    mPathData->setPathData(*pathData);
    if (isTreeValid()) {
        //nSetPathData(getNativePtr(), mPathData->getNativePtr());
        hwui::Path*dst = (hwui::Path*)getNativePtr();
        hwui::PathData*src = (hwui::PathData*)mPathData->getNativePtr();
        dst->mutateStagingProperties()->setData(*src);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Clip path, which only has name and pathData.
 */
//static class VClipPath extends VPath {
    
VectorDrawable::VClipPath::VClipPath() {
    mNativePtr = new hwui::ClipPath();//nCreateClipPath();
}

VectorDrawable::VClipPath::VClipPath(const VClipPath* copy)
    :VPath(copy){
    //nCreateClipPath(copy->mNativePtr);
    mNativePtr = new hwui::ClipPath(*copy->mNativePtr);
}

VectorDrawable::VClipPath::~VClipPath(){
    delete mNativePtr;
}

long VectorDrawable::VClipPath::getNativePtr() {
    return (long)mNativePtr;
}


void VectorDrawable::VClipPath::inflate(XmlPullParser&,const AttributeSet& attrs) {
    updateStateFromTypedArray(attrs);
}

bool VectorDrawable::VClipPath::canApplyTheme() {
    return false;
}

void VectorDrawable::VClipPath::applyTheme(Theme theme) {
    // No-op.
}

bool VectorDrawable::VClipPath::onStateChange(const std::vector<int>& stateSet) {
    return false;
}

bool VectorDrawable::VClipPath::isStateful() const{
    return false;
}

bool VectorDrawable::VClipPath::hasFocusStateSpecified()const {
    return false;
}

void VectorDrawable::VClipPath::updateStateFromTypedArray(const AttributeSet&atts) {
    // Account for any configuration changes.
    mChangingConfigurations =0;//|= a.getChangingConfigurations();

    const std::string pathName = atts.getString("name");
    if (!pathName.empty()) {
        mPathName = pathName;
        //nSetName(mNativePtr, mPathName);
        mNativePtr->setName(mPathName.c_str());
    }

    const std::string pathDataString = atts.getString("pathData");
    if (!pathDataString.empty()) {
        mPathData = new PathParser::PathData(pathDataString);
        //nSetPathString(mNativePtr, pathDataString, pathDataString.length());
        hwui::PathData data;
        hwui::PathParser::ParseResult result;
        hwui::PathParser::getPathDataFromAsciiString(&data, &result, pathDataString.c_str(), pathDataString.length());
        ((hwui::Path*)mNativePtr)->mutateStagingProperties()->setData(data);
        //mNativePtr->mutateStagingProperties()->setData(pathDataString);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Normal path, which contains all the fill / paint information.
 */
DEFINE_FLOATPROPERTY(VectorDrawable::VFullPath,"strokeWidth",StrokeWidth,STROKE_WIDTH);
DEFINE_INTPROPERTY(VectorDrawable::VFullPath,"strokeColor",StrokeColor,STROKE_COLOR);
DEFINE_FLOATPROPERTY(VectorDrawable::VFullPath,"strokeAlpha",StrokeAlpha,STROKE_ALPHA);

DEFINE_INTPROPERTY(VectorDrawable::VFullPath,"fillColor",FillColor,FILL_COLOR);
DEFINE_FLOATPROPERTY(VectorDrawable::VFullPath,"fillAlpha",FillAlpha,FILL_ALPHA);

DEFINE_FLOATPROPERTY(VectorDrawable::VFullPath,"trimPathStart",TrimPathStart,TRIM_PATH_START);
DEFINE_FLOATPROPERTY(VectorDrawable::VFullPath,"trimPathEnd",TrimPathEnd,TRIM_PATH_END);
DEFINE_FLOATPROPERTY(VectorDrawable::VFullPath,"trimPathOffset",TrimPathOffset,TRIM_PATH_OFFSET);

//static class VFullPath extends VPath
// Property map for animatable attributes.
std::unordered_map<std::string, int> VectorDrawable::VFullPath::sPropertyIndexMap={
      {"strokeWidth", (int)STROKE_WIDTH_INDEX},
      {"strokeColor", (int)STROKE_COLOR_INDEX},
      {"strokeAlpha", (int)STROKE_ALPHA_INDEX},
      {"fillColor", (int)FILL_COLOR_INDEX},
      {"fillAlpha", (int)FILL_ALPHA_INDEX},
      {"trimPathStart", (int)TRIM_PATH_START_INDEX},
      {"trimPathEnd", (int)TRIM_PATH_END_INDEX},
      {"trimPathOffset", (int)TRIM_PATH_OFFSET_INDEX}
};

// Below are the Properties that wrap the setters to avoid reflection overhead in animations

const std::unordered_map<std::string, const Property*> VectorDrawable::VFullPath::sPropertyMap={
    {"strokeWidth", STROKE_WIDTH},
    {"strokeColor", STROKE_COLOR},
    {"strokeAlpha", STROKE_ALPHA},
    {"fillColor", FILL_COLOR},
    {"fillAlpha", FILL_ALPHA},
    {"trimPathStart", TRIM_PATH_START},
    {"trimPathEnd", TRIM_PATH_END},
    {"trimPathOffset", TRIM_PATH_OFFSET}
};

VectorDrawable::VFullPath::VFullPath() {
    mNativePtr = new hwui::FullPath();//nCreateFullPath();
}

VectorDrawable::VFullPath::VFullPath(const VFullPath* copy):VPath(copy){
    mNativePtr = new hwui::FullPath(*copy->mNativePtr);//nCreateFullPath(copy->mNativePtr);
    //mThemeAttrs = copy->mThemeAttrs;
    mStrokeColors = copy->mStrokeColors;
    mFillColors = copy->mFillColors;
}

VectorDrawable::VFullPath::~VFullPath(){
    delete mNativePtr;
}

const Property* VectorDrawable::VFullPath::getProperty(const std::string& propertyName) {
    const Property* p = VPath::getProperty(propertyName);
    if (p != nullptr) {
        return p;
    }
    auto it =sPropertyMap.find(propertyName);
    if (it!=sPropertyMap.end()) {
        return it->second;
    } else {
        // property not found
        return nullptr;
    }
}

int VectorDrawable::VFullPath::getPropertyIndex(const std::string& propertyName) {
    auto it = sPropertyIndexMap.find(propertyName);
    if (it==sPropertyIndexMap.end()) {
        return -1;
    } else {
        return it->second;
    }
}

bool VectorDrawable::VFullPath::onStateChange(const std::vector<int>&stateSet) {
    bool changed = false;

    if (mStrokeColors != nullptr && dynamic_cast<ColorStateList*>(mStrokeColors)) {
        const int oldStrokeColor = getStrokeColor();
        const int newStrokeColor =((ColorStateList*) mStrokeColors)->getColorForState(stateSet, oldStrokeColor);
        changed |= oldStrokeColor != newStrokeColor;
        if (oldStrokeColor != newStrokeColor) {
            //nSetStrokeColor(mNativePtr, newStrokeColor);
            mNativePtr->mutateStagingProperties()->setStrokeColor(newStrokeColor);
        }
    }

    if (mFillColors != nullptr && dynamic_cast<ColorStateList*>(mFillColors)) {
        const int oldFillColor = getFillColor();
        const int newFillColor = ((ColorStateList*) mFillColors)->getColorForState(stateSet, oldFillColor);
        changed |= oldFillColor != newFillColor;
        if (oldFillColor != newFillColor) {
            //nSetFillColor(mNativePtr, newFillColor);
            mNativePtr->mutateStagingProperties()->setFillColor(newFillColor);
        }
    }

    return changed;
}

bool VectorDrawable::VFullPath::isStateful() const{
    return mStrokeColors != nullptr || mFillColors != nullptr;
}

bool VectorDrawable::VFullPath::hasFocusStateSpecified() const{
    return (mStrokeColors != nullptr && dynamic_cast<ColorStateList*>(mStrokeColors) &&
            ((ColorStateList*) mStrokeColors)->hasFocusStateSpecified()) &&
            (mFillColors != nullptr && dynamic_cast<ColorStateList*>(mFillColors) &&
            ((ColorStateList*) mFillColors)->hasFocusStateSpecified());
}

long VectorDrawable::VFullPath::getNativePtr() {
    return (long)mNativePtr;
}

void VectorDrawable::VFullPath::inflate(XmlPullParser&parser,const AttributeSet& attrs) {
    /*final TypedArray a = obtainAttributes(r, theme, attrs,R.styleable.VectorDrawablePath);
    updateStateFromTypedArray(a);
    a.recycle();*/
    updateStateFromTypedArray(attrs);
    inflateGradients(parser,attrs);
}

void VectorDrawable::VFullPath::updateStateFromTypedArray(const AttributeSet& atts) {
    auto properties = mNativePtr->stagingProperties();
    float strokeWidth = properties->getStrokeWidth();
    int strokeColor = properties->getStrokeColor();
    float strokeAlpha = properties->getStrokeAlpha();
    int fillColor =  properties->getFillColor();
    float fillAlpha = properties->getFillAlpha();
    float trimPathStart = properties->getTrimPathStart();
    float trimPathEnd = properties->getTrimPathEnd();
    float trimPathOffset = properties->getTrimPathOffset();
    int strokeLineCap =  properties->getStrokeLineCap();
    int strokeLineJoin = properties->getStrokeLineJoin();
    float strokeMiterLimit = properties->getStrokeMiterLimit();
    int fillType = properties->getFillType();
    //Shader fillGradient = null;
    //Shader strokeGradient = null;
    // Account for any configuration changes.
    mChangingConfigurations = 0;//!=atts.getChangingConfigurations();

    // Extract the theme attributes, if any.
    //mThemeAttrs = a.extractThemeAttrs();

    const std::string pathName = atts.getString("name");
    if (!pathName.empty()) {
        mPathName = pathName;
        //nSetName(mNativePtr, mPathName);
        mNativePtr->setName(mPathName.c_str());
    }

    const std::string pathString = atts.getString("pathData");
    if (!pathString.empty()) {
        mPathData = new PathParser::PathData(pathString);
        //nSetPathString(mNativePtr, pathString, pathString.length());
        hwui::PathData data;
        hwui::PathParser::ParseResult result;
        hwui::PathParser::getPathDataFromAsciiString(&data, &result, pathString.c_str(), pathString.length());
        ((hwui::Path*)mNativePtr)->mutateStagingProperties()->setData(data);
    }
#if 10
    ComplexColor* fillColors = atts.getColorStateList("fillColor");
    if (fillColors != nullptr) {
        // If the colors is a gradient color, or the color state list is stateful, keep the
        // colors information. Otherwise, discard the colors and keep the default color.
        /*if (fillColors instanceof  GradientColor) {
            mFillColors = fillColors;
            fillGradient = ((GradientColor) fillColors).getShader();
        } else */if (fillColors->isStateful()) {
            mFillColors = fillColors;
        } else {
            mFillColors = nullptr;
        }
        fillColor = fillColors->getDefaultColor();
    }

    ComplexColor* strokeColors = atts.getColorStateList("strokeColor");
    if (strokeColors != nullptr) {
        // If the colors is a gradient color, or the color state list is stateful, keep the
        // colors information. Otherwise, discard the colors and keep the default color.
        /*if (strokeColors instanceof GradientColor) {
            mStrokeColors = strokeColors;
            strokeGradient = ((GradientColor) strokeColors).getShader();
        } else */if (strokeColors->isStateful()) {
            mStrokeColors = strokeColors;
        } else {
            mStrokeColors = nullptr;
        }
        strokeColor = strokeColors->getDefaultColor();
    }

    // Update the gradient info, even if the gradiet is null.
    //nUpdateFullPathFillGradient(mNativePtr,fillGradient != nullptr ? fillGradient.getNativeInstance() : 0);
    //nUpdateFullPathStrokeGradient(mNativePtr,strokeGradient != nullptr ? strokeGradient.getNativeInstance() : 0);
    //mNativePtr->mutateStagingProperties()->setFillGradient(fillGradient);
    //mNativePtr->mutateStagingProperties()->setStrokeGradient(strokeGradient);
    //LOGD("path %p gradient=%p,%p",this,mStrokeGradient.get(),mFillGradient.get());
#endif
    fillAlpha = atts.getFloat("fillAlpha", fillAlpha);
    strokeLineCap = atts.getInt("strokeLineCap",std::unordered_map<std::string,int>{
            {"butt", (int)Cairo::Context::LineCap::BUTT},
            {"round",(int)Cairo::Context::LineCap::ROUND},
            {"square",(int)Cairo::Context::LineCap::SQUARE} }, strokeLineCap);
    strokeLineJoin = atts.getInt("strokeLineJoin",std::unordered_map<std::string,int>{
            {"bevel",(int)Cairo::Context::LineJoin::BEVEL},
            {"miter",(int)Cairo::Context::LineJoin::MITER}, 
            {"round",(int)Cairo::Context::LineJoin::ROUND} }, strokeLineJoin);
    strokeMiterLimit = atts.getFloat("strokeMiterLimit", strokeMiterLimit);
    strokeAlpha = atts.getFloat("strokeAlpha",strokeAlpha);
    strokeWidth = atts.getFloat("strokeWidth",strokeWidth);
    trimPathEnd = atts.getFloat("trimPathEnd",trimPathEnd);
    trimPathOffset = atts.getFloat("trimPathOffset", trimPathOffset);
    trimPathStart = atts.getFloat("trimPathStart", trimPathStart);
    fillType = atts.getInt("fillType",std::unordered_map<std::string,int>{
            {"evenOdd",(int)Cairo::Context::FillRule::EVEN_ODD},
            {"nonZero",(int)Cairo::Context::FillRule::WINDING} }, fillType);

    //nUpdateFullPathProperties(
    mNativePtr->mutateStagingProperties()->updateProperties(strokeWidth, strokeColor, strokeAlpha,
            fillColor, fillAlpha, trimPathStart, trimPathEnd, trimPathOffset,
            strokeMiterLimit, strokeLineCap, strokeLineJoin, fillType);
}

void VectorDrawable::VFullPath::inflateGradients(XmlPullParser&parser,const AttributeSet&atts){
    int eventType,gradientType,strokeFill=-1;
    const int innerDepth = parser.getDepth();
    Cairo::RefPtr<Cairo::Gradient>gradient;
    // Parse everything until the end of the vector element.
    while (((eventType =parser.next())!= XmlPullParser::END_DOCUMENT)
            && (parser.getDepth() >= innerDepth || eventType != XmlPullParser::END_TAG)) {
        std::string tagName = parser.getName();
        if ((eventType==XmlPullParser::END_TAG)&&tagName.compare("gradient")==0){
            if(strokeFill==0)mNativePtr->mutateStagingProperties()->setStrokeGradient(gradient);
            else mNativePtr->mutateStagingProperties()->setFillGradient(gradient);
            LOGV("===add gradient %p to %s",gradient.get(),(gradientType?"fill":"stroke"));
        }
        if(tagName.compare("path")==0)break;
        if (eventType != XmlPullParser::START_TAG)continue;
        if(tagName.find("attr")!=std::string::npos){
            const std::string name = atts.getString("name");
            LOGV("tag=%s name=%s depth=%d/%d",tagName.c_str(),name.c_str(),innerDepth,parser.getDepth());
            strokeFill=(name.find("stroke")!=std::string::npos)?0:1;
        }
        if(tagName.compare("gradient")==0){
            float centerX,centerY,radius;
            gradientType = atts.getInt("type",std::unordered_map<std::string,int>{
                    {"linear",0},{"radial",1},{"sweep",2}},0);
            switch(gradientType){
            case 0:
                gradient = Cairo::LinearGradient::create(
                        atts.getFloat("startX",0), atts.getFloat("startY",0),
                        atts.getFloat("endX",0) , atts.getFloat("endY",0));
                break;
            case 1:
                centerX= atts.getFloat("centerX",0);
                centerY= atts.getFloat("centerY",0);
                radius = atts.getFloat("gradientRadius",0);
                gradient=Cairo::RadialGradient::create(centerX,centerY,0,centerX,centerY,radius);
                break;
            case 2:
            default:LOGD("TODO: GradientType=%s",atts.getString("type").c_str());break;
            }
        }
        if(tagName.compare("item")==0){
            const float offset = atts.getFloat("offset",0.f);
            const uint32_t color =atts.getColor("color",0);
            Color c(color);
            LOGV("gradient %p %.2f colorstop=%x",gradient.get(),offset,color);
            gradient->add_color_stop_rgba(offset,c.red(),c.green(),c.blue(),c.alpha());
        }
    }
}

bool VectorDrawable::VFullPath::canApplyTheme() {
#ifndef __clang__
    if (mThemeAttrs != nullptr)
#endif
    {
        return true;
    }

    const bool fillCanApplyTheme = canComplexColorApplyTheme(mFillColors);
    const bool strokeCanApplyTheme = canComplexColorApplyTheme(mStrokeColors);
    if (fillCanApplyTheme || strokeCanApplyTheme) {
        return true;
    }
    return false;

}

void VectorDrawable::VFullPath::applyTheme(Theme t) {
    // Resolve the theme attributes directly referred by the VectorDrawable.
#if 0
    if (mThemeAttrs != null) {
        final TypedArray a = t.resolveAttributes(mThemeAttrs, R.styleable.VectorDrawablePath);
        updateStateFromTypedArray(a);
        a.recycle();
    }

    // Resolve the theme attributes in-directly referred by the VectorDrawable, for example,
    // fillColor can refer to a color state list which itself needs to apply theme.
    // And this is the reason we still want to keep partial update for the path's properties.
    bool fillCanApplyTheme = canComplexColorApplyTheme(mFillColors);
    bool strokeCanApplyTheme = canComplexColorApplyTheme(mStrokeColors);

    if (fillCanApplyTheme) {
        mFillColors = mFillColors.obtainForTheme(t);
        if (mFillColors instanceof GradientColor) {
            nUpdateFullPathFillGradient(mNativePtr,
                    ((GradientColor) mFillColors).getShader().getNativeInstance());
        } else if (mFillColors instanceof ColorStateList) {
            nSetFillColor(mNativePtr, mFillColors.getDefaultColor());
        }
    }

    if (strokeCanApplyTheme) {
        mStrokeColors = mStrokeColors.obtainForTheme(t);
        if (mStrokeColors instanceof GradientColor) {
            nUpdateFullPathStrokeGradient(mNativePtr,
                    ((GradientColor) mStrokeColors).getShader().getNativeInstance());
        } else if (mStrokeColors instanceof ColorStateList) {
            nSetStrokeColor(mNativePtr, mStrokeColors.getDefaultColor());
        }
    }
#endif
}

bool VectorDrawable::VFullPath::canComplexColorApplyTheme(ComplexColor* complexColor) {
    return complexColor != nullptr ;//&& complexColor->canApplyTheme();
}

/* Setters and Getters, used by animator from AnimatedVectorDrawable. */
int VectorDrawable::VFullPath::getStrokeColor() {
    //return isTreeValid() ? nGetStrokeColor(mNativePtr) : 0;
    return isTreeValid()?mNativePtr->stagingProperties()->getStrokeColor():0;
}

void VectorDrawable::VFullPath::setStrokeColor(int strokeColor) {
    mStrokeColors = nullptr;
    if (isTreeValid()) {
        //nSetStrokeColor(mNativePtr, strokeColor);
        mNativePtr->mutateStagingProperties()->setStrokeColor(strokeColor);
    }
}

float VectorDrawable::VFullPath::getStrokeWidth() {
    //return isTreeValid() ? nGetStrokeWidth(mNativePtr) : 0;
    return isTreeValid()?mNativePtr->stagingProperties()->getStrokeWidth():0;
}

void VectorDrawable::VFullPath::setStrokeWidth(float strokeWidth) {
    if (isTreeValid()) {
        //nSetStrokeWidth(mNativePtr, strokeWidth);
        mNativePtr->mutateStagingProperties()->setStrokeWidth(strokeWidth);
    }
}

float VectorDrawable::VFullPath::getStrokeAlpha() {
    //return isTreeValid() ? nGetStrokeAlpha(mNativePtr) : 0;
    return isTreeValid()? mNativePtr->stagingProperties()->getStrokeAlpha():0;
}

void VectorDrawable::VFullPath::setStrokeAlpha(float strokeAlpha) {
    if (isTreeValid()) {
        //nSetStrokeAlpha(mNativePtr, strokeAlpha);
        mNativePtr->mutateStagingProperties()->setStrokeAlpha(strokeAlpha);
    }
}

int VectorDrawable::VFullPath::getFillColor() {
    //return isTreeValid() ? nGetFillColor(mNativePtr) : 0;
    return isTreeValid() ? mNativePtr->stagingProperties()->getFillColor():0;
}

void VectorDrawable::VFullPath::setFillColor(int fillColor) {
    mFillColors = nullptr;
    if (isTreeValid()) {
        //nSetFillColor(mNativePtr, fillColor);
        mNativePtr->mutateStagingProperties()->setFillColor(fillColor);
    }
}

float VectorDrawable::VFullPath::getFillAlpha() {
    //return isTreeValid() ? nGetFillAlpha(mNativePtr) : 0;
    return isTreeValid()?mNativePtr->stagingProperties()->getFillAlpha():0;
}

void VectorDrawable::VFullPath::setFillAlpha(float fillAlpha) {
    if (isTreeValid()) {
        //nSetFillAlpha(mNativePtr, fillAlpha);
        mNativePtr->mutateStagingProperties()->setFillAlpha(fillAlpha);
    }
}

float VectorDrawable::VFullPath::getTrimPathStart() {
    //return isTreeValid() ? nGetTrimPathStart(mNativePtr) : 0;
    return isTreeValid()?mNativePtr->stagingProperties()->getTrimPathStart():0;
}

void VectorDrawable::VFullPath::setTrimPathStart(float trimPathStart) {
    if (isTreeValid()) {
        //nSetTrimPathStart(mNativePtr, trimPathStart);
        mNativePtr->mutateStagingProperties()->setTrimPathStart(trimPathStart);
    }
}

float VectorDrawable::VFullPath::getTrimPathEnd() {
    //return isTreeValid() ? nGetTrimPathEnd(mNativePtr) : 0;
    return isTreeValid()?mNativePtr->stagingProperties()->getTrimPathEnd():0;
}

void VectorDrawable::VFullPath::setTrimPathEnd(float trimPathEnd) {
    if (isTreeValid()) {
        //nSetTrimPathEnd(mNativePtr, trimPathEnd);
        mNativePtr->mutateStagingProperties()->setTrimPathEnd(trimPathEnd);
    }
}

float VectorDrawable::VFullPath::getTrimPathOffset() {
    //return isTreeValid() ? nGetTrimPathOffset(mNativePtr) : 0;
    return isTreeValid()?mNativePtr->stagingProperties()->getTrimPathOffset():0;
}

void VectorDrawable::VFullPath::setTrimPathOffset(float trimPathOffset) {
    if (isTreeValid()) {
        //nSetTrimPathOffset(mNativePtr, trimPathOffset);
        mNativePtr->mutateStagingProperties()->setTrimPathOffset(trimPathOffset);
    }
}

}/*endof namespace*/
