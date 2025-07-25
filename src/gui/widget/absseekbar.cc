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
    mThumbTintList = attrs.getColorStateList("thumbTint");
    mTickMarkTintList = attrs.getColorStateList("tickMarkTint");
    const int thumbOffset = attrs.getDimensionPixelOffset("thumbOffset",getThumbOffset());
    setThumbOffset(thumbOffset);

    const bool useDisabledAlpha = attrs.getBoolean("useDisabledAlpha", true);
    mDisabledAlpha = useDisabledAlpha?attrs.getFloat("disabledAlpha", 0.5f):1.f;
    mSplitTrack = attrs.getBoolean("splitTrack",false);
    mThumbExclusionMaxSize = ctx->getDimension("cdroid:dimen/seekbar_thumb_exclusion_max_size");

    applyThumbTint();
    applyTickMarkTint();
}

AbsSeekBar::AbsSeekBar(int w,int h):ProgressBar(w,h){
    initSeekBar();
    applyThumbTint();
    applyTickMarkTint();
}

void AbsSeekBar::initSeekBar(){
    mKeyProgressIncrement = 5;
    mSplitTrack = false;
    mThumb = nullptr;
    mThumbOffset= 0;
    mTickMark = nullptr;
    mIsUserSeekable=true;
    mIsDragging = false;
    mTouchDownX = 0.f;
    mHasThumbTint = false;
    mHasThumbTintMode = false;
    mTickMarkTintList = nullptr;
    mThumbTintList = nullptr;
    mDisabledAlpha = 1.f;
    mTouchProgressOffset =0.f;
    mTouchThumbOffset = 0.f;
    mScaledTouchSlop  = ViewConfiguration::get(mContext).getScaledTouchSlop();
    setFocusable(true);
}

AbsSeekBar::~AbsSeekBar(){
    delete mThumb;
    delete mTickMark;
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
            setThumbPos((getProgressOrientation()==HORIZONTAL)?getWidth():getHeight(),mThumb, scale, INT_MIN);
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

void AbsSeekBar::onMeasure(int widthMeasureSpec, int heightMeasureSpec){
    Drawable* d = getCurrentDrawable();
    const int thumbHeight = mThumb == nullptr ? 0 : mThumb->getIntrinsicHeight();
    const int thumbWidth  = mThumb == nullptr ? 0 : mThumb->getIntrinsicWidth();
    int dw = 0;
    int dh = 0;
    if (d != nullptr) {
	    if(getProgressOrientation()==HORIZONTAL){
            dw = std::max(mMinWidth, std::min(mMaxWidth, d->getIntrinsicWidth()));
            dh = std::max(mMinHeight, std::min(mMaxHeight, d->getIntrinsicHeight()));
            dh = std::max(thumbHeight, dh);
	    }else{
            dw = std::max(mMinWidth, std::min(mMaxWidth, d->getIntrinsicWidth()));
            dh = std::max(mMinHeight, std::min(mMaxHeight, d->getIntrinsicHeight()));
            dw = std::max(thumbWidth, dw);
        }
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
    if ((mThumb != nullptr) && (thumb != mThumb)) {
        mThumb->setCallback(nullptr);
        needUpdate = true;

    }

    if (thumb != nullptr) {
        thumb->setCallback(this);
        if (canResolveLayoutDirection()) thumb->setLayoutDirection(getLayoutDirection());

        // Assuming the thumb drawable is symmetric, set the thumb offset
        // such that the thumb will hang halfway off either edge of the
        // progress bar.
        if(getProgressOrientation()==HORIZONTAL)
            mThumbOffset = thumb->getIntrinsicWidth() / 2;
        else
            mThumbOffset = thumb->getIntrinsicHeight() / 2;
        // If we're updating get the new states
        if (needUpdate && (thumb->getIntrinsicWidth() != mThumb->getIntrinsicWidth()
                || thumb->getIntrinsicHeight() != mThumb->getIntrinsicHeight())) {
            requestLayout();
        }
    }
    delete mThumb;
    mThumb = thumb;
    applyThumbTint();
    invalidate();

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
    if ((mThumb != nullptr) && (mHasThumbTint || mHasThumbTintMode)) {
        mThumb = mThumb->mutate();//mutate not tested, this will caused crash 

        if (mHasThumbTint)mThumb->setTintList(mThumbTintList);

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
        applyTickMarkTint();
    }
    invalidate();
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
    const int paddedHeight = h - mPaddingTop - mPaddingBottom;
    const int paddedWidth  = w - mPaddingLeft- mPaddingRight;
    Drawable* track = getCurrentDrawable();
    Drawable* thumb = mThumb;
    // The max height does not incorporate padding, whereas the height
    // parameter does.
    int trackHeight = std::min(mMaxHeight, paddedHeight);
    int trackWidth  = std::min(mMaxWidth, paddedWidth);
    const int thumbHeight = thumb == nullptr ? 0 : thumb->getIntrinsicHeight();
    const int thumbWidth  = thumb == nullptr ? 0 : thumb->getIntrinsicWidth();

    // Apply offset to whichever item is taller.
    int trackOffset;
    int thumbOffset;
    if(getProgressOrientation()==HORIZONTAL){
        if (thumbHeight > trackHeight) {
            const int offsetHeight = (paddedHeight - thumbHeight) / 2;
            trackOffset = offsetHeight + (thumbHeight - trackHeight) / 2;
            thumbOffset = offsetHeight;
        } else {
            const int offsetHeight = (paddedHeight - trackHeight) / 2;
            trackOffset = offsetHeight;
            thumbOffset = offsetHeight + (trackHeight - thumbHeight) / 2;
        }
        if(thumb)
	    setThumbPos(w,thumb,getScale(), thumbOffset);
    }else{
        if (thumbWidth > trackWidth) {
            const int offsetWidth = (paddedWidth - thumbWidth) / 2;
            trackOffset = offsetWidth + (thumbWidth - trackWidth) / 2;
            thumbOffset = offsetWidth;
        } else {
            const int offsetWidth = (paddedWidth - trackWidth) / 2;
            trackOffset = offsetWidth;
            thumbOffset = offsetWidth + (trackWidth - thumbWidth) / 2;
        }
        if(thumb)
            setThumbPos(h,thumb, getScale(), thumbOffset);
    }

    if (track) {
        if(getProgressOrientation()==HORIZONTAL){
            trackWidth = w - mPaddingRight - mPaddingLeft;
            track->setBounds(0, trackOffset, trackWidth, trackHeight);
            LOGV("%p:%d bounds=(%d,%d,%d,%d) %d",this,mID,0, trackOffset, trackWidth, trackHeight,mMaxHeight);
	    }else{
            trackHeight = h - mPaddingTop - mPaddingBottom;
            track->setBounds(trackOffset,0, trackWidth, trackHeight);
            LOGV("%p:%d bounds=(%d,%d,%d,%d) %d",this,mID,trackOffset,0,trackWidth, trackHeight,mMaxWidth);
	    }
    }
}

float AbsSeekBar::getScale()const{
    int min = getMin();
    int max = getMax();
    int range = max - min;
    return range > 0 ? (getProgress() - min) / (float) range : 0;
}

void AbsSeekBar::setThumbPos(int wh, Drawable* thumb, float scale, int offset){

    const int thumbWidth = thumb->getIntrinsicWidth();
    const int thumbHeight= thumb->getIntrinsicHeight();
    int available = (getProgressOrientation()==HORIZONTAL)?(wh - mPaddingLeft - mPaddingRight):(wh- mPaddingTop - mPaddingBottom);
    available -= (getProgressOrientation()==HORIZONTAL)?thumbWidth:thumbHeight;

    // The extra space for the thumb to move on the track
    available += mThumbOffset * 2;

    const int thumbPos = (int) (scale * available + 0.5f);

    int left,right,top,bottom;
    const int progressGravity = Gravity::getAbsoluteGravity(getProgressGravity(),getLayoutDirection());
    if(getProgressOrientation()==HORIZONTAL){
        const int absGravity = progressGravity & Gravity::HORIZONTAL_GRAVITY_MASK;
        if (offset == INT_MIN) {
            const Rect oldBounds = thumb->getBounds();
            top = oldBounds.top;
            bottom = oldBounds.bottom();
        } else {
            top = offset;
            bottom = offset + thumbHeight;
        }

        left = ((isLayoutRtl() && mMirrorForRtl)||(absGravity==Gravity::RIGHT)) ? available - thumbPos : thumbPos;
        right = left + thumbWidth;
        Drawable* background = getBackground();
        if (background != nullptr) {
            const int offsetX = mPaddingLeft - mThumbOffset;
            const int offsetY = mPaddingTop;
            //background->setHotspotBounds(left + offsetX, top + offsetY,thumbWidth,thumbHeight);
            background->setHotspotBounds(left + offsetX, top + offsetY, right-left,bottom-top);
        }
        // Canvas will be translated, so 0,0 is where we start drawing
        thumb->setBounds(left, top,right-left,bottom-top);//thumbWidth,thumbHeight);
    }else{
        const int absGravity = progressGravity & Gravity::VERTICAL_GRAVITY_MASK;
        if(offset == INT_MIN){
            const Rect oldBounds = thumb->getBounds();
            left = oldBounds.left;
            right= oldBounds.right();
        }else{
            left = offset;
            right= offset + thumbWidth;
        }
        top =  (absGravity==Gravity::BOTTOM)?(available - thumbPos):thumbPos;
        bottom= top+ thumbHeight;
        Drawable* background = getBackground();
        if (background != nullptr) {
            const int offsetX = mPaddingLeft - mThumbOffset;
            const int offsetY = mPaddingTop + mThumbOffset;
            background->setHotspotBounds(left , top + offsetY,right-left,bottom-top);//thumbWidth,thumbHeight);
        }
        thumb->setBounds(left, top,right-left,bottom-top);//thumbWidth,thumbHeight);
    }
    updateGestureExclusionRects();
}

void AbsSeekBar::setSystemGestureExclusionRects(const std::vector<Rect>&rects){
    mUserGestureExclusionRects = rects;
    updateGestureExclusionRects();
}

void AbsSeekBar::updateGestureExclusionRects(){
    if (mThumb == nullptr) {
        ProgressBar::setSystemGestureExclusionRects(mUserGestureExclusionRects);
        return;
    }
    mGestureExclusionRects.clear();
    mThumb->copyBounds(mThumbRect);
    mThumbRect.offset(mPaddingLeft - mThumbOffset, mPaddingTop);
    growRectTo(mThumbRect, std::min(getHeight(), mThumbExclusionMaxSize));
    mGestureExclusionRects.push_back(mThumbRect);
    mGestureExclusionRects.insert(mGestureExclusionRects.end(),
            mUserGestureExclusionRects.begin(),
            mUserGestureExclusionRects.end());
    ProgressBar::setSystemGestureExclusionRects(mGestureExclusionRects);
}

void AbsSeekBar::growRectTo(Rect& r, int minimumSize){
    const int dy = minimumSize - r.height;
    if (dy > 0) {
        r.top -= (dy + 1) / 2;
        r.width += dy;
    }
    const int dx = minimumSize - r.width;
    if (dx > 0) {
        r.left -= (dx + 1) / 2;
        r.width += dx;
    }
}

void AbsSeekBar::onResolveDrawables(int layoutDirection){
    ProgressBar::onResolveDrawables(layoutDirection);
    if (mThumb != nullptr) {
        mThumb->setLayoutDirection(layoutDirection);
    }
}

void AbsSeekBar::drawThumb(Canvas&canvas) {
    if (mThumb != nullptr) {
        Rect r = mThumb->getBounds();
        canvas.save();
        // Translate the padding. For the x, we need to allow the thumb to
        // draw in its extra space
        if(getProgressOrientation()==HORIZONTAL)
            canvas.translate( mPaddingLeft - mThumbOffset, mPaddingTop);
        else
            canvas.translate( mPaddingLeft, mPaddingTop - mThumbOffset);
        mThumb->draw(canvas);
        canvas.restore();
    }
}

void  AbsSeekBar::drawTickMarks(Canvas&canvas){
    const int count = getMax() - getMin();
    if ((count > 1) && mTickMark) {
        const int w = mTickMark->getIntrinsicWidth();
        const int h = mTickMark->getIntrinsicHeight();
        const int halfW = w >= 0 ? w / 2 : 1;
        const int halfH = h >= 0 ? h / 2 : 1;
        mTickMark->setBounds(-halfW, -halfH, halfW*2, halfH*2);

        float spacing = 0;
        if(getProgressOrientation()==HORIZONTAL)
            spacing = float(getWidth()-mPaddingLeft-mPaddingRight)/count;
        else
            spacing = float(getHeight()-mPaddingTop-mPaddingBottom)/count;
        canvas.save();
        if(getProgressOrientation()==HORIZONTAL)
            canvas.translate(mPaddingLeft, getHeight() / 2);
        else{
            canvas.translate(getWidth()/2,mPaddingTop);
        }
        for (int i = 0; i <= count; i++) {
            mTickMark->draw(canvas);
            if(getProgressOrientation()==HORIZONTAL)
                canvas.translate(spacing, 0);
            else
                canvas.translate(0, spacing);
        }
        canvas.restore();
    }
}

void AbsSeekBar::drawTrack(Canvas&canvas){
    Drawable* thumbDrawable = mThumb;
    if ((thumbDrawable != nullptr) && mSplitTrack) {
        const Insets insets = thumbDrawable->getOpticalInsets();
        Rect tempRect = thumbDrawable->getBounds();
        tempRect.offset(mPaddingLeft - mThumbOffset, mPaddingTop);
        tempRect.left += insets.left;
        tempRect.width-= (insets.left+insets.right);

        canvas.save();
#if 0
        //canvas.clipRect(tempRect, Op.DIFFERENCE);
        Cairo::Path*current_clip=canvas.copy_path();
        canvas.begin_new_path();
        canvas.rectangle(tempRect.left,tempRect.top,tempRect.width,tempRect.height);
        //canvas.set_fill_rule(Cairo::Context::FillRule::EVEN_ODD);
        canvas.clip();
        canvas.append_path(*current_clip);
        canvas.clip();
        delete current_clip;
#endif
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
    if(isEnabled()){
        int increment = mKeyProgressIncrement;
        switch(keyCode){
        case KeyEvent::KEYCODE_DPAD_LEFT :
        case KeyEvent::KEYCODE_MINUS:
            increment = -increment;
            //fallthrought
        case KeyEvent::KEYCODE_DPAD_RIGHT:
        case KeyEvent::KEYCODE_PLUS :
        case KeyEvent::KEYCODE_EQUALS:
            increment = isLayoutRtl() ? -increment : increment;
            if(setProgressInternal(getProgress() + increment,true,true)){
                onKeyChange();
                return true;
            }
        default:break;
        }
    }
    return ProgressBar::onKeyDown(keyCode,event);
}

void AbsSeekBar::startDrag(MotionEvent& event){
    setPressed(true);
    if (mThumb != nullptr) {
        // This may be within the padding region.
        invalidate((const Rect*)&mThumb->getBounds());
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
    const int x = event.getX();
    const int y = event.getY();
    const int width = getWidth();
    const int height= getHeight();
    const int availableWidth = width - mPaddingLeft - mPaddingRight;
    const int availableHeight= height- mPaddingTop - mPaddingBottom;

    float scale;
    float progress = 0.0f;
    const int progressGravity = Gravity::getAbsoluteGravity(getProgressGravity(),getLayoutDirection());
    if(getProgressOrientation()==HORIZONTAL){
        const int absGravity = progressGravity & Gravity::HORIZONTAL_GRAVITY_MASK;
        if ((isLayoutRtl() && mMirrorForRtl)||(absGravity==Gravity::RIGHT)) {
            if (x > width - mPaddingRight) {
                scale = 0.0f;
            } else if (x < mPaddingLeft) {
                scale = 1.0f;
            } else {
                scale = (availableWidth - x + mPaddingLeft) / (float) availableWidth + mTouchThumbOffset;
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
    }else{
        const int absGravity = progressGravity & Gravity::VERTICAL_GRAVITY_MASK;
        if ((/*isLayoutRtl() &&*/ mMirrorForRtl)&&(absGravity==Gravity::TOP)) {
            if (y > height - mPaddingBottom) {
                scale = 1.0f;
            } else if (y < mPaddingTop) {
                scale = 0.f;
            } else {
                scale = float(y - mPaddingTop) / availableHeight  + mTouchThumbOffset;
                progress = mTouchProgressOffset;
            }
        } else {
            if (y < mPaddingTop) {
                scale = 1.0f;
            } else if (y > height - mPaddingBottom) {
                scale = .0f;
            } else {
                scale = float(availableHeight - y - mPaddingBottom) / availableHeight + mTouchThumbOffset;
                progress = mTouchProgressOffset;
            }
        }
    }

    progress += scale * (getMax() - getMin()) + getMin();

    setHotspot(x, y);
    setProgressInternal((int)std::round(progress), true, false);
}

bool AbsSeekBar::onTouchEvent(MotionEvent& event){
    if (!mIsUserSeekable || !isEnabled()) return false;

    switch (event.getAction()) {
    case MotionEvent::ACTION_DOWN:
        if(mThumb!=nullptr){
            const int availableWidth = getWidth() - mPaddingLeft - mPaddingRight;
            const int availableHeight = getHeight() - mPaddingTop - mPaddingBottom;
	        if(getProgressOrientation()==HORIZONTAL){
                 mTouchThumbOffset = (getProgress() - getMin()) / (float) (getMax()
                    - getMin()) - (event.getX() - mPaddingLeft) / availableWidth;
                 if (std::abs(mTouchThumbOffset * availableWidth) > getThumbOffset()) {
                     mTouchThumbOffset = 0;
                 }
	        }else{
                 mTouchThumbOffset = (getProgress() - getMin()) / (float) (getMax()
                    - getMin()) - (event.getY() - mPaddingTop) / availableHeight;
                 if (std::abs(mTouchThumbOffset * availableHeight) > getThumbOffset()) {
                     mTouchThumbOffset = 0;
                 }
	        }
	    }
        if (isInScrollingContainer()) {
            mTouchDownX = event.getX();
            mTouchDownY = event.getY();
        } else {
            startDrag(event);
        }
        break;

    case MotionEvent::ACTION_MOVE:
        if (mIsDragging) {
            trackTouchEvent(event);
        } else {
            const float x = event.getX();
            const float y = event.getY();
            const int orientation = getProgressOrientation();
            if (((orientation==HORIZONTAL)&&(std::abs(x - mTouchDownX) > mScaledTouchSlop))||
	        ((orientation==VERTICAL)&&(std::abs(y - mTouchDownY) > mScaledTouchSlop))) {
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

std::string AbsSeekBar::getAccessibilityClassName()const{
    return "AbsSeekBar";
}

void AbsSeekBar::onInitializeAccessibilityNodeInfoInternal(AccessibilityNodeInfo& info){
    ProgressBar::onInitializeAccessibilityNodeInfoInternal(info);

    if (isEnabled()) {
        const int progress = getProgress();
        if (progress > getMin()) {
            info.addAction(AccessibilityNodeInfo::ACTION_SCROLL_BACKWARD);
        }
        if (progress < getMax()) {
            info.addAction(AccessibilityNodeInfo::ACTION_SCROLL_FORWARD);
        }
    }
}

bool AbsSeekBar::performAccessibilityActionInternal(int action, Bundle* arguments){
     if (ProgressBar::performAccessibilityActionInternal(action, arguments)) {
         return true;
     }

     if (!isEnabled()) {
         return false;
     }

     switch (action) {
     case R::id::accessibilityActionSetProgress: {
         if (!canUserSetProgress()) {
             return false;
         }
         if (arguments == nullptr /*|| !arguments.containsKey(AccessibilityNodeInfo::ACTION_ARGUMENT_PROGRESS_VALUE)*/) {
             return false;
         }
         //const float value = arguments.getFloat( AccessibilityNodeInfo.ACTION_ARGUMENT_PROGRESS_VALUE);
         return false;//setProgressInternal((int) value, true, true);
     }
     case AccessibilityNodeInfo::ACTION_SCROLL_FORWARD:
     case AccessibilityNodeInfo::ACTION_SCROLL_BACKWARD: {
         if (!canUserSetProgress()) {
             return false;
         }
         const int range = getMax() - getMin();
         int increment = std::max(1.f, std::round((float) range / 20));
         if (action == AccessibilityNodeInfo::ACTION_SCROLL_BACKWARD) {
             increment = -increment;
         }

         // Let progress bar handle clamping values.
         if (setProgressInternal(getProgress() + increment, true, true)) {
             onKeyChange();
             return true;
         }
         return false;
     }
     }
     return false;
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
