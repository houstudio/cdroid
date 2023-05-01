#ifndef __CANDIDATE_VIEW_H__
#define __CANDIDATE_VIEW_H__
#include <view/view.h>

namespace cdroid{

class CandidateView:public View{
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
    CandidateView(Context*ctx,const AttributeSet&atts);
    int computeHorizontalScrollRange()override;
    void setMaxSuggestion(int max);
    void setSuggestions(const std::vector<std::string>&suggestions, 
		    bool completions,bool typedWordValid);
    void clear();
    bool onTouchEvent(MotionEvent& me)override;
    void takeSuggestionAt(float x);
};
}/*endof namespace*/
#endif
