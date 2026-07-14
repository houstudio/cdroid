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
#include <text/textutils.h>
#include <widget/candidateview.h>
namespace cdroid{

DECLARE_WIDGET(CandidateView)

CandidateView::CandidateView(int w,int h):View(w,h){
    mSelectionHighlight = nullptr;
    mColorNormal= 0xFFFFFFFF;
    mColorRecommended =0xFFFF0000;
    mColorOther =0xFF00FF00;
    mVerticalPadding=0;
    mBgPadding.set(5,5,5,5);
    setMinimumHeight(28);
    mPaint.setTextSize(20);
    initView();
}

CandidateView::CandidateView(Context*ctx,const AttributeSet&atts):View(ctx,atts){
     mSelectionHighlight = atts.getDrawable("list_selector_background");
     setBackgroundColor(atts.getColor("candidate_background"));
     mColorNormal = atts.getColor("candidate_normal");
     mColorRecommended = atts.getColor("candidate_recommand");
     mColorOther = atts.getColor("candidate_other");
     mVerticalPadding = atts.getDimensionPixelSize("candidate_vertical_padding");
     const int textSize = atts.getDimensionPixelSize("candidate_font_height",20);
     setHorizontalFadingEdgeEnabled(true);
     setWillNotDraw(false);
     setHorizontalScrollBarEnabled(false);
     setVerticalScrollBarEnabled(false);
     setMaxSuggestion(MAX_SUGGESTION);
     mBgPadding.set(5,5,5,5);
     mPaint.setTextSize(textSize);
     initView();
}

void CandidateView::initView(){
     mOnPredict = nullptr;
     GestureDetector::OnGestureListener gl;
     gl.onScroll=[this](MotionEvent* e1, MotionEvent& e2, float distanceX, float distanceY){
         mScrolled = true;
         int sx = getScrollX();
         sx += distanceX;
         if (sx < 0) {
             sx = 0;
         }
         if (sx + getWidth() > mTotalWidth) {
             sx -= distanceX;
         }
         mTargetScrollX = sx;
         scrollTo(sx, getScrollY());
         invalidate();
         return true;
     };
     mGestureDetector = new GestureDetector(mContext,gl);
     mPaint.setColor(mColorNormal);
     setMinimumHeight(mPaint.getTextSize()+8);

     setWillNotDraw(false);
     setHorizontalFadingEdgeEnabled(true);
     setHorizontalScrollBarEnabled(false);
     setVerticalScrollBarEnabled(false);
}

CandidateView::~CandidateView(){
    delete mSelectionHighlight;
    delete mGestureDetector;
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
    int desiredHeight = mPaint.getTextSize() + mVerticalPadding + padding.top + padding.height;
    if(getBackground())desiredHeight =std::max(desiredHeight,getSuggestedMinimumHeight());
    // Maximum possible width and desired height
    LOGV("size=%dx%d",measuredWidth,resolveSize(desiredHeight, heightMeasureSpec));
    setMeasuredDimension(measuredWidth, resolveSize(desiredHeight, heightMeasureSpec));
}

static void drawText(Canvas& canvas,const std::string& text,const Rect&r,Paint&paint) {
    std::u16string u16s=TextUtils::utf8_utf16(text);
    auto fm = paint.getFontMetricsInt();
    auto textHeight = (fm.bottom-fm.top);
    auto textWidth = paint.measureText(text,0,text.length());
    int x = r.left + (r.width - textWidth)/2;
    int y = r.top + (r.height - textHeight)/2;
    y-=fm.ascent;
    paint.drawTextRun(canvas,u16s.c_str(),0,u16s.length(),0,0,x,y,false);
}

void CandidateView::onDrawInternal(Canvas* canvas) {
    if(canvas!=nullptr){
        View::onDraw(*canvas);
    }
    mTotalWidth = 0;
    if (mSuggestions.empty()) return;
    LOGV("%d suggestion canvas=%p",mSuggestions.size(),canvas);

    if (mBgPadding.empty()&&(getBackground()!=nullptr)) {
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
    for (int i = 0; i < count; i++) {
	    std::string suggestion = mSuggestions.at(i);
        const int wordWidth = mPaint.measureText(suggestion) + X_GAP * 2;

        mWordX[i] = x;
        mWordWidth[i] = wordWidth;
	    if((x + wordWidth) < scrollX)continue;
	    if(x >= (scrollX+getWidth()))break;
	    suggestionRect.set(x,0,wordWidth,height);
        if ((touchX + scrollX >= x) && (touchX + scrollX < x + wordWidth) && !scrolled) {
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
                mPaint.setColor(mColorRecommended);
            } else if (i != 0) {
                mPaint.setColor(mColorOther);
            }
	        drawText(*canvas,suggestion,suggestionRect,mPaint);
	        if(fakeBold){
                canvas->translate(.5f,.5f);
                drawText(*canvas,suggestion,suggestionRect,mPaint);
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
    invalidate();
    requestLayout();
}

void CandidateView::clear() {
    mSuggestions.clear();
    mTouchX = OUT_OF_BOUNDS;
    mSelectedIndex = -1;
    invalidate(true);
}

void CandidateView::setPredictListener(const OnPredictChange& ls){
    mOnPredict = ls;
}

bool CandidateView::onTouchEvent(MotionEvent& me) {
    const int action = me.getAction();
    const int x = (int) me.getX();
    const int y = (int) me.getY();
    if(mGestureDetector->onTouchEvent(me)){
        return true;
    }
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
                mOnPredict(*this,mSuggestions[mSelectedIndex],mSelectedIndex);
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
        mOnPredict(*this,mSuggestions[mSelectedIndex],mSelectedIndex);
        //mService.pickSuggestionManually(mSelectedIndex);
    }
    invalidate();
}

void CandidateView::removeHighlight() {
    mTouchX = OUT_OF_BOUNDS;
    invalidate();
}

}
