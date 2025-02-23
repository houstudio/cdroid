#if 0
#include <drawables/vectordrawable.h>
namespace cdroid{

VectorDrawable::VectorDrawable()
    :VectorDrawable(std::make_shared<VectorDrawableState>(nullptr)){
}


VectorDrawable::VectorDrawable(std::shared_ptr<VectorDrawableState> state) {
    mVectorState = state;
    //updateLocalState(res);
}

/*void VectorDrawable::updateLocalState(Resources res) {
    const int density = Drawable::resolveDensity(res, mVectorState::mDensity);
    if (mTargetDensity != density) {
        mTargetDensity = density;
        mDpiScaledDirty = true;
    }

    mTintFilter = updateTintFilter(mTintFilter, mVectorState->mTint, mVectorState->mTintMode);
}*/

Drawable* VectorDrawable::mutate() {
    if (!mMutated && Drawable::mutate() == this) {
        mVectorState = std::make_shared<VectorDrawableState>(mVectorState.get());
        mMutated = true;
    }
    return this;
}

void VectorDrawable::clearMutated() {
    Drawable::clearMutated();
    mMutated = false;
}

void* VectorDrawable::getTargetByName(const std::string& name) {
    auto it =mVectorState->mVGTargetsMap.find(name);
    return it->second;//mVectorState->mVGTargetsMap.get(name);
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
    bool canReuseCache = mVectorState->canReuseCache();
    int pixelCount = 0/*nDraw(mVectorState->getNativeRenderer(), canvas.getNativeCanvasWrapper(),
            colorFilterNativeInstance, mTmpBounds, needMirroring(),
            canReuseCache)*/;
    if (pixelCount == 0) {
        // Invalid canvas matrix or drawable bounds. This would not affect existing bitmap
        // cache, if any.
        return;
    }

    int deltaInBytes;
    // Track different bitmap cache based whether the canvas is hw accelerated. By doing so,
    // we don't over count bitmap cache allocation: if the input canvas is always of the same
    // type, only one bitmap cache is allocated.
    /*if (canvas.isHardwareAccelerated()) {
        // Each pixel takes 4 bytes.
        deltaInBytes = (pixelCount - mVectorState->mLastHWCachePixelCount) * 4;
        mVectorState->mLastHWCachePixelCount = pixelCount;
    } else */{
        // Each pixel takes 4 bytes.
        deltaInBytes = (pixelCount - mVectorState->mLastSWCachePixelCount) * 4;
        mVectorState->mLastSWCachePixelCount = pixelCount;
    }
}

int VectorDrawable::getAlpha() const{
    return (int) (mVectorState->getAlpha() * 255);
}

void VectorDrawable::setAlpha(int alpha) {
    if (mVectorState->setAlpha(alpha / 255.f)) {
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
    return mVectorState != nullptr && mVectorState->hasFocusStateSpecified();
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
    if (mVectorState == nullptr ||
            mVectorState->mBaseWidth == 0 ||
            mVectorState->mBaseHeight == 0 ||
            mVectorState->mViewportHeight == 0 ||
            mVectorState->mViewportWidth == 0) {
        return 1; // fall back to 1:1 pixel mapping.
    }
    float intrinsicWidth = mVectorState->mBaseWidth;
    float intrinsicHeight = mVectorState->mBaseHeight;
    float viewportWidth = mVectorState->mViewportWidth;
    float viewportHeight = mVectorState->mViewportHeight;
    float scaleX = viewportWidth / intrinsicWidth;
    float scaleY = viewportHeight / intrinsicHeight;
    return std::min(scaleX, scaleY);
}

VectorDrawable* VectorDrawable::create(Context*, const std::string&rid) {
    VectorDrawable* drawable = new VectorDrawable();
    /*drawable->inflate(resources, parser, attrs);
    if(mVectorState->mRootGroup||mVectorState->mNativeTree){
        if(mVectorState->mRootGroup){
            mVectorState->mRootGroup->setTree(nullptr);
        }
        VectorState->mRootGroup = new VGroup();
        if(mVectorState->mNativeTree){
        }
    }*/
    return drawable;
}

#if 0
void VectorDrawable::inflate(@NonNull Resources r, @NonNull XmlPullParser parser,
        @NonNull AttributeSet attrs, @Nullable Theme theme){
    try {
        Trace.traceBegin(Trace.TRACE_TAG_RESOURCES, "VectorDrawable#inflate");
        if (mVectorState.mRootGroup != null || mVectorState.mNativeTree != null) {
            // This VD has been used to display other VD resource content, clean up.
            if (mVectorState.mRootGroup != null) {
                // Subtract the native allocation for all the nodes.
                VMRuntime.getRuntime().registerNativeFree(
                        mVectorState.mRootGroup.getNativeSize());
                // Remove child nodes' reference to tree
                mVectorState.mRootGroup.setTree(null);
            }
            mVectorState.mRootGroup = new VGroup();
            if (mVectorState.mNativeTree != null) {
                // Subtract the native allocation for the tree wrapper, which contains root node
                // as well as rendering related data.
                VMRuntime.getRuntime().registerNativeFree(mVectorState.NATIVE_ALLOCATION_SIZE);
                mVectorState.mNativeTree.release();
            }
            mVectorState.createNativeTree(mVectorState.mRootGroup);
        }
        final VectorDrawableState state = mVectorState;
        state.setDensity(Drawable.resolveDensity(r, 0));

        final TypedArray a = obtainAttributes(r, theme, attrs, R.styleable.VectorDrawable);
        updateStateFromTypedArray(a);
        a.recycle();

        mDpiScaledDirty = true;

        state.mCacheDirty = true;
        inflateChildElements(r, parser, attrs, theme);

        state.onTreeConstructionFinished();
        // Update local properties.
        updateLocalState(r);
    } finally {
        Trace.traceEnd(Trace.TRACE_TAG_RESOURCES);
    }
}

void VectorDrawable::updateStateFromTypedArray(TypedArray a) throws XmlPullParserException {
    auto state = mVectorState;

    // Account for any configuration changes.
    state->mChangingConfigurations |= a.getChangingConfigurations();

    // Extract the theme attributes, if any.
    state.mThemeAttrs = a.extractThemeAttrs();

    final int tintMode = a.getInt(R.styleable.VectorDrawable_tintMode, -1);
    if (tintMode != -1) {
        state.mTintMode = Drawable.parseTintMode(tintMode, Mode.SRC_IN);
    }

    final ColorStateList tint = a.getColorStateList(R.styleable.VectorDrawable_tint);
    if (tint != null) {
        state.mTint = tint;
    }

    state.mAutoMirrored = a.getBoolean(
            R.styleable.VectorDrawable_autoMirrored, state.mAutoMirrored);

    float viewportWidth = a.getFloat(
            R.styleable.VectorDrawable_viewportWidth, state.mViewportWidth);
    float viewportHeight = a.getFloat(
            R.styleable.VectorDrawable_viewportHeight, state.mViewportHeight);
    state.setViewportSize(viewportWidth, viewportHeight);

    if (state.mViewportWidth <= 0) {
        throw new XmlPullParserException(a.getPositionDescription() +
                "<vector> tag requires viewportWidth > 0");
    } else if (state.mViewportHeight <= 0) {
        throw new XmlPullParserException(a.getPositionDescription() +
                "<vector> tag requires viewportHeight > 0");
    }

    state.mBaseWidth = a.getDimensionPixelSize(
            R.styleable.VectorDrawable_width, state.mBaseWidth);
    state.mBaseHeight = a.getDimensionPixelSize(
            R.styleable.VectorDrawable_height, state.mBaseHeight);

    if (state.mBaseWidth <= 0) {
        throw new XmlPullParserException(a.getPositionDescription() +
                "<vector> tag requires width > 0");
    } else if (state.mBaseHeight <= 0) {
        throw new XmlPullParserException(a.getPositionDescription() +
                "<vector> tag requires height > 0");
    }

    final int insetLeft = a.getDimensionPixelOffset(
            R.styleable.VectorDrawable_opticalInsetLeft, state.mOpticalInsets.left);
    final int insetTop = a.getDimensionPixelOffset(
            R.styleable.VectorDrawable_opticalInsetTop, state.mOpticalInsets.top);
    final int insetRight = a.getDimensionPixelOffset(
            R.styleable.VectorDrawable_opticalInsetRight, state.mOpticalInsets.right);
    final int insetBottom = a.getDimensionPixelOffset(
            R.styleable.VectorDrawable_opticalInsetBottom, state.mOpticalInsets.bottom);
    state.mOpticalInsets = Insets.of(insetLeft, insetTop, insetRight, insetBottom);

    final float alphaInFloat = a.getFloat(
            R.styleable.VectorDrawable_alpha, state.getAlpha());
    state.setAlpha(alphaInFloat);

    final String name = a.getString(R.styleable.VectorDrawable_name);
    if (name != null) {
        state.mRootName = name;
        state.mVGTargetsMap.put(name, state);
    }
}

void VectorDrawable::inflateChildElements(Resources res, XmlPullParser parser, AttributeSet attrs,
        Theme theme){
    final VectorDrawableState state = mVectorState;
    bool noPathTag = true;

    // Use a stack to help to build the group tree.
    // The top of the stack is always the current group.
    final Stack<VGroup> groupStack = new Stack<VGroup>();
    groupStack.push(state.mRootGroup);

    int eventType = parser.getEventType();
    final int innerDepth = parser.getDepth() + 1;

    // Parse everything until the end of the vector element.
    while (eventType != XmlPullParser.END_DOCUMENT
            && (parser.getDepth() >= innerDepth || eventType != XmlPullParser.END_TAG)) {
        if (eventType == XmlPullParser.START_TAG) {
            final String tagName = parser.getName();
            final VGroup currentGroup = groupStack.peek();

            if (SHAPE_PATH.equals(tagName)) {
                final VFullPath path = new VFullPath();
                path.inflate(res, attrs, theme);
                currentGroup.addChild(path);
                if (path.getPathName() != null) {
                    state.mVGTargetsMap.put(path.getPathName(), path);
                }
                noPathTag = false;
                state.mChangingConfigurations |= path.mChangingConfigurations;
            } else if (SHAPE_CLIP_PATH.equals(tagName)) {
                final VClipPath path = new VClipPath();
                path.inflate(res, attrs, theme);
                currentGroup.addChild(path);
                if (path.getPathName() != null) {
                    state.mVGTargetsMap.put(path.getPathName(), path);
                }
                state.mChangingConfigurations |= path.mChangingConfigurations;
            } else if (SHAPE_GROUP.equals(tagName)) {
                VGroup newChildGroup = new VGroup();
                newChildGroup.inflate(res, attrs, theme);
                currentGroup.addChild(newChildGroup);
                groupStack.push(newChildGroup);
                if (newChildGroup.getGroupName() != null) {
                    state.mVGTargetsMap.put(newChildGroup.getGroupName(),
                            newChildGroup);
                }
                state.mChangingConfigurations |= newChildGroup.mChangingConfigurations;
            }
        } else if (eventType == XmlPullParser.END_TAG) {
            final String tagName = parser.getName();
            if (SHAPE_GROUP.equals(tagName)) {
                groupStack.pop();
            }
        }
        eventType = parser.next();
    }

    if (noPathTag) {
        final StringBuffer tag = new StringBuffer();

        if (tag.length() > 0) {
            tag.append(" or ");
        }
        tag.append(SHAPE_PATH);

        throw new XmlPullParserException("no " + tag + " defined");
    }
}
#endif

int VectorDrawable::getChangingConfigurations()const {
    return Drawable::getChangingConfigurations() | mVectorState->getChangingConfigurations();
}

void VectorDrawable::setAllowCaching(bool allowCaching) {
    //nSetAllowCaching(mVectorState->getNativeRenderer(), allowCaching);
}

bool VectorDrawable::needMirroring() {
    return isAutoMirrored() && getLayoutDirection() == LayoutDirection::RTL;
}

void VectorDrawable::setAutoMirrored(bool mirrored) {
    if (mVectorState->mAutoMirrored != mirrored) {
        mVectorState->mAutoMirrored = mirrored;
        invalidateSelf();
    }
}

bool VectorDrawable::isAutoMirrored() {
    return mVectorState->mAutoMirrored;
}

long VectorDrawable::getNativeTree() {
    return mVectorState->getNativeRenderer();
}

void VectorDrawable::setAntiAlias(bool aa) {
    //nSetAntiAlias(mVectorState.mNativeTree.get(), aa);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////
///static class VectorDrawableState extends ConstantState {

Property* /*<VectorDrawableState, Float>*/ VectorDrawable::VectorDrawableState::ALPHA;

Property* VectorDrawable::VectorDrawableState::getProperty(const std::string& propertyName) {
    if (ALPHA->getName().compare(propertyName)==0) {
        return ALPHA;
    }
    return nullptr;
}

// If copy is not null, deep copy the given VectorDrawableState. Otherwise, create a
// native vector drawable tree with an empty root group.
VectorDrawable::VectorDrawableState::VectorDrawableState(const VectorDrawableState* copy) {
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
            mVGTargetsMap.emplace(copy->mRootName, this);
        }
    } else {
        mRootGroup = new VGroup();
        createNativeTree(mRootGroup);
    }
    onTreeConstructionFinished();
}

void VectorDrawable::VectorDrawableState::createNativeTree(VGroup* rootGroup) {
   // mNativeTree = new VirtualRefBasePtr(nCreateTree(rootGroup->mNativePtr));
}

// Create a new native tree with the given root group, and copy the properties from the
// given VectorDrawableState's native tree.
void VectorDrawable::VectorDrawableState::createNativeTreeFromCopy(const VectorDrawableState& copy, VGroup* rootGroup) {
    //mNativeTree = new VirtualRefBasePtr(nCreateTreeFromCopy(copy->mNativeTree.get(), rootGroup->mNativePtr));
}

// This should be called every time after a new RootGroup and all its subtrees are created
// (i.e. in constructors of VectorDrawableState and in inflate).
void VectorDrawable::VectorDrawableState::onTreeConstructionFinished() {
    mRootGroup->setTree(mNativeTree);
    mAllocationOfAllNodes = mRootGroup->getNativeSize();
    //VMRuntime.getRuntime().registerNativeAllocation(mAllocationOfAllNodes);
}

long VectorDrawable::VectorDrawableState::getNativeRenderer() {
    if (mNativeTree == nullptr) {
        return 0;
    }
    return (long)mNativeTree;
}

bool VectorDrawable::VectorDrawableState::canReuseCache() {
    if (!mCacheDirty
            && mCachedThemeAttrs == mThemeAttrs
            && mCachedTint == mTint
            && mCachedTintMode == mTintMode
            && mCachedAutoMirrored == mAutoMirrored) {
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
    nSetRendererViewportSize(getNativeRenderer(), viewportWidth, viewportHeight);
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
}

/**
 * setAlpha() and getAlpha() are used mostly for animation purpose. Return true if alpha
 * has changed.
 */
bool VectorDrawable::VectorDrawableState::setAlpha(float alpha) {
    return false;//nSetRootAlpha(mNativeTree.get(), alpha);
}

float VectorDrawable::VectorDrawableState::getAlpha() {
    return 1.f;//nGetRootAlpha(mNativeTree.get());
}

//////////////////////////////////////////////////////////////////////////////////////////////
//static class VGroup extends VObject {
std::unordered_map<std::string, int> VectorDrawable::VGroup::sPropertyIndexMap ={
    {"translateX", TRANSLATE_X_INDEX},
    {"translateY", TRANSLATE_Y_INDEX},
    {"scaleX", SCALE_X_INDEX},
    {"scaleY", SCALE_Y_INDEX},
    {"pivotX", PIVOT_X_INDEX},
    {"pivotY", PIVOT_Y_INDEX},
    {"rotation", ROTATION_INDEX}
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
Property* /*<VGroup,float>*/ VectorDrawable::VGroup::TRANSLATE_X ;
Property* /*<VGroup,float>*/ VectorDrawable::VGroup::TRANSLATE_Y ;
Property* /*<VGroup,float>*/ VectorDrawable::VGroup::SCALE_X ;
Property* /*<VGroup,float>*/ VectorDrawable::VGroup::SCALE_Y ;
Property* /*<VGroup,float>*/ VectorDrawable::VGroup::PIVOT_X ;
Property* /*<VGroup,float>*/ VectorDrawable::VGroup::PIVOT_Y ;
Property* /*<VGroup,float>*/ VectorDrawable::VGroup::ROTATION ;
std::unordered_map<std::string, Property*> VectorDrawable::VGroup::sPropertyMap ={
    {"translateX", TRANSLATE_X},
    {"translateY", TRANSLATE_Y},
    {"scaleX", SCALE_X},
    {"scaleY", SCALE_Y},
    {"pivotX", PIVOT_X},
    {"pivotY", PIVOT_Y},
    {"rotation", ROTATION}
};


// Temp array to store transform values obtained from native.
VectorDrawable::VGroup::VGroup(const VGroup* copy,std::unordered_map<std::string, void*>& targetsMap) {

    mIsStateful = copy->mIsStateful;
    //mThemeAttrs = copy->mThemeAttrs;
    mGroupName = copy->mGroupName;
    mChangingConfigurations = copy->mChangingConfigurations;
    if (!mGroupName.empty()) {
        targetsMap.emplace(mGroupName, this);
    }
    mNativePtr = nCreateGroup(copy->mNativePtr);

    const std::vector<VObject*>& children = copy->mChildren;
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

VectorDrawable::VGroup::VGroup() {
    //mNativePtr = nCreateGroup();
}

Property* VectorDrawable::VGroup::getProperty(const std::string& propertyName) {
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
    nAddChild(mNativePtr, child->getNativePtr());
    mChildren.push_back(child);
    mIsStateful |= child->isStateful();
}

void VectorDrawable::VGroup::setTree(VirtualRefBasePtr treeRoot) {
    VObject::setTree(treeRoot);
    for (int i = 0; i < mChildren.size(); i++) {
        mChildren.at(i)->setTree(treeRoot);
    }
}

long VectorDrawable::VGroup::getNativePtr() {
    return mNativePtr;
}

#if 0
void VectorDrawable::VGroup::inflate(Resources res, AttributeSet attrs, Theme theme) {
    final TypedArray a = obtainAttributes(res, theme, attrs,R.styleable.VectorDrawableGroup);
    updateStateFromTypedArray(a);
    a.recycle();
}

void VectorDrawable::VGroup::updateStateFromTypedArray(TypedArray a) {
    // Account for any configuration changes.
    mChangingConfigurations |= a.getChangingConfigurations();

    // Extract the theme attributes, if any.
    mThemeAttrs = a.extractThemeAttrs();
    if (mTransform == null) {
        // Lazy initialization: If the group is created through copy constructor, this may
        // never get called.
        mTransform = new float[TRANSFORM_PROPERTY_COUNT];
    }
    bool success = nGetGroupProperties(mNativePtr, mTransform, TRANSFORM_PROPERTY_COUNT);
    if (!success) {
        throw new RuntimeException("Error: inconsistent property count");
    }
    float rotate = a.getFloat(R.styleable.VectorDrawableGroup_rotation,
            mTransform[ROTATION_INDEX]);
    float pivotX = a.getFloat(R.styleable.VectorDrawableGroup_pivotX,
            mTransform[PIVOT_X_INDEX]);
    float pivotY = a.getFloat(R.styleable.VectorDrawableGroup_pivotY,
            mTransform[PIVOT_Y_INDEX]);
    float scaleX = a.getFloat(R.styleable.VectorDrawableGroup_scaleX,
            mTransform[SCALE_X_INDEX]);
    float scaleY = a.getFloat(R.styleable.VectorDrawableGroup_scaleY,
            mTransform[SCALE_Y_INDEX]);
    float translateX = a.getFloat(R.styleable.VectorDrawableGroup_translateX,
            mTransform[TRANSLATE_X_INDEX]);
    float translateY = a.getFloat(R.styleable.VectorDrawableGroup_translateY,
            mTransform[TRANSLATE_Y_INDEX]);

    final String groupName = a.getString(R.styleable.VectorDrawableGroup_name);
    if (groupName != null) {
        mGroupName = groupName;
        nSetName(mNativePtr, mGroupName);
    }
     nUpdateGroupProperties(mNativePtr, rotate, pivotX, pivotY, scaleX, scaleY,
             translateX, translateY);
}
#endif

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

int VectorDrawable::VGroup::getNativeSize() const{
    // Return the native allocation needed for the subtree.
    int size = NATIVE_ALLOCATION_SIZE;
    for (int i = 0; i < mChildren.size(); i++) {
        size += mChildren.at(i)->getNativeSize();
    }
    return size;
}

bool VectorDrawable::VGroup::canApplyTheme() {
    if (mThemeAttrs != nullptr) {
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
    return isTreeValid() ? nGetRotation(mNativePtr) : 0;
}

void VectorDrawable::VGroup::setRotation(float rotation) {
    if (isTreeValid()) {
        nSetRotation(mNativePtr, rotation);
    }
}

float VectorDrawable::VGroup::getPivotX() {
    return isTreeValid() ? nGetPivotX(mNativePtr) : 0;
}

void VectorDrawable::VGroup::setPivotX(float pivotX) {
    if (isTreeValid()) {
        nSetPivotX(mNativePtr, pivotX);
    }
}

float VectorDrawable::VGroup::getPivotY() {
    return isTreeValid() ? nGetPivotY(mNativePtr) : 0;
}

void VectorDrawable::VGroup::setPivotY(float pivotY) {
    if (isTreeValid()) {
        nSetPivotY(mNativePtr, pivotY);
    }
}

float VectorDrawable::VGroup::getScaleX() {
    return isTreeValid() ? nGetScaleX(mNativePtr) : 0;
}

void VectorDrawable::VGroup::setScaleX(float scaleX) {
    if (isTreeValid()) {
        nSetScaleX(mNativePtr, scaleX);
    }
}

float VectorDrawable::VGroup::getScaleY() {
    return isTreeValid() ? nGetScaleY(mNativePtr) : 0;
}

void VectorDrawable::VGroup::setScaleY(float scaleY) {
    if (isTreeValid()) {
        nSetScaleY(mNativePtr, scaleY);
    }
}

float VectorDrawable::VGroup::getTranslateX() {
    return isTreeValid() ? nGetTranslateX(mNativePtr) : 0;
}

void VectorDrawable::VGroup::setTranslateX(float translateX) {
    if (isTreeValid()) {
        nSetTranslateX(mNativePtr, translateX);
    }
}

float VectorDrawable::VGroup::getTranslateY() {
    return isTreeValid() ? nGetTranslateY(mNativePtr) : 0;
}

void VectorDrawable::VGroup::setTranslateY(float translateY) {
    if (isTreeValid()) {
        nSetTranslateY(mNativePtr, translateY);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Common Path information for clip path and normal path.
 */
//static abstract class VPath extends VObject {

Property* /*<VPath, PathParser.PathData>*/VectorDrawable::VPath::PATH_DATA;

Property* VectorDrawable::VPath::getProperty(const std::string& propertyName) {
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
    //mPathData = copy->mPathData == nullptr ? nullptr : new PathData(copy->mPathData);
}

std::string VectorDrawable::VPath::getPathName() const{
    return mPathName;
}


/* Setters and Getters, used by animator from AnimatedVectorDrawable. */
PathData* VectorDrawable::VPath::getPathData() {
    return mPathData;
}

// TODO: Move the PathEvaluator and this setter and the getter above into native.
void VectorDrawable::VPath::setPathData(PathData* pathData) {
    /*mPathData->setPathData(pathData);
    if (isTreeValid()) {
        nSetPathData(getNativePtr(), mPathData.getNativePtr());
    }*/
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Clip path, which only has name and pathData.
 */
//static class VClipPath extends VPath {
    
VectorDrawable::VClipPath::VClipPath() {
    mNativePtr = nCreateClipPath();
}

VectorDrawable::VClipPath::VClipPath(const VClipPath* copy)
    :VPath(copy){
    mNativePtr = nCreateClipPath(copy->mNativePtr);
}

long VectorDrawable::VClipPath::getNativePtr() {
    return mNativePtr;
}


void VectorDrawable::VClipPath::inflate(Context*,const AttributeSet& attrs, Theme theme) {
    /*final TypedArray a = obtainAttributes(r, theme, attrs,R.styleable.VectorDrawableClipPath);
    updateStateFromTypedArray(a);
    a.recycle();*/
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

int VectorDrawable::VClipPath::getNativeSize() const{
    return NATIVE_ALLOCATION_SIZE;
}

/*void VectorDrawable::VClipPath::updateStateFromTypedArray(TypedArray a) {
    // Account for any configuration changes.
    mChangingConfigurations |= a.getChangingConfigurations();

    final String pathName = a.getString(R.styleable.VectorDrawableClipPath_name);
    if (pathName != nullptr) {
        mPathName = pathName;
        nSetName(mNativePtr, mPathName);
    }

    final String pathDataString = a.getString(R.styleable.VectorDrawableClipPath_pathData);
    if (pathDataString != nullptr) {
        mPathData = new PathParser.PathData(pathDataString);
        nSetPathString(mNativePtr, pathDataString, pathDataString.length());
    }
}*/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Normal path, which contains all the fill / paint information.
 */
//static class VFullPath extends VPath
// Property map for animatable attributes.
std::unordered_map<std::string, int> VectorDrawable::VFullPath::sPropertyIndexMap={
      {"strokeWidth", STROKE_WIDTH_INDEX},
      {"strokeColor", STROKE_COLOR_INDEX},
      {"strokeAlpha", STROKE_ALPHA_INDEX},
      {"fillColor", FILL_COLOR_INDEX},
      {"fillAlpha", FILL_ALPHA_INDEX},
      {"trimPathStart", TRIM_PATH_START_INDEX},
      {"trimPathEnd", TRIM_PATH_END_INDEX},
      {"trimPathOffset", TRIM_PATH_OFFSET_INDEX}
};
#if 0
// Below are the Properties that wrap the setters to avoid reflection overhead in animations
private static final Property<VFullPath, Float> STROKE_WIDTH =
        new FloatProperty<VFullPath> ("strokeWidth") {
            @Override
            public void setValue(VFullPath object, float value) {
                object.setStrokeWidth(value);
            }

            @Override
            public Float get(VFullPath object) {
                return object.getStrokeWidth();
            }
        };

private static final Property<VFullPath, Integer> STROKE_COLOR =
        new IntProperty<VFullPath> ("strokeColor") {
            @Override
            public void setValue(VFullPath object, int value) {
                object.setStrokeColor(value);
            }

            @Override
            public Integer get(VFullPath object) {
                return object.getStrokeColor();
            }
        };

private static final Property<VFullPath, Float> STROKE_ALPHA =
        new FloatProperty<VFullPath> ("strokeAlpha") {
            @Override
            public void setValue(VFullPath object, float value) {
                object.setStrokeAlpha(value);
            }

            @Override
            public Float get(VFullPath object) {
                return object.getStrokeAlpha();
            }
        };

private static final Property<VFullPath, Integer> FILL_COLOR =
        new IntProperty<VFullPath>("fillColor") {
            @Override
            public void setValue(VFullPath object, int value) {
                object.setFillColor(value);
            }

            @Override
            public Integer get(VFullPath object) {
                return object.getFillColor();
            }
        };

private static final Property<VFullPath, Float> FILL_ALPHA =
        new FloatProperty<VFullPath> ("fillAlpha") {
            @Override
            public void setValue(VFullPath object, float value) {
                object.setFillAlpha(value);
            }

            @Override
            public Float get(VFullPath object) {
                return object.getFillAlpha();
            }
        };

private static final Property<VFullPath, Float> TRIM_PATH_START =
        new FloatProperty<VFullPath> ("trimPathStart") {
            @Override
            public void setValue(VFullPath object, float value) {
                object.setTrimPathStart(value);
            }

            @Override
            public Float get(VFullPath object) {
                return object.getTrimPathStart();
            }
        };

private static final Property<VFullPath, Float> TRIM_PATH_END =
        new FloatProperty<VFullPath> ("trimPathEnd") {
            @Override
            public void setValue(VFullPath object, float value) {
                object.setTrimPathEnd(value);
            }

            @Override
            public Float get(VFullPath object) {
                return object.getTrimPathEnd();
            }
        };

private static final Property<VFullPath, Float> TRIM_PATH_OFFSET =
        new FloatProperty<VFullPath> ("trimPathOffset") {
            @Override
            public void setValue(VFullPath object, float value) {
                object.setTrimPathOffset(value);
            }

            @Override
            public Float get(VFullPath object) {
                return object.getTrimPathOffset();
            }
        };
#endif
std::unordered_map<std::string, Property*> VectorDrawable::VFullPath::sPropertyMap={
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
    mNativePtr = nCreateFullPath();
}

VectorDrawable::VFullPath::VFullPath(const VFullPath* copy):VPath(copy){
    mNativePtr = nCreateFullPath(copy->mNativePtr);
    //mThemeAttrs = copy->mThemeAttrs;
    mStrokeColors = copy->mStrokeColors;
    mFillColors = copy->mFillColors;
}

Property* VectorDrawable::VFullPath::getProperty(const std::string& propertyName) {
    Property* p = VPath::getProperty(propertyName);
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
            nSetStrokeColor(mNativePtr, newStrokeColor);
        }
    }

    if (mFillColors != nullptr && dynamic_cast<ColorStateList*>(mFillColors)) {
        const int oldFillColor = getFillColor();
        const int newFillColor = ((ColorStateList*) mFillColors)->getColorForState(stateSet, oldFillColor);
        changed |= oldFillColor != newFillColor;
        if (oldFillColor != newFillColor) {
            nSetFillColor(mNativePtr, newFillColor);
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

int VectorDrawable::VFullPath::getNativeSize() const{
    return NATIVE_ALLOCATION_SIZE;
}

long VectorDrawable::VFullPath::getNativePtr() {
    return mNativePtr;
}

void VectorDrawable::VFullPath::inflate(Context*,const AttributeSet& attrs, Theme theme) {
    /*final TypedArray a = obtainAttributes(r, theme, attrs,R.styleable.VectorDrawablePath);
    updateStateFromTypedArray(a);
    a.recycle();*/
}

#if 0
void VectorDrawable::VFullPath::updateStateFromTypedArray(TypedArray a) {
    const int byteCount = TOTAL_PROPERTY_COUNT * 4;
    if (mPropertyData == null) {
        // Lazy initialization: If the path is created through copy constructor, this may
        // never get called.
        mPropertyData = new byte[byteCount];
    }
    // The bulk getters/setters of property data (e.g. stroke width, color, etc) allows us
    // to pull current values from native and store modifications with only two methods,
    // minimizing JNI overhead.
    bool success = nGetFullPathProperties(mNativePtr, mPropertyData, byteCount);
    if (!success) {
        throw new RuntimeException("Error: inconsistent property count");
    }

    ByteBuffer properties = ByteBuffer.wrap(mPropertyData);
    properties.order(ByteOrder.nativeOrder());
    float strokeWidth = properties.getFloat(STROKE_WIDTH_INDEX * 4);
    int strokeColor = properties.getInt(STROKE_COLOR_INDEX * 4);
    float strokeAlpha = properties.getFloat(STROKE_ALPHA_INDEX * 4);
    int fillColor =  properties.getInt(FILL_COLOR_INDEX * 4);
    float fillAlpha = properties.getFloat(FILL_ALPHA_INDEX * 4);
    float trimPathStart = properties.getFloat(TRIM_PATH_START_INDEX * 4);
    float trimPathEnd = properties.getFloat(TRIM_PATH_END_INDEX * 4);
    float trimPathOffset = properties.getFloat(TRIM_PATH_OFFSET_INDEX * 4);
    int strokeLineCap =  properties.getInt(STROKE_LINE_CAP_INDEX * 4);
    int strokeLineJoin = properties.getInt(STROKE_LINE_JOIN_INDEX * 4);
    float strokeMiterLimit = properties.getFloat(STROKE_MITER_LIMIT_INDEX * 4);
    int fillType = properties.getInt(FILL_TYPE_INDEX * 4);
    Shader fillGradient = null;
    Shader strokeGradient = null;
    // Account for any configuration changes.
    mChangingConfigurations |= a.getChangingConfigurations();

    // Extract the theme attributes, if any.
    mThemeAttrs = a.extractThemeAttrs();

    final String pathName = a.getString(R.styleable.VectorDrawablePath_name);
    if (pathName != null) {
        mPathName = pathName;
        nSetName(mNativePtr, mPathName);
    }

    final String pathString = a.getString(R.styleable.VectorDrawablePath_pathData);
    if (pathString != null) {
        mPathData = new PathParser.PathData(pathString);
        nSetPathString(mNativePtr, pathString, pathString.length());
    }

    final ComplexColor fillColors = a.getComplexColor(
            R.styleable.VectorDrawablePath_fillColor);
    if (fillColors != null) {
        // If the colors is a gradient color, or the color state list is stateful, keep the
        // colors information. Otherwise, discard the colors and keep the default color.
        if (fillColors instanceof  GradientColor) {
            mFillColors = fillColors;
            fillGradient = ((GradientColor) fillColors).getShader();
        } else if (fillColors.isStateful()) {
            mFillColors = fillColors;
        } else {
            mFillColors = null;
        }
        fillColor = fillColors.getDefaultColor();
    }

    final ComplexColor strokeColors = a.getComplexColor(
            R.styleable.VectorDrawablePath_strokeColor);
    if (strokeColors != null) {
        // If the colors is a gradient color, or the color state list is stateful, keep the
        // colors information. Otherwise, discard the colors and keep the default color.
        if (strokeColors instanceof GradientColor) {
            mStrokeColors = strokeColors;
            strokeGradient = ((GradientColor) strokeColors).getShader();
        } else if (strokeColors.isStateful()) {
            mStrokeColors = strokeColors;
        } else {
            mStrokeColors = null;
        }
        strokeColor = strokeColors.getDefaultColor();
    }
    // Update the gradient info, even if the gradiet is null.
    nUpdateFullPathFillGradient(mNativePtr,
            fillGradient != null ? fillGradient.getNativeInstance() : 0);
    nUpdateFullPathStrokeGradient(mNativePtr,
            strokeGradient != null ? strokeGradient.getNativeInstance() : 0);

    fillAlpha = a.getFloat(R.styleable.VectorDrawablePath_fillAlpha, fillAlpha);

    strokeLineCap = a.getInt(R.styleable.VectorDrawablePath_strokeLineCap, strokeLineCap);
    strokeLineJoin = a.getInt(R.styleable.VectorDrawablePath_strokeLineJoin, strokeLineJoin);
    strokeMiterLimit = a.getFloat(R.styleable.VectorDrawablePath_strokeMiterLimit, strokeMiterLimit);
    strokeAlpha = a.getFloat(R.styleable.VectorDrawablePath_strokeAlpha,strokeAlpha);
    strokeWidth = a.getFloat(R.styleable.VectorDrawablePath_strokeWidth,strokeWidth);
    trimPathEnd = a.getFloat(R.styleable.VectorDrawablePath_trimPathEnd,trimPathEnd);
    trimPathOffset = a.getFloat(R.styleable.VectorDrawablePath_trimPathOffset, trimPathOffset);
    trimPathStart = a.getFloat(R.styleable.VectorDrawablePath_trimPathStart, trimPathStart);
    fillType = a.getInt(R.styleable.VectorDrawablePath_fillType, fillType);

    nUpdateFullPathProperties(mNativePtr, strokeWidth, strokeColor, strokeAlpha,
            fillColor, fillAlpha, trimPathStart, trimPathEnd, trimPathOffset,
            strokeMiterLimit, strokeLineCap, strokeLineJoin, fillType);
}
#endif

bool VectorDrawable::VFullPath::canApplyTheme() {
    if (mThemeAttrs != nullptr) {
        return true;
    }

    bool fillCanApplyTheme = canComplexColorApplyTheme(mFillColors);
    bool strokeCanApplyTheme = canComplexColorApplyTheme(mStrokeColors);
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
    return isTreeValid() ? nGetStrokeColor(mNativePtr) : 0;
}

void VectorDrawable::VFullPath::setStrokeColor(int strokeColor) {
    mStrokeColors = nullptr;
    if (isTreeValid()) {
        nSetStrokeColor(mNativePtr, strokeColor);
    }
}

float VectorDrawable::VFullPath::getStrokeWidth() {
    return isTreeValid() ? nGetStrokeWidth(mNativePtr) : 0;
}

void VectorDrawable::VFullPath::setStrokeWidth(float strokeWidth) {
    if (isTreeValid()) {
        nSetStrokeWidth(mNativePtr, strokeWidth);
    }
}

float VectorDrawable::VFullPath::getStrokeAlpha() {
    return isTreeValid() ? nGetStrokeAlpha(mNativePtr) : 0;
}

void VectorDrawable::VFullPath::setStrokeAlpha(float strokeAlpha) {
    if (isTreeValid()) {
        nSetStrokeAlpha(mNativePtr, strokeAlpha);
    }
}

int VectorDrawable::VFullPath::getFillColor() {
    return isTreeValid() ? nGetFillColor(mNativePtr) : 0;
}

void VectorDrawable::VFullPath::setFillColor(int fillColor) {
    mFillColors = nullptr;
    if (isTreeValid()) {
        nSetFillColor(mNativePtr, fillColor);
    }
}

float VectorDrawable::VFullPath::getFillAlpha() {
    return isTreeValid() ? nGetFillAlpha(mNativePtr) : 0;
}

void VectorDrawable::VFullPath::setFillAlpha(float fillAlpha) {
    if (isTreeValid()) {
        nSetFillAlpha(mNativePtr, fillAlpha);
    }
}

float VectorDrawable::VFullPath::getTrimPathStart() {
    return isTreeValid() ? nGetTrimPathStart(mNativePtr) : 0;
}

void VectorDrawable::VFullPath::setTrimPathStart(float trimPathStart) {
    if (isTreeValid()) {
        nSetTrimPathStart(mNativePtr, trimPathStart);
    }
}

float VectorDrawable::VFullPath::getTrimPathEnd() {
    return isTreeValid() ? nGetTrimPathEnd(mNativePtr) : 0;
}

void VectorDrawable::VFullPath::setTrimPathEnd(float trimPathEnd) {
    if (isTreeValid()) {
        nSetTrimPathEnd(mNativePtr, trimPathEnd);
    }
}

float VectorDrawable::VFullPath::getTrimPathOffset() {
    return isTreeValid() ? nGetTrimPathOffset(mNativePtr) : 0;
}

void VectorDrawable::VFullPath::setTrimPathOffset(float trimPathOffset) {
    if (isTreeValid()) {
        nSetTrimPathOffset(mNativePtr, trimPathOffset);
    }
}

#if 0
////////////////////////////////////////////////////////////////////////////////////////////////////////////
//VectorDrawable's native Methods
    private static native int nDraw(long rendererPtr, long canvasWrapperPtr,
            long colorFilterPtr, Rect bounds, bool needsMirroring, bool canReuseCache);
    private static native bool nGetFullPathProperties(long pathPtr, byte[] properties,
            int length);
    private static native void nSetName(long nodePtr, String name);
    private static native bool nGetGroupProperties(long groupPtr, float[] properties,
            int length);
    private static native void nSetPathString(long pathPtr, String pathString, int length);

    // ------------- @FastNative ------------------

    @FastNative
    private static native long nCreateTree(long rootGroupPtr);
    @FastNative
    private static native long nCreateTreeFromCopy(long treeToCopy, long rootGroupPtr);
    @FastNative
    private static native void nSetRendererViewportSize(long rendererPtr, float viewportWidth,
            float viewportHeight);
    @FastNative
    private static native bool nSetRootAlpha(long rendererPtr, float alpha);
    @FastNative
    private static native float nGetRootAlpha(long rendererPtr);
    @FastNative
    private static native void nSetAntiAlias(long rendererPtr, bool aa);
    @FastNative
    private static native void nSetAllowCaching(long rendererPtr, bool allowCaching);

    @FastNative
    private static native long nCreateFullPath();
    @FastNative
    private static native long nCreateFullPath(long nativeFullPathPtr);

    @FastNative
    private static native void nUpdateFullPathProperties(long pathPtr, float strokeWidth,
            int strokeColor, float strokeAlpha, int fillColor, float fillAlpha, float trimPathStart,
            float trimPathEnd, float trimPathOffset, float strokeMiterLimit, int strokeLineCap,
            int strokeLineJoin, int fillType);
    @FastNative
    private static native void nUpdateFullPathFillGradient(long pathPtr, long fillGradientPtr);
    @FastNative
    private static native void nUpdateFullPathStrokeGradient(long pathPtr, long strokeGradientPtr);

    @FastNative
    private static native long nCreateClipPath();
    @FastNative
    private static native long nCreateClipPath(long clipPathPtr);

    @FastNative
    private static native long nCreateGroup();
    @FastNative
    private static native long nCreateGroup(long groupPtr);
    @FastNative
    private static native void nUpdateGroupProperties(long groupPtr, float rotate, float pivotX,
            float pivotY, float scaleX, float scaleY, float translateX, float translateY);

    @FastNative
    private static native void nAddChild(long groupPtr, long nodePtr);

    /**
     * The setters and getters below for paths and groups are here temporarily, and will be
     * removed once the animation in AVD is replaced with RenderNodeAnimator, in which case the
     * animation will modify these properties in native. By then no JNI hopping would be necessary
     * for VD during animation, and these setters and getters will be obsolete.
     */
    // Setters and getters during animation.
    @FastNative
    private static native float nGetRotation(long groupPtr);
    @FastNative
    private static native void nSetRotation(long groupPtr, float rotation);
    @FastNative
    private static native float nGetPivotX(long groupPtr);
    @FastNative
    private static native void nSetPivotX(long groupPtr, float pivotX);
    @FastNative
    private static native float nGetPivotY(long groupPtr);
    @FastNative
    private static native void nSetPivotY(long groupPtr, float pivotY);
    @FastNative
    private static native float nGetScaleX(long groupPtr);
    @FastNative
    private static native void nSetScaleX(long groupPtr, float scaleX);
    @FastNative
    private static native float nGetScaleY(long groupPtr);
    @FastNative
    private static native void nSetScaleY(long groupPtr, float scaleY);
    @FastNative
    private static native float nGetTranslateX(long groupPtr);
    @FastNative
    private static native void nSetTranslateX(long groupPtr, float translateX);
    @FastNative
    private static native float nGetTranslateY(long groupPtr);
    @FastNative
    private static native void nSetTranslateY(long groupPtr, float translateY);

    // Setters and getters for VPath during animation.
    @FastNative
    private static native void nSetPathData(long pathPtr, long pathDataPtr);
    @FastNative
    private static native float nGetStrokeWidth(long pathPtr);
    @FastNative
    private static native void nSetStrokeWidth(long pathPtr, float width);
    @FastNative
    private static native int nGetStrokeColor(long pathPtr);
    @FastNative
    private static native void nSetStrokeColor(long pathPtr, int strokeColor);
    @FastNative
    private static native float nGetStrokeAlpha(long pathPtr);
    @FastNative
    private static native void nSetStrokeAlpha(long pathPtr, float alpha);
    @FastNative
    private static native int nGetFillColor(long pathPtr);
    @FastNative
    private static native void nSetFillColor(long pathPtr, int fillColor);
    @FastNative
    private static native float nGetFillAlpha(long pathPtr);
    @FastNative
    private static native void nSetFillAlpha(long pathPtr, float fillAlpha);
    @FastNative
    private static native float nGetTrimPathStart(long pathPtr);
    @FastNative
    private static native void nSetTrimPathStart(long pathPtr, float trimPathStart);
    @FastNative
    private static native float nGetTrimPathEnd(long pathPtr);
    @FastNative
    private static native void nSetTrimPathEnd(long pathPtr, float trimPathEnd);
    @FastNative
    private static native float nGetTrimPathOffset(long pathPtr);
    @FastNative
    private static native void nSetTrimPathOffset(long pathPtr, float trimPathOffset);
}
#endif
}/*endof namespace*/
#endif
