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
#ifndef __BADGE_DRAWABLE_H__
#define __BADGE_DRAWABLE_H__
#include <core/parcel.h>
#include <core/parcelable.h>
#include <drawable/gradientdrawable.h>
#include <drawable/badgestate.h>
namespace cdroid{
class View;
class Layout;
class FrameLayout;
class BadgeDrawable:public Drawable{// implements TextDrawableDelegate {
private:
    static constexpr int DEFAULT_MAX_BADGE_CHARACTER_COUNT = 4;
    static constexpr int BADGE_NUMBER_NONE = -1;
    static constexpr int MAX_CIRCULAR_BADGE_NUMBER_COUNT = 9;
    /** The font scale threshold to changing the vertical offset of the badge. **/
    static constexpr float FONT_SCALE_THRESHOLD = .3F;

    //static constexpr int DEFAULT_STYLE = R.style.Widget_MaterialComponents_Badge;
    //static constexpr int DEFAULT_THEME_ATTR = R.attr.badgeStyle;
    static constexpr const char* const DEFAULT_EXCEED_MAX_BADGE_NUMBER_SUFFIX = "+";
    static constexpr const char* const DEFAULT_EXCEED_MAX_BADGE_TEXT_SUFFIX = "\u2026";/*...*/
    Context* mContext;
    GradientDrawable* mShapeDrawable;
    //TextDrawableHelper textDrawableHelper;
    Rect mBadgeBounds;
    float mBadgeRadius;
    float mBadgeWithTextRadius;
    float mBadgeWidePadding;
    float mBadgeCenterX;
    float mBadgeCenterY;
    float mCornerRadius;
    float mHalfBadgeWidth;
    float mHalfBadgeHeight;

    int mMaxBadgeNumber;
    
    Layout*mTextLayout;
    BadgeState* mState;
    View* mAnchorView;
    FrameLayout* mCustomBadgeParent;
private:
    void restoreState();
    static void updateAnchorParentToNotClip(View* anchorView);
    void onVisibilityUpdated();
    void onBackgroundColorUpdated();
    void onBadgeTextColorUpdated();
    void onNumberUpdated();
    void onTextUpdated();
    void onMaxBadgeLengthUpdated();
    void onBadgeGravityUpdated();
    void onBadgeTextAppearanceUpdated();
    void onBadgeShapeAppearanceUpdated();
    void onBadgeContentUpdated();
    //static int readColorFromAttributes(Context* context, TypedArray a, int index);
    BadgeDrawable(Context* context,const std::string&badgeResId, const std::string&defStyleAttr,
            const std::string&defStyleRes, BadgeState::State*state);
    void tryWrapAnchorInCompatParent(View* anchorView);
    void updateCenterAndBounds();
    int getTotalVerticalOffsetForState() const;
    int getTotalHorizontalOffsetForState() const;
    void calculateCenterAndBounds(const Rect& anchorRect, View* anchorView);
    void autoAdjustWithinViewBounds(View* anchorView, View* ancestorView);
    void autoAdjustWithinGrandparentBounds(View*anchorView);
    float getTopCutOff(float totalAnchorYOffset) const;
    float getLeftCutOff(float totalAnchorXOffset) const;
    float getBottomCutOff(float ancestorHeight, float totalAnchorYOffset) const;
    float getRightCutoff(float ancestorWidth, float totalAnchorXOffset) const;
    void drawBadgeContent(Canvas& canvas);
    bool hasBadgeContent()const;
    std::string getBadgeContent()const;
    std::string getTextBadgeText()const;
    std::string getNumberBadgeText()const;
    void updateMaxBadgeNumber();
public:
    /** The badge is positioned along the top and end edges of its anchor view */
    static constexpr int TOP_END = Gravity::TOP | Gravity::END;
    static constexpr int TOP_START = Gravity::TOP | Gravity::START;
    static constexpr int BOTTOM_END = Gravity::BOTTOM | Gravity::END;
    static constexpr int BOTTOM_START = Gravity::BOTTOM | Gravity::START;

    /**
     * The badge offset begins at the edge of the anchor.
     */
    static constexpr int OFFSET_ALIGNMENT_MODE_EDGE = 0;
    /**
     * Follows the legacy offset alignment behavior. The horizontal offset begins at a variable
     * permanent inset from the edge of the anchor, and the vertical offset begins at the center
     * of the badge aligned with the edge of the anchor.
     */
    static constexpr int OFFSET_ALIGNMENT_MODE_LEGACY = 1;

    /**
     * The badge's edge is fixed at the start and grows towards the end.
     */
    static constexpr int BADGE_FIXED_EDGE_START = 0;

    /**
     * The badge's edge is fixed at the end and grows towards the start.
     */
    static constexpr int BADGE_FIXED_EDGE_END = 1;

    /** A value to indicate that a badge radius has not been specified. */
    static constexpr int BADGE_RADIUS_NOT_SPECIFIED = -1;

    /** A value to indicate that badge content should not be truncated. */
    static constexpr int BADGE_CONTENT_NOT_TRUNCATED = -2;

    BadgeState::State* getSavedState() const;
   
    static BadgeDrawable* create(Context* context);
    static BadgeDrawable* createFromState(Context* context, BadgeState::State* savedState);
    static BadgeDrawable* createFromResource(Context* context,const std::string& id);
    ~BadgeDrawable()override;
  
    void setVisible(bool visible);
    void setBadgeFixedEdge(int fixedEdge);
    void updateBadgeCoordinates(View* anchorView);
    void updateBadgeCoordinates(View* anchorView, FrameLayout* customBadgeParent);
    FrameLayout* getCustomBadgeParent();
  
    int getBackgroundColor() const;
    void setBackgroundColor(int backgroundColor);
  
    int getBadgeTextColor() const;
    void setBadgeTextColor(int badgeTextColor);
  
    /** Returns whether this badge will display a number. */
    bool hasNumber() const;
    int  getNumber() const;
    void setNumber(int number);
    void clearNumber();
    bool hasText()const;
    std::string getText()const;
    void setText(const std::string&);
    void clearText();
  
    int getMaxCharacterCount() const;
    void setMaxCharacterCount(int maxCharacterCount);
    int getMaxNumber()const;
    void setMaxNumber(int);
    int getBadgeGravity() const;
  
    void setBadgeGravity(int gravity);
    bool isStateful() const override;
  
    void setColorFilter(const cdroid::RefPtr<ColorFilter>& colorFilter) override;
  
    int getAlpha() const override;
    void setAlpha(int alpha)override;
  
    int getOpacity() override;
  
    int getIntrinsicHeight() override;
    int getIntrinsicWidth() override;
    void draw(Canvas& canvas) override;
  
    /**
     * Implements the TextDrawableHelper.TextDrawableDelegate interface.
     *
     * @hide
     */
    void onTextSizeChange();//override;
  
    bool onStateChange(const std::vector<int>& state) override;
    void setContentDescriptionNumberless(const std::string& charSequence);
    void setContentDescriptionExceedsMaxBadgeNumberStringResource(int stringsResource);
    std::string getContentDescription()const;
  
    void setHorizontalOffset(int px);
    int getHorizontalOffset() const;
    void setHorizontalOffsetWithoutText(int px);
    int getHorizontalOffsetWithoutText() const;
    void setHorizontalOffsetWithText(int px);
    int getHorizontalOffsetWithText()const;

    void setVerticalOffset(int px);
    int getVerticalOffset() const;
    void setVerticalOffsetWithoutText(int px);
    int getVerticalOffsetWithoutText()const;
    void setVerticalOffsetWithText(int);
    int getVerticalOffsetWithText()const;
    void setLargeFontVerticalOffsetAdjustment(int px);
    int getLargeFontVerticalOffsetAdjustment() const;
    void setAdditionalVerticalOffset(int px);//
    int getAdditionalVerticalOffset() const;
    void setTextAppearance(const std::string& id);
    void setBadgeWithTextShapeAppearance(const std::string& id);
    void setBadgeWithTextShapeAppearanceOverlay(const std::string& id);
    void setBadgeWithoutTextShapeAppearance(const std::string& id);
    void setBadgeWithoutTextShapeAppearanceOverlay(const std::string& id);
};
}/*endof namespace*/
#endif/*__BADGE_DRAWABLE_H__*/
