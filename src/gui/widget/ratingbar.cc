#include <widget/ratingbar.h>

namespace cdroid{

RatingBar::RatingBar(int w,int h):AbsSeekBar(w,h){
}

RatingBar::RatingBar(Context*ctx,const AttributeSet&atts):AbsSeekBar(ctx,atts){
}

void RatingBar::setOnRatingBarChangeListener(OnRatingBarChangeListener listener){
    mOnRatingBarChangeListener=listener;
}

RatingBar::OnRatingBarChangeListener RatingBar::getOnRatingBarChangeListener(){
    return mOnRatingBarChangeListener;
}

void RatingBar::setIsIndicator(bool isIndicator){
    mIsUserSeekable = !isIndicator;
    if (isIndicator) {
        setFocusable(FOCUSABLE_AUTO);
    } else {
        setFocusable(FOCUSABLE);
    }
}

bool RatingBar::isIndicator()const{
    return !mIsUserSeekable;
}

void RatingBar::setNumStars(int numStars){
    if (numStars <= 0||numStars==mNumStars) 
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
        return 1.f * getMax() / mNumStars;
    } else {
        return 1.;
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

}
