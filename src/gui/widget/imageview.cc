#include <widget/imageview.h>
#include <widget/measurespec.h>
#include <app.h>
#include <cdlog.h>
namespace cdroid{

ImageView::ImageView(Context*ctx,const AttributeSet& attrs):View(ctx,attrs){
    initImageView();
    setImageResource(attrs.getString("src"));
}

ImageView::ImageView(int w, int h)
  : View(w,h){
    initImageView();
}

void ImageView::initImageView(){
    mColorMod=false;
    mHasColorFilter=false;
    mHasDrawableTint=false;
    mHasDrawableTintMode=false;
    mBaselineAlignBottom=false;
    mBaseline=-1;
    mAlpha=255;
    mViewAlphaScale=256;
    mScaleType =FIT_CENTER;
    mMergeState=false;
    mCropToPadding=false;
    mDrawable=nullptr;
    mHaveFrame=true;
    mDrawMatrix=identity_matrix();
    mMatrix=identity_matrix();
    mRecycleableBitmapDrawable=nullptr;
    mMaxWidth = mMaxHeight = INT_MAX;
    mDrawableTintList=nullptr;
    mColorFilter=nullptr;
    mDrawableTintMode=-1;
}

ImageView::~ImageView() {
    delete mDrawable;
    delete mDrawableTintList;
    delete mColorFilter;
    delete mRecycleableBitmapDrawable;
}

void ImageView::resolveUri(){
    if (mDrawable != nullptr) {
        return;
    }
    Drawable* d = nullptr;
    if (!mResource.empty()) {
        d = getContext()->getDrawable(mResource);
        LOGW_IF(d==nullptr,"Unable to find resource: %s",mResource.c_str());
    }
    updateDrawable(d);
}
int ImageView::resolveAdjustedSize(int desiredSize, int maxSize,int measureSpec){
    int result = desiredSize;
    int specMode = MeasureSpec::getMode(measureSpec);
    int specSize = MeasureSpec::getSize(measureSpec);
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
    mScaleType=st;
    invalidate(true);
}

bool ImageView::getCropToPadding()const{
    return mCropToPadding;
}

void ImageView::setCropToPadding(bool cropToPadding){
    if (mCropToPadding != cropToPadding) {
        mCropToPadding = cropToPadding;
        //requestLayout();
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
    return mBaseline;
}

bool ImageView::verifyDrawable(Drawable* dr)const{
    return mDrawable == dr || View::verifyDrawable(dr);
}

void ImageView::jumpDrawablesToCurrentState(){
}

std::vector<int> ImageView::onCreateDrawableState()const{
    if (mState.size()==0) {
        return View::onCreateDrawableState();
    } else if (!mMergeState) {
        return mState;
    } else {
		std::vector<int>sts=View::onCreateDrawableState();
        return mergeDrawableStates(sts, mState);
    }
}
//ref: https://github.com/google/skia/blob/master/src/core/SkMatrix.cpp
//SkMatrix::setRectToRect(const SkRect& src, const SkRect& dst, ScaleToFit align)
//ScaleToFit { kFill_ScaleToFit, kStart_ScaleToFit, kCenter_ScaleToFit, kEnd_ScaleToFit, }
static void SetRect2Rect(Matrix&m,const Rect&src,const Rect&dst,int align){
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
        else   diff = dst.height - src.height * sy;
        if (align == FIT_CENTER/*kCenter_ScaleToFit*/) diff*=0.5;//diff = SkScalarHalf(diff);

        if (xLarger) tx += diff;
        else  ty += diff;
    }
    m.scale(sx,sy); m.translate(tx,ty);//this->setScaleTranslate(sx, sy, tx, ty);
}

bool ImageView::setFrame(int l, int t, int w, int h){
    bool changed = View::setFrame(l, t, w, h);
    mHaveFrame = true;
    configureBounds();
    return changed;
}
void ImageView::configureBounds(){
    if (mDrawable == nullptr /*|| !mHaveFrame*/) return;
    const int mPaddingLeft=0,mPaddingRight=0,mPaddingTop=0,mPaddingBottom=0;
    const int dwidth = mDrawableWidth;
    const int dheight = mDrawableHeight;

    const int vwidth = getWidth() - mPaddingLeft - mPaddingRight;
    const int vheight = getHeight() - mPaddingTop - mPaddingBottom;
    

    const bool fits = (dwidth < 0 || vwidth == dwidth) && (dheight < 0 || vheight == dheight);
    LOGV("drawables.setBounds(%d,%d) fits=%d mScaleType=%d",vwidth,vheight,fits,mScaleType);

    mDrawable->setBounds(0, 0, vwidth, vheight);
    
    if (dwidth <= 0 || dheight <= 0 || FIT_XY == mScaleType) {
        /* If the drawable has no intrinsic size, or we're told to
            scaletofit, then we just fill our entire view.*/
        mDrawable->setBounds(0, 0, vwidth, vheight);
        mDrawMatrix =identity_matrix();//nullptr
    } else {
        // We need to do the scaling ourself, so have the drawable
        // use its native size.
        mDrawable->setBounds(0, 0, dwidth, dheight);

        if (MATRIX == mScaleType) {
            // Use the specified matrix as-is.
            if (!IsIdentity(mMatrix)){
                mDrawMatrix = mMatrix;
            }
        } else if (fits) {
            // The bitmap fits exactly, no transform needed.
            mDrawMatrix = identity_matrix();
        } else if (CENTER == mScaleType) {
            // Center bitmap in view, no scaling.
            mDrawMatrix = mMatrix;
            mDrawMatrix.translate(round((vwidth - dwidth) * 0.5f),  round((vheight - dheight) * 0.5f));
        } else if (CENTER_CROP == mScaleType) {
            mDrawMatrix = mMatrix;

            float scale;
            float dx = 0, dy = 0;

            if (dwidth * vheight > vwidth * dheight) {
                scale = (float) vheight / (float) dheight;
                dx = (vwidth - dwidth * scale) * 0.5f;
            } else {
                scale = (float) vwidth / (float) dwidth;
                dy = (vheight - dheight * scale) * 0.5f;
            }

            mDrawMatrix.scale(scale, scale);
            mDrawMatrix.translate(round(dx), round(dy));
        } else if (CENTER_INSIDE == mScaleType) {
            mDrawMatrix = mMatrix;
            float scale;
            float dx;
            float dy;

            if (dwidth <= vwidth && dheight <= vheight) {
                scale = 1.0f;
            } else {
                scale = std::min((float) vwidth / (float) dwidth, (float) vheight / (float) dheight);
            }

            dx = round((vwidth - dwidth * scale) * 0.5f);
            dy = round((vheight- dheight * scale) * 0.5f);

            mDrawMatrix.scale(scale, scale);
            mDrawMatrix.translate(dx, dy);
        } else {
            // Generate the required transform.
            Rect src={0, 0, dwidth, dheight};
            Rect dst={0, 0, vwidth, vheight};
            mDrawMatrix = mMatrix;
	    SetRect2Rect(mDrawMatrix,src,dst,mScaleType);
            //mDrawMatrix.setRectToRect(mTempSrc, mTempDst, scaleTypeToScaleToFit(mScaleType));
        }
    }
    LOGV("ScaleType=%d DrawMatrix=%.2f,%.2f, %.2f,%.2f, %.2f,%.2f",mScaleType,
	mDrawMatrix.xx,mDrawMatrix.yx,mDrawMatrix.xy,mDrawMatrix.yy,mDrawMatrix.x0,mDrawMatrix.y0);
}

void ImageView::invalidateDrawable(Drawable& dr){
    if (mDrawable==&dr) {
        // update cached drawable dimensions if they've changed
        const int w = dr.getIntrinsicWidth();
        const int h = dr.getIntrinsicHeight();
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
        //mResource = 0;
        //mUri = null;

        const int oldWidth = mDrawableWidth;
        const int oldHeight = mDrawableHeight;

        updateDrawable(drawable);

        if (oldWidth != mDrawableWidth || oldHeight != mDrawableHeight) {
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

    updateDrawable(nullptr);
    mResource = resId;
    //mUri = nullptr;
    resolveUri();

    if (oldWidth != mDrawableWidth || oldHeight != mDrawableHeight) {
        requestLayout();
    }
    invalidate(true);
}

void ImageView::setImageTintList(ColorStateList*tint){
    mDrawableTintList = tint;
    mHasDrawableTint = true;
    applyImageTint();
}

ColorStateList* ImageView::getImageTintList(){
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
    setColorFilter(color, SRC_ATOP);
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
    mAlpha=alpha&0xFF;
}
int  ImageView::getImageAlpha()const{
    return mAlpha;
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

void ImageView::updateDrawable(Drawable*d){
    if (d != mRecycleableBitmapDrawable && mRecycleableBitmapDrawable != nullptr) {
        mRecycleableBitmapDrawable->setBitmap(nullptr);
    }

    bool sameDrawable = false;

    if (mDrawable != nullptr) {
        sameDrawable = mDrawable == d;
        mDrawable->setCallback(nullptr);
        unscheduleDrawable(*mDrawable);
        if ( !sameDrawable ){//&& isAttachedToWindow()) {
            mDrawable->setVisible(false, false);
        }
    }

    mDrawable = d;

    if (d != nullptr) {
        d->setCallback(this);
        d->setLayoutDirection(getLayoutDirection());
        if (d->isStateful()) {
            d->setState(getDrawableState());
        }
        if (!sameDrawable ) {
            const bool visible =getVisibility() == VISIBLE;
            d->setVisible(visible, true);
        }
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
            mDrawableHeight = h;
            requestLayout();
        }
    }
}

void ImageView::setSelected(bool selected){
    View::setSelected(selected);
    resizeFromDrawable();
}

void ImageView::onDraw(Canvas& canvas) {
    View::onDraw(canvas);
    const int mPaddingLeft=0,mPaddingTop=0,mPaddingRight=0,mPaddingBottom=0;
    LOGV("%p'mDrawable %p pos:%d,%d Size=%dx%d alpha=%f",this,mDrawable,mLeft,mTop,mDrawableWidth,mDrawableHeight,getAlpha());
    if (mDrawable == nullptr||mDrawableWidth == 0 || mDrawableHeight == 0) return;

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
        LOGV("DrawMatrix=%.2f,%.2f, %.2f,%.2f, %.2f,%.2f",mDrawMatrix.xx,mDrawMatrix.yx,
		mDrawMatrix.xy,mDrawMatrix.yy,mDrawMatrix.x0,mDrawMatrix.y0);
        canvas.translate(mPaddingLeft, mPaddingTop);
        canvas.transform(mDrawMatrix);
    
        mDrawable->draw(canvas);
        canvas.restore();
    }
}

}

