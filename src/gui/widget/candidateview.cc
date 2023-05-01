#include <widget/candidateview.h>
namespace cdroid{

DECLARE_WIDGET(CandidateView)

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
}

int CandidateView::computeHorizontalScrollRange(){
    return mTotalWidth;
}

void CandidateView::onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
    const int measuredWidth = resolveSize(50, widthMeasureSpec);
        
    // Get the desired height of the icon menu view (last row of items does
    // not have a divider below)
    Rect padding;
    if(mSelectionHighlight)
        mSelectionHighlight->getPadding(padding);
    const int desiredHeight = mTextSize + mVerticalPadding + padding.top + padding.height;
      
    // Maximum possible width and desired height
    setMeasuredDimension(measuredWidth, resolveSize(desiredHeight, heightMeasureSpec));
}

void CandidateView::onDrawInternal(Canvas* canvas) {
    if(canvas)View::onDraw(*canvas);
    mTotalWidth = 0;
    if (mSuggestions.empty()) return;

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
    for (int i = 0; i < count; i++) {
	Cairo::TextExtents te;
	std::string suggestion = mSuggestions.at(i);
	mContext->get_text_extents(suggestion,te);
        int wordWidth = (int) te.x_advance + X_GAP * 2;

        mWordX[i] = x;
        mWordWidth[i] = wordWidth;
	suggestionRect.set(x,bgPadding.top,wordWidth,height);
        if (touchX + scrollX >= x && touchX + scrollX < x + wordWidth && !scrolled) {
            if (canvas) {
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
		canvas->save();
		canvas->translate(.5f,.5f);
		canvas->draw_text(suggestionRect,suggestion,Gravity::CENTER);
		canvas->restore();
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
    mTargetScrollX = 0;
    // Compute the total width
    onDrawInternal(nullptr);
    invalidate();
    requestLayout();
}

void CandidateView::clear() {
    mSuggestions.clear();// = EMPTY_LIST;
    mTouchX = OUT_OF_BOUNDS;
    mSelectedIndex = -1;
    invalidate();
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
