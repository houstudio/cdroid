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
#include <core/layout.h>
#include <core/typeface.h>
#include <widget/scroller.h>
#include <widget/textwatcher.h>

namespace cdroid {

class CompletionInfo;

class TextView : public View{
private:
    static constexpr int DEFAULT_TYPEFACE = -1;
    static constexpr int SANS = 1;
    static constexpr int SERIF= 2;
    static constexpr int MONOSPACE = 3;
    static constexpr int ANIMATED_SCROLL_GAP = 250;
public:
    static constexpr int AUTO_SIZE_TEXT_TYPE_NONE = 0;
    static constexpr int AUTO_SIZE_TEXT_TYPE_UNIFORM = 1;
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
        const ColorStateList* mTintList;
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
    int mGravity;
    int mMaximum;
    int mMinimum;
    int mMaxMode;
    int mMinMode;
    int mMaxWidth;
    int mMinWidth;
    int mMaxWidthMode;
    int mMinWidthMode;
    int mDesiredHeightAtMeasure;
    int mShadowColor;
    int mDeferScroll;
    int mAutoSizeTextType;
    float mShadowRadius, mShadowDx, mShadowDy;
    float mSpacingMult;
    float mSpacingAdd;
    float mTextScaleX;
    bool mSingleLine;
    bool mIncludePad;
    bool mHorizontallyScrolling;
    bool mNeedsAutoSizeText;
    bool mRestartMarquee;
    bool mUserSetTextScaleX;
    // This is used to reflect the current user preference for changing font weight and making text
    // more bold.
    int mFontWeightAdjustment;
    Typeface* mOriginalTypeface;

    const ColorStateList *mTextColor;
    const ColorStateList *mHintTextColor;
    const ColorStateList *mLinkTextColor;
    int mCurTextColor;
    int mCurHintTextColor;
    int mHighlightColor;
    std::vector<TextWatcher>mListeners;

    class Drawables*mDrawables;
    class Marquee*mMarquee;
    int  mEllipsize;
    int  mMarqueeFadeMode;
    int  mMarqueeRepeatLimit;
    int  mLastLayoutDirection;
    int64_t mLastScroll;
    Layout* mSavedMarqueeModeLayout;
    Scroller*mScroller;
private:
    void initView();
    int  getLayoutAlignment()const;
    void applyCompoundDrawableTint();
    int  getVerticalOffset(bool forceNormal);
    int  getBottomVerticalOffset(bool forceNormal);
    void updateTextColors();
    int  getDesiredHeight();
    int  getDesiredHeight( Layout* layout, bool cap);
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

    bool isAutoSizeEnabled()const;
    bool isMarqueeFadeEnabled()const;
    void startStopMarquee(bool start);
    float getHorizontalFadingEdgeStrength(float position1, float position2);
    void applySingleLine(bool singleLine, bool applyTransformation, bool changeMaxLines);
    void setTypefaceFromAttrs(Typeface* typeface,const std::string& familyName,
           int typefaceIndex,int style,int weight);
    void resolveStyleAndSetTypeface(Typeface* typeface,int style,int weight);
    void restartMarqueeIfNeeded();
    void setRawTextSize(float size, bool shouldRequestLayout);
    void setTextSizeInternal(int unit, float size, bool shouldRequestLayout);
    void applyTextAppearance(class TextAppearanceAttributes *atts);
    void setRelativeDrawablesIfNeeded(Drawable* start, Drawable* end) ;
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
    std::wstring& getEditable();
    void setEditable(bool b);
    int getFontSize()const;
    void drawableStateChanged()override;
    void onDetachedFromWindowInternal()override;
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
public:
    enum EDITMODE{
       READONLY,
       INSERT,
       REPLACE
    };
    TextView(Context*ctx,const AttributeSet&attrs);
    TextView(int width, int height);
    TextView(const std::string& text, int width, int height);
    virtual ~TextView();
    void setTypeface(Typeface* tf);
    void setTypeface(Typeface* tf,int style);
    Typeface* getTypeface()const;
    int getTypefaceStyle() const;
    virtual void setText(const std::string&txt);
    const std::string getText()const;
    void  setTextAppearance(const std::string&);
    void  setTextAppearance(Context*,const std::string&);
    void setHint(const std::string&txt)override;
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
    void setBreakStrategy(int breakStrategy);
    int getBreakStrategy()const;
    int getLineHeight()const;
    void setLineHeight(int height);
    void setTextSize(float size);
    void setTextSize(int unit, float size);
    float getTextSize()const;
    float getTextScaleX()const;
    void setTextScaleX(float);
    void setTextColor(int color);
    void setTextColor(const ColorStateList* colors);
    Layout* getLayout()const;
    Layout* getHintLayout()const;
    void setShadowLayer(float radius, float dx, float dy, int color);
    float getShadowRadius()const;
    float getShadowDx()const;
    float getShadowDy()const;
    int getShadowColor()const;
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
    int  getEllipsize() const;
    void setEllipsize(int ellipsize);

    const ColorStateList* getTextColors()const;
    int getCurrentTextColor()const;
    void setHighlightColor(int color);
    int getHighlightColor()const;
    void setHintTextColor(int color);
    void setHintTextColor(const ColorStateList* colors);
    const ColorStateList* getHintTextColors()const;
    int getCurrentHintTextColor()const;
    void setLinkTextColor(int color);
    void setLinkTextColor(const ColorStateList* colors);
    const ColorStateList* getLinkTextColors()const;
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
    bool getHorizontallyScrolling()const;
    void setSelected(bool selected)override;
    virtual int getCompoundPaddingLeft()const;
    virtual int getCompoundPaddingRight()const;
    virtual int getCompoundPaddingTop()const;
    virtual int getCompoundPaddingBottom()const;
    int getExtendedPaddingTop()const;
    int getExtendedPaddingBottom()const;
    virtual int getCompoundPaddingStart();
    virtual int getCompoundPaddingEnd();
    int getTotalPaddingTop();
    int getTotalPaddingBottom();

    void setCompoundDrawablePadding(int pad);
    int getCompoundDrawablePadding()const;
    std::vector<Drawable*>getCompoundDrawables()const;
    std::vector<Drawable*> getCompoundDrawablesRelative() const;
    void setCompoundDrawableTintList(const ColorStateList* tint);
    const ColorStateList* getCompoundDrawableTintList()const;
    void drawableHotspotChanged(float x,float y)override;
    void setCompoundDrawableTintMode(int tintMode);
    int getCompoundDrawableTintMode()const;
    void jumpDrawablesToCurrentState()override;
    void invalidateDrawable(Drawable& drawable)override;
    void setCompoundDrawables(Drawable* left,Drawable* top,Drawable* right,Drawable*bottom);
    void setCompoundDrawablesWithIntrinsicBounds(Drawable* left,Drawable* top,Drawable* right,Drawable*bottom);
    void setCompoundDrawablesWithIntrinsicBounds(const std::string& left, const std::string& top,
                const std::string& right,const std::string& bottom);
    int computeHorizontalScrollRange()override;
    int computeVerticalScrollRange()override;

    void addTextChangedListener(const TextWatcher& watcher);
    void removeTextChangedListener(const TextWatcher& watcher);
    void onResolveDrawables(int layoutDirection)override;
    bool onTouchEvent(MotionEvent& event)override;
    virtual void onCommitCompletion(CompletionInfo* completion);

    std::string getAccessibilityClassName()const override;
    void onInitializeAccessibilityEventInternal(AccessibilityEvent& event)override;
    void onInitializeAccessibilityNodeInfoInternal(AccessibilityNodeInfo& info)override;
    bool performAccessibilityActionInternal(int action, Bundle* arguments)override;
    void sendAccessibilityEventInternal(int eventType)override;
    void sendAccessibilityEventUnchecked(AccessibilityEvent& event)override;
};

}  // namespace cdroid

#endif  //__TEXTVIEW_H__
