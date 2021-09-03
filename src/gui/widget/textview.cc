/*
 * Copyright (C) 2015 UI project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <widget/textview.h>
#include <widget/measurespec.h>
#include <gravity.h>
#include <app.h>
#include <layout.h>
#include <cdlog.h>
#include <textutils.h>

#define VERY_WIDE 1024*1024
#define KEY_EVENT_NOT_HANDLED 0
#define KEY_EVENT_HANDLED -1

namespace cdroid {
class Drawables{
public:
    enum{
       LEFT  = 0,
       TOP   = 1,
       RIGHT = 2,
       BOTTOM= 3
    };
    enum{
        DRAWABLE_NONE = -1,
        DRAWABLE_RIGHT= 0,
        DRAWABLE_LEFT = 1
    };
    Drawable* mShowing[4];
    Drawable* mDrawableStart, *mDrawableEnd, *mDrawableError, *mDrawableTemp;
    Drawable* mDrawableLeftInitial, *mDrawableRightInitial;
    bool mIsRtlCompatibilityMode;
    bool mOverride;
    bool mHasTint, mHasTintMode;
    ColorStateList* mTintList;
    int mTintMode;
    int mDrawableSizeTop, mDrawableSizeBottom, mDrawableSizeLeft, mDrawableSizeRight;
    int mDrawableSizeStart, mDrawableSizeEnd, mDrawableSizeError, mDrawableSizeTemp;

    int mDrawableWidthTop, mDrawableWidthBottom, mDrawableHeightLeft, mDrawableHeightRight;
    int mDrawableHeightStart, mDrawableHeightEnd, mDrawableHeightError, mDrawableHeightTemp;
    int mDrawablePadding;
    Rect mCompoundRect;
public:
    bool hasMetadata() {   return mDrawablePadding != 0 || mHasTintMode || mHasTint; }
    Drawables(Context*ctx){
        mIsRtlCompatibilityMode= false;
        mHasTint = mHasTintMode= mOverride =false;
        mTintList=nullptr;
        mShowing[0]=mShowing[1]=mShowing[2]=mShowing[3]=nullptr;
        mDrawableStart=mDrawableEnd=mDrawableError=mDrawableTemp=nullptr;
        mDrawableLeftInitial=mDrawableRightInitial=nullptr;
        mDrawableSizeTop=mDrawableSizeBottom=mDrawableSizeLeft=0;
        mDrawableSizeRight=mDrawableSizeStart=mDrawableSizeEnd=0;
        mDrawableSizeError=mDrawableSizeTemp=0;
        mDrawableWidthTop=mDrawableWidthBottom=mDrawableHeightLeft=0;
        mDrawableHeightRight=mDrawableHeightStart=mDrawableHeightEnd=0;
        mDrawableHeightError=mDrawableHeightTemp=mDrawablePadding=0;
        mCompoundRect.set(0,0,0,0);
    }
    ~Drawables(){
        for(int i=0;i<3;i++)delete mShowing[i];
    }
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
class Marquee {
private:
    static constexpr  float MARQUEE_DELTA_MAX = 0.07f;
    static constexpr  int MARQUEE_DELAY = 1200;
    static constexpr  int MARQUEE_DP_PER_SECOND = 10;//30;

    static constexpr  int MARQUEE_STOPPED = 0x0;
    static constexpr  int MARQUEE_STARTING= 0x1;
    static constexpr  int MARQUEE_RUNNING = 0x2;

    TextView* mView;
    Layout*mLayout;
    int mStatus ;//= MARQUEE_STOPPED;
    float mPixelsPerMs;
    float mMaxScroll;
    float mMaxFadeScroll;
    float mGhostStart;
    float mGhostOffset;
    float mFadeStop;
    int mRepeatLimit;

    float mScroll;
    long mLastAnimationMs;
    Runnable /*Choreographer::FrameCallback*/ mTickCallback;
    Runnable /*Choreographer::FrameCallback*/ mStartCallback;
    Runnable /*Choreographer::FrameCallback*/ mRestartCallback;
	
private:
    void resetScroll() {
        mScroll = 0.0f;
        if (mView ) mView->invalidate();
    }	
public:
    Marquee(TextView* v,Layout*lt) {
        mStatus = MARQUEE_STOPPED;
        float density = 2.65f;//v.getContext().getResources().getDisplayMetrics().density;
        mPixelsPerMs = MARQUEE_DP_PER_SECOND * density / 1000.f;
        mView = v;
        mLayout=lt;
        mTickCallback=[this](){tick();};
        mStartCallback=[this]( ){
            mStatus = MARQUEE_RUNNING;
            mLastAnimationMs = SystemClock::uptimeMillis();//mChoreographer->getFrameTime();
            tick();
        };
        mRestartCallback=[this](){
            if (mStatus == MARQUEE_RUNNING) {
                if (mRepeatLimit >= 0) mRepeatLimit--;
                start(mRepeatLimit);
            }
        };
    }
	
    void tick() {
        if (mStatus != MARQUEE_RUNNING) {
            return;
        }
	
        mView->removeCallbacks(mTickCallback);
	
        if (mView  && (mView->isFocused() || mView->isSelected())) {
            long currentMs = SystemClock::uptimeMillis();
            long deltaMs = currentMs - mLastAnimationMs;
            mLastAnimationMs = currentMs;
            float deltaPx = deltaMs * mPixelsPerMs;
            mScroll += deltaPx;
            if (mScroll > mMaxScroll) {
                mScroll = mMaxScroll;
                mView->postDelayed(mRestartCallback,MARQUEE_DELAY);
            } else {
                mView->postDelayed(mTickCallback,200);
            }
            mView->invalidate();
        }
    }
	
    void stop() {
        mStatus = MARQUEE_STOPPED;
        mView->removeCallbacks(mStartCallback);
        mView->removeCallbacks(mRestartCallback);
        mView->removeCallbacks(mTickCallback);
        resetScroll();
    }
	
    void start(int repeatLimit) {
        if (repeatLimit == 0) {
            stop();
            return;
        }
        mRepeatLimit = repeatLimit;
        if (mView && mLayout ) {
            mStatus = MARQUEE_STARTING;
            mScroll = 0.0f;
            int textWidth = mView->getWidth() - mView->getCompoundPaddingLeft()
                    - mView->getCompoundPaddingRight();
            float lineWidth = mLayout->getLineWidth(0);
            float gap = textWidth / 3.0f;
            mGhostStart = lineWidth - textWidth + gap;
            mMaxScroll = mGhostStart + textWidth;
            mGhostOffset = lineWidth + gap;
            mFadeStop = lineWidth + textWidth / 6.0f;
            mMaxFadeScroll = mGhostStart + lineWidth + lineWidth;
	
            mView->invalidate();
            mView->postDelayed(mStartCallback,200);
        }
    }
    float getGhostOffset(){return mGhostOffset; }
    float getScroll() { return mScroll; }
    float getMaxFadeScroll(){ return mMaxFadeScroll; }	
    bool shouldDrawLeftFade(){ return mScroll <= mFadeStop; }	
    bool shouldDrawGhost() {  return mStatus == MARQUEE_RUNNING && mScroll > mGhostStart; }
    bool isRunning() { return mStatus == MARQUEE_RUNNING; }
    bool isStopped() { return mStatus == MARQUEE_STOPPED; }
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
class TextAppearanceAttributes{
public:
    int mTextColorHighlight = 0;
    ColorStateList* mTextColor = nullptr;
    ColorStateList* mTextColorHint = nullptr;
    ColorStateList* mTextColorLink = nullptr;
    int mTextSize = 0;
    std::string mFontFamily;
    //Typeface mFontTypeface;
    bool mFontFamilyExplicit = false;
    int mTypefaceIndex = -1;
    int mStyleIndex = -1;
    int mFontWeight = -1;
    bool mAllCaps = false;
    int mShadowColor = 0;
    float mShadowDx = 0, mShadowDy = 0, mShadowRadius = 0;
    bool mHasElegant = false;
    bool mElegant = false;
    bool mHasFallbackLineSpacing = false;
    bool mFallbackLineSpacing = false;
    bool mHasLetterSpacing = false;
    float mLetterSpacing = 0;
public:
    void readTextAppearance(Context*ctx,const AttributeSet&atts);
};

void TextAppearanceAttributes::readTextAppearance(Context*ctx,const AttributeSet&atts){
    mTextColorHighlight = atts.getColor("textColorHighlight",mTextColorHighlight);
    mTextColor = ctx->getColorStateList(atts.getString("textColor"));
    mTextColorHint = ctx->getColorStateList(atts.getString("textColorHint"));
    mTextColorLink = ctx->getColorStateList(atts.getString("textColorLink"));
    mTextSize = atts.getDimensionPixelSize("textSize",mTextSize);

    mShadowColor = atts.getInt("shadowColor",mShadowColor);
    mShadowDx =atts.getInt("shadowDx",mShadowDx);
    mShadowDy =atts.getInt("shadowDy",mShadowDy);
    mShadowRadius = atts.getInt("shadowRadius",mShadowRadius);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
TextView::TextView(Context*ctx,const AttributeSet& attrs):View(ctx,attrs){
    initView();
    mTextColor =ctx->getColorStateList(attrs.getString("textColor"));
    mLayout=new Layout(20,50);
    mHintLayout=new Layout(20,50);
    std::string txt=attrs.getString("text");
    mLayout->setText(txt);
    
    Drawable* left =ctx->getDrawable(attrs.getString("drawableLeft"));
    Drawable*right =ctx->getDrawable(attrs.getString("drawableRight"));
    Drawable* top  =ctx->getDrawable(attrs.getString("drawableTop"));
    Drawable*bottom=ctx->getDrawable(attrs.getString("drawableBottom"));
    setCompoundDrawables(left,top,right,bottom);

    setCompoundDrawablePadding(attrs.getDimensionPixelSize("drawablePadding",0));
    
    setMinHeight(attrs.getDimensionPixelSize("minHeight", -1));
    setMaxHeight(attrs.getDimensionPixelSize("maxHeight", -1));

    setMinWidth(attrs.getDimensionPixelSize("minWidth", INT_MIN));
    setMaxWidth(attrs.getDimensionPixelSize("maxWidth", INT_MAX));
    mSingleLine=attrs.getBoolean("singleline",true);

    setTextColor(0xFFFFFFFF);
    setHintTextColor(0xFFFFFFFF);
    
    setMarqueeRepeatLimit(attrs.getInt("marqueeRepeatLimit",mMarqueeRepeatLimit));
    setEllipsize(attrs.getInt("ellipsize",Layout::ELLIPSIS_NONE));
    //Drawable*start =ctx->getDrawable(attrs.getString("drawableStart"));
    //Drawable* end  =ctx->getDrawable(attrs.getString("drawableEnd"));
}

TextView::TextView(int width, int height):TextView(std::string(),width,height){
}

TextView::TextView(const std::string& text, int width, int height)
  : View( width, height) {
    initView();
    mHintLayout->setWidth(width);
    mLayout->setWidth(width);
    mLayout->setText(text);
    //setTextColor(mContext->getColorStateList("cdroid:color/textview.xml"));
}

void TextView::initView(){
    mDrawables=nullptr;
    mMarquee=nullptr;
    mSavedMarqueeModeLayout=nullptr;
    mMaxWidth =INT_MAX;
    mMinWidth =0;
    mMaximum  =INT_MAX;
    mMinimum  =0;
    mSpacingMult=1.0;
    mSpacingAdd =0.f;
    mBlinkOn  =false;
    mRestartMarquee =true;
    mCaretPos =0;
    mCaretRect.set(0,0,0,0);
    mMaxWidthMode =PIXELS;
    mMinWidthMode = PIXELS;
    mMaxMode =LINES;
    mMinMode =LINES;
    mMarqueeRepeatLimit =3;
    mMarqueeFadeMode = MARQUEE_FADE_NORMAL;
    mHorizontallyScrolling =false;
    mEllipsize =Layout::ELLIPSIS_NONE;
    mLayout=new Layout(18,1);
    mHintLayout = new Layout(mLayout->getFontSize(),1);
    mGravity=Gravity::NO_GRAVITY;
    setGravity(Gravity::LEFT|Gravity::CENTER_VERTICAL);
    setTextAlignment(TEXT_ALIGNMENT_TEXT_START);
    mTextColor =mHintTextColor =mLinkTextColor =nullptr;
    mHighlightColor=0x6633B5E5;
    mShadowRadius = .0;
    mShadowDx = .0;
    mShadowDy = .0;
    mShadowColor = 0;
    mCurTextColor= mCurHintTextColor=0;
    mSingleLine =true;
    mEditMode =READONLY;
    setTextColor(0xFFFFFFFF);
    setHintTextColor(0xFFFFFFFF);

    TextAppearanceAttributes attributes;
    attributes.mTextColor = ColorStateList::valueOf(0xFFFFFFFF);
    attributes.mTextSize  = 18;
    applyTextAppearance(&attributes);
}

TextView::~TextView() {
    delete mTextColor;
    delete mHintTextColor;
    delete mLinkTextColor;
    delete mLayout;
    delete mHintLayout;
    delete mDrawables;
}

int TextView::getLayoutAlignment()const{
    int alignment;
    switch (getTextAlignment()) {
    case TEXT_ALIGNMENT_GRAVITY:
         switch (mGravity & Gravity::RELATIVE_HORIZONTAL_GRAVITY_MASK) {
         case Gravity::START:  alignment = Layout::Alignment::ALIGN_NORMAL;      break;
         case Gravity::END:    alignment = Layout::Alignment::ALIGN_OPPOSITE;    break;
         case Gravity::LEFT:   alignment = Layout::Alignment::ALIGN_LEFT;        break;
         case Gravity::RIGHT:  alignment = Layout::Alignment::ALIGN_RIGHT;       break;
         case Gravity::CENTER_HORIZONTAL:alignment = Layout::Alignment::ALIGN_CENTER;  break;
         default: alignment = Layout::Alignment::ALIGN_NORMAL;                   break;
         }break;
    case TEXT_ALIGNMENT_TEXT_START:alignment = Layout::Alignment::ALIGN_NORMAL;  break;
    case TEXT_ALIGNMENT_TEXT_END:  alignment = Layout::Alignment::ALIGN_OPPOSITE;break;
    case TEXT_ALIGNMENT_CENTER:    alignment = Layout::Alignment::ALIGN_CENTER;  break;
    case TEXT_ALIGNMENT_VIEW_START:alignment = (getLayoutDirection() == LAYOUT_DIRECTION_RTL)
                        ? Layout::Alignment::ALIGN_RIGHT : Layout::Alignment::ALIGN_LEFT;
         break;
    case TEXT_ALIGNMENT_VIEW_END:  alignment = (getLayoutDirection() == LAYOUT_DIRECTION_RTL)
                        ? Layout::Alignment::ALIGN_LEFT : Layout::Alignment::ALIGN_RIGHT;
         break;
    case TEXT_ALIGNMENT_INHERIT:
                // This should never happen as we have already resolved the text alignment
                // but better safe than sorry so we just fall through
    default: alignment = Layout::Alignment::ALIGN_NORMAL;  break;
    }
    return alignment;
}

void TextView::applyTextAppearance(class TextAppearanceAttributes *attributes){
    if (attributes->mTextColor) setTextColor(attributes->mTextColor);

    if (attributes->mTextColorHint) setHintTextColor(attributes->mTextColorHint);

    if (attributes->mTextColorLink) setLinkTextColor(attributes->mTextColorLink);

    if (attributes->mTextColorHighlight) setHighlightColor(attributes->mTextColorHighlight);

    if (attributes->mTextSize != 0) setRawTextSize(attributes->mTextSize, true /* shouldRequestLayout */);

    /*if (attributes.mTypefaceIndex != -1 && !attributes.mFontFamilyExplicit) {
        attributes.mFontFamily = nullptr;
    }
    setTypefaceFromAttrs(attributes.mFontTypeface, attributes.mFontFamily,
            attributes.mTypefaceIndex, attributes.mStyleIndex, attributes.mFontWeight);*/

    if (attributes->mShadowColor != 0) {
        setShadowLayer(attributes->mShadowRadius, attributes->mShadowDx, attributes->mShadowDy,
                attributes->mShadowColor);
    }

    /*if (attributes.mAllCaps) setTransformationMethod(new AllCapsTransformationMethod(getContext()));

    if (attributes.mHasElegant) setElegantTextHeight(attributes.mElegant);

    if (attributes.mHasFallbackLineSpacing) {
        setFallbackLineSpacing(attributes.mFallbackLineSpacing);
    }

    if (attributes.mHasLetterSpacing) {
        setLetterSpacing(attributes.mLetterSpacing);
    }

    if (attributes.mFontFeatureSettings != null) {
        setFontFeatureSettings(attributes.mFontFeatureSettings);
    }*/    
}

void TextView::addTextChangedListener(TextWatcher watcher){
    mListeners.push_back(watcher);
}

void TextView::removeTextChangedListener(TextWatcher watcher){
    for(auto i= mListeners.begin();i!= mListeners.end();i++){
        TextWatcher w=(*i);
        if( (w.onTextChanged==watcher.onTextChanged)
            &&(w.beforeTextChanged==watcher.beforeTextChanged) ){
            mListeners.erase(i);
            break;
        }
    } 
}

void TextView::sendBeforeTextChanged(const std::wstring& text, int start, int before, int after){
    for(auto l:mListeners){
        if(l.beforeTextChanged) l.beforeTextChanged(text, start, before, after);
    }
}

void TextView::sendAfterTextChanged(std::wstring& text){
    for (auto l:mListeners) {
        if(l.afterTextChanged) l.afterTextChanged(text);
    }

    // Always notify AutoFillManager - it will return right away if autofill is disabled.
    //notifyAutoFillManagerAfterTextChangedIfNeeded();
    //hideErrorIfUnchanged();
}
void TextView::sendOnTextChanged(const std::wstring& text, int start, int before, int after){
    for(auto l:mListeners){
        if(l.onTextChanged) l.onTextChanged(text, start, before, after);
    }
    //if (mEditor != null) mEditor.sendOnTextChanged(start, before, after);
}

void TextView::setRawTextSize(float size, bool shouldRequestLayout){
    mLayout->setFontSize(size);
    mHintLayout->setFontSize(size);
    if(shouldRequestLayout)
        requestLayout();
}

void TextView::setTextSizeInternal(int unit, float size, bool shouldRequestLayout){
    setRawTextSize(size,shouldRequestLayout);
}

void TextView::setTextSize(int unit, float size){
    setTextSizeInternal(unit,size,true);
}

void TextView::setTextSize(float size){
    return setTextSize(0,size);
}

float TextView::getTextSize()const{
    return mLayout->getFontSize();
}

int TextView::computeVerticalScrollRange(){
    return mLayout->getHeight();
}

int TextView::computeHorizontalScrollRange(){
    return mLayout->getMaxLineWidth();
}

int TextView::getHorizontalOffsetForDrawables()const{
    return 0;
}

void TextView::setText(const std::string&txt){
    mLayout->setText(txt);
    mCaretPos=0;
    mLayout->setCaretPos(mCaretPos);
    std::wstring&wText=getEditable();
    invalidate(true);
}

const std::string TextView::getText()const{
    return mLayout->getString();
}

View& TextView::setHint(const std::string& hint){
    View::setHint(hint);
    mHintLayout->setText(hint);
    if(getEditable().empty())
        invalidate();
    return *this;
}

std::wstring& TextView::getEditable(){
    return mLayout->getText();
}

void TextView::setEditable(bool b){
    mLayout->setEditable(b);
}

int TextView::getFontSize()const{
    return mLayout->getFontSize();
}

void TextView::setCaretPos(int pos){
    mCaretPos=pos;
    mBlinkOn=true;
    mLayout->setCaretPos(pos);
    invalidate(true);
}

int TextView::getCaretPos()const{
    return mCaretPos;
}

bool TextView::moveCaret2Line(int line){
    int curline = mLayout->getLineForOffset(mCaretPos);
    int curcolumns = mCaretPos - mLayout->getLineStart(curline);
    if(line<getLineCount()){
        int newcolumns = mLayout->getLineEnd(line)-mLayout->getLineStart(line);
        newcolumns=std::min(newcolumns,curcolumns);
        setCaretPos(mLayout->getLineStart(line) + newcolumns);
        return true;
    }
    return false;
}

void TextView::setLineSpacing(float add, float mult) {
    if (mSpacingAdd != add || mSpacingMult != mult) {
        mSpacingAdd = add;
        mSpacingMult = mult;
        if ( mLayout ) {
            mLayout->setLineSpacing(add,mult);
            requestLayout();
            invalidate();
        }
    }
}

float TextView::getLineSpacingMultiplier()const{
    return mSpacingMult;
}

float TextView::getLineSpacingExtra()const{
    return mSpacingAdd;
}

void TextView::onSizeChanged(int w,int h,int ow,int oh){
    View::onSizeChanged(w,h,ow,oh);
    mLayout->setWidth(w-mPaddingLeft-mPaddingRight);
    mHintLayout->setWidth(w-mPaddingLeft-mPaddingRight);
    LOGV("(%d,%d)->(%d,%d)",ow,oh,w,h);
}

void TextView::onLayout(bool changed, int left, int top, int w, int h){

}

void TextView::setGravity(int gravity){
    if ((gravity & Gravity::RELATIVE_HORIZONTAL_GRAVITY_MASK) == 0) {
        gravity |= Gravity::START;
    }
    if ((gravity & Gravity::VERTICAL_GRAVITY_MASK) == 0) {
        gravity |= Gravity::TOP;
    }

    bool newLayout = false;

    if ((gravity & Gravity::RELATIVE_HORIZONTAL_GRAVITY_MASK)
            != (mGravity & Gravity::RELATIVE_HORIZONTAL_GRAVITY_MASK)) {
        newLayout = true;
    }

    if (gravity != mGravity)  invalidate(true);

    mGravity = gravity;
    mLayout->setAlignment(getLayoutAlignment());
}

int TextView::getGravity()const{
    return mGravity;
}

void TextView::setMinWidth(int minPixels){
    mMinWidth = minPixels;
    mMinWidthMode = PIXELS;
    requestLayout();
    invalidate(true);    
}
int TextView::getMinWidth()const{
    return mMinWidthMode == PIXELS ? mMinWidth : -1;
}
void TextView::setMaxWidth(int maxPixels){
    mMaxWidth = maxPixels;
    mMaxWidthMode = PIXELS;
    requestLayout();
    invalidate(true);
}

int TextView::getMaxWidth()const{
    return mMaxWidthMode == PIXELS ? mMaxWidth : -1;
}
int TextView::getLineCount()const{
    return mLayout? mLayout->getLineCount() : 0;
}
int TextView::getLineBounds(int line, Rect&bounds) {
    int baseline = mLayout->getLineBounds(line, bounds);

    int voffset = getExtendedPaddingTop();
    if ((mGravity & Gravity::VERTICAL_GRAVITY_MASK) != Gravity::TOP) {
        voffset += getVerticalOffset(true);
    }
    bounds.offset(getCompoundPaddingLeft(), voffset);
    return baseline + voffset;
}

int TextView::getLineHeight()const{
    return mLayout->getLineBottom(0)-mLayout->getLineTop(0);
}

void TextView::setLineHeight(int h){
    
}

int TextView::getDesiredHeight(){
    return getDesiredHeight(mLayout,true);
}

int TextView::getDesiredHeight(Layout* layout, bool cap){
    if (layout == nullptr) {
        return 0;
    }

    layout->relayout();
    /*
    * Don't cap the hint to a certain number of lines.
    * (Do cap it, though, if we have a maximum pixel height.)
    */

    int desired = layout->getHeight(cap);

    Drawables* dr = mDrawables;
    if (dr != nullptr) {
        desired = std::max(desired, dr->mDrawableHeightLeft);
        desired = std::max(desired, dr->mDrawableHeightRight);
    }

    int linecount = layout->getLineCount();
    int padding = getCompoundPaddingTop() + getCompoundPaddingBottom();
    desired += padding;

    if (mMaxMode != LINES) {
        desired = std::min(desired, mMaximum);
    } else if (cap && linecount > mMaximum/* && (layout instanceof DynamicLayout
            || layout instanceof BoringLayout)*/) {
        desired = layout->getLineTop(mMaximum);

        if (dr != nullptr) {
            desired = std::max(desired, dr->mDrawableHeightLeft);
            desired = std::max(desired, dr->mDrawableHeightRight);
        }

        desired += padding;
        linecount = mMaximum;
    }

    if (mMinMode == LINES) {
        if (linecount < mMinimum) {
            desired += getLineHeight() * (mMinimum - linecount);
        }
    } else {
        desired = std::max(desired, mMinimum);
    }

    // Check against our minimum height
    desired = std::max(desired, getSuggestedMinimumHeight());
    return desired;
}

int TextView::getMinHeight()const {
    return mMinMode == PIXELS ? mMinimum : -1;
}

void TextView::setMinHeight(int minPixels){
    mMinimum = minPixels;
    mMinMode = PIXELS;
    requestLayout();
    invalidate(true);
}

void TextView::setMaxLines(int maxLines){
    mMaximum = maxLines;
    mMaxMode = LINES;

    requestLayout();
    invalidate();
}

int TextView::getMaxLines()const{
    return mMaxMode == LINES ? mMaximum : -1;
}

int TextView::getMaxHeight()const{
    return mMaxMode == PIXELS ? mMaximum : -1;
}

void TextView::setMaxHeight(int maxPixels){
    mMaximum = maxPixels;
    mMaxMode = PIXELS;

    requestLayout();
    invalidate(true);
}

int TextView::desired(Layout*layout){
    int N=layout->getLineCount();
    int max=0;
    const std::wstring& text=layout->getText();
    for (int i = 0; i < N - 1; i++) {
        if (text[layout->getLineEnd(i) - 1]!= '\n') {
            return -1;
        }
    }

    for (int i = 0; i < N; i++) {
        max = std::max(max, layout->getLineWidth(i));
    }

    return max;
}

void TextView::onMeasure(int widthMeasureSpec, int heightMeasureSpec){
    int widthMode  = MeasureSpec::getMode(widthMeasureSpec);
    int heightMode = MeasureSpec::getMode(heightMeasureSpec);
    int widthSize  = MeasureSpec::getSize(widthMeasureSpec);
    int heightSize = MeasureSpec::getSize(heightMeasureSpec);

    int width;
    int height;

    int des = -1;
    bool fromexisting = false;
    mLayout->setAlignment(getLayoutAlignment());
    if (widthMode == MeasureSpec::EXACTLY) {
        // Parent has told us how big to be. So be it.
        width = widthSize;
        mLayout->setWidth(width);
    } else {
        int txtWidth,txtHeight;
        mLayout->setWidth(INT_MAX);
        mLayout->relayout();
        mHintLayout->relayout();
        txtWidth=desired(mLayout);
        txtHeight=mLayout->getHeight();
        LOGV("%p Measuredsize=%dx%d fontsize=%d",this,txtWidth,txtHeight,getFontSize());
        width=txtWidth+getPaddingLeft()+getPaddingRight();;
        Drawables* dr = mDrawables;
        if (dr != nullptr) {
            width = std::max(width, dr->mDrawableWidthTop);
            width = std::max(width, dr->mDrawableWidthBottom);
        }

        width += getCompoundPaddingLeft() + getCompoundPaddingRight();

        if (mMaxWidthMode == EMS)width = std::min(width, mMaxWidth * getLineHeight());
        else   width = std::min(width, mMaxWidth);

        if (mMinWidthMode == EMS) width = std::max(width, mMinWidth * getLineHeight());
        else  width = std::max(width, mMinWidth);

        // Check against our minimum width
        width = std::max(width, getSuggestedMinimumWidth());
        if (widthMode == MeasureSpec::AT_MOST)width = std::min(widthSize, width);
    }

    int want = width - getCompoundPaddingLeft() - getCompoundPaddingRight();
    int unpaddedWidth = want;

    if (mHorizontallyScrolling) want = VERY_WIDE;

    int hintWant = want;
    int hintWidth = (mHintLayout == nullptr) ? hintWant : mHintLayout->getMaxLineWidth();

    mLayout->setWidth(width);
    if (heightMode == MeasureSpec::EXACTLY) {
        // Parent has told us how big to be. So be it.
        height = heightSize;
        mDesiredHeightAtMeasure = -1;
    } else {
        int desired = getDesiredHeight();
        height = desired;
        mDesiredHeightAtMeasure = desired;

        if (heightMode == MeasureSpec::AT_MOST) {
            height = std::min(desired, heightSize);
        }
    }

    int unpaddedHeight = height - getCompoundPaddingTop() - getCompoundPaddingBottom();
    setMeasuredDimension(width, height);
}

std::vector<Drawable*>TextView::getCompoundDrawables(){
    std::vector<Drawable*>ret;
    for(int i=0;i<4;i++){
        ret.push_back(mDrawables->mShowing[i]);
    }
    return ret;
}

void TextView::setCompoundDrawables(Drawable* left,Drawable* top,Drawable* right,Drawable*bottom){
    // We're switching to absolute, discard relative.
    Drawables*dr=mDrawables;
    if (dr != nullptr) {
        if (dr->mDrawableStart != nullptr)dr->mDrawableStart->setCallback(nullptr);
        dr->mDrawableStart = nullptr;
        if (dr->mDrawableEnd != nullptr) dr->mDrawableEnd->setCallback(nullptr);
        dr->mDrawableEnd = nullptr;
        dr->mDrawableSizeStart= dr->mDrawableHeightStart = 0;
        dr->mDrawableSizeEnd  = dr->mDrawableHeightEnd = 0;
    }
    const bool drawables = left != nullptr || top != nullptr || right != nullptr || bottom != nullptr;
    if (!drawables) {
        // Clearing drawables...  can we free the data structure?
        if (dr != nullptr) {
            if (!dr->hasMetadata()) {
                mDrawables = nullptr;
            } else {
                // We need to retain the last set padding, so just clear
                // out all of the fields in the existing structure.
                for (int i = 3/*dr->mShowing.length - 1*/; i >= 0; i--) {
                    if (dr->mShowing[i] != nullptr) {
                        dr->mShowing[i]->setCallback(nullptr);
                    }
                    dr->mShowing[i] = nullptr;
                }
                dr->mDrawableSizeLeft  = dr->mDrawableHeightLeft  = 0;
                dr->mDrawableSizeRight = dr->mDrawableHeightRight = 0;
                dr->mDrawableSizeTop   = dr->mDrawableWidthTop    = 0;
                dr->mDrawableSizeBottom= dr->mDrawableWidthBottom = 0;
            }
        }
    } else {
        if (dr == nullptr)  mDrawables = dr = new Drawables(getContext());

        mDrawables->mOverride = false;

        if (dr->mShowing[Drawables::LEFT] != left && dr->mShowing[Drawables::LEFT] != nullptr) {
            dr->mShowing[Drawables::LEFT]->setCallback(nullptr);
        }
        dr->mShowing[Drawables::LEFT] = left;
        if (dr->mShowing[Drawables::TOP] != top && dr->mShowing[Drawables::TOP] != nullptr) {
            dr->mShowing[Drawables::TOP]->setCallback(nullptr);
        }
        dr->mShowing[Drawables::TOP] = top;

        if (dr->mShowing[Drawables::RIGHT] != right && dr->mShowing[Drawables::RIGHT] != nullptr) {
            dr->mShowing[Drawables::RIGHT]->setCallback(nullptr);
        }
        dr->mShowing[Drawables::RIGHT] = right;
        if (dr->mShowing[Drawables::BOTTOM] != bottom && dr->mShowing[Drawables::BOTTOM] != nullptr) {
            dr->mShowing[Drawables::BOTTOM]->setCallback(nullptr);
        }
        dr->mShowing[Drawables::BOTTOM] = bottom;
        Rect compoundRect = dr->mCompoundRect;
        std::vector<int>state = getDrawableState();
        if (left != nullptr) {
            left->setState(state);
            compoundRect=left->getBounds();
            left->setCallback(this);
            dr->mDrawableSizeLeft  = compoundRect.width;
            dr->mDrawableHeightLeft= compoundRect.height;
        } else {
            dr->mDrawableSizeLeft = dr->mDrawableHeightLeft = 0;
        }
        if (right != nullptr) {
            right->setState(state);
            compoundRect=right->getBounds();
            right->setCallback(this);
            dr->mDrawableSizeRight = compoundRect.width;
            dr->mDrawableHeightRight=compoundRect.height;
        } else {
            dr->mDrawableSizeRight = dr->mDrawableHeightRight = 0;
        }

        if (top != nullptr) {
            top->setState(state);
            compoundRect=top->getBounds();
            top->setCallback(this);
            dr->mDrawableSizeTop = compoundRect.height;
            dr->mDrawableWidthTop = compoundRect.width;
        } else {
            dr->mDrawableSizeTop = dr->mDrawableWidthTop = 0;
        }

        if (bottom != nullptr) {
            bottom->setState(state);
            compoundRect=bottom->getBounds();
            bottom->setCallback(this);
            dr->mDrawableSizeBottom = compoundRect.height;
            dr->mDrawableWidthBottom = compoundRect.width;
        } else {
            dr->mDrawableSizeBottom = dr->mDrawableWidthBottom = 0;
        }
    }

    // Save initial left/right drawables
    if (dr != nullptr) {
        dr->mDrawableLeftInitial = left;
        dr->mDrawableRightInitial = right;
    }

    resetResolvedDrawables();
    resolveDrawables();
    applyCompoundDrawableTint();
    invalidate(true);
    requestLayout();
}

void TextView::setCompoundDrawablesWithIntrinsicBounds(Drawable* left,Drawable* top,Drawable* right,Drawable*bottom){
    if (left) left->setBounds(0, 0, left->getIntrinsicWidth(), left->getIntrinsicHeight());      
    if (right)right->setBounds(0, 0, right->getIntrinsicWidth(), right->getIntrinsicHeight());
    if (top)  top->setBounds(0, 0, top->getIntrinsicWidth(), top->getIntrinsicHeight());
    if (bottom)bottom->setBounds(0, 0, bottom->getIntrinsicWidth(), bottom->getIntrinsicHeight());
    setCompoundDrawables(left, top, right, bottom);
}

void TextView::setCompoundDrawablesWithIntrinsicBounds(const std::string& left, const std::string& top,
		const std::string& right,const std::string& bottom){
    Context* context = getContext();
    setCompoundDrawablesWithIntrinsicBounds(context->getDrawable(left),context->getDrawable(top),
            context->getDrawable(right),context->getDrawable(bottom));    
}
void TextView::drawableStateChanged(){
    View::drawableStateChanged();

    if (mTextColor && mTextColor->isStateful()
            || (mHintTextColor && mHintTextColor->isStateful())
            || (mLinkTextColor && mLinkTextColor->isStateful())) {
        updateTextColors();
    }
    if (mDrawables) {
        const std::vector<int>& state = getDrawableState();
        for (int i=0;i<4;i++){
            Drawable* dr=mDrawables->mShowing[i];
            if (dr != nullptr && dr->isStateful() && dr->setState(state)) {
                invalidateDrawable(*dr);
            }
        }
    }
}

void TextView::updateTextColors(){
    bool inval = false;
    const std::vector<int>&drawableState = getDrawableState();
    int color = mTextColor->getColorForState(drawableState,mTextColor->getDefaultColor());
    if (color != mCurTextColor) {
        mCurTextColor = color;
        inval = true;
    }
    if (mLinkTextColor != nullptr) {
        color = mLinkTextColor->getColorForState(drawableState, mLinkTextColor->getDefaultColor());
        //if (color != mTextPaint.linkColor) {
          //  mTextPaint.linkColor = color;
            inval = true;
        //}
    }
    if (mHintTextColor != nullptr) {
        color = mHintTextColor->getColorForState(drawableState, mHintTextColor->getDefaultColor());
        if (color != mCurHintTextColor) {
            mCurHintTextColor = color;
            //if (mText.length() == 0) inval = true;
        }
    }
    if (inval) {
        // Text needs to be redrawn with the new color
        //if (mEditor != null) mEditor.invalidateTextDisplayList();
        invalidate(true);
    }
}

void TextView::setMarqueeRepeatLimit(int marqueeLimit) {
    mMarqueeRepeatLimit = marqueeLimit;
}
int TextView::getMarqueeRepeatLimit()const{
    return mMarqueeRepeatLimit;
}

int TextView::getEllipsize()const{
    return mEllipsize;
}

void TextView::setEllipsize(int where){
    if (mEllipsize != where) {
        mEllipsize = where;
        if (mLayout) {
            mLayout->setEllipsis(where);
            //nullLayouts();
            requestLayout();
            invalidate();
        }
    }    
}

void TextView::setTextColor(int color){
    delete mTextColor;
    mTextColor = ColorStateList::valueOf(color);
    updateTextColors();
}

void TextView::setTextColor(ColorStateList* colors){
    if(colors!=nullptr){
        delete mTextColor;
        mTextColor = colors;
        updateTextColors();
    }
}

ColorStateList* TextView::getTextColors()const{
    return mTextColor;
}

int TextView::getCurrentTextColor()const{
    return mCurTextColor;
}

Layout* TextView::getLayout()const{
    return mLayout;
}


Layout* TextView::getHintLayout()const{
    return mHintLayout;
}

void TextView::setHighlightColor(int color){
    if(mHighlightColor != color){
        mHighlightColor = color;
        invalidate(true);
    }
}

int TextView::getHighlightColor()const{
    return mHighlightColor;
}

void TextView::setHintTextColor(int color){
    delete mHintTextColor;
    mHintTextColor = ColorStateList::valueOf(color);
    updateTextColors();
}

void TextView::setHintTextColor(ColorStateList* colors){
    if(colors!=nullptr){
        delete mHintTextColor;
        mHintTextColor=colors;
        updateTextColors();
    }
}

ColorStateList* TextView::getHintTextColors()const{
    return mHintTextColor;
}

int TextView::getCurrentHintTextColor()const{
    return mHintTextColor != nullptr ? mCurHintTextColor : mCurTextColor;
}


void TextView::setLinkTextColor(int color){
    delete mLinkTextColor;
    mLinkTextColor = ColorStateList::valueOf(color);
    updateTextColors();
}

void TextView::setLinkTextColor(ColorStateList* colors){
    if(colors){
        delete mLinkTextColor;
        mLinkTextColor=colors;
        updateTextColors();
    }
}

ColorStateList* TextView::getLinkTextColors()const{
    return mLinkTextColor;
}

void TextView::applyCompoundDrawableTint(){
    if (mDrawables == nullptr) return;
    if ( (mDrawables->mHasTint==false)&&(mDrawables->mHasTintMode==false) )return ;

    ColorStateList* tintList = mDrawables->mTintList;
    const int tintMode = mDrawables->mTintMode;
    const bool hasTint = mDrawables->mHasTint;
    const bool hasTintMode = mDrawables->mHasTintMode;
    const std::vector<int>state = getDrawableState();

    for (int i=0;i<4;i++){
        Drawable* dr=mDrawables->mShowing[i];
        if (dr == nullptr)continue;

        if (dr == mDrawables->mDrawableError) {
            // From a developer's perspective, the error drawable isn't 
            // a compound drawable. Don't apply the generic compound drawable tint to it.
            continue;
        }

        dr->mutate();

        if (hasTint)dr->setTintList(tintList);

        if (hasTintMode) dr->setTintMode(tintMode);

        // The drawable (or one of its children) may not have been
        // stateful before applying the tint, so let's try again.
        if (dr->isStateful())  dr->setState(state);
    }
}

void TextView::setCompoundDrawablePadding(int pad){
    if (pad == 0) {
       if (mDrawables != nullptr)
           mDrawables->mDrawablePadding = pad;
    } else {
         if (mDrawables == nullptr) 
             mDrawables =new Drawables(getContext());
         mDrawables->mDrawablePadding = pad;
    }
    invalidate(true);
}

int TextView::getCompoundDrawablePadding()const{
    return mDrawables?mDrawables->mDrawablePadding:0;
}

int TextView::getCompoundPaddingLeft(){
    Drawables* dr = mDrawables;
    if (dr == nullptr || dr->mShowing[Drawables::LEFT] == nullptr) {
        return mPaddingLeft;
    } else {
        return mPaddingLeft + dr->mDrawablePadding + dr->mDrawableSizeLeft;
    }
}

int TextView::getCompoundPaddingRight(){
    Drawables* dr = mDrawables;
    if (dr == nullptr || dr->mShowing[Drawables::RIGHT] == nullptr) {
        return mPaddingRight;
    } else {
        return mPaddingRight + dr->mDrawablePadding + dr->mDrawableSizeRight;
    }
}

int TextView::getCompoundPaddingTop(){
    Drawables* dr = mDrawables;
    if (dr == nullptr || dr->mShowing[Drawables::TOP] == nullptr) {
        return mPaddingTop;
    } else {
        return mPaddingTop + dr->mDrawablePadding + dr->mDrawableSizeTop;
    }
}

int TextView::getCompoundPaddingBottom(){
    Drawables* dr = mDrawables;
    if (dr == nullptr || dr->mShowing[Drawables::BOTTOM] == nullptr) {
        return mPaddingBottom;
    } else {
        return mPaddingBottom + dr->mDrawablePadding + dr->mDrawableSizeBottom;
    }
}

int TextView::getCompoundPaddingStart(){
    resolveDrawables();
    switch(getLayoutDirection()) {
    default:
    case LAYOUT_DIRECTION_LTR:return getCompoundPaddingLeft();
    case LAYOUT_DIRECTION_RTL:return getCompoundPaddingRight();
    }
}

int TextView::getCompoundPaddingEnd(){
    resolveDrawables();
    switch(getLayoutDirection()) {
    default:
    case LAYOUT_DIRECTION_LTR:return getCompoundPaddingRight();
    case LAYOUT_DIRECTION_RTL:return getCompoundPaddingLeft();
    }
}

int TextView::getExtendedPaddingTop(){
    if (mMaxMode != LINES) {
        return getCompoundPaddingTop();
    }

    //if (mLayout == nullptr)assumeLayout();

    if (mLayout->getLineCount() <= mMaximum) {
        return getCompoundPaddingTop();
    }

    int top = getCompoundPaddingTop();
    int bottom = getCompoundPaddingBottom();
    int viewht = getHeight() - top - bottom;
    int layoutht = mLayout->getLineTop(mMaximum);
    if (layoutht >= viewht) {
        return top;
    }

    const int gravity = mGravity & Gravity::VERTICAL_GRAVITY_MASK;
    if (gravity == Gravity::TOP) {
        return top;
    } else if (gravity == Gravity::BOTTOM) {
        return top + viewht - layoutht;
    } else { // (gravity == Gravity::CENTER_VERTICAL)
        return top + (viewht - layoutht) / 2;
    }
}

int TextView::getExtendedPaddingBottom(){
    return getCompoundPaddingBottom();
}

void TextView::setCompoundDrawableTintList(ColorStateList* tint){
    if (mDrawables == nullptr) {
        mDrawables = new Drawables(getContext());
    }
    mDrawables->mTintList = tint;
    mDrawables->mHasTint = true;

    applyCompoundDrawableTint();
}

ColorStateList* TextView::getCompoundDrawableTintList(){
    return mDrawables ? mDrawables->mTintList : nullptr;
}

void TextView::setCompoundDrawableTintMode(int tintMode){
    if (mDrawables == nullptr) {
        mDrawables = new Drawables(getContext());
    }
    mDrawables->mTintMode = tintMode;
    mDrawables->mHasTintMode = true;

    applyCompoundDrawableTint();
}

int TextView::getCompoundDrawableTintMode(){
    return mDrawables ? mDrawables->mTintMode : -1;
}

int TextView::getBoxHeight(Layout* l){
    int padding = getExtendedPaddingTop() + getExtendedPaddingBottom();
    return getMeasuredHeight()?(getMeasuredHeight() - padding):getHeight();
}

int TextView::getVerticalOffset(bool forceNormal){
    int voffset = 0;
    const int gravity = mGravity & Gravity::VERTICAL_GRAVITY_MASK;
    Layout* l = mLayout;
    if (gravity != Gravity::TOP) {
        int boxht = getBoxHeight(l);
        int textht = mLayout->getHeight();//LineHeight(true);
        if (textht < boxht) {
            if (gravity == Gravity::BOTTOM) {
                voffset = boxht - textht;
            } else { // (gravity == Gravity::CENTER_VERTICAL)
                voffset = (boxht - textht) >> 1;
            }
        }
    }
    return voffset;    
}

int TextView::getBottomVerticalOffset(bool forceNormal){
    int voffset = 0;
    const int gravity = mGravity & Gravity::VERTICAL_GRAVITY_MASK;
    Layout* l = mLayout;

    if (gravity != Gravity::BOTTOM) {
        int boxht = getBoxHeight(l);
        int textht = l->getHeight();

        if (textht < boxht) {
            if (gravity == Gravity::TOP) {
                voffset = boxht - textht;
            } else { // (gravity == Gravity::CENTER_VERTICAL)
                voffset = (boxht - textht) >> 1;
            }
        }
    }
    return voffset;
}

void TextView::setShadowLayer(float radius, float dx, float dy, int color){
    mShadowRadius = radius;
    mShadowDx = dx;
    mShadowDy = dy;
    mShadowColor = color;
}

float TextView::getShadowRadius()const{
    return mShadowRadius;
}
float TextView::getShadowDx()const{
    return mShadowDx;
}
float TextView::getShadowDy()const{
    return mShadowDy;
}
int TextView::getShadowColor()const{
    return mShadowColor;
}

int TextView::getTotalPaddingTop(){
    return getExtendedPaddingTop() + getVerticalOffset(true);
}

int TextView::getTotalPaddingBottom(){
    return getExtendedPaddingBottom() + getBottomVerticalOffset(true);
}

void TextView::setSingleLine(bool single){
    mSingleLine=single;
    mLayout->setMultiline(!single);
    mLayout->relayout();
    invalidate(true);
}
void TextView::setBreakStrategy(int breakStrategy){
    mLayout->setBreakStrategy(breakStrategy);
}
int TextView::getBreakStrategy()const{
    return mLayout->getBreakStrategy();
}
bool TextView::isSingleLine()const{
    return mSingleLine;
}

bool TextView::isMarqueeFadeEnabled(){
    return mEllipsize == Layout::ELLIPSIS_MARQUEE && mMarqueeFadeMode != MARQUEE_FADE_SWITCH_SHOW_ELLIPSIS;
}

bool TextView::canMarquee(){
    int width = mWidth - getCompoundPaddingLeft() - getCompoundPaddingRight();
    return width > 0 && (mLayout->getLineWidth(0) > width);
        /*|| (mMarqueeFadeMode != MARQUEE_FADE_NORMAL && mSavedMarqueeModeLayout != nullptr
                    && mSavedMarqueeModeLayout->getLineWidth(0) > width));*/
}

void TextView::startMarquee(){
    /*if (getKeyListener() != nullptr) return;
    if (compressText(getWidth() - getCompoundPaddingLeft() - getCompoundPaddingRight())) {
        return;
    }*/
    if ((mMarquee == nullptr || mMarquee->isStopped()) && (isFocused() || isSelected())
                && getLineCount() == 1 && canMarquee()) {
        if (mMarqueeFadeMode == MARQUEE_FADE_SWITCH_SHOW_ELLIPSIS) {
            mMarqueeFadeMode = MARQUEE_FADE_SWITCH_SHOW_FADE;
            Layout* tmp = mLayout;
            mLayout = mSavedMarqueeModeLayout;
            mSavedMarqueeModeLayout = tmp;
            setHorizontalFadingEdgeEnabled(true);
            requestLayout();
            invalidate();
        }

        if (mMarquee == nullptr) mMarquee = new Marquee(this,mLayout);
        mMarquee->start(mMarqueeRepeatLimit);
    }
}

void TextView::stopMarquee(){
    if (mMarquee  && !mMarquee->isStopped()) {
        mMarquee->stop();
    }

    if (mMarqueeFadeMode == MARQUEE_FADE_SWITCH_SHOW_FADE) {
        mMarqueeFadeMode = MARQUEE_FADE_SWITCH_SHOW_ELLIPSIS;
        Layout* tmp = mSavedMarqueeModeLayout;
        mSavedMarqueeModeLayout = mLayout;
        mLayout = tmp;
        setHorizontalFadingEdgeEnabled(false);
        requestLayout();
        invalidate();
    }
}

void TextView::startStopMarquee(bool start){
    if (mEllipsize == Layout::ELLIPSIS_MARQUEE) {
        if (start) {
            startMarquee();
        } else {
            stopMarquee();
        }
    }
}

void TextView::restartMarqueeIfNeeded(){
    if (mRestartMarquee && mEllipsize == Layout::ELLIPSIS_MARQUEE) {
        mLayout->relayout();
        mRestartMarquee = false;
        startMarquee();
    }
}

void TextView::onDraw(Canvas& canvas) {
    restartMarqueeIfNeeded();
    View::onDraw(canvas);
    Rect rcimg,rect=getClientRect();
    const int compoundPaddingLeft = getCompoundPaddingLeft();
    const int compoundPaddingTop = getCompoundPaddingTop();
    const int compoundPaddingRight = getCompoundPaddingRight();
    const int compoundPaddingBottom = getCompoundPaddingBottom();
    const int offset =  getHorizontalOffsetForDrawables();
    const int leftOffset = isLayoutRtl() ? 0 : offset;
    const int rightOffset = isLayoutRtl() ? offset : 0;
    Drawables* dr = mDrawables;
    mLayout->setAlignment(getLayoutAlignment());
    mLayout->relayout();
    if (dr != nullptr) {
        /* Compound, not extended, because the icon is not clipped if the text height is smaller. */

        int vspace = getHeight()- compoundPaddingBottom - compoundPaddingTop;
        int hspace = getWidth() - compoundPaddingRight - compoundPaddingLeft;

        // IMPORTANT: The coordinates computed are also used in invalidateDrawable()
        // Make sure to update invalidateDrawable() when changing this code.
        if (dr->mShowing[Drawables::LEFT] != nullptr) {
            canvas.save();
            canvas.translate(mScrollX + mPaddingLeft + leftOffset,
                    mScrollY + compoundPaddingTop + (vspace - dr->mDrawableHeightLeft) / 2);
            dr->mShowing[Drawables::LEFT]->draw(canvas);
            canvas.restore();
        }

        // IMPORTANT: The coordinates computed are also used in invalidateDrawable()
        // Make sure to update invalidateDrawable() when changing this code.
        if (dr->mShowing[Drawables::RIGHT] != nullptr) {
            canvas.save();
            canvas.translate(mScrollX + getWidth() - mPaddingRight - dr->mDrawableSizeRight - rightOffset,
                     mScrollY + compoundPaddingTop + (vspace - dr->mDrawableHeightRight) / 2);
            dr->mShowing[Drawables::RIGHT]->draw(canvas);
            canvas.restore();
        }

        // IMPORTANT: The coordinates computed are also used in invalidateDrawable()
        // Make sure to update invalidateDrawable() when changing this code.
        if (dr->mShowing[Drawables::TOP] != nullptr) {
            canvas.save();
            canvas.translate(mScrollX + compoundPaddingLeft  + (hspace - dr->mDrawableWidthTop) / 2, mScrollY + mPaddingTop);
            dr->mShowing[Drawables::TOP]->draw(canvas);
            canvas.restore();
        }

        // IMPORTANT: The coordinates computed are also used in invalidateDrawable()
        // Make sure to update invalidateDrawable() when changing this code.
        if (dr->mShowing[Drawables::BOTTOM] != nullptr) {
            canvas.save();
            canvas.translate(mScrollX + compoundPaddingLeft + (hspace - dr->mDrawableWidthBottom) / 2,
                     mScrollY + getHeight() - mPaddingBottom - dr->mDrawableSizeBottom);
            dr->mShowing[Drawables::BOTTOM]->draw(canvas);
            canvas.restore();
        }
    }
    // Text
    int extendedPaddingTop = getExtendedPaddingTop();
    int extendedPaddingBottom = getExtendedPaddingBottom();

    const int vspace = getHeight() - compoundPaddingBottom - compoundPaddingTop;
    const int maxScrollY = 0;//mLayout.getHeight() - vspace;

    int clipLeft  = compoundPaddingLeft + mScrollX;
    int clipTop   = (mScrollY == 0) ? 0 : extendedPaddingTop + mScrollY;
    int clipRight = getWidth() - compoundPaddingRight+ mScrollX;
    int clipBottom= getHeight() + mScrollY - ((mScrollY == maxScrollY) ? 0 : extendedPaddingBottom);
    LOGV_IF(dr!=nullptr,"%p rect=%d,%d-%d,%d ==>%d,%d-%d,%d paddings=%d,%d,%d,%d",this,
                  rect.x,rect.y,rect.width,rect.height, clipLeft, clipTop, clipRight-clipLeft, clipBottom-clipTop,
                compoundPaddingLeft,compoundPaddingTop,compoundPaddingRight,compoundPaddingBottom);


    if (mShadowRadius != 0) {
        clipLeft += std::min(.0f, mShadowDx - mShadowRadius);
        clipRight += std::max(.0f, mShadowDx + mShadowRadius);

        clipTop += std::min(.0f, mShadowDy - mShadowRadius);
        clipBottom += std::max(.0f, mShadowDy + mShadowRadius);
    }
    int voffsetText = 0;
    int voffsetCursor = 0;

    // translate in by our padding
    /* shortcircuit calling getVerticaOffset() */
    LOGV("%p height=%d voffsetText=%d %s gravity=%x alignment=%x",this,mHeight,voffsetText,getText().c_str(),
           (mGravity & Gravity::VERTICAL_GRAVITY_MASK),getTextAlignment());
    if ((mGravity & Gravity::VERTICAL_GRAVITY_MASK) != Gravity::TOP) {
        voffsetText = getVerticalOffset(false);
        voffsetCursor = getVerticalOffset(true);
    }
    int color=mCurTextColor;
    Layout*layout=mLayout;
    if(getText().empty()&&mHint.length()){
        color=mCurHintTextColor;
        layout = mHintLayout;
        mHintLayout->relayout();
    }
    
    canvas.save();
    canvas.rectangle(clipLeft, clipTop, clipRight-clipLeft, clipBottom-clipTop);
    canvas.clip();

    canvas.translate(compoundPaddingLeft+offset, extendedPaddingTop + voffsetText);

    const int layoutDirection = getLayoutDirection();
    const int absoluteGravity = Gravity::getAbsoluteGravity(mGravity, layoutDirection);

    if (isMarqueeFadeEnabled()) {
        if (!mSingleLine && getLineCount() == 1 && canMarquee()
            && (absoluteGravity & Gravity::HORIZONTAL_GRAVITY_MASK) != Gravity::LEFT) {
            const int width = mWidth;
            const int padding = getCompoundPaddingLeft() + getCompoundPaddingRight();
            const float dx = layout->getLineRight(0) - (width - padding);
            canvas.translate(layout->getParagraphDirection(0) * dx, 0.0f);
        }

        if (mMarquee  && mMarquee->isRunning()) {
            float dx = -mMarquee->getScroll();
            canvas.translate(layout->getParagraphDirection(0) * dx, 0.0f);
        }
    }

    LOGV("text:%s maxlinewidth=%d lines=%d size=%dx%d marquee=%d mSingleLine=%d",getText().c_str(),
          layout->getMaxLineWidth(),layout->getLineCount(),clipRight-clipLeft, clipBottom-clipTop,
          isMarqueeFadeEnabled(),mSingleLine);
    canvas.set_color(color);
    layout->draw(canvas);
    mLayout->getCaretRect(mCaretRect);
    mCaretRect.offset(compoundPaddingLeft+offset, extendedPaddingTop + voffsetText);
    canvas.restore();
}

}  // namespace ui
