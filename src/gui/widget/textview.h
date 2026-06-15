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
#ifndef __TEXTVIEW_H__
#define __TEXTVIEW_H__

#include <view/view.h>
#include <core/typeface.h>
#include <widget/scroller.h>
#include <widget/textwatcher.h>
#include <text/textutils.h>
#include <text/spannablestring.h>
#include <text/boringlayout.h>
#include <text/dynamiclayout.h>
#include <text/transformationmethod.h>
namespace cdroid {
class Layout;
class CompletionInfo;
class PrecomputesText;
class TextView : public View{
private:
    static constexpr int DEFAULT_TYPEFACE = -1;
    static constexpr int SANS = 1;
    static constexpr int SERIF= 2;
    static constexpr int MONOSPACE = 3;
    static constexpr int VERY_WIDE = 1024 * 1024;
    static constexpr int ANIMATED_SCROLL_GAP = 250;
    static constexpr int KEY_EVENT_NOT_HANDLED = 0;
    static constexpr int KEY_EVENT_HANDLED = -1;
    static constexpr int KEY_DOWN_HANDLED_BY_KEY_LISTENER = 1;
    static constexpr int KEY_DOWN_HANDLED_BY_MOVEMENT_METHOD = 2;
public:
    static constexpr int AUTO_SIZE_TEXT_TYPE_NONE = 0;
    static constexpr int AUTO_SIZE_TEXT_TYPE_UNIFORM = 1;
    enum BufferType {
        NORMAL, SPANNABLE, EDITABLE
    };
    class Drawables {
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
    private:
        friend class TextView;
        Drawable* mShowing[4];
        Drawable* mDrawableStart, *mDrawableEnd, *mDrawableError, *mDrawableTemp;
        Drawable* mDrawableLeftInitial, *mDrawableRightInitial;
        bool mIsRtlCompatibilityMode;
        bool mOverride;
        bool mHasTint, mHasTintMode;
        cdroid::RefPtr<ColorStateList> mTintList;
        int mTintMode;
        int mDrawableSizeTop, mDrawableSizeBottom, mDrawableSizeLeft, mDrawableSizeRight;
        int mDrawableSizeStart, mDrawableSizeEnd, mDrawableSizeError, mDrawableSizeTemp;

        int mDrawableWidthTop, mDrawableWidthBottom, mDrawableHeightLeft, mDrawableHeightRight;
        int mDrawableHeightStart, mDrawableHeightEnd, mDrawableHeightError, mDrawableHeightTemp;
        int mDrawablePadding;
        int mDrawableSaved = DRAWABLE_NONE;
        Rect mCompoundRect;
    private:
	void applyErrorDrawableIfNeeded(int layoutDirection);
    public:
        Drawables(Context*ctx);
        ~Drawables();
        bool hasMetadata()const;
	bool resolveWithLayoutDirection(int layoutDirection);
	void setErrorDrawable(Drawable* dr, TextView* tv);
    };
private:
    static constexpr int LINES = 1;
    static constexpr int EMS = LINES;
    static constexpr int PIXELS = 2;
    static constexpr int MARQUEE_FADE_NORMAL=0;
    static constexpr int MARQUEE_FADE_SWITCH_SHOW_ELLIPSIS =1;
    static constexpr int MARQUEE_FADE_SWITCH_SHOW_FADE =2;
    static constexpr int DEFAULT_AUTO_SIZE_MIN_TEXT_SIZE_IN_SP=12;
    static constexpr int DEFAULT_AUTO_SIZE_MAX_TEXT_SIZE_IN_SP = 112;
    static constexpr int DEFAULT_AUTO_SIZE_GRANULARITY_IN_PX = 1;
    static constexpr float UNSET_AUTO_SIZE_UNIFORM_CONFIGURATION_VALUE = -1.f;
    int mGravity;
    int mMaximum;
    int mMinimum;
    int mMaxMode;
    int mMinMode;
    int mMaxWidth;
    int mMinWidth;
    int mOldMaximum;
    int mOldMaxMode;
    int mMaxWidthMode;
    int mMinWidthMode;
    int mDesiredHeightAtMeasure;
    int mShadowColor;
    int mDeferScroll;
    int mAutoSizeTextType;
    float mShadowRadius, mShadowDx, mShadowDy;
    float mSpacingMult;
    float mSpacingAdd;
    int mBreakStrategy;
    int mHyphenationFrequency;
    int mJustificationMode;
    bool mHideHint;
    bool mSingleLine;
    bool mIncludePad;
    bool mHorizontallyScrolling;
    bool mNeedsAutoSizeText;
    bool mRestartMarquee;
    bool mUserSetTextScaleX;
    bool mHighlightPathBogus;
    bool mTextSetFromXmlOrResourceId;
    bool mUseFallbackLineSpacing;
    bool mHasPresetAutoSizeValues;
    bool mPreDrawRegistered;
    bool mPreDrawListenerDetached;
    // This is used to reflect the current user preference for changing font weight and making text
    // more bold.
    int mFontWeightAdjustment;
    TextPaint mTextPaint;
    ViewTreeObserver::OnPreDrawListener mOnPreDrawListener;

    cdroid::RefPtr<ColorStateList> mTextColor;
    cdroid::RefPtr<ColorStateList> mHintTextColor;
    cdroid::RefPtr<ColorStateList> mLinkTextColor;
    int mCurTextColor;
    int mCurHintTextColor;
    int mHighlightColor;
    int mAutoSizeMinTextSizeInPx;
    int mAutoSizeMaxTextSizeInPx;
    int mAutoSizeStepGranularityInPx;
    std::vector<int>mAutoSizeTextSizesInPx;
    std::vector<TextWatcher>mListeners;

    class Drawables*mDrawables;
    class Marquee*mMarquee;
    class CharWrapper* mCharWrapper;
    Drawable* mCursorDrawable;
    TextUtils::TruncateAt mEllipsize;
    int  mMarqueeFadeMode;
    int  mMarqueeRepeatLimit;
    int  mLastLayoutDirection;
    int64_t mLastScroll;
    BoringLayout* mSavedLayout;
    BoringLayout* mSavedHintLayout;
    Layout* mSavedMarqueeModeLayout;
    const TextDirectionHeuristic* mTextDir;
    Scroller*mScroller;
    BoringLayout::Metrics* mBoring;
    BoringLayout::Metrics* mHintBoring;
private:
    void initView();
    void setTextInternal(CharSequence* text);
    bool setupAutoSizeUniformPresetSizesConfiguration();
    void validateAndSetAutoSizeTextTypeUniformConfiguration(float autoSizeMinTextSizeInPx,
            float autoSizeMaxTextSizeInPx, float autoSizeStepGranularityInPx);
    void clearAutoSizeConfiguration();
    std::vector<int> cleanupAutoSizePresetSizes(std::vector<int>&presetValues);
    bool setupAutoSizeText();
    void setTypefaceFromAttrs(Typeface* typeface,const std::string& familyName,
           int typefaceIndex,int style,int weight);
    void resolveStyleAndSetTypeface(Typeface* typeface,int style,int weight);
    void setRelativeDrawablesIfNeeded(Drawable* start, Drawable* end);///
    Layout::Alignment getLayoutAlignment()const;
    void applyCompoundDrawableTint();
    void registerForPreDraw();
    void unregisterForPreDraw();
    int  getVerticalOffset(bool forceNormal);
    int  getBottomVerticalOffset(bool forceNormal);
    void updateTextColors();
    int findLargestTextSizeWhichFits(const RectF& availableSpace);
    bool suggestedSizeFitsInSpace(int suggestedSizeInPx,const RectF& availableSpace);
    int  getDesiredHeight();
    int  getDesiredHeight(Layout* layout, bool cap);
    void getInterestingRect(Rect& r, int line);
    void convertFromViewportToContentCoordinates(Rect&);

    void checkForRelayout();
    bool isShowingHint()const;
    bool bringTextIntoView();
    void autoSizeText();
    bool compressText(float width);
    static int desired(Layout*);
    int  getBoxHeight(Layout* l);
    void prepareDrawableForDisplay(Drawable*d);

    void createEditorIfNeeded();
    void assumeLayout();
    bool isAutoSizeEnabled()const;
    bool isMarqueeFadeEnabled()const;
    void startStopMarquee(bool start);
    float getHorizontalFadingEdgeStrength(float position1, float position2);
    void applySingleLine(bool singleLine, bool applyTransformation, bool changeMaxLines);
    void restartMarqueeIfNeeded();
    void setRawTextSize(float size, bool shouldRequestLayout);
    void setTextSizeInternal(int unit, float size, bool shouldRequestLayout);
    void applyTextAppearance(class TextAppearanceAttributes *atts);
    void setText(CharSequence* text, BufferType type, bool notifyBefore, int oldlen);
    void sendBeforeTextChanged(const std::wstring& text, int start, int before, int after);
    void sendAfterTextChanged(std::wstring& text);
    void sendOnTextChanged(const std::wstring& text, int start, int before, int after);
protected:
    int mEditMode;//0--readonly 1--insert 2--replace
    int mCaretPos;
    int mMaxLength;
    bool mBlinkOn;
    Rect mCaretRect;
    Cairo::RefPtr<Cairo::FontFace>mTypeFace;
    Layout* mLayout;
    Layout* mHintLayout;
    //std::string mHint;
    TransformationMethod* mTransformation;
    CharSequence*mText;
    CharSequence*mHint;
    Spannable*mSpannable;
    PrecomputedText* mPrecomputed;
    CharSequence*mTransformed;
    BufferType mBufferType = BufferType::NORMAL;
    std::wstring& getEditable();
    void setEditable(bool b);
    int getFontSize()const;
    void drawableStateChanged()override;
    bool verifyDrawable(Drawable* who)const override;
    void onMeasure(int widthMeasureSpec, int heightMeasureSpec)override;
    bool supportsAutoSizeText()const;
    bool isPaddingOffsetRequired()const override;
    bool setFrame(int l, int t, int w, int h)override;
    int getLeftPaddingOffset()override;
    int getTopPaddingOffset()override;
    int getBottomPaddingOffset()override;
    int getRightPaddingOffset()override;
    bool canMarquee()const;
    void startMarquee();
    void stopMarquee();
    virtual void onTextChanged(const std::wstring& text, int start, int lengthBefore, int lengthAfter);
    virtual void onSelectionChanged(int selStart, int selEnd);
    void onAttachedToWindow();
    void onDetachedFromWindowInternal()override;
    bool onPreDraw();
    virtual void onDraw(Canvas& canvas) override;
    virtual int getHorizontalOffsetForDrawables()const;
    void onLayout(bool changed, int left, int top, int w, int h)override;
    void onFocusChanged(bool focused, int direction, Rect* previouslyFocusedRect)override;
    void onWindowFocusChanged(bool hasWindowFocus)override;
    void onVisibilityChanged(View& changedView, int visibility)override;
    void viewClicked(class InputMethodManager*);
    void resetResolvedDrawables()override;
    int viewportToContentHorizontalOffset();
    int viewportToContentVerticalOffset();
    float getLeftFadingEdgeStrength()override;
    float getRightFadingEdgeStrength()override;
    Layout* makeSingleLayout(int wantWidth, BoringLayout::Metrics* boring, int ellipsisWidth,
        Layout::Alignment alignment, bool shouldEllipsize, TextUtils::TruncateAt effectiveEllipsize, bool useSaved);
public:
    enum EDITMODE{
        READONLY,
        INSERT,
        REPLACE
    };
    TextView(Context*ctx,const AttributeSet&attrs);
    TextView(int width, int height);
    TextView(const std::string& text, int width, int height);
    ~TextView()override;
    void setAutoSizeTextTypeWithDefaults(int autoSizeTextType);
    void setAutoSizeTextTypeUniformWithConfiguration(int autoSizeMinTextSize,
        int autoSizeMaxTextSize, int autoSizeStepGranularity, int unit);
    void setAutoSizeTextTypeUniformWithPresetSizes(const std::vector<int>& presetSizes, int unit);
    int getAutoSizeTextType()const;
    int getAutoSizeStepGranularity()const;
    int getAutoSizeMinTextSize()const;
    int getAutoSizeMaxTextSize()const;
    std::vector<int> getAutoSizeTextAvailableSizes()const;
    void setEnabled(bool);
    void setTypeface(Typeface* tf);
    void setTypeface(Typeface* tf,int style);///
    Typeface* getTypeface()const;
    int getTypefaceStyle() const;
    virtual void setText(const std::string&txt);
    virtual void setText(CharSequence*txt);
    void setText(const std::vector<char16_t>&text, int start, int len);
    void append(CharSequence* text);
    void append(CharSequence* text, int start, int end);
    const std::string getText()const;
    void setTextCursorDrawable(Drawable*);
    Drawable* getTextCursorDrawable()const;
    void setTextAppearance(const std::string&);
    void setTextAppearance(Context*,const std::string&);
    virtual void setHint(const std::string&txt);
    std::string getHint()const;
    bool bringPointIntoView(int offset);
    bool moveCursorToVisibleOffset();
    void computeScroll()override;
    bool canSelectAllText()const;
    bool selectAllText();
    int getSelectionStart()const;
    int getSelectionEnd()const;
    bool hasSelection()const;
    std::string getSelectedText()const;
    virtual void setSingleLine(bool single);
    bool isSingleLine()const;
    bool hasPasswordTransformationMethod()const;
    void setBreakStrategy(int breakStrategy);
    int getBreakStrategy()const;
    int getLineHeight()const;
    void setLineHeight(int height);
    void setTextSize(float size);
    void setTextSize(int unit, float size);
    float getTextSize()const;
    float getScaledTextSize() const;
    float getTextScaleX()const;
    void setTextScaleX(float);
    void setElegantTextHeight(bool elegant);
    bool isElegantTextHeight()const;
    void setFallbackLineSpacing(bool);
    bool isFallbackLineSpacing()const;
    float getLetterSpacing() const;
    void setLetterSpacing(float letterSpacing);
    void setHyphenationFrequency(int hyphenationFrequency);
    int getHyphenationFrequency()const;
    void setJustificationMode(int justificationMode);
    int getJustificationMode() const;
    void setTextColor(int color);
    void setTextColor(const cdroid::RefPtr<ColorStateList>& colors);
    Layout* getLayout()const;
    Layout* getHintLayout()const;
    void setShadowLayer(float radius, float dx, float dy, int color);
    float getShadowRadius()const;
    float getShadowDx()const;
    float getShadowDy()const;
    int getShadowColor()const;
    const TextPaint& getPaint()const;
    int getPaintFlags() const;
    void setPaintFlags(int);
    void setLineSpacing(float add, float mult);
    float getLineSpacingMultiplier()const;
    float getLineSpacingExtra()const;

    void setPadding(int left, int top, int right, int bottom)override;
    void setPaddingRelative(int start, int top, int end, int bottom)override;
    void setFirstBaselineToTopHeight(int firstBaselineToTopHeight);
    void setLastBaselineToBottomHeight(int lastBaselineToBottomHeight);
    int getFirstBaselineToTopHeight();
    int getLastBaselineToBottomHeight();

    void setIncludeFontPadding(bool includepad);
    bool getIncludeFontPadding()const;
    void setMarqueeRepeatLimit(int marqueeLimit);
    int  getMarqueeRepeatLimit() const;
    TextUtils::TruncateAt getEllipsize() const;
    void setEllipsize(TextUtils::TruncateAt ellipsize);

    const cdroid::RefPtr<ColorStateList> getTextColors()const;
    int getCurrentTextColor()const;
    void setHighlightColor(int color);
    int getHighlightColor()const;
    void setHintTextColor(int color);
    void setHintTextColor(const cdroid::RefPtr<ColorStateList>& colors);
    const cdroid::RefPtr<ColorStateList> getHintTextColors()const;
    int getCurrentHintTextColor()const;
    void setLinkTextColor(int color);
    void setLinkTextColor(const cdroid::RefPtr<ColorStateList>& colors);
    const cdroid::RefPtr<ColorStateList> getLinkTextColors()const;
    void setMinWidth(int minPixels);
    int getMinWidth()const;
    void setMaxWidth(int maxPixels);
    int getMaxWidth()const;
    void setWidth(int pixels);

    int getMinHeight()const;
    void setMinHeight(int minPixels);
    void setMaxLines(int maxLines);
    int getMaxLines()const;
    void setMinLines(int minLines);
    int getMinLines()const;
    int getMaxHeight()const;
    void setMaxHeight(int maxPixels);
    void setHeight(int pixels);
    void setLines(int lines);
    void setMinEms(int minEms);
    int getMinEms()const;
    void setMaxEms(int maxEms);
    int getMaxEms()const;
    void setEms(int ems);
    int getLineCount()const;
    int getLineBounds(int line, Rect&bounds);
    int getBaseline()override;
    int getBaselineOffset();
    void setCaretPos(int pos);
    bool moveCaret2Line(int line);
    int getCaretPos()const;
    int getGravity()const;
    void setGravity(int gravity);
    void setHorizontallyScrolling(bool whether);
    bool isHorizontallyScrollable()const;
    bool getHorizontallyScrolling()const;
    void setSelected(bool selected)override;
    TransformationMethod* getTransformationMethod()const;
    void setTransformationMethod(TransformationMethod*);
    virtual int getCompoundPaddingLeft()const;
    virtual int getCompoundPaddingRight()const;
    virtual int getCompoundPaddingTop()const;
    virtual int getCompoundPaddingBottom()const;
    int getExtendedPaddingTop();
    int getExtendedPaddingBottom();
    virtual int getCompoundPaddingStart();
    virtual int getCompoundPaddingEnd();
    int getTotalPaddingLeft();
    int getTotalPaddingRight();
    int getTotalPaddingStart();
    int getTotalPaddingEnd();
    int getTotalPaddingTop();
    int getTotalPaddingBottom();

    void setCompoundDrawablePadding(int pad);
    int getCompoundDrawablePadding()const;
    std::vector<Drawable*>getCompoundDrawables()const;
    std::vector<Drawable*> getCompoundDrawablesRelative() const;
    void setCompoundDrawableTintList(const cdroid::RefPtr<ColorStateList>& tint);
    const cdroid::RefPtr<ColorStateList> getCompoundDrawableTintList()const;
    void drawableHotspotChanged(float x,float y)override;
    void setCompoundDrawableTintMode(int tintMode);
    int getCompoundDrawableTintMode()const;
    void jumpDrawablesToCurrentState()override;
    void invalidateDrawable(Drawable& drawable)override;
    bool isTextSelectable()const;
    void setTextSelectable(bool);
    void setCompoundDrawables(Drawable* left,Drawable* top,Drawable* right,Drawable*bottom);
    void setCompoundDrawablesWithIntrinsicBounds(Drawable* left,Drawable* top,Drawable* right,Drawable*bottom);
    void setCompoundDrawablesWithIntrinsicBounds(const std::string& left, const std::string& top,
                const std::string& right,const std::string& bottom);
    int computeHorizontalScrollRange()override;
    int computeVerticalScrollRange()override;

    void addTextChangedListener(const TextWatcher& watcher);
    void removeTextChangedListener(const TextWatcher& watcher);
    const TextDirectionHeuristic*getTextDirectionHeuristic()const;
    void onResolveDrawables(int layoutDirection)override;
    bool onTouchEvent(MotionEvent& event)override;
    virtual void onCommitCompletion(CompletionInfo* completion);
    bool useDynamicLayout() const;
    void nullLayouts();
    void makeNewLayout(int wantWidth, int hintWidth, BoringLayout::Metrics* boring,
                    BoringLayout::Metrics* hintBoring,int ellipsisWidth, bool bringIntoView);

    std::string getAccessibilityClassName()const override;
    void onInitializeAccessibilityEventInternal(AccessibilityEvent& event)override;
    void onInitializeAccessibilityNodeInfoInternal(AccessibilityNodeInfo& info)override;
    bool performAccessibilityActionInternal(int action, Bundle* arguments)override;
    void sendAccessibilityEventInternal(int eventType)override;
    void sendAccessibilityEventUnchecked(AccessibilityEvent& event)override;
};

}  // namespace cdroid

#endif  //__TEXTVIEW_H__
