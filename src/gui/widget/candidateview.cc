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

#include <widget/candidateview.h>
namespace cdroid{

DECLARE_WIDGET(CandidateView)

CandidateView::CandidateView(int w,int h):View(w,h){
    mSelectionHighlight = nullptr;
    mColorNormal= 0xFFFFFFFF;
    mColorRecommended =0xFFFF0000;
    mColorOther =0xFF00FF00;
    mVerticalPadding=0;
    mTextSize=20;
    mBgPadding.set(5,5,5,5);
    setMinimumHeight(mTextSize+8);
    mOnPredict = nullptr;
}

CandidateView::CandidateView(Context*ctx,const AttributeSet&atts):View(ctx,atts){
     mSelectionHighlight = atts.getDrawable("list_selector_background");
     setBackgroundColor(atts.getColor("candidate_background"));
     mColorNormal = atts.getColor("candidate_normal");
     mColorRecommended = atts.getColor("candidate_recommand");
     mColorOther = atts.getColor("candidate_other");
     mVerticalPadding = atts.getDimensionPixelSize("candidate_vertical_padding");
     mTextSize = atts.getDimensionPixelSize("candidate_text_size",20);
     setHorizontalFadingEdgeEnabled(true);
     setWillNotDraw(false);
     setHorizontalScrollBarEnabled(false);
     setVerticalScrollBarEnabled(false);
     setMaxSuggestion(MAX_SUGGESTION);
     mBgPadding.set(5,5,5,5);
     mOnPredict = nullptr;
     setMinimumHeight(mTextSize+8);
}

int CandidateView::computeHorizontalScrollRange(){
    return mTotalWidth;
}

void CandidateView::onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
    const int measuredWidth = resolveSize(50, widthMeasureSpec);
        
    // Get the desired height of the icon menu view (last row of items does
    // not have a divider below)
    Rect padding={0,0,0,0};
    if(mSelectionHighlight)
        mSelectionHighlight->getPadding(padding);
    int desiredHeight = mTextSize + mVerticalPadding + padding.top + padding.height;
    if(getBackground())desiredHeight =std::max(desiredHeight,getSuggestedMinimumHeight());
    // Maximum possible width and desired height
    LOGV("size=%dx%d",measuredWidth,resolveSize(desiredHeight, heightMeasureSpec));
    setMeasuredDimension(measuredWidth, resolveSize(desiredHeight, heightMeasureSpec));
}

void CandidateView::onDrawInternal(Canvas* canvas) {
    if (mSuggestions.empty()) return;
    LOGV("%d suggestion canvas=%p",mSuggestions.size(),canvas);

    if (getBackground() ) {
        getBackground()->getPadding(mBgPadding);
    }
    int  x = 0;
    int  count  = mSuggestions.size();
    int  height = getHeight();
    Rect bgPadding = mBgPadding;
    int  touchX  = mTouchX;
    int  scrollX = getScrollX();
    bool scrolled = mScrolled;
    bool typedWordValid = mTypedWordValid;
    Rect suggestionRect;
    Cairo::RefPtr<Cairo::ImageSurface>image = Cairo::ImageSurface::create(Cairo::Surface::Format::ARGB32,1,1);
    Cairo::RefPtr<Cairo::Context>mContext = Cairo::Context::create(image);
    mContext->set_font_size(mTextSize);
    mTotalWidth = 0;
    if(canvas)canvas->set_font_size(mTextSize);
    for (int i = 0; i < count; i++) {
	Cairo::TextExtents te;
	std::string suggestion = mSuggestions.at(i);
	mContext->get_text_extents(suggestion,te);
        int wordWidth = (int) te.x_advance + X_GAP * 2;

        mWordX[i] = x;
        mWordWidth[i] = wordWidth;
	if(x+wordWidth<scrollX)continue;
	if(x>=scrollX+getWidth())break;
	suggestionRect.set(x,bgPadding.top,wordWidth,height);
        if (touchX + scrollX >= x && touchX + scrollX < x + wordWidth && !scrolled) {
            if (canvas && mSelectionHighlight) {
                canvas->translate(x, 0);
                mSelectionHighlight->setBounds(0, bgPadding.top, wordWidth, height);
                mSelectionHighlight->draw(*canvas);
                canvas->translate(-x, 0);
            }
            mSelectedIndex = i;
        }
	if(canvas){
	    bool fakeBold = false;
	    canvas->set_color(mColorNormal);
            if ((i == 1 && !typedWordValid) || (i == 0 && typedWordValid)) {
		fakeBold = true;
                canvas->set_color(mColorRecommended);
            } else if (i != 0) {
                canvas->set_color(mColorOther);
            }
	    canvas->draw_text(suggestionRect,suggestion,Gravity::CENTER);
	    if(fakeBold){
		canvas->translate(.5f,.5f);
		canvas->draw_text(suggestionRect,suggestion,Gravity::CENTER);
		canvas->translate(-.5f,-.5f);
	    }
            canvas->set_color(mColorOther);
	    canvas->move_to(x + wordWidth + 0.5f, bgPadding.top);
            canvas->line_to(x + wordWidth + 0.5f, height + 1);
	}
        x += wordWidth;
    }
    mTotalWidth = x;
    if (mTargetScrollX != getScrollX()) {
         scrollToTarget();
    }
}

void CandidateView::onDraw(Canvas&canvas){
    onDrawInternal(&canvas);
}

void CandidateView::scrollToTarget() {
    int sx = getScrollX();
    if (mTargetScrollX > sx) {
        sx += SCROLL_PIXELS;
        if (sx >= mTargetScrollX) {
            sx = mTargetScrollX;
            requestLayout();
        }
    } else {
        sx -= SCROLL_PIXELS;
        if (sx <= mTargetScrollX) {
            sx = mTargetScrollX;
            requestLayout();
        }
    }
    scrollTo(sx, getScrollY());
    invalidate();
}

void CandidateView::setMaxSuggestion(int max){
    mWordWidth.resize(max);
    mWordX.resize(max);
}

void CandidateView::setSuggestions(const std::vector<std::string>& suggestions, bool completions,
       bool typedWordValid) {
    clear();
    mTypedWordValid = typedWordValid;
    scrollTo(0, 0);
    setMaxSuggestion(suggestions.size());
    mSuggestions = suggestions;
    mTargetScrollX = 0;
    // Compute the total width
    onDrawInternal(nullptr);
    requestLayout();
}

void CandidateView::clear() {
    mSuggestions.clear();
    mTouchX = OUT_OF_BOUNDS;
    mSelectedIndex = -1;
    invalidate(true);
}

void CandidateView::setPredictListener(CandidateView::OnPredictChange ls){
    mOnPredict = ls;
}

bool CandidateView::onTouchEvent(MotionEvent& me) {
    int action = me.getAction();
    int x = (int) me.getX();
    int y = (int) me.getY();
    mTouchX = x;

    switch (action) {
    case MotionEvent::ACTION_DOWN:
        mScrolled = false;
        invalidate();
        break;
    case MotionEvent::ACTION_MOVE:
        if (y <= 0) {
            // Fling up!?
            if (mSelectedIndex >= 0) {
                //mService.pickSuggestionManually(mSelectedIndex);
                mSelectedIndex = -1;
            }
        }
        invalidate();
        break;
    case MotionEvent::ACTION_UP:
        if (!mScrolled) {
            if (mSelectedIndex >= 0) {
		mOnPredict(*this,mSuggestions[mSelectedIndex],mSelectedIndex);
                //mService.pickSuggestionManually(mSelectedIndex);
            }
        }
        mSelectedIndex = -1;
        removeHighlight();
        requestLayout();
        break;
    }
    return true;
}

void CandidateView::takeSuggestionAt(float x) {
    mTouchX = (int) x;
    // To detect candidate
    onDrawInternal(nullptr);
    if (mSelectedIndex >= 0) {
        //mService.pickSuggestionManually(mSelectedIndex);
    }
    invalidate();
}

void CandidateView::removeHighlight() {
    mTouchX = OUT_OF_BOUNDS;
    invalidate();
}

}
