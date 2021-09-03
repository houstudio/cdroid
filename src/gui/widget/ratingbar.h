#pragma once
#include <widget/absseekbar.h>
namespace cdroid{

class RatingBar:public AbsSeekBar{
public:
    DECLARE_UIEVENT(void,OnRatingBarChangeListener,RatingBar& ratingBar, float rating, bool fromUser);
private:
    int mNumStars = 5;
    int mProgressOnStartTracking;
    OnRatingBarChangeListener mOnRatingBarChangeListener;
    float getProgressPerStar()const;
    void updateSecondaryProgress(int progress);
protected:
    void onProgressRefresh(float scale, bool fromUser, int progress)override;
    void onMeasure(int widthMeasureSpec, int heightMeasureSpec)override;
    void onStartTrackingTouch()override;
    void onStopTrackingTouch()override;
    void onKeyChange()override;
    void dispatchRatingChange(bool fromUser);
    bool canUserSetProgress()const override;
public:
    RatingBar(int w,int h);
    RatingBar(Context*ctx,const AttributeSet&atts);
    void setOnRatingBarChangeListener(OnRatingBarChangeListener listener);
    OnRatingBarChangeListener getOnRatingBarChangeListener();
    void setIsIndicator(bool isIndicator);
    bool isIndicator()const;
    void setNumStars(int numStars);
    int getNumStars()const;
    void setRating(float rating);
    float getRating()const;
    void setStepSize(float stepSize);
    float getStepSize()const;
    void setMax(int max);
};

}
