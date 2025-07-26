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

#ifndef __CANDIDATE_VIEW_H__
#define __CANDIDATE_VIEW_H__
#include <view/view.h>

namespace cdroid{

class CandidateView:public View{
public:
    DECLARE_UIEVENT(void,OnPredictChange,CandidateView&,const std::string&,int candidateId);
private:
    static constexpr int OUT_OF_BOUNDS = -1;
    static constexpr int MAX_SUGGESTION=32;
    static constexpr int SCROLL_PIXELS = 20;
    static constexpr int X_GAP = 10;
private:
    int mSelectedIndex;
    int mTouchX;
    int mColorNormal;
    int mColorRecommended;
    int mColorOther;
    int mTextSize;
    int mVerticalPadding;
    int mTargetScrollX;
    int mTotalWidth;
    bool mScrolled;
    bool mTypedWordValid;
    std::vector<std::string>mSuggestions;
    Drawable*mSelectionHighlight;
    OnPredictChange mOnPredict;
    Rect mBgPadding;
    std::vector<int>mWordWidth;
    std::vector<int>mWordX;
private:
    void scrollToTarget();
    void removeHighlight();
protected:
    void onMeasure(int widthMeasureSpec, int heightMeasureSpec)override;
    void onDrawInternal(Canvas* canvas);
    void onDraw(Canvas& canvas)override;
public:
    CandidateView(int,int);
    CandidateView(Context*ctx,const AttributeSet&atts);
    int computeHorizontalScrollRange()override;
    void setMaxSuggestion(int max);
    void setSuggestions(const std::vector<std::string>&suggestions, 
		    bool completions,bool typedWordValid);
    void clear();
    void setPredictListener(OnPredictChange ls);
    bool onTouchEvent(MotionEvent& me)override;
    void takeSuggestionAt(float x);
};
}/*endof namespace*/
#endif
