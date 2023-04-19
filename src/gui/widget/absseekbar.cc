#include <view/viewgroup.h>
#include <widget/absseekbar.h>
#include <widget/R.h>
#include <math.h>
#include <cdtypes.h>
#include <cdlog.h>

#define NO_ALPHA 0xFF
namespace cdroid{

DECLARE_WIDGET(AbsSeekBar)

AbsSeekBar::AbsSeekBar(Context*ctx,const AttributeSet&attrs):ProgressBar(ctx,attrs){
    initSeekBar();

    setThumb(attrs.getDrawable("thumb"));
    setTickMark(attrs.getDrawable("tickMark"));
    const int thumbOffset=attrs.getDimensionPixelOffset("thumbOffset",getThumbOffset());
    setThumbOffset(thumbOffset);

    const bool useDisabledAlpha = attrs.getBoolean("useDisabledAlpha", true);
    mDisabledAlpha = useDisabledAlpha?attrs.getFloat("disabledAlpha", 0.5f):1.f;

    applyThumbTint();
    applyTickMarkTint();

    mScaledTouchSlop = ViewConfiguration::get(ctx).getScaledTouchSlop();
}

AbsSeekBar::AbsSeekBar(int w,int h):ProgressBar(w,h){
    initSeekBar();
}

void AbsSeekBar::initSeekBar(){
    mKeyProgressIncrement=5;
    mSplitTrack=false;
    mThumb=nullptr;
    mThumbOffset=0;
    mTickMark = nullptr;
    mIsUserSeekable=true;
    mIsDragging = false;
    mTouchDownX = .0;
    mHasThumbTint = false;
    mHasThumbTintMode =false;
    mDisabledAlpha=1.f;
    mTouchProgressOffset =.0f;
    mTouchThumbOffset =.0f;
    mScaledTouchSlop=ViewConfiguration::get(mContext).getScaledTouchSlop();
    setFocusable(true);
}

void AbsSeekBar::setMin(int min) {
    ProgressBar::setMin(min);
    int range = getMax() - getMin();

    if ((mKeyProgressIncrement == 0) || (range / mKeyProgressIncrement > 20)) {
        // It will take the user too long to change this via keys, change it
        // to something more reasonable
        setKeyProgressIncrement(std::max(1.f,(float)round((float) range / 20)));
    }
}

void AbsSeekBar::setMax(int max) {
    ProgressBar::setMax(max);
    int range = getMax() - getMin();

    if ((mKeyProgressIncrement == 0) || (range / mKeyProgressIncrement > 20)) {
        // It will take the user too long to change this via keys, change it
        // to something more reasonable
        setKeyProgressIncrement(std::max(1.f,(float)round((float) range / 20)));
    }
}

void AbsSeekBar::onSizeChanged(int w,int h,int ow,int oh){
    ProgressBar::onSizeChanged(w,h,ow,oh);
    updateThumbAndTrackPos(w,h);
}

void AbsSeekBar::onVisualProgressChanged(int id, float scale){
    ProgressBar::onVisualProgressChanged(id,scale);
    if (id ==R::id::progress) {
        if (mThumb != nullptr) {
            setThumbPos(getWidth(),mThumb, scale, INT_MIN);
            // Since we draw translated, the drawable's bounds that it signals
            // for invalidation won't be the actual bounds we want invalidated,
            // so just invalidate this whole view.
            invalidate(true);
        }
    }
}

void AbsSeekBar::jumpDrawablesToCurrentState() {
    ProgressBar::jumpDrawablesToCurrentState();
    if (mThumb) {
        mThumb->jumpToCurrentState();
    }
    if (mTickMark) {
        mTickMark->jumpToCurrentState();
    }
}

void AbsSeekBar::drawableStateChanged(){
    ProgressBar::drawableStateChanged();

    Drawable*progressDrawable = getProgressDrawable();
    if (progressDrawable != nullptr && mDisabledAlpha < 1.0f) {
        progressDrawable->setAlpha(isEnabled() ? NO_ALPHA : (int) (NO_ALPHA * mDisabledAlpha));
    }

    if (mThumb != nullptr && mThumb->isStateful()
            && mThumb->setState(getDrawableState())) {
        invalidateDrawable(*mThumb);
    }

    if (mTickMark != nullptr && mTickMark->isStateful()
            && mTickMark->setState(getDrawableState())) {
        invalidateDrawable(*mTickMark);
    }
}

void AbsSeekBar::drawableHotspotChanged(float x,float y){
    ProgressBar::drawableHotspotChanged(x,y);
    if(mThumb)mThumb->setHotspot(x,y);
}

bool AbsSeekBar::verifyDrawable(Drawable* who)const{
    return who == mThumb || who == mTickMark || ProgressBar::verifyDrawable(who);
}

void AbsSeekBar::onProgressRefresh(float scale, bool fromUser, int progress){
}

void AbsSeekBar::onMeasure(int widthMeasureSpec, int heightMeasureSpec){
    Drawable* d = getCurrentDrawable();
    int thumbHeight = mThumb == nullptr ? 0 : mThumb->getIntrinsicHeight();
    int dw = 0;
    int dh = 0;
    if (d != nullptr) {
        dw = std::max(mMinWidth, std::min(mMaxWidth, d->getIntrinsicWidth()));
        dh = std::max(mMinHeight, std::min(mMaxHeight, d->getIntrinsicHeight()));
        dh = std::max(thumbHeight, dh);
    }
    dw += mPaddingLeft + mPaddingRight;
    dh += mPaddingTop + mPaddingBottom;

    setMeasuredDimension(resolveSizeAndState(dw, widthMeasureSpec, 0),
        resolveSizeAndState(dh, heightMeasureSpec, 0));
}

void AbsSeekBar::applyTickMarkTint(){
    if (mTickMark && (mHasTickMarkTint || mHasTickMarkTintMode)) {
        mTickMark = mTickMark->mutate();

        if (mHasTickMarkTint) {
            mTickMark->setTintList(mTickMarkTintList);
        }

        if (mHasTickMarkTintMode) {
            mTickMark->setTintMode(mTickMarkTintMode);
        }

        // The drawable (or one of its children) may not have been
        // stateful before applying the tint, so let's try again.
        if (mTickMark->isStateful()) {
            mTickMark->setState(getDrawableState());
        }
    }
}

void AbsSeekBar::setKeyProgressIncrement(int increment){
    mKeyProgressIncrement=increment;
}

int AbsSeekBar::getKeyProgressIncrement()const{
    return mKeyProgressIncrement;
}

void AbsSeekBar::setThumb(Drawable*thumb){
     bool needUpdate = false;
    // This way, calling setThumb again with the same bitmap will result in
    // it recalcuating mThumbOffset (if for example it the bounds of the
    // drawable changed)
    if (mThumb != nullptr && thumb != mThumb) {
        mThumb->setCallback(nullptr);
        needUpdate = true;

    }

    if (thumb != nullptr) {
        thumb->setCallback(this);
        if (canResolveLayoutDirection()) thumb->setLayoutDirection(getLayoutDirection());

        // Assuming the thumb drawable is symmetric, set the thumb offset
        // such that the thumb will hang halfway off either edge of the
        // progress bar.
        mThumbOffset = thumb->getIntrinsicWidth() / 2;
        // If we're updating get the new states
        if (needUpdate && (thumb->getIntrinsicWidth() != mThumb->getIntrinsicWidth()
                || thumb->getIntrinsicHeight() != mThumb->getIntrinsicHeight())) {
            requestLayout();
        }
    }
    delete mThumb;
    mThumb = thumb;
    applyThumbTint();
    invalidate(true);

    if (needUpdate||thumb) {
        updateThumbAndTrackPos(getWidth(), getHeight());
        if (thumb && thumb->isStateful()) {
            // Note that if the states are different this won't work.
            // For now, let's consider that an app bug.
            const std::vector<int>&state = getDrawableState();
            thumb->setState(state);
        }
    }
}

void AbsSeekBar::applyThumbTint(){
    if (mThumb != nullptr && (mHasThumbTint || mHasThumbTintMode)) {
        mThumb = mThumb->mutate();//mutate not tested, this will caused crash 

        //if (mHasThumbTint)mThumb->setTintList(mThumbTintList);

        //if (mHasThumbTintMode) mThumb->setTintMode(mThumbTintMode);

        // The drawable (or one of its children) may not have been
        // stateful before applying the tint, so let's try again.
        if (mThumb->isStateful()) {
            const std::vector<int>&state=getDrawableState();
            mThumb->setState(state);
        }
    }
}

Drawable*AbsSeekBar::getThumb()const{
    return mThumb;
}

void AbsSeekBar::setTickMark(Drawable* tickMark){
    if (mTickMark != nullptr) mTickMark->setCallback(nullptr);

    mTickMark = tickMark;

    if (tickMark != nullptr) {
        tickMark->setCallback(this);
        tickMark->setLayoutDirection(getLayoutDirection());
        if (tickMark->isStateful()) {
            tickMark->setState(getDrawableState());
        }
        //applyTickMarkTint();
    }
    invalidate(true);
}

Drawable* AbsSeekBar::getTickMark()const{
    return mTickMark;
}

void AbsSeekBar::setThumbOffset(int thumbOffset){
    mThumbOffset = thumbOffset;
    invalidate(true);
}

int AbsSeekBar::getThumbOffset()const{
    return mThumbOffset;
}

void AbsSeekBar::updateThumbAndTrackPos(int w, int h) {
    const int mPaddingTop=0,mPaddingLeft=0,mPaddingRight=0,mPaddingBottom=0;
    const int paddedHeight = h - mPaddingTop - mPaddingBottom;
    Drawable* track = getCurrentDrawable();
    Drawable* thumb = mThumb;
    // The max height does not incorporate padding, whereas the height
    // parameter does.
    const int trackHeight = std::min(mMaxHeight, paddedHeight);
    const int thumbHeight = thumb == nullptr ? 0 : thumb->getIntrinsicHeight();

    // Apply offset to whichever item is taller.
    int trackOffset;
    int thumbOffset;
    if (thumbHeight > trackHeight) {
        const int offsetHeight = (paddedHeight - thumbHeight) / 2;
        trackOffset = offsetHeight + (thumbHeight - trackHeight) / 2;
        thumbOffset = offsetHeight;
    } else {
        const int offsetHeight = (paddedHeight - trackHeight) / 2;
        trackOffset = offsetHeight;
        thumbOffset = offsetHeight + (trackHeight - thumbHeight) / 2;
    }

    if (track) {
        const int trackWidth = w - mPaddingRight - mPaddingLeft;
        track->setBounds(0, trackOffset, trackWidth, trackOffset + trackHeight);
    }

    if (thumb) {
        setThumbPos(w, thumb, getScale(), thumbOffset);
    }
}

float AbsSeekBar::getScale()const{
    int min = getMin();
    int max = getMax();
    int range = max - min;
    return range > 0 ? (getProgress() - min) / (float) range : 0;
}

void AbsSeekBar::setThumbPos(int w, Drawable* thumb, float scale, int offset){

    int available = w -mPaddingLeft - mPaddingRight;
    const int thumbWidth = thumb->getIntrinsicWidth();
    const int thumbHeight= thumb->getIntrinsicHeight();
    available -= thumbWidth;

    // The extra space for the thumb to move on the track
    available += mThumbOffset * 2;

    const int thumbPos = (int) (scale * available + 0.5f);

    int top, bottom;
    if (offset == INT_MIN) {
        const Rect oldBounds = thumb->getBounds();
        top = oldBounds.top;
        bottom = oldBounds.bottom();
    } else {
        top = offset;
        bottom = offset + thumbHeight;
    }

    const int left = (isLayoutRtl() && mMirrorForRtl) ? available - thumbPos : thumbPos;
    const int right = left + thumbWidth;

    Drawable* background = getBackground();
    if (background != nullptr) {
        const int offsetX = mPaddingLeft - mThumbOffset;
        const int offsetY = mPaddingTop;
        background->setHotspotBounds(left + offsetX, top + offsetY,thumbWidth,thumbHeight);
    }
    LOGV("thumb.size=%dx%d (%d,%d-%d,%d)",thumbWidth,thumbHeight,left,right,top,bottom);
    // Canvas will be translated, so 0,0 is where we start drawing
    thumb->setBounds(left, top,thumbWidth,thumbHeight);
}


void AbsSeekBar::drawThumb(Canvas&canvas) {
    if (mThumb != nullptr) {
        Rect r=mThumb->getBounds();
        canvas.save();
        // Translate the padding. For the x, we need to allow the thumb to
        // draw in its extra space
        canvas.translate( mPaddingLeft - mThumbOffset, mPaddingTop);
        mThumb->draw(canvas);
        canvas.restore();
    }
}

void  AbsSeekBar::drawTickMarks(Canvas&canvas){
    int count = getMax() - getMin();
    if (count > 1&& mTickMark) {
        int w = mTickMark->getIntrinsicWidth();
        int h = mTickMark->getIntrinsicHeight();
        int halfW = w >= 0 ? w / 2 : 1;
        int halfH = h >= 0 ? h / 2 : 1;
        mTickMark->setBounds(-halfW, -halfH, halfW, halfH);

        float spacing = getWidth()/ (float) count;
        canvas.save();
        canvas.translate(0, getHeight() / 2);
        for (int i = 0; i <= count; i++) {
            mTickMark->draw(canvas);
            canvas.translate(spacing, 0);
        }
        canvas.restore();
    }
}

void AbsSeekBar::drawTrack(Canvas&canvas){
    Drawable* thumbDrawable = mThumb;
    const int mPaddingTop=0,mPaddingLeft=0;
    if (thumbDrawable != nullptr && mSplitTrack) {
        const Insets insets = thumbDrawable->getOpticalInsets();
        Rect tempRect = thumbDrawable->getBounds();
        tempRect.offset(mPaddingLeft - mThumbOffset, mPaddingTop);
        tempRect.left += insets.left;
        tempRect.width-= (insets.left+insets.right);

        canvas.save();
        //canvas.clipRect(tempRect, Op.DIFFERENCE);
        ProgressBar::drawTrack(canvas);
        drawTickMarks(canvas);
        canvas.restore();
    } else {
        ProgressBar::drawTrack(canvas);
        drawTickMarks(canvas);
    }
}

void AbsSeekBar::onStartTrackingTouch(){
     mIsDragging = true;
}

void AbsSeekBar::attemptClaimDrag() {
    if (mParent) {
        mParent->requestDisallowInterceptTouchEvent(true);
    }
}

void AbsSeekBar::onStopTrackingTouch(){
     mIsDragging = false;
}

void AbsSeekBar::onKeyChange(){
}

void AbsSeekBar::onDraw(Canvas&canvas){
    ProgressBar::onDraw(canvas);
    drawThumb(canvas);
}

bool AbsSeekBar::onKeyDown(int keyCode,KeyEvent&event){
    switch(keyCode){
    case KEY_LEFT :
    case KEY_MINUS:
         setProgressInternal(getProgress()-mKeyProgressIncrement,true);
         return true;
    case KEY_RIGHT:
    case KEY_PLUS :
         setProgressInternal(getProgress()+mKeyProgressIncrement,true);
         return true;
    default:return ProgressBar::onKeyDown(keyCode,event);
    }
}

void AbsSeekBar::startDrag(MotionEvent& event){
    setPressed(true);
    if (mThumb != nullptr) {
        // This may be within the padding region.
        invalidate(&mThumb->getBounds());
    }
    onStartTrackingTouch();
    trackTouchEvent(event);
    attemptClaimDrag();
}

void AbsSeekBar::setHotspot(float x,float y){
    Drawable*bg=getBackground();
    if(bg)bg->setHotspot(x,y);
}

void AbsSeekBar::trackTouchEvent(MotionEvent&event){
    int x = event.getX();
    int y = event.getY();
    int width = getWidth();
    int availableWidth = width - mPaddingLeft - mPaddingRight;

    float scale;
    float progress = 0.0f;
    if (isLayoutRtl() && mMirrorForRtl) {
        if (x > width - mPaddingRight) {
            scale = 0.0f;
        } else if (x < mPaddingLeft) {
            scale = 1.0f;
        } else {
            scale = (availableWidth - x + mPaddingLeft) / (float) availableWidth
		    +mTouchThumbOffset;
            progress = mTouchProgressOffset;
        }
    } else {
        if (x < mPaddingLeft) {
            scale = 0.0f;
        } else if (x > width - mPaddingRight) {
            scale = 1.0f;
        } else {
            scale = (x - mPaddingLeft) / (float) availableWidth + mTouchThumbOffset;
            progress = mTouchProgressOffset;
        }
    }

    int range = getMax() - getMin();
    progress += scale * range + getMin();

    setHotspot(x, y);
    setProgressInternal((int)std::round(progress), true, false);
}

bool AbsSeekBar::onTouchEvent(MotionEvent& event){
    if (!mIsUserSeekable || !isEnabled()) return false;

    switch (event.getAction()) {
    case MotionEvent::ACTION_DOWN:
	if(mThumb){
             const int availableWidth = getWidth() - mPaddingLeft - mPaddingRight;
             mTouchThumbOffset = (getProgress() - getMin()) / (float) (getMax()
                    - getMin()) - (event.getX() - mPaddingLeft) / availableWidth;
             if (std::abs(mTouchThumbOffset * availableWidth) > getThumbOffset()) {
                 mTouchThumbOffset = 0;
             }
	}
        if (isInScrollingContainer()) {
            mTouchDownX = event.getX();
        } else {
            startDrag(event);
        }
        break;

    case MotionEvent::ACTION_MOVE:
        if (mIsDragging) {
            trackTouchEvent(event);
        } else {
            const float x = event.getX();
            if (std::abs(x - mTouchDownX) > mScaledTouchSlop) {
                startDrag(event);
            }
        }
        break;

    case MotionEvent::ACTION_UP:
        if (mIsDragging) {
            trackTouchEvent(event);
            onStopTrackingTouch();
            setPressed(false);
        } else {
            // Touch up when we never crossed the touch slop threshold should
            // be interpreted as a tap-seek to that location.
            onStartTrackingTouch();
            trackTouchEvent(event);
            onStopTrackingTouch();
        }
        // ProgressBar doesn't know to repaint the thumb drawable
        // in its inactive state when the touch stops (because the
        // value has not apparently changed)
        invalidate(true);
        break;

    case MotionEvent::ACTION_CANCEL:
        if (mIsDragging) {
            onStopTrackingTouch();
            setPressed(false);
        }
        invalidate(true); // see above explanation
        break;
    }
    return true;
}

bool AbsSeekBar::canUserSetProgress()const{
    return !isIndeterminate() && isEnabled();
}

void AbsSeekBar::onRtlPropertiesChanged(int layoutDirection) {
    ProgressBar::onRtlPropertiesChanged(layoutDirection);

    if (mThumb) {
        setThumbPos(getWidth(), mThumb, getScale(), INT_MIN);//Integer.MIN_VALUE);
        // Since we draw translated, the drawable's bounds that it signals
        // for invalidation won't be the actual bounds we want invalidated,
        // so just invalidate this whole view.
        invalidate();
    }
}
}//namespace
