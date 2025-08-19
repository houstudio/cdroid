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
#include <widget/imageview.h>
#include <core/textutils.h>
#include <porting/cdlog.h>
using namespace Cairo;
namespace cdroid{

DECLARE_WIDGET(ImageView)

ImageView::ImageView(Context*ctx,const AttributeSet& attrs)
  :View(ctx,attrs){
    initImageView();
    mBaselineAlignBottom = attrs.getBoolean("baselineAlignBottom",false);
    mBaseline = attrs.getDimensionPixelSize("baseline",-1);
    setAdjustViewBounds(attrs.getBoolean("adjustViewBounds",false));
    mCropToPadding = attrs.getBoolean("cropToPadding",false);
    const int scaleType = attrs.getInt("scaleType",std::unordered_map<std::string,int>{
            {"matrix",ScaleType::MATRIX}, {"fitXY",ScaleType::FIT_XY},
            {"fitStart",ScaleType::FIT_START},{"fitCenter",ScaleType::FIT_CENTER},
            {"fitEnd",ScaleType::FIT_END},   {"center",ScaleType::CENTER},
            {"centerCrop",ScaleType::CENTER_CROP},{"centerInside",ScaleType::CENTER_INSIDE}
         },-1);
    if(scaleType>=0)setScaleType(scaleType);
    Drawable*d = attrs.getDrawable("src");
    if(d)setImageDrawable(d);
    mDrawableTintList = attrs.getColorStateList("tint");
    mHasDrawableTint = mDrawableTintList!=nullptr;
    setMaxWidth (attrs.getDimensionPixelSize("maxWidth" ,INT_MAX));
    setMaxHeight(attrs.getDimensionPixelSize("maxHeight",INT_MAX));
    setImageAlpha(attrs.getInt("alpha",255));
    const int radii = attrs.getInt("radius",0);
    mRadii[0] = attrs.getInt("topLeftRadius",radii);
    mRadii[1] = attrs.getInt("topRightRadius",radii);
    mRadii[2] = attrs.getInt("bottomRightRadius",radii);
    mRadii[3] = attrs.getInt("bottomLeftRadius",radii);
    mCropToPadding = attrs.getBoolean("cropToPadding",false);
    applyImageTint();
}

ImageView::ImageView(int w, int h)
  : View(w,h){
    initImageView();
}

void ImageView::initImageView(){
    mColorMod = false;
    mHasColorFilter = false;
    mHasDrawableTint= false;
    mHasDrawableTintMode = false;
    mBaselineAlignBottom = false;
    mBaseline = -1;
    mAlpha = 255;
    mLevel = INT_MIN;
    mViewAlphaScale= 256;
    mDrawableWidth = mDrawableHeight = -1;
    mScaleType  = FIT_CENTER;
    mHaveFrame  = false;
    mMergeState = false;
    mCropToPadding = false;
    mAdjustViewBounds = false;
    mDrawable   = nullptr;
    mDrawMatrix = identity_matrix();
    mMatrix = identity_matrix();
    mRecycleableBitmapDrawable = nullptr;
    mMaxWidth = mMaxHeight = INT_MAX;
    mDrawableTintList = nullptr;
    mColorFilter = nullptr;
    mDrawableTintMode = -1;
    mRadii[0] = mRadii[1] = 0;
    mRadii[2] = mRadii[3] = 0;
}

ImageView::~ImageView() {
    if(mDrawable!=mRecycleableBitmapDrawable)
        delete mRecycleableBitmapDrawable;
    delete mDrawable;
    //delete mDrawableTintList;//cant be destroied.
    delete mColorFilter;
}

void ImageView::resolveUri(){
    if (mDrawable != nullptr) {
        return;
    }
    if (!mResource.empty()) {
        if(strpbrk(mResource.c_str(),"@:")==nullptr){
            RefPtr<Cairo::ImageSurface>bitmap = getContext()->loadImage(mResource);
            setImageBitmap(bitmap);
            LOGW_IF(bitmap==nullptr,"Unable to find resource: %s",mResource.c_str());
        }else if(mResource.compare("@null")){
            Drawable* d = getContext()->getDrawable(mResource);
            LOGW_IF(d==nullptr,"Unable to find resource: %s",mResource.c_str());
            updateDrawable(d);
        }else{
            updateDrawable(nullptr);
        }
    }
    //updateDrawable(d);
}

int ImageView::resolveAdjustedSize(int desiredSize, int maxSize,int measureSpec){
    int result = desiredSize;
    const int specMode = MeasureSpec::getMode(measureSpec);
    const int specSize = MeasureSpec::getSize(measureSpec);
    switch (specMode) {
    case MeasureSpec::UNSPECIFIED:
        /* Parent says we can be as big as we want. Just don't be larger
			than max size imposed on ourselves.*/
        result = std::min(desiredSize, maxSize);
        break;
    case MeasureSpec::AT_MOST:
        // Parent says we can be as big as we want, up to specSize.
        // Don't be larger than specSize, and don't be larger than
        // the max size imposed on ourselves.
        result = std::min(std::min(desiredSize, specSize), maxSize);
        break;
    case MeasureSpec::EXACTLY:
        // No choice. Do what we are told.
        result = specSize;
        break;
    }
    return result;
}

void ImageView::onMeasure(int widthMeasureSpec, int heightMeasureSpec){
    int w,h;
    resolveUri();
    // Desired aspect ratio of the view's contents (not including padding)
    float desiredAspect = 0.0f;

    // We are allowed to change the view's width
    bool resizeWidth = false;
    // We are allowed to change the view's height
    bool resizeHeight = false;

    const int widthSpecMode = MeasureSpec::getMode(widthMeasureSpec);
    const int heightSpecMode = MeasureSpec::getMode(heightMeasureSpec);

    if (mDrawable == nullptr) {
        // If no drawable, its intrinsic size is 0.
        mDrawableWidth = -1;
        mDrawableHeight = -1;
        w = h = 0;
    } else {
        w = mDrawableWidth;
        h = mDrawableHeight;
        if (w <= 0) w = 1;
        if (h <= 0) h = 1;
        // We are supposed to adjust view bounds to match the aspect
        // ratio of our drawable. See if that is possible.
        if (mAdjustViewBounds) {
            resizeWidth = widthSpecMode != MeasureSpec::EXACTLY;
            resizeHeight = heightSpecMode != MeasureSpec::EXACTLY;
            desiredAspect = (float) w / (float) h;
        }
    }

    int pleft = mPaddingLeft;
    int pright = mPaddingRight;
    int ptop = mPaddingTop;
    int pbottom = mPaddingBottom;

    int widthSize;
    int heightSize;

    if (resizeWidth || resizeHeight) {
        /* If we get here, it means we want to resize to match the
            drawables aspect ratio, and we have the freedom to change at
            least one dimension.*/

        // Get the max possible width given our constraints
        widthSize = resolveAdjustedSize(w + pleft + pright, mMaxWidth, widthMeasureSpec);
        // Get the max possible height given our constraints
        heightSize = resolveAdjustedSize(h + ptop + pbottom, mMaxHeight, heightMeasureSpec);

        if (desiredAspect != 0.0f) {
            // See what our actual aspect ratio is
            float actualAspect = (float)(widthSize-pleft-pright)/(heightSize - ptop - pbottom);
            if (std::abs(actualAspect - desiredAspect) > 0.0000001) {
                bool done = false;
                // Try adjusting width to be proportional to height
                if (resizeWidth) {
                    int newWidth = (int)(desiredAspect*(heightSize-ptop-pbottom)) +pleft + pright;
                    // Allow the width to outgrow its original estimate if height is fixed.
                    if (!resizeHeight /*&& !sCompatAdjustViewBounds*/) {
                        widthSize = resolveAdjustedSize(newWidth, mMaxWidth, widthMeasureSpec);
                    }
                    if (newWidth <= widthSize) {
                        widthSize = newWidth;
                        done = true;
                    }
                }
                // Try adjusting height to be proportional to width
                if (!done && resizeHeight) {
                    int newHeight = (int)((widthSize-pleft-pright)/desiredAspect)+ptop + pbottom;
                    // Allow the height to outgrow its original estimate if width is fixed.
                    if (!resizeWidth /*&& !sCompatAdjustViewBounds*/) {
                        heightSize = resolveAdjustedSize(newHeight, mMaxHeight,heightMeasureSpec);
                    }
                    if (newHeight <= heightSize) {
                        heightSize = newHeight;
                    }
                }
            }
        }
    } else {
        /* We are either don't want to preserve the drawables aspect ratio,
            or we are not allowed to change view dimensions. Just measure in
            the normal way.*/
        w += pleft + pright;
        h += ptop + pbottom;
        w = std::max(w, getSuggestedMinimumWidth());
        h = std::max(h, getSuggestedMinimumHeight());

        widthSize = resolveSizeAndState(w, widthMeasureSpec, 0);
        heightSize = resolveSizeAndState(h, heightMeasureSpec, 0);
    }
    setMeasuredDimension(widthSize, heightSize);
}

static bool IsEmptyMatrix(const Matrix&m){
	return m.xx==.0&&m.yx==.0&&m.xy==.0&&m.yy==.0&&m.x0==0&&m.y0==0;
}

static bool IsIdentity(const Matrix&m){
	return m.xx==1.&&m.yx==.0&&m.xy==.0&&m.yy==1&&m.x0==0&&m.y0==0;
}

int ImageView::getScaleType()const{
    return mScaleType;
}

void ImageView::setScaleType(int st){
    if(mScaleType != st){
        mScaleType = st;
        requestLayout();
        invalidate(true);
    }
}

static bool operator==(const Cairo::Matrix& m1, const Cairo::Matrix& m2) {
    return (m1.xx == m2.xx )&& (m1.yx == m2.yx) && (m1.xy == m2.xy) && (m1.yy == m2.yy) && (m1.x0 == m2.x0) && (m1.y0 == m2.y0);
}

static bool operator!=(const Cairo::Matrix& m1, const Cairo::Matrix& m2) {
    return (m1.xx != m2.xx) || (m1.yx != m2.yx) || (m1.xy != m2.xy) || (m1.yy == m2.yy) || (m1.x0 == m2.x0) ||( m1.y0 == m2.y0);
}

void ImageView::setImageMatrix(const Cairo::Matrix& matrix) {
    // collapse null and identity to just null
#if 0
    if (matrix != null && matrix.isIdentity()) {
        matrix = null;
    }

    // don't invalidate unless we're actually changing our matrix
    if (matrix == null && !IsIdentity(mMatrix) ||
            matrix != null && !(mMatrix==matrix)) {
        mMatrix =matrix;
        configureBounds();
        invalidate();
    }
#else
    if(/*!IsIdentity(mMatrix) &&*/ (mMatrix!=matrix)){
        mMatrix = matrix;
        configureBounds();
        invalidate();
    }
#endif
}

Cairo::Matrix ImageView::getImageMatrix() const{
    return mDrawMatrix;
}

bool ImageView::getCropToPadding()const{
    return mCropToPadding;
}

void ImageView::setCropToPadding(bool cropToPadding){
    if (mCropToPadding != cropToPadding) {
        mCropToPadding = cropToPadding;
        requestLayout();
        invalidate(true);
    }
}

void ImageView::setMaxWidth(int maxWidth){
    mMaxWidth = maxWidth;
}

void ImageView::setMaxHeight(int maxHeight){
    mMaxHeight = maxHeight;
}

int ImageView::getMaxWidth()const{
    return mMaxWidth;
}

int ImageView::getMaxHeight()const{
    return mMaxWidth;
}

Drawable*ImageView::getDrawable(){
    if (mDrawable == mRecycleableBitmapDrawable) {
        // Consider our cached version dirty since app code now has a reference to its
        mRecycleableBitmapDrawable = nullptr;
    }
    return mDrawable;
}

void ImageView::onPopulateAccessibilityEventInternal(AccessibilityEvent& event){
    View::onPopulateAccessibilityEventInternal(event);
    const std::string contentDescription = getContentDescription();
    if (!TextUtils::isEmpty(contentDescription)) {
         event.getText().push_back(contentDescription);
    }
}

bool ImageView::getAdjustViewBounds()const{
    return mAdjustViewBounds;
}

void ImageView::setAdjustViewBounds(bool adjustViewBounds){
    mAdjustViewBounds=adjustViewBounds;
    if (adjustViewBounds) {
        setScaleType(FIT_CENTER);
    }
}

void ImageView::setBaseline(int baseline){
    if (mBaseline != baseline) {
        mBaseline = baseline;
        requestLayout();
    }
}

void ImageView::setBaselineAlignBottom(bool aligned) {
    if (mBaselineAlignBottom != aligned) {
        mBaselineAlignBottom = aligned;
        requestLayout();
    }
}

bool ImageView::getBaselineAlignBottom()const{
    return mBaselineAlignBottom;
}

int ImageView::getBaseline(){
    if(mBaselineAlignBottom)return getMeasuredHeight();
    return mBaseline;
}

bool ImageView::verifyDrawable(Drawable* dr)const{
    return mDrawable == dr || View::verifyDrawable(dr);
}

void ImageView::jumpDrawablesToCurrentState(){
    View::jumpDrawablesToCurrentState();
    if (mDrawable) mDrawable->jumpToCurrentState();
}

std::vector<int> ImageView::onCreateDrawableState(int extraSpace){
    if (mState.size()==0) {
        return View::onCreateDrawableState(extraSpace);
    } else if (!mMergeState) {
        return mState;
    } else {
        std::vector<int>sts=View::onCreateDrawableState(extraSpace+mState.size());
        return mergeDrawableStates(sts, mState);
    }
}

//ref: https://github.com/google/skia/blob/master/src/core/SkMatrix.cpp
//SkMatrix::setRectToRect(const SkRect& src, const SkRect& dst, ScaleToFit align)
//ScaleToFit { kFill_ScaleToFit, kStart_ScaleToFit, kCenter_ScaleToFit, kEnd_ScaleToFit, }
static void setRect2Rect(Matrix&m,const Rect&src,const Rect&dst,int align){
    float tx, sx = (float)dst.width/ src.width;
    float ty, sy = (float)dst.height / src.height;
    bool  xLarger = false;
    if (align != FIT_XY){//kFill_ScaleToFit) {
        if (sx > sy) {  xLarger = true;  sx = sy; }
        else { sy = sx;  }
    }
    tx = dst.left - src.left * sx;
    ty = dst.top - src.top * sy;
    if (align ==FIT_CENTER||align==FIT_END){// kCenter_ScaleToFit || align == kEnd_ScaleToFit) {
        float diff;
        if (xLarger) diff = dst.width - src.width * sy;
        else diff = dst.height - src.height * sy;
        if (align == FIT_CENTER/*kCenter_ScaleToFit*/) diff*=0.5;//diff = SkScalarHalf(diff);

        if (xLarger) tx += diff;
        else ty += diff;
    }
    m.translate(tx,ty);//this->setScaleTranslate(sx, sy, tx, ty);
    m.scale(sx,sy);
}

bool ImageView::setFrame(int l, int t, int w, int h){
    const bool changed = View::setFrame(l, t, w, h);
    mHaveFrame = true;
    configureBounds();
    return changed;
}

void ImageView::configureBounds(){
    if (mDrawable == nullptr || !mHaveFrame) return;
    const int dwidth = mDrawableWidth;
    const int dheight = mDrawableHeight;

    const int vwidth = getWidth() - mPaddingLeft - mPaddingRight;
    const int vheight = getHeight() - mPaddingTop - mPaddingBottom;
    
    const bool fits = (dwidth < 0 || vwidth == dwidth) && (dheight < 0 || vheight == dheight);

    if ((dwidth <= 0) || (dheight <= 0) || (ScaleType::FIT_XY == mScaleType)) {
        /* If the drawable has no intrinsic size, or we're told to
           scaletofit, then we just fill our entire view.*/
        mDrawable->setBounds(0, 0, vwidth, vheight);
        mDrawMatrix = identity_matrix();
    } else {
        // We need to do the scaling ourself, so have the drawable
        // use its native size.
        mDrawable->setBounds(0, 0, dwidth, dheight);
        if (ScaleType::MATRIX == mScaleType) {
            // Use the specified matrix as-is.
            if (!IsIdentity(mMatrix)){
                mDrawMatrix = mMatrix;
            }else{
                mDrawMatrix = identity_matrix();
            }
        } else if (fits) {
            // The bitmap fits exactly, no transform needed.
            mDrawMatrix = identity_matrix();
        } else if (ScaleType::CENTER == mScaleType) {
            // Center bitmap in view, no scaling.
            mDrawMatrix = mMatrix;
            mDrawMatrix.translate(std::round((vwidth - dwidth) * 0.5f),  std::round((vheight - dheight) * 0.5f));
        } else if (ScaleType::CENTER_CROP == mScaleType) {
            float scale , dx = 0, dy = 0;
            mDrawMatrix = mMatrix;

            if (dwidth * vheight > vwidth * dheight) {
                scale = (float) vheight / (float) dheight;
                dx = (vwidth - dwidth * scale) * 0.5f;
            } else {
                scale = (float) vwidth / (float) dwidth;
                dy = (vheight - dheight * scale) * 0.5f;
            }

            mDrawMatrix.translate(round(dx), round(dy));
            mDrawMatrix.scale(scale, scale);
        } else if (ScaleType::CENTER_INSIDE == mScaleType) {
            float scale , dx, dy;
            mDrawMatrix = mMatrix;
            if (dwidth <= vwidth && dheight <= vheight) {
                scale = 1.0f;
            } else {
                scale = std::min((float) vwidth / (float) dwidth, (float) vheight / (float) dheight);
            }

            dx = std::round((vwidth - dwidth * scale) * 0.5f);
            dy = std::round((vheight- dheight * scale) * 0.5f);

            mDrawMatrix.translate(dx, dy);
            mDrawMatrix.scale(scale, scale);
        } else {
            // Generate the required transform.
            Rect src = {0, 0, dwidth, dheight};
            Rect dst = {0, 0, vwidth, vheight};
            mDrawMatrix = mMatrix;
            setRect2Rect(mDrawMatrix,src,dst,mScaleType);
        }
    }
    LOGV("%p:%d ScaleType=%d DrawMatrix=%.2f,%.2f, %.2f,%.2f, %.2f,%.2f",this,mID,mScaleType,
	    mDrawMatrix.xx,mDrawMatrix.yx,mDrawMatrix.xy,mDrawMatrix.yy,mDrawMatrix.x0,mDrawMatrix.y0);
}

void ImageView::drawableStateChanged(){
    View::drawableStateChanged();
    if(mDrawable && mDrawable->isStateful() && mDrawable->setState(getDrawableState())){
        invalidateDrawable(*mDrawable);
    }
}

void ImageView::drawableHotspotChanged(float x, float y){
    View::drawableHotspotChanged(x,y);
    if(mDrawable)mDrawable->setHotspot(x,y);
}

void ImageView::invalidateDrawable(Drawable& dr){
    if (mDrawable==&dr) {
        // update cached drawable dimensions if they've changed
        const int w = dr.getIntrinsicWidth();
        const int h = dr.getIntrinsicHeight();
        LOGV("wh=%d,%d ->%d,%d",w,h,mDrawableWidth,mDrawableHeight);
        if (w != mDrawableWidth || h != mDrawableHeight) {
            mDrawableWidth = w;
            mDrawableHeight = h;
            // updates the matrix, which is dependent on the bounds
            configureBounds();
        }
        /* we invalidate the whole view in this case because it's very
        * hard to know where the drawable actually is. This is made
        * complicated because of the offsets and transformations that
        * can be applied. In theory we could get the drawable's bounds
        * and run them through the transformation and offsets, but this
        * is probably not worth the effort.
        */
        invalidate(true);
    } else {
        View::invalidateDrawable(dr);
    }
}

void ImageView::setImageDrawable(Drawable*drawable){
   if (mDrawable != drawable) {
        mResource.clear();
        //mUri = null;

        const int oldWidth = mDrawableWidth;
        const int oldHeight = mDrawableHeight;

        updateDrawable(drawable);

        if ((oldWidth != mDrawableWidth) || (oldHeight != mDrawableHeight)) {
            requestLayout();
        }
        invalidate(true);
    }
}

void ImageView::setImageLevel(int level){
    mLevel = level;
    if (mDrawable != nullptr) {
        mDrawable->setLevel(level);
        resizeFromDrawable();
    }
}

void ImageView::setImageResource(const std::string& resId) {
    // The resource configuration may have changed, so we should always
    // try to load the resource even if the resId hasn't changed.
    const int oldWidth = mDrawableWidth;
    const int oldHeight = mDrawableHeight;
    if(mResource==resId)return;
    updateDrawable(nullptr);
    mResource = resId;
    resolveUri();

    if ((oldWidth != mDrawableWidth) || (oldHeight != mDrawableHeight)) {
        requestLayout();
    }
    invalidate(true);
}

void ImageView::imageDrawableCallback(Drawable*d,const std::string&uri,const std::string resid){
}

Runnable ImageView::setImageResourceAsync(const std::string&resid){
    Runnable r;
    return r;
}

void ImageView::setImageURI(const std::string&uri){
}

Runnable ImageView::setImageURIAsync(const std::string&uri){
    Runnable r;
    return r;
}

void ImageView::setImageTintList(const ColorStateList*tint){
    if(mDrawableTintList!=tint){
        mDrawableTintList = tint;
        mHasDrawableTint = (tint!=nullptr);
        applyImageTint();
    }
}

const ColorStateList* ImageView::getImageTintList(){
    return mDrawableTintList;
}

void ImageView::setImageTintMode(int tintMode){
    mDrawableTintMode = tintMode;
    mHasDrawableTintMode = true;

    applyImageTint();    
}

int ImageView::getImageTintMode()const{
    return mDrawableTintMode;
}

void ImageView::setColorFilter(int color,int mode){
    setColorFilter(new PorterDuffColorFilter(color, mode));
}

void ImageView::setColorFilter(int color){
    setColorFilter(color, PorterDuff::Mode::SRC_ATOP);
}

void ImageView::setColorFilter(ColorFilter* cf){
    if (mColorFilter != cf) {
        mColorFilter = cf;
        mHasColorFilter = true;
        mColorMod = true;
        applyColorMod();
        invalidate();
    } 
}

void ImageView::clearColorFilter(){
    setColorFilter(nullptr);
}

ColorFilter* ImageView::getColorFilter(){
    return mColorFilter;
}

void ImageView::setImageAlpha(int alpha){
    mAlpha = alpha & 0xFF;
}

int  ImageView::getImageAlpha()const{
    return mAlpha;
}

bool ImageView::isOpaque() const{
    return View::isOpaque() || mDrawable /*&& mXfermode == null*/
            && (mDrawable->getOpacity() == PixelFormat::OPAQUE)
            && (mAlpha * mViewAlphaScale >> 8 == 255)
            && isFilledByImage();
}

static bool isRectilinear(const Cairo::Matrix& matrix) {
    // 判断矩阵是否保持矩形形状
    if ((matrix.xx == 0) || (matrix.yy == 0) || (matrix.yx != 0) || (matrix.xy != 0) ) {
        return false;
    }

    return true;
}

bool ImageView::isFilledByImage() const{
    if (mDrawable == nullptr) {
        return false;
    }

    Rect bounds = mDrawable->getBounds();
    Matrix matrix = mDrawMatrix;
    if (IsIdentity(mDrawMatrix)) {
        return (bounds.left <= 0) && (bounds.top <= 0) && (bounds.width >= getWidth())
                && (bounds.height >= getHeight());
    } else if (isRectilinear(matrix)){//matrix.rectStaysRect()) {
        Rect boundsDst = bounds;
        matrix.transform_rectangle((Cairo::RectangleInt&)boundsDst);
        return (boundsDst.left <= 0) && (boundsDst.top <= 0) && (boundsDst.width >= getWidth())
                && (boundsDst.height >= getHeight());
    }
    // If the matrix doesn't map to a rectangle, assume the worst.
    return false;
}

void ImageView::onVisibilityAggregated(bool isVisible) {
    View::onVisibilityAggregated(isVisible);
    // Only do this for new apps post-Nougat
    if (mDrawable/*&& !sCompatDrawableVisibilityDispatch*/) {
        mDrawable->setVisible(isVisible, false);
    }
}

void ImageView::setVisibility(int visibility) {
    View::setVisibility(visibility);
    // Only do this for old apps pre-Nougat; new apps use onVisibilityAggregated
    if (mDrawable/* && sCompatDrawableVisibilityDispatch*/) {
        mDrawable->setVisible(visibility == VISIBLE, false);
    }
}

void ImageView::onAttachedToWindow() {
    View::onAttachedToWindow();
    // Only do this for old apps pre-Nougat; new apps use onVisibilityAggregated
    if (mDrawable/*&& sCompatDrawableVisibilityDispatch*/) {
        mDrawable->setVisible(getVisibility() == VISIBLE, false);
    }
}

void ImageView::onDetachedFromWindow() {
    View::onDetachedFromWindow();
    // Only do this for old apps pre-Nougat; new apps use onVisibilityAggregated
    if (mDrawable/*&& sCompatDrawableVisibilityDispatch*/) {
        mDrawable->setVisible(false, false);
        unscheduleDrawable(*mDrawable);/*added by zhhou*/
    }
}

void ImageView::applyImageTint() {
    if (mDrawable != nullptr && (mHasDrawableTint || mHasDrawableTintMode)) {
        mDrawable = mDrawable->mutate();

        if (mHasDrawableTint) {
            mDrawable->setTintList(mDrawableTintList);
        }

        if (mHasDrawableTintMode) {
            mDrawable->setTintMode(mDrawableTintMode);
        }

        // The drawable (or one of its children) may not have been
        // stateful before applying the tint, so let's try again.
        if (mDrawable->isStateful()) {
            mDrawable->setState(getDrawableState());
        }
    }
}

void ImageView::applyColorMod(){
    if (mDrawable != nullptr && mColorMod) {
        mDrawable = mDrawable->mutate();
        if (mHasColorFilter) {
            mDrawable->setColorFilter(mColorFilter);
        }
        //mDrawable->setXfermode(mXfermode);
        mDrawable->setAlpha(mAlpha * mViewAlphaScale >> 8);
    }
}

void ImageView::resizeFromDrawable(){
    if (mDrawable != nullptr) {
        int w = mDrawable->getIntrinsicWidth();
        if (w < 0) w = mDrawableWidth;
        int h = mDrawable->getIntrinsicHeight();
        if (h < 0) h = mDrawableHeight;
        if (w != mDrawableWidth || h != mDrawableHeight) {
            mDrawableWidth = w;
            mDrawableHeight = h;
            requestLayout();
        }
    }
}

void ImageView::onRtlPropertiesChanged(int layoutDirection) {
    View::onRtlPropertiesChanged(layoutDirection);

    if (mDrawable != nullptr) {
        mDrawable->setLayoutDirection(layoutDirection);
    }
}

void ImageView::updateDrawable(Drawable*d){
    if (d != mRecycleableBitmapDrawable && mRecycleableBitmapDrawable != nullptr) {
        mRecycleableBitmapDrawable->setBitmap(nullptr);
    }

    bool sameDrawable = false;

    if (mDrawable != nullptr) {
        sameDrawable = (mDrawable == d);
        mDrawable->setCallback(nullptr);
        unscheduleDrawable(*mDrawable);
        if ( !sameDrawable && isAttachedToWindow()) {
            mDrawable->setVisible(false, false);
        }
        if(mRecycleableBitmapDrawable!=mDrawable)
            delete mDrawable;
    }

    mDrawable = d;

    if (d != nullptr) {
        d->setCallback(this);
        d->setLayoutDirection(getLayoutDirection());
        if (d->isStateful()) {
            d->setState(getDrawableState());
        }
        if (!sameDrawable ) {
            const bool visible = isAttachedToWindow() && (getWindowVisibility() == VISIBLE) && isShown();
            d->setVisible(visible, true);
        }
        if(mLevel!=INT_MIN)
            d->setLevel(mLevel);
        mDrawableWidth = d->getIntrinsicWidth();
        mDrawableHeight = d->getIntrinsicHeight();
        applyImageTint();
        applyColorMod();
        configureBounds();
    } else {
        mDrawableWidth = mDrawableHeight = -1;
    }
}

void ImageView::setImageBitmap(RefPtr<ImageSurface>bitmap){
    if(mDrawable!=mRecycleableBitmapDrawable)
        delete mDrawable;
    mDrawable = nullptr;
    if (mRecycleableBitmapDrawable == nullptr) {
        mRecycleableBitmapDrawable = new BitmapDrawable(bitmap);
    } else {
        mRecycleableBitmapDrawable->setBitmap(bitmap);
    }
    setImageDrawable(mRecycleableBitmapDrawable);
}

void ImageView::setImageState(const std::vector<int>&state, bool merge){
    if (mDrawable ) {
        int w = mDrawable->getIntrinsicWidth();
        if (w < 0) w = mDrawableWidth;
        int h = mDrawable->getIntrinsicHeight();
        if (h < 0) h = mDrawableHeight;
        if (w != mDrawableWidth || h != mDrawableHeight) {
            mDrawableWidth = w;
            mDrawableHeight= h;
            requestLayout();
        }
    }
}

void ImageView::setSelected(bool selected){
    View::setSelected(selected);
    resizeFromDrawable();
}

void ImageView::setCornerRadii(int radius){
    mRadii[0] = mRadii[1] = radius;
    mRadii[2] = mRadii[3] = radius;
    invalidate();
}

void ImageView::setCornerRadii(int topLeftRadius,int topRightRadius,int bottomRightRadius,int bottomLeftRadius){
    mRadii[0] = topLeftRadius;
    mRadii[1] = topRightRadius;
    mRadii[2] = bottomRightRadius;
    mRadii[3] = bottomLeftRadius;
    invalidate();
}

void ImageView::animateTransform(const Cairo::Matrix* matrix) {
    if (mDrawable == nullptr) {
        return;
    }
    if (matrix == nullptr) {
        const int vwidth = getWidth() - mPaddingLeft - mPaddingRight;
        const int vheight = getHeight() - mPaddingTop - mPaddingBottom;
        mDrawable->setBounds(0, 0, vwidth, vheight);
        mDrawMatrix = identity_matrix();
    } else {
        mDrawable->setBounds(0, 0, mDrawableWidth, mDrawableHeight);
        mDrawMatrix = *matrix;
    }
    invalidate();
}

void ImageView::onDraw(Canvas& canvas) {
    if ((mDrawable == nullptr)||(mDrawableWidth == 0) || (mDrawableHeight == 0)) return;
 
    const double degrees = M_PI / 180.f;

    const int width = getWidth();
    const int height= getHeight();
    if(mRadii[0]||mRadii[1]||mRadii[2]||mRadii[3]){
        canvas.begin_new_sub_path();
        canvas.arc( width - mRadii[1], mRadii[1], mRadii[1], -90 * degrees, 0 * degrees);
        canvas.arc( width - mRadii[2], height - mRadii[2], mRadii[2], 0 * degrees, 90 * degrees);
        canvas.arc( mRadii[3], height - mRadii[3], mRadii[3], 90 * degrees, 180 * degrees);
        canvas.arc( mRadii[0], mRadii[0], mRadii[0], 180 * degrees, 270 * degrees);
        canvas.close_path();
        canvas.clip();
    }

    if (IsIdentity(mDrawMatrix) && mPaddingTop == 0 && mPaddingLeft == 0) {
        mDrawable->setAlpha(getAlpha()*255);
        mDrawable->draw(canvas);
    } else {
        canvas.save();
        if (mCropToPadding) {
            canvas.rectangle(mScrollX + mPaddingLeft, mScrollY + mPaddingTop,
                    mScrollX + getWidth() - mPaddingRight,
                    mScrollY  + getHeight()- mPaddingBottom);
            canvas.clip();
        }
        LOGV("%p:%d DrawMatrix=%.2f,%.2f, %.2f,%.2f, %.2f,%.2f",this,mID,mDrawMatrix.xx,mDrawMatrix.yx,
            mDrawMatrix.xy,mDrawMatrix.yy,mDrawMatrix.x0,mDrawMatrix.y0);
        canvas.translate(mPaddingLeft, mPaddingTop);
        canvas.transform(mDrawMatrix);

        mDrawable->draw(canvas);
        canvas.restore();
    }
}

}

