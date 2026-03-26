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
namespace cdroid{
class View;
class Layout;
class FrameLayout;
class BadgeDrawable:public Drawable{// implements TextDrawableDelegate {
public:
    class SavedState:public Parcelable {
    private: 
        friend BadgeDrawable;
        int mBackgroundColor;
        int mBadgeTextColor;
        int mAlpha = 255;
        int mNumber = BADGE_NUMBER_NONE;
        int mMaxCharacterCount;
        std::string contentDescriptionNumberless;
        int contentDescriptionQuantityStrings;
        int contentDescriptionExceedsMaxBadgeNumberRes;
        int mBadgeGravity;
        bool mIsVisible;
  
        int mHorizontalOffset;
        int mVerticalOffset;
    public:
         SavedState(Context* context);
         SavedState(Parcel& in);
         int describeContents();
         void writeToParcel(Parcel& dest, int flags);
    };
private:
    static constexpr int DEFAULT_MAX_BADGE_CHARACTER_COUNT = 4;
    static constexpr int BADGE_NUMBER_NONE = -1;
    static constexpr int MAX_CIRCULAR_BADGE_NUMBER_COUNT = 9;
    //static constexpr int DEFAULT_STYLE = R.style.Widget_MaterialComponents_Badge;
    //static constexpr int DEFAULT_THEME_ATTR = R.attr.badgeStyle;
    //static final String DEFAULT_EXCEED_MAX_BADGE_NUMBER_SUFFIX = "+";
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
    SavedState* mSavedState;
    View* mAnchorView;
    FrameLayout* mCustomBadgeParent;
private:
    static BadgeDrawable* createFromAttributes(Context* context,const AttributeSet& attrs,int defStyleAttr, int defStyleRes);
    void restoreFromSavedState(SavedState& savedState);
    void loadDefaultStateFromAttributes(Context* context,const AttributeSet& attrs, int defStyleAttr, int defStyleRes);
    //static int readColorFromAttributes(Context* context, TypedArray a, int index);
    BadgeDrawable(Context* context);
    void tryWrapAnchorInCompatParent(View* anchorView);
    static void updateAnchorParentToNotClip(View* anchorView);
    void setTextAppearanceResource(const std::string& id);
    //void setTextAppearance(TextAppearance& textAppearance);
    void updateCenterAndBounds();
    void calculateCenterAndBounds(Context* context,const Rect& anchorRect, View* anchorView);
    void drawText(Canvas& canvas);
    std::string getBadgeText();
    void updateMaxBadgeNumber();
public:
    /** The badge is positioned along the top and end edges of its anchor view */
    static constexpr int TOP_END = Gravity::TOP | Gravity::END;
    static constexpr int TOP_START = Gravity::TOP | Gravity::START;
    static constexpr int BOTTOM_END = Gravity::BOTTOM | Gravity::END;
    static constexpr int BOTTOM_START = Gravity::BOTTOM | Gravity::START;

    SavedState* getSavedState() const;
   
    static BadgeDrawable* create(Context* context);
    static BadgeDrawable* createFromSavedState(Context* context, SavedState* savedState);
    static BadgeDrawable* createFromResource(Context* context,const std::string& id);
    ~BadgeDrawable()override;
  
    void setVisible(bool visible);
  
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
  
    int getMaxCharacterCount() const;
    void setMaxCharacterCount(int maxCharacterCount);
  
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
    void setVerticalOffset(int px);
    int getVerticalOffset() const;  
};
}/*endof namespace*/
#endif/*__BADGE_DRAWABLE_H__*/
