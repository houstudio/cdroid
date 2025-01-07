#include <drawables/gradientdrawable.h>
#include <image-decoders/imagedecoder.h>
#include <cfloat>
#include <color.h>
#include <cdlog.h>
using namespace Cairo;
namespace cdroid {

#define DEFAULT_INNER_RADIUS_RATIO 3.0f
#define DEFAULT_THICKNESS_RATIO 9.0f

GradientDrawable::GradientState::GradientState() {
    mShape  = RECTANGLE;
    mGradient = LINEAR_GRADIENT;
    mTint = nullptr;
    mSolidColors = nullptr;
    mStrokeColors= nullptr;
    mImagePattern= nullptr;
    mAngle = 0;
    mStrokeWidth = -1;//if >= 0 use stroking
    mStrokeDashWidth = 0.0f;
    mStrokeDashGap = 0.0f;
    mRadius = 0.0f;
    mWidth = mHeight = -1;
    mPadding.setEmpty();
    mInnerRadiusRatio = DEFAULT_INNER_RADIUS_RATIO;
    mThicknessRatio = DEFAULT_THICKNESS_RATIO;
    mInnerRadius = -1;
    mThickness = -1;
    mDither = false;
    mCenterX = mCenterY =0.5f;
    mGradientRadius = 0.5f;
    mGradientRadiusType = RADIUS_TYPE_PIXELS;
    mUseLevel = false;
    mUseLevelForShape = true;
    mDensity = DisplayMetrics::DENSITY_DEFAULT;
}

GradientDrawable::GradientState::GradientState(Orientation orientation, const std::vector<int>&gradientColors)
    :GradientState() {
    mOrientation = orientation;
    setGradientColors(gradientColors);
}

GradientDrawable::GradientState::GradientState(const GradientState& orig) {
    mChangingConfigurations = orig.mChangingConfigurations;
    mShape = orig.mShape;
    mGradient = orig.mGradient;
    mAngle = orig.mAngle;
    mOrientation = orig.mOrientation;
    mSolidColors = orig.mSolidColors;
    mGradientColors = orig.mGradientColors;
    mPositions = orig.mPositions;
    mStrokeColors = orig.mStrokeColors;
    mImagePattern = orig.mImagePattern;
    mStrokeWidth = orig.mStrokeWidth;
    mStrokeDashWidth = orig.mStrokeDashWidth;
    mStrokeDashGap = orig.mStrokeDashGap;
    mRadius = orig.mRadius;
    mRadiusArray = orig.mRadiusArray;
    mPadding= orig.mPadding;
    mWidth  = orig.mWidth;
    mHeight = orig.mHeight;
    mInnerRadiusRatio = orig.mInnerRadiusRatio;
    mThicknessRatio = orig.mThicknessRatio;
    mInnerRadius = orig.mInnerRadius;
    mThickness = orig.mThickness;
    mDither = orig.mDither;
    mOpticalInsets = orig.mOpticalInsets;
    mCenterX = orig.mCenterX;
    mCenterY = orig.mCenterY;
    mGradientRadius = orig.mGradientRadius;
    mGradientRadiusType = orig.mGradientRadiusType;
    mUseLevel = orig.mUseLevel;
    mUseLevelForShape = orig.mUseLevelForShape;
    mOpaqueOverBounds = orig.mOpaqueOverBounds;
    mOpaqueOverShape = orig.mOpaqueOverShape;
    mTint = orig.mTint?new ColorStateList(*orig.mTint):nullptr;
    mTintMode = orig.mTintMode;
    //mThemeAttrs = orig.mThemeAttrs;
    mAttrSize = orig.mAttrSize;
    mAttrGradient = orig.mAttrGradient;
    mAttrSolid = orig.mAttrSolid;
    mAttrStroke = orig.mAttrStroke;
    mAttrCorners = orig.mAttrCorners;
    mAttrPadding = orig.mAttrPadding;

    mDensity = Drawable::resolveDensity(/*res,*/orig.mDensity);
    if (orig.mDensity != mDensity) {
        applyDensityScaling(orig.mDensity, mDensity);
    }
}

GradientDrawable::GradientState::~GradientState(){
    //delete mTint;
    //delete mStrokeColors;
    //delete mSolidColors;
}

void GradientDrawable::GradientState::setDensity(int targetDensity) {
    if (mDensity != targetDensity) {
        const int sourceDensity = mDensity;
        mDensity = targetDensity;
        applyDensityScaling(sourceDensity, targetDensity);
    }
}

bool GradientDrawable::GradientState::hasCenterColor()const {
    return mGradientColors.size() == 3;
}

void GradientDrawable::GradientState::applyDensityScaling(int sourceDensity, int targetDensity) {
    if (mInnerRadius > 0)mInnerRadius = Drawable::scaleFromDensity(mInnerRadius, sourceDensity, targetDensity, true);

    if (mThickness > 0)  mThickness = Drawable::scaleFromDensity(mThickness, sourceDensity, targetDensity, true);

    if (mOpticalInsets != Insets::NONE) {
        const int left  = Drawable::scaleFromDensity(mOpticalInsets.left, sourceDensity, targetDensity, true);
        const int top   = Drawable::scaleFromDensity( mOpticalInsets.top, sourceDensity, targetDensity, true);
        const int right = Drawable::scaleFromDensity(mOpticalInsets.right, sourceDensity, targetDensity, true);
        const int bottom= Drawable::scaleFromDensity(mOpticalInsets.bottom, sourceDensity, targetDensity, true);
        mOpticalInsets.set(left, top, right, bottom);
    }
    if (mPadding.empty()) {
        mPadding.left  = Drawable::scaleFromDensity(mPadding.left, sourceDensity, targetDensity, false);
        mPadding.top   = Drawable::scaleFromDensity(mPadding.top, sourceDensity, targetDensity, false);
        mPadding.width = Drawable::scaleFromDensity(mPadding.width, sourceDensity, targetDensity, false);
        mPadding.height= Drawable::scaleFromDensity(mPadding.height, sourceDensity, targetDensity, false);
    }
    if (mRadius > 0)    mRadius = Drawable::scaleFromDensity(mRadius, sourceDensity, targetDensity);
    if (mRadiusArray.size()) {
        mRadiusArray[0] = Drawable::scaleFromDensity(static_cast<int>(mRadiusArray[0]), sourceDensity, targetDensity, true);
        mRadiusArray[1] = Drawable::scaleFromDensity(static_cast<int>(mRadiusArray[1]), sourceDensity, targetDensity, true);
        mRadiusArray[2] = Drawable::scaleFromDensity(static_cast<int>(mRadiusArray[2]), sourceDensity, targetDensity, true);
        mRadiusArray[3] = Drawable::scaleFromDensity(static_cast<int>(mRadiusArray[3]), sourceDensity, targetDensity, true);
    }
    if (mStrokeWidth > 0)  mStrokeWidth = Drawable::scaleFromDensity(mStrokeWidth, sourceDensity, targetDensity, true);
    if (mStrokeDashWidth>0)mStrokeDashWidth = Drawable::scaleFromDensity(mStrokeDashGap, sourceDensity, targetDensity);

    if (mStrokeDashGap > 0)mStrokeDashGap = Drawable::scaleFromDensity(mStrokeDashGap, sourceDensity, targetDensity);

    if (mGradientRadiusType == RADIUS_TYPE_PIXELS)
        mGradientRadius = Drawable::scaleFromDensity(mGradientRadius, sourceDensity, targetDensity);

    if (mWidth > 0) mWidth = Drawable::scaleFromDensity(mWidth, sourceDensity, targetDensity, true);
    if (mHeight > 0)mHeight= Drawable::scaleFromDensity(mHeight, sourceDensity, targetDensity, true);
}


GradientDrawable* GradientDrawable::GradientState::newDrawable() {
    return new GradientDrawable(shared_from_this());
}

int GradientDrawable::GradientState::getChangingConfigurations()const {
    return 0;
}

void GradientDrawable::GradientState::setShape( int shape) {
    mShape = shape;
    computeOpacity();
}

void GradientDrawable::GradientState::setSolidColors(const ColorStateList*colors) {
    mGradientColors.clear();
    if(mSolidColors!=colors){
        mSolidColors = colors;
    }
    computeOpacity();
}

void GradientDrawable::GradientState::setGradientType(int gradient) {
    mGradient = gradient;
}

void GradientDrawable::GradientState::setGradientCenter(float x, float y) {
    mCenterX = x;
    mCenterY = y;
}

void GradientDrawable::GradientState::setGradientColors(const std::vector<int>& colors) {
    mGradientColors = colors;
    mSolidColors = nullptr;
    computeOpacity();
}

void GradientDrawable::GradientState::setImagePattern(Cairo::RefPtr<Cairo::ImageSurface>image){
    mImagePattern = image;
}

void GradientDrawable::GradientState::computeOpacity() {
    mOpaqueOverBounds = false;
    mOpaqueOverShape = false;

    for (auto gc:mGradientColors) {
        if (!isOpaque(gc)) {
            return;
        }
    }
    // An unfilled shape is not opaque over bounds or shape
    if ((mGradientColors.size()==0) && (mSolidColors == nullptr)) {
        return;
    }
    if(mImagePattern){
        const int transparent=ImageDecoder::getTransparency(mImagePattern);
        if(transparent!=PixelFormat::OPAQUE)return;
    }
    // Colors are opaque, so opaqueOverShape=true,
    mOpaqueOverShape = true;
    // and opaqueOverBounds=true if shape fills bounds
    mOpaqueOverBounds = (mShape == RECTANGLE) && (mRadius <= 0)  && mRadiusArray.empty();
}

void GradientDrawable::GradientState::setStroke(int width,const ColorStateList*colors, float dashWidth,float dashGap) {
    mStrokeWidth = width;
    if(mStrokeColors!=colors){
        mStrokeColors = colors;
    }
    mStrokeDashWidth = dashWidth;
    mStrokeDashGap = dashGap;
    computeOpacity();
}

void GradientDrawable::GradientState::setCornerRadius(float radius) {
    if (radius < 0) radius = 0;
    mRadius = radius;
    mRadiusArray.clear();// = null;
}

void GradientDrawable::GradientState::setCornerRadii(const std::vector<float>& radii) {
    mRadiusArray = radii;
    if (radii.size() == 0)
        mRadius = 0;
}

void GradientDrawable::GradientState::setSize(int width, int height) {
    mWidth = width;
    mHeight = height;
}

void GradientDrawable::GradientState::setGradientRadius(float gradientRadius,int type) {
    mGradientRadius    = gradientRadius;
    mGradientRadiusType= type;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

GradientDrawable::GradientDrawable()
    :GradientDrawable(std::make_shared<GradientState>()) {
}

GradientDrawable::GradientDrawable(std::shared_ptr<GradientState>state) {
    mPathIsDirty = mGradientIsDirty = false;
    mPadding.set(0,0,0,0);
    mGradientState = state;
    mMutated = false;
    mGradientRadius = 0.5f;
    mStrokeWidth =-1;
    mAlpha = 255;
    updateLocalState();
}

GradientDrawable::GradientDrawable(Orientation orientation,const std::vector<int>&colors)
    :GradientDrawable(std::make_shared<GradientState>(orientation,colors)) {
}

std::shared_ptr<Drawable::ConstantState>GradientDrawable::getConstantState() {
    return mGradientState;
}

void GradientDrawable::updateLocalState() {
    mPathIsDirty = true;
    mGradientIsDirty = true;
    auto state = mGradientState;
    if(state->mSolidColors) {
        const std::vector<int> currentState = getState();
        const int stateColor = state->mSolidColors->getColorForState(currentState,0);
        Color c(stateColor);
        mFillPaint = SolidPattern::create_rgba(c.red(),c.green(),c.blue(),(c.alpha()*mAlpha)/255.f);
    } else if(state->mGradientColors.size()) {
        //mFillPaint = SolidPattern::create_rgba(0,0,0,0)
        ensureValidRect();
    } else if(state->mImagePattern){
        //mFillPaint = SurfacePattern::create(state->mImagePattern);
    }
    mPadding = state->mPadding;
    if(state->mStrokeWidth>=0) {
        int strokeStateColor = 0;
        if(state->mStrokeColors) {
            const std::vector<int>currentState = getState();
            auto cls = state->mStrokeColors;
            strokeStateColor = cls->getColorForState(currentState,0);
        }
        setStroke(state->mStrokeWidth,strokeStateColor,state->mStrokeDashWidth,state->mStrokeDashGap);
    }
    state->computeOpacity();
}

GradientDrawable* GradientDrawable::mutate() {
    if (!mMutated && Drawable::mutate() == this) {
        mGradientState=std::make_shared<GradientState>(*mGradientState);
        updateLocalState();
        mMutated = true;
    }
    return this;
}

void GradientDrawable::clearMutated() {
    Drawable::clearMutated();
    mMutated = false;
}

bool GradientDrawable::getPadding(Rect& padding) {
    if ((mPadding.left>0)||(mPadding.top>0)||(mPadding.width>0)||(mPadding.height>0)) {
        padding=mPadding;
        return true;
    } else {
        return Drawable::getPadding(padding);
    }
}

void GradientDrawable::setCornerRadii(const std::vector<float>& radii) {
    mGradientState->setCornerRadii(radii);
    mPathIsDirty = true;
    invalidateSelf();
}

const std::vector<float>&GradientDrawable::getCornerRadii()const {
    return mGradientState->mRadiusArray;
}

void GradientDrawable::setCornerRadius(float radius) {
    mGradientState->setCornerRadius(radius);
    mPathIsDirty = true;
    invalidateSelf();
}

float GradientDrawable::getCornerRadius()const {
    return mGradientState->mRadius;
}

void GradientDrawable::setStroke(int width,int color) {
    setStroke(width, color, 0, 0);
}

void GradientDrawable::setStroke(int width, const ColorStateList*colorStateList) {
    setStroke(width, colorStateList, 0, 0);
}

void GradientDrawable::setStroke(int width,int color, float dashWidth, float dashGap) {
    mGradientState->setStroke(width, ColorStateList::valueOf(color), dashWidth, dashGap);
    setStrokeInternal(width, color, dashWidth, dashGap);
}

void GradientDrawable::setStroke(int width,const ColorStateList* colorStateList, float dashWidth, float dashGap) {
    mGradientState->setStroke(width, colorStateList, dashWidth, dashGap);
    int color;
    if (colorStateList == nullptr) {
        color = Color::TRANSPARENT;
    } else {
        const std::vector<int>& stateSet = getState();
        color = colorStateList->getColorForState(stateSet,0);
    }
    setStrokeInternal(width, color, dashWidth, dashGap);
}

void GradientDrawable::setStrokeInternal(int width, int color, float dashWidth, float dashGap) {
    Color c(color);
    mStrokePaint = SolidPattern::create_rgba(c.red(),c.green(),c.blue(),(c.alpha()*mAlpha)/255.f);
    mStrokeWidth = width;
    mDashArray = std::vector<double> {dashWidth,dashGap};
    invalidateSelf();
}

void GradientDrawable::setInnerRadiusRatio(float innerRadiusRatio) {
    if(innerRadiusRatio<=0)
        return ;
    mGradientState->mInnerRadiusRatio = innerRadiusRatio;
    mPathIsDirty = true;
    invalidateSelf();
}

float GradientDrawable::getInnerRadiusRatio()const {
    return mGradientState->mInnerRadiusRatio;
}

void GradientDrawable::setInnerRadius(int innerRadius) {
    mGradientState->mInnerRadius = innerRadius;
    mPathIsDirty = true;
    invalidateSelf();
}

int  GradientDrawable::getInnerRadius()const {
    return mGradientState->mInnerRadius;
}

void GradientDrawable::setThicknessRatio(float thicknessRatio) {
    if(thicknessRatio<=0)
        return ;
    mGradientState->mThicknessRatio = thicknessRatio;
    mPathIsDirty = true;
    invalidateSelf();
}

float GradientDrawable::getThicknessRatio()const {
    return mGradientState->mThicknessRatio;
}

void GradientDrawable::setThickness(int thickness) {
    mGradientState->mThickness = thickness;
    mPathIsDirty = true;
    invalidateSelf();
}

int  GradientDrawable::getThickness()const {
    return mGradientState->mThickness;
}

void GradientDrawable::setPadding(int left,int top,int right,int bottom) {
    mGradientState->mPadding.set(left, top, right, bottom);
    mPadding = mGradientState->mPadding;
    invalidateSelf();
}

void GradientDrawable::setShape(/*@Shape*/ int shape) {
    //mRingPath = nullptr;
    mPathIsDirty = true;
    mGradientState->setShape(shape);
    invalidateSelf();
}

int GradientDrawable::getShape()const {
    return mGradientState->mShape;
}

int GradientDrawable::getIntrinsicWidth()const {
    return mGradientState->mWidth;
}

int GradientDrawable::getIntrinsicHeight()const {
    return mGradientState->mHeight;
}

Insets GradientDrawable::getOpticalInsets() {
    return mGradientState->mOpticalInsets;
}

void GradientDrawable::setSize(int width, int height) {
    mGradientState->setSize(width, height);
    mPathIsDirty = true;
    invalidateSelf();
    LOGV("%p setSize(%d,%d)",this,width,height);
}

void GradientDrawable::setGradientType(int gradient) {
    mGradientState->setGradientType(gradient);
    mGradientIsDirty = true;
    invalidateSelf();
}

int GradientDrawable::getGradientType()const {
    return mGradientState->mGradient;
}

void GradientDrawable::setGradientCenter(float x, float y) {
    mGradientState->setGradientCenter(x, y);
    mGradientIsDirty = true;
    invalidateSelf();
}

float GradientDrawable::getGradientCenterX()const {
    return mGradientState->mCenterX;
}

float GradientDrawable::getGradientCenterY()const {
    return mGradientState->mCenterY;
}

void  GradientDrawable::setGradientRadius(float gradientRadius) {
    mGradientState->setGradientRadius(gradientRadius, 0);//TypedValue.COMPLEX_UNIT_PX);
    mGradientIsDirty = true;
    invalidateSelf();
}

float GradientDrawable::getGradientRadius() {
    if (mGradientState->mGradient != RADIAL_GRADIENT) {
        return 0;
    }
    ensureValidRect();
    return mGradientRadius;
}

void GradientDrawable::setUseLevel(bool useLevel) {
    mGradientState->mUseLevel = useLevel;
    mGradientIsDirty = true;
    invalidateSelf();
}

bool GradientDrawable::getUseLevel()const {
    return mGradientState->mUseLevel;
}

int GradientDrawable::modulateAlpha(int alpha) {
    int scale = mAlpha + (mAlpha >> 7);
    return alpha * scale >> 8;
}

GradientDrawable::Orientation GradientDrawable::getOrientation()const {
    return mGradientState->mOrientation;
}

void GradientDrawable::setOrientation(Orientation orientation) {
    mGradientState->mOrientation = orientation;
    mGradientIsDirty = true;
    invalidateSelf();
}

void GradientDrawable::setColors(const std::vector<int>& colors) {
    mGradientState->setGradientColors(colors);
    switch(colors.size()) {
    case 1:
        LOGE("gradient must has at least 2 color");
        break;
    case 2:
        mGradientState->mPositions=std::vector<float> {.0f,1.f};
        break;
    case 3:
        mGradientState->mPositions=std::vector<float> {.0f,.5f,1.f};
        break;
    }
    mGradientIsDirty = true;
    invalidateSelf();
}

void GradientDrawable::setColors(const std::vector<int>&colors,const std::vector<float>&offsets) {
    mGradientState->setGradientColors(colors);
    mGradientState->mPositions = offsets;
    mGradientIsDirty = true;
    invalidateSelf();
}

const std::vector<int>&GradientDrawable::getColors()const {
    return mGradientState->mGradientColors;
}

void GradientDrawable::setImagePattern(Cairo::RefPtr<Cairo::ImageSurface>image){
    mGradientState->mImagePattern = image;
}

void GradientDrawable::setImagePattern(Context*ctx,const std::string&res){
    if(ctx)setImagePattern(ctx->loadImage(res));
}

void GradientDrawable::buildPathIfDirty() {
    if (mPathIsDirty) {
        ensureValidRect();
        //mPath->reset();
        //mPath->round_rectangle(mRect,mGradientState->mRadiusArray);
        mPathIsDirty = false;
    }
}

void GradientDrawable::setColor(int argb) {
    Color c(argb);
    mGradientState->setSolidColors(ColorStateList::valueOf(argb));
    mFillPaint = SolidPattern::create_rgba(c.red(),c.green(),c.blue(),c.alpha());
    invalidateSelf();
}

void GradientDrawable::setColor(const ColorStateList* colorStateList) {
    mGradientState->setSolidColors(colorStateList);
    int color;
    if (colorStateList == nullptr) {
        color = Color::TRANSPARENT;
    } else {
        const std::vector<int>& stateSet = getState();
        color = colorStateList->getColorForState(stateSet,0);
    }
    Color c(color);
    mFillPaint=SolidPattern::create_rgba(c.red(),c.green(),c.blue(),c.alpha());
    invalidateSelf();
}

const ColorStateList* GradientDrawable::getColor() {
    return mGradientState->mSolidColors;
}

bool GradientDrawable::onStateChange(const std::vector<int>& stateSet) {
    bool bInvalidateSelf = false;

    GradientState& s = *mGradientState;
    const ColorStateList* solidColors = s.mSolidColors;
    double r,g,b,a;
    if (solidColors != nullptr) {
        RefPtr<Cairo::SolidPattern>pat = std::dynamic_pointer_cast<Cairo::SolidPattern>(mFillPaint);
        pat->get_rgba(r,g,b,a);
        const int newColor = solidColors->getColorForState(stateSet, 0);
        const int oldColor = Color::toArgb((float)r,(float)g,(float)b,(float)a);
        if (oldColor != newColor) {
            Color cc(newColor);
            mFillPaint = SolidPattern::create_rgba((float)cc.red(),(float)cc.green(),(float)cc.blue(),(float)cc.alpha());
            bInvalidateSelf = true;
        }
    }

    if (mStrokePaint != nullptr) {
        const ColorStateList* strokeColors = s.mStrokeColors;
        if (strokeColors ) {
            RefPtr<Cairo::SolidPattern>pat = std::dynamic_pointer_cast<Cairo::SolidPattern>(mStrokePaint);
            pat->get_rgba(r,g,b,a);
            const int newColor = strokeColors->getColorForState(stateSet, 0);
            const int oldColor = Color::toArgb((float)r,(float)g,(float)b,(float)a);//mStrokePaint->getColor();
            if (oldColor != newColor) {
                Color cc(newColor);
                mStrokePaint =SolidPattern::create_rgba((float)cc.red(),(float)cc.green(),(float)cc.blue(),(float)cc.alpha());
                bInvalidateSelf = true;
            }
        }
    }

    /*if (s->mTint != nullptr && s.mBlendMode != null) {
        mBlendModeColorFilter = updateBlendModeFilter(mBlendModeColorFilter, s.mTint, s.mBlendMode);
        invalidateSelf = true;
    }*/

    if (bInvalidateSelf) {
        invalidateSelf();
        return true;
    }

    return false;
}

bool GradientDrawable::isStateful()const {
    GradientState&s =*mGradientState;
    return Drawable::isStateful()
           || (s.mSolidColors  && s.mSolidColors->isStateful())
           || (s.mStrokeColors && s.mStrokeColors->isStateful())
           || (s.mTint && s.mTint->isStateful());
}

bool GradientDrawable::hasFocusStateSpecified()const {
    GradientState& s = *mGradientState;
    return (s.mSolidColors && s.mSolidColors->hasFocusStateSpecified())
           || (s.mStrokeColors&& s.mStrokeColors->hasFocusStateSpecified())
           || (s.mTint && s.mTint->hasFocusStateSpecified());
}

int  GradientDrawable::getChangingConfigurations()const {
    return Drawable::getChangingConfigurations() | mGradientState->getChangingConfigurations();
}

void  GradientDrawable::setAlpha(int alpha) {
    if (alpha != mAlpha) {
        mAlpha = alpha;
        updateLocalState();
        invalidateSelf();
    }
}

int GradientDrawable::getAlpha()const {
    return mAlpha;
}

void GradientDrawable::setDither(bool dither) {
    if (dither != mGradientState->mDither) {
        mGradientState->mDither = dither;
        invalidateSelf();
    }
}

int GradientDrawable::getOpacity()const {
    return (mAlpha == 255 && mGradientState->mOpaqueOverBounds && isOpaqueForState()) ?
           OPAQUE : TRANSLUCENT;
}

void GradientDrawable::setColorFilter(ColorFilter*colorFilter){
    if(colorFilter !=mColorFilter){
        mColorFilter = colorFilter;
	invalidateSelf();
    }
}

ColorFilter*GradientDrawable::getColorFilter(){
    return mColorFilter;
}

void GradientDrawable::setTintList(const ColorStateList*tint){
    mGradientState->mTint = tint;
    invalidateSelf();
}

void GradientDrawable::onBoundsChange(const Rect& r) {
    Drawable::onBoundsChange(r);
    //mRingPath = null;
    mPathIsDirty = true;
    mGradientIsDirty = true;
}

bool GradientDrawable::onLevelChange(int level) {
    Drawable::onLevelChange(level);
    mGradientIsDirty = true;
    mPathIsDirty = true;
    invalidateSelf();
    return true;
}

static double distance(float x1, float y1, float x2, float y2) {
    return std::sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
}

static double getRadius(const RectF& r, int x, int y) {
    PointF topLeft = {r.left, r.top};
    PointF topRight = {r.left + r.width, r.top};
    PointF bottomRight = {r.left + r.width, r.top + r.height};
    PointF bottomLeft = {r.left, r.top + r.height};

    double dist1 = distance(float(x), float(y), topLeft.x, topLeft.y);
    double dist2 = distance(float(x), float(y), topRight.x, topRight.y);
    double dist3 = distance(float(x), float(y), bottomRight.x, bottomRight.y);
    double dist4 = distance(float(x), float(y), bottomLeft.x, bottomLeft.y);

    return std::max({dist1, dist2, dist3, dist4});
}

bool GradientDrawable::ensureValidRect() {
    if (mGradientIsDirty) {
        mGradientIsDirty = false;
        Rect bounds = getBounds();
        float inset = 0;

        if (mStrokePaint)inset = static_cast<float>(mStrokeWidth*0.5f);

        GradientState&st =*mGradientState;
        mRect.set(bounds.left + inset, bounds.top + inset, bounds.width - 2*inset, bounds.height - 2*inset);

        const std::vector<int>&gradientColors = st.mGradientColors;
        if (gradientColors.size()) {
            const RectF r = mRect;
            float x0, y0;

            if (st.mGradient == LINEAR_GRADIENT) {
                float x1,y1;
                const float level = st.mUseLevel ? getLevel() / 10000.0f : 1.0f;
                switch (st.mOrientation) {
                case TOP_BOTTOM:
                    x0 = r.left;
                    y0 = r.top;
                    x1 = x0;
                    y1 = level *r.height;
                    break;
                case TR_BL:
                    x0 = r.width;
                    y0 = r.top;
                    x1 = level * r.left;
                    y1 = level *r.height;
                    break;
                case RIGHT_LEFT:
                    x0 = r.width;
                    y0 = r.top;
                    x1 = level * r.left;
                    y1 = y0;
                    break;
                case BR_TL:
                    x0 = r.width;
                    y0 = r.height;
                    x1 = level * r.left;
                    y1 = level * r.top;
                    break;
                case BOTTOM_TOP:
                    x0 = r.left;
                    y0 = r.height;
                    x1 = x0;
                    y1 = level * r.top;
                    break;
                case BL_TR:
                    x0 = r.left;
                    y0 = r.height;
                    x1 = level * r.width;
                    y1 = level * r.top;
                    break;
                case LEFT_RIGHT:
                    x0 = r.left;
                    y0 = r.top;
                    x1 = level * r.width;
                    y1 = y0;
                    break;
                default:/*TL_BR*/
                    x0 = r.left;
                    y0 = r.top;
                    x1 = level * r.width;
                    y1 = level *r.height;
                    break;
                }
                RefPtr<Cairo::LinearGradient>pat=LinearGradient::create(x0, y0, x1, y1);
                //gradientColors, st.mPositions, Shader.TileMode.CLAMP));
                for(int i=0; i<gradientColors.size(); i++) {
                    Color c((uint32_t)gradientColors[i]);
                    pat->add_color_stop_rgba(st.mPositions[i],c.red(),c.green(),c.blue(),(c.alpha()*mAlpha)/255.f);
                }
                mFillPaint = pat;
            } else if (st.mGradient == RADIAL_GRADIENT) {
                x0 = r.left + r.width* st.mCenterX;
                y0 = r.top + r.height * st.mCenterY;

                float radius = st.mGradientRadius;
                if (st.mGradientRadiusType == RADIUS_TYPE_FRACTION) {
                    // Fall back to parent width or height if intrinsic size is not specified.
                    const float width = st.mWidth >= 0 ? st.mWidth   :r.width;
                    const float height = st.mHeight >= 0? st.mHeight : r.height;
                    radius *= std::min(width, height);
                } else if (st.mGradientRadiusType == RADIUS_TYPE_FRACTION_PARENT) {
                    radius *= std::min(r.width, r.height);
                }
                if (st.mUseLevel) radius *= getLevel() / 10000.0f;

                mGradientRadius = radius;

                if (radius <= 0) {
                    // We can't have a shader with non-positive radius, so
                    // let's have a very, very small radius.
                    radius = 0.001f;
                }
                RefPtr<Cairo::RadialGradient>pat = RadialGradient::create( x0, y0, radius,x0,y0,0);
                for(int i=0; i<gradientColors.size(); i++) {
                    Color c((uint32_t)gradientColors[i]);
                    pat->add_color_stop_rgba(st.mPositions[i],c.red(),c.green(),c.blue(),(c.alpha()*mAlpha)/255.f);
                }//gradientColors, null, Shader.TileMode.CLAMP));
                mFillPaint = pat;
            } else if (st.mGradient == SWEEP_GRADIENT) {
                x0 = mRect.left+ mRect.width * st.mCenterX;
                y0 = mRect.top + mRect.height * st.mCenterY;
                const double RADIUS = getRadius(mRect,x0,y0);
                std::vector<Cairo::ColorStop> stops;
                for(int i=0; i<gradientColors.size(); i++) {
                    Color c = gradientColors[i];
                    stops.push_back({0,c.red(),c.green(),c.blue(),(c.alpha()*mAlpha)/255.f});
                }
                RefPtr<SweepGradient>pat = SweepGradient::create(x0, y0,RADIUS,M_PI/2.f,stops);
                mFillPaint = pat;
            } else if(st.mGradient = BITMAP_PATTERN){
                //mFillPaint = SurfacePattern::create(st.mImagePattern);
            }

            // If we don't have a solid color, the alpha channel must be
            // maxed out so that alpha modulation works correctly.
            //if (st.mSolidColors == nullptr)
            //    mFillPaint=SolidPattern::create_rgb(0,0,0);//setColor(Color.BLACK);
        } else{ //gradientColors.size()==0
            //transparent ,mFillPaint=nullptr
        }
        LOGE_IF((mFillPaint==nullptr)&&(mStrokePaint==nullptr)&&(st.mImagePattern==nullptr),"stroke and solid must be setted one or both of them");
    }
    return !mRect.empty();
}

bool GradientDrawable::isOpaqueForState()const {
    if (mGradientState->mStrokeWidth >= 0 && mStrokePaint ) {
        double r,g,b,a;
        RefPtr<Cairo::SolidPattern>pat = std::dynamic_pointer_cast<Cairo::SolidPattern>(mStrokePaint);
        pat->get_rgba(r,g,b,a);
        if(a!=1.f)return false;
    }
    // Don't check opacity if we're using a gradient, as we've already
    // checked the gradient opacity in mOpaqueOverShape.
    if (mGradientState->mGradientColors.size() == 0) {
        double r,g,b,a;
        RefPtr<Cairo::SolidPattern>pat = std::dynamic_pointer_cast<Cairo::SolidPattern>(mFillPaint);
        pat->get_rgba(r,g,b,a);
        if(a!=1.f)return false;
    }
    return true;
}

void GradientDrawable::getPatternAlpha(int& strokeAlpha,int& fillApha){
    strokeAlpha=255;
    fillApha = 255;
    if ((mGradientState->mStrokeWidth >= 0) && mStrokePaint ) {
        double r,g,b,a;
        RefPtr<Cairo::SolidPattern>pat = std::dynamic_pointer_cast<Cairo::SolidPattern>(mStrokePaint);
        pat->get_rgba(r,g,b,a);
        strokeAlpha = int(255.f*a);
    }
    // Don't check opacity if we're using a gradient, as we've already
    // checked the gradient opacity in mOpaqueOverShape.
    if (mFillPaint && (mGradientState->mGradientColors.size() == 0)) {
        double r,g,b,a;
        RefPtr<Cairo::SolidPattern>pat = std::dynamic_pointer_cast<Cairo::SolidPattern>(mFillPaint);
        pat->get_rgba(r,g,b,a);
        fillApha = int(255.f*a);
    }
}

static double min3(double a,double b,double c){
    return std::min(a,std::min(b,c));
}
static void drawRoundedRect(Canvas& cr,const RectF&rect,
                double topLeftRadius, double topRightRadius, double bottomRightRadius, double bottomLeftRadius) {
    const double x = rect.left;
	const double y = rect.top;
	const double width = rect.width;
	const double height= rect.height;
#if 0
    const double maxRadius = std::min(width,height);
    const double maxTopRadius = std::min(maxRadius, topLeftRadius + topRightRadius);
    const double maxBottomRadius = std::min(maxRadius, bottomLeftRadius + bottomRightRadius);
    const double maxLeftRadius = std::min(maxRadius, topLeftRadius + bottomLeftRadius);
    const double maxRightRadius = std::min(maxRadius, topRightRadius + bottomRightRadius);

    const double topLeftRadiusClipped = std::min(topLeftRadius, std::min(maxTopRadius, maxLeftRadius));
    const double topRightRadiusClipped = std::min(topRightRadius, std::min(maxTopRadius, maxRightRadius));
    const double bottomLeftRadiusClipped = std::min(bottomLeftRadius, std::min(maxBottomRadius, maxLeftRadius));
    const double bottomRightRadiusClipped = std::min(bottomRightRadius, std::min(maxBottomRadius, maxRightRadius));
#else
    const double topLeftRadiusClipped = (topLeftRadius<=FLT_EPSILON)?0:min3(topLeftRadius,
            width*topLeftRadius/(topLeftRadius+topRightRadius), height*topLeftRadius/(topLeftRadius+bottomLeftRadius));
    const double topRightRadiusClipped = (topRightRadius<=FLT_EPSILON)?0:min3(topRightRadius,
            width*topRightRadius/(topLeftRadius+topRightRadius), height*topRightRadius/(topRightRadius+bottomRightRadius));
    const double bottomLeftRadiusClipped = (bottomLeftRadius<=FLT_EPSILON)?0:min3(bottomLeftRadius,
            width*bottomLeftRadius/(bottomLeftRadius+bottomRightRadius), height*bottomLeftRadius/(topLeftRadius+bottomLeftRadius));
    const double bottomRightRadiusClipped = (bottomRightRadius<=FLT_EPSILON)?0:min3(bottomRightRadius,
            width*bottomRightRadius/(bottomLeftRadius+bottomRightRadius), height*bottomRightRadius/(topRightRadius+bottomRightRadius));    
#endif
    cr.move_to(x + topLeftRadiusClipped, y);
    cr.line_to(x + width - topRightRadiusClipped, y);
    cr.arc(x + width - topRightRadiusClipped, y + topRightRadiusClipped, topRightRadiusClipped, -M_PI_2, 0);
    cr.line_to(x + width, y + height - bottomRightRadiusClipped);
    cr.arc(x + width - bottomRightRadiusClipped, y + height - bottomRightRadiusClipped, bottomRightRadiusClipped, 0, M_PI_2);
    cr.line_to(x + bottomLeftRadiusClipped, y + height);
    cr.arc(x + bottomLeftRadiusClipped, y + height - bottomLeftRadiusClipped, bottomLeftRadiusClipped, M_PI_2, M_PI);
    cr.line_to(x, y + topLeftRadiusClipped);
    cr.arc(x + topLeftRadiusClipped, y + topLeftRadiusClipped, topLeftRadiusClipped, M_PI, -M_PI_2);
    cr.close_path();
}

void GradientDrawable::prepareStrokeProps(Canvas&canvas) {
    if(mGradientState->mStrokeWidth>0)
        canvas.set_line_width(mGradientState->mStrokeWidth);
    if((mGradientState->mStrokeDashWidth!=.0f)&&(mGradientState->mStrokeDashGap!=.0f))
        canvas.set_dash(std::vector<double> {mGradientState->mStrokeDashWidth,mGradientState->mStrokeDashGap},0);
    canvas.set_source(mStrokePaint);
}

void GradientDrawable::draw(Canvas&canvas) {
    if (!ensureValidRect())return; // nothing to draw
    auto st = mGradientState;
    const ColorFilter* colorFilter = mColorFilter;// ? mColorFilter : mTintFilter;
    int preStrokeAlpha,preFillAlpha;
    getPatternAlpha(preStrokeAlpha,preFillAlpha);
    const int currStrokeAlpha = modulateAlpha(preStrokeAlpha);
    const int currFillAlpha = modulateAlpha(preFillAlpha);

    const bool haveStroke = currStrokeAlpha > 0 && mStrokePaint &&  mStrokeWidth> 0;
    const bool haveFill = currFillAlpha>0;
    /*const bool useLayer =(haveStroke || haveFill) && (st->mShape!=LINE) &&
	    (currStrokeAlpha<255) && ((mAlpha<255)|| colorFilter);*/
    const float sweep = st->mUseLevelForShape ? (360.f*getLevel()/10000.f) : 360.f;
    const Pattern::Dither ditherMode = mGradientState->mDither
               ? Pattern::Dither::GOOD : Pattern::Dither::DEFAULT;
    float rad = .0f;

    std::vector<float>radii;
    if( (mFillPaint==nullptr) && (st->mImagePattern==nullptr) && (haveStroke==false) )return;
    
    canvas.save();
    if(mFillPaint)
        mFillPaint->set_dither(ditherMode);
    switch (st->mShape) {
    case RECTANGLE:
        rad = std::min(st->mRadius,std::min(mRect.width, mRect.height) * 0.5f);
        if(st->mRadiusArray.size())radii = st->mRadiusArray;
        if(st->mRadius > 0.0f)radii = {rad,rad,rad,rad};
        if(radii.size())
            drawRoundedRect(canvas,mRect,radii[0],radii[1],radii[2],radii[3]);
        else
            canvas.rectangle(int(mRect.left),int(mRect.top),int(mRect.width),int(mRect.height));

        break;
    case LINE:
        if (haveStroke) {
            const float y = mRect.top + mRect.height/2.f;
            prepareStrokeProps(canvas);
            canvas.move_to(mRect.left, y);
            canvas.line_to(mRect.left+mRect.width, y);
            canvas.stroke();
        }
        break;
    case OVAL:
        rad = mRect.height/2.f;
        canvas.scale(float(mRect.width)/mRect.height,1.f);
        LOGV("%p size=%.fx%.f radius=%f strokewidth=%d",this,mRect.width,mRect.height,(float)st->mRadius,st->mStrokeWidth);
        canvas.begin_new_sub_path();
        canvas.arc(mRect.centerX(),mRect.centerY(),rad,0,M_PI*2.f*(getUseLevel()?(float)getLevel()/10000.f:1));
		
        break;
    case RING: {
        //inner
        float innerRadius = float(st->mInnerRadius);
        RectF bounds= {mRect.left,mRect.top,mRect.width,mRect.height};
        float thickness = st->mThickness!=-1 ? st->mThickness:(bounds.width/st->mThicknessRatio);
        float radius = st->mInnerRadius!=-1 ? st->mInnerRadius :(bounds.width/st->mInnerRadiusRatio);
        canvas.scale(bounds.width/bounds.height,1.f);
        RectF innerBounds = bounds;
        const float x = bounds.centerX();
        const float y = bounds.centerY();
        if(innerRadius<=0.f)
            innerRadius=std::min(mRect.width,mRect.height)/2.f-thickness;
        canvas.begin_new_sub_path();
        if( sweep<360.f && sweep>-360.f ) {
            const double end_angle = M_PI*2*sweep/360.f;
            canvas.set_fill_rule(Cairo::Context::FillRule::WINDING);//EVEN_ODD);//WINDING);
            canvas.move_to(x + radius,y);
            canvas.arc(x,y,radius + thickness,0.f,end_angle);
            canvas.arc_negative(x,y,radius,end_angle,0.f);
            canvas.close_path();
        } else {
            //canvas.set_fill_rule(Cairo::Context::FillRule::EVEN_ODD);
            canvas.arc(x,y,radius + thickness,0,M_PI*2.f);
            canvas.begin_new_sub_path();
            canvas.arc_negative(x,y,radius,M_PI*2.f,0.f);
        }
        }break;
    }/*endof switch*/

    if(st->mShape!=LINE){
        const bool needFill = (mFillPaint!=nullptr)||(st->mImagePattern!=nullptr);
        if(mFillPaint)canvas.set_source(mFillPaint);
        else if(st->mImagePattern)canvas.set_source(st->mImagePattern,0,0);
        if(needFill){
            if(haveStroke) canvas.fill_preserve();
            else canvas.fill();
        }
        if (haveStroke) {
            prepareStrokeProps(canvas);
            canvas.stroke();
        }
    }
    canvas.restore();
}

Drawable*GradientDrawable::inflate(Context*ctx,const AttributeSet&atts) {
    GradientDrawable*d = new GradientDrawable();
    const int shapeType = atts.getInt("shape",std::map<const std::string,int> {
        {"rectangle",GradientDrawable::Shape::RECTANGLE},{"oval",GradientDrawable::Shape::OVAL},
        {"line",GradientDrawable::Shape::LINE},          {"ring",GradientDrawable::Shape::RING}
    },GradientDrawable::Shape::RECTANGLE);
    d->setShape(shapeType);

    if(shapeType == GradientDrawable::Shape::RING) {
        int tmp = atts.getDimensionPixelSize("innerRadius",-1);
        d->setInnerRadius(tmp);
        if(tmp == -1){
            d->setInnerRadiusRatio(atts.getDimensionPixelSize("innerRadiusRatio",-1));
        }
        tmp = atts.getDimensionPixelSize("thickness",-1);
        d->setThickness(tmp);
        if( tmp == -1){
            d->setThicknessRatio(atts.getDimensionPixelSize("thicknessRatio"));
        }
        d->mGradientState->mUseLevelForShape=atts.getBoolean("useLevel");
    }
    auto state = d->mGradientState;
    const int insetLeft = atts.getDimensionPixelSize("opticalInsetLeft", state->mOpticalInsets.left);
    const int insetTop = atts.getDimensionPixelSize("opticalInsetTop", state->mOpticalInsets.top);
    const int insetRight = atts.getDimensionPixelSize("opticalInsetRight", state->mOpticalInsets.right);
    const int insetBottom = atts.getDimensionPixelSize("opticalInsetBottom", state->mOpticalInsets.bottom);
    state->mOpticalInsets.set(insetLeft, insetTop, insetRight, insetBottom);
    return d;
}

}//end namespace
