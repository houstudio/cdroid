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

#ifndef __TEXTVIEW_H__
#define __TEXTVIEW_H__

#include <view/view.h>
#include <core/layout.h>

namespace cdroid {

struct TextWatcher{
    //void beforeTextChanged(CharSequence s,int start,int count, int after)
    CallbackBase<void,const std::wstring&,int,int,int>beforeTextChanged;
    //void onTextChanged(CharSequence s, int start, int before, int count);
    CallbackBase<void,const std::wstring&,int,int,int>onTextChanged;
    //void afterTextChanged(Editable s);
    CallbackBase<void,std::wstring&>afterTextChanged;
};
class TextView : public View{
public:
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
        ColorStateList* mTintList;
        int mTintMode;
        int mDrawableSizeTop, mDrawableSizeBottom, mDrawableSizeLeft, mDrawableSizeRight;
        int mDrawableSizeStart, mDrawableSizeEnd, mDrawableSizeError, mDrawableSizeTemp;

        int mDrawableWidthTop, mDrawableWidthBottom, mDrawableHeightLeft, mDrawableHeightRight;
        int mDrawableHeightStart, mDrawableHeightEnd, mDrawableHeightError, mDrawableHeightTemp;
        int mDrawablePadding;
        Rect mCompoundRect;
    public:
        Drawables(Context*ctx);
        ~Drawables();
        bool hasMetadata()const;
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
    bool mSingleLine;
    int mDeferScroll;
    float mShadowRadius, mShadowDx, mShadowDy;
    float mSpacingMult;
    float mSpacingAdd;
    bool mHorizontallyScrolling;
    ColorStateList *mTextColor;
    ColorStateList *mHintTextColor;
    ColorStateList *mLinkTextColor;
    int mCurTextColor;
    int mCurHintTextColor;
    int mHighlightColor;
    std::vector<TextWatcher>mListeners;

    class Drawables*mDrawables;
    class Marquee*mMarquee;
    int mEllipsize;
    bool mRestartMarquee;
    int mMarqueeFadeMode;
    int mMarqueeRepeatLimit;
    Layout* mSavedMarqueeModeLayout;

    void initView();
    int getLayoutAlignment()const;
    void applyCompoundDrawableTint();
    int getVerticalOffset(bool forceNormal);
    int getBottomVerticalOffset(bool forceNormal);
    void updateTextColors();
    int getDesiredHeight();
    int getDesiredHeight( Layout* layout, bool cap);
    static int desired(Layout*);
    int getBoxHeight(Layout* l);

    bool isMarqueeFadeEnabled();
    bool canMarquee();
    void startMarquee();
    void stopMarquee();
    void startStopMarquee(bool start);
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
    bool mBlinkOn;
    Rect mCaretRect;
    Layout* mLayout;
    Layout* mHintLayout;
    std::wstring& getEditable();
    void setEditable(bool b);
    int getFontSize()const;
    void drawableStateChanged()override;
    void onDetachedFromWindowInternal()override;
    bool verifyDrawable(Drawable* who)const override;
    void onMeasure(int widthMeasureSpec, int heightMeasureSpec)override;
    virtual void onDraw(Canvas& canvas) override;
    virtual int getHorizontalOffsetForDrawables()const;
    void onSizeChanged(int w,int h,int ow,int oh)override;
    void onLayout(bool changed, int left, int top, int w, int h)override;
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
    
    virtual void setText(const std::string&txt);
    const std::string getText()const;
    void  setTextAppearance(const std::string&);
    View& setHint(const std::string&txt)override;
    virtual void setSingleLine(bool single);
    bool isSingleLine()const;
    void setBreakStrategy(int breakStrategy);
    int getBreakStrategy()const;
    int getLineHeight()const;
    void setLineHeight(int height);
    void setTextSize(float size);
    void setTextSize(int unit, float size);
    float getTextSize()const;
    void setTextColor(int color);
    void setTextColor(ColorStateList* colors);
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

    void setMarqueeRepeatLimit(int marqueeLimit);
    int  getMarqueeRepeatLimit() const;
    int  getEllipsize() const;
    void setEllipsize(int ellipsize);

    ColorStateList* getTextColors()const;
    int getCurrentTextColor()const;
    void setHighlightColor(int color);
    int getHighlightColor()const;
    void setHintTextColor(int color);
    void setHintTextColor(ColorStateList* colors);
    ColorStateList* getHintTextColors()const;
    int getCurrentHintTextColor()const;
    void setLinkTextColor(int color);
    void setLinkTextColor(ColorStateList* colors);
    ColorStateList* getLinkTextColors()const;
    void setMinWidth(int minPixels);
    int getMinWidth()const;
    void setMaxWidth(int maxPixels);
    int getMaxWidth()const;

    int getMinHeight()const;
    void setMinHeight(int minPixels);
    void setMaxLines(int maxLines);
    int getMaxLines()const;
    int getMaxHeight()const;
    void setMaxHeight(int maxPixels);
    int getLineCount()const;
    int getLineBounds(int line, Rect&bounds);

    void setCaretPos(int pos);
    bool moveCaret2Line(int line);
    int getCaretPos()const;
    int getGravity()const;
    void setGravity(int gravity);
    int getCompoundPaddingLeft();
    int getCompoundPaddingRight();
    int getCompoundPaddingTop();
    int getCompoundPaddingBottom();
    int getExtendedPaddingTop();
    int getExtendedPaddingBottom();
    int getCompoundPaddingStart();
    int getCompoundPaddingEnd();
    int getTotalPaddingTop();
    int getTotalPaddingBottom();

    void setCompoundDrawablePadding(int pad);
    int getCompoundDrawablePadding()const;
    std::vector<Drawable*>getCompoundDrawables();
    void setCompoundDrawableTintList(ColorStateList* tint);
    ColorStateList* getCompoundDrawableTintList();
    void drawableHotspotChanged(float x,float y)override;
    void setCompoundDrawableTintMode(int tintMode);
    int getCompoundDrawableTintMode();
    void jumpDrawablesToCurrentState()override;
    void invalidateDrawable(Drawable& drawable)override;
    void setCompoundDrawables(Drawable* left,Drawable* top,Drawable* right,Drawable*bottom);
    void setCompoundDrawablesWithIntrinsicBounds(Drawable* left,Drawable* top,Drawable* right,Drawable*bottom);
    void setCompoundDrawablesWithIntrinsicBounds(const std::string& left, const std::string& top,
                const std::string& right,const std::string& bottom);
    int computeHorizontalScrollRange()override;
    int computeVerticalScrollRange()override;

    void addTextChangedListener(TextWatcher watcher);
    void removeTextChangedListener(TextWatcher watcher);
};

}  // namespace cdroid

#endif  //__TEXTVIEW_H__
