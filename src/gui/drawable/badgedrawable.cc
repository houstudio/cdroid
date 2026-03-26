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
#include <drawable/badgeutils.h>
#include <widget/framelayout.h>
#include <core/xmlpullparser.h>
#include <core/layout.h>
#include <widget/R.h>
#if 10
namespace cdroid{

BadgeDrawable::SavedState::SavedState(Context* context) {
    // If the badge text color attribute was not explicitly set, use the text color specified in
    // the TextAppearance.
    const AttributeSet atts= context->obtainStyledAttributes("cdroid:style/TextAppearance.MaterialComponents.Badge");
    //TextAppearance textAppearance = new TextAppearance(context, R.style.TextAppearance_MaterialComponents_Badge);
    mBadgeTextColor = ~0;//atts.getColorStateList("textColor");//textAppearance.textColor.getDefaultColor();
    mBackgroundColor= 0xFFFF0000;
    //contentDescriptionNumberless =context->getString(R.string.mtrl_badge_numberless_content_description);
    //contentDescriptionQuantityStrings = R.plurals.mtrl_badge_content_description;
    //contentDescriptionExceedsMaxBadgeNumberRes = R.string.mtrl_exceed_max_badge_number_content_description;
    mIsVisible = true;
    mBadgeGravity = Gravity::NO_GRAVITY;
    mVerticalOffset  = 0;
    mHorizontalOffset= 0;
}

BadgeDrawable::SavedState::SavedState(Parcel& in) {
    mBackgroundColor = in.readInt();
    mBadgeTextColor = in.readInt();
    mAlpha = in.readInt();
    mNumber = in.readInt();
    mMaxCharacterCount = in.readInt();
    contentDescriptionNumberless = in.readString();
    contentDescriptionQuantityStrings = in.readInt();
    mBadgeGravity = in.readInt();
    mHorizontalOffset = in.readInt();
    mVerticalOffset = in.readInt();
    mIsVisible = in.readInt() != 0;
}

int BadgeDrawable::SavedState::describeContents() {
    return 0;
}

void BadgeDrawable::SavedState::writeToParcel(Parcel& dest, int flags) {
    dest.writeInt(mBackgroundColor);
    dest.writeInt(mBadgeTextColor);
    dest.writeInt(mAlpha);
    dest.writeInt(mNumber);
    dest.writeInt(mMaxCharacterCount);
    dest.writeString(contentDescriptionNumberless);
    dest.writeInt(contentDescriptionQuantityStrings);
    dest.writeInt(mBadgeGravity);
    dest.writeInt(mHorizontalOffset);
    dest.writeInt(mVerticalOffset);
    dest.writeInt(mIsVisible ? 1 : 0);
}


BadgeDrawable::SavedState* BadgeDrawable::getSavedState() const{
    return mSavedState;
}

BadgeDrawable* BadgeDrawable::createFromSavedState(Context* context, SavedState* savedState) {
    BadgeDrawable* badge = new BadgeDrawable(context);
    badge->restoreFromSavedState(*savedState);
    return badge;
}

BadgeDrawable::~BadgeDrawable(){
    delete mShapeDrawable;
    delete mSavedState;
    delete mTextLayout;
}

BadgeDrawable* BadgeDrawable::create(Context* context) {
    const AttributeSet attrs = context->obtainStyledAttributes("cdroid:attr/badgeStyle");
    return createFromAttributes(context, attrs, 0/*DEFAULT_THEME_ATTR*/,0/*DEFAULT_STYLE*/);
}

BadgeDrawable* BadgeDrawable::createFromResource(Context* context, const std::string& id) {
    int type;
    XmlPullParser parser(context,id);
    const AttributeSet& attrs = parser;
    if(!parser)return nullptr;
    while( ((type=parser.next())!=XmlPullParser::START_TAG) && (type!=XmlPullParser::END_DOCUMENT)){
        //NOTHING
    }
    //AttributeSet attrs = DrawableUtils::parseDrawableXml(context, id, "badge");
    const int style = 0;//attrs.getStyleAttribute();
    //if (style == 0) { style = DEFAULT_STYLE; }
    return createFromAttributes(context, attrs, 0/*DEFAULT_THEME_ATTR*/, style);
}

/** Returns a {@code BadgeDrawable} from the given attributes. */
BadgeDrawable* BadgeDrawable::createFromAttributes(Context* context,const AttributeSet& attrs, int defStyleAttr, int defStyleRes) {
    BadgeDrawable* badge = new BadgeDrawable(context);
    badge->loadDefaultStateFromAttributes(context, attrs, defStyleAttr, defStyleRes);
    return badge;
}

void BadgeDrawable::setVisible(bool visible) {
    Drawable::setVisible(visible, /* restart= */ false);
    mSavedState->mIsVisible = visible;
    // When hiding a badge in pre-API 18, invalidate the custom parent in order to trigger a draw
    // pass to remove this badge from its foreground.
    if (BadgeUtils::USE_COMPAT_PARENT && getCustomBadgeParent() != nullptr && !visible) {
        ((ViewGroup*) getCustomBadgeParent()->getParent())->invalidate();
    }
}

void BadgeDrawable::restoreFromSavedState(SavedState& savedState) {
    setMaxCharacterCount(savedState.mMaxCharacterCount);
  
    // Only set the badge number if it exists in the style.
    // Defaulting it to 0 means the badge will incorrectly show text when the user may want a
    // numberless badge.
    if (savedState.mNumber != BADGE_NUMBER_NONE) {
        setNumber(savedState.mNumber);
    }
  
    setBackgroundColor(savedState.mBackgroundColor);
  
    // Only set the badge text color if this attribute has explicitly been set, otherwise use the
    // text color specified in the TextAppearance.
    setBadgeTextColor(savedState.mBadgeTextColor);
  
    setBadgeGravity(savedState.mBadgeGravity);
  
    setHorizontalOffset(savedState.mHorizontalOffset);
    setVerticalOffset(savedState.mVerticalOffset);
    setVisible(savedState.mIsVisible);
}

void BadgeDrawable::loadDefaultStateFromAttributes( Context* context,const AttributeSet& attrs, int defStyleAttr, int defStyleRes) {
    setMaxCharacterCount( attrs.getInt("maxCharacterCount", DEFAULT_MAX_BADGE_CHARACTER_COUNT));
  
    // Only set the badge number if it exists in the style.
    // Defaulting it to 0 means the badge will incorrectly show text when the user may want a
    // numberless badge.
    if (attrs.hasAttribute("number")) {
        setNumber(attrs.getInt("number", 0));
    }
    if(attrs.hasAttribute("backgroundColor")){
        setBackgroundColor(attrs.getColor("backgroundColor"));
    }
  
    // Only set the badge text color if this attribute has explicitly been set, otherwise use the
    // text color specified in the TextAppearance.
    if (attrs.hasAttribute("badgeTextColor")) {
        setBadgeTextColor(attrs.getColor("badgeTextColor"));
    }
  
    setBadgeGravity(attrs.getInt("badgeGravity",std::unordered_map<std::string,int>{
                {"TOP_END"     , (int)TOP_END},
                {"TOP_START"   , (int)TOP_START},
                {"BOTTOM_END"  , (int)BOTTOM_END},
                {"BOTTOM_START", (int)BOTTOM_START},
            },TOP_END));
  
    setHorizontalOffset(attrs.getDimensionPixelOffset("horizontalOffset", 0));
    setVerticalOffset(attrs.getDimensionPixelOffset("verticalOffset", 0));
    LOGD("mBackgroundColor=%x mBadgeTextColor=%x",mSavedState->mBackgroundColor,mSavedState->mBadgeTextColor);
}

BadgeDrawable::BadgeDrawable(Context* context) {
    mContext = context;
    mTextLayout = new Layout(14,INT_MAX);
    mTextLayout->setMultiline(false);
    mAnchorView = nullptr;
    mCustomBadgeParent = nullptr;
    mBadgeRadius = 6;
    mBadgeCenterX = 0;
    mBadgeCenterY = 0;
    mCornerRadius = 6;
    mBadgeWithTextRadius =6;
    //ThemeEnforcement.checkMaterialTheme(context);
    //Resources res = context.getResources();
    mShapeDrawable = new GradientDrawable();
  
    mBadgeRadius = context->getDimensionPixelSize("cdroid:dimen/mtrl_badge_radius",mBadgeRadius);
    mBadgeWidePadding = context->getDimensionPixelSize("cdroid:dimen/mtrl_badge_long_text_horizontal_padding",0);
    mBadgeWithTextRadius = context->getDimensionPixelSize("cdroid::dimen/mtrl_badge_with_text_radius",mBadgeWithTextRadius);
  
    mSavedState = new SavedState(context);
    setBackgroundColor(mSavedState->mBackgroundColor);
    setTextAppearanceResource("@cdroid:style/TextAppearance.MaterialComponents.Badge");
}

void BadgeDrawable::updateBadgeCoordinates(View* anchorView) {
    updateBadgeCoordinates(anchorView, nullptr);
}

void BadgeDrawable::updateBadgeCoordinates(View* anchorView, FrameLayout* customBadgeParent) {
    mAnchorView = anchorView;
  
    if (BadgeUtils::USE_COMPAT_PARENT && (customBadgeParent == nullptr)) {
        tryWrapAnchorInCompatParent(anchorView);
    } else {
        mCustomBadgeParent = customBadgeParent;
    }
    if (!BadgeUtils::USE_COMPAT_PARENT) {
        updateAnchorParentToNotClip(anchorView);
    }
    updateCenterAndBounds();
    invalidateSelf();
}

/** Returns a {@link FrameLayout} that will set this {@code BadgeDrawable} as its foreground. */
FrameLayout* BadgeDrawable::getCustomBadgeParent() {
    return mCustomBadgeParent != nullptr ? mCustomBadgeParent : nullptr;
}

void BadgeDrawable::tryWrapAnchorInCompatParent(View* anchorView) {
    ViewGroup* anchorViewParent = (ViewGroup*) anchorView->getParent();
    if ((anchorViewParent != nullptr && anchorViewParent->getId() == R::id::mtrl_anchor_parent)
        || (mCustomBadgeParent != nullptr && mCustomBadgeParent == anchorViewParent)) {
        return;
    }
    // Must call this before wrapping the anchor in a FrameLayout.
    updateAnchorParentToNotClip(anchorView);
  
    // Create FrameLayout and configure it to wrap the anchor.
    FrameLayout* frameLayout = new FrameLayout(-1,-1);//anchorView->getContext());
    frameLayout->setId(R::id::mtrl_anchor_parent);
    frameLayout->setClipChildren(false);
    frameLayout->setClipToPadding(false);
    frameLayout->setLayoutParams(anchorView->getLayoutParams());/*maybe caused double free in cdroid*/
    frameLayout->setMinimumWidth(anchorView->getWidth());
    frameLayout->setMinimumHeight(anchorView->getHeight());
  
    const int anchorIndex = anchorViewParent->indexOfChild(anchorView);
    anchorViewParent->removeViewAt(anchorIndex);
    anchorView->setLayoutParams(new LayoutParams(ViewGroup::LayoutParams::MATCH_PARENT, ViewGroup::LayoutParams::MATCH_PARENT));
  
    frameLayout->addView(anchorView);
    anchorViewParent->addView(frameLayout, anchorIndex);
    mCustomBadgeParent = frameLayout;
  
    // Update the badge's coordinates after the FrameLayout has been added to the view hierarchy and
    // has a size.
    frameLayout->post([this,anchorView,frameLayout](){
            updateBadgeCoordinates(anchorView, frameLayout);
        });
}

void BadgeDrawable::updateAnchorParentToNotClip(View* anchorView) {
    ViewGroup* anchorViewParent = (ViewGroup*) anchorView->getParent();
    anchorViewParent->setClipChildren(false);
    anchorViewParent->setClipToPadding(false);
}

int BadgeDrawable::getBackgroundColor() const{
    return mSavedState->mBackgroundColor;
}

void BadgeDrawable::setBackgroundColor(int backgroundColor) {
    mSavedState->mBackgroundColor = backgroundColor;
    auto  backgroundColorStateList = ColorStateList::valueOf(backgroundColor);
    if (mShapeDrawable->getColor()!=backgroundColorStateList){
        mShapeDrawable->setColor(backgroundColorStateList);
        invalidateSelf();
    }
}

int BadgeDrawable::getBadgeTextColor() const{
    return mSavedState->mBackgroundColor;
}

void BadgeDrawable::setBadgeTextColor(int badgeTextColor) {
    if (mSavedState->mBadgeTextColor != badgeTextColor){
        mSavedState->mBadgeTextColor = badgeTextColor;
        invalidateSelf();
    }
}

bool BadgeDrawable::hasNumber() const{
    return mSavedState->mNumber != BADGE_NUMBER_NONE;
}

int BadgeDrawable::getNumber() const{
    if (!hasNumber()) {
        return 0;
    }
    return mSavedState->mNumber;
}

void BadgeDrawable::setNumber(int number) {
    number = std::max(0, number);
    if (mSavedState->mNumber != number) {
        mSavedState->mNumber = number;
        updateCenterAndBounds();
        invalidateSelf();
    }
}

/** Resets any badge number so that a numberless badge will be displayed. */
void BadgeDrawable::clearNumber() {
    mSavedState->mNumber = BADGE_NUMBER_NONE;
    invalidateSelf();
}

int BadgeDrawable::getMaxCharacterCount() const{
     return mSavedState->mMaxCharacterCount;
}

void BadgeDrawable::setMaxCharacterCount(int maxCharacterCount) {
    if (mSavedState->mMaxCharacterCount != maxCharacterCount) {
       mSavedState->mMaxCharacterCount = maxCharacterCount;
       updateMaxBadgeNumber();
       updateCenterAndBounds();
       invalidateSelf();
    }
}

int BadgeDrawable::getBadgeGravity() const{
    return mSavedState->mBadgeGravity;
}

void BadgeDrawable::setBadgeGravity(int gravity) {
    if (mSavedState->mBadgeGravity != gravity) {
        mSavedState->mBadgeGravity = gravity;
        if (mAnchorView != nullptr && mAnchorView != nullptr) {
            updateBadgeCoordinates(
                mAnchorView, mCustomBadgeParent != nullptr ? mCustomBadgeParent : nullptr);
        }
    }
}

bool BadgeDrawable::isStateful() const{
    return false;
}

void BadgeDrawable::setColorFilter(const cdroid::RefPtr<ColorFilter>& colorFilter) {
    // Intentionally empty.
}

int BadgeDrawable::getAlpha()const {
    return mSavedState->mAlpha;
}

void BadgeDrawable::setAlpha(int alpha) {
    mSavedState->mAlpha = alpha;
    invalidateSelf();
}

int BadgeDrawable::getOpacity() {
    return PixelFormat::TRANSLUCENT;
}

/** Returns the height at which the badge would like to be laid out. */
int BadgeDrawable::getIntrinsicHeight() {
    return mBadgeBounds.height;
}

/** Returns the width at which the badge would like to be laid out. */
int BadgeDrawable::getIntrinsicWidth() {
    return mBadgeBounds.width;
}

void BadgeDrawable::draw(Canvas& canvas) {
    Rect bounds = getBounds();
    if (bounds.empty() || getAlpha() == 0 || !isVisible()) {
        return;
    }
    mShapeDrawable->draw(canvas);
    if (hasNumber()) {
        drawText(canvas);
    }
}

void BadgeDrawable::onTextSizeChange() {
    invalidateSelf();
}

bool BadgeDrawable::onStateChange(const std::vector<int>& state) {
    return Drawable::onStateChange(state);
}

void BadgeDrawable::setContentDescriptionNumberless(const std::string& charSequence) {
    mSavedState->contentDescriptionNumberless = charSequence;
}

/*void BadgeDrawable::setContentDescriptionQuantityStringsResource(int stringsResource) {
    mSavedState->contentDescriptionQuantityStrings = stringsResource;
}*/

void BadgeDrawable::setContentDescriptionExceedsMaxBadgeNumberStringResource(int stringsResource) {
    mSavedState->contentDescriptionExceedsMaxBadgeNumberRes = stringsResource;
}

std::string BadgeDrawable::getContentDescription() const{
    /*if (!isVisible()) {
        return null;
    }
    if (hasNumber()) {
        if (mSavedState.contentDescriptionQuantityStrings > 0) {
          Context context = mContext.get();
          if (context == null) {
              return null;
          }
          if (getNumber() <= mMaxBadgeNumber) {
              return context
                  .getResources()
                  .getQuantityString(
                      mSavedState.contentDescriptionQuantityStrings, getNumber(), getNumber());
          } else {
              return context.getString(
                  mSavedState.contentDescriptionExceedsMaxBadgeNumberRes, mMaxBadgeNumber);
          }
        } else {
            return null;
        }
    } else */{
        return mSavedState->contentDescriptionNumberless;
    }
}

void BadgeDrawable::setHorizontalOffset(int px) {
    mSavedState->mHorizontalOffset = px;
    updateCenterAndBounds();
}

int BadgeDrawable::getHorizontalOffset() const{
    return mSavedState->mHorizontalOffset;
}

void BadgeDrawable::setVerticalOffset(int px) {
    mSavedState->mVerticalOffset = px;
    updateCenterAndBounds();
}

int BadgeDrawable::getVerticalOffset() const{
    return mSavedState->mVerticalOffset;
}

void BadgeDrawable::setTextAppearanceResource(const std::string& id) {
    const AttributeSet atts = mContext->obtainStyledAttributes(id);
    const int textSize = atts.getInt("textSize",12);
    Typeface*tf =Typeface::create(atts.getString("fontFamily"),0);
    mTextLayout->setFontSize(textSize);
    mTextLayout->setTypeface(tf);
    atts.getColor("textColor");
}

void BadgeDrawable::updateCenterAndBounds() {
    View* anchorView = mAnchorView != nullptr ? mAnchorView : nullptr;
    if (mContext == nullptr || anchorView == nullptr) {
        return;
    }

    Rect anchorRect;
    Rect tmpRect = mBadgeBounds;
    // Retrieves the visible bounds of the anchor view.
    anchorView->getDrawingRect(anchorRect);
  
    ViewGroup* customBadgeParent = mCustomBadgeParent != nullptr ? mCustomBadgeParent : nullptr;
    if ((customBadgeParent != nullptr) || BadgeUtils::USE_COMPAT_PARENT) {
      // Calculates coordinates relative to the parent.
        ViewGroup* viewGroup = customBadgeParent == nullptr ? (ViewGroup*) anchorView->getParent() : customBadgeParent;
        viewGroup->offsetDescendantRectToMyCoords(anchorView, anchorRect);
    }
  
    calculateCenterAndBounds(mContext, anchorRect, anchorView);
  
    BadgeUtils::updateBadgeBounds(mBadgeBounds, mBadgeCenterX, mBadgeCenterY, mHalfBadgeWidth, mHalfBadgeHeight);
  
    mShapeDrawable->setCornerRadius(mCornerRadius);
    if (tmpRect!=mBadgeBounds) {
        mShapeDrawable->setBounds(mBadgeBounds);
    }
}

void BadgeDrawable::calculateCenterAndBounds(Context* context,const Rect& anchorRect, View* anchorView) {
    switch (mSavedState->mBadgeGravity) {
    case BOTTOM_END:
    case BOTTOM_START:
        mBadgeCenterY = anchorRect.bottom() - mSavedState->mVerticalOffset;
        break;
    case TOP_END:
    case TOP_START:
    default:
        mBadgeCenterY = anchorRect.top + mSavedState->mVerticalOffset;
        break;
    }
    if (getNumber() <= MAX_CIRCULAR_BADGE_NUMBER_COUNT) {
        mCornerRadius = !hasNumber() ? mBadgeRadius : mBadgeWithTextRadius;
        mHalfBadgeHeight = mCornerRadius;
        mHalfBadgeWidth = mCornerRadius;
    } else {
        mCornerRadius = mBadgeWithTextRadius;
        mHalfBadgeHeight = mCornerRadius;
        std::string badgeText = getBadgeText();
        mTextLayout->setText(badgeText);
        mTextLayout->relayout(1);
        mHalfBadgeWidth = mTextLayout->getMaxLineWidth()/2.f + mBadgeWidePadding;
    }
  
    const int inset = context->getDimensionPixelSize(hasNumber()
                    ? "@cdroid:dimen/mtrl_badge_text_horizontal_edge_offset"
                    : "@cdroid:dimen/mtrl_badge_horizontal_edge_offset");
    // Update the centerX based on the badge width and 'inset' from start or end boundary of anchor.
    switch (mSavedState->mBadgeGravity) {
    case BOTTOM_START:
    case TOP_START:
        mBadgeCenterX = anchorView->getLayoutDirection() == View::LAYOUT_DIRECTION_LTR
              ? anchorRect.left - mHalfBadgeWidth + inset + mSavedState->mHorizontalOffset
              : anchorRect.right() + mHalfBadgeWidth - inset - mSavedState->mHorizontalOffset;
      break;
    case BOTTOM_END:
    case TOP_END:
    default:
        mBadgeCenterX =anchorView->getLayoutDirection() == View::LAYOUT_DIRECTION_LTR
              ? anchorRect.right() + mHalfBadgeWidth - inset - mSavedState->mHorizontalOffset
              : anchorRect.left - mHalfBadgeWidth + inset + mSavedState->mHorizontalOffset;
        break;
    }
}

void BadgeDrawable::drawText(Canvas& canvas) {
    Rect textBounds;
    const std::string badgeText = getBadgeText();
    const int textBoundHeight= mTextLayout->getHeight()/2;
    const int textBoundWidth = mTextLayout->getLineWidth(0);
    canvas.move_to(mBadgeCenterX-textBoundWidth/2,/*mBadgeCenterY + textBoundHeight+*/mTextLayout->getLineBaseline(0));
    canvas.set_color(mSavedState->mBadgeTextColor);
    canvas.set_font_size(mTextLayout->getFontSize());
    //canvas.show_text(badgeText);
    canvas.draw_text(mBadgeBounds,badgeText,Gravity::CENTER);
}

std::string BadgeDrawable::getBadgeText() {
  // If number exceeds max count, show badgeMaxCount+ instead of the number.
  if (getNumber() <= mMaxBadgeNumber) {
      //return NumberFormat.getInstance().format(getNumber());
      return std::to_string(getNumber());
  } else {
      //return context->getString(R.string.mtrl_exceed_max_badge_number_suffix,
      //    mMaxBadgeNumber, DEFAULT_EXCEED_MAX_BADGE_NUMBER_SUFFIX);
      return "";
  }
}

void BadgeDrawable::updateMaxBadgeNumber() {
    mMaxBadgeNumber = (int) std::pow(10.0d, (double) getMaxCharacterCount() - 1) - 1;
}
}/*endof namespace*/
#endif
