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
#include <widget/textview.h>
#include <cairomm/fontface.h>
#include <core/inputmethodmanager.h>
#include <core/app.h>
#include <text/layout.h>
#include <text/precomputedtext.h>
#include <core/textutils.h>
#include <porting/cdlog.h>
#include <float.h>

namespace cdroid{

DECLARE_WIDGET2(TextView,"cdroid:attr/textViewStyle")

TextView::Drawables::Drawables(Context*ctx){
    mIsRtlCompatibilityMode= false;
    mHasTintMode= mOverride = false;
    mTintList= nullptr;
    mShowing[0] = mShowing[1] = nullptr;
    mShowing[2] = mShowing[3] = nullptr;
    mDrawableStart = mDrawableEnd  = nullptr;
    mDrawableError = mDrawableTemp = nullptr;
    mDrawableLeftInitial= mDrawableRightInitial = nullptr;
    mDrawableSizeTop = mDrawableSizeBottom = mDrawableSizeLeft = 0;
    mDrawableSizeRight  = mDrawableSizeStart = mDrawableSizeEnd= 0;
    mDrawableSizeError  = mDrawableSizeTemp  = 0;
    mDrawableWidthTop   = mDrawableWidthBottom = mDrawableHeightLeft= 0;
    mDrawableHeightRight= mDrawableHeightStart = mDrawableHeightEnd = 0;
    mDrawableHeightError= mDrawableHeightTemp  = mDrawablePadding   = 0;
    mCompoundRect.set(0,0,0,0);
}

TextView::Drawables::~Drawables(){
    for(int i=0;i<4;i++){
        delete mShowing[i];
    }
}

bool TextView::Drawables::hasMetadata()const{
    return (mDrawablePadding != 0) || mHasTintMode || (mTintList!=nullptr);
}

bool TextView::Drawables::resolveWithLayoutDirection(int layoutDirection){
    Drawable* previousLeft = mShowing[Drawables::LEFT];
    Drawable* previousRight = mShowing[Drawables::RIGHT];

    // First reset "left" and "right" drawables to their initial values
    mShowing[Drawables::LEFT] = mDrawableLeftInitial;
    mShowing[Drawables::RIGHT] = mDrawableRightInitial;

    if (mIsRtlCompatibilityMode) {
        // Use "start" drawable as "left" drawable if the "left" drawable was not defined
        if (mDrawableStart && mShowing[Drawables::LEFT] == nullptr) {
            mShowing[Drawables::LEFT] = mDrawableStart;
            mDrawableSizeLeft = mDrawableSizeStart;
            mDrawableHeightLeft = mDrawableHeightStart;
        }
        // Use "end" drawable as "right" drawable if the "right" drawable was not defined
        if (mDrawableEnd  && mShowing[Drawables::RIGHT] == nullptr) {
            mShowing[Drawables::RIGHT] = mDrawableEnd;
            mDrawableSizeRight = mDrawableSizeEnd;
            mDrawableHeightRight = mDrawableHeightEnd;
        }
    } else {
        // JB-MR1+ normal case: "start" / "end" drawables are overriding "left" / "right"
        // drawable if and only if they have been defined
        switch(layoutDirection) {
        case LAYOUT_DIRECTION_RTL:
            if (mOverride) {
                mShowing[Drawables::RIGHT] = mDrawableStart;
                mDrawableSizeRight = mDrawableSizeStart;
                mDrawableHeightRight = mDrawableHeightStart;

                mShowing[Drawables::LEFT] = mDrawableEnd;
                mDrawableSizeLeft = mDrawableSizeEnd;
                mDrawableHeightLeft = mDrawableHeightEnd;
            }
            break;

        case LAYOUT_DIRECTION_LTR:
        default:
            if (mOverride) {
                mShowing[Drawables::LEFT] = mDrawableStart;
                mDrawableSizeLeft = mDrawableSizeStart;
                mDrawableHeightLeft = mDrawableHeightStart;

                mShowing[Drawables::RIGHT] = mDrawableEnd;
                mDrawableSizeRight = mDrawableSizeEnd;
                mDrawableHeightRight = mDrawableHeightEnd;
            }
            break;
        }
    }

    applyErrorDrawableIfNeeded(layoutDirection);

    return (mShowing[Drawables::LEFT] != previousLeft)
            || (mShowing[Drawables::RIGHT] != previousRight);
}

void TextView::Drawables::setErrorDrawable(Drawable* dr, TextView* tv) {
    if ((mDrawableError != dr) && (mDrawableError != nullptr)) {
        mDrawableError->setCallback(nullptr);
    }
    mDrawableError = dr;

    if (mDrawableError != nullptr) {
        Rect compoundRect = mCompoundRect;
        const std::vector<int> state = tv->getDrawableState();

        mDrawableError->setState(state);
        compoundRect = mDrawableError->getBounds();
        mDrawableError->setCallback(tv);
        mDrawableSizeError = compoundRect.width;
        mDrawableHeightError = compoundRect.height;
    } else {
        mDrawableSizeError = mDrawableHeightError = 0;
    }
}

void TextView::Drawables::applyErrorDrawableIfNeeded(int layoutDirection) {
    // first restore the initial state if needed
    switch (mDrawableSaved) {
    case DRAWABLE_LEFT:
        mShowing[Drawables::LEFT] = mDrawableTemp;
        mDrawableSizeLeft = mDrawableSizeTemp;
        mDrawableHeightLeft = mDrawableHeightTemp;
        break;
    case DRAWABLE_RIGHT:
        mShowing[Drawables::RIGHT] = mDrawableTemp;
        mDrawableSizeRight = mDrawableSizeTemp;
        mDrawableHeightRight = mDrawableHeightTemp;
        break;
    case DRAWABLE_NONE:
        default:break;
    }
    // then, if needed, assign the Error drawable to the correct location
    if (mDrawableError != nullptr) {
        switch(layoutDirection) {
        case LAYOUT_DIRECTION_RTL:
            mDrawableSaved = DRAWABLE_LEFT;

            mDrawableTemp = mShowing[Drawables::LEFT];
            mDrawableSizeTemp = mDrawableSizeLeft;
            mDrawableHeightTemp = mDrawableHeightLeft;

            mShowing[Drawables::LEFT] = mDrawableError;
            mDrawableSizeLeft = mDrawableSizeError;
            mDrawableHeightLeft = mDrawableHeightError;
            break;
        case LAYOUT_DIRECTION_LTR:
        default:
            mDrawableSaved = DRAWABLE_RIGHT;

            mDrawableTemp = mShowing[Drawables::RIGHT];
            mDrawableSizeTemp = mDrawableSizeRight;
            mDrawableHeightTemp = mDrawableHeightRight;

            mShowing[Drawables::RIGHT] = mDrawableError;
            mDrawableSizeRight = mDrawableSizeError;
            mDrawableHeightRight = mDrawableHeightError;
            break;
        }
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CharWrapper:public CharSequence{//, GetChars, GraphicsOperations {
private:
    std::vector<char16_t> mChars;
    int mStart, mLength;
public:
    CharWrapper(const std::vector<char16_t>&chars, int start, int len) {
        mChars = chars;
        mStart = start;
        mLength = len;
    }
    void set(const std::vector<char16_t>& chars, int start, int len) {
        mChars = chars;
        mStart = start;
        mLength = len;
    }
    size_t length() const override{
        return mLength;
    }
    int charAt(int off) const override{
        return mChars[off + mStart];
    }
    std::string toString() const{
        return "";//std::u16tring(mChars.data()+mStart, mLength);
    }
    CharSequence* subSequence(int start, int end) const override{
        if (start < 0 || end < 0 || start > mLength || end > mLength) {
            //throw new IndexOutOfBoundsException(start + ", " + end);
        }
        return nullptr;//new SpannedString(mChars, start + mStart, end - start);
    }
    void getChars(int start, int end, char16_t* buf, int off) const override{
        if (start < 0 || end < 0 || start > mLength || end > mLength) {
            //throw new IndexOutOfBoundsException(start + ", " + end);
        }
        memcpy(buf+off,mChars.data()+(mStart+start),(end-start)*2);
        //System.arraycopy(mChars, start + mStart, buf, off, end - start);
    }
    void drawText(Canvas& c, int start, int end, float x, float y, Paint& p) {
        p.drawTextRun(c,mChars.data(), start + mStart, end - start,0,0, x, y,false);
    }
    void drawTextRun(Canvas& c, int start, int end,
            int contextStart, int contextEnd, float x, float y, bool isRtl, Paint& p) {
        const int count = end - start;
        const int contextCount = contextEnd - contextStart;
        p.drawTextRun(c,mChars.data(), start + mStart, count, contextStart + mStart,
                contextCount, x, y, isRtl);
    }
    float measureText(int start, int end, Paint p) {
        return p.measureText(mChars.data(), start + mStart, end - start);
    }
    int getTextWidths(int start, int end, float* widths, Paint& p) {
        return 0;//p.getTextWidths(mChars.data(), start + mStart, end - start, widths);
    }
    float getTextRunAdvances(int start, int end, int contextStart, int contextEnd,
            bool isRtl, float* advances, int advancesIndex,Paint& p) {
        const int count = end - start;
        const int contextCount = contextEnd - contextStart;
        return p.getTextRunAdvances(mChars.data(), start + mStart, count,
                contextStart + mStart, contextCount, isRtl, advances,
                advancesIndex);
    }
    int getTextRunCursor(int contextStart, int contextEnd, bool isRtl,
            int offset, int cursorOpt, Paint& p) {
        int contextCount = contextEnd - contextStart;
        return p.getTextRunCursor(mChars.data(), contextStart + mStart,
                contextCount, isRtl, offset + mStart, cursorOpt);
    }
};

class Marquee {
private:
    static constexpr  float MARQUEE_DELTA_MAX = 0.07f;
    static constexpr  int MARQUEE_DELAY = 1200;
    static constexpr  int MARQUEE_DP_PER_SECOND = 30;

    static constexpr  int MARQUEE_STOPPED = 0x0;
    static constexpr  int MARQUEE_STARTING= 0x1;
    static constexpr  int MARQUEE_RUNNING = 0x2;
    friend TextView;
    TextView* mView;
    Layout*mLayout;
    Choreographer*mChoreographer;
    int mStatus ;//= MARQUEE_STOPPED;
    float mPixelsPerMs;
    float mMaxScroll;
    float mMaxFadeScroll;
    float mGhostStart;
    float mGhostOffset;
    float mFadeStop;
    int mRepeatLimit;

    float mScroll;
    int64_t mLastAnimationMs;
    Choreographer::FrameCallback mTickCallback;
    Choreographer::FrameCallback mStartCallback;
    Choreographer::FrameCallback mRestartCallback;

private:
    void resetScroll() {
        mScroll = 0.0f;
        if (mView ) mView->invalidate();
    }
public:
    Marquee(TextView* v,Layout*lt) {
        const float density = v->getContext()->getDisplayMetrics().density;
        mStatus = MARQUEE_STOPPED;
        mPixelsPerMs = (MARQUEE_DP_PER_SECOND * density) / 1000.f;
        mView = v;
        mLayout=lt;
        mChoreographer=&Choreographer::getInstance();
        mTickCallback = [this](int64_t) {tick();};
        mStartCallback= [this](int64_t) {
            mStatus = MARQUEE_RUNNING;
            mLastAnimationMs = mChoreographer->getFrameTime();
            tick();
        };
        mRestartCallback= [this](int64_t) {
            if (mStatus == MARQUEE_RUNNING) {
                if (mRepeatLimit >= 0) mRepeatLimit--;
                start(mRepeatLimit);
            }
        };
    }

    ~Marquee(){
        stop();
    }

    void tick() {
        if (mStatus != MARQUEE_RUNNING) {
            return;
        }

        mChoreographer->removeFrameCallback(mTickCallback);

        if (mView  && (mView->isFocused() || mView->isSelected())) {
            const int64_t currentMs = mChoreographer->getFrameTime();
            const int64_t deltaMs = currentMs - mLastAnimationMs;
            const float deltaPx = deltaMs * mPixelsPerMs;
            mLastAnimationMs = currentMs;
            mScroll += deltaPx;
            if (mScroll > mMaxScroll) {
                mScroll = mMaxScroll;
                mChoreographer->postFrameCallbackDelayed(mRestartCallback,MARQUEE_DELAY);
            } else {
                mChoreographer->postFrameCallback(mTickCallback);
            }
            mView->invalidate();
        }
    }

    void stop() {
        mStatus = MARQUEE_STOPPED;
        mChoreographer->removeFrameCallback(mStartCallback);
        mChoreographer->removeFrameCallback(mRestartCallback);
        mChoreographer->removeFrameCallback(mTickCallback);
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
            const int textWidth = mView->getWidth() - mView->getCompoundPaddingLeft()
                    - mView->getCompoundPaddingRight();
            const float lineWidth = mLayout->getLineWidth(0);
            const float gap = textWidth / 3.0f;
            mGhostStart = lineWidth - textWidth + gap;
            mMaxScroll = mGhostStart + textWidth;
            mGhostOffset = lineWidth + gap;
            mFadeStop = lineWidth + textWidth / 6.0f;
            mMaxFadeScroll = mGhostStart + lineWidth + lineWidth;
	
            mView->invalidate();
            mChoreographer->postFrameCallback(mStartCallback);
        }
    }
    float getGhostOffset()const { return mGhostOffset; }
    float getScroll()const { return mScroll; }
    float getMaxFadeScroll()const { return mMaxFadeScroll; }
    bool shouldDrawLeftFade()const { return mScroll <= mFadeStop; }
    bool shouldDrawGhost()const { return (mStatus == MARQUEE_RUNNING) && (mScroll > mGhostStart); }
    bool isRunning()const{ return mStatus == MARQUEE_RUNNING; }
    bool isStopped()const{ return mStatus == MARQUEE_STOPPED; }
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
class TextAppearanceAttributes{
public:
    int mTextColorHighlight = 0;
    cdroid::RefPtr<ColorStateList> mTextColor;
    cdroid::RefPtr<ColorStateList> mTextColorHint;
    cdroid::RefPtr<ColorStateList> mTextColorLink;
    int mTextSize = 0;
    std::string mFontFamily;
    Typeface* mFontTypeface;
    int mTypefaceIndex = -1;
    int mTextStyle=0;
    int mFontWeight = -1;
    int mShadowColor = 0;
    float mLetterSpacing = 0;
    float mShadowDx = 0, mShadowDy = 0, mShadowRadius = 0;
    bool mFontFamilyExplicit = false;
    bool mAllCaps = false;
    bool mHasElegant = false;
    bool mElegant = false;
    bool mHasFallbackLineSpacing = false;
    bool mFallbackLineSpacing = false;
    bool mHasLetterSpacing = false;
public:
    TextAppearanceAttributes();
    void readTextAppearance(Context*ctx,const AttributeSet&atts);
};

TextAppearanceAttributes::TextAppearanceAttributes(){
    mTextStyle = Typeface::NORMAL;
}

void TextAppearanceAttributes::readTextAppearance(Context*ctx,const AttributeSet&atts){
    if(atts.hasAttribute("textColorHighlight"))
        mTextColorHighlight = atts.getColor("textColorHighlight",mTextColorHighlight);

    mTextColor = atts.getColorStateList("textColor");
    mTextColorHint = atts.getColorStateList("textColorHint");
    mTextColorLink = atts.getColorStateList("textColorLink");
    mTextSize = atts.getDimensionPixelSize("textSize",mTextSize);
    mTextStyle= atts.getInt("textStyle",std::unordered_map<std::string,int>{
	   {"normal",(int)Typeface::NORMAL},
	   {"bold"  ,(int)Typeface::BOLD},
	   {"italic",(int)Typeface::ITALIC}
	},Typeface::NORMAL);
    mFontWeight  = atts.getInt("textfontWeight",-1);
    mShadowColor = atts.getColor("shadowColor",mShadowColor);
    mShadowDx = atts.getFloat("shadowDx",mShadowDx);
    mShadowDy = atts.getFloat("shadowDy",mShadowDy);
    mShadowRadius = atts.getFloat("shadowRadius",mShadowRadius);
    mTypefaceIndex= atts.getInt("typeface",-1);
    mFontFamily   = atts.getString("fontFamily","");
    mFontTypeface = Typeface::create(mFontFamily,mTextStyle);
    mAllCaps   = atts.getBoolean("textAllCaps",false);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

static BoringLayout::Metrics UNKNOWN_BORING;
TextView::TextView(Context*ctx,const AttributeSet& attrs)
  :View(ctx,attrs){
    initView();

    setText(ctx->getString(attrs.getString("text")));
    setHint(ctx->getString(attrs.getString("hint")));
    setHorizontallyScrolling(attrs.getBoolean("scrollHorizontally",mHorizontallyScrolling));

    Drawable* left = attrs.getDrawable("drawableLeft");
    Drawable*right = attrs.getDrawable("drawableRight");
    Drawable*  top = attrs.getDrawable("drawableTop");
    Drawable*bottom= attrs.getDrawable("drawableBottom");
    Drawable*start = attrs.getDrawable("drawableStart");
    Drawable*  end = attrs.getDrawable("drawableEnd");

    setCompoundDrawablesWithIntrinsicBounds(left,top,right,bottom);
    if(mDrawables){
        mDrawables->mTintList = attrs.getColorStateList("drawableTint");
    }
    applyCompoundDrawableTint();
    setRelativeDrawablesIfNeeded(start, end);

    setCompoundDrawablePadding(attrs.getDimensionPixelSize("drawablePadding",0));
    setMaxLines(attrs.getInt("maxLines",-1));
    setMinLines(attrs.getInt("minLines",-1));
    setLines(attrs.getInt("lines",-1));
    setHeight(attrs.getDimensionPixelSize("height",-1));
    setMinHeight(attrs.getDimensionPixelSize("minHeight", -1));
    setMaxHeight(attrs.getDimensionPixelSize("maxHeight", mMaximum));
    setTextScaleX(attrs.getFloat("textScaleX",1.f));

    setMinWidth(attrs.getDimensionPixelSize("minWidth", INT_MIN));
    setMaxWidth(attrs.getDimensionPixelSize("maxWidth", INT_MAX));
    setSingleLine(attrs.getBoolean("singleLine",mSingleLine));
    setGravity(attrs.getGravity("gravity",Gravity::TOP|Gravity::START));
    mMaxLength = attrs.getInt("maxLength",-1);

    setLineSpacing( attrs.getDimensionPixelSize("lineSpacingExtra",0),
             attrs.getFloat("lineSpacingMultiplier",1.f) );
    const int breakStrategy = attrs.getInt("breakStrategy",std::unordered_map<std::string,int>{
        {"simple"  ,(int)Layout::BREAK_STRATEGY_SIMPLE},
        {"balanced",(int)Layout::BREAK_STRATEGY_BALANCED},
        {"high_quality",(int)Layout::BREAK_STRATEGY_HIGH_QUALITY},
      },Layout::BREAK_STRATEGY_SIMPLE);
    setBreakStrategy(breakStrategy);

    TextAppearanceAttributes attributes;
    const std::string appearance = attrs.getString("textAppearance");
    if(appearance.empty()==false){
        AttributeSet tmp = attrs;
        AttributeSet attrs2 = ctx->obtainStyledAttributes(appearance);
        tmp.inherit(attrs2);
        attributes.readTextAppearance(ctx,tmp);
    }else{
        attributes.readTextAppearance(ctx,attrs);
    }
    applyTextAppearance(&attributes);
    setMarqueeRepeatLimit(attrs.getInt("marqueeRepeatLimit",std::unordered_map<std::string,int>{
            {"marquee_forever",-1}
        },mMarqueeRepeatLimit));
    setEllipsize(static_cast<TextUtils::TruncateAt>(attrs.getInt("ellipsize",std::unordered_map<std::string,int>{
        {"start",TextUtils::TruncateAt::START},{"middle",TextUtils::TruncateAt::MIDDLE},
        {"end" ,TextUtils::TruncateAt::END},{"marquee",TextUtils::TruncateAt::MARQUEE}
      },TextUtils::TruncateAt::NONE)));
    // If not explicitly specified this view is important for accessibility.
    if (getImportantForAccessibility() == IMPORTANT_FOR_ACCESSIBILITY_AUTO) {
        setImportantForAccessibility(IMPORTANT_FOR_ACCESSIBILITY_YES);
    }

    const int lineHeight = attrs.getDimensionPixelSize("lineHeight",-1);
    const int firstBaselineToTopHeight = attrs.getDimensionPixelSize("firstBaselineToTopHeight",-1);
    const int lastBaselineToBottomHeight = attrs.getDimensionPixelSize("lastBaselineToBottomHeight", -1);
    if (firstBaselineToTopHeight >= 0) setFirstBaselineToTopHeight(firstBaselineToTopHeight);
    if (lastBaselineToBottomHeight >= 0) setLastBaselineToBottomHeight(lastBaselineToBottomHeight);
    if(lineHeight>=0) setLineHeight(lineHeight);
}

TextView::TextView(int width, int height):TextView(std::string(),width,height){
}

TextView::TextView(const std::string& text, int width, int height)
  : View( width, height) {
    initView();
    mText=new SpannedString(TextUtils::utf8_utf16(text));
    mTransformed=mText;
    //mHintLayout->setWidth(width);
    //mLayout->setWidth(width);
    //mLayout->setText(text);
}

void TextView::initView(){
    mDrawables= nullptr;
    mMarquee  = nullptr;
    mScroller = nullptr;
    mCursorDrawable = nullptr;
    mSavedMarqueeModeLayout=nullptr;
    mMaxWidth = INT_MAX;
    mMinWidth = 0;
    mMaximum  = INT_MAX;
    mMinimum  = 0;
    mSpacingMult= 1.0;
    mSpacingAdd = 0.f;
    mRestartMarquee = true;
    mCaretPos = 0;
    mCaretRect.set(0,0,0,0);
    mMaxWidthMode = PIXELS;
    mMinWidthMode = PIXELS;
    mMaxMode = LINES;
    mMinMode = LINES;
    mDeferScroll = -1;
    mMaxLength= -1;
    mBlinkOn  = false;
    mIncludePad = true;
    mSingleLine = false;
    mMarqueeRepeatLimit =3;
    mText = nullptr;
    mHint = nullptr;
    mTransformed = nullptr;
    mTextDir= nullptr;
    mLayout = nullptr;
    mSpannable = nullptr;
    mHintLayout= nullptr;
    mSavedLayout = nullptr;
    mTransformation = nullptr;
    mSavedMarqueeModeLayout = nullptr;
    mBoring = mHintBoring =nullptr;
    mLastLayoutDirection = -1;
    mFontWeightAdjustment= 16;//INT_MAX;
    mMarqueeFadeMode = MARQUEE_FADE_NORMAL;
    mHorizontallyScrolling =false;
    mNeedsAutoSizeText = false;
    mUserSetTextScaleX = false;
    mPreDrawRegistered=false;
    mPreDrawListenerDetached=true;
    mEllipsize = TextUtils::TruncateAt::NONE;
    mAutoSizeTextType = AUTO_SIZE_TEXT_TYPE_NONE;
    //mLayout = new Layout(18,1);
    //mHintLayout = new Layout(mLayout->getFontSize(),1);
    //mLayout->setMultiline(!mSingleLine);
    //mHintLayout->setMultiline(!mSingleLine);
    mGravity = Gravity::START|Gravity::TOP;
    mTextColor = mHintTextColor = mLinkTextColor =nullptr;
    mHighlightColor= 0x6633B5E5;
    mShadowRadius = .0;
    mShadowDx = 0.0f;
    mShadowDy = 0.0f;
    mShadowColor = 0;
    mCurTextColor= mCurHintTextColor=0;
    mEditMode   = READONLY;
    setTextColor(0xFFFFFFFF);
    setHintTextColor(0xFFFFFFFF);
}

TextView::~TextView() {
    //delete mTextColor;
    //delete mHintTextColor;
    //delete mLinkTextColor;
    if(mTransformed!=mText)
        delete mTransformed;
    delete mHint;
    delete mText;
    delete mMarquee;
    delete mScroller;
    delete mLayout;
    delete mHintLayout;
    delete mDrawables;
    delete mCursorDrawable;
}

void TextView::setTextInternal(CharSequence* text){
    mText =text;
    mSpannable = dynamic_cast<Spannable*>(text);
    mPrecomputed = dynamic_cast<PrecomputedText*>(text);
}

void TextView::setAutoSizeTextTypeWithDefaults(int autoSizeTextType) {
    const DisplayMetrics displayMetrics = mContext->getDisplayMetrics();
    float autoSizeMinTextSizeInPx,autoSizeMaxTextSizeInPx;
    if (supportsAutoSizeText()) {
         switch (autoSizeTextType) {
         case AUTO_SIZE_TEXT_TYPE_NONE:
             clearAutoSizeConfiguration();
             break;
         case AUTO_SIZE_TEXT_TYPE_UNIFORM:
             autoSizeMinTextSizeInPx =DEFAULT_AUTO_SIZE_MIN_TEXT_SIZE_IN_SP;
                 //TypedValue.applyDimension( TypedValue.COMPLEX_UNIT_SP,
                 //    DEFAULT_AUTO_SIZE_MIN_TEXT_SIZE_IN_SP, displayMetrics);
             autoSizeMaxTextSizeInPx = DEFAULT_AUTO_SIZE_MAX_TEXT_SIZE_IN_SP;
                 //TypedValue.applyDimension( TypedValue.COMPLEX_UNIT_SP,
                 //    DEFAULT_AUTO_SIZE_MAX_TEXT_SIZE_IN_SP, displayMetrics);

             validateAndSetAutoSizeTextTypeUniformConfiguration(
                     autoSizeMinTextSizeInPx, autoSizeMaxTextSizeInPx,
                     DEFAULT_AUTO_SIZE_GRANULARITY_IN_PX);
             if (setupAutoSizeText()) {
                 autoSizeText();
                 invalidate();
             }
             break;
         default:
             LOGE("Unknown auto-size text type: %d",autoSizeTextType);
         }
    }
}

void TextView::setAutoSizeTextTypeUniformWithConfiguration(int autoSizeMinTextSize,
        int autoSizeMaxTextSize, int autoSizeStepGranularity, int unit) {
    if (supportsAutoSizeText()) {
        const DisplayMetrics displayMetrics = mContext->getDisplayMetrics();
        const float autoSizeMinTextSizeInPx = autoSizeMinTextSize;//TypedValue.applyDimension(unit, autoSizeMinTextSize, displayMetrics);
        const float autoSizeMaxTextSizeInPx = autoSizeMaxTextSize;//TypedValue.applyDimension(unit, autoSizeMaxTextSize, displayMetrics);
        const float autoSizeStepGranularityInPx = autoSizeStepGranularity;//TypedValue.applyDimension( unit, autoSizeStepGranularity, displayMetrics);

        validateAndSetAutoSizeTextTypeUniformConfiguration(autoSizeMinTextSizeInPx,
                autoSizeMaxTextSizeInPx, autoSizeStepGranularityInPx);

        if (setupAutoSizeText()) {
            autoSizeText();
            invalidate();
        }
    }
}

void TextView::setAutoSizeTextTypeUniformWithPresetSizes(const std::vector<int>& presetSizes, int unit){
    if (supportsAutoSizeText()) {
        const int presetSizesLength = presetSizes.size();
        if (presetSizesLength > 0) {
            std::vector<int> presetSizesInPx(presetSizesLength);

            if (unit == TypedValue::COMPLEX_UNIT_PX) {
                presetSizesInPx = presetSizes;
            } else {
                const DisplayMetrics displayMetrics = mContext->getDisplayMetrics();
                // Convert all to sizes to pixels.
                for (int i = 0; i < presetSizesLength; i++) {
                    presetSizesInPx[i] = presetSizes[i];
                        //std::round(TypedValue::applyDimension(unit,presetSizes[i], displayMetrics));
                }
            }

            mAutoSizeTextSizesInPx = cleanupAutoSizePresetSizes(presetSizesInPx);
            if (!setupAutoSizeUniformPresetSizesConfiguration()) {
                FATAL("None of the preset sizes is valid: ");// + Arrays.toString(presetSizes));
            }
        } else {
            mHasPresetAutoSizeValues = false;
        }

        if (setupAutoSizeText()) {
            autoSizeText();
            invalidate();
        }
    }
}

int TextView::getAutoSizeTextType() const{
    return mAutoSizeTextType;
}
int TextView::getAutoSizeStepGranularity() const{
    return std::round(mAutoSizeStepGranularityInPx);
}
int TextView::getAutoSizeMinTextSize() const{
    return std::round(mAutoSizeMinTextSizeInPx);
}
int TextView::getAutoSizeMaxTextSize() const{
    return std::round(mAutoSizeMaxTextSizeInPx);
}
std::vector<int> TextView::getAutoSizeTextAvailableSizes() const{
    return mAutoSizeTextSizesInPx;
}
bool TextView::setupAutoSizeUniformPresetSizesConfiguration() {
    const int sizesLength = mAutoSizeTextSizesInPx.size();
    mHasPresetAutoSizeValues = sizesLength > 0;
    if (mHasPresetAutoSizeValues) {
        mAutoSizeTextType = AUTO_SIZE_TEXT_TYPE_UNIFORM;
        mAutoSizeMinTextSizeInPx = mAutoSizeTextSizesInPx[0];
        mAutoSizeMaxTextSizeInPx = mAutoSizeTextSizesInPx[sizesLength - 1];
        mAutoSizeStepGranularityInPx = UNSET_AUTO_SIZE_UNIFORM_CONFIGURATION_VALUE;
    }
    return mHasPresetAutoSizeValues;
}
void TextView::validateAndSetAutoSizeTextTypeUniformConfiguration(float autoSizeMinTextSizeInPx,
         float autoSizeMaxTextSizeInPx, float autoSizeStepGranularityInPx){
    // First validate.
    if (autoSizeMinTextSizeInPx <= 0) {
        FATAL("Minimum auto-size text size (%dpx) is less or equal to (0px)",autoSizeMinTextSizeInPx);
    }

    if (autoSizeMaxTextSizeInPx <= autoSizeMinTextSizeInPx) {
        FATAL("Maximum auto-size text size (%dpx) is less or equal to minimum auto-size text size (%dpx)",
                autoSizeMaxTextSizeInPx,autoSizeMinTextSizeInPx);
    }

    if (autoSizeStepGranularityInPx <= 0) {
        FATAL("The auto-size step granularity (%d px) is less or equal to (0px)",autoSizeStepGranularityInPx);
    }

    // All good, persist the configuration.
    mAutoSizeTextType = AUTO_SIZE_TEXT_TYPE_UNIFORM;
    mAutoSizeMinTextSizeInPx = autoSizeMinTextSizeInPx;
    mAutoSizeMaxTextSizeInPx = autoSizeMaxTextSizeInPx;
    mAutoSizeStepGranularityInPx = autoSizeStepGranularityInPx;
    mHasPresetAutoSizeValues = false;
}
void TextView::clearAutoSizeConfiguration() {
    mAutoSizeTextType = AUTO_SIZE_TEXT_TYPE_NONE;
    mAutoSizeMinTextSizeInPx = UNSET_AUTO_SIZE_UNIFORM_CONFIGURATION_VALUE;
    mAutoSizeMaxTextSizeInPx = UNSET_AUTO_SIZE_UNIFORM_CONFIGURATION_VALUE;
    mAutoSizeStepGranularityInPx = UNSET_AUTO_SIZE_UNIFORM_CONFIGURATION_VALUE;
    mAutoSizeTextSizesInPx.clear();// = EmptyArray.INT;
    mNeedsAutoSizeText = false;
}

std::vector<int> TextView::cleanupAutoSizePresetSizes(std::vector<int>&presetValues){
    const int presetValuesLength = presetValues.size();
    if (presetValuesLength == 0) {
        return presetValues;
    }
    std::sort(presetValues.begin(),presetValues.end());

    std::vector<int>uniqueValidSizes;
    for (int i = 0; i < presetValuesLength; i++) {
        const int currentPresetValue = presetValues[i];

        if (currentPresetValue > 0
                && std::binary_search(uniqueValidSizes.begin(),uniqueValidSizes.end(),currentPresetValue)==false) {
            uniqueValidSizes.push_back(currentPresetValue);
        }
    }

    return presetValuesLength == uniqueValidSizes.size()
        ? presetValues
        : uniqueValidSizes;
}

bool TextView::setupAutoSizeText() {
    if (supportsAutoSizeText() && mAutoSizeTextType == AUTO_SIZE_TEXT_TYPE_UNIFORM) {
        // Calculate the sizes set based on minimum size, maximum size and step size if we do
        // not have a predefined set of sizes or if the current sizes array is empty.
        if (!mHasPresetAutoSizeValues || mAutoSizeTextSizesInPx.size() == 0) {
            const int autoSizeValuesLength = ((int) std::floor((mAutoSizeMaxTextSizeInPx
                    - mAutoSizeMinTextSizeInPx) / mAutoSizeStepGranularityInPx)) + 1;
            std::vector<int> autoSizeTextSizesInPx(autoSizeValuesLength);
            for (int i = 0; i < autoSizeValuesLength; i++) {
                autoSizeTextSizesInPx[i] = std::round(
                        mAutoSizeMinTextSizeInPx + (i * mAutoSizeStepGranularityInPx));
            }
            mAutoSizeTextSizesInPx = cleanupAutoSizePresetSizes(autoSizeTextSizesInPx);
        }

        mNeedsAutoSizeText = true;
    } else {
        mNeedsAutoSizeText = false;
    }

    return mNeedsAutoSizeText;
}

void TextView::setTypefaceFromAttrs(Typeface* typeface,const std::string& familyName,
       int typefaceIndex,int style,int weight){
    if ((typeface == nullptr) && (familyName.empty()==false)) {
         // Lookup normal Typeface from system font map.
         Typeface* normalTypeface = Typeface::create(familyName, Typeface::NORMAL);
         resolveStyleAndSetTypeface(normalTypeface, style, weight);
     } else if (typeface != nullptr) {
         resolveStyleAndSetTypeface(typeface, style, weight);
     } else {// both typeface and familyName is null.
         switch (typefaceIndex) {
         case SANS:  resolveStyleAndSetTypeface(Typeface::SANS_SERIF, style, weight); break;
         case SERIF: resolveStyleAndSetTypeface(Typeface::SERIF, style, weight); break;
         case MONOSPACE:  resolveStyleAndSetTypeface(Typeface::MONOSPACE, style, weight);  break;
         case DEFAULT_TYPEFACE:
         default: resolveStyleAndSetTypeface(nullptr, style, weight);  break;
        }
    }
}

void TextView::resolveStyleAndSetTypeface(Typeface* typeface,int style,int weight){
    if (weight >= 0) {
        weight = std::min((int)FontStyle::FONT_WEIGHT_MAX, weight);
        const bool italic = (style & Typeface::ITALIC) != 0;
        setTypeface(Typeface::create(typeface, weight, italic));
    } else {
        setTypeface(typeface, style);
    }
}

void TextView::setRelativeDrawablesIfNeeded(Drawable* start, Drawable* end) {
    if (start||end) {
        Drawables* dr = mDrawables;
        if (dr == nullptr) {
            mDrawables = dr = new Drawables(getContext());
        }
        mDrawables->mOverride = true;
        Rect compoundRect = dr->mCompoundRect;
        std::vector<int> state = getDrawableState();
        if (start) {
            start->setBounds(0, 0, start->getIntrinsicWidth(), start->getIntrinsicHeight());
            start->setState(state);
            compoundRect = start->getBounds();
            start->setCallback(this);

            dr->mDrawableStart = start;
            dr->mDrawableSizeStart = compoundRect.width;
            dr->mDrawableHeightStart = compoundRect.height;
        } else {
            dr->mDrawableSizeStart = dr->mDrawableHeightStart = 0;
        }
        if (end){
            end->setBounds(0, 0, end->getIntrinsicWidth(), end->getIntrinsicHeight());
            end->setState(state);
            compoundRect = end->getBounds();
            end->setCallback(this);

            dr->mDrawableEnd = end;
            dr->mDrawableSizeEnd = compoundRect.width;
            dr->mDrawableHeightEnd = compoundRect.height;
        } else {
            dr->mDrawableSizeEnd = dr->mDrawableHeightEnd = 0;
        }
        resetResolvedDrawables();
        resolveDrawables();
        applyCompoundDrawableTint();
    }
}

void TextView::setEnabled(bool _enabled) {
    if (_enabled == isEnabled()) {
        return;
    }
    /*if (!_enabled) {
        // Hide the soft input if the currently active TextView is disabled
        InputMethodManager imm = getInputMethodManager();
        if (imm != nullptr && imm.isActive(this)) {
            imm.hideSoftInputFromWindow(getWindowToken(), 0);
        }
    }*/
    View::setEnabled(_enabled);
    /*if (_enabled) {
        // Make sure IME is updated with current editor info.
        InputMethodManager imm = getInputMethodManager();
        if (imm != nullptr) imm->restartInput(this);
    }

    // Will change text color
    if (mEditor != nullptr) {
        mEditor.invalidateTextDisplayList();
        mEditor.prepareCursorControllers();

        // start or stop the cursor blinking as appropriate
        mEditor.makeBlink();
    }*/
}

void TextView::setTypeface(Typeface* tf,int style){
    if (style > 0) {
        if (tf == nullptr) {
            tf = Typeface::defaultFromStyle(style);
        } else {
            tf = Typeface::create(tf, style);
        }
        setTypeface(tf);
        // now compute what (if any) algorithmic styling is needed
        const int typefaceStyle = tf ? tf->getStyle() : 0;
        const int need = style & ~typefaceStyle;
        mTextPaint.setFakeBoldText((need & Typeface::ITALIC) != 0 ? -0.25:0.0);
        mTextPaint.setTextSkewX((need & Typeface::ITALIC) != 0 ? -0.25:0.0);
    } else {
        mTextPaint.setFakeBoldText(false);
        mTextPaint.setTextSkewX(0.0);
        setTypeface(tf);
    }
}

void TextView::registerForPreDraw() {
    if (!mPreDrawRegistered) {
        getViewTreeObserver()->addOnPreDrawListener(mOnPreDrawListener);
        mPreDrawRegistered = true;
    }
}

void TextView::unregisterForPreDraw() {
    getViewTreeObserver()->removeOnPreDrawListener(mOnPreDrawListener);
    mPreDrawRegistered = false;
    mPreDrawListenerDetached = false;
}

bool TextView::onPreDraw() {
    if (mLayout == nullptr) {
        assumeLayout();
    }

    /*if (mMovement != nullptr) {
        int curs = getSelectionEnd();
        // Do not create the controller if it is not already created.
        if (mEditor != null && mEditor.mSelectionModifierCursorController != null
                && mEditor.mSelectionModifierCursorController.isSelectionStartDragged()) {
            curs = getSelectionStart();
        }
        if (curs < 0 && (mGravity & Gravity::VERTICAL_GRAVITY_MASK) == Gravity::BOTTOM) {
            curs = mText->length();
        }
        if (curs >= 0) {
            bringPointIntoView(curs);
        }
    } else */{
        bringTextIntoView();
    }

    // This has to be checked here since:
    // - onFocusChanged cannot start it when focus is given to a view with selected text (after
    //   a screen rotation) since layout is not yet initialized at that point.
    /*if (mEditor != null && mEditor.mCreatedWithASelection) {
        mEditor.refreshTextActionMode();
        mEditor.mCreatedWithASelection = false;
    }*/
    unregisterForPreDraw();
    return true;
}

void TextView::onAttachedToWindow() {
    View::onAttachedToWindow();
    //if (mEditor != null) mEditor.onAttachedToWindow();
    if(mOnPreDrawListener==nullptr){
        mOnPreDrawListener=[this](){return onPreDraw();};
    }
    if (mPreDrawListenerDetached) {
        getViewTreeObserver()->addOnPreDrawListener(mOnPreDrawListener);
        mPreDrawListenerDetached = false;
    }
}

void TextView::onDetachedFromWindowInternal(){
    if (mPreDrawRegistered) {
        getViewTreeObserver()->removeOnPreDrawListener(mOnPreDrawListener);
        mPreDrawListenerDetached = true;
    }
    stopMarquee();
    for(int i = 0; mDrawables && ( i<4 );i++){
        Drawable*d = mDrawables->mShowing[i];
        if( d == nullptr)continue;
        unscheduleDrawable(*d);
    }
    View::onDetachedFromWindowInternal();
}

Layout::Alignment TextView::getLayoutAlignment()const{
    Layout::Alignment alignment;
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

void TextView::applyTextAppearance(class TextAppearanceAttributes *attr){
    if (attr->mTextColor)setTextColor(attr->mTextColor);

    if (attr->mTextColorHint)setHintTextColor(attr->mTextColorHint);

    if (attr->mTextColorLink)setLinkTextColor(attr->mTextColorLink);

    if (attr->mTextColorHighlight) setHighlightColor(attr->mTextColorHighlight);

    if (attr->mTextSize != 0) setRawTextSize(attr->mTextSize, true /* shouldRequestLayout */);

    if ((attr->mTypefaceIndex != -1) && !attr->mFontFamilyExplicit) {
        attr->mFontFamily.clear();
    }
    setTypefaceFromAttrs(attr->mFontTypeface, attr->mFontFamily,
            attr->mTypefaceIndex, attr->mTextStyle, attr->mFontWeight);

    if (attr->mShadowColor != 0) {
        setShadowLayer(attr->mShadowRadius, attr->mShadowDx, attr->mShadowDy, attr->mShadowColor);
    }

    /*if (attr->mAllCaps) setTransformationMethod(new AllCapsTransformationMethod(getContext()));

    if (attr->mHasElegant) setElegantTextHeight(attr->mElegant);

    if (attr->mHasFallbackLineSpacing) {
        setFallbackLineSpacing(attr->mFallbackLineSpacing);
    }

    if (attr->mHasLetterSpacing) {
        setLetterSpacing(attr.mLetterSpacing);
    }

    if (attr->mFontFeatureSettings != null) {
        setFontFeatureSettings(attr->mFontFeatureSettings);
    }*/
}

void TextView::addTextChangedListener(const TextWatcher& watcher){
    auto it = std::find(mListeners.begin(),mListeners.end(),watcher);
    if(it==mListeners.end()){
        mListeners.push_back(watcher);
    }
}

void TextView::removeTextChangedListener(const TextWatcher& watcher){
    auto it = std::find(mListeners.begin(),mListeners.end(),watcher);
    if( it !=mListeners.end()){
	    mListeners.erase(it);
    }
}

void TextView::sendBeforeTextChanged(const std::wstring& text, int start, int before, int after){
    for(auto l:mListeners){
        if(l.beforeTextChanged) l.beforeTextChanged(text, start, before, after);
    }
}

void TextView::sendAfterTextChanged(std::wstring& text){
    for (auto l:mListeners) {
        if(l.afterTextChanged){
            l.afterTextChanged(*this,text);
        }
    }

    // Always notify AutoFillManager - it will return right away if autofill is disabled.
    //notifyAutoFillManagerAfterTextChangedIfNeeded();
    //hideErrorIfUnchanged();
}
void TextView::sendOnTextChanged(const std::wstring& text, int start, int before, int after){
    for(auto l:mListeners){
        if(l.onTextChanged){
            l.onTextChanged(text, start, before, after);
        }
    }
    //if (mEditor != null) mEditor.sendOnTextChanged(start, before, after);
}

void TextView::setRawTextSize(float size, bool shouldRequestLayout){
    if(size != mTextPaint.getTextSize()){
        mTextPaint.setTextSize(size);
        if (shouldRequestLayout && mLayout != nullptr) {
            // Do not auto-size right after setting the text size.
            mNeedsAutoSizeText = false;
            nullLayouts();
            requestLayout();
            invalidate();
        }
    }
}

void TextView::setPadding(int left, int top, int right, int bottom){
    if ((left != mPaddingLeft) || (right != mPaddingRight)
            || (top != mPaddingTop) ||(bottom != mPaddingBottom)) {
        nullLayouts();
    }
    // the super call will requestLayout()
    View::setPadding(left, top, right, bottom);
    invalidate();
}

void TextView::setPaddingRelative(int start, int top, int end, int bottom){
    if ( (start != getPaddingStart()) || (end != getPaddingEnd())
            || (top != mPaddingTop) || (bottom != mPaddingBottom)) {
        nullLayouts();
    }

    // the super call will requestLayout()
    View::setPaddingRelative(start, top, end, bottom);
    invalidate();
}

void TextView::setFirstBaselineToTopHeight(int firstBaselineToTopHeight){
    const Paint::FontMetricsInt fontMetrics = getPaint().getFontMetricsInt();
    int fontMetricsTop;
    if (getIncludeFontPadding()) {
        fontMetricsTop = fontMetrics.top;
    } else {
        fontMetricsTop = fontMetrics.ascent;
    }
    // TODO: Decide if we want to ignore density ratio (i.e. when the user changes font size
    // in settings). At the moment, we don't.

    if (firstBaselineToTopHeight > std::abs(fontMetricsTop)) {
        const int paddingTop = firstBaselineToTopHeight - (-fontMetricsTop);
        setPadding(getPaddingLeft(), paddingTop, getPaddingRight(), getPaddingBottom());
    }
}

void TextView::setLastBaselineToBottomHeight(int lastBaselineToBottomHeight){
    const Paint::FontMetricsInt fontMetrics = getPaint().getFontMetricsInt();
    int fontMetricsBottom;
    if (getIncludeFontPadding()) {
        fontMetricsBottom = fontMetrics.bottom;
    } else {
        fontMetricsBottom = fontMetrics.descent;
    }

    // TODO: Decide if we want to ignore density ratio (i.e. when the user changes font size
    // in settings). At the moment, we don't.

    if (lastBaselineToBottomHeight > std::abs(fontMetricsBottom)) {
        const int paddingBottom = lastBaselineToBottomHeight - fontMetricsBottom;
        setPadding(getPaddingLeft(), getPaddingTop(), getPaddingRight(), paddingBottom);
    }
}

int TextView::getFirstBaselineToTopHeight(){
    return getPaddingTop() - getPaint().getFontMetricsInt().top;
}

int TextView::getLastBaselineToBottomHeight(){
    return getPaddingBottom() - getPaint().getFontMetricsInt().bottom;
}

void TextView::setTextCursorDrawable(Drawable*d){
    delete mCursorDrawable;
    mCursorDrawable = d;
}

Drawable* TextView::getTextCursorDrawable()const{
    return mCursorDrawable;
}
void TextView::setTextAppearance(Context*context,const std::string&appearance){
    TextAppearanceAttributes attributes;
    if(appearance.empty()==false){
        AttributeSet attrs = context->obtainStyledAttributes(appearance);
        if(attrs.getAttributeCount()){
            attributes.readTextAppearance(mContext,attrs);
            applyTextAppearance(&attributes);
        }
    }
}

void TextView::setTextAppearance(const std::string&appearance){
    setTextAppearance(mContext,appearance);
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
    return mTextPaint.getTextSize();
}

float TextView::getScaledTextSize() const{
    return mTextPaint.getTextSize();// / mTextPaint.density;
}

float TextView::getTextScaleX()const{
    return mTextPaint.getTextScaleX();
}

void TextView::setTextScaleX(float size){
    if( size!=mTextPaint.getTextScaleX() ){
        mUserSetTextScaleX = true;
        mTextPaint.setTextScaleX(size);
        mUserSetTextScaleX = true;
        if(mLayout!=nullptr){
            nullLayouts();
            requestLayout();
            invalidate();
        }
    }
}

void TextView::setElegantTextHeight(bool elegant) {
    if (elegant != mTextPaint.isElegantTextHeight()) {
        mTextPaint.setElegantTextHeight(elegant);
        if (mLayout != nullptr) {
            nullLayouts();
            requestLayout();
            invalidate();
        }
    }
}

bool TextView::isElegantTextHeight() const{
    return mTextPaint.isElegantTextHeight();
}

void TextView::setFallbackLineSpacing(bool enabled) {
    if (mUseFallbackLineSpacing != enabled) {
        mUseFallbackLineSpacing = enabled;
        if (mLayout != nullptr) {
            nullLayouts();
            requestLayout();
            invalidate();
        }
    }
}

bool TextView::isFallbackLineSpacing() const{
        return mUseFallbackLineSpacing;
}

float TextView::getLetterSpacing() const{
    return mTextPaint.getLetterSpacing();
}

void TextView::setLetterSpacing(float letterSpacing) {
    if (letterSpacing != mTextPaint.getLetterSpacing()) {
        mTextPaint.setLetterSpacing(letterSpacing);

        if (mLayout != nullptr) {
            nullLayouts();
            requestLayout();
            invalidate();
        }
    }
}

void TextView::setJustificationMode(int justificationMode) {
    mJustificationMode = justificationMode;
    if (mLayout != nullptr) {
        nullLayouts();
        requestLayout();
        invalidate();
    }
}
int TextView::getJustificationMode() const{
        return mJustificationMode;
}

int TextView::computeVerticalScrollRange(){
    if (mLayout != nullptr) {
        return mLayout->getHeight();
    }
    return View::computeVerticalScrollRange();
}

int TextView::computeHorizontalScrollRange(){
    if (mLayout != nullptr) {
            return mSingleLine && (mGravity & Gravity::HORIZONTAL_GRAVITY_MASK) == Gravity::LEFT
                    ? (int) mLayout->getLineWidth(0) : mLayout->getWidth();
    }
    return View::computeHorizontalScrollRange();
}

int TextView::getHorizontalOffsetForDrawables()const{
    return 0;
}

void TextView::setText(CharSequence* txt) {
    setTextInternal(txt);
    checkForRelayout();
    startStopMarquee(false);
    startStopMarquee(true);
    setText(txt->toString());
}

void TextView::append(CharSequence* text){
    append(text,0,text->length());
}

void TextView::append(CharSequence* text, int start, int end){
#if 0
    if (!(mText instanceof Editable)) {
            setText(mText, BufferType::EDITABLE);
    }

    ((Editable*) mText)->append(text, start, end);

    if (mAutoLinkMask != 0) {
        bool linksWereAdded = Linkify.addLinks(mSpannable, mAutoLinkMask);
        // Do not change the movement method for text that support text selection as it
        // would prevent an arbitrary cursor displacement.
        if (linksWereAdded && mLinksClickable && !textCanBeSelected()) {
            setMovementMethod(LinkMovementMethod.getInstance());
        }
    }
#endif
}

void TextView::setText(const std::string&txt){
    if(mText==nullptr)
        mText=new SpannableString(TextUtils::utf8_utf16(txt));
    if(mTransformed==nullptr)
        mTransformed=mText;
    /*if(mLayout->setText(txt) && (getVisibility()==View::VISIBLE) ){
        std::wstring&ws=getEditable();
        if(mCaretPos<ws.length())
            mCaretPos = int(ws.length()-1);
        checkForRelayout();
        startStopMarquee(false);
        startStopMarquee(true);
    }*/
}

void TextView::setText(CharSequence* text, TextView::BufferType type, bool notifyBefore, int oldlen){
#if 0
    mTextSetFromXmlOrResourceId = false;
    if (text == nullptr) {
        //text = "";
    }

    // If suggestions are not enabled, remove the suggestion spans from the text
    /*if (!isSuggestionsEnabled()) {
        text = removeSuggestionSpans(text);
    }*/

    if (!mUserSetTextScaleX) mTextPaint.setTextScaleX(1.0f);
    auto spannedText=dynamic_cast<Spanned*>(text);
    if (spannedText && spannedText->getSpanStart(TextUtils::TruncateAt::MARQUEE) >= 0) {
        if (ViewConfiguration::get(mContext)->isFadingMarqueeEnabled()) {
            setHorizontalFadingEdgeEnabled(true);
            mMarqueeFadeMode = MARQUEE_FADE_NORMAL;
        } else {
            setHorizontalFadingEdgeEnabled(false);
            mMarqueeFadeMode = MARQUEE_FADE_SWITCH_SHOW_ELLIPSIS;
        }
        setEllipsize(TextUtils::TruncateAt::MARQUEE);
    }

    /*int n = mFilters.length;
    for (int i = 0; i < n; i++) {
        CharSequence* out = mFilters[i].filter(text, 0, text.length(), EMPTY_SPANNED, 0, 0);
        if (out != nullptr) {
            text = out;
        }
    }*/

    if (notifyBefore) {
        if (mText != nullptr) {
            oldlen = mText->length();
            //sendBeforeTextChanged(mText, 0, oldlen, text.length());
        } else {
            //sendBeforeTextChanged("", 0, 0, text.length());
        }
    }

    bool needEditableForNotification = false;

    if (/*mListeners != nullptr &&*/ mListeners.size() != 0) {
        needEditableForNotification = true;
    }

    PrecomputedText* precomputed =dynamic_cast<PrecomputedText*>(text);
    if (type == BufferType::EDITABLE /*|| getKeyListener() != nullptr*/|| needEditableForNotification) {
        createEditorIfNeeded();
        //mEditor->forgetUndoRedo();
        //Editable* t = mEditableFactory.newEditable(text);
        //text = t;
        //setFilters(t, mFilters);
        //InputMethodManager* imm = getInputMethodManager();
        //if (imm != nullptr) imm->restartInput(this);
    } else if (precomputed != nullptr) {
        if (mTextDir == nullptr) {
            mTextDir = getTextDirectionHeuristic();
        }
        const int checkResult =
                precomputed.getParams().checkResultUsable(getPaint(), mTextDir, mBreakStrategy,
                        mHyphenationFrequency);
        switch (checkResult) {
        case PrecomputedText::Params::UNUSABLE:
            throw
                "PrecomputedText's Parameters don't match the parameters of this TextView."
                "Consider using setTextMetricsParams(precomputedText.getParams()) "
                "to override the settings of this TextView: "
                "PrecomputedText: ";// + precomputed.getParams()
                "TextView: ";// + getTextMetricsParams());
        case PrecomputedText::Params::NEED_RECOMPUTE:
            precomputed = PrecomputedText::create(precomputed, getTextMetricsParams());
            break;
        case PrecomputedText::Params::USABLE:/*pass through*/break;
        }
    } else if (type == BufferType::SPANNABLE || mMovement != nullptr) {
        text = mSpannableFactory.newSpannable(text);
    } else if (dynamic_cast<CharWrapper*>(text)!=nullptr) {
        text = TextUtils::stringOrSpannedString(text);
    }

    if (mAutoLinkMask != 0) {
        Spannable* s2;

        if (type == BufferType::EDITABLE || dynamic_cast<Spannable*>(text)) {
            s2 = (Spannable) text;
        } else {
            s2 = mSpannableFactory.newSpannable(text);
        }

        if (Linkify.addLinks(s2, mAutoLinkMask)) {
            text = s2;
            type = (type == BufferType::EDITABLE) ? BufferType::EDITABLE : BufferType::SPANNABLE;

            /*
             * We must go ahead and set the text before changing the
             * movement method, because setMovementMethod() may call
             * setText() again to try to upgrade the buffer type.
             */
            setTextInternal(text);

            // Do not change the movement method for text that support text selection as it
            // would prevent an arbitrary cursor displacement.
            if (mLinksClickable && !textCanBeSelected()) {
                //setMovementMethod(LinkMovementMethod.getInstance());
            }
        }
    }

    mBufferType = type;
    setTextInternal(text);

    /*if (mTransformation == nullptr) {
        mTransformed = text;
    } else {
        mTransformed = mTransformation.getTransformation(text, this);
    }
    if (mTransformed == nullptr) {
        // Should not happen if the transformation method follows the non-null postcondition.
        mTransformed = "";
    }*/

    const int textLength = text->length();

    if (dynamic_cast<Spannable*>(text) && !mAllowTransformationLengthChange) {
        Spannable* sp = (Spannable*) text;

        // Remove any ChangeWatchers that might have come from other TextViews.
        auto watchers = sp->getSpans(0, sp->length(), make_span_filter<ChangeWatcher>());
        const int count = watchers.size();
        for (int i = 0; i < count; i++) {
            sp->removeSpan(watchers[i]);
        }

        if (mChangeWatcher == nullptr) mChangeWatcher = new ChangeWatcher();

        sp->setSpan(mChangeWatcher, 0, textLength, Spanned::SPAN_INCLUSIVE_INCLUSIVE
                | (CHANGE_WATCHER_PRIORITY << Spanned::SPAN_PRIORITY_SHIFT));

        /*if (mEditor != nullptr) mEditor->addSpanWatchers(sp);
        if (mTransformation != nullptr) {
            sp->setSpan(mTransformation, 0, textLength, Spanned::SPAN_INCLUSIVE_INCLUSIVE);
        }
        if (mMovement != nullptr) {
            mMovement.initialize(this, (Spannable) text);
            if (mEditor != nullptr) mEditor->SelectionMoved = false;
        }*/
    }

    if (mLayout != nullptr) {
        checkForRelayout();
    }

    /*sendOnTextChanged(text, 0, oldlen, textLength);
    onTextChanged(text, 0, oldlen, textLength);

    notifyViewAccessibilityStateChangedIfNeeded(AccessibilityEvent.CONTENT_CHANGE_TYPE_TEXT);
    if (needEditableForNotification) {
        sendAfterTextChanged((Editable) text);
    } else {
        notifyListeningManagersAfterTextChanged();
    }
    SelectionModifierCursorController depends on textCanBeSelected, which depends on text
    if (mEditor != nullptr) mEditor->prepareCursorControllers();
    */
#endif
}

const std::string TextView::getText()const{
    return mText->toString();
}

void TextView::setHint(const std::string& hint){
    //mHint = hint;
    //mHintLayout->setText(hint);
    checkForRelayout();
}

std::string TextView::getHint()const{
    return mHint->toString();
}

std::wstring& TextView::getEditable(){
    static std::wstring ss;
    return ss;//mLayout->getText();
}

void TextView::setEditable(bool b){
    //mLayout->setEditable(b);
}

void TextView::setCaretPos(int pos){
    mCaretPos= pos;
    mBlinkOn = true;
    //mLayout->setCaretPos(pos);
    invalidate(true);
}

int TextView::getCaretPos()const{
    return mCaretPos;
}

bool TextView::moveCaret2Line(int line){
    int curline = mLayout->getLineForOffset(mCaretPos);
    int curcolumns = mCaretPos - mLayout->getLineStart(curline);
    if( (line<getLineCount()) && (line>=0) ){
        int newcolumns = mLayout->getLineEnd(line)-mLayout->getLineStart(line);
        newcolumns=std::min(newcolumns,curcolumns);
        setCaretPos(mLayout->getLineStart(line) + newcolumns);
        return true;
    }
    return false;
}

void TextView::setWidth(int pixels) {
    mMaxWidth = (mMinWidth = pixels);
    mMaxWidthMode = (mMinWidthMode = PIXELS);

    requestLayout();
    invalidate();
}

void TextView::setLineSpacing(float add, float mult) {
    if ((mSpacingAdd != add) || (mSpacingMult != mult)) {
        mSpacingAdd = add;
        mSpacingMult = mult;
        if (mLayout != nullptr) {
            nullLayouts();
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

void TextView::checkForRelayout() {
    // If we have a fixed width, we can just swap in a new text layout
    // if the text height stays the same or if the view height is fixed.
    if(mLayoutParams==nullptr) return;
    if (( (mLayoutParams->width != LayoutParams::WRAP_CONTENT)
            || (mMaxWidthMode == mMinWidthMode && mMaxWidth == mMinWidth))
            && ((mHint==nullptr)||(mHintLayout==nullptr) )
            && (mRight - mLeft - getCompoundPaddingLeft() - getCompoundPaddingRight() > 0)) {
        // Static width, so try making a new text layout.

        const int oldht = mLayout->getHeight();
        const int want = mLayout->getWidth();
        const int hintWant = mHintLayout ?mHintLayout->getWidth():0;

        /*
         * No need to bring the text into view, since the size is not
         * changing (unless we do the requestLayout(), in which case it
         * will happen at measure).
         */
        makeNewLayout(want, hintWant, &UNKNOWN_BORING, &UNKNOWN_BORING,
                      mRight - mLeft - getCompoundPaddingLeft() - getCompoundPaddingRight(),
                      false);
        if (mEllipsize != TextUtils::TruncateAt::MARQUEE) {
            // In a fixed-height view, so use our new text layout.
            if (mLayoutParams->height != LayoutParams::WRAP_CONTENT
                    && mLayoutParams->height != LayoutParams::MATCH_PARENT) {
                autoSizeText();
                invalidate();
                return;
            }

            // Dynamic height, but height has stayed the same,
            // so use our new text layout.
            if ( (mLayout->getHeight() == oldht) && ((mHintLayout==nullptr) || (mHintLayout->getHeight() == oldht))) {
                autoSizeText();
                invalidate();
                return;
            }
        }

        // We lose: the height has changed and we have a dynamic height.
        // Request a new view layout using our new text layout.
        requestLayout();
        invalidate();
    } else {
        // Dynamic width, so we have no choice but to request a new
        // view layout with a new text layout.
        nullLayouts();
        requestLayout();
        invalidate();
    }
}

bool TextView::isShowingHint()const{
    //mLayout->getText().empty()&&(mHintLayout->getText().empty()==false);
    return TextUtils::isEmpty(mText) && !TextUtils::isEmpty(mHint) && !mHideHint;
}

bool TextView::bringTextIntoView(){
    Layout* layout = isShowingHint() ? mHintLayout : mLayout;
    int line = 0;
    if ((mGravity & Gravity::VERTICAL_GRAVITY_MASK) == Gravity::BOTTOM) {
        line = layout->getLineCount() - 1;
    }

    int dir = layout->getParagraphDirection(line);
    int hspace = mRight - mLeft - getCompoundPaddingLeft() - getCompoundPaddingRight();
    int vspace = mBottom - mTop - getExtendedPaddingTop() - getExtendedPaddingBottom();
    int ht = layout->getHeight();
    int scrollx, scrolly;

    Layout::Alignment a = layout->getParagraphAlignment(line);

    // Convert to left, center, or right alignment.
    if (a == Layout::Alignment::ALIGN_NORMAL) {
        a = dir == Layout::DIR_LEFT_TO_RIGHT
                ? Layout::Alignment::ALIGN_LEFT : Layout::Alignment::ALIGN_RIGHT;
    } else if (a == Layout::Alignment::ALIGN_OPPOSITE) {
        a = dir == Layout::DIR_LEFT_TO_RIGHT
                ? Layout::Alignment::ALIGN_RIGHT : Layout::Alignment::ALIGN_LEFT;
    }

    if (a == Layout::Alignment::ALIGN_CENTER) {
        /*
         * Keep centered if possible, or, if it is too wide to fit,
         * keep leading edge in view.
         */

        int left = (int) std::floor(layout->getLineLeft(line));
        int right = (int) std::ceil(layout->getLineRight(line));
        if(mHorizontallyScrolling&&mLayout->getWidth()==VERY_WIDE){
            mLayout->increaseWidthTo(right-left);//this case added by zhhou
            left = (int) std::floor(layout->getLineLeft(line));
            right= (int) std::ceil(layout->getLineRight(line));
        }
        if (right - left < hspace) {
            scrollx = (right + left) / 2 - hspace / 2;
        } else {
            if (dir < 0) {
                scrollx = right - hspace;
            } else {
                scrollx = left;
            }
        }
    } else if (a == Layout::Alignment::ALIGN_RIGHT) {
        int right = (int) std::ceil(layout->getLineRight(line));
        scrollx = right - hspace;
    } else { // a == Layout.Alignment.ALIGN_LEFT (will also be the default)
        scrollx = (int) std::floor(layout->getLineLeft(line));
    }
    if (ht < vspace) {
        scrolly = 0;
    } else {
        if ((mGravity & Gravity::VERTICAL_GRAVITY_MASK) == Gravity::BOTTOM) {
            scrolly = ht - vspace;
        } else {
            scrolly = 0;
        }
    }

    if (scrollx != mScrollX || scrolly != mScrollY) {
        scrollTo(scrollx, scrolly);
        return true;
    } else {
        return false;
    }
}

bool TextView::bringPointIntoView(int offset) {
    if (isLayoutRequested()) {
        mDeferScroll = offset;
        return false;
    }
    bool changed = false;

    Layout* layout = isShowingHint() ? mHintLayout : mLayout;

    if (layout == nullptr) return changed;

    int line = layout->getLineForOffset(offset);

    int grav;
    switch (layout->getParagraphAlignment(line)) {
    case Layout::ALIGN_LEFT:
        grav = 1;   break;
    case Layout::ALIGN_RIGHT:
        grav = -1;  break;
    case Layout::ALIGN_NORMAL:
        grav = layout->getParagraphDirection(line);
        break;
    case Layout::ALIGN_OPPOSITE:
        grav = -layout->getParagraphDirection(line);
        break;
    case Layout::ALIGN_CENTER:
    default:   grav = 0;   break;
    }
    // We only want to clamp the cursor to fit within the layout width
    // in left-to-right modes, because in a right to left alignment,
    // we want to scroll to keep the line-right on the screen, as other
    // lines are likely to have text flush with the right margin, which
    // we want to keep visible.
    // A better long-term solution would probably be to measure both
    // the full line and a blank-trimmed version, and, for example, use
    // the latter measurement for centering and right alignment, but for
    // the time being we only implement the cursor clamping in left to
    // right where it is most likely to be annoying.
    const bool clamped = grav > 0;
    // FIXME: Is it okay to truncate this, or should we round?
    const int x = 0;// (int) layout->getPrimaryHorizontal(offset, clamped);
    const int top = layout->getLineTop(line);
    const int bottom = layout->getLineTop(line + 1);

    int left = (int) std::floor(layout->getLineLeft(line));
    int right = (int) std::ceil(layout->getLineRight(line));
    int ht = layout->getHeight();

    int hspace = mRight - mLeft - getCompoundPaddingLeft() - getCompoundPaddingRight();
    int vspace = mBottom - mTop - getExtendedPaddingTop() - getExtendedPaddingBottom();
    if (!mHorizontallyScrolling && right - left > hspace && right > x) {
        // If cursor has been clamped, make sure we don't scroll.
        right = std::max(x, left + hspace);
    }

    int hslack = (bottom - top) / 2;
    int vslack = hslack;

    if (vslack > vspace / 4) {
        vslack = vspace / 4;
    }
    if (hslack > hspace / 4) {
        hslack = hspace / 4;
    }

    int hs = mScrollX;
    int vs = mScrollY;

    if (top - vs < vslack) {
        vs = top - vslack;
    }
    if (bottom - vs > vspace - vslack) {
        vs = bottom - (vspace - vslack);
    }
    if (ht - vs < vspace) {
        vs = ht - vspace;
    }
    if (0 - vs > 0) {
        vs = 0;
    }

    if (grav != 0) {
        if (x - hs < hslack) {
            hs = x - hslack;
        }
        if (x - hs > hspace - hslack) {
            hs = x - (hspace - hslack);
        }
    }

    if (grav < 0) {
        if (left - hs > 0) {
            hs = left;
        }
        if (right - hs < hspace) {
            hs = right - hspace;
        }
    } else if (grav > 0) {
        if (right - hs < hspace) {
            hs = right - hspace;
        }
        if (left - hs > 0) {
            hs = left;
        }
    } else /* grav == 0 */ {
        if (right - left <= hspace) {
            /*
             * If the entire text fits, center it exactly.
             */
            hs = left - (hspace - (right - left)) / 2;
        } else if (x > right - hslack) {
            /*
             * If we are near the right edge, keep the right edge
             * at the edge of the view.
             */
            hs = right - hspace;
        } else if (x < left + hslack) {
            /*
             * If we are near the left edge, keep the left edge
             * at the edge of the view.
             */
            hs = left;
        } else if (left > hs) {
            /*
             * Is there whitespace visible at the left?  Fix it if so.
             */
            hs = left;
        } else if (right < hs + hspace) {
            /*
             * Is there whitespace visible at the right?  Fix it if so.
             */
            hs = right - hspace;
        } else {
            /*
             * Otherwise, float as needed.
             */
            if (x - hs < hslack) {
                hs = x - hslack;
            }
            if (x - hs > hspace - hslack) {
                hs = x - (hspace - hslack);
            }
        }
    }

    if (hs != mScrollX || vs != mScrollY) {
        if (mScroller == nullptr) {
            scrollTo(hs, vs);
        } else {
            const long duration = AnimationUtils::currentAnimationTimeMillis() - mLastScroll;
            int dx = hs - mScrollX;
            int dy = vs - mScrollY;

            if (duration > ANIMATED_SCROLL_GAP) {
                mScroller->startScroll(mScrollX, mScrollY, dx, dy);
                awakenScrollBars(mScroller->getDuration(),true);
                invalidate();
            } else {
                if (!mScroller->isFinished()) {
                    mScroller->abortAnimation();
                }

                scrollBy(dx, dy);
            }

            mLastScroll = AnimationUtils::currentAnimationTimeMillis();
        }

        changed = true;
    }

    if (isFocused()) {
        // This offsets because getInterestingRect() is in terms of viewport coordinates, but
        // requestRectangleOnScreen() is in terms of content coordinates.

        // The offsets here are to ensure the rectangle we are using is
        // within our view bounds, in case the cursor is on the far left
        // or right.  If it isn't withing the bounds, then this request
        // will be ignored.
        Rect mTempRect;
        mTempRect.set(x - 2, top, x + 2, bottom);
        getInterestingRect(mTempRect, line);
        mTempRect.offset(mScrollX, mScrollY);

        if (requestRectangleOnScreen(mTempRect)) {
            changed = true;
        }
    }

    return changed;
}

bool TextView::moveCursorToVisibleOffset() {
    if (dynamic_cast<Spannable*>(mText)==nullptr){
        return false;
    }
    int start = getSelectionStart();
    int end = getSelectionEnd();
    if (start != end) {
        return false;
    }

    // First: make sure the line is visible on screen:

    int line = mLayout->getLineForOffset(start);

    const int top = mLayout->getLineTop(line);
    const int bottom = mLayout->getLineTop(line + 1);
    const int vspace = mBottom - mTop - getExtendedPaddingTop() - getExtendedPaddingBottom();
    int vslack = (bottom - top) / 2;
    if (vslack > vspace / 4) {
        vslack = vspace / 4;
    }
    const int vs = mScrollY;

    if (top < (vs + vslack)) {
        line = mLayout->getLineForVertical(vs + vslack + (bottom - top));
    } else if (bottom > (vspace + vs - vslack)) {
        line = mLayout->getLineForVertical(vspace + vs - vslack - (bottom - top));
    }

    // Next: make sure the character is visible on screen:

    const int hspace = mRight - mLeft - getCompoundPaddingLeft() - getCompoundPaddingRight();
    const int hs = mScrollX;
    const int leftChar = mLayout->getOffsetForHorizontal(line, hs);
    const int rightChar = mLayout->getOffsetForHorizontal(line, hspace + hs);

    // line might contain bidirectional text
    const int lowChar = leftChar < rightChar ? leftChar : rightChar;
    const int highChar = leftChar > rightChar ? leftChar : rightChar;

    int newStart = start;
    if (newStart < lowChar) {
        newStart = lowChar;
    } else if (newStart > highChar) {
        newStart = highChar;
    }

    if (newStart != start) {
        //Selection.setSelection(mSpannable, newStart);
        return true;
    }
    return false;
}

void TextView::getInterestingRect(Rect& r, int line) {
    convertFromViewportToContentCoordinates(r);

    // Rectangle can can be expanded on first and last line to take
    // padding into account.
    // TODO Take left/right padding into account too?
    if (line == 0) r.top -= getExtendedPaddingTop();
    if (line == mLayout->getLineCount() - 1) r.width +=getExtendedPaddingTop() + getExtendedPaddingBottom();
}

void TextView::convertFromViewportToContentCoordinates(Rect& r) {
    const int horizontalOffset = viewportToContentHorizontalOffset();
    r.left += horizontalOffset;
    r.width += horizontalOffset;

    const int verticalOffset = viewportToContentVerticalOffset();
    r.top += verticalOffset;
    r.height += verticalOffset;
}

int TextView::viewportToContentHorizontalOffset() {
    return getCompoundPaddingLeft() - mScrollX;
}

int TextView::viewportToContentVerticalOffset() {
    int offset = getExtendedPaddingTop() - mScrollY;
    if ((mGravity & Gravity::VERTICAL_GRAVITY_MASK) != Gravity::TOP) {
        offset += getVerticalOffset(false);
    }
    return offset;
}

void TextView::computeScroll() {
    if (mScroller != nullptr) {
        if (mScroller->computeScrollOffset()) {
            mScrollX = mScroller->getCurrX();
            mScrollY = mScroller->getCurrY();
            invalidateParentCaches();
            postInvalidate();  // So we draw again
        }
    }
}

void TextView::onLayout(bool changed, int left, int top, int width, int height){
    View::onLayout(changed, left, top, width, height);
    if (mDeferScroll >= 0) {
       int curs = mDeferScroll;
       mDeferScroll = -1;
       bringPointIntoView(std::min(curs, (int)getText().length()));
    }
    // Call auto-size after the width and height have been calculated.
    autoSizeText();
}

void TextView::onFocusChanged(bool focused, int direction, Rect* previouslyFocusedRect){
    if (isTemporarilyDetached()) {
        // If we are temporarily in the detach state, then do nothing.
        View::onFocusChanged(focused, direction, previouslyFocusedRect);
        return;
    }
    startStopMarquee(focused);
    View::onFocusChanged(focused, direction, previouslyFocusedRect);
}

void TextView::onWindowFocusChanged(bool hasWindowFocus) {
    View::onWindowFocusChanged(hasWindowFocus);
    //if (mEditor != null) mEditor.onWindowFocusChanged(hasWindowFocus);
    startStopMarquee(hasWindowFocus);
}

void TextView::onVisibilityChanged(View& changedView, int visibility) {
    View::onVisibilityChanged(changedView, visibility);
    if (/*mEditor != null &&*/ visibility != VISIBLE) {
        //mEditor.hideCursorAndSpanControllers();
        //stopTextActionMode();
    }
}

/**
 * Called by the framework in response to a text completion from
 * the current input method, provided by it calling
 * {@link InputConnection#commitCompletion
 * InputConnection.commitCompletion()}.  The default implementation does
 * nothing; text views that are supporting auto-completion should override
 * this to do their desired behavior.
 *
 * @param text The auto complete text the user has selected.
 */
void TextView::onCommitCompletion(CompletionInfo* completion){
    // intentionally empty
}

void TextView::setSelected(bool selected){
    const bool wasSelected = isSelected();

    View::setSelected(selected);

    if ((selected != wasSelected) && (mEllipsize == TextUtils::TruncateAt::MARQUEE)) {
        if (selected) {
            startMarquee();
        } else {
            stopMarquee();
        }
    }
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
    if(mLayout!=nullptr && newLayout){
        // XXX this is heavy-handed because no actual content changes.
        const int want = mLayout->getWidth();
        const int hintWant = mHintLayout == nullptr ? 0 : mHintLayout->getWidth();

        makeNewLayout(want, hintWant, &UNKNOWN_BORING, &UNKNOWN_BORING,
                 mRight - mLeft - getCompoundPaddingLeft() - getCompoundPaddingRight(), true);
    }
}

int TextView::getGravity()const{
    return mGravity;
}

int TextView::getPaintFlags() const{
    return mTextPaint.getFlags();
}

/**
 * Sets flags on the Paint being used to display the text and
 * reflows the text if they are different from the old flags.
 * @see Paint#setFlags
 */
void TextView::setPaintFlags(int flags) {
    if (mTextPaint.getFlags() != flags) {
        mTextPaint.setFlags(flags);

        if (mLayout != nullptr) {
            nullLayouts();
            requestLayout();
            invalidate();
        }
    }
}

void TextView::setHorizontallyScrolling(bool whether) {
    if (mHorizontallyScrolling != whether) {
        mHorizontallyScrolling = whether;
        if (mLayout != nullptr) {
            nullLayouts();
            requestLayout();
            invalidate();
        }
    }
}
bool TextView::isHorizontallyScrollable() const{
    return mHorizontallyScrolling;
}
bool TextView::getHorizontallyScrolling() const{
    return mHorizontallyScrolling;
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
    return (mMaxWidthMode == PIXELS) ? mMaxWidth : -1;
}

int TextView::getLineCount()const{
    return mLayout? mLayout->getLineCount() : 0;
}

int TextView::getLineBounds(int line, Rect&bounds) {
    if(mLayout==nullptr){
        bounds.setEmpty();
        return 0;
    }else{
        int baseline = mLayout->getLineBounds(line, &bounds);
        int voffset = getExtendedPaddingTop();
        if ((mGravity & Gravity::VERTICAL_GRAVITY_MASK) != Gravity::TOP) {
            voffset += getVerticalOffset(true);
        }
        bounds.offset(getCompoundPaddingLeft(), voffset);
        return baseline + voffset;
    }
}

int TextView::getBaseline(){
    if(mLayout == nullptr)
        return View::getBaseline();
    return getBaselineOffset() + mLayout->getLineBaseline(0);
}

int TextView::getBaselineOffset(){
    int voffset = 0;
    if((mGravity&Gravity::VERTICAL_GRAVITY_MASK)!=Gravity::TOP)
        voffset = getVerticalOffset(true);
    if(isLayoutModeOptical((View*)mParent))
        voffset -= getOpticalInsets().top;
    return getExtendedPaddingTop()+voffset;
}

int TextView::getLineHeight()const{
    return std::round(mTextPaint.getFontMetricsInt(nullptr) * mSpacingMult + mSpacingAdd);
}

void TextView::setLineHeight(int lineHeight){
    const int fontHeight = getPaint().getFontMetricsInt(nullptr);
    // Make sure we don't setLineSpacing if it's not needed to avoid unnecessary redraw.
    if(lineHeight!=fontHeight){
        // Set lineSpacingExtra by the difference of lineSpacing with lineHeight
        setLineSpacing(lineHeight - fontHeight,1.f);
    }
}

bool TextView::isAutoSizeEnabled() const{
    return supportsAutoSizeText() && mAutoSizeTextType != AUTO_SIZE_TEXT_TYPE_NONE;
}

bool TextView::supportsAutoSizeText()const {
    return true;
}

void TextView::autoSizeText() {
    if (!isAutoSizeEnabled()) {
        return;
    }

    if (mNeedsAutoSizeText) {
        if (getMeasuredWidth() <= 0 || getMeasuredHeight() <= 0) {
            return;
        }

        const int availableWidth = mHorizontallyScrolling  ? VERY_WIDE
                : getMeasuredWidth() - getTotalPaddingLeft() - getTotalPaddingRight();
        const int availableHeight = getMeasuredHeight() - getExtendedPaddingBottom()
                    - getExtendedPaddingTop();

        if (availableWidth <= 0 || availableHeight <= 0) {
            return;
        }
        RectF TEMP_RECTF;
        TEMP_RECTF.setEmpty();
        TEMP_RECTF.width = availableWidth;
        TEMP_RECTF.height = availableHeight;
        const float optimalTextSize = findLargestTextSizeWhichFits(TEMP_RECTF);
        if (optimalTextSize != getTextSize()) {
            setTextSizeInternal(TypedValue::COMPLEX_UNIT_PX, optimalTextSize,
                    false /* shouldRequestLayout */);

            makeNewLayout(availableWidth, 0 /* hintWidth */, &UNKNOWN_BORING,&UNKNOWN_BORING,
                    mRight - mLeft - getCompoundPaddingLeft() - getCompoundPaddingRight(),
                    false /* bringIntoView */);
        }
    }
    // Always try to auto-size if enabled. Functions that do not want to trigger auto-sizing
    // after the next layout pass should set this to false.
    mNeedsAutoSizeText = true;
}

int TextView::findLargestTextSizeWhichFits(const RectF& availableSpace) {
    int sizesCount = mAutoSizeTextSizesInPx.size();
    if (sizesCount == 0) {
        //throw new IllegalStateException("No available text sizes to choose from.");
    }

    int bestSizeIndex = 0;
    int lowIndex = bestSizeIndex + 1;
    int highIndex = sizesCount - 1;
    int sizeToTryIndex;
    while (lowIndex <= highIndex) {
        sizeToTryIndex = (lowIndex + highIndex) / 2;
        if (suggestedSizeFitsInSpace(mAutoSizeTextSizesInPx[sizeToTryIndex], availableSpace)) {
            bestSizeIndex = lowIndex;
            lowIndex = sizeToTryIndex + 1;
        } else {
            highIndex = sizeToTryIndex - 1;
            bestSizeIndex = highIndex;
        }
    }
    return mAutoSizeTextSizesInPx[bestSizeIndex];
}

bool TextView::suggestedSizeFitsInSpace(int suggestedSizeInPx,const RectF& availableSpace) {
    CharSequence* text = nullptr;//mTransformed != nullptr? mTransformed: getText();
    const int maxLines = getMaxLines();
    TextPaint mTempTextPaint;
    mTempTextPaint.set(getPaint());
    mTempTextPaint.setTextSize(suggestedSizeInPx);

    StaticLayout::Builder* layoutBuilder = StaticLayout::Builder::obtain(
            text, 0, text->length(), &mTempTextPaint, std::round(availableSpace.right()));

    layoutBuilder->setAlignment(getLayoutAlignment())
            .setLineSpacing(getLineSpacingExtra(), getLineSpacingMultiplier())
            .setIncludePad(getIncludeFontPadding())
            .setUseLineSpacingFromFallbacks(mUseFallbackLineSpacing)
            .setBreakStrategy(getBreakStrategy())
            .setHyphenationFrequency(mHyphenationFrequency)
            .setJustificationMode(mJustificationMode)
            .setMaxLines(mMaxMode == LINES ? mMaximum : INT_MAX)
            .setTextDirection(getTextDirectionHeuristic());

    StaticLayout* layout = layoutBuilder->build();
    // Lines overflow.
    if (maxLines != -1 && layout->getLineCount() > maxLines) {
        return false;
    }
    // Height overflow.
    if (layout->getHeight() > availableSpace.bottom()) {
        return false;
    }
    return true;
}

int TextView::getDesiredHeight(){
    return getDesiredHeight(mLayout,true);
}

int TextView::getDesiredHeight(Layout* layout, bool cap){
    if (layout == nullptr) {
        return 0;
    }

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
    } else if (cap && (linecount > mMaximum) && (dynamic_cast<DynamicLayout*>(layout)
            || dynamic_cast<BoringLayout*>(layout))) {
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
    return (mMaxMode == LINES) ? mMaximum : -1;
}

int TextView::getMaxHeight()const{
    return (mMaxMode == PIXELS) ? mMaximum : -1;
}

void TextView::setMaxHeight(int maxPixels){
    mMaximum = maxPixels;
    mMaxMode = PIXELS;

    requestLayout();
    invalidate(true);
}

void TextView::setMinLines(int minLines) {
    mMinimum = minLines;
    mMinMode = LINES;

    requestLayout();
    invalidate();
}

int TextView::getMinLines() const{
    return (mMinMode == LINES) ? mMinimum : -1;
}

void TextView::setLines(int lines){
    mMaximum = mMinimum= lines;
    mMaxMode = mMinMode = LINES;
    requestLayout();
    invalidate();
}

void TextView::setHeight(int pixels) {
    mMaximum = mMinimum = pixels;
    mMaxMode = mMinMode = PIXELS;

    requestLayout();
    invalidate();
}

void TextView::setMinEms(int minEms) {
    mMinWidth = minEms;
    mMinWidthMode = EMS;

    requestLayout();
    invalidate();
}

int TextView::getMinEms() const{
    return mMinWidthMode == EMS ? mMinWidth : -1;
}

void TextView::setMaxEms(int maxEms) {
    mMaxWidth = maxEms;
    mMaxWidthMode = EMS;

    requestLayout();
    invalidate();
}

int TextView::getMaxEms() const{
    return mMaxWidthMode == EMS ? mMaxWidth : -1;
}

void TextView::setEms(int ems) {
    mMaxWidth = mMinWidth = ems;
    mMaxWidthMode = mMinWidthMode = EMS;

    requestLayout();
    invalidate();
}

void TextView::nullLayouts() {
    if (dynamic_cast<BoringLayout*>(mLayout) && mSavedLayout == nullptr) {
        mSavedLayout = (BoringLayout*) mLayout;
    }
    if (dynamic_cast<BoringLayout*>(mHintLayout) && mSavedHintLayout == nullptr) {
        mSavedHintLayout = (BoringLayout*) mHintLayout;
    }

    mSavedMarqueeModeLayout = mLayout = mHintLayout = nullptr;

    mBoring = mHintBoring = nullptr;

    // Since it depends on the value of mLayout
    //if (mEditor != nullptr) mEditor.prepareCursorControllers();
}

void TextView::assumeLayout() {
    int width = mRight - mLeft - getCompoundPaddingLeft() - getCompoundPaddingRight();
    if (width < 1) {
        width = 0;
    }
    const int physicalWidth = width;
    if (mHorizontallyScrolling) {
        width = VERY_WIDE;
    }
    makeNewLayout(width, physicalWidth, &UNKNOWN_BORING, &UNKNOWN_BORING, physicalWidth, false);
}

void TextView::makeNewLayout(int wantWidth, int hintWidth, BoringLayout::Metrics* boring,
            BoringLayout::Metrics* hintBoring,int ellipsisWidth, bool bringIntoView){
    stopMarquee();

    // Update "old" cached values
    mOldMaximum = mMaximum;
    mOldMaxMode = mMaxMode;

    mHighlightPathBogus = true;

    if (wantWidth < 0) {
        wantWidth = 0;
    }
    if (hintWidth < 0) {
        hintWidth = 0;
    }

    Layout::Alignment alignment = getLayoutAlignment();
    const bool testDirChange = mSingleLine && mLayout != nullptr
            && (alignment == Layout::Alignment::ALIGN_NORMAL
                    || alignment == Layout::Alignment::ALIGN_OPPOSITE);
    int oldDir = 0;
    if (testDirChange) oldDir = mLayout->getParagraphDirection(0);
    bool shouldEllipsize = mEllipsize != TextUtils::TruncateAt::NONE;// && getKeyListener() == nullptr;
    const bool switchEllipsize = mEllipsize == TextUtils::TruncateAt::MARQUEE
            && mMarqueeFadeMode != MARQUEE_FADE_NORMAL;
    TextUtils::TruncateAt effectiveEllipsize = mEllipsize;
    if (mEllipsize == TextUtils::TruncateAt::MARQUEE
            && mMarqueeFadeMode == MARQUEE_FADE_SWITCH_SHOW_ELLIPSIS) {
        effectiveEllipsize = TextUtils::TruncateAt::END_SMALL;
    }

    if (mTextDir == nullptr) {
        mTextDir = getTextDirectionHeuristic();
    }

    mLayout = makeSingleLayout(wantWidth, boring, ellipsisWidth, alignment, shouldEllipsize,
            effectiveEllipsize, effectiveEllipsize == mEllipsize);
    if (switchEllipsize) {
        TextUtils::TruncateAt oppositeEllipsize = effectiveEllipsize == TextUtils::TruncateAt::MARQUEE
                ? TextUtils::TruncateAt::END : TextUtils::TruncateAt::MARQUEE;
        mSavedMarqueeModeLayout = makeSingleLayout(wantWidth, boring, ellipsisWidth, alignment,
                shouldEllipsize, oppositeEllipsize, effectiveEllipsize != mEllipsize);
    }

    shouldEllipsize = mEllipsize != TextUtils::TruncateAt::NONE;
    mHintLayout = nullptr;

    if (mHint != nullptr) {
        if (shouldEllipsize) hintWidth = wantWidth;

        if (hintBoring == &UNKNOWN_BORING) {
            hintBoring = BoringLayout::isBoring(mHint, &mTextPaint, mTextDir, mHintBoring);
            if (hintBoring != nullptr) {
                mHintBoring = hintBoring;
            }
        }

        if (hintBoring != nullptr) {
            if (hintBoring->width <= hintWidth
                    && (!shouldEllipsize || hintBoring->width <= ellipsisWidth)) {
                if (mSavedHintLayout != nullptr) {
                    mHintLayout = mSavedHintLayout->replaceOrMake(mHint, &mTextPaint,
                            hintWidth, alignment, mSpacingMult, mSpacingAdd,
                            *hintBoring, mIncludePad);
                } else {
                    mHintLayout = BoringLayout::make(mHint, &mTextPaint,
                            hintWidth, alignment, mSpacingMult, mSpacingAdd,
                            *hintBoring, mIncludePad);
                }

                mSavedHintLayout = (BoringLayout*) mHintLayout;
            } else if (shouldEllipsize && hintBoring->width <= hintWidth) {
                if (mSavedHintLayout != nullptr) {
                    mHintLayout = mSavedHintLayout->replaceOrMake(mHint, &mTextPaint,
                            hintWidth, alignment, mSpacingMult, mSpacingAdd,
                            *hintBoring, mIncludePad, mEllipsize,
                            ellipsisWidth);
                } else {
                    mHintLayout = BoringLayout::make(mHint, &mTextPaint,
                            hintWidth, alignment, mSpacingMult, mSpacingAdd,
                            *hintBoring, mIncludePad, mEllipsize,
                            ellipsisWidth);
                }
            }
        }
        // TODO: code duplication with makeSingleLayout()
        if (mHintLayout == nullptr) {
            StaticLayout::Builder* builder = StaticLayout::Builder::obtain(mHint, 0,
                       mHint->length(), &mTextPaint, hintWidth);
                    builder->setAlignment(alignment)
                    .setTextDirection(mTextDir)
                    .setLineSpacing(mSpacingAdd, mSpacingMult)
                    .setIncludePad(mIncludePad)
                    .setUseLineSpacingFromFallbacks(mUseFallbackLineSpacing)
                    .setBreakStrategy(mBreakStrategy)
                    .setHyphenationFrequency(mHyphenationFrequency)
                    .setJustificationMode(mJustificationMode)
                    .setMaxLines(mMaxMode == LINES ? mMaximum : INT_MAX);
            if (shouldEllipsize) {
                builder->setEllipsize(mEllipsize)
                        .setEllipsizedWidth(ellipsisWidth);
            }
            mHintLayout = builder->build();
        }
    }

    if (bringIntoView || (testDirChange && oldDir != mLayout->getParagraphDirection(0))) {
        //registerForPreDraw();
    }

    if (mEllipsize == TextUtils::TruncateAt::MARQUEE) {
        if (!compressText(ellipsisWidth)) {
            const int height = mLayoutParams->height;
            // If the size of the view does not depend on the size of the text, try to
            // start the marquee immediately
            if (height != LayoutParams::WRAP_CONTENT && height != LayoutParams::MATCH_PARENT) {
                startMarquee();
            } else {
                // Defer the start of the marquee until we know our width (see setFrame())
                mRestartMarquee = true;
            }
        }
    }

    // CursorControllers need a non-null mLayout
    //if (mEditor != nullptr) mEditor->prepareCursorControllers();
}

bool TextView::useDynamicLayout() const{
    return isTextSelectable() || (mSpannable != nullptr && mPrecomputed == nullptr);
}

Layout* TextView::makeSingleLayout(int wantWidth, BoringLayout::Metrics* boring, int ellipsisWidth,
        Layout::Alignment alignment, bool shouldEllipsize, TextUtils::TruncateAt effectiveEllipsize, bool useSaved) {
    Layout* result = nullptr;
    if (useDynamicLayout()) {
        DynamicLayout::Builder* builder = DynamicLayout::Builder::obtain(mText, &mTextPaint,wantWidth);
                builder->setDisplayText(mTransformed)
                .setAlignment(alignment)
                .setTextDirection(mTextDir)
                .setLineSpacing(mSpacingAdd, mSpacingMult)
                .setIncludePad(mIncludePad)
                .setUseLineSpacingFromFallbacks(mUseFallbackLineSpacing)
                .setBreakStrategy(mBreakStrategy)
                .setHyphenationFrequency(mHyphenationFrequency)
                .setJustificationMode(mJustificationMode)
                .setEllipsize(/*getKeyListener()==nullptr?effectiveEllipsize:*/TextUtils::TruncateAt::NONE)
                .setEllipsizedWidth(ellipsisWidth);
        result = builder->build();
    } else {
        if (boring == &UNKNOWN_BORING) {
            boring = BoringLayout::isBoring(mTransformed, &mTextPaint, mTextDir, mBoring);
            if (boring != nullptr) {
                mBoring = boring;
            }
        }

        if (boring != nullptr) {
            if (boring->width <= wantWidth
                    && (effectiveEllipsize == TextUtils::TruncateAt::NONE || boring->width <= ellipsisWidth)) {
                if (useSaved && mSavedLayout != nullptr) {
                    result = mSavedLayout->replaceOrMake(mTransformed, &mTextPaint,
                            wantWidth, alignment, mSpacingMult, mSpacingAdd,
                            *boring, mIncludePad);
                } else {
                    result = BoringLayout::make(mTransformed, &mTextPaint,
                            wantWidth, alignment, mSpacingMult, mSpacingAdd,
                            *boring, mIncludePad);
                }

                if (useSaved) {
                    mSavedLayout = (BoringLayout*) result;
                }
            } else if (shouldEllipsize && boring->width <= wantWidth) {
                if (useSaved && mSavedLayout != nullptr) {
                    result = mSavedLayout->replaceOrMake(mTransformed, &mTextPaint,
                            wantWidth, alignment, mSpacingMult, mSpacingAdd,
                            *boring, mIncludePad, effectiveEllipsize,
                            ellipsisWidth);
                } else {
                    result = BoringLayout::make(mTransformed, &mTextPaint,
                            wantWidth, alignment, mSpacingMult, mSpacingAdd,
                            *boring, mIncludePad, effectiveEllipsize,
                            ellipsisWidth);
                }
            }
        }
    }
    if (result == nullptr) {
        StaticLayout::Builder* builder = StaticLayout::Builder::obtain(mTransformed,
                    0, mTransformed->length(), &mTextPaint, wantWidth);
                builder->setAlignment(alignment)
                .setTextDirection(mTextDir)
                .setLineSpacing(mSpacingAdd, mSpacingMult)
                .setIncludePad(mIncludePad)
                .setUseLineSpacingFromFallbacks(mUseFallbackLineSpacing)
                .setBreakStrategy(mBreakStrategy)
                .setHyphenationFrequency(mHyphenationFrequency)
                .setJustificationMode(mJustificationMode)
                .setMaxLines(mMaxMode == LINES ? mMaximum : INT_MAX);
        if (shouldEllipsize) {
            builder->setEllipsize(effectiveEllipsize)
                    .setEllipsizedWidth(ellipsisWidth);
        }
        result = builder->build();
    }
    return result;
}

bool TextView::compressText(float width) {
    if (isHardwareAccelerated()) return false;

    // Only compress the text if it hasn't been compressed by the previous pass
    if ((width > 0.0f) && mLayout && (getLineCount() == 1) && !mUserSetTextScaleX
            && (mTextPaint.getTextScaleX() == 1.0f)) {
        const float textWidth = mLayout->getLineWidth(0);
        const float overflow = (textWidth + 1.0f - width) / width;
        if (overflow > 0.0f && overflow <= Marquee::MARQUEE_DELTA_MAX) {
            setTextScaleX(1.0f - overflow - 0.005f);
            post([this]() {
                requestLayout();
            });
            return true;
        }
    }
    return false;
}

int TextView::desired(Layout*layout){
    int max = 0;
    const int N = layout->getLineCount();
    CharSequence* text = layout->getText();
    for (int i = 0; i < N - 1; i++) {
        if (text->charAt(layout->getLineEnd(i) - 1)!= '\n') {
            return -1;
        }
    }
    for (int i = 0; i < N; i++) {
        max = std::max(max, (int)layout->getLineWidth(i));
    }
    /*if (useBoundsForWidth) {
        max = std::max(max, layout->computeDrawingBoundingBox().width());
    }*/
    return (int) std::ceil(max);
}

void TextView::onMeasure(int widthMeasureSpec, int heightMeasureSpec){
    const int widthMode  = MeasureSpec::getMode(widthMeasureSpec);
    const int heightMode = MeasureSpec::getMode(heightMeasureSpec);
    const int widthSize  = MeasureSpec::getSize(widthMeasureSpec);
    const int heightSize = MeasureSpec::getSize(heightMeasureSpec);

    int width;
    int height;

    BoringLayout::Metrics* boring = &UNKNOWN_BORING;
    BoringLayout::Metrics* hintBoring = &UNKNOWN_BORING;
    if (mTextDir == nullptr) {
        mTextDir = getTextDirectionHeuristic();
    }
    int des = -1;
    bool fromexisting = false;
    const float widthLimit = (widthMode == MeasureSpec::AT_MOST) ?  (float) widthSize : FLT_MAX;
    if (widthMode == MeasureSpec::EXACTLY) {
        // Parent has told us how big to be. So be it.
        width = widthSize;
    } else {
        if (mLayout != nullptr && mEllipsize == TextUtils::TruncateAt::NONE) {
            des = desired(mLayout);
        }

        if (des < 0) {
            boring = BoringLayout::isBoring(mTransformed, &mTextPaint, mTextDir, mBoring);
            if (boring != nullptr) {
                mBoring = boring;
            }
        } else {
            fromexisting = true;
        }

        if (boring == nullptr || boring == &UNKNOWN_BORING) {
            if (des < 0) {
                des = (int) std::ceil(Layout::getDesiredWidthWithLimit(mTransformed, 0,
                        mTransformed->length(), mTextPaint, mTextDir, widthLimit));
            }
            width = des;
        } else {
            width = boring->width;
        }

        Drawables* dr = mDrawables;
        if (dr != nullptr) {
            width = std::max(width, dr->mDrawableWidthTop);
            width = std::max(width, dr->mDrawableWidthBottom);
        }

        if (mHint != nullptr) {
            int hintDes = -1;
            int hintWidth;

            if (mHintLayout != nullptr && mEllipsize == TextUtils::TruncateAt::NONE) {
                hintDes = desired(mHintLayout);
            }

            if (hintDes < 0) {
                hintBoring = BoringLayout::isBoring(mHint, &mTextPaint, mTextDir, mHintBoring);
                if (hintBoring != nullptr) {
                    mHintBoring = hintBoring;
                }
            }

            if (hintBoring == nullptr || hintBoring == &UNKNOWN_BORING) {
                if (hintDes < 0) {
                    hintDes = (int) std::ceil(Layout::getDesiredWidthWithLimit(mHint, 0,
                            mHint->length(), mTextPaint, mTextDir, widthLimit));
                }
                hintWidth = hintDes;
            } else {
                hintWidth = hintBoring->width;
            }

            if (hintWidth > width) {
                width = hintWidth;
            }
        }

        width += getCompoundPaddingLeft() + getCompoundPaddingRight();

        if (mMaxWidthMode == EMS) {
            width = std::min(width, mMaxWidth * getLineHeight());
        } else {
            width = std::min(width, mMaxWidth);
        }

        if (mMinWidthMode == EMS) {
            width = std::max(width, mMinWidth * getLineHeight());
        } else {
            width = std::max(width, mMinWidth);
        }

        // Check against our minimum width
        width = std::max(width, getSuggestedMinimumWidth());

        if (widthMode == MeasureSpec::AT_MOST) {
            width = std::min(widthSize, width);
        }
    }

    int want = width - getCompoundPaddingLeft() - getCompoundPaddingRight();
    int unpaddedWidth = want;

    if (mHorizontallyScrolling) want = VERY_WIDE;

    int hintWant = want;
    int hintWidth = (mHintLayout == nullptr) ? hintWant : mHintLayout->getWidth();

    if (mLayout == nullptr) {
        makeNewLayout(want, hintWant, boring, hintBoring,
                      width - getCompoundPaddingLeft() - getCompoundPaddingRight(), false);
    } else {
        const bool layoutChanged = (mLayout->getWidth() != want) || (hintWidth != hintWant)
                || (mLayout->getEllipsizedWidth()
                        != width - getCompoundPaddingLeft() - getCompoundPaddingRight());

        const bool widthChanged = (mHint == nullptr) && (mEllipsize == TextUtils::TruncateAt::NONE)
                && (want > mLayout->getWidth())
                && (dynamic_cast<BoringLayout*>(mLayout)
                        || (fromexisting && des >= 0 && des <= want));

        const bool maximumChanged = (mMaxMode != mOldMaxMode) || (mMaximum != mOldMaximum);

        if (layoutChanged || maximumChanged) {
            if (!maximumChanged && widthChanged) {
                mLayout->increaseWidthTo(want);
            } else {
                makeNewLayout(want, hintWant, boring, hintBoring,
                        width - getCompoundPaddingLeft() - getCompoundPaddingRight(), false);
            }
        } else {
            // Nothing has changed
        }
    }

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
    if (mMaxMode == LINES && mLayout->getLineCount() > mMaximum) {
        unpaddedHeight = std::min(unpaddedHeight, mLayout->getLineTop(mMaximum));
    }

    /*
     * We didn't let makeNewLayout() register to bring the cursor into view,
     * so do it here if there is any possibility that it is needed.
     */
    if (/*mMovement != nullptr ||*/ mLayout->getWidth() > unpaddedWidth
            || mLayout->getHeight() > unpaddedHeight) {
        //registerForPreDraw();
    } else {
        scrollTo(0, 0);
    }

    setMeasuredDimension(width, height);
}

std::vector<Drawable*>TextView::getCompoundDrawables()const{
    if(mDrawables){
        Drawable**ds = mDrawables->mShowing;
        return std::vector<Drawable*>({ds[Drawables::LEFT],ds[Drawables::TOP],ds[Drawables::RIGHT],ds[Drawables::BOTTOM]});
    }
    return std::vector<Drawable*>{nullptr,nullptr,nullptr,nullptr};
}

void TextView::setCompoundDrawables(Drawable* left,Drawable* top,Drawable* right,Drawable*bottom){
    // We're switching to absolute, discard relative.
    Drawables*dr = mDrawables;
    if (dr != nullptr) {
        if (dr->mDrawableStart != nullptr)dr->mDrawableStart->setCallback(nullptr);
        dr->mDrawableStart = nullptr;
        if (dr->mDrawableEnd != nullptr) dr->mDrawableEnd->setCallback(nullptr);
        dr->mDrawableEnd = nullptr;
        dr->mDrawableSizeStart= dr->mDrawableHeightStart = 0;
        dr->mDrawableSizeEnd  = dr->mDrawableHeightEnd = 0;
    }
    const bool drawables = (left != nullptr) || (top != nullptr) || (right != nullptr) || (bottom != nullptr);
    if (!drawables) {
        // Clearing drawables...  can we free the data structure?
        if (dr != nullptr) {
            // We need to retain the last set padding, so just clear
            // out all of the fields in the existing structure.
            for (int i = 3/*dr->mShowing.length - 1*/; i >= 0; i--) {
                if (dr->mShowing[i] != nullptr) {
                    dr->mShowing[i]->setCallback(nullptr);
                    delete dr->mShowing[i];
                }
                dr->mShowing[i] = nullptr;
            }
            dr->mDrawableSizeLeft  = dr->mDrawableHeightLeft  = 0;
            dr->mDrawableSizeRight = dr->mDrawableHeightRight = 0;
            dr->mDrawableSizeTop   = dr->mDrawableWidthTop    = 0;
            dr->mDrawableSizeBottom= dr->mDrawableWidthBottom = 0;
            delete mDrawables;
            mDrawables = nullptr;
        }
    } else {
        if (dr == nullptr)  mDrawables = dr = new Drawables(getContext());

        mDrawables->mOverride = false;

        if ((dr->mShowing[Drawables::LEFT] != left) && (dr->mShowing[Drawables::LEFT] != nullptr)) {
            dr->mShowing[Drawables::LEFT]->setCallback(nullptr);
            delete dr->mShowing[Drawables::LEFT];
        }
        dr->mShowing[Drawables::LEFT] = left;

        if ((dr->mShowing[Drawables::TOP] != top) && (dr->mShowing[Drawables::TOP] != nullptr)) {
            dr->mShowing[Drawables::TOP]->setCallback(nullptr);
            delete dr->mShowing[Drawables::TOP];
        }
        dr->mShowing[Drawables::TOP] = top;

        if ((dr->mShowing[Drawables::RIGHT] != right) && (dr->mShowing[Drawables::RIGHT] != nullptr)) {
            dr->mShowing[Drawables::RIGHT]->setCallback(nullptr);
            delete dr->mShowing[Drawables::RIGHT];
        }
        dr->mShowing[Drawables::RIGHT] = right;

        if ((dr->mShowing[Drawables::BOTTOM] != bottom) && (dr->mShowing[Drawables::BOTTOM] != nullptr)) {
            dr->mShowing[Drawables::BOTTOM]->setCallback(nullptr);
            delete dr->mShowing[Drawables::BOTTOM];
        }
        dr->mShowing[Drawables::BOTTOM] = bottom;

        Rect compoundRect = dr->mCompoundRect;
        std::vector<int>state = getDrawableState();
        if (left != nullptr) {
            left->setState(state);
            compoundRect = left->getBounds();
            left->setCallback(this);
            dr->mDrawableSizeLeft  = compoundRect.width;
            dr->mDrawableHeightLeft= compoundRect.height;
        } else {
            dr->mDrawableSizeLeft = dr->mDrawableHeightLeft = 0;
        }
        if (right != nullptr) {
            right->setState(state);
            compoundRect = right->getBounds();
            right->setCallback(this);
            dr->mDrawableSizeRight  = compoundRect.width;
            dr->mDrawableHeightRight= compoundRect.height;
        } else {
            dr->mDrawableSizeRight = dr->mDrawableHeightRight = 0;
        }

        if (top != nullptr) {
            top->setState(state);
            compoundRect = top->getBounds();
            top->setCallback(this);
            dr->mDrawableSizeTop = compoundRect.height;
            dr->mDrawableWidthTop= compoundRect.width;
        } else {
            dr->mDrawableSizeTop = dr->mDrawableWidthTop = 0;
        }

        if (bottom != nullptr) {
            bottom->setState(state);
            compoundRect = bottom->getBounds();
            bottom->setCallback(this);
            dr->mDrawableSizeBottom = compoundRect.height;
            dr->mDrawableWidthBottom= compoundRect.width;
        } else {
            dr->mDrawableSizeBottom = dr->mDrawableWidthBottom = 0;
        }
    }

    // Save initial left/right drawables
    if (dr != nullptr) {
        dr->mDrawableLeftInitial = left;
        dr->mDrawableRightInitial= right;
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

std::vector<Drawable*> TextView::getCompoundDrawablesRelative() const{
    if (mDrawables != nullptr) {
        Drawables* dr = mDrawables;
        return std::vector<Drawable*>{
            dr->mDrawableStart, dr->mShowing[Drawables::TOP],
            dr->mDrawableEnd, dr->mShowing[Drawables::BOTTOM]
        };
    } else {
        return std::vector<Drawable*>{ nullptr, nullptr, nullptr, nullptr };
    }
}

const TextDirectionHeuristic*TextView::getTextDirectionHeuristic()const{
    if (hasPasswordTransformationMethod()) {
        // passwords fields should be LTR
        return TextDirectionHeuristics::LTR;
    }

    /*if (mEditor != nullptr && (mEditor.mInputType & EditorInfo.TYPE_MASK_CLASS)  == EditorInfo.TYPE_CLASS_PHONE) {
        // Phone numbers must be in the direction of the locale's digits. Most locales have LTR
        // digits, but some locales, such as those written in the Adlam or N'Ko scripts, have
        // RTL digits.
        final DecimalFormatSymbols symbols = DecimalFormatSymbols.getInstance(getTextLocale());
         String zero = symbols.getDigitStrings()[0];
        // In case the zero digit is multi-codepoint, just use the first codepoint to determine
        // direction.
        const int firstCodepoint = zero.codePointAt(0);
        const uint8_t digitDirection = Character::getDirectionality(firstCodepoint);
        if (digitDirection == Character::DIRECTIONALITY_RIGHT_TO_LEFT
                || digitDirection == Character::DIRECTIONALITY_RIGHT_TO_LEFT_ARABIC) {
            return TextDirectionHeuristics::RTL;
        } else {
            return TextDirectionHeuristics::LTR;
        }
    }*/

    // Always need to resolve layout direction first
    const bool defaultIsRtl = (getLayoutDirection() == View::LAYOUT_DIRECTION_RTL);

    // Now, we can select the heuristic
    switch (getTextDirection()) {
        default:
        case View::TEXT_DIRECTION_FIRST_STRONG:
            return (defaultIsRtl ? TextDirectionHeuristics::FIRSTSTRONG_RTL :
                    TextDirectionHeuristics::FIRSTSTRONG_LTR);
        case View::TEXT_DIRECTION_ANY_RTL:
            return TextDirectionHeuristics::ANYRTL_LTR;
        case View::TEXT_DIRECTION_LTR:
            return TextDirectionHeuristics::LTR;
        case View::TEXT_DIRECTION_RTL:
            return TextDirectionHeuristics::RTL;
        case View::TEXT_DIRECTION_LOCALE:
            return TextDirectionHeuristics::LOCALE;
        case View::TEXT_DIRECTION_FIRST_STRONG_LTR:
            return TextDirectionHeuristics::FIRSTSTRONG_LTR;
        case View::TEXT_DIRECTION_FIRST_STRONG_RTL:
            return TextDirectionHeuristics::FIRSTSTRONG_RTL;
    }
    return TextDirectionHeuristics::FIRSTSTRONG_LTR;
}

void TextView::onResolveDrawables(int layoutDirection){
    if (mLastLayoutDirection == layoutDirection) {
        return;
    }
    mLastLayoutDirection = layoutDirection;

    // Resolve drawables
    if (mDrawables != nullptr) {
        if (mDrawables->resolveWithLayoutDirection(layoutDirection)) {
            prepareDrawableForDisplay(mDrawables->mShowing[Drawables::LEFT]);
            prepareDrawableForDisplay(mDrawables->mShowing[Drawables::RIGHT]);
            applyCompoundDrawableTint();
        }
    }
}

void TextView::viewClicked(InputMethodManager*imm){
    if(imm)imm->viewClicked(this);
    LOGV("%p:%d",this,mID);
}

bool TextView::onTouchEvent(MotionEvent& event){
    const int action = event.getActionMasked();
    const bool superResult = View::onTouchEvent(event);
    if(action == MotionEvent::ACTION_UP){
        return superResult;
    }
    bool handled = false;
    const bool touchIsFinished = (action == MotionEvent::ACTION_UP) && isFocused();
           //&& (mEditor == null || !mEditor.mIgnoreActionUpEvent);
    if (touchIsFinished && isFocusable() && isEnabled()){//isTextEditable()){
        InputMethodManager& imm = InputMethodManager::getInstance();
        viewClicked(&imm);
        /*if (isTextEditable() && mEditor.mShowSoftInputOnFocus && imm != null
                && !showAutofillDialog()) {
            imm.showSoftInput(this, 0);
        }*/

        // The above condition ensures that the mEditor is not null
        //mEditor.onTouchUpEvent(event);

        handled = true;
    }
    if(handled)return true;
    return superResult;
}

void TextView::prepareDrawableForDisplay(Drawable* dr) {
    if (dr == nullptr) {
        return;
    }
    dr->setLayoutDirection(getLayoutDirection());
    if (dr->isStateful()) {
        dr->setState(getDrawableState());
        dr->jumpToCurrentState();
    }
}

void TextView::resetResolvedDrawables(){
    View::resetResolvedDrawables();
    mLastLayoutDirection = -1;
}

void TextView::setTypeface(Typeface* tf){
    if(mTextPaint.getTypeface()!=tf){
        mTextPaint.setTypeface(tf);
        if (mLayout != nullptr) {
            nullLayouts();
            requestLayout();
            invalidate();
        }
    }
}

Typeface*TextView::getTypeface()const{
    return mTextPaint.getTypeface();
}

int TextView::getTypefaceStyle() const{
    Typeface* typeface = mTextPaint.getTypeface();
    return typeface != nullptr ? typeface->getStyle() : Typeface::NORMAL;
}

void TextView::drawableStateChanged(){
    View::drawableStateChanged();

    if ((mTextColor && mTextColor->isStateful())
            || (mHintTextColor && mHintTextColor->isStateful())
            || (mLinkTextColor && mLinkTextColor->isStateful())) {
        updateTextColors();
    }
    if (mDrawables) {
        const std::vector<int> state = getDrawableState();
        for (int i=0;i<4;i++){
            Drawable* dr = mDrawables->mShowing[i];
            if ((dr != nullptr) && dr->isStateful() && dr->setState(state)) {
                invalidateDrawable(*dr);
            }
        }
    }
}

void TextView::drawableHotspotChanged(float x,float y){
    View::drawableHotspotChanged(x,y);
    for(int i = 0;mDrawables&&(i<4);i++){
        Drawable* dr = mDrawables->mShowing[i];
        if(dr)dr->setHotspot(x,y);
    }
}

bool TextView::isPaddingOffsetRequired() const{
    return (mShadowRadius != 0) || mDrawables != nullptr;
}

int TextView::getLeftPaddingOffset() {
    return getCompoundPaddingLeft() - mPaddingLeft
            + (int) std::min(.0f, mShadowDx - mShadowRadius);
}

int TextView::getTopPaddingOffset() {
    return (int) std::min(.0f, mShadowDy - mShadowRadius);
}

int TextView::getBottomPaddingOffset() {
    return (int) std::max(.0f, mShadowDy + mShadowRadius);
}

int TextView::getRightPaddingOffset() {
    return -(getCompoundPaddingRight() - mPaddingRight)
            + (int) std::max(.0f, mShadowDx + mShadowRadius);
}

bool TextView::verifyDrawable(Drawable* who)const {
    bool verified = View::verifyDrawable(who);
    if (!verified && mDrawables) {
        for (int i=0;i<4;i++){
            if (who == mDrawables->mShowing[i]) {
                return true;
            }
        }
    }
    return verified;
}

void TextView::jumpDrawablesToCurrentState(){
    View::jumpDrawablesToCurrentState();
    if (mDrawables != nullptr) {
        for (int i=0;i<4;i++){
            Drawable* dr = mDrawables->mShowing[i];
            if (dr != nullptr) {
                dr->jumpToCurrentState();
            }
        }
    }
}

void TextView::invalidateDrawable(Drawable& drawable){
    bool handled = false;

    if (verifyDrawable(&drawable)) {
        Rect dirty = drawable.getBounds();
        int scrollX = mScrollX;
        int scrollY = mScrollY;

        // IMPORTANT: The coordinates below are based on the coordinates computed
        // for each compound drawable in onDraw(). Make sure to update each section
        // accordingly.
        if (mDrawables != nullptr) {
            if (&drawable == mDrawables->mShowing[Drawables::LEFT]) {
                int compoundPaddingTop = getCompoundPaddingTop();
                int compoundPaddingBottom = getCompoundPaddingBottom();
                int vspace = mBottom - mTop - compoundPaddingBottom - compoundPaddingTop;

                scrollX += mPaddingLeft;
                scrollY += compoundPaddingTop + (vspace - mDrawables->mDrawableHeightLeft) / 2;
                handled = true;
            } else if (&drawable == mDrawables->mShowing[Drawables::RIGHT]) {
                int compoundPaddingTop = getCompoundPaddingTop();
                int compoundPaddingBottom = getCompoundPaddingBottom();
                int vspace = mBottom - mTop - compoundPaddingBottom - compoundPaddingTop;

                scrollX += (mRight - mLeft - mPaddingRight - mDrawables->mDrawableSizeRight);
                scrollY += compoundPaddingTop + (vspace - mDrawables->mDrawableHeightRight) / 2;
                handled = true;
            } else if (&drawable == mDrawables->mShowing[Drawables::TOP]) {
                int compoundPaddingLeft = getCompoundPaddingLeft();
                int compoundPaddingRight = getCompoundPaddingRight();
                int hspace = mRight - mLeft - compoundPaddingRight - compoundPaddingLeft;

                scrollX += compoundPaddingLeft + (hspace - mDrawables->mDrawableWidthTop) / 2;
                scrollY += mPaddingTop;
                handled = true;
            } else if (&drawable == mDrawables->mShowing[Drawables::BOTTOM]) {
                int compoundPaddingLeft = getCompoundPaddingLeft();
                int compoundPaddingRight = getCompoundPaddingRight();
                int hspace = mRight - mLeft - compoundPaddingRight - compoundPaddingLeft;

                scrollX += compoundPaddingLeft + (hspace - mDrawables->mDrawableWidthBottom) / 2;
                scrollY += (mBottom - mTop - mPaddingBottom - mDrawables->mDrawableSizeBottom);
                handled = true;
            }
        }

        if (handled) {
            dirty.offset(scrollX,scrollY);
            invalidate(dirty);
        }
    }

    if (!handled) {
        View::invalidateDrawable(drawable);
    }
}

bool TextView::isTextSelectable()const{
    return false;
}

void TextView::setTextSelectable(bool v){
}

void TextView::updateTextColors(){
    bool inval = false;
    int color;
    const std::vector<int>&drawableState = getDrawableState();
    if (mTextColor) {
        color = mTextColor->getColorForState(drawableState,0);
        LOGV("%p:%d change color %x->%x",this,mID,mCurTextColor,color);
        mCurTextColor = color;
        inval = true;
    }
    if (mLinkTextColor) {
        color = mLinkTextColor->getColorForState(drawableState,0);
        inval = true;
    }
    if (mHintTextColor) {
        color = mHintTextColor->getColorForState(drawableState,0);
        if (color != mCurHintTextColor) {
            mCurHintTextColor = color;
            if(mText->length()==0){
                inval = true;
            }
        }
    }
    if (inval) {
        // Text needs to be redrawn with the new color
        //if (mEditor != null) mEditor.invalidateTextDisplayList();
        invalidate(true);
    }
}

void TextView::setIncludeFontPadding(bool includepad){
    mIncludePad = includepad;
}

bool TextView::getIncludeFontPadding()const{
    return mIncludePad;
}

void TextView::setMarqueeRepeatLimit(int marqueeLimit) {
    mMarqueeRepeatLimit = marqueeLimit;
}

int TextView::getMarqueeRepeatLimit()const{
    return mMarqueeRepeatLimit;
}

TextUtils::TruncateAt TextView::getEllipsize()const{
    return mEllipsize;
}

void TextView::setEllipsize(TextUtils::TruncateAt where){
    if (mEllipsize != where) {
        mEllipsize = where;
        if (mLayout!=nullptr) {
            nullLayouts();
            requestLayout();
            invalidate();
        }
    }
}

void TextView::applySingleLine(bool singleLine, bool applyTransformation, bool changeMaxLines) {
   if (singleLine) {
       setLines(1);
       setHorizontallyScrolling(true);
#if 0
       if (applyTransformation) {
           setTransformationMethod(SingleLineTransformationMethod.getInstance());
       }

       if (!changeMaxLength) return;
       // Single line length filter is only applicable editable text.
       if (mBufferType != BufferType::EDITABLE) return;

       final InputFilter[] prevFilters = getFilters();
       for (InputFilter filter: getFilters()) {
           // We don't add LengthFilter if already there.
           if (filter instanceof InputFilter.LengthFilter) return;
       }

       if (mSingleLineLengthFilter == nullptr) {
           mSingleLineLengthFilter = new InputFilter.LengthFilter(
               MAX_LENGTH_FOR_SINGLE_LINE_EDIT_TEXT);
       }

       final InputFilter[] newFilters = new InputFilter[prevFilters.length + 1];
       System.arraycopy(prevFilters, 0, newFilters, 0, prevFilters.length);
       newFilters[prevFilters.length] = mSingleLineLengthFilter;

       setFilters(newFilters);

       // Since filter doesn't apply to existing text, trigger filter by setting text.
        setText(getText());
#endif
   } else {
       if (changeMaxLines) {
           setMaxLines(INT_MAX);
       }
       setHorizontallyScrolling(false);
#if 0
       if (applyTransformation) {
           setTransformationMethod(null);
       }

       if (!changeMaxLength) return;

       // Single line length filter is only applicable editable text.
       if (mBufferType != BufferType::EDITABLE) return;

       final InputFilter[] prevFilters = getFilters();
       if (prevFilters.length == 0) return;

       // Short Circuit: if mSingleLineLengthFilter is not allocated, nobody sets automated
       // single line char limit filter.
       if (mSingleLineLengthFilter == nullptr) return;

       // If we need to remove mSingleLineLengthFilter, we need to allocate another array.
       // Since filter list is expected to be small and want to avoid unnecessary array
       // allocation, check if there is mSingleLengthFilter first.
       int targetIndex = -1;
       for (int i = 0; i < prevFilters.length; ++i) {
           if (prevFilters[i] == mSingleLineLengthFilter) {
               targetIndex = i;
               break;
           }
       }
       if (targetIndex == -1) return;  // not found. Do nothing.

       if (prevFilters.length == 1) {
           setFilters(NO_FILTERS);
           return;
       }

       // Create new array which doesn't include mSingleLengthFilter.
       InputFilter[] newFilters = new InputFilter[prevFilters.length - 1];
       System.arraycopy(prevFilters, 0, newFilters, 0, targetIndex);
       System.arraycopy(
               prevFilters,
               targetIndex + 1,
               newFilters,
               targetIndex,
               prevFilters.length - targetIndex - 1);
       setFilters(newFilters);
       mSingleLineLengthFilter = nullptr;
#endif
   }
}

void TextView::setTextColor(int color){
    mTextColor = ColorStateList::valueOf(color);
    updateTextColors();
}

void TextView::setTextColor(const cdroid::RefPtr<ColorStateList>& colors){
    if(colors==nullptr){
        FATAL("NullPointerException");
    }
    mTextColor = colors;
    updateTextColors();
}

const cdroid::RefPtr<ColorStateList> TextView::getTextColors()const{
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
    mCurHintTextColor = color;
    //setHintTextColor(ColorStateList::valueOf(color));
}

void TextView::setHintTextColor(const cdroid::RefPtr<ColorStateList>& colors){
    if(mHintTextColor!=colors){
        mHintTextColor = colors;
        updateTextColors();
    }
}

const cdroid::RefPtr<ColorStateList> TextView::getHintTextColors()const{
    return mHintTextColor;
}

int TextView::getCurrentHintTextColor()const{
    return mHintTextColor != nullptr ? mCurHintTextColor : mCurTextColor;
}


void TextView::setLinkTextColor(int color){
    setLinkTextColor(ColorStateList::valueOf(color));
}

void TextView::setLinkTextColor(const cdroid::RefPtr<ColorStateList>& colors){
    if(mLinkTextColor!=colors){
        mLinkTextColor = colors;
        updateTextColors();
    }
}

const cdroid::RefPtr<ColorStateList> TextView::getLinkTextColors()const{
    return mLinkTextColor;
}

void TextView::applyCompoundDrawableTint(){
    if (mDrawables == nullptr) return;
    if ( (mDrawables->mTintList==nullptr)&&(mDrawables->mHasTintMode==false) )return ;

    const auto tintList = mDrawables->mTintList;
    const int tintMode = mDrawables->mTintMode;
    const bool hasTint = (mDrawables->mTintList!=nullptr);
    const bool hasTintMode = mDrawables->mHasTintMode;
    const std::vector<int>state = getDrawableState();

    for (int i=0;i<4;i++){
        Drawable* dr = mDrawables->mShowing[i];
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

TransformationMethod* TextView::getTransformationMethod()const{
    return mTransformation;
}

void TextView::setTransformationMethod(TransformationMethod*method){
    if (method == mTransformation) {
        // Avoid the setText() below if the transformation is
        // the same.
        return;
    }
    if (mTransformation != nullptr) {
        if (mSpannable != nullptr) {
            //mSpannable->removeSpan(mTransformation);
        }
    }
    mTransformation = method;
#if 0
    if (method instanceof TransformationMethod2) {
        TransformationMethod2 method2 = (TransformationMethod2) method;
        mAllowTransformationLengthChange = !isTextSelectable() && !(mText instanceof Editable);
        method2.setLengthChangesAllowed(mAllowTransformationLengthChange);
    } else {
        mAllowTransformationLengthChange = false;
    }
    setText(mText);
    if (hasPasswordTransformationMethod()) {
        //notifyViewAccessibilityStateChangedIfNeeded(AccessibilityEvent.CONTENT_CHANGE_TYPE_UNDEFINED);
    }
#endif
    // PasswordTransformationMethod always have LTR text direction heuristics returned by
    // getTextDirectionHeuristic, needs reset
    mTextDir = getTextDirectionHeuristic();

}
void TextView::setCompoundDrawablePadding(int pad){
    if (pad == 0) {
       if (mDrawables != nullptr)
           mDrawables->mDrawablePadding = pad;
    } else {
         if (mDrawables == nullptr)
             mDrawables = new Drawables(getContext());
         mDrawables->mDrawablePadding = pad;
    }
    invalidate(true);
}

int TextView::getCompoundDrawablePadding()const{
    return mDrawables?mDrawables->mDrawablePadding:0;
}

int TextView::getCompoundPaddingLeft()const{
    Drawables* dr = mDrawables;
    if ((dr == nullptr) || (dr->mShowing[Drawables::LEFT] == nullptr)) {
        return mPaddingLeft;
    } else {
        return mPaddingLeft + dr->mDrawablePadding + dr->mDrawableSizeLeft;
    }
}

int TextView::getCompoundPaddingRight()const{
    Drawables* dr = mDrawables;
    if ((dr == nullptr) || (dr->mShowing[Drawables::RIGHT] == nullptr)) {
        return mPaddingRight;
    } else {
        return mPaddingRight + dr->mDrawablePadding + dr->mDrawableSizeRight;
    }
}

int TextView::getCompoundPaddingTop()const{
    Drawables* dr = mDrawables;
    if ((dr == nullptr) || (dr->mShowing[Drawables::TOP] == nullptr)) {
        return mPaddingTop;
    } else {
        return mPaddingTop + dr->mDrawablePadding + dr->mDrawableSizeTop;
    }
}

int TextView::getCompoundPaddingBottom()const{
    Drawables* dr = mDrawables;
    if ((dr == nullptr) || (dr->mShowing[Drawables::BOTTOM] == nullptr)) {
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

int TextView::getExtendedPaddingTop() {
    if (mMaxMode != LINES) {
        return getCompoundPaddingTop();
    }

    if (mLayout == nullptr){
        assumeLayout();
    }

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

int TextView::getExtendedPaddingBottom() {
    if(mMaxMode !=LINES){
        return getCompoundPaddingBottom();
    }

    if (mLayout == nullptr) {
        assumeLayout();
    }
    if (mLayout->getLineCount() <= mMaximum) {
        return getCompoundPaddingBottom();
    }

    int top = getCompoundPaddingTop();
    int bottom = getCompoundPaddingBottom();
    int viewht = getHeight() - top - bottom;
    int layoutht = mLayout->getLineTop(mMaximum);

    if (layoutht >= viewht) {
        return bottom;
    }

    const int gravity = mGravity & Gravity::VERTICAL_GRAVITY_MASK;
    if (gravity == Gravity::TOP) {
        return bottom + viewht - layoutht;
    } else if (gravity == Gravity::BOTTOM) {
        return bottom;
    } else { // (gravity == Gravity::CENTER_VERTICAL)
        return bottom + (viewht - layoutht) / 2;
    }
}

void TextView::setCompoundDrawableTintList(const RefPtr<ColorStateList>& tint){
    if (mDrawables == nullptr) {
        mDrawables = new Drawables(getContext());
    }
    if(mDrawables->mTintList!=tint){
        mDrawables->mTintList = tint;
        applyCompoundDrawableTint();
    }
}

const RefPtr<ColorStateList> TextView::getCompoundDrawableTintList()const{
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

int TextView::getCompoundDrawableTintMode()const{
    return mDrawables ? mDrawables->mTintMode : -1;
}

int TextView::getBoxHeight(Layout* l) {
    Insets opticalInsets = isLayoutModeOptical((View*)mParent) ? getOpticalInsets() : Insets::NONE;
    const int padding = (l ==mHintLayout)
	    ?getCompoundPaddingTop() + getCompoundPaddingBottom()
            :getExtendedPaddingTop() + getExtendedPaddingBottom();
    int measuedHeight=getMeasuredHeight();
    if(measuedHeight==0)measuedHeight=getHeight();
    return measuedHeight - padding +opticalInsets.top + opticalInsets.bottom;
}

int TextView::getVerticalOffset(bool forceNormal) {
    int voffset = 0;
    const int gravity = mGravity & Gravity::VERTICAL_GRAVITY_MASK;
    Layout* l = mLayout;
    if (!forceNormal && mText->length() == 0 && mHintLayout != nullptr) {
        l = mHintLayout;
    }
    if (gravity != Gravity::TOP) {
        const int boxht = getBoxHeight(l);
        const int textht = l->getHeight();
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
    if (!forceNormal && mText->length() == 0 && mHintLayout != nullptr) {
        l = mHintLayout;
    }
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
    invalidate();
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

const TextPaint& TextView::getPaint() const{
        return mTextPaint;
}

bool TextView::canSelectAllText()const{
    //mLayout->setSelection(-1,-1);
    return false;
}

bool TextView::selectAllText(){
    //const int length = (int)mLayout->getText().length();
    //mLayout->setSelection(0,length);
    return false;
}

int TextView::getTotalPaddingLeft() {
    return getCompoundPaddingLeft();
}

/**
 * Returns the total right padding of the view, including the right
 * Drawable if any.
 */
int TextView::getTotalPaddingRight() {
    return getCompoundPaddingRight();
}

/**
 * Returns the total start padding of the view, including the start
 * Drawable if any.
 */
int TextView::getTotalPaddingStart() {
    return getCompoundPaddingStart();
}

/**
 * Returns the total end padding of the view, including the end
 * Drawable if any.
 */
int TextView::getTotalPaddingEnd() {
    return getCompoundPaddingEnd();
}

int TextView::getTotalPaddingTop() {
    return getExtendedPaddingTop() + getVerticalOffset(true);
}

int TextView::getTotalPaddingBottom() {
    return getExtendedPaddingBottom() + getBottomVerticalOffset(true);
}

int TextView::getSelectionStart()const{
    return -1;
}

int TextView::getSelectionEnd()const{
    return -1;
}

bool TextView::hasSelection()const{
    const int selectionStart = getSelectionStart();
    const int selectionEnd = getSelectionEnd();
    return (selectionStart >= 0) && (selectionEnd > 0) && (selectionStart != selectionEnd);
}

std::string TextView::getSelectedText()const{
     if (!hasSelection()) {
         return std::string();
     }
     const int start = getSelectionStart();
     const int end = getSelectionEnd();
     //const std::wstring &text =mLayout->getText();
     //std::wstring ret = start<end?text.substr(start,end):text.substr(end,start);
     return "";//TextUtils::unicode2utf8(ret);
}

void TextView::setSingleLine(bool single){
    mSingleLine = single;
    applySingleLine(single,true,true);
}

void TextView::setBreakStrategy(int breakStrategy){
    mBreakStrategy = breakStrategy;
    if (mLayout != nullptr) {
        nullLayouts();
        requestLayout();
        invalidate();
    }
}

int TextView::getBreakStrategy()const{
    return mBreakStrategy;
}

void TextView::setHyphenationFrequency(int hyphenationFrequency) {
    mHyphenationFrequency = hyphenationFrequency;
    if (mLayout != nullptr) {
        nullLayouts();
        requestLayout();
        invalidate();
    }
}
int TextView::getHyphenationFrequency() const{
    return mHyphenationFrequency;
}

bool TextView::isSingleLine()const{
    return mSingleLine;
}

bool TextView::hasPasswordTransformationMethod()const{
    return mTransformation;
}

float TextView::getLeftFadingEdgeStrength(){
    if (isMarqueeFadeEnabled() && mMarquee && !mMarquee->isStopped()) {
        if (mMarquee->shouldDrawLeftFade()) {
            return getHorizontalFadingEdgeStrength(mMarquee->getScroll(), 0.0f);
        } else {
            return 0.f;
        }
    } else if (getLineCount() == 1) {
        const float lineLeft = getLayout()->getLineLeft(0);
        if (lineLeft > mScrollX) return 0.0f;
        return getHorizontalFadingEdgeStrength(mScrollX, lineLeft);
    }
    return View::getLeftFadingEdgeStrength();
}

float TextView::getRightFadingEdgeStrength(){
    if (isMarqueeFadeEnabled() && mMarquee && !mMarquee->isStopped()) {
        return getHorizontalFadingEdgeStrength(mMarquee->getMaxFadeScroll(), mMarquee->getScroll());
    } else if (getLineCount() == 1) {
        const float rightEdge = mScrollX +
                (getWidth() - getCompoundPaddingLeft() - getCompoundPaddingRight());
        const float lineRight = getLayout()->getLineRight(0);
        if (lineRight < rightEdge) return 0.0f;
        return getHorizontalFadingEdgeStrength(rightEdge, lineRight);
    }
    return View::getRightFadingEdgeStrength();
}

float TextView::getHorizontalFadingEdgeStrength(float position1, float position2) {
    const int horizontalFadingEdgeLength = getHorizontalFadingEdgeLength();
    if (horizontalFadingEdgeLength == 0) return 0.f;
    const float diff = std::abs(position1 - position2);
    if (diff > horizontalFadingEdgeLength) return 1.0f;
    return diff / horizontalFadingEdgeLength;
}

bool TextView::isMarqueeFadeEnabled()const{
    return (mEllipsize == TextUtils::TruncateAt::MARQUEE) && (mMarqueeFadeMode != MARQUEE_FADE_SWITCH_SHOW_ELLIPSIS);
}

bool TextView::canMarquee()const{
    const int width = mRight-mLeft - getCompoundPaddingLeft() - getCompoundPaddingRight();
    return (width > 0) && ((mLayout->getLineWidth(0) > width) 
            || (mMarqueeFadeMode != MARQUEE_FADE_NORMAL && mSavedMarqueeModeLayout != nullptr
                    && mSavedMarqueeModeLayout->getLineWidth(0) > width));
}

void TextView::startMarquee(){
    //if (getKeyListener() != nullptr) return;
    if (compressText(getWidth() - getCompoundPaddingLeft() - getCompoundPaddingRight())) {
        return;
    }
    if (((mMarquee == nullptr) || mMarquee->isStopped()) && (isFocused() || isSelected())
                && (getLineCount() == 1) && canMarquee()) {
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
    if (mEllipsize == TextUtils::TruncateAt::MARQUEE) {
        if (start) {
            startMarquee();
        } else {
            stopMarquee();
        }
    }
}

void TextView::onTextChanged(const std::wstring& text, int start, int lengthBefore, int lengthAfter){
}

void TextView::onSelectionChanged(int selStart, int selEnd){
}

bool TextView::setFrame(int l, int t, int w, int h) {
    const bool result = View::setFrame(l, t, w, h);
    restartMarqueeIfNeeded();
    return result;
}

void TextView::restartMarqueeIfNeeded(){
    if (mRestartMarquee && mEllipsize == TextUtils::TruncateAt::MARQUEE) {
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
    const int vspace = getHeight()- compoundPaddingBottom - compoundPaddingTop;
    const int hspace = getWidth() - compoundPaddingRight - compoundPaddingLeft;
    Drawables* dr = mDrawables;
    if (dr != nullptr) {
        /* Compound, not extended, because the icon is not clipped if the text height is smaller. */

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
    int color = mCurTextColor;
    if(mLayout==nullptr){
        assumeLayout();
    }
    Layout*layout = mLayout;
    if(mHint!=nullptr && mText->length()==0){
        color = mCurHintTextColor;
        layout= mHintLayout;
    }
    mTextPaint.setColor(color);

    canvas.save();
    const int extendedPaddingTop = getExtendedPaddingTop();
    const int extendedPaddingBottom = getExtendedPaddingBottom();

    const int maxScrollY = mLayout->getHeight() - vspace;

    int clipLeft  = compoundPaddingLeft + mScrollX;
    int clipTop   = (mScrollY == 0) ? 0 : extendedPaddingTop + mScrollY;
    int clipRight = getWidth() - compoundPaddingRight+ mScrollX;
    int clipBottom= getHeight() + mScrollY - ((mScrollY == maxScrollY) ? 0 : extendedPaddingBottom);
    LOGV_IF(dr!=nullptr,"%p rect=%d,%d-%d,%d ==>%d,%d-%d,%d paddings=%d,%d,%d,%d",this,
          rect.left,rect.top,rect.width,rect.height, clipLeft, clipTop, clipRight-clipLeft, clipBottom-clipTop,
          compoundPaddingLeft,compoundPaddingTop,compoundPaddingRight,compoundPaddingBottom);

    if (mShadowRadius != 0) {
        clipLeft += std::min(0, int(mShadowDx - mShadowRadius));
        clipRight+= std::max(0, int(mShadowDx + mShadowRadius));

        clipTop   += std::min(0, int(mShadowDy - mShadowRadius));
        clipBottom+= std::max(0, int(mShadowDy + mShadowRadius));
    }
    canvas.rectangle(clipLeft, clipTop, clipRight-clipLeft, clipBottom-clipTop);
    canvas.clip();

    int voffsetText = 0;
    int voffsetCursor = 0;

    // translate in by our padding
    /* shortcircuit calling getVerticaOffset() */
    if ((mGravity & Gravity::VERTICAL_GRAVITY_MASK) != Gravity::TOP) {
        voffsetText = getVerticalOffset(false);
        voffsetCursor = getVerticalOffset(true);
    }


    canvas.translate(compoundPaddingLeft , extendedPaddingTop + voffsetText);

    const int layoutDirection = getLayoutDirection();
    const int absoluteGravity = Gravity::getAbsoluteGravity(mGravity, layoutDirection);

    if (isMarqueeFadeEnabled()) {
        if (!mSingleLine && (getLineCount() == 1) && canMarquee()
            && ((absoluteGravity & Gravity::HORIZONTAL_GRAVITY_MASK) != Gravity::LEFT)) {
            const int width = mRight-mLeft;
            const int padding = getCompoundPaddingLeft() + getCompoundPaddingRight();
            const float dx = layout->getLineRight(0) - (width - padding);
            canvas.translate(layout->getParagraphDirection(0) * dx, 0.0f);
        }

        if (mMarquee  && mMarquee->isRunning()) {
            float dx = -mMarquee->getScroll();
            canvas.translate(layout->getParagraphDirection(0) * dx, 0.0f);
        }
    }

    const int cursorOffsetVertical = voffsetCursor - voffsetText;
    if( (std::abs(mShadowDx)>0.05f)||(std::abs(mShadowDy)>0.05f)){
        canvas.set_color(mShadowColor);
        canvas.translate(mShadowDx,mShadowDy);
        layout->draw(canvas);
        canvas.translate(-mShadowDx,-mShadowDy);
    }
    canvas.set_color(color);
    layout->draw(canvas);
    //mLayout->getCaretRect(mCaretRect);
    mCaretRect.offset(compoundPaddingLeft+offset, extendedPaddingTop + voffsetText);

    if (mMarquee && mMarquee->shouldDrawGhost()) {
        const float dx = mMarquee->getGhostOffset();
        canvas.translate(layout->getParagraphDirection(0) * dx, 0.0f);
        layout->draw(canvas);//, highlight, mHighlightPaint, cursorOffsetVertical);
    }
    canvas.restore();
}

//////////////////////////////////////////////////////////////////////////////////////////////////
std::string TextView::getAccessibilityClassName()const{
    return "TextView";
}

void TextView::onInitializeAccessibilityEventInternal(AccessibilityEvent& event){
    View::onInitializeAccessibilityEventInternal(event);

    const bool isPassword =false;//hasPasswordTransformationMethod();
    event.setPassword(isPassword);

    if (event.getEventType() == AccessibilityEvent::TYPE_VIEW_TEXT_SELECTION_CHANGED) {
        std::string text = getText();
        event.setFromIndex(0);//Selection.getSelectionStart(mText));
        event.setToIndex(text.length());//Selection.getSelectionEnd(mText));
        event.setItemCount(text.length());
    }
}

void TextView::onInitializeAccessibilityNodeInfoInternal(AccessibilityNodeInfo& info){
    View::onInitializeAccessibilityNodeInfoInternal(info);
    const bool isPassword =false;// hasPasswordTransformationMethod();
#if 0
    info.setPassword(isPassword);
    info.setText(getText());//getTextForAccessibility());
    info.setHintText(mHint);
    info.setShowingHintText(isShowingHint());
    if (mBufferType == BufferType.EDITABLE) {
        info.setEditable(true);
        if (isEnabled()) {
            info.addAction(AccessibilityNodeInfo.AccessibilityAction.ACTION_SET_TEXT);
        }
    }

    if (mEditor != null) {
        info.setInputType(mEditor.mInputType);

        if (mEditor.mError != null) {
            info.setContentInvalid(true);
            info.setError(mEditor.mError);
        }
    }

    if (!TextUtils.isEmpty(mText)) {
        info.addAction(AccessibilityNodeInfo.ACTION_NEXT_AT_MOVEMENT_GRANULARITY);
        info.addAction(AccessibilityNodeInfo.ACTION_PREVIOUS_AT_MOVEMENT_GRANULARITY);
        info.setMovementGranularities(AccessibilityNodeInfo.MOVEMENT_GRANULARITY_CHARACTER
                | AccessibilityNodeInfo.MOVEMENT_GRANULARITY_WORD
                | AccessibilityNodeInfo.MOVEMENT_GRANULARITY_LINE
                | AccessibilityNodeInfo.MOVEMENT_GRANULARITY_PARAGRAPH
                | AccessibilityNodeInfo.MOVEMENT_GRANULARITY_PAGE);
        info.addAction(AccessibilityNodeInfo.ACTION_SET_SELECTION);
        info.setAvailableExtraData(
                Arrays.asList(EXTRA_DATA_TEXT_CHARACTER_LOCATION_KEY));
    }

    if (isFocused()) {
        if (canCopy()) {
            info.addAction(AccessibilityNodeInfo.ACTION_COPY);
        }
        if (canPaste()) {
            info.addAction(AccessibilityNodeInfo.ACTION_PASTE);
        }
        if (canCut()) {
            info.addAction(AccessibilityNodeInfo.ACTION_CUT);
        }
        if (canShare()) {
            info.addAction(new AccessibilityNodeInfo.AccessibilityAction(
                    ACCESSIBILITY_ACTION_SHARE,
                    getResources().getString(com.android.internal.R.string.share)));
        }
        if (canProcessText()) {  // also implies mEditor is not null.
            mEditor.mProcessTextIntentActionsHandler.onInitializeAccessibilityNodeInfo(info);
        }
    }

    // Check for known input filter types.
    const int numFilters = mFilters.length;
    for (int i = 0; i < numFilters; i++) {
        final InputFilter filter = mFilters[i];
        if (filter instanceof InputFilter.LengthFilter) {
            info.setMaxTextLength(((InputFilter.LengthFilter) filter).getMax());
        }
    }
#endif
    if (!isSingleLine()) {
        info.setMultiLine(true);
    }
}
bool TextView::performAccessibilityActionInternal(int action, Bundle* arguments){
    return true;
}
void TextView::sendAccessibilityEventInternal(int eventType){
    LOGD_IF(AccessibilityManager::getInstance(mContext).isEnabled(),"TODO");
    /*if (eventType == AccessibilityEvent.TYPE_VIEW_ACCESSIBILITY_FOCUSED && mEditor != null) {
        mEditor.mProcessTextIntentActionsHandler.initializeAccessibilityActions();
    }*/
    View::sendAccessibilityEventInternal(eventType);
}

void TextView::sendAccessibilityEventUnchecked(AccessibilityEvent& event){
    // Do not send scroll events since first they are not interesting for
    // accessibility and second such events a generated too frequently.
    // For details see the implementation of bringTextIntoView().
    if (event.getEventType() == AccessibilityEvent::TYPE_VIEW_SCROLLED) {
        return;
    }
    View::sendAccessibilityEventUnchecked(event);
}
}/*endof namespace*/
