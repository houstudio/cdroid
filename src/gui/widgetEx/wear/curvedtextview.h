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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  0211-130 1 USA
*/
#ifndef __WEAR_CURVEDTEXTVIEW_H__
#define __WEAR_CURVEDTEXTVIEW_H__
#include <string>
#include <core/canvas.h>
#include <core/path.h>
#include <core/rect.h>
#include <core/typeface.h>
#include <text/textpaint.h>
#include <text/textutils.h>
#include <view/view.h>
#include <widgetEx/wear/arclayout.h>

namespace cdroid{

class ColorStateList;
class Configuration;
class TypedArray;

// Line-aligned port of androidx.wear.widget.CurvedTextView
// (CurvedTextView.java:61-1005). A component allowing developers to easily
// write curved text following the curvature of the largest circle that can be
// inscribed in the view. ArcLayout could be used to concatenate multiple
// curved texts, also layout together with other widgets such as icons.
class CurvedTextView:public View,public ArcLayout::Widget{
private:
    static constexpr float UNSET_ANCHOR_DEGREE = -1.f;
    static constexpr int UNSET_ANCHOR_TYPE = -1;
    static constexpr float MIN_SWEEP_DEGREE = 0.f;
    static constexpr float MAX_SWEEP_DEGREE = 359.9f;
    static constexpr float DEFAULT_TEXT_SIZE = 24.f;
    static constexpr int DEFAULT_TEXT_COLOR = Color::WHITE;
    static constexpr int DEFAULT_TEXT_STYLE = Typeface::NORMAL;
    static constexpr bool DEFAULT_CLOCKWISE = true;
    static constexpr int FONT_WEIGHT_MAX = 1000;
    static constexpr float ITALIC_SKEW_X = -0.25f;
    // make 0 degree at 12 o'clock, since canvas assumes 0 degree is 3 o'clock
    static constexpr float ANCHOR_DEGREE_OFFSET = -90.f;

    /** Set of attribute that can be defined in a Text Appearance. (java:633-646) */
    struct TextAppearanceAttributes {
        RefPtr<ColorStateList> mTextColor;   // @Nullable -> nullptr-able RefPtr
        float mTextSize = DEFAULT_TEXT_SIZE;
        std::string mFontFamily;             // @Nullable -> empty string
        bool mFontFamilyExplicit = false;
        int mTypefaceIndex = -1;
        int mTextStyle = DEFAULT_TEXT_STYLE;
        int mFontWeight = -1;
        float mLetterSpacing = 0.f;
        std::string mFontFeatureSettings;    // @Nullable -> empty string
        std::string mFontVariationSettings;  // @Nullable -> empty string
    };

    Path mPath;
    Path mBgPath;
    TextPaint mPaint;
    Rect mBounds;
    Rect mBgBounds;
    bool mDirty = true;
    std::string mTextToDraw;
    float mPathRadius = 0.f;
    float mTextSweepDegrees = 0.f;
    float mBackgroundSweepDegrees = MAX_SWEEP_DEGREE;
    int mLastUsedTextAlignment = -1;
    float mLocalRotateAngle = 0.f;

    int mAnchorType;
    float mAnchorAngleDegrees;
    float mMinSweepDegrees;
    float mMaxSweepDegrees;
    std::string mText;
    float mTextSize = DEFAULT_TEXT_SIZE;
    Typeface* mTypeface = nullptr;
    Typeface* mOriginalTypeface = nullptr;
    int mTextStyle = DEFAULT_TEXT_STYLE;
    int mFontWeight = -1;
    bool mClockwise = DEFAULT_CLOCKWISE;
    int mTextColor = DEFAULT_TEXT_COLOR;
    // @Nullable TruncateAt -> TextUtils::TruncateAt::NONE is the null sentinel
    TextUtils::TruncateAt mEllipsize = TextUtils::TruncateAt::NONE;
    float mLetterSpacing = 0.f;
    std::string mFontFeatureSettings;    // @Nullable -> empty string
    std::string mFontVariationSettings;  // @Nullable -> empty string

    // If true, it means we got the touch_down event and are receiving the touch events that follow.
    bool mHandlingTouch = false;

    float getWidthSelf();
    std::string ellipsize(int ellipsizedWidth);
    void updatePathsIfNeeded(bool withBackground);
    void setTypefaceFromAttrs(const std::string& familyName, int typefaceIndex, int style, int weight);
    void resolveStyleAndSetTypeface(Typeface* tf, int style, int weight);
    Typeface* getAdjustedTypeface(Typeface* tf, int style, int weight);
    void applyTextAppearance(const TextAppearanceAttributes& attributes);
    void readTextAppearance(const TypedArray& appearance, TextAppearanceAttributes& attributes,
            bool isTextAppearance);
    void doUpdate();
    void doRedraw();
public:
    CurvedTextView(Context* context);
    CurvedTextView(Context* context,const AttributeSet* attrs);
    CurvedTextView(Context* context,const AttributeSet* attrs,int defStyle);
    CurvedTextView(Context* context,const AttributeSet* attrs,int defStyle,int defStyleRes);

    // ArcLayout.Widget (java:193-249)
    float getSweepAngleDegrees() override;
    void setSweepAngleDegrees(float angleDegrees) override;
    int getThickness() override;
    void checkInvalidAttributeAsChild() override;
    bool isPointInsideClickArea(float x,float y) override;

    void onSizeChanged(int w,int h,int oldw,int oldh) override;
    void onConfigurationChanged(Configuration& newConfig) override;
    void onMeasure(int widthMeasureSpec,int heightMeasureSpec) override;
    // We only filter events and defer to View::onTouchEvent (java:449-483)
    bool onTouchEvent(MotionEvent& event) override;
    void draw(Canvas& canvas) override;
    void onDraw(Canvas& canvas) override;
    void onInitializeAccessibilityNodeInfo(AccessibilityNodeInfo& info) override;
    void onPopulateAccessibilityEvent(AccessibilityEvent& event) override;

    /** Returns the anchor type for positioning the curved text */
    int getAnchorType() const;
    /** Sets the anchor type for positioning the curved text. */
    void setAnchorType(int value);
    /** Returns the anchor angle used for positioning the text, in degrees. */
    float getAnchorAngleDegrees() const;
    /** Sets the anchor angle used for positioning the text, in degrees. */
    void setAnchorAngleDegrees(float value);
    /** Sets the minimum and maximum sweep angle in degrees for rendering the text. */
    void setSweepRangeDegrees(float minSweep,float maxSweep);
    /** Returns the sweep angle in degrees for rendering the text */
    float getMinSweepDegrees() const;
    /** Returns the maximum sweep angle in degrees for rendering the text */
    float getMaxSweepDegrees() const;
    /** Returns the text to be rendered */
    const std::string& getText() const;
    /** Sets the text to be rendered (null String == empty string) */
    void setText(const std::string& value);
    /** Returns the text size for rendering the text */
    float getTextSize() const;
    /** Sets the text size for rendering the text */
    void setTextSize(float value);
    /** Gets the current Typeface that is used to style the text. */
    Typeface* getTypeface() const;
    /** Sets the typeface and style in which the text should be displayed. */
    void setTypeface(Typeface* value);
    /** Sets the typeface and style in which the text should be displayed. Note
     * that not all Typeface families actually have bold and italic variants */
    void setTypeface(Typeface* tf,int style);
    /** Returns the curved text layout direction */
    bool isClockwise() const;
    /** Sets the curved text layout direction */
    void setClockwise(bool value);
    /** Returns the color for rendering the text */
    int getTextColor() const;
    /** Sets the color for rendering the text */
    void setTextColor(int value);
    /** Returns where, if anywhere, words that are longer than the view is wide
     * should be ellipsized (TruncateAt::NONE == null). */
    TextUtils::TruncateAt getEllipsize() const;
    /** Causes words in the text that are longer than the view's width to be
     * ellipsized. Use TruncateAt::NONE to turn off ellipsizing. */
    void setEllipsize(TextUtils::TruncateAt value);
    /** Returns the text letter-space value in ems. */
    float getLetterSpacing() const;
    /** Sets text letter-spacing in ems. */
    void setLetterSpacing(float value);
    /** Returns the font feature settings (empty == null). */
    const std::string& getFontFeatureSettings() const;
    /** Sets font feature settings (CSS font-feature-settings format). */
    void setFontFeatureSettings(const std::string& value);
    /** Returns TrueType or OpenType font variation settings (empty == null). */
    const std::string& getFontVariationSettings() const;
    /** Sets TrueType or OpenType font variation settings. */
    void setFontVariationSettings(const std::string& value);
};

}/*endof namespace*/
#endif/*__WEAR_CURVEDTEXTVIEW_H__*/
