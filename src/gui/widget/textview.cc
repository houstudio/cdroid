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
#include <core/layout.h>
#include <core/textutils.h>
#include <porting/cdlog.h>
#include <float.h>

#define VERY_WIDE 1024*1024
#define KEY_EVENT_NOT_HANDLED 0
#define KEY_EVENT_HANDLED -1

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
    //delete mTintList;//tintlist cant destroied!
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
    int mTextColor;
    int mTextColorHint;
    int mTextColorLink;
    ColorStateList* mTextColors = nullptr;
    ColorStateList* mTextColorHints = nullptr;
    ColorStateList* mTextColorLinks = nullptr;
    int mTextSize = 0;
    std::string mFontFamily;
    Typeface* mFontTypeface;
    int mTypefaceIndex = -1;
    int mTextStyle=0;
    int mStyleIndex = -1;
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
    mTextColor = 0xFF000000;
    mTextColors    = nullptr;
    mTextColorHints= nullptr;
    mTextColorLinks= nullptr;
}

void TextAppearanceAttributes::readTextAppearance(Context*ctx,const AttributeSet&atts){
    if(atts.hasAttribute("textColorHighlight"))
        mTextColorHighlight = atts.getColor("textColorHighlight",mTextColorHighlight);
    if(atts.hasAttribute("textColor")){
        try{
            mTextColor = atts.getColorWithException("textColor");
        }catch(std::exception&e){
            mTextColors= atts.getColorStateList("textColor");
        }
    }
    if(atts.hasAttribute("textColorHint")){
        try{
            mTextColorHint = atts.getColorWithException("textColorHint");
        }catch(std::exception&e){
            mTextColorHints= atts.getColorStateList("textColorHint");
        }
    }
    if(atts.hasAttribute("textColorLink")){
        try{
            mTextColorLink = atts.getColorWithException("textColorLink");
        }catch(std::exception&e){
            mTextColorLinks= atts.getColorStateList("textColorLink");
        }
    }
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
    setHeight(attrs.getDimensionPixelSize("height",-1));
    setMinHeight(attrs.getDimensionPixelSize("minHeight", -1));
    setMaxHeight(attrs.getDimensionPixelSize("maxHeight", mMaximum));
    setTextScaleX(attrs.getFloat("textScaleX",mTextScaleX));

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
    setEllipsize(attrs.getInt("ellipsize",std::unordered_map<std::string,int>{
        {"start",Layout::ELLIPSIS_START},{"middle",Layout::ELLIPSIS_MIDDLE},
        {"end" ,Layout::ELLIPSIS_END},{"marquee",Layout::ELLIPSIS_MARQUEE}
      },Layout::ELLIPSIS_NONE));
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
    mHintLayout->setWidth(width);
    mLayout->setWidth(width);
    mLayout->setText(text);
}

void TextView::initView(){
    mDrawables= nullptr;
    mMarquee  = nullptr;
    mScroller = nullptr;
    mSavedMarqueeModeLayout=nullptr;
    mOriginalTypeface = nullptr;
    mMaxWidth = INT_MAX;
    mMinWidth = 0;
    mMaximum  = INT_MAX;
    mMinimum  = 0;
    mSpacingMult= 1.0;
    mSpacingAdd = 0.f;
    mTextScaleX = 1.f;
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
    mLastLayoutDirection = -1;
    mFontWeightAdjustment= 16;//INT_MAX;
    mMarqueeFadeMode = MARQUEE_FADE_NORMAL;
    mHorizontallyScrolling =false;
    mNeedsAutoSizeText = false;
    mUserSetTextScaleX = false;
    mEllipsize = Layout::ELLIPSIS_NONE;
    mAutoSizeTextType = AUTO_SIZE_TEXT_TYPE_NONE;
    mLayout = new Layout(18,1);
    mHintLayout = new Layout(mLayout->getFontSize(),1);
    mLayout->setMultiline(!mSingleLine);
    mHintLayout->setMultiline(!mSingleLine);
    mGravity = Gravity::NO_GRAVITY;
    mTextColor = mHintTextColor = mLinkTextColor =nullptr;
    mHighlightColor= 0x6633B5E5;
    mShadowRadius = .0;
    mShadowDx = .0;
    mShadowDy = .0;
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
    delete mMarquee;
    delete mScroller;
    delete mLayout;
    delete mHintLayout;
    delete mDrawables;
}

#if 0
bool TextView::onPreDraw() {
    if (mLayout == nullptr) {
        assumeLayout();
    }

    if (mMovement != null) {
        /* This code also provides auto-scrolling when a cursor is moved using a
         * CursorController (insertion point or selection limits).
         * For selection, ensure start or end is visible depending on controller's state.
         */
        int curs = getSelectionEnd();
        // Do not create the controller if it is not already created.
        if (mEditor != null && mEditor.mSelectionModifierCursorController != null
                && mEditor.mSelectionModifierCursorController.isSelectionStartDragged()) {
            curs = getSelectionStart();
        }

        /*
         * TODO: This should really only keep the end in view if
         * it already was before the text changed.  I'm not sure
         * of a good way to tell from here if it was.
         */
        if (curs < 0 && (mGravity & Gravity::VERTICAL_GRAVITY_MASK) == Gravity::BOTTOM) {
            curs = mText.length();
        }

        if (curs >= 0) {
            bringPointIntoView(curs);
        }
    } else {
        bringTextIntoView();
    }

    // This has to be checked here since:
    // - onFocusChanged cannot start it when focus is given to a view with selected text (after
    //   a screen rotation) since layout is not yet initialized at that point.
    if (mEditor != null && mEditor.mCreatedWithASelection) {
        mEditor.refreshTextActionMode();
        mEditor.mCreatedWithASelection = false;
    }

    unregisterForPreDraw();

    return true;
}
#endif

void TextView::onDetachedFromWindowInternal(){
    stopMarquee();
    for(int i = 0; mDrawables && ( i<4 );i++){
        Drawable*d = mDrawables->mShowing[i];
        if( d == nullptr)continue;
        unscheduleDrawable(*d);
    }
    View::onDetachedFromWindowInternal();
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

void TextView::applyTextAppearance(class TextAppearanceAttributes *attr){
    if (attr->mTextColors)setTextColor(attr->mTextColors);
    else setTextColor(attr->mTextColor);

    if (attr->mTextColorHints)setHintTextColor(attr->mTextColorHints);
    else setHintTextColor(attr->mTextColorHint);

    if (attr->mTextColorLinks)setLinkTextColor(attr->mTextColorLinks);
    else setLinkTextColor(attr->mTextColorLink);

    if (attr->mTextColorHighlight) setHighlightColor(attr->mTextColorHighlight);

    if (attr->mTextSize != 0) setRawTextSize(attr->mTextSize, true /* shouldRequestLayout */);

    if ((attr->mTypefaceIndex != -1) && !attr->mFontFamilyExplicit) {
        attr->mFontFamily.clear();
    }
    setTypefaceFromAttrs(attr->mFontTypeface, attr->mFontFamily,
            attr->mTypefaceIndex, attr->mStyleIndex, attr->mFontWeight);

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
    if((size!=mLayout->getFontSize())&&shouldRequestLayout){
        mLayout->setFontSize(size);
        mHintLayout->setFontSize(size);
        requestLayout();
    }
}

void TextView::setPadding(int left, int top, int right, int bottom){
    if ((left != mPaddingLeft) || (right != mPaddingRight)
            || (top != mPaddingTop) ||(bottom != mPaddingBottom)) {
        mLayout->relayout(true);
        mHintLayout->relayout(true);
    }
    // the super call will requestLayout()
    View::setPadding(left, top, right, bottom);
    invalidate();
}

void TextView::setPaddingRelative(int start, int top, int end, int bottom){
    if ( (start != getPaddingStart()) || (end != getPaddingEnd())
            || (top != mPaddingTop) || (bottom != mPaddingBottom)) {
        mLayout->relayout(true);//nullLayouts();
        mHintLayout->relayout(true);
    }

    // the super call will requestLayout()
    View::setPaddingRelative(start, top, end, bottom);
    invalidate();
}

void TextView::setFirstBaselineToTopHeight(int firstBaselineToTopHeight){
    const Cairo::FontExtents& fontMetrics = mLayout->getFontExtents();//MetricsInt();
    int fontMetricsTop;
    if (getIncludeFontPadding()) {
        fontMetricsTop = fontMetrics.max_y_advance;
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
    const Cairo::FontExtents& fontMetrics = mLayout->getFontExtents();
    int fontMetricsBottom;
    if (getIncludeFontPadding()) {
        fontMetricsBottom = fontMetrics.descent;//bottom;
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
    return getPaddingTop() - mLayout->getFontExtents().ascent;
}

int TextView::getLastBaselineToBottomHeight(){
    return getPaddingBottom() - mLayout->getFontExtents().descent;
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
    return mLayout->getFontSize();
}

float TextView::getTextScaleX()const{
    return mTextScaleX;
}

void TextView::setTextScaleX(float size){
    if( (size!=mTextScaleX) && (size>FLT_EPSILON) ){
        mTextScaleX = size;
        mUserSetTextScaleX = true;
        if(mLayout){
            requestLayout();
            invalidate();
        }
    }
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
    if(mLayout->setText(txt) && (getVisibility()==View::VISIBLE) ){
        std::wstring&ws=getEditable();
        if(mCaretPos<ws.length())
            mCaretPos = int(ws.length()-1);
        mLayout->setCaretPos(mCaretPos);
        checkForRelayout();
        startStopMarquee(false);
        startStopMarquee(true);
        mLayout->relayout();//use to fix getBaselineError for empty text
    }
}

const std::string TextView::getText()const{
    return mLayout->getString();
}

void TextView::setHint(const std::string& hint){
    View::setHint(hint);
    mHintLayout->setText(hint);
    checkForRelayout();
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
    //mLayout->setWidth(w-mPaddingLeft-mPaddingRight);
    //mHintLayout->setWidth(w-mPaddingLeft-mPaddingRight);
}

void TextView::checkForRelayout() {
    // If we have a fixed width, we can just swap in a new text layout
    // if the text height stays the same or if the view height is fixed.
    if(mLayoutParams==nullptr) return;
    if (( (mLayoutParams->width != LayoutParams::WRAP_CONTENT)
            || (mMaxWidthMode == mMinWidthMode && mMaxWidth == mMinWidth))
            && ((mHintLayout==nullptr) || mHintLayout->getText().empty())
            && (mRight - mLeft - getCompoundPaddingLeft() - getCompoundPaddingRight() > 0)) {
        // Static width, so try making a new text layout.

        const int oldht = mLayout->getHeight();
        const int want = mLayout->getMaxLineWidth();
        const int hintWant = mHintLayout ?mHintLayout->getMaxLineWidth():0;

        /*
         * No need to bring the text into view, since the size is not
         * changing (unless we do the requestLayout(), in which case it
         * will happen at measure).
         */
        /*makeNewLayout(want, hintWant, UNKNOWN_BORING, UNKNOWN_BORING,
                      mRight - mLeft - getCompoundPaddingLeft() - getCompoundPaddingRight(),
                      false);*/
        mLayout->relayout();
        if (mEllipsize != Layout::ELLIPSIS_MARQUEE){//TextUtils.TruncateAt.MARQUEE) {
            // In a fixed-height view, so use our new text layout.
            if (mLayoutParams->height != LayoutParams::WRAP_CONTENT
                    && mLayoutParams->height != LayoutParams::MATCH_PARENT) {
                autoSizeText();
                invalidate();
                return;
            }

            // Dynamic height, but height has stayed the same,
            // so use our new text layout.
            if ( (mLayout->getHeight() == oldht) && (mHintLayout->getText().empty() || (mHintLayout->getHeight() == oldht))) {
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
        //nullLayouts();
        requestLayout();
        invalidate();
    }
}

bool TextView::isShowingHint()const{
    return mLayout->getText().empty()&&(mHintLayout->getText().empty()==false);
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
#if __TODO_
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
#endif
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
#if __TODO_
    switch (layout->getParagraphAlignment(line)) {
    case ALIGN_LEFT:
        grav = 1;   break;
    case ALIGN_RIGHT:
        grav = -1;  break;
    case ALIGN_NORMAL:
        grav = layout->getParagraphDirection(line);
        break;
    case ALIGN_OPPOSITE:
        grav = -layout->getParagraphDirection(line);
        break;
    case ALIGN_CENTER:
    default:   grav = 0;   break;
    }
#endif
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
#if _TODO_
    if (!(mText instanceof Spannable)) {
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
        Selection.setSelection(mSpannable, newStart);
        return true;
    }
#endif
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

void TextView::setSelected(bool selected){
    const bool wasSelected = isSelected();

    View::setSelected(selected);

    if ((selected != wasSelected) && (mEllipsize == Layout::ELLIPSIS_MARQUEE)) {
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
    mLayout->setAlignment(getLayoutAlignment());
    mLayout->setWidth(mRight - mLeft - getCompoundPaddingLeft() - getCompoundPaddingRight());
}

int TextView::getGravity()const{
    return mGravity;
}

void TextView::setHorizontallyScrolling(bool whether) {
    if (mHorizontallyScrolling != whether) {
        mHorizontallyScrolling = whether;
        if (mLayout != nullptr) {
            //nullLayouts();
            requestLayout();
            invalidate();
        }
    }
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
    int baseline = mLayout->getLineBounds(line, bounds);

    int voffset = getExtendedPaddingTop();
    if ((mGravity & Gravity::VERTICAL_GRAVITY_MASK) != Gravity::TOP) {
        voffset += getVerticalOffset(true);
    }
    bounds.offset(getCompoundPaddingLeft(), voffset);
    return baseline + voffset;
}

int TextView::getBaseline(){
    if(mLayout == nullptr)
        return View::getBaseline();
    mLayout->relayout(false);
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
    return std::round(mLayout->getFontExtents().height * mSpacingMult + mSpacingAdd);
    //return mLayout->getLineBottom(0)-mLayout->getLineTop(0);
}

void TextView::setLineHeight(int lineHeight){
    const int fontHeight = mLayout->getFontExtents().height;
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
                : getMeasuredWidth() //- getTotalPaddingLeft() - getTotalPaddingRight();
				 - getCompoundPaddingLeft()-getCompoundPaddingRight();
        const int availableHeight = getMeasuredHeight() - getExtendedPaddingBottom()
                    - getExtendedPaddingTop();

        if (availableWidth <= 0 || availableHeight <= 0) {
            return;
        }
#if 0
        synchronized (TEMP_RECTF) {
            TEMP_RECTF.setEmpty();
            TEMP_RECTF.right = availableWidth;
            TEMP_RECTF.bottom = availableHeight;
            final float optimalTextSize = findLargestTextSizeWhichFits(TEMP_RECTF);
            if (optimalTextSize != getTextSize()) {
                setTextSizeInternal(TypedValue.COMPLEX_UNIT_PX, optimalTextSize,
                        false /* shouldRequestLayout */);

                makeNewLayout(availableWidth, 0 /* hintWidth */, UNKNOWN_BORING, UNKNOWN_BORING,
                        mRight - mLeft - getCompoundPaddingLeft() - getCompoundPaddingRight(),
                        false /* bringIntoView */);
            }
        }
#endif
    }
    // Always try to auto-size if enabled. Functions that do not want to trigger auto-sizing
    // after the next layout pass should set this to false.
    mNeedsAutoSizeText = true;
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

bool TextView::compressText(float width) {
    if (isHardwareAccelerated()) return false;

    // Only compress the text if it hasn't been compressed by the previous pass
    if ((width > 0.0f) && mLayout && (getLineCount() == 1) && !mUserSetTextScaleX
            && (mTextScaleX == 1.0f)) {
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
        //mLayout->setWidth(width- getCompoundPaddingLeft() - getCompoundPaddingRight());
    } else {
        int txtWidth,txtHeight;
        mLayout->setWidth(widthSize/*mRight - mLeft*/ - getCompoundPaddingLeft() - getCompoundPaddingRight());
        mLayout->relayout();
        mHintLayout->relayout();
        txtWidth = mLayout->getMaxLineWidth();//desired(mLayout);
        txtHeight= mLayout->getHeight();
        width = txtWidth+getPaddingLeft() + getPaddingRight();
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
        if (widthMode == MeasureSpec::AT_MOST)
            width = std::min(widthSize, width);
    }

    int want = width - getCompoundPaddingLeft() - getCompoundPaddingRight();
    int unpaddedWidth = want;

    if (mHorizontallyScrolling) want = VERY_WIDE;

    int hintWant = want;
    int hintWidth = (mHintLayout == nullptr) ? hintWant : mHintLayout->getMaxLineWidth();

    mLayout->setWidth(want);
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
        bool italic = (style & Typeface::ITALIC) != 0;
        setTypeface(Typeface::create(typeface, weight, italic));
    } else {
        setTypeface(typeface, style);
    }
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
        int typefaceStyle = tf ? tf->getStyle() : 0;
        int need = style & ~typefaceStyle;
        //mTextPaint.setFakeBoldText((need & Typeface::BOLD) != 0);
        //mTextPaint.setTextSkewX((need & Typeface::ITALIC) != 0 ? -0.25f : 0);
    } else {
        //mTextPaint.setFakeBoldText(false);
        //mTextPaint.setTextSkewX(0);
        setTypeface(tf);
    }
}

void TextView::setTypeface(Typeface* tf){
    mOriginalTypeface = tf;
    if ( (mFontWeightAdjustment != 0)
            && (mFontWeightAdjustment != INT_MAX)/*Configuration::FONT_WEIGHT_ADJUSTMENT_UNDEFINED*/) {
        if (tf == nullptr) {
            tf = Typeface::DEFAULT;
        } else {
            const int newWeight = std::min(
                    std::max(tf->getWeight() + mFontWeightAdjustment, (int)FontStyle::FONT_WEIGHT_MIN),
                    (int)FontStyle::FONT_WEIGHT_MAX);
            const int typefaceStyle = tf ? tf->getStyle() : 0;
            const bool italic = (typefaceStyle & Typeface::ITALIC) != 0;
            tf = Typeface::create(tf, newWeight, italic);
        }
    }
    if ( tf && mLayout && (tf!=mLayout->getTypeface()) ){
        if (mLayout != nullptr) {
            //nullLayouts();
            mLayout->setTypeface(tf);
            requestLayout();
            invalidate();
        }
    }
}

Typeface*TextView::getTypeface()const{
    return mOriginalTypeface;
}

int TextView::getTypefaceStyle() const{
    return mOriginalTypeface?mOriginalTypeface->getStyle():Typeface::NORMAL;
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

void TextView::drawableStateChanged(){
    View::drawableStateChanged();

    if ((mTextColor && mTextColor->isStateful())
            || (mHintTextColor && mHintTextColor->isStateful())
            || (mLinkTextColor && mLinkTextColor->isStateful())) {
        updateTextColors();
    }
    if (mDrawables) {
        const std::vector<int>& state = getDrawableState();
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

void TextView::updateTextColors(){
    bool inval = false;
    int color;
    const std::vector<int>&drawableState = getDrawableState();
    if (mTextColor) {
        color = mTextColor->getColorForState(drawableState,0);
        LOGV("%p:%d change color %x->%x",this,mID,color,mCurTextColor);
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
            inval = !mLayout->getText().empty();
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

void TextView::applySingleLine(bool singleLine, bool applyTransformation, bool changeMaxLines) {
    mSingleLine = singleLine;
    if (singleLine) {
        setLines(1);
        setHorizontallyScrolling(true);
        /*if (applyTransformation) {
            setTransformationMethod(SingleLineTransformationMethod.getInstance());
        }*/
    } else {
        if (changeMaxLines) {
            setMaxLines(INT_MAX);//Integer.MAX_VALUE);
        }
        setHorizontallyScrolling(false);
        /*if (applyTransformation) {
            setTransformationMethod(null);
        }*/
    }
}

void TextView::setTextColor(int color){
    mCurTextColor = color;
    invalidate();
    //setTextColor(ColorStateList::valueOf(color));
}

void TextView::setTextColor(const ColorStateList* colors){
    if(mTextColor!=colors){
        mTextColor = colors;
        updateTextColors();
    }
}

const ColorStateList* TextView::getTextColors()const{
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

void TextView::setHintTextColor(const ColorStateList* colors){
    if(mHintTextColor!=colors){
        mHintTextColor = colors;
        updateTextColors();
    }
}

const ColorStateList* TextView::getHintTextColors()const{
    return mHintTextColor;
}

int TextView::getCurrentHintTextColor()const{
    return mHintTextColor != nullptr ? mCurHintTextColor : mCurTextColor;
}


void TextView::setLinkTextColor(int color){
    //setLinkTextColor(ColorStateList::valueOf(color));
}

void TextView::setLinkTextColor(const ColorStateList* colors){
    if(mLinkTextColor!=colors){
        mLinkTextColor = colors;
        updateTextColors();
    }
}

const ColorStateList* TextView::getLinkTextColors()const{
    return mLinkTextColor;
}

void TextView::applyCompoundDrawableTint(){
    if (mDrawables == nullptr) return;
    if ( (mDrawables->mTintList==nullptr)&&(mDrawables->mHasTintMode==false) )return ;

    const ColorStateList* tintList = mDrawables->mTintList;
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

int TextView::getExtendedPaddingTop()const{
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

int TextView::getExtendedPaddingBottom()const{
    if(mMaxMode !=LINES){
        return getCompoundPaddingBottom();
    }

    //if (mLayout == nullptr)assumeLayout();
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

void TextView::setCompoundDrawableTintList(const ColorStateList* tint){
    if (mDrawables == nullptr) {
        mDrawables = new Drawables(getContext());
    }
    if(mDrawables->mTintList!=tint){
        mDrawables->mTintList = tint;
        applyCompoundDrawableTint();
    }
}

const ColorStateList* TextView::getCompoundDrawableTintList()const{
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

int TextView::getBoxHeight(Layout* l){
    Insets opticalInsets = isLayoutModeOptical((View*)mParent) ? getOpticalInsets() : Insets::NONE;
    const int padding = (l ==mHintLayout)
	    ?getCompoundPaddingTop() + getCompoundPaddingBottom()
            :getExtendedPaddingTop() + getExtendedPaddingBottom();
    int measuedHeight=getMeasuredHeight();
    if(measuedHeight==0)measuedHeight=getHeight();
    return measuedHeight - padding +opticalInsets.top + opticalInsets.bottom;
}

int TextView::getVerticalOffset(bool forceNormal){
    int voffset = 0;
    const int gravity = mGravity & Gravity::VERTICAL_GRAVITY_MASK;
    Layout* l = mLayout;
    if (gravity != Gravity::TOP) {
        const int boxht = getBoxHeight(l);
        const int textht = mLayout->getHeight();//LineHeight(true);
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

bool TextView::canSelectAllText()const{
    mLayout->setSelection(-1,-1);
    return false;
}

bool TextView::selectAllText(){
    const int length = (int)mLayout->getText().length();
    mLayout->setSelection(0,length);
    return false;
}

int TextView::getTotalPaddingTop(){
    return getExtendedPaddingTop() + getVerticalOffset(true);
}

int TextView::getTotalPaddingBottom(){
    return getExtendedPaddingBottom() + getBottomVerticalOffset(true);
}

int TextView::getSelectionStart()const{
    return mLayout ? mLayout->getSelectionStart():-1;
}

int TextView::getSelectionEnd()const{
    return  mLayout ? mLayout->getSelectionEnd():-1;
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
     const std::wstring &text =mLayout->getText();
     std::wstring ret = start<end?text.substr(start,end):text.substr(end,start);
     return TextUtils::unicode2utf8(ret);
}

void TextView::setSingleLine(bool single){
    mSingleLine = single;
    mLayout->setMultiline(!single);
    mLayout->relayout();
    applySingleLine(single,true,true);
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
    return (mEllipsize == Layout::ELLIPSIS_MARQUEE) && (mMarqueeFadeMode != MARQUEE_FADE_SWITCH_SHOW_ELLIPSIS);
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
    if (mEllipsize == Layout::ELLIPSIS_MARQUEE) {
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
    const int vspace = getHeight()- compoundPaddingBottom - compoundPaddingTop;
    const int hspace = getWidth() - compoundPaddingRight - compoundPaddingLeft;
    Drawables* dr = mDrawables;
    mLayout->setWidth(hspace);
    mLayout->setTextDirection(getTextDirection());
    mHintLayout->setWidth(hspace);
    mHintLayout->setTextDirection(getTextDirection());
    mHintLayout->setAlignment(getLayoutAlignment());
    mLayout->setAlignment(getLayoutAlignment());
    mLayout->relayout();
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
    // Text
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
    int color=mCurTextColor;
    Layout*layout=mLayout;
    if(getText().empty()&&mHint.length()){
        color = mCurHintTextColor;
        layout= mHintLayout;
        mHintLayout->relayout();
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
    if(mUserSetTextScaleX)
        canvas.scale(mTextScaleX,1.f);
    if( (std::abs(mShadowDx)>0.05f)||(std::abs(mShadowDy)>0.05f)){
        canvas.set_color(mShadowColor);
        canvas.translate(mShadowDx,mShadowDy);
        layout->draw(canvas);
        canvas.translate(-mShadowDx,-mShadowDy);
    }
    canvas.set_color(color);
    layout->draw(canvas);
    mLayout->getCaretRect(mCaretRect);
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
    info.setPassword(isPassword);
    info.setText(getText());//getTextForAccessibility());
    info.setHintText(mHint);
    info.setShowingHintText(isShowingHint());
#if 0
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
    LOGD("TODO");
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
    LOGD("TODO");
    // Do not send scroll events since first they are not interesting for
    // accessibility and second such events a generated too frequently.
    // For details see the implementation of bringTextIntoView().
    if (event.getEventType() == AccessibilityEvent::TYPE_VIEW_SCROLLED) {
        return;
    }
    View::sendAccessibilityEventUnchecked(event);
}
}/*endof namespace*/
