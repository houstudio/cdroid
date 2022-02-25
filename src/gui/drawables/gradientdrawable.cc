#include <drawables/gradientdrawable.h>
#include <color.h>
#include <cdlog.h>
namespace cdroid{

#define DEFAULT_INNER_RADIUS_RATIO 3.0f
#define DEFAULT_THICKNESS_RATIO 9.0f

GradientDrawable::GradientState::GradientState(){
   mShape  = RECTANGLE;
   mGradient = LINEAR_GRADIENT;
   mAngle = 0;
   mStrokeWidth = -1;//if >= 0 use stroking
   mStrokeDashWidth = 0.0f;
   mStrokeDashGap = 0.0f;
   mRadius = 0.0f;
   mWidth =mHeight= -1;
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
    :GradientState(){
    mOrientation = orientation;
    setGradientColors(gradientColors);
}

GradientDrawable::GradientState::GradientState(const GradientState& orig){
    mChangingConfigurations = orig.mChangingConfigurations;
    mShape = orig.mShape;
    mGradient = orig.mGradient;
    mAngle = orig.mAngle;
    mOrientation = orig.mOrientation;
    mSolidColors = orig.mSolidColors;
    mGradientColors = orig.mGradientColors;
    mPositions = orig.mPositions;
    mStrokeColors = orig.mStrokeColors;
    mStrokeWidth = orig.mStrokeWidth;
    mStrokeDashWidth = orig.mStrokeDashWidth;
    mStrokeDashGap = orig.mStrokeDashGap;
    mRadius = orig.mRadius;
    mRadiusArray = orig.mRadiusArray;
    mPadding=orig.mPadding;
    mWidth = orig.mWidth;
    mHeight = orig.mHeight;
    mInnerRadiusRatio = orig.mInnerRadiusRatio;
    mThicknessRatio = orig.mThicknessRatio;
    mInnerRadius = orig.mInnerRadius;
    mThickness = orig.mThickness;
    mDither = orig.mDither;
    //mOpticalInsets = orig.mOpticalInsets;
    mCenterX = orig.mCenterX;
    mCenterY = orig.mCenterY;
    mGradientRadius = orig.mGradientRadius;
    mGradientRadiusType = orig.mGradientRadiusType;
    mUseLevel = orig.mUseLevel;
    mUseLevelForShape = orig.mUseLevelForShape;
    mOpaqueOverBounds = orig.mOpaqueOverBounds;
    mOpaqueOverShape = orig.mOpaqueOverShape;
    mTint = orig.mTint;
    mTintMode = orig.mTintMode;
    //mThemeAttrs = orig.mThemeAttrs;
    mAttrSize = orig.mAttrSize;
    mAttrGradient = orig.mAttrGradient;
    mAttrSolid = orig.mAttrSolid;
    mAttrStroke = orig.mAttrStroke;
    mAttrCorners = orig.mAttrCorners;
    mAttrPadding = orig.mAttrPadding;

    mDensity =orig.mDensity;// Drawable::resolveDensity(/*res,*/orig.mDensity);
    if (orig.mDensity != mDensity) {
        applyDensityScaling(orig.mDensity, mDensity);
    }
}

void GradientDrawable::GradientState::setDensity(int targetDensity) {
    if (mDensity != targetDensity) {
        const int sourceDensity = mDensity;
        mDensity = targetDensity;
        applyDensityScaling(sourceDensity, targetDensity);
    }
}

bool GradientDrawable::GradientState::hasCenterColor()const{
    return mGradientColors.size() == 3;
}

void GradientDrawable::GradientState::applyDensityScaling(int sourceDensity, int targetDensity) {
    if (mInnerRadius > 0)mInnerRadius = Drawable::scaleFromDensity(mInnerRadius, sourceDensity, targetDensity, true);

    if (mThickness > 0)  mThickness = Drawable::scaleFromDensity(mThickness, sourceDensity, targetDensity, true);

    if (mOpticalInsets.left!=0||mOpticalInsets.top!=0||mOpticalInsets.right!=0||mOpticalInsets.right!=0) {
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
        mRadiusArray[0] = Drawable::scaleFromDensity((int) mRadiusArray[0], sourceDensity, targetDensity, true);
        mRadiusArray[1] = Drawable::scaleFromDensity((int) mRadiusArray[1], sourceDensity, targetDensity, true);
        mRadiusArray[2] = Drawable::scaleFromDensity((int) mRadiusArray[2], sourceDensity, targetDensity, true);
        mRadiusArray[3] = Drawable::scaleFromDensity((int) mRadiusArray[3], sourceDensity, targetDensity, true);
    }
    if (mStrokeWidth > 0)  mStrokeWidth = Drawable::scaleFromDensity(mStrokeWidth, sourceDensity, targetDensity, true);
    if (mStrokeDashWidth>0)mStrokeDashWidth = Drawable::scaleFromDensity(mStrokeDashGap, sourceDensity, targetDensity);

    if (mStrokeDashGap > 0)mStrokeDashGap = Drawable::scaleFromDensity(mStrokeDashGap, sourceDensity, targetDensity);
    
    if (mGradientRadiusType == RADIUS_TYPE_PIXELS)
        mGradientRadius = Drawable::scaleFromDensity(mGradientRadius, sourceDensity, targetDensity);

    if (mWidth > 0) mWidth = Drawable::scaleFromDensity(mWidth, sourceDensity, targetDensity, true);
    if (mHeight > 0)mHeight= Drawable::scaleFromDensity(mHeight, sourceDensity, targetDensity, true);
}


Drawable* GradientDrawable::GradientState::newDrawable() {
    return new GradientDrawable(shared_from_this());
}

int GradientDrawable::GradientState::getChangingConfigurations()const{
    return 0;
}

void GradientDrawable::GradientState::setShape( int shape) {
    mShape = shape;
    computeOpacity();
}

void GradientDrawable::GradientState::setSolidColors(ColorStateList*tint){

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
    mSolidColors=nullptr;
    computeOpacity();
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
    if (mGradientColors.size()==0 && mSolidColors == nullptr) {
        return;
    }

    // Colors are opaque, so opaqueOverShape=true,
    mOpaqueOverShape = true;
    // and opaqueOverBounds=true if shape fills bounds
    mOpaqueOverBounds = mShape == RECTANGLE && mRadius <= 0  && mRadiusArray.size()==0;
}

void GradientDrawable::GradientState::setStroke(int width,/*@Nullable*/ColorStateList*colors, float dashWidth,float dashGap){
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

GradientDrawable::GradientDrawable(){
    mGradientState=std::make_shared<GradientState>();
}

GradientDrawable::GradientDrawable(std::shared_ptr<GradientState>state){
    mGradientState=state;
}

GradientDrawable::GradientDrawable(Orientation orientation,const std::vector<int>&colors){
    mGradientState=std::make_shared<GradientState>(orientation,colors);
}

std::shared_ptr<Drawable::ConstantState>GradientDrawable::getConstantState(){
    return mGradientState;
}

void GradientDrawable::updateLocalState(){
}
Drawable* GradientDrawable::mutate() {
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
    if (1){//mPadding != null) {
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
const std::vector<float>&GradientDrawable::getCornerRadii()const{
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

void GradientDrawable::setStroke(int width, ColorStateList*colorStateList) {
    setStroke(width, colorStateList, 0, 0);
}

void GradientDrawable::setStroke(int width,int color, float dashWidth, float dashGap) {
    mGradientState->setStroke(width, ColorStateList::valueOf(color), dashWidth, dashGap);
    setStrokeInternal(width, color, dashWidth, dashGap);
}

void GradientDrawable::setStroke(int width,ColorStateList* colorStateList, float dashWidth, float dashGap) {
    mGradientState->setStroke(width, colorStateList, dashWidth, dashGap);
    int color;
    if (colorStateList == nullptr) {
        color = Color::TRANSPARENT;
    } else {
        const std::vector<int>& stateSet = getState();
        color = colorStateList->getColorForState(stateSet, 0);
    }
    setStrokeInternal(width, color, dashWidth, dashGap);
}

void GradientDrawable::setStrokeInternal(int width, int color, float dashWidth, float dashGap) {
    Color c(color);
    mStrokePaint = SolidPattern::create_rgba(c.red(),c.green(),c.blue(),c.alpha());
    mStrokeWidth =width;
    mDashArray=std::vector<double>{dashWidth,dashGap};
    invalidateSelf();
}

void GradientDrawable::setShape(/*@Shape*/ int shape){
    //mRingPath = nullptr;
    mPathIsDirty = true;
    mGradientState->setShape(shape);
    invalidateSelf();
}

int GradientDrawable::getShape()const{
    return mGradientState->mShape;
}

int GradientDrawable::getIntrinsicWidth()const{
    return mGradientState->mWidth;
}

int GradientDrawable::getIntrinsicHeight()const{
    return mGradientState->mHeight;
}

Insets GradientDrawable::getOpticalInsets(){
    return mGradientState->mOpticalInsets;
}

void GradientDrawable::setSize(int width, int height) {
    mGradientState->setSize(width, height);
    mPathIsDirty = true;
    invalidateSelf();
}

void GradientDrawable::setGradientType(int gradient) {
    mGradientState->setGradientType(gradient);
    mGradientIsDirty = true;
    invalidateSelf();
}

int GradientDrawable::getGradientType()const{
    return mGradientState->mGradient;
}

void GradientDrawable::setGradientCenter(float x, float y) {
    mGradientState->setGradientCenter(x, y);
    mGradientIsDirty = true;
    invalidateSelf();
}

float GradientDrawable::getGradientCenterX()const{
    return mGradientState->mCenterX;
}

float GradientDrawable::getGradientCenterY()const{
    return mGradientState->mCenterY;
}

void  GradientDrawable::setGradientRadius(float gradientRadius) {
    mGradientState->setGradientRadius(gradientRadius, 0);//TypedValue.COMPLEX_UNIT_PX);
    mGradientIsDirty = true;
    invalidateSelf();
}

float GradientDrawable::getGradientRadius(){
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

bool GradientDrawable::getUseLevel()const{
    return mGradientState->mUseLevel;
}

int GradientDrawable::modulateAlpha(int alpha) {
    int scale = mAlpha + (mAlpha >> 7);
    return alpha * scale >> 8;
}

GradientDrawable::Orientation GradientDrawable::getOrientation()const{
    return mGradientState->mOrientation;
}

void GradientDrawable::setOrientation(Orientation orientation) {
    mGradientState->mOrientation = orientation;
    mGradientIsDirty = true;
    invalidateSelf();
}

void GradientDrawable::setColors(const std::vector<int>& colors) {
    mGradientState->setGradientColors(colors);
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

void GradientDrawable::buildPathIfDirty() {
    if (mPathIsDirty) {
        ensureValidRect();
        mPath->reset();
        mPath->round_rectangle(mRect,mGradientState->mRadiusArray);
        mPathIsDirty = false;
    }
}

//Path buildRing(GradientState st){}

void GradientDrawable::setColor(int argb) {
    Color c(argb);
    mGradientState->setSolidColors(ColorStateList::valueOf(argb));
    mFillPaint=SolidPattern::create_rgba(c.red(),c.green(),c.blue(),c.alpha());
    invalidateSelf();
}

void GradientDrawable::setColor(ColorStateList* colorStateList) {
    mGradientState->setSolidColors(colorStateList);
    int color;
    if (colorStateList == nullptr) {
        color = Color::TRANSPARENT;
    } else {
        std::vector< int>stateSet = getState();
        color = colorStateList->getColorForState(stateSet, 0);
    }
    Color c(color);
    mFillPaint=SolidPattern::create_rgba(c.red(),c.green(),c.blue(),c.alpha());
    invalidateSelf();
}

ColorStateList* GradientDrawable::getColor() {
    return mGradientState->mSolidColors;
}

bool GradientDrawable::onStateChange(const std::vector<int>& stateSet){
    return true;
}

bool GradientDrawable::isStateful()const{
    GradientState&s =*mGradientState;
    return Drawable::isStateful()
        || (s.mSolidColors  && s.mSolidColors->isStateful())
        || (s.mStrokeColors && s.mStrokeColors->isStateful())
        || (s.mTint != nullptr && s.mTint->isStateful());
}

bool GradientDrawable::hasFocusStateSpecified()const{
    GradientState& s = *mGradientState;
    return (s.mSolidColors && s.mSolidColors->hasFocusStateSpecified())
        || (s.mStrokeColors&& s.mStrokeColors->hasFocusStateSpecified())
        || (s.mTint != nullptr && s.mTint->hasFocusStateSpecified());
}

int  GradientDrawable::getChangingConfigurations()const{
    return Drawable::getChangingConfigurations() | mGradientState->getChangingConfigurations();
}

void  GradientDrawable::setAlpha(int alpha) {
    if (alpha != mAlpha) {
        mAlpha = alpha;
        invalidateSelf();
    }
}

int GradientDrawable::getAlpha()const{
    return mAlpha;
}

void GradientDrawable::setDither(bool dither) {
    if (dither != mGradientState->mDither) {
        mGradientState->mDither = dither;
        invalidateSelf();
    }
}

int GradientDrawable::getOpacity()const{
    return (mAlpha == 255 && mGradientState->mOpaqueOverBounds && isOpaqueForState()) ?
            OPAQUE : TRANSLUCENT;
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

bool GradientDrawable::ensureValidRect(){
    if (mGradientIsDirty) {
        mGradientIsDirty = false;
        Rect bounds = getBounds();
        float inset = 0;

        //if (mStrokePaint != nullptr)inset = mStrokePaint.getStrokeWidth() * 0.5f;

        GradientState&st =*mGradientState;
        mRect.set(bounds.left + inset, bounds.top + inset, bounds.width - 2*inset, bounds.height - 2*inset);

        std::vector<int>&gradientColors = st.mGradientColors; 
        if (gradientColors.size()) {
             const RectF r = mRect;
             float x0, x1, y0, y1;

             if (st.mGradient == LINEAR_GRADIENT) {
                 const float level = st.mUseLevel ? getLevel() / 10000.0f : 1.0f;
                 switch (st.mOrientation) {
                 case TOP_BOTTOM: x0 = r.left;  y0 = r.top;    x1 = x0;                y1 = level *r.height; break;
                 case TR_BL:      x0 = r.width; y0 = r.top;    x1 = level * r.left;    y1 = level *r.height; break;
                 case RIGHT_LEFT: x0 = r.width; y0 = r.top;    x1 = level * r.left;    y1 = y0;              break;
                 case BR_TL:      x0 = r.width; y0 = r.height; x1 = level * r.left;    y1 = level * r.top;   break;
                 case BOTTOM_TOP: x0 = r.left;  y0 = r.height; x1 = x0;                y1 = level * r.top;   break;
                 case BL_TR:      x0 = r.left;  y0 = r.height; x1 = level * r.width;   y1 = level * r.top;   break;
                 case LEFT_RIGHT: x0 = r.left;  y0 = r.top;    x1 = level * r.width;   y1 = y0;              break;
                 default:/*TL_BR*/x0 = r.left;  y0 = r.top;    x1 = level * r.width;   y1 = level *r.height; break;
                 }
                 RefPtr<Cairo::LinearGradient>pat=LinearGradient::create(x0, y0, x1, y1);
                 //gradientColors, st.mPositions, Shader.TileMode.CLAMP));
                 for(int i=0;i<gradientColors.size();i++){
                     Color c((uint32_t)gradientColors[i]);
                     pat->add_color_stop_rgba(st.mPositions[i],c.red(),c.green(),c.blue(),c.alpha());
                 }
                 mFillPaint=pat;
            } else if (st.mGradient == RADIAL_GRADIENT) {
                x0 = r.left + r.width* st.mCenterX;
                y0 = r.top + r.height * st.mCenterY;

                float radius = st.mGradientRadius;
                if (st.mGradientRadiusType == RADIUS_TYPE_FRACTION) {
                    // Fall back to parent width or height if intrinsic size is not specified.
                    const float width = st.mWidth >= 0 ? st.mWidth   :r.width;
                    const float height = st.mHeight >= 0?st.mHeight:r.height;
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
                RefPtr<Cairo::RadialGradient>pat=RadialGradient::create( x0, y0, radius,x0,y0,0);
                for(int i=0;i<gradientColors.size();i++){
                    Color c((uint32_t)gradientColors[i]);
                    pat->add_color_stop_rgba(st.mPositions[i],c.red(),c.green(),c.blue(),c.alpha());
                }// , gradientColors, null, Shader.TileMode.CLAMP));
                mFillPaint=pat;
            } else if (st.mGradient == SWEEP_GRADIENT) {
                x0 = r.left + r.width * st.mCenterX;
                y0 = r.top + r.height * st.mCenterY;
                std::vector<int> tempColors = gradientColors;
                std::vector<float> tempPositions ;

                if (st.mUseLevel) {
                    const int length = gradientColors.size();
                    /*if (tempColors == null || tempColors.length != length + 1) {
                        tempColors = st.mTempColors = new int[length + 1];
                    }
                    System.arraycopy(gradientColors, 0, tempColors, 0, length);*/
                    
                    tempColors.push_back( gradientColors[length - 1]);
                    tempPositions.resize(length);
                    const float fraction = 1.0f / (length - 1);

                    const float level = getLevel() / 10000.0f;
                    for (int i = 0; i < length; i++) {
                         tempPositions[i] = i * fraction * level;
                    }
                    tempPositions.push_back(1.f);//[length] = 1.0f;
                }
                RefPtr<SweepGradient>pat=SweepGradient::create(x0, y0,200);
                //tempColors, tempPositions));
                mFillPaint=pat;
            }

            // If we don't have a solid color, the alpha channel must be
            // maxed out so that alpha modulation works correctly.
            //if (st.mSolidColors == nullptr) 
            //    mFillPaint=SolidPattern::create_rgb(0,0,0);//setColor(Color.BLACK);
        }
    }
    return !mRect.empty();
}

bool GradientDrawable::isOpaqueForState()const{
    if (mGradientState->mStrokeWidth >= 0 /*&& mStrokePaint != nullptr
                && !isOpaque(mStrokePaint.getColor())*/) {
        return false;
     }
     // Don't check opacity if we're using a gradient, as we've already
     // checked the gradient opacity in mOpaqueOverShape.
    if (mGradientState->mGradientColors.size() == 0/* && !isOpaque(mFillPaint.getColor())*/) {
        return false;
    }
    return true;
}

static void drawRound(Canvas&canvas,const RectF&r,const std::vector<float>&radii){
    constexpr double degree = M_PI/180.f;
    if(radii.size()==0)
        canvas.rectangle(r.left,r.top,r.width,r.height);
    else{
        float db=180.f;
        float pts[8];
        pts[0]=r.left + radii[0];    pts[1]=r.top + radii[0];
        pts[2]=r.right()-radii[1];   pts[3]=r.top + radii[1];
        pts[4]=r.right()-radii[2];   pts[5]=r.bottom()-radii[2];
        pts[6]=r.left + radii[3];    pts[7]=r.bottom()-radii[3];
        for(int i=0,j=0;i<8;i+=2,j++){
            canvas.translate(pts[i],pts[i+1]);
            canvas.arc(0,0,radii[j],db*degree,(db+90)*degree);
            canvas.translate(-pts[i],-pts[i+1]);
            db+=90.f;
        }canvas.line_to(pts[0]-radii[0],pts[1]);
    }
}

void GradientDrawable::draw(Canvas&canvas){
    if (!ensureValidRect())return; // nothing to draw
    auto st = mGradientState;
    const bool haveStroke = /*currStrokeAlpha > 0 &&*/ mStrokePaint &&  mStrokeWidth> 0;
    switch (st->mShape) {
    case RECTANGLE:{
            const float rad = std::min(st->mRadius,std::min(mRect.width, mRect.height) * 0.5f);
            std::vector<float>radii;
            if(st->mRadiusArray.size())radii=st->mRadiusArray;
            if(st->mRadius > 0.0f)radii={rad,rad,rad,rad};
            canvas.set_source(mFillPaint);
            drawRound(canvas,mRect,radii);
            if (haveStroke) {
                canvas.fill_preserve();
                canvas.set_source(mStrokePaint);
                canvas.stroke();
            }else{
                canvas.fill();
            } 
        }
        break;
    case LINE: {
        RectF r = mRect;
        const float y = r.top+r.height/2.f;
        if (haveStroke) {
            canvas.set_source(mStrokePaint);      
            canvas.move_to(r.left, y);
            canvas.line_to(r.left+r.width, y);
            canvas.stroke(); 
        }
        break;
    }
    case OVAL:
        canvas.save();
        canvas.translate(mRect.left+mRect.width/2.f,mRect.top+mRect.height/2.f);
        canvas.scale(1.f,mRect.height/mRect.width);
        canvas.arc(0,0,mRect.width/2.f,0,M_PI*2.f);
        if (haveStroke) {
            canvas.fill_preserve(); 
            canvas.set_source(mStrokePaint);
            canvas.stroke();
        }else canvas.fill();
        canvas.restore();
        break;
#if 0
    case RING:
        Path path = buildRing(st);
        canvas.drawPath(path, mFillPaint);
        if (haveStroke) {
            canvas.drawPath(path, mStrokePaint);
        }
#endif
        break;
    }
}

Drawable*GradientDrawable::inflate(Context*ctx,const AttributeSet&atts){
    return nullptr;	
}

}//end namespace 
