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
#include <widget/internal_R.h>
#include <drawable/gradientdrawable.h>
#include <image-decoders/imagedecoder.h>
#include <cairomm/mesh_pattern.h>
#include <cfloat>
#include <color.h>
#include <cdlog.h>
#include <widget/framework_styleable.h>
using namespace Cairo;
namespace cdroid {
using namespace cdroid::internal;

GradientDrawable::GradientState::GradientState() {
    mShape  = RECTANGLE;
    mGradient = LINEAR_GRADIENT;
    mTint = nullptr;
    mSolidColors = nullptr;
    mStrokeColors= nullptr;
    mImagePattern= nullptr;
    mAngle = 0;
    /*AOSP field initializer: Orientation mOrientation = TOP_BOTTOM — leaving
      it unset made the default orientation read as garbage (the CTS test
      constructs GradientDrawable() and expects TOP_BOTTOM).*/
    mOrientation = DEFAULT_ORIENTATION;
    //mStrokeColor=mSolidColor=0;
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
    mTintMode = DEFAULT_TINT_MODE;
}

GradientDrawable::GradientState::GradientState(Orientation orientation, const std::vector<int>&gradientColors)
    :GradientState() {
    mOrientation = orientation;
    setGradientColors(gradientColors);
}

GradientDrawable::GradientState::GradientState(const GradientState& orig, Resources* res) {
    mChangingConfigurations = orig.mChangingConfigurations;
    mShape = orig.mShape;
    mGradient = orig.mGradient;
    mAngle = orig.mAngle;
    mOrientation = orig.mOrientation;
    mGradientColors = orig.mGradientColors;
    mPositions = orig.mPositions;
    mSolidColors = orig.mSolidColors;
    mStrokeColors = orig.mStrokeColors;
    //mStrokeColor= orig.mStrokeColor;
    //mSolidColor = orig.mSolidColor;
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
    mTint = orig.mTint;//?new ColorStateList(*orig.mTint):nullptr;
    mTintMode = orig.mTintMode;
    //mThemeAttrs = orig.mThemeAttrs;
    mAttrSize  = orig.mAttrSize;
    mAttrSolid = orig.mAttrSolid;
    mAttrGradient= orig.mAttrGradient;
    mAttrStroke  = orig.mAttrStroke;
    mAttrCorners = orig.mAttrCorners;
    mAttrPadding = orig.mAttrPadding;

    mDensity = Drawable::resolveDensity(res, orig.mDensity);
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
    // AOSP scales padding only when it was SET (non-empty); the inverted
    // guard scaled zeroes.
    if (!mPadding.empty()) {
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
    if (mStrokeDashWidth>0)mStrokeDashWidth = Drawable::scaleFromDensity(mStrokeDashWidth, sourceDensity, targetDensity);

    if (mStrokeDashGap > 0)mStrokeDashGap = Drawable::scaleFromDensity(mStrokeDashGap, sourceDensity, targetDensity);

    if (mGradientRadiusType == RADIUS_TYPE_PIXELS)
        mGradientRadius = Drawable::scaleFromDensity(mGradientRadius, sourceDensity, targetDensity);

    if (mWidth > 0) mWidth = Drawable::scaleFromDensity(mWidth, sourceDensity, targetDensity, true);
    if (mHeight > 0)mHeight= Drawable::scaleFromDensity(mHeight, sourceDensity, targetDensity, true);
}


GradientDrawable* GradientDrawable::GradientState::newDrawable() {
    return new GradientDrawable(std::dynamic_pointer_cast<GradientState>(shared_from_this()), nullptr);
}

Drawable* GradientDrawable::GradientState::newDrawable(Resources* res) {
    // AOSP java:2366-2377: if this drawable is being created for a different
    // density, just create a new constant state and call it a day.
    std::shared_ptr<GradientState> state;
    const int density = Drawable::resolveDensity(res, mDensity);
    if (density != mDensity) {
        state = std::make_shared<GradientState>(*this, res);
    } else {
        state = std::dynamic_pointer_cast<GradientState>(shared_from_this());
    }

    return new GradientDrawable(state, res);
}

int GradientDrawable::GradientState::getChangingConfigurations()const {
    return 0;
}

void GradientDrawable::GradientState::setShape( int shape) {
    mShape = shape;
    computeOpacity();
}

void GradientDrawable::GradientState::setSolidColors(const RefPtr<ColorStateList>&colors) {
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

void GradientDrawable::GradientState::setStroke(int width,const RefPtr<ColorStateList>&colors, float dashWidth,float dashGap) {
    mStrokeWidth = width;
    mStrokeColors = colors;
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
    :GradientDrawable(std::make_shared<GradientState>(), nullptr) {
}

GradientDrawable::GradientDrawable(std::shared_ptr<GradientState>state, Resources* res) {
    mPathIsDirty = mGradientIsDirty = false;
    mPadding.set(0,0,0,0);
    mGradientState = state;
    mMutated = false;
    mGradientRadius = 0.5f;
    mStrokeWidth =-1;
    mAlpha = 255;
    mRect.setEmpty();
    mPath = std::make_shared<cdroid::Path>();
    updateLocalState(res);
}

GradientDrawable::GradientDrawable(Orientation orientation,const std::vector<int>&colors)
    :GradientDrawable(std::make_shared<GradientState>(orientation,colors), nullptr) {
}

GradientDrawable::~GradientDrawable(){
}

std::shared_ptr<Drawable::ConstantState>GradientDrawable::getConstantState() {
    return mGradientState;
}

void GradientDrawable::updateLocalState(Resources* res) {
    mPathIsDirty = true;
    auto state = mGradientState;
    if(state->mSolidColors) {
        const std::vector<int> currentState = getState();
        const int stateColor = state->mSolidColors->getColorForState(currentState,0);
        Color c(stateColor);
        mFillPaint = SolidPattern::create_rgba(c.red(),c.green(),c.blue(),(c.alpha()*mAlpha)/255.f);
    } else if(state->mGradientColors.size()) {
        mFillPaint = SolidPattern::create_rgba(0,0,0,0);
        ensureValidRect();
    } else if(state->mImagePattern){
        //mFillPaint = SurfacePattern::create(state->mImagePattern);
    } else {
        // No solid color: AOSP leaves the fill paint null — a <shape> with
        // only a <stroke> renders as an unfilled outline, not a black rect.
        mFillPaint = nullptr;
    }
    mPadding = state->mPadding;
    if(state->mStrokeWidth>0){
        if(state->mStrokeColors!=nullptr) {
            setStroke(state->mStrokeWidth,state->mStrokeColors,state->mStrokeDashWidth,state->mStrokeDashGap);
        }else{
            //setStroke(state->mStrokeWidth,state->mStrokeColor,state->mStrokeDashWidth,state->mStrokeDashGap);
        }
    }
    mGradientIsDirty = true;
    state->computeOpacity();
    mTintFilter = updateTintFilter(mTintFilter, state->mTint, state->mTintMode);
}

void GradientDrawable::getOutline(Outline& outline) {
    auto st = mGradientState;
    const Rect bounds = getBounds();
    float rad , halfStrokeWidth;
    int top;
    // only report non-zero alpha if shape being drawn has consistent opacity over shape. Must
    // either not have a stroke, or have same stroke/fill opacity
    const bool useFillOpacity = st->mOpaqueOverShape && (mGradientState->mStrokeWidth <= 0
            || mStrokePaint == nullptr /*|| mStrokePaint->getAlpha() == mFillPaint->getAlpha()*/);
    // AOSP: report the fill alpha when the shape covers it consistently,
    // 0 otherwise (a hardcoded 255 drew elevation shadows for translucent
    // and stroke-only shapes).
    outline.setAlpha(useFillOpacity ? getAlpha() / 255.0f : 0.0f);

    switch (st->mShape) {
        case RECTANGLE:
            if (!st->mRadiusArray.empty()){// != null) {
                buildPathIfDirty();
                outline.setConvexPath(*mPath);
                return;
            }
            rad = 0;
            if (st->mRadius > 0.0f) {
                // clamp the radius based on width & height, matching behavior in draw()
                rad = std::min(st->mRadius,std::min(bounds.width, bounds.height) * 0.5f);
            }
            outline.setRoundRect(bounds, rad);
            return;
        case OVAL:  outline.setOval(bounds); return;
        case LINE:
            // Hairlines (0-width stroke) must have a non-empty outline for
            // shadows to draw correctly, so we'll use a very small width.
            halfStrokeWidth = mStrokePaint == nullptr ? 0.0001f : mGradientState->mStrokeWidth * 0.5f;
            top = (int) std::floor(bounds.centerY() - halfStrokeWidth);
            outline.setRect(bounds.left, top, bounds.width, halfStrokeWidth*2);
            return;
        default:break;
            // TODO: support more complex shapes
    }
}

GradientDrawable* GradientDrawable::mutate() {
    if (!mMutated && Drawable::mutate() == this) {
        mGradientState=std::make_shared<GradientState>(*mGradientState);
        updateLocalState(nullptr);
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

void GradientDrawable::setStroke(int width, const RefPtr<ColorStateList>&colorStateList) {
    setStroke(width, colorStateList, 0, 0);
}

void GradientDrawable::setStroke(int width,int color, float dashWidth, float dashGap) {
    mGradientState->setStroke(width, ColorStateList::valueOf(color), dashWidth, dashGap);
    setStrokeInternal(width, color, dashWidth, dashGap);
}

void GradientDrawable::setStroke(int width,const RefPtr<ColorStateList>& colorStateList, float dashWidth, float dashGap) {
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
    mGradientIsDirty = true;
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
    mRingPath = nullptr;
    mPathIsDirty = true;
    mGradientState->setShape(shape);
    invalidateSelf();
}

int GradientDrawable::getShape()const {
    return mGradientState->mShape;
}

int GradientDrawable::getIntrinsicWidth() {
    return mGradientState->mWidth;
}

int GradientDrawable::getIntrinsicHeight() {
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
        mPath->reset();
        mPath->addRoundRect(mRect,mGradientState->mRadiusArray);
        mPathIsDirty = false;
    }
}

Cairo::RefPtr<cdroid::Path> GradientDrawable::buildRing(GradientState* st) {
    if (mRingPath != nullptr && (!st->mUseLevelForShape || !mPathIsDirty)) return mRingPath;
    mPathIsDirty = false;

    const float sweep = st->mUseLevelForShape ? (360.0f * getLevel() / 10000.0f) : 360.f;

    RectF bounds;
    bounds.set(mRect.left,mRect.top,mRect.width,mRect.height);

    const float x = bounds.width / 2.0f;
    const float y = bounds.height / 2.0f;

    const float thickness = st->mThickness != -1 ? st->mThickness : bounds.width / st->mThicknessRatio;
    // inner radius
    const float radius = st->mInnerRadius != -1 ? st->mInnerRadius : bounds.width / st->mInnerRadiusRatio;

    RectF innerBounds;
    innerBounds.set(bounds.left,bounds.top,bounds.width,bounds.height);
    innerBounds.inset(x - radius, y - radius);

    bounds.set(innerBounds.left,innerBounds.top,innerBounds.width,innerBounds.height);
    bounds.inset(-thickness, -thickness);

    if (mRingPath == nullptr) {
        mRingPath = std::make_shared<cdroid::Path>();
    } else {
        mRingPath->reset();
    }

    // arcTo treats the sweep angle mod 360, so check for that, since we
    // think 360 means draw the entire oval
    if (sweep < 360 && sweep > -360) {
        //mRingPath->setFillType(Path.FillType.EVEN_ODD);
        // inner top
        mRingPath->moveTo(x + radius, y);
        // outer top
        mRingPath->lineTo(x + radius + thickness, y);
        // outer arc
        mRingPath->arcTo(bounds, 0.0f, sweep, false);
        // inner arc
        mRingPath->arcTo(innerBounds, sweep, -sweep, false);
        mRingPath->close();//_path();
    } else {
        // add the entire ovals
        mRingPath->addOval(bounds, true);//Path.Direction.CW);
        mRingPath->addOval(innerBounds, false);//Path.Direction.CCW);
    }

    return mRingPath;
}

void GradientDrawable::setColor(int argb) {
    Color c(argb);
    mGradientState->setSolidColors(ColorStateList::valueOf(argb));
    mFillPaint = SolidPattern::create_rgba(c.red(),c.green(),c.blue(),c.alpha());
    invalidateSelf();
}

void GradientDrawable::setColor(const RefPtr<ColorStateList>& colorStateList) {
    if(colorStateList==nullptr){
        setColor(Color::TRANSPARENT);
    }else {
        const std::vector<int>& stateSet = getState();
        const int color = colorStateList->getColorForState(stateSet,0);
        Color c(color);
        mGradientState->setSolidColors(colorStateList);
        mFillPaint = SolidPattern::create_rgba(c.red(),c.green(),c.blue(),c.alpha());
        invalidateSelf();
    }
}

const RefPtr<ColorStateList> GradientDrawable::getColor() const{
    return mGradientState->mSolidColors;
}

bool GradientDrawable::onStateChange(const std::vector<int>& stateSet) {
    bool bInvalidateSelf = false;
    double r,g,b,a;

    auto st = mGradientState;
    if (st->mSolidColors != nullptr) {
        RefPtr<Cairo::SolidPattern>pat = std::dynamic_pointer_cast<Cairo::SolidPattern>(mFillPaint);
        pat->get_rgba(r,g,b,a);
        const int newColor = st->mSolidColors->getColorForState(stateSet, 0);
        const int oldColor = Color::toArgb((float)r,(float)g,(float)b,(float)a);
        if (oldColor != newColor) {
            const Color cc(newColor);
            mFillPaint = SolidPattern::create_rgba((float)cc.red(),(float)cc.green(),(float)cc.blue(),(float)cc.alpha());
            bInvalidateSelf = true;
        }
    }

    if ((mStrokePaint != nullptr) && st->mStrokeColors){
        auto strokeColors = st->mStrokeColors;
        RefPtr<Cairo::SolidPattern>pat = std::dynamic_pointer_cast<Cairo::SolidPattern>(mStrokePaint);
        pat->get_rgba(r,g,b,a);
        const int newColor = st->mStrokeColors->getColorForState(stateSet, 0);
        const int oldColor = Color::toArgb((float)r,(float)g,(float)b,(float)a);
        if (oldColor != newColor) {
            const Color cc(newColor);
            mStrokePaint =SolidPattern::create_rgba((float)cc.red(),(float)cc.green(),(float)cc.blue(),(float)cc.alpha());
            bInvalidateSelf = true;
        }
    }

    if (st->mTint != nullptr && st->mTintMode != PorterDuff::Mode::NOOP) {
        // AOSP GradientDrawable.onStateChange re-resolves the tint filter on
        // every state change (updateBlendModeFilter) — without it a stateful
        // tint freezes at the load-time default state (e.g. the Switch track's
        // @color/switch_track_material never turning the checked color).
        mTintFilter = updateTintFilter(mTintFilter, st->mTint, st->mTintMode);
        bInvalidateSelf = true;
    }

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
        updateLocalState(nullptr);
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

void GradientDrawable::setColorFilter(const cdroid::RefPtr<ColorFilter>&colorFilter){
    if(colorFilter !=mColorFilter){
        mColorFilter = colorFilter;
        invalidateSelf();
    }
}

const cdroid::RefPtr<ColorFilter>GradientDrawable::getColorFilter()const{
    return mColorFilter;
}

void GradientDrawable::setTintList(const RefPtr<ColorStateList>&tint){
    mGradientState->mTint = tint;
    mTintFilter= updateTintFilter(mTintFilter,tint,mGradientState->mTintMode);
    invalidateSelf();
}

void GradientDrawable::setTintMode( int tintMode){
    mGradientState->mTintMode = tintMode;
    mTintFilter= updateTintFilter(mTintFilter,mGradientState->mTint,tintMode);
    invalidateSelf();
}

void GradientDrawable::onBoundsChange(const Rect& r) {
    Drawable::onBoundsChange(r);
    mRingPath = nullptr;
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
            // Default missing/mismatched stop positions to even spacing. The ctor / setColors only
            // seed mPositions for 2-3 colors; for any other count mPositions is empty and indexing it
            // per color below would read out of bounds. Android distributes stops evenly by default.
            const size_t n = gradientColors.size();
            std::vector<float> positions;
            if(st.mPositions.size() == n) positions = st.mPositions;
            else { positions.resize(n); for(size_t i=0;i<n;i++) positions[i] = (n>1)?(float)i/(n-1):0.f; }
            const RectF r = mRect;
            float x0, y0;

            if (st.mGradient == LINEAR_GRADIENT) {
                float x1,y1;
                const float level = st.mUseLevel ? getLevel() / 10000.0f : 1.0f;
                // AOSP anchors the gradient vector on right()/bottom()
                // (absolute coordinates), not width/height — for any rect with
                // a non-zero origin (layer offsets, stroke inset) the
                // width/height form reversed the gradient.
                const float rRight = r.right(), rBottom = r.bottom();
                switch (st.mOrientation) {
                case TOP_BOTTOM:
                    x0 = r.left;  y0 = r.top;
                    x1 = x0;      y1 = y0 + level * r.height;
                    break;
                case TR_BL:
                    x0 = rRight;  y0 = r.top;
                    x1 = level * r.left;
                    y1 = level * rBottom;
                    break;
                case RIGHT_LEFT:
                    x0 = rRight;  y0 = r.top;
                    y1 = y0;      x1 = level * r.left;
                    break;
                case BR_TL:
                    x0 = rRight;  y0 = rBottom;
                    x1 = level * r.left;
                    y1 = level * r.top;
                    break;
                case BOTTOM_TOP:
                    x0 = r.left;  y0 = rBottom;
                    x1 = x0;      y1 = level * r.top;
                    break;
                case BL_TR:
                    x0 = r.left;  y0 = rBottom;
                    x1 = level * rRight;
                    y1 = level * r.top;
                    break;
                case LEFT_RIGHT:
                    x0 = r.left;  y0 = r.top;
                    y1 = y0;      x1 = level * rRight;
                    break;
                default:/*TL_BR*/
                    x0 = r.left;  y0 = r.top;
                    x1 = level * rRight;
                    y1 = level *rBottom;
                    break;
                }
                RefPtr<Cairo::LinearGradient>pat = LinearGradient::create(x0, y0, x1, y1);
                for(int i=0; i<gradientColors.size(); i++) {
                    Color c((uint32_t)gradientColors[i]);
                    pat->add_color_stop_rgba(positions[i],c.red(),c.green(),c.blue(),(c.alpha()*mAlpha)/255.f);
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
                RefPtr<Cairo::RadialGradient>pat = RadialGradient::create( x0, y0, 0,x0,y0,radius);
                for(int i=0; i<gradientColors.size(); i++) {
                    Color c((uint32_t)gradientColors[i]);
                    pat->add_color_stop_rgba(positions[i],c.red(),c.green(),c.blue(),(c.alpha()*mAlpha)/255.f);
                }//gradientColors, null, Shader.TileMode.CLAMP));
                mFillPaint = pat;
            } else if (st.mGradient == SWEEP_GRADIENT) {
                x0 = mRect.left+ mRect.width * st.mCenterX;
                y0 = mRect.top + mRect.height * st.mCenterY;
                const double RADIUS = getRadius(mRect,x0,y0);
                // AOSP: append the last color (n+1 stops) and distribute evenly
                // unless explicit positions exist; useLevel scales the stop
                // positions (never all-zero offsets — that collapsed the sweep
                // to the first color whenever the cairo mesh honored them).
                const float level = st.mUseLevel ? getLevel() / 10000.0f : 1.0f;
                const int n = gradientColors.size();
                std::vector<Cairo::ColorStop> stops;
                for(int i=0; i<n; i++) {
                    Color c = gradientColors[i];
                    const float pos = (st.mPositions.size()==(size_t)n)
                            ? st.mPositions[i] : (n>1)?(float)i/(n-1):0.f;
                    stops.push_back({pos*level,c.red(),c.green(),c.blue(),(c.alpha()*mAlpha)/255.f});
                }
                Color cl((uint32_t)gradientColors[n-1]);
                stops.push_back({1.f,cl.red(),cl.green(),cl.blue(),(cl.alpha()*mAlpha)/255.f});
                mFillPaint = SweepGradient::create(x0, y0,RADIUS,M_PI*2.0,stops);
            } else if(st.mGradient == BITMAP_PATTERN){
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

void GradientDrawable::drawRoundedRect(Canvas& cr,const RectF&rect,double topLeftRadius,
        double topRightRadius, double bottomRightRadius, double bottomLeftRadius) {
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
    if((mGradientState->mStrokeDashWidth!=0.f)&&(mGradientState->mStrokeDashGap!=0.f))
        canvas.set_dash(std::vector<double> {mGradientState->mStrokeDashWidth,mGradientState->mStrokeDashGap},0);
    canvas.set_source(mStrokePaint);
}

void GradientDrawable::draw(Canvas&canvas) {
    if (!ensureValidRect())return; // nothing to draw
    auto st = mGradientState;
    const auto colorFilter = mColorFilter;// ? mColorFilter : mTintFilter;
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
    float rad = 0.0f;

    std::vector<float>radii;
    if( (mFillPaint==nullptr) && (st->mImagePattern==nullptr) && (haveStroke==false) )return;
    
    canvas.save();
    ColorFilter* tintFilter = beginTintGroup(canvas, mBounds, mTintFilter.get());
    if(mFillPaint)
        mFillPaint->set_dither(ditherMode);
    switch (st->mShape) {
    case RECTANGLE:
        rad = std::min(st->mRadius,std::min(mRect.width, mRect.height) * 0.5f);
        if(st->mRadiusArray.size())radii = st->mRadiusArray;
        if(st->mRadius > 0.0f)radii.assign(8,rad);
        if(radii.size())
            drawRoundedRect(canvas,mRect,radii[0],radii[2],radii[4],radii[6]);
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
    case OVAL: {
        // AOSP drawOval: always the FULL ellipse centered in the bounds —
        // useLevel only scales the gradient shader, never clips the shape.
        // Scale about the center (translate first); the old origin-scale
        // shifted the arc center to centerX*(w/h).
        const float cx = mRect.centerX(), cy = mRect.centerY();
        canvas.translate(cx, cy);
        canvas.scale(double(mRect.width)/double(mRect.height),1.f);
        canvas.begin_new_sub_path();
        canvas.arc(0, 0, mRect.height/2.f, 0, M_PI*2.f);
        break;
    }
    case RING:
        if(0){/*new ring with cdroid::Path*/
            auto path = buildRing(st.get());
            path->append_to_context(&canvas);
        }else {/*old ring*/
            //inner
            float innerRadius = float(st->mInnerRadius);
            RectF bounds= {mRect.left,mRect.top,mRect.width,mRect.height};
            float thickness = st->mThickness!=-1 ? st->mThickness:(bounds.width/st->mThicknessRatio);
            float radius = st->mInnerRadius!=-1 ? st->mInnerRadius :(bounds.width/st->mInnerRadiusRatio);
            // AOSP buildRing/drawOval center the ring in the bounds. Scale
            // about the CENTER (translate first), never about the origin:
            // origin-scaling multiplied the center too, shifting every
            // non-square ring horizontally.
            const float x = bounds.centerX();
            const float y = bounds.centerY();
            canvas.translate(x, y);
            canvas.scale(bounds.width/bounds.height,1.f);
            RectF innerBounds = bounds;
            if(innerRadius<=0.f)
                innerRadius=std::min(mRect.width,mRect.height)/2.f-thickness;
            canvas.begin_new_sub_path();
            if( sweep<360.f && sweep>-360.f ) {
                const double end_angle = M_PI*2*sweep/360.f;
                canvas.set_fill_rule(Cairo::Context::FillRule::WINDING);//EVEN_ODD);//WINDING);
                canvas.move_to(radius,0);
                canvas.arc(0,0,radius + thickness,0.f,end_angle);
                canvas.arc_negative(0,0,radius,end_angle,0.f);
                canvas.close_path();
            } else {
                //canvas.set_fill_rule(Cairo::Context::FillRule::EVEN_ODD);
                canvas.arc(0,0,radius + thickness,0,M_PI*2.f);
                canvas.begin_new_sub_path();
                canvas.arc_negative(0,0,radius,M_PI*2.f,0.f);
            }
        }break;
    }/*endof switch*/

    if(st->mShape!=LINE){
        const bool needFill = (mFillPaint!=nullptr)||(st->mImagePattern!=nullptr);
        if(mFillPaint)canvas.set_source(mFillPaint);
        if(st->mImagePattern)canvas.set_source(st->mImagePattern,0,0);
        if(needFill){
            if(haveStroke) canvas.fill_preserve();
            else canvas.fill();
        }
        if (haveStroke) {
            prepareStrokeProps(canvas);
            canvas.stroke();
        }
    }
    if(tintFilter) endTintGroup(canvas, mBounds, tintFilter);
    canvas.restore();
}

void GradientDrawable::inflate(Resources& r,XmlPullParser&parser,const AttributeSet&atts,const Resources::Theme* theme){
    Drawable::inflate(r, parser, atts, theme);
    mGradientState->setDensity(Drawable::resolveDensity(&r, 0));
    auto ta = obtainAttributes(r, theme, atts, R::styleable::GradientDrawable);
    if (ta) updateStateFromTypedArray(*ta);
    inflateChildElements(r,parser,atts,theme);
    updateLocalState(&r);
}

// AOSP GradientDrawable.canApplyTheme: theme attrs pending re-resolution.
bool GradientDrawable::canApplyTheme(){
    return (mGradientState && !mGradientState->mThemeAttrs.empty()) || Drawable::canApplyTheme();
}

// AOSP GradientDrawable.applyTheme(Theme): re-resolve the recorded ?attr ids
// through the new theme and refresh the state (AOSP also re-themes the tint /
// solid / stroke / gradient ColorStateLists — CDROID CSLs resolve ?attr up
// front at inflation, so only the top-level attrs are re-resolved here).
void GradientDrawable::applyTheme(const Resources::Theme& t){
    Drawable::applyTheme(t);
    auto state = mGradientState;
    if (!state) return;
    if (!state->mThemeAttrs.empty()) {
        auto a = t.resolveAttributes(state->mThemeAttrs, R::styleable::GradientDrawable);
        if (a) updateStateFromTypedArray(*a);
        state->mThemeAttrs.clear();
    }
    updateLocalState(&t.getResources());
}

void GradientDrawable::updateStateFromTypedArray(const TypedArray& a) {
    auto state = mGradientState;

    // Account for any configuration changes.
    //state.mChangingConfigurations |= a.getChangingConfigurations();
    // Extract the theme attributes, if any.
    state->mThemeAttrs = a.extractThemeAttrs();

    // aapt2 pre-resolves the shape enum (rectangle/oval/line/ring).
    state->mShape = a.getInt(R::styleable::GradientDrawable_shape, state->mShape);

    state->mDither = a.getBoolean(R::styleable::GradientDrawable_dither, state->mDither);

    if (state->mShape == GradientDrawable::RING) {
        state->mInnerRadius = a.getDimensionPixelSize(R::styleable::GradientDrawable_innerRadius, state->mInnerRadius);
        if (state->mInnerRadius == -1) {
            state->mInnerRadiusRatio = a.getFloat(R::styleable::GradientDrawable_innerRadiusRatio, state->mInnerRadiusRatio);
        }

        state->mThickness = a.getDimensionPixelSize(R::styleable::GradientDrawable_thickness, state->mThickness);
        if (state->mThickness == -1) {
            state->mThicknessRatio = a.getFloat(R::styleable::GradientDrawable_thicknessRatio, state->mThicknessRatio);
        }

        state->mUseLevelForShape = a.getBoolean(R::styleable::GradientDrawable_useLevel, state->mUseLevelForShape);
    }

    // aapt2 pre-resolves the tintMode enum; -1 means "not specified".
    const int tintMode = a.getInt(R::styleable::GradientDrawable_tintMode, -1);
    if (tintMode != -1) {
        state->mTintMode = tintMode;
    }

    auto tint = a.getColorStateList(R::styleable::GradientDrawable_tint);
    if (tint != nullptr) {
        state->mTint = tint;
    }

    const int insetLeft = a.getDimensionPixelSize(R::styleable::GradientDrawable_opticalInsetLeft, state->mOpticalInsets.left);
    const int insetTop = a.getDimensionPixelSize(R::styleable::GradientDrawable_opticalInsetTop, state->mOpticalInsets.top);
    const int insetRight = a.getDimensionPixelSize(R::styleable::GradientDrawable_opticalInsetRight, state->mOpticalInsets.right);
    const int insetBottom = a.getDimensionPixelSize(R::styleable::GradientDrawable_opticalInsetBottom, state->mOpticalInsets.bottom);
    state->mOpticalInsets = Insets::of(insetLeft, insetTop, insetRight, insetBottom);
}

void GradientDrawable::inflateChildElements(Resources& r,XmlPullParser&parser,const AttributeSet&atts,const Resources::Theme* theme){
    int type,depth;
    const int innerDepth = parser.getDepth()+1;

    while (((type=parser.next()) != XmlPullParser::END_DOCUMENT)
           && ((depth=parser.getDepth()) >= innerDepth || type != XmlPullParser::END_TAG)) {
        if ( (type != XmlPullParser::START_TAG) || (depth > innerDepth) ){
            continue;
        }

        const std::string name = parser.getName();
        if (name.compare("size")==0) {
            auto ta = obtainAttributes(r, theme, atts, R::styleable::GradientDrawableSize);
            if (ta) updateGradientDrawableSize(*ta);
        } else if (name.compare("gradient")==0) {
            auto ta = obtainAttributes(r, theme, atts, R::styleable::GradientDrawableGradient);
            if (ta) updateGradientDrawableGradient(*ta);
            // CDROID-private "pattern" attr (image pattern for gradient fill).
            { auto ta2 = obtainAttributes(r, theme, atts, R::styleable::GradientDrawablePattern);
              auto pattern = ta2->getString(R::styleable::GradientDrawablePattern_pattern);
              if (!pattern.empty()) setImagePattern(r.getContext(), pattern);
            }
        } else if (name.compare("solid")==0) {
            auto ta = obtainAttributes(r, theme, atts, R::styleable::GradientDrawableSolid);
            if (ta) updateGradientDrawableSolid(*ta);
        } else if (name.compare("stroke")==0) {
            auto ta = obtainAttributes(r, theme, atts, R::styleable::GradientDrawableStroke);
            if (ta) updateGradientDrawableStroke(*ta);
        } else if (name.compare("corners")==0) {
            auto ta = obtainAttributes(r, theme, atts, R::styleable::DrawableCorners);
            if (ta) updateDrawableCorners(*ta);
        } else if (name.compare("padding")==0) {
            auto ta = obtainAttributes(r, theme, atts, R::styleable::GradientDrawablePadding);
            if (ta) updateGradientDrawablePadding(*ta);
        } else {
            LOGW("drawable", "Bad element %s under <shape>: ",name.c_str());
        }
    }
}
    
void GradientDrawable::updateGradientDrawableSize(const TypedArray& a){
    auto st = mGradientState;
    st->mWidth = a.getDimensionPixelSize(R::styleable::GradientDrawableSize_width, st->mWidth);
    st->mHeight = a.getDimensionPixelSize(R::styleable::GradientDrawableSize_height, st->mHeight);
}

// Faithful port of AOSP GradientDrawable.getFloatOrFraction: a fraction-typed
// value resolves via getFraction, everything else via getFloat.
static float getFloatOrFraction(const TypedArray& a, size_t idx, float defaultValue) {
    TypedValue tv;
    if (!a.peekValue(idx, &tv)) return defaultValue;
    if (tv.type == TypedValue::TYPE_FRACTION)
        return a.getFraction(idx, 1, 1, defaultValue);
    return a.getFloat(idx, defaultValue);
}

void GradientDrawable::updateGradientDrawableGradient(const TypedArray& a){
    auto st = mGradientState;

    st->mCenterX = getFloatOrFraction(a, R::styleable::GradientDrawableGradient_centerX, st->mCenterX);
    st->mCenterY = getFloatOrFraction(a, R::styleable::GradientDrawableGradient_centerY, st->mCenterY);
    st->mUseLevel = a.getBoolean(R::styleable::GradientDrawable_useLevel, st->mUseLevel);
    // aapt2 pre-resolves the type enum (linear/radial/sweep); "pattern" is a
    // private value with no framework id and is handled via the bitmap string bridge
    // in inflateChildElements().
    st->mGradient = a.getInt(R::styleable::GradientDrawableGradient_type, st->mGradient);

    // TODO: Update these to be themeable.
    const int startColor = a.getColor(R::styleable::GradientDrawableGradient_startColor, 0);
    const bool hasCenterColor = a.hasValue(R::styleable::GradientDrawableGradient_centerColor);
    const bool hasStartColor = a.hasValue(R::styleable::GradientDrawableGradient_startColor);
    const bool hasEndColor = a.hasValue(R::styleable::GradientDrawableGradient_endColor);
    const int centerColor = a.getColor(R::styleable::GradientDrawableGradient_centerColor, 0);
    const int endColor = a.getColor(R::styleable::GradientDrawableGradient_endColor, 0);

    if (hasCenterColor) {
        st->mGradientColors.resize(3);
        st->mGradientColors[0] = startColor;
        st->mGradientColors[1] = centerColor;
        st->mGradientColors[2] = endColor;

        st->mPositions.resize(3);
        st->mPositions[0] = 0.0f;
        // Since 0.5f is default value, try to take the one that isn't 0.5f
        st->mPositions[1] = st->mCenterX != 0.5f ? st->mCenterX : st->mCenterY;
        st->mPositions[2] = 1.f;
    } else if(hasStartColor||hasEndColor){
        st->mPositions.resize(2);
        st->mGradientColors.resize(2);
        st->mGradientColors[0] = startColor;
        st->mGradientColors[1] = endColor;
        st->mPositions[0] = 0;
        st->mPositions[1] = 1.f;
    }

    const int angle = ((int) a.getFloat(R::styleable::GradientDrawableGradient_angle, st->mAngle))%360;
    // GradientDrawable historically has not parsed negative angle measurements and always
    // stays on the default orientation for API levels older than Q.
    // Only configure the orientation if the angle is greater than zero.
    // Otherwise fallback on Orientation.TOP_BOTTOM
    // In Android Q and later, actually wrap the negative angle measurement to the correct
    // value
    if (sWrapNegativeAngleMeasurements) {
        st->mAngle = ((angle % 360) + 360) % 360; // offset negative angle measures
    } else {
        st->mAngle = angle % 360;
    }
    if (st->mAngle>=0) {
        switch (angle) {
        case 0:  st->mOrientation = Orientation::LEFT_RIGHT;  break;
        case 45: st->mOrientation = Orientation::BL_TR;       break;
        case 90: st->mOrientation = Orientation::BOTTOM_TOP;  break;
        case 135:st->mOrientation = Orientation::BR_TL;       break;
        case 180:st->mOrientation = Orientation::RIGHT_LEFT;  break;
        case 225:st->mOrientation = Orientation::TR_BL;       break;
        case 270: st->mOrientation = Orientation::TOP_BOTTOM;  break;
        case 315:st->mOrientation = Orientation::TL_BR;       break;
        }
    } else {
        st->mOrientation = DEFAULT_ORIENTATION;
    }

    // gradientRadius: dispatch on the raw typed value (AOSP peekValue pattern).
    TypedValue tv;
    if (a.peekValue(R::styleable::GradientDrawableGradient_gradientRadius, &tv)) {
        float radius;
        int radiusType;
        if (tv.type == TypedValue::TYPE_FRACTION) {
            radius = a.getFraction(R::styleable::GradientDrawableGradient_gradientRadius, 1, 1, 1.0f);
            const int unit = (tv.data >> TypedValue::COMPLEX_UNIT_SHIFT) & TypedValue::COMPLEX_UNIT_MASK;
            radiusType = (unit == TypedValue::COMPLEX_UNIT_FRACTION_PARENT)
                         ? RADIUS_TYPE_FRACTION_PARENT : RADIUS_TYPE_FRACTION;
        } else if (tv.type == TypedValue::TYPE_DIMENSION) {
            radius = a.getDimension(R::styleable::GradientDrawableGradient_gradientRadius, 0);
            radiusType = RADIUS_TYPE_PIXELS;
        } else {
            radius = a.getFloat(R::styleable::GradientDrawableGradient_gradientRadius, 0);
            radiusType = RADIUS_TYPE_PIXELS;
        }

        st->mGradientRadius = radius;
        st->mGradientRadiusType = radiusType;
    }
    mGradientIsDirty =true;
}

void GradientDrawable::updateGradientDrawableSolid(const TypedArray& a){
    auto colorStateList = a.getColorStateList(R::styleable::GradientDrawableSolid_color);
    if(colorStateList) {
        setColor(colorStateList);
    }
}

void GradientDrawable::updateGradientDrawableStroke(const TypedArray& a){
    auto st = mGradientState;
    const int defaultStrokeWidth = std::max(0,st->mStrokeWidth);
    const int width = a.getDimensionPixelSize(R::styleable::GradientDrawableStroke_width, defaultStrokeWidth);
    const float dashWidth = a.getDimension(R::styleable::GradientDrawableStroke_dashWidth, st->mStrokeDashWidth);
    const float dashGap = a.getDimension(R::styleable::GradientDrawableStroke_dashGap, st->mStrokeDashGap);

    auto colorStateList = a.getColorStateList(R::styleable::GradientDrawableStroke_color);
    if(colorStateList==nullptr){
        colorStateList = st->mStrokeColors;
    }
    if(dashWidth!=0.0f){
        setStroke(width,colorStateList,dashWidth,dashGap);
    }else{
        setStroke(width,colorStateList);
    }
}

void GradientDrawable::updateDrawableCorners(const TypedArray& a){
    auto st = mGradientState;
    // AOSP: radius comes from the android:radius attribute (defaulting to the
    // state's current radius), NOT from the state alone — the state is still 0
    // during inflate, so skipping the read dropped every <corners android:radius>.
    const int radius = a.getDimensionPixelSize(R::styleable::DrawableCorners_radius, (int)st->mRadius);
    setCornerRadius(radius);

    // TODO: Update these to be themeable.
    const float topLeftRadius = a.getDimensionPixelSize(R::styleable::DrawableCorners_topLeftRadius, radius);
    const float topRightRadius = a.getDimensionPixelSize(R::styleable::DrawableCorners_topRightRadius, radius);
    const float bottomLeftRadius = a.getDimensionPixelSize(R::styleable::DrawableCorners_bottomLeftRadius, radius);
    const float bottomRightRadius = a.getDimensionPixelSize(R::styleable::DrawableCorners_bottomRightRadius, radius);

    if ( (topLeftRadius != radius) || (topRightRadius != radius) ||
            (bottomLeftRadius != radius) || (bottomRightRadius != radius)) {
        // The corner radii are specified in clockwise order (see Path.addRoundRect()):
        // tl,tl, tr,tr, br,br, bl,bl — the bottom pair was previously flipped,
        // swapping the bottom-left/bottom-right corners of every shape that
        // declares per-corner radii.
        setCornerRadii(std::vector<float>{ topLeftRadius, topLeftRadius, topRightRadius, topRightRadius,
                bottomRightRadius, bottomRightRadius, bottomLeftRadius, bottomLeftRadius });
    }
}

void GradientDrawable::updateGradientDrawablePadding(const TypedArray& a){
    Rect pad = mGradientState->mPadding;
    pad.set(a.getDimensionPixelOffset(R::styleable::GradientDrawablePadding_left, pad.left),
            a.getDimensionPixelOffset(R::styleable::GradientDrawablePadding_top, pad.top),
            a.getDimensionPixelOffset(R::styleable::GradientDrawablePadding_right, pad.width),
            a.getDimensionPixelOffset(R::styleable::GradientDrawablePadding_bottom, pad.height));
    mGradientState->mPadding = pad;  // androidx: persist into the state so shared/cached instances read it
    mPadding = pad;
}

}//end namespace
