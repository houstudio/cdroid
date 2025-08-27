#ifndef __SWITCH_H__
#define __SWITCH_H__
#include <widget/compoundbutton.h>

namespace cdroid{

class Switch:public CompoundButton{
private:
    static constexpr int THUMB_ANIMATION_DURATION = 250;

    static constexpr int TOUCH_MODE_IDLE = 0;
    static constexpr int TOUCH_MODE_DOWN = 1;
    static constexpr int TOUCH_MODE_DRAGGING = 2;
    // Enum for the "typeface" XML parameter.
    static constexpr int SANS = 1;
    static constexpr int SERIF = 2;
    static constexpr int MONOSPACE = 3;
    Drawable* mThumbDrawable;
    const ColorStateList* mThumbTintList;
    //BlendMode mThumbBlendMode = null;
    bool mHasThumbTint = false;
    bool mHasThumbTintMode = false;

    Drawable* mTrackDrawable;
    const ColorStateList* mTrackTintList;
    //BlendMode* mTrackBlendMode = null;
    bool mHasTrackTint = false;
    bool mHasTrackTintMode = false;

    int mThumbTextPadding;
    int mSwitchMinWidth;
    int mSwitchPadding;
    bool mSplitTrack;
    std::string mTextOn;
    std::string mTextOff;
    bool mShowText;
    bool mUseFallbackLineSpacing;

    int mTouchMode;
    int mTouchSlop;
    float mTouchX;
    float mTouchY;
    VelocityTracker* mVelocityTracker;
    int mMinFlingVelocity;

    float mThumbPosition;

    /**
     * Width required to draw the switch track and thumb. Includes padding and
     * optical bounds for both the track and thumb.
     */
    int mSwitchWidth;

    /**
     * Height required to draw the switch track and thumb. Includes padding and
     * optical bounds for both the track and thumb.
     */
    int mSwitchHeight;

    /**
     * Width of the thumb's content region. Does not include padding or
     * optical bounds.
     */
    int mThumbWidth;

    /** Left bound for drawing the switch track and thumb. */
    int mSwitchLeft;

    /** Top bound for drawing the switch track and thumb. */
    int mSwitchTop;

    /** Right bound for drawing the switch track and thumb. */
    int mSwitchRight;

    /** Bottom bound for drawing the switch track and thumb. */
    int mSwitchBottom;

    const ColorStateList* mTextColors;
    Layout* mOnLayout;
    Layout* mOffLayout;
    //TransformationMethod2 mSwitchTransformationMethod;
    ObjectAnimator* mPositionAnimator;
    class THUMB_POS;
private:
    void init();
    void setSwitchTypefaceByIndex(int typefaceIndex, int styleIndex);
    void applyTrackTint();
    void applyThumbTint();
    Layout* makeLayout(const std::string& text);
    bool hitThumb(float x, float y);
    void cancelSuperTouch(MotionEvent& ev);
    void stopDrag(MotionEvent& ev);
    void animateThumbToCheckedState(bool newCheckedState);
    void cancelPositionAnimator();
    bool getTargetCheckedState()const;
    void setThumbPosition(float position);
    int getThumbOffset();
    int getThumbScrollRange();
protected:
    void doSetChecked(bool checked)override;
    void onDetachedFromWindow()override;
    void onLayout(bool changed, int left, int top, int width, int height)override;
    void onDraw(Canvas&)override;
    std::vector<int>onCreateDrawableState(int)override;
    void drawableStateChanged()override;
    bool verifyDrawable(Drawable* who)const override;
public:
    Switch(int w,int h);
    Switch(Context* context,const AttributeSet& attrs);
    ~Switch();
    void setSwitchTextAppearance(Context* context,const std::string&resid);
    void setSwitchTypeface(Typeface* tf, int style);
    void setSwitchTypeface(Typeface* tf);
    void setSwitchPadding(int pixels);
    int  getSwitchPadding()const;
    void setSwitchMinWidth(int pixels);
    int  getSwitchMinWidth()const;
    void setThumbTextPadding(int pixels);
    int  getThumbTextPadding()const;
    void setTrackDrawable(Drawable* track);
    void setTrackResource(const std::string& resId);
    Drawable* getTrackDrawable();
    void setTrackTintList(const ColorStateList* tint);
    const ColorStateList* getTrackTintList();
    void setTrackTintMode(PorterDuffMode tintMode);
    PorterDuffMode getTrackTintMode()const;
    void setThumbDrawable(Drawable* thumb);
    void setThumbResource(const std::string& resId);
    Drawable* getThumbDrawable();
    void setThumbTintList(const ColorStateList* tint);
    const ColorStateList* getThumbTintList()const;
    void setThumbTintMode(PorterDuffMode tintMode);
    PorterDuffMode getThumbTintMode()const;
    void setSplitTrack(bool splitTrack);
    bool getSplitTrack()const;

    std::string getTextOn()const;
    void setTextOn(const std::string&);
    std::string getTextOff()const;
    void setTextOff(const std::string&);

    bool getShowText()const;
    void setShowText(bool showText);
    std::string getAccessibilityClassName()const override;
    void onMeasure(int widthMeasureSpec, int heightMeasureSpec)override;
    void onPopulateAccessibilityEventInternal(AccessibilityEvent& event)override;
    void onInitializeAccessibilityNodeInfoInternal(AccessibilityNodeInfo& info)override;
    bool onTouchEvent(MotionEvent& ev)override;
    std::string getButtonStateDescription();//override;
    void draw(Canvas&)override;

    int getCompoundPaddingLeft()const override;
    int getCompoundPaddingRight()const override;
    void drawableHotspotChanged(float x, float y)override;
    void jumpDrawablesToCurrentState()override;
};

}//endof namespace

#endif
