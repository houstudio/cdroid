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
#include <drawable/badgedrawable.h>
#include <drawable/badgestate.h>
namespace cdroid{
BadgeState::BadgeState(Context* context,const std::string& badgeResId,const std::string& defStyleAttr,const std::string& defStyleRes,State* storedState) {
    currentState = new State();
    if (storedState == nullptr) {
        storedState = new State();
    }
    storedState->badgeResId = badgeResId;

    //AttributeSet a = generateTypedArray(context, storedState->badgeResId, defStyleAttr, defStyleRes);

    int type;
    XmlPullParser parser(context,badgeResId);
    const AttributeSet& a = parser;
    while( ((type=parser.next())!=XmlPullParser::START_TAG) && (type!=XmlPullParser::END_DOCUMENT)){
        //NOTHING
    }
    const std::string tag=parser.getName();
    LOGD_IF(tag.compare("badge"),"invalid resource tag:%s[%s] ",tag.c_str(),badgeResId.c_str());

    mBadgeRadius = a.getDimensionPixelSize("badgeRadius", (int)BadgeDrawable::BADGE_RADIUS_NOT_SPECIFIED);

    mHorizontalInset = context->getDimensionPixelSize("@cdroid:dimen/mtrl_badge_horizontal_edge_offset");

    mHorizontalInsetWithText =context->getDimensionPixelSize("@cdroid:dimen/mtrl_badge_text_horizontal_edge_offset");

    mBadgeWithTextRadius = a.getDimensionPixelSize("badgeWithTextRadius", (int)BadgeDrawable::BADGE_RADIUS_NOT_SPECIFIED);
    mBadgeWidth = a.getDimension("badgeWidth", context->getDimension("@cdroid:dimen/m3_badge_size"));
    mBadgeWithTextWidth =a.getDimension("badgeWithTextWidth",
            context->getDimension("@cdroid:dimen/m3_badge_with_text_size"));
    mBadgeHeight = a.getDimension("badgeHeight", context->getDimension("@cdroid:dimen/m3_badge_size"));
    mBadgeWithTextHeight = a.getDimension("badgeWithTextHeight",
            context->getDimension("@cdroid:dimen/m3_badge_with_text_size"));

    mOffsetAlignmentMode = a.getInt("offsetAlignmentMode",std::unordered_map<std::string,int>{
            {"edge" ,(int)BadgeDrawable::OFFSET_ALIGNMENT_MODE_EDGE},
            {"legacy",(int)BadgeDrawable::OFFSET_ALIGNMENT_MODE_LEGACY}
            }, (int)BadgeDrawable::OFFSET_ALIGNMENT_MODE_LEGACY);

    mBadgeFixedEdge = a.getInt("badgeFixedEdge",std::unordered_map<std::string,int>{
            {"start",(int)BadgeDrawable::BADGE_FIXED_EDGE_START},
            {"end",(int)BadgeDrawable::BADGE_FIXED_EDGE_END}
            }, (int)BadgeDrawable::BADGE_FIXED_EDGE_START);

    currentState->alpha = storedState->alpha == State::NOT_SET ? 255 : storedState->alpha;

    // Only set the badge number if it exists in the style.
    // Defaulting it to 0 means the badge will incorrectly show text when the user may want a
    // numberless badge.
    if (storedState->number != State::NOT_SET) {
        currentState->number = storedState->number;
    } else if (a.hasAttribute("number")) {
        currentState->number = a.getInt("number", 0);
    } else {
        currentState->number = State::BADGE_NUMBER_NONE;
    }

    if (!storedState->text.empty()) {
        currentState->text = storedState->text;
    } else if (a.hasAttribute("badgeText")) {
        currentState->text = a.getString("badgeText");
    }
#if 0
    currentState->contentDescriptionForText = storedState->contentDescriptionForText;

    currentState->contentDescriptionNumberless =
        storedState->contentDescriptionNumberless == null
        ? context->getString("@cdroid:string/mtrl_badge_numberless_content_description")
        : storedState->contentDescriptionNumberless;

    currentState->contentDescriptionQuantityStrings =
        storedState->contentDescriptionQuantityStrings == 0
        ? R.plurals.mtrl_badge_content_description
        : storedState->contentDescriptionQuantityStrings;

    currentState->contentDescriptionExceedsMaxBadgeNumberRes =
        storedState->contentDescriptionExceedsMaxBadgeNumberRes == 0
        ? R.string.mtrl_exceed_max_badge_number_content_description
        : storedState->contentDescriptionExceedsMaxBadgeNumberRes;
#endif
    currentState->isVisible = storedState->isVisible/* == null*/ || storedState->isVisible;

    currentState->maxCharacterCount =
        storedState->maxCharacterCount == State::NOT_SET
        ? a.getInt("maxCharacterCount", (int)BadgeDrawable::BADGE_CONTENT_NOT_TRUNCATED)
        : storedState->maxCharacterCount;

    currentState->maxNumber =
        storedState->maxNumber == State::NOT_SET
        ? a.getInt("maxNumber", (int)BadgeDrawable::BADGE_CONTENT_NOT_TRUNCATED)
        : storedState->maxNumber;

    currentState->badgeShapeAppearanceResId =
        storedState->badgeShapeAppearanceResId.empty()
        ? a.getString("badgeShapeAppearance", "@cdroid:style/ShapeAppearance_M3_Sys_Shape_Corner_Full")
        : storedState->badgeShapeAppearanceResId;

    currentState->badgeShapeAppearanceOverlayResId =
        storedState->badgeShapeAppearanceOverlayResId.empty()
        ? a.getString("badgeShapeAppearanceOverlay")
        : storedState->badgeShapeAppearanceOverlayResId;

    currentState->badgeWithTextShapeAppearanceResId =
        storedState->badgeWithTextShapeAppearanceResId.empty()
        ? a.getString("badgeWithTextShapeAppearance", "@cdroid:style/ShapeAppearance_M3_Sys_Shape_Corner_Full")
        : storedState->badgeWithTextShapeAppearanceResId;

    currentState->badgeWithTextShapeAppearanceOverlayResId =
        storedState->badgeWithTextShapeAppearanceOverlayResId.empty()
        ? a.getString("badgeWithTextShapeAppearanceOverlay")
        : storedState->badgeWithTextShapeAppearanceOverlayResId;

    currentState->backgroundColor = a.getColor("backgroundColor",storedState->backgroundColor);
        /*storedState->backgroundColor == null
        ? readColorFromAttributes(context, a, R.styleable.Badge_backgroundColor)
        : storedState->backgroundColor;*/

    currentState->badgeTextAppearanceResId = a.getString("badgeTextAppearance",storedState->badgeTextAppearanceResId);
        /*storedState->badgeTextAppearanceResId == null
        ? a.getString("badgeTextAppearance","cdroid:style/TextAppearance.MaterialComponents.Badge")
        : storedState->badgeTextAppearanceResId;*/

    // Only set the badge text color if this attribute has explicitly been set, otherwise use the
    // text color specified in the TextAppearance.
    if (a.hasAttribute("badgeTextColor")) {
        currentState->badgeTextColor = a.getColor("badgeTextColor");
    } else if(!currentState->badgeTextAppearanceResId.empty()){
        //TextAppearance textAppearance = new TextAppearance(context, currentState->badgeTextAppearanceResId);
        //currentState->badgeTextColor = textAppearance.getTextColor().getDefaultColor();
        const AttributeSet attrText = context->obtainStyledAttributes(currentState->badgeTextAppearanceResId);
        currentState->badgeTextColor= attrText.getColorStateList("textColor")->getDefaultColor();
    }else{
        currentState->badgeTextColor = storedState->badgeTextColor;
    }

    currentState->badgeGravity = (storedState->badgeGravity == Gravity::NO_GRAVITY)
        ? a.getInt("badgeGravity",std::unordered_map<std::string,int>{
                {"TOP_END"     , (int)BadgeDrawable::TOP_END},
                {"TOP_START"   , (int)BadgeDrawable::TOP_START},
                {"BOTTOM_END"  , (int)BadgeDrawable::BOTTOM_END},
                {"BOTTOM_START", (int)BadgeDrawable::BOTTOM_START}},BadgeDrawable::TOP_END)
        : storedState->badgeGravity;

    currentState->badgeHorizontalPadding = (storedState->badgeHorizontalPadding == INT_MIN)
        ? a.getDimensionPixelSize("badgeWidePadding",
                context->getDimensionPixelSize("cdroid:dimen/mtrl_badge_long_text_horizontal_padding"))
        : storedState->badgeHorizontalPadding;
    currentState->badgeVerticalPadding = (storedState->badgeVerticalPadding == INT_MIN)
        ? a.getDimensionPixelSize("badgeVerticalPadding",
                context->getDimensionPixelSize("cdroid:dimen/m3_badge_with_text_vertical_padding"))
        : storedState->badgeVerticalPadding;

    currentState->horizontalOffsetWithoutText = (storedState->horizontalOffsetWithoutText == INT_MIN)
        ? a.getDimensionPixelOffset("horizontalOffset", 0)
        : storedState->horizontalOffsetWithoutText;

    currentState->verticalOffsetWithoutText = (storedState->verticalOffsetWithoutText == INT_MIN)
        ? a.getDimensionPixelOffset("verticalOffset", 0)
        : storedState->verticalOffsetWithoutText;

    // Set the offsets when the badge has text. Default to using the badge "dot" offsets
    // (horizontalOffsetWithoutText and verticalOffsetWithoutText) if there is no offsets defined
    // for badges with text.
    currentState->horizontalOffsetWithText = (storedState->horizontalOffsetWithText == INT_MIN)
        ? a.getDimensionPixelOffset("horizontalOffsetWithText", currentState->horizontalOffsetWithoutText)
        : storedState->horizontalOffsetWithText;

    currentState->verticalOffsetWithText = (storedState->verticalOffsetWithText == INT_MIN)
        ? a.getDimensionPixelOffset("verticalOffsetWithText", currentState->verticalOffsetWithoutText)
        : storedState->verticalOffsetWithText;

    currentState->largeFontVerticalOffsetAdjustment = (storedState->largeFontVerticalOffsetAdjustment == INT_MIN)
        ? a.getDimensionPixelOffset("largeFontVerticalOffsetAdjustment", 0)
        : storedState->largeFontVerticalOffsetAdjustment;

    currentState->additionalHorizontalOffset =
        storedState->additionalHorizontalOffset == INT_MIN ? 0 : storedState->additionalHorizontalOffset;

    currentState->additionalVerticalOffset =
        storedState->additionalVerticalOffset == INT_MIN ? 0 : storedState->additionalVerticalOffset;

    currentState->autoAdjustToWithinGrandparentBounds =
        a.getBoolean("autoAdjustToWithinGrandparentBounds",storedState->autoAdjustToWithinGrandparentBounds);
        /*storedState->autoAdjustToWithinGrandparentBounds == null
        ? a.getBoolean("autoAdjustToWithinGrandparentBounds", false)
        : storedState->autoAdjustToWithinGrandparentBounds;*/

    /*if (storedState->numberLocale == null) {
        currentState->numberLocale =
            VERSION.SDK_INT >= VERSION_CODES.N
            ? Locale.getDefault(Locale.Category.FORMAT)
            : Locale.getDefault();
    } else {
        currentState->numberLocale = storedState->numberLocale;
    }*/
    overridingState = storedState;
}

BadgeState::~BadgeState(){
    delete currentState;
    delete overridingState;
}

AttributeSet BadgeState::generateTypedArray(Context* context,const std::string& badgeResId,const std::string& defStyleAttr,const std::string& defStyleRes) {
    XmlPullParser parser(context,badgeResId);
    const AttributeSet& attrs = parser;
    int type;
    if(!parser)return attrs;
    while( ((type=parser.next())!=XmlPullParser::START_TAG) && (type!=XmlPullParser::END_DOCUMENT)){
        //NOTHING
    }
    /*AttributeSet attrs = context->obtainStyledAttributes(badgeResId);
    int style = 0;
    if (badgeResId != 0) {
        attrs = DrawableUtils.parseDrawableXml(context, badgeResId, BADGE_RESOURCE_TAG);
        style = attrs.getStyleAttribute();
    }
    if (style == 0) {
        style = defStyleRes;
    }*/
    //return ThemeEnforcement.obtainStyledAttributes(context, attrs, R.styleable.Badge, defStyleAttr, style);
    return attrs;
}

BadgeState::State* BadgeState::getOverridingState() const{
    return overridingState;
}

bool BadgeState::isVisible() const{
    return currentState->isVisible;
}

void BadgeState::setVisible(bool visible) {
    overridingState->isVisible = visible;
    currentState->isVisible = visible;
}

bool BadgeState::hasNumber() const{
    return currentState->number != State::BADGE_NUMBER_NONE;
}

int BadgeState::getNumber() const{
    return currentState->number;
}

void BadgeState::setNumber(int number) {
    overridingState->number = number;
    currentState->number = number;
}

void BadgeState::clearNumber() {
    setNumber(State::BADGE_NUMBER_NONE);
}

bool BadgeState::hasText() const{
    return !currentState->text.empty();
}

const std::string BadgeState::getText() const{
    return currentState->text;
}

void BadgeState::setText(const std::string& text) {
    overridingState->text = text;
    currentState->text = text;
}

void BadgeState::clearText() {
    setText("");
}

int BadgeState::getAlpha() const{
    return currentState->alpha;
}

void BadgeState::setAlpha(int alpha) {
    overridingState->alpha = alpha;
    currentState->alpha = alpha;
}

int BadgeState::getMaxCharacterCount() const{
    return currentState->maxCharacterCount;
}

void BadgeState::setMaxCharacterCount(int maxCharacterCount) {
    overridingState->maxCharacterCount = maxCharacterCount;
    currentState->maxCharacterCount = maxCharacterCount;
}

int BadgeState::getMaxNumber() const{
    return currentState->maxNumber;
}

void BadgeState::setMaxNumber(int maxNumber) {
    overridingState->maxNumber = maxNumber;
    currentState->maxNumber = maxNumber;
}

int BadgeState::getBackgroundColor() const{
    return currentState->backgroundColor;
}

void BadgeState::setBackgroundColor(int backgroundColor) {
    overridingState->backgroundColor = backgroundColor;
    currentState->backgroundColor = backgroundColor;
}

int BadgeState::getBadgeTextColor() const{
    return currentState->badgeTextColor;
}

void BadgeState::setBadgeTextColor(int badgeTextColor) {
    overridingState->badgeTextColor = badgeTextColor;
    currentState->badgeTextColor = badgeTextColor;
}

std::string BadgeState::getTextAppearanceResId() const{
    return currentState->badgeTextAppearanceResId;
}

void BadgeState::setTextAppearanceResId(const std::string& textAppearanceResId) {
    overridingState->badgeTextAppearanceResId = textAppearanceResId;
    currentState->badgeTextAppearanceResId = textAppearanceResId;
}

std::string BadgeState::getBadgeShapeAppearanceResId() const{
    return currentState->badgeShapeAppearanceResId;
}

void BadgeState::setBadgeShapeAppearanceResId(const std::string& shapeAppearanceResId) {
    overridingState->badgeShapeAppearanceResId = shapeAppearanceResId;
    currentState->badgeShapeAppearanceResId = shapeAppearanceResId;
}

std::string BadgeState::getBadgeShapeAppearanceOverlayResId() const{
    return currentState->badgeShapeAppearanceOverlayResId;
}

void BadgeState::setBadgeShapeAppearanceOverlayResId(const std::string& shapeAppearanceOverlayResId) {
    overridingState->badgeShapeAppearanceOverlayResId = shapeAppearanceOverlayResId;
    currentState->badgeShapeAppearanceOverlayResId = shapeAppearanceOverlayResId;
}

std::string BadgeState::getBadgeWithTextShapeAppearanceResId() const{
    return currentState->badgeWithTextShapeAppearanceResId;
}

void BadgeState::setBadgeWithTextShapeAppearanceResId(const std::string& shapeAppearanceResId) {
    overridingState->badgeWithTextShapeAppearanceResId = shapeAppearanceResId;
    currentState->badgeWithTextShapeAppearanceResId = shapeAppearanceResId;
}

std::string BadgeState::getBadgeWithTextShapeAppearanceOverlayResId() const{
    return currentState->badgeWithTextShapeAppearanceOverlayResId;
}

void BadgeState::setBadgeWithTextShapeAppearanceOverlayResId(const std::string& shapeAppearanceOverlayResId) {
    overridingState->badgeWithTextShapeAppearanceOverlayResId = shapeAppearanceOverlayResId;
    currentState->badgeWithTextShapeAppearanceOverlayResId = shapeAppearanceOverlayResId;
}

int BadgeState::getBadgeGravity() const{
    return currentState->badgeGravity;
}

void BadgeState::setBadgeGravity(int badgeGravity) {
    overridingState->badgeGravity = badgeGravity;
    currentState->badgeGravity = badgeGravity;
}

int BadgeState::getBadgeHorizontalPadding() const{
    return currentState->badgeHorizontalPadding;
}

void BadgeState::setBadgeHorizontalPadding(int horizontalPadding) {
    overridingState->badgeHorizontalPadding = horizontalPadding;
    currentState->badgeHorizontalPadding = horizontalPadding;
}

int BadgeState::getBadgeVerticalPadding() const{
    return currentState->badgeVerticalPadding;
}

void BadgeState::setBadgeVerticalPadding(int verticalPadding) {
    overridingState->badgeVerticalPadding = verticalPadding;
    currentState->badgeVerticalPadding = verticalPadding;
}

int BadgeState::getHorizontalOffsetWithoutText() const{
    return currentState->horizontalOffsetWithoutText;
}

void BadgeState::setHorizontalOffsetWithoutText(int offset) {
    overridingState->horizontalOffsetWithoutText = offset;
    currentState->horizontalOffsetWithoutText = offset;
}

int BadgeState::getVerticalOffsetWithoutText() const{
    return currentState->verticalOffsetWithoutText;
}

void BadgeState::setVerticalOffsetWithoutText(int offset) {
    overridingState->verticalOffsetWithoutText = offset;
    currentState->verticalOffsetWithoutText = offset;
}

int BadgeState::getHorizontalOffsetWithText() const{
    return currentState->horizontalOffsetWithText;
}

void BadgeState::setHorizontalOffsetWithText(int offset) {
    overridingState->horizontalOffsetWithText = offset;
    currentState->horizontalOffsetWithText = offset;
}

int BadgeState::getVerticalOffsetWithText() const{
    return currentState->verticalOffsetWithText;
}

void BadgeState::setVerticalOffsetWithText(int offset) {
    overridingState->verticalOffsetWithText = offset;
    currentState->verticalOffsetWithText = offset;
}

int BadgeState::getLargeFontVerticalOffsetAdjustment() const{
    return currentState->largeFontVerticalOffsetAdjustment;
}

void BadgeState::setLargeFontVerticalOffsetAdjustment(int offsetAdjustment) {
    overridingState->largeFontVerticalOffsetAdjustment = offsetAdjustment;
    currentState->largeFontVerticalOffsetAdjustment = offsetAdjustment;
}

int BadgeState::getAdditionalHorizontalOffset() const{
    return currentState->additionalHorizontalOffset;
}

void BadgeState::setAdditionalHorizontalOffset(int offset) {
    overridingState->additionalHorizontalOffset = offset;
    currentState->additionalHorizontalOffset = offset;
}

int BadgeState::getAdditionalVerticalOffset() const{
    return currentState->additionalVerticalOffset;
}

void BadgeState::setAdditionalVerticalOffset(int offset) {
    overridingState->additionalVerticalOffset = offset;
    currentState->additionalVerticalOffset = offset;
}

std::string BadgeState::getContentDescriptionForText() const{
    return currentState->contentDescriptionForText;
}

void BadgeState::setContentDescriptionForText(const std::string& contentDescription) {
    overridingState->contentDescriptionForText = contentDescription;
    currentState->contentDescriptionForText = contentDescription;
}

std::string BadgeState::getContentDescriptionNumberless() const{
    return currentState->contentDescriptionNumberless;
}

void BadgeState::setContentDescriptionNumberless(const std::string& contentDescriptionNumberless) {
    overridingState->contentDescriptionNumberless = contentDescriptionNumberless;
    currentState->contentDescriptionNumberless = contentDescriptionNumberless;
}

int BadgeState::getContentDescriptionQuantityStrings() const{
    return currentState->contentDescriptionQuantityStrings;
}

void BadgeState::setContentDescriptionQuantityStringsResource(int stringsResource) {
    overridingState->contentDescriptionQuantityStrings = stringsResource;
    currentState->contentDescriptionQuantityStrings = stringsResource;
}

int BadgeState::getContentDescriptionExceedsMaxBadgeNumberStringResource() const{
    return currentState->contentDescriptionExceedsMaxBadgeNumberRes;
}

void BadgeState::setContentDescriptionExceedsMaxBadgeNumberStringResource(int stringsResource) {
    overridingState->contentDescriptionExceedsMaxBadgeNumberRes = stringsResource;
    currentState->contentDescriptionExceedsMaxBadgeNumberRes = stringsResource;
}

/*Locale BadgeState::getNumberLocale() const{
    return currentState->numberLocale;
}

void BadgeState::setNumberLocale(Locale locale) {
    overridingState->numberLocale = locale;
    currentState->numberLocale = locale;
}*/

/** Deprecated; badges now adjust to within bounds of first ancestor that clips its children */
bool BadgeState::isAutoAdjustedToGrandparentBounds() const{
    return currentState->autoAdjustToWithinGrandparentBounds;
}

/** Deprecated; badges now adjust to within bounds of first ancestor that clips its children */
void BadgeState::setAutoAdjustToGrandparentBounds(bool autoAdjustToGrandparentBounds) {
    overridingState->autoAdjustToWithinGrandparentBounds = autoAdjustToGrandparentBounds;
    currentState->autoAdjustToWithinGrandparentBounds = autoAdjustToGrandparentBounds;
}

/*int BadgeState::readColorFromAttributes(Context* context, TypedArray& a, int index) {
    return MaterialResources.getColorStateList(context, a, index).getDefaultColor();
}*/

/**
 * Internal {@link Parcelable} state used to represent, save, and restore {@link BadgeDrawable}
 * states.
 */
BadgeState::State::State() {
    backgroundColor= 0xFFFF0000;
    badgeTextColor = 0xFFFFFFFF;
    badgeGravity = BadgeDrawable::TOP_END;
    badgeHorizontalPadding = INT_MIN;
    badgeVerticalPadding = INT_MIN;
    horizontalOffsetWithoutText =INT_MIN;
    verticalOffsetWithoutText =INT_MIN;
    horizontalOffsetWithText =INT_MIN;
    verticalOffsetWithText =INT_MIN;
    largeFontVerticalOffsetAdjustment =INT_MIN;
    additionalHorizontalOffset =INT_MIN;
    additionalVerticalOffset =INT_MIN;
    isVisible = true;
    autoAdjustToWithinGrandparentBounds = true;
    badgeFixedEdge =INT_MIN;
    alpha = 255;
    number= NOT_SET;
    maxNumber = NOT_SET;
    maxCharacterCount =NOT_SET;
}

BadgeState::State::State(Parcel& in) {
#if 0
    badgeResId = in.readInt();
    backgroundColor = (Integer) in.readSerializable();
    badgeTextColor = (Integer) in.readSerializable();
    badgeTextAppearanceResId = (Integer) in.readSerializable();
    badgeShapeAppearanceResId = (Integer) in.readSerializable();
    badgeShapeAppearanceOverlayResId = (Integer) in.readSerializable();
    badgeWithTextShapeAppearanceResId = (Integer) in.readSerializable();
    badgeWithTextShapeAppearanceOverlayResId = (Integer) in.readSerializable();
    alpha = in.readInt();
    text = in.readString();
    number = in.readInt();
    maxCharacterCount = in.readInt();
    maxNumber = in.readInt();
    contentDescriptionForText = in.readString();
    contentDescriptionNumberless = in.readString();
    contentDescriptionQuantityStrings = in.readInt();
    badgeGravity = (Integer) in.readSerializable();
    badgeHorizontalPadding = (Integer) in.readSerializable();
    badgeVerticalPadding = (Integer) in.readSerializable();
    horizontalOffsetWithoutText = (Integer) in.readSerializable();
    verticalOffsetWithoutText = (Integer) in.readSerializable();
    horizontalOffsetWithText = (Integer) in.readSerializable();
    verticalOffsetWithText = (Integer) in.readSerializable();
    largeFontVerticalOffsetAdjustment = (Integer) in.readSerializable();
    additionalHorizontalOffset = (Integer) in.readSerializable();
    additionalVerticalOffset = (Integer) in.readSerializable();
    isVisible = (Boolean) in.readSerializable();
    numberLocale = (Locale) in.readSerializable();
    autoAdjustToWithinGrandparentBounds = (Boolean) in.readSerializable();
    badgeFixedEdge = (Integer) in.readSerializable();
#endif
}


int BadgeState::State::describeContents() {
    return 0;
}

void BadgeState::State::writeToParcel(Parcel& dest, int flags) {
#if 0
    dest.writeInt(badgeResId);
    dest.writeSerializable(backgroundColor);
    dest.writeSerializable(badgeTextColor);
    dest.writeSerializable(badgeTextAppearanceResId);
    dest.writeSerializable(badgeShapeAppearanceResId);
    dest.writeSerializable(badgeShapeAppearanceOverlayResId);
    dest.writeSerializable(badgeWithTextShapeAppearanceResId);
    dest.writeSerializable(badgeWithTextShapeAppearanceOverlayResId);
    dest.writeInt(alpha);
    dest.writeString(text);
    dest.writeInt(number);
    dest.writeInt(maxCharacterCount);
    dest.writeInt(maxNumber);
    dest.writeString(
        contentDescriptionForText != null ? contentDescriptionForText.toString() : null);
    dest.writeString(
        contentDescriptionNumberless != null ? contentDescriptionNumberless.toString() : null);
    dest.writeInt(contentDescriptionQuantityStrings);
    dest.writeSerializable(badgeGravity);
    dest.writeSerializable(badgeHorizontalPadding);
    dest.writeSerializable(badgeVerticalPadding);
    dest.writeSerializable(horizontalOffsetWithoutText);
    dest.writeSerializable(verticalOffsetWithoutText);
    dest.writeSerializable(horizontalOffsetWithText);
    dest.writeSerializable(verticalOffsetWithText);
    dest.writeSerializable(largeFontVerticalOffsetAdjustment);
    dest.writeSerializable(additionalHorizontalOffset);
    dest.writeSerializable(additionalVerticalOffset);
    dest.writeSerializable(isVisible);
    dest.writeSerializable(numberLocale);
    dest.writeSerializable(autoAdjustToWithinGrandparentBounds);
    dest.writeSerializable(badgeFixedEdge);
#endif
}
}/*endof namespace*/
