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
#ifndef __RATINGBAR_H__
#define __RATINGBAR_H__
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
    void setOnRatingBarChangeListener(const OnRatingBarChangeListener& listener);
    OnRatingBarChangeListener getOnRatingBarChangeListener();
    void setIsIndicator(bool isIndicator);
    bool isIndicator()const;
    void setNumStars(int numStars);
    int getNumStars()const;
    void setRating(float rating);
    float getRating()const;
    void setStepSize(float stepSize);
    float getStepSize()const;
    void setMax(int max)override;

    std::string getAccessibilityClassName()const override;
    void onInitializeAccessibilityNodeInfoInternal(AccessibilityNodeInfo& info)override;
};

}/*endof namesapce*/
#endif
