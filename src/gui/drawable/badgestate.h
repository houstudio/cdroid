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
#ifndef __BADGE_STATE_H__
#define __BADGE_STATE_H__
namespace cdroid{
class BadgeState {
public:
    class State;
private:
    State* overridingState;
    State* currentState ;
    friend class BadgeDrawable;
protected:
    float mBadgeRadius;
    float mBadgeWithTextRadius;
    float mBadgeWidth;
    float mBadgeHeight;
    float mBadgeWithTextWidth;
    float mBadgeWithTextHeight;
    int mHorizontalInset;
    int mHorizontalInsetWithText;
    int mOffsetAlignmentMode;
    int mBadgeFixedEdge;
private:
    AttributeSet generateTypedArray(Context* context, const std::string& badgeResId,
        const std::string& defStyleAttr, const std::string& defStyleRes);
public:
    BadgeState(Context* context, const std::string& badgeResId,const std::string& defStyleAttr,
            const std::string& defStyleRes,State* storedState);
    ~BadgeState();
    State* getOverridingState() const;
    bool isVisible() const;
    void setVisible(bool visible);
    bool hasNumber() const;
    int getNumber() const;

    void setNumber(int number);
    void clearNumber();

    bool hasText() const;
    const std::string getText() const;
    void setText(const std::string& text);
    void clearText();

    int getAlpha() const;
    void setAlpha(int alpha);

    int getMaxCharacterCount() const;
    void setMaxCharacterCount(int maxCharacterCount);
    int getMaxNumber() const;
    void setMaxNumber(int maxNumber);

    int getBackgroundColor() const;

    void setBackgroundColor(int backgroundColor);

    int getBadgeTextColor() const;
    void setBadgeTextColor(int badgeTextColor);

    std::string getTextAppearanceResId() const;
    void setTextAppearanceResId(const std::string& textAppearanceResId);

    std::string getBadgeShapeAppearanceResId()const;
    void setBadgeShapeAppearanceResId(const std::string& shapeAppearanceResId);

    std::string getBadgeShapeAppearanceOverlayResId() const;
    void setBadgeShapeAppearanceOverlayResId(const std::string& shapeAppearanceOverlayResId);

    std::string getBadgeWithTextShapeAppearanceResId() const;
    void setBadgeWithTextShapeAppearanceResId(const std::string& shapeAppearanceResId);

    std::string getBadgeWithTextShapeAppearanceOverlayResId() const;
    void setBadgeWithTextShapeAppearanceOverlayResId(const std::string& shapeAppearanceOverlayResId);

    int getBadgeGravity() const;
    void setBadgeGravity(int badgeGravity);

    int getBadgeHorizontalPadding() const;
    void setBadgeHorizontalPadding( int horizontalPadding);
    int getBadgeVerticalPadding() const;
    void setBadgeVerticalPadding(int verticalPadding);

    int getHorizontalOffsetWithoutText()const;
    void setHorizontalOffsetWithoutText(int offset);

    int getVerticalOffsetWithoutText() const;

    void setVerticalOffsetWithoutText(int offset);

    int getHorizontalOffsetWithText() const;
    void setHorizontalOffsetWithText(int offset);

    int getVerticalOffsetWithText() const;
    void setVerticalOffsetWithText(int offset);

    int getLargeFontVerticalOffsetAdjustment() const;
    void setLargeFontVerticalOffsetAdjustment(int offsetAdjustment);

    int getAdditionalHorizontalOffset() const;
    void setAdditionalHorizontalOffset(int offset);

    int getAdditionalVerticalOffset() const;
    void setAdditionalVerticalOffset(int offset);

    std::string getContentDescriptionForText() const;
    void setContentDescriptionForText(const std::string& contentDescription);

    std::string getContentDescriptionNumberless() const;
    void setContentDescriptionNumberless(const std::string& contentDescriptionNumberless);

    int getContentDescriptionQuantityStrings() const;
    void setContentDescriptionQuantityStringsResource(int stringsResource);

    int getContentDescriptionExceedsMaxBadgeNumberStringResource() const;
    void setContentDescriptionExceedsMaxBadgeNumberStringResource(int stringsResource);

    /*Locale getNumberLocale() {
        return currentState->numberLocale;
    }

    void setNumberLocale(Locale locale) {
        overridingState->numberLocale = locale;
        currentState->numberLocale = locale;
    }*/

    /** Deprecated; badges now adjust to within bounds of first ancestor that clips its children */
    //@Deprecated
    bool isAutoAdjustedToGrandparentBounds() const;

    /** Deprecated; badges now adjust to within bounds of first ancestor that clips its children */
    //@Deprecated
    void setAutoAdjustToGrandparentBounds(bool autoAdjustToGrandparentBounds);

    /*static int readColorFromAttributes( Context* context,TypedArray a, int index) {
        return MaterialResources.getColorStateList(context, a, index).getDefaultColor();
    }*/
};

class BadgeState::State :public Parcelable {
private:
    static constexpr int BADGE_NUMBER_NONE = -1;
    static constexpr int NOT_SET = -2;
    friend BadgeState;
    std::string badgeResId;
    int backgroundColor;
    int badgeTextColor;

    std::string badgeTextAppearanceResId;
    std::string badgeShapeAppearanceResId;
    std::string badgeShapeAppearanceOverlayResId;
    std::string badgeWithTextShapeAppearanceResId;
    std::string badgeWithTextShapeAppearanceOverlayResId;
    std::string text;

    int alpha = 255;
    int number = NOT_SET;
    int maxCharacterCount = NOT_SET;
    int maxNumber = NOT_SET;
    //Locale numberLocale;

    std::string contentDescriptionForText;
    std::string contentDescriptionNumberless;
    int contentDescriptionQuantityStrings;
    int contentDescriptionExceedsMaxBadgeNumberRes;

    int badgeGravity;
    int badgeHorizontalPadding;
    int badgeVerticalPadding;

    int horizontalOffsetWithoutText;
    int verticalOffsetWithoutText;
    int horizontalOffsetWithText;
    int verticalOffsetWithText;

    int additionalHorizontalOffset;
    int additionalVerticalOffset;
    int largeFontVerticalOffsetAdjustment;
    int badgeFixedEdge;
    bool isVisible = true;
    bool autoAdjustToWithinGrandparentBounds;

public:
    State();
    State(Parcel& in);
    int describeContents();
    void writeToParcel(Parcel& dest, int flags)override; 
};
}/*endof namespace*/
#endif/*__BADGE_STATE_H__*/
