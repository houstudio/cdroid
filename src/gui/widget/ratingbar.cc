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
#include <widget/ratingbar.h>

namespace cdroid{

DECLARE_WIDGET2(RatingBar,"cdroid:attr/ratingBarStyle")

RatingBar::RatingBar(int w,int h):AbsSeekBar(w,h){
    mNumStars = 5;
    mIsUserSeekable = true;
    mProgressOnStartTracking=0;
    mTouchProgressOffset = 0.6f;
    setStepSize(0.5f);
}

RatingBar::RatingBar(Context*ctx,const AttributeSet&atts)
    :AbsSeekBar(ctx,atts){
    mNumStars = 5;
    mIsUserSeekable = true;
    mProgressOnStartTracking =0;

    setIsIndicator(atts.getBoolean("isIndicator",!mIsUserSeekable));
    const int numStars  = atts.getInt("numStars",mNumStars);
    const float rating  = atts.getFloat("rating",-1);
    const float stepSize= atts.getFloat("stepSize",-1);
    if( (numStars>0) && (numStars!=mNumStars) )
        setNumStars(numStars);
    setStepSize((stepSize>=0)?stepSize:0.5f);
    if(rating>=0)setRating(rating);

    // A touch inside a star fill up to that fractional area (slightly more
    // than 0.5 so boundaries round up).
    mTouchProgressOffset = 0.6f;
}

void RatingBar::setOnRatingBarChangeListener(const OnRatingBarChangeListener& listener){
    mOnRatingBarChangeListener=listener;
}

RatingBar::OnRatingBarChangeListener RatingBar::getOnRatingBarChangeListener(){
    return mOnRatingBarChangeListener;
}

void RatingBar::setIsIndicator(bool isIndicator){
    mIsUserSeekable = !isIndicator;
    if (isIndicator) {
        setFocusable((int)FOCUSABLE_AUTO);
    } else {
        setFocusable((int)FOCUSABLE);
    }
}

bool RatingBar::isIndicator()const{
    return !mIsUserSeekable;
}

void RatingBar::setNumStars(int numStars){
    if ((numStars <= 0)||(numStars==mNumStars))
        return;

    mNumStars = numStars;
    // This causes the width to change, so re-layout
    requestLayout();
}

int RatingBar::getNumStars()const{
    return mNumStars;
}

void RatingBar::setRating(float rating){
    setProgress(std::round(rating * getProgressPerStar()));
}

float RatingBar::getRating()const{
     return getProgress() / getProgressPerStar();
}

void RatingBar::setStepSize(float stepSize){
    if (stepSize <= 0) return;

    float newMax = mNumStars / stepSize;
    int newProgress = (int) (newMax / getMax() * getProgress());
    setMax((int) newMax);
    setProgress(newProgress);
}

float RatingBar::getStepSize()const{
    return (float) getNumStars() / getMax();
}

float RatingBar::getProgressPerStar()const{
    if (mNumStars > 0) {
        return float(getMax()) / mNumStars;
    } else {
        return 1.f;
    }
}

void RatingBar::onProgressRefresh(float scale, bool fromUser, int progress){
    AbsSeekBar::onProgressRefresh(scale, fromUser, progress);

    // Keep secondary progress in sync with primary
    updateSecondaryProgress(progress);

    if (!fromUser) {
        // Callback for non-user rating changes
        dispatchRatingChange(false);
    }
}

void RatingBar::updateSecondaryProgress(int progress){
    float ratio = getProgressPerStar();
    if (ratio > 0) {
        float progressInStars = progress / ratio;
        int secondaryProgress = (int) (std::ceil(progressInStars) * ratio);
        setSecondaryProgress(secondaryProgress);
    }
}

void RatingBar::onMeasure(int widthMeasureSpec, int heightMeasureSpec){
    AbsSeekBar::onMeasure(widthMeasureSpec, heightMeasureSpec);

    if (mSampleWidth > 0) {
        const int width = mSampleWidth * mNumStars;
        setMeasuredDimension(resolveSizeAndState(width, widthMeasureSpec, 0), getMeasuredHeight());
    }
}

void RatingBar::onStartTrackingTouch() {
    mProgressOnStartTracking = getProgress();

    AbsSeekBar::onStartTrackingTouch();
}

void RatingBar::onStopTrackingTouch() {
    AbsSeekBar::onStopTrackingTouch();

    if (getProgress() != mProgressOnStartTracking) {
        dispatchRatingChange(true);
    }
}

void RatingBar::onKeyChange() {
    AbsSeekBar::onKeyChange();
    dispatchRatingChange(true);
}

void RatingBar::dispatchRatingChange(bool fromUser){
    if (mOnRatingBarChangeListener) {
        mOnRatingBarChangeListener(*this, getRating(),fromUser);
    }
}

void RatingBar::setMax(int max) {
    // Disallow max progress = 0
    if (max <= 0) return;
    AbsSeekBar::setMax(max);
}

bool RatingBar::canUserSetProgress()const{
    return AbsSeekBar::canUserSetProgress() && !isIndicator();
}

std::string RatingBar::getAccessibilityClassName()const{
    return "RatingBar";
}

void RatingBar::onInitializeAccessibilityNodeInfoInternal(AccessibilityNodeInfo& info){
    AbsSeekBar::onInitializeAccessibilityNodeInfoInternal(info);

    if (canUserSetProgress()) {
        info.addAction(AccessibilityNodeInfo::AccessibilityAction::ACTION_SET_PROGRESS.getId());
    }
}
}
