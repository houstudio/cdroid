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
#include <drawable/badgestate.h>
#include <animation/animationutils.h>
#include <widget/framelayout.h>
#include <core/xmlpullparser.h>
#include <core/layout.h>
#include <widget/R.h>
namespace cdroid{

BadgeState::State*BadgeDrawable::getSavedState()const{
    return mState->getOverridingState();
}

BadgeDrawable* BadgeDrawable::createFromState(Context* context, BadgeState::State* savedState) {
    BadgeDrawable* badge = new BadgeDrawable(context,"","","",savedState);
    return badge;
}

BadgeDrawable::~BadgeDrawable(){
    delete mShapeDrawable;
    delete mState;
    delete mTextLayout;
}

BadgeDrawable* BadgeDrawable::create(Context* context) {
    return new BadgeDrawable(context,"","","", nullptr);
}

BadgeDrawable* BadgeDrawable::createFromResource(Context* context, const std::string& id) {
    int type;
    XmlPullParser parser(context,id);
    const AttributeSet& attrs = parser;
    if(!parser)return nullptr;
    while( ((type=parser.next())!=XmlPullParser::START_TAG) && (type!=XmlPullParser::END_DOCUMENT)){
        //NOTHING
    }
    const std::string tag=parser.getName();
    LOGE_IF(tag.compare("badge"),"invalid resource tag:%s[%s] ",tag.c_str(),id.c_str());
    return new BadgeDrawable(context, id, "","", 0/*style*/);
}

void BadgeDrawable::setVisible(bool visible) {
    mState->setVisible(visible);
    onVisibilityUpdated();
}

void BadgeDrawable::onVisibilityUpdated(){
    bool visible = mState->isVisible();
    Drawable::setVisible(visible,/*restart*/false);
}

void BadgeDrawable::setBadgeFixedEdge(int fixedEdge) {
    if (mState->mBadgeFixedEdge != fixedEdge) {
        mState->mBadgeFixedEdge = fixedEdge;
        updateCenterAndBounds();
    }
}

void BadgeDrawable::restoreState() {
    onBadgeShapeAppearanceUpdated();
    onBadgeTextAppearanceUpdated();

    onMaxBadgeLengthUpdated();

    onBadgeContentUpdated();
    //onAlphaUpdated();
    onBackgroundColorUpdated();
    onBadgeTextColorUpdated();
    onBadgeGravityUpdated();

    updateCenterAndBounds();
    onVisibilityUpdated();
}

BadgeDrawable::BadgeDrawable(Context* context,const std::string&badgeResId,
            const std::string&defStyleAttr,const std::string&defStyleRes,BadgeState::State*savedState) {
    mContext = context;
    mTextLayout = new Layout(14,INT_MAX);
    mTextLayout->setMultiline(false);
    mAnchorView = nullptr;
    mCustomBadgeParent = nullptr;
    mBadgeRadius = 6;
    mBadgeCenterX = 0;
    mBadgeCenterY = 0;
    mCornerRadius = BADGE_RADIUS_NOT_SPECIFIED;
    mBadgeWithTextRadius =6;
    mMaxBadgeNumber =BADGE_CONTENT_NOT_TRUNCATED;
    //ThemeEnforcement.checkMaterialTheme(context);
    //Resources res = context.getResources();
    mShapeDrawable = new GradientDrawable();

    mState =new BadgeState(context,badgeResId,defStyleAttr,defStyleRes,savedState);
    mBadgeRadius = context->getDimensionPixelSize("cdroid:dimen/mtrl_badge_radius",mBadgeRadius);
    mBadgeWidePadding = context->getDimensionPixelSize("cdroid:dimen/mtrl_badge_long_text_horizontal_padding",0);
    mBadgeWithTextRadius = context->getDimensionPixelSize("cdroid::dimen/mtrl_badge_with_text_radius",mBadgeWithTextRadius);
  
    mState = new BadgeState(context,badgeResId,defStyleAttr,defStyleRes,savedState);
    //setBackgroundColor(mState->mBackgroundColor);
    setTextAppearance("@cdroid:style/TextAppearance.MaterialComponents.Badge");
    restoreState();
}

void BadgeDrawable::updateBadgeCoordinates(View* anchorView) {
    updateBadgeCoordinates(anchorView, nullptr);
}

void BadgeDrawable::updateBadgeCoordinates(View* anchorView, FrameLayout* customBadgeParent) {
    mAnchorView = anchorView;
    mCustomBadgeParent = customBadgeParent;

    updateAnchorParentToNotClip(anchorView);
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
    return mShapeDrawable->getColor()->getDefaultColor();
}

void BadgeDrawable::setBackgroundColor(int backgroundColor) {
    mState->setBackgroundColor(backgroundColor);
    onBackgroundColorUpdated();
}

void BadgeDrawable::onBackgroundColorUpdated(){
    auto  backgroundColorStateList = ColorStateList::valueOf(mState->getBackgroundColor());
    if (mShapeDrawable->getColor()!=backgroundColorStateList){
        mShapeDrawable->setColor(backgroundColorStateList);
        invalidateSelf();
    }
}

int BadgeDrawable::getBadgeTextColor() const{
    return mState->getBadgeTextColor();
}

void BadgeDrawable::setBadgeTextColor(int badgeTextColor) {
    if (mState->getBadgeTextColor() != badgeTextColor){
        mState->setBadgeTextColor (badgeTextColor);
        onBadgeTextColorUpdated();
    }
}

void BadgeDrawable::onBadgeTextColorUpdated() {
    //textDrawableHelper.getTextPaint().setColor(state.getBadgeTextColor());
    invalidateSelf();
}

bool BadgeDrawable::hasNumber() const{
    return !mState->hasText()&&mState->hasNumber();
}

int BadgeDrawable::getNumber() const{
    return mState->hasNumber()?mState->getNumber():0;
}

void BadgeDrawable::setNumber(int number) {
    number = std::max(0, number);
    if (mState->getNumber() != number) {
        mState->setNumber(number);
        updateCenterAndBounds();
        onNumberUpdated();
    }
}

/** Resets any badge number so that a numberless badge will be displayed. */
void BadgeDrawable::clearNumber() {
    if(mState->hasNumber()){
        mState->clearNumber();
        onNumberUpdated();
    }
}

void BadgeDrawable::onNumberUpdated() {
    // The text has priority over the number so when the number changes, the badge is updated
    // only if there is no text.
    if (!hasText()) {
        onBadgeContentUpdated();
    }
}

bool BadgeDrawable::hasText()const{
    return mState->hasText();
}

std::string BadgeDrawable::getText()const{
    return mState->getText();
}

void BadgeDrawable::setText(const std::string& text) {
    if (mState->getText()!= text) {
        mState->setText(text);
        onTextUpdated();
    }
}

void BadgeDrawable::clearText(){
    if(mState->hasText()){
        mState->clearText();
        onTextUpdated();
    }
}

void BadgeDrawable::onTextUpdated() {
    // The text has priority over the number so any text change updates the badge content.
    onBadgeContentUpdated();
}

int BadgeDrawable::getMaxCharacterCount() const{
     return mState->getMaxCharacterCount();
}

void BadgeDrawable::setMaxCharacterCount(int maxCharacterCount) {
    if (mState->getMaxCharacterCount() != maxCharacterCount) {
       mState->setMaxCharacterCount(maxCharacterCount);
       onMaxBadgeLengthUpdated();
    }
}

int BadgeDrawable::getMaxNumber() const{
    return mState->getMaxNumber();
}

void BadgeDrawable::setMaxNumber(int maxNumber) {
    if (mState->getMaxNumber() != maxNumber) {
        mState->setMaxNumber(maxNumber);
        onMaxBadgeLengthUpdated();
    }
}

void BadgeDrawable::onMaxBadgeLengthUpdated() {
    updateMaxBadgeNumber();
    //textDrawableHelper.setTextSizeDirty(true);
    updateCenterAndBounds();
    invalidateSelf();
}

int BadgeDrawable::getBadgeGravity() const{
    return mState->getBadgeGravity();
}

void BadgeDrawable::setBadgeGravity(int gravity) {
    if (mState->getBadgeGravity() != gravity) {
        mState->setBadgeGravity(gravity);
         onBadgeGravityUpdated();
    }
}
void BadgeDrawable::onBadgeGravityUpdated(){
    if (mAnchorView != nullptr && mAnchorView != nullptr) {
        updateBadgeCoordinates(
            mAnchorView, mCustomBadgeParent != nullptr ? mCustomBadgeParent : nullptr);
    }
}

bool BadgeDrawable::isStateful() const{
    return false;
}

void BadgeDrawable::setColorFilter(const cdroid::RefPtr<ColorFilter>& colorFilter) {
    // Intentionally empty.
}

int BadgeDrawable::getAlpha()const {
    return mState->getAlpha();
}

void BadgeDrawable::setAlpha(int alpha) {
    mState->setAlpha(alpha);
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
    if (hasBadgeContent()) {
        drawBadgeContent(canvas);
    }
}

void BadgeDrawable::onTextSizeChange() {
    invalidateSelf();
}

bool BadgeDrawable::onStateChange(const std::vector<int>& state) {
    return Drawable::onStateChange(state);
}

void BadgeDrawable::setContentDescriptionNumberless(const std::string& charSequence) {
    mState->setContentDescriptionNumberless(charSequence);
}

/*void BadgeDrawable::setContentDescriptionQuantityStringsResource(int stringsResource) {
    mState->contentDescriptionQuantityStrings = stringsResource;
}*/

void BadgeDrawable::setContentDescriptionExceedsMaxBadgeNumberStringResource(int stringsResource) {
    //mState->setContentDescriptionExceedsMaxBadgeNumberRes(stringsResource);
}

std::string BadgeDrawable::getContentDescription() const{
    /*if (!isVisible()) {
        return null;
    }
    if (hasNumber()) {
        if (mState.contentDescriptionQuantityStrings > 0) {
          Context context = mContext.get();
          if (context == null) {
              return null;
          }
          if (getNumber() <= mMaxBadgeNumber) {
              return context
                  .getResources()
                  .getQuantityString(
                      mState.contentDescriptionQuantityStrings, getNumber(), getNumber());
          } else {
              return context.getString(
                  mState.contentDescriptionExceedsMaxBadgeNumberRes, mMaxBadgeNumber);
          }
        } else {
            return null;
        }
    } else {
        return mState->contentDescriptionNumberless;
    }*/
    return"";
}

void BadgeDrawable::setHorizontalOffset(int px) {
    setHorizontalOffsetWithoutText(px);
    setHorizontalOffsetWithText(px);
}

int BadgeDrawable::getHorizontalOffset() const{
    return mState->getHorizontalOffsetWithoutText();
}

void BadgeDrawable::setHorizontalOffsetWithoutText(int px) {
    mState->setHorizontalOffsetWithoutText(px);
    updateCenterAndBounds();
}

int BadgeDrawable::getHorizontalOffsetWithoutText() const{
    return mState->getHorizontalOffsetWithoutText();
}

void BadgeDrawable::setHorizontalOffsetWithText(int px) {
    mState->setHorizontalOffsetWithText(px);
    updateCenterAndBounds();
}

int BadgeDrawable::getHorizontalOffsetWithText() const{
    return mState->getHorizontalOffsetWithText();
}

void BadgeDrawable::setVerticalOffset(int px) {
    setVerticalOffsetWithoutText(px);
    setVerticalOffsetWithText(px);
}

int BadgeDrawable::getVerticalOffset() const{
    return mState->getVerticalOffsetWithoutText();
}

void BadgeDrawable::setVerticalOffsetWithoutText(int px) {
    mState->setVerticalOffsetWithoutText(px);
    updateCenterAndBounds();
}

int BadgeDrawable::getVerticalOffsetWithoutText()const{
    return mState->getVerticalOffsetWithoutText();
}

void BadgeDrawable::setVerticalOffsetWithText(int px) {
    mState->setVerticalOffsetWithText(px);
    updateCenterAndBounds();
}

int BadgeDrawable::getVerticalOffsetWithText() const{
    return mState->getVerticalOffsetWithText();
}

void BadgeDrawable::setLargeFontVerticalOffsetAdjustment(int px) {
    mState->setLargeFontVerticalOffsetAdjustment(px);
    updateCenterAndBounds();
}

int BadgeDrawable::getLargeFontVerticalOffsetAdjustment() const{
    return mState->getLargeFontVerticalOffsetAdjustment();
}

void BadgeDrawable::setAdditionalVerticalOffset(int px) {
    mState->setAdditionalVerticalOffset(px);
    updateCenterAndBounds();
}

int BadgeDrawable::getAdditionalVerticalOffset() const{
    return mState->getAdditionalVerticalOffset();
}

void BadgeDrawable::setTextAppearance(const std::string& id) {
    const AttributeSet atts = mContext->obtainStyledAttributes(id);
    const int textSize = atts.getInt("textSize",12);
    Typeface*tf =Typeface::create(atts.getString("fontFamily"),0);
    mTextLayout->setFontSize(textSize);
    mTextLayout->setTypeface(tf);
    atts.getColor("textColor");
}

void BadgeDrawable::onBadgeTextAppearanceUpdated() {
    if (mContext == nullptr) {
        return;
    }
    /*TextAppearance textAppearance = new TextAppearance(context, state.getTextAppearanceResId());
    if (textDrawableHelper.getTextAppearance() == textAppearance) {
        return;
    }
    //textDrawableHelper.setTextAppearance(textAppearance, context);
    */
    onBadgeTextColorUpdated();
    updateCenterAndBounds();
    invalidateSelf();
}

void BadgeDrawable::setBadgeWithoutTextShapeAppearance(const std::string& id) {
    mState->setBadgeShapeAppearanceResId(id);
    onBadgeShapeAppearanceUpdated();
}

void BadgeDrawable::setBadgeWithoutTextShapeAppearanceOverlay(const std::string& id) {
    mState->setBadgeShapeAppearanceOverlayResId(id);
    onBadgeShapeAppearanceUpdated();
}

void BadgeDrawable::setBadgeWithTextShapeAppearance(const std::string& id) {
    mState->setBadgeWithTextShapeAppearanceResId(id);
    onBadgeShapeAppearanceUpdated();
}

void BadgeDrawable::setBadgeWithTextShapeAppearanceOverlay(const std::string& id) {
    mState->setBadgeWithTextShapeAppearanceOverlayResId(id);
    onBadgeShapeAppearanceUpdated();
}
void BadgeDrawable::onBadgeShapeAppearanceUpdated() {
    if (mContext == nullptr) {
        return;
    }
    /*shapeDrawable.setShapeAppearanceModel(
      ShapeAppearanceModel.builder(
              context,
              hasBadgeContent()
                  ? state.getBadgeWithTextShapeAppearanceResId()
                  : state.getBadgeShapeAppearanceResId(),
              hasBadgeContent()
                  ? state.getBadgeWithTextShapeAppearanceOverlayResId()
                  : state.getBadgeShapeAppearanceOverlayResId())
          .build());*/
    invalidateSelf();
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
    if (customBadgeParent != nullptr) {
        // Calculates coordinates relative to the parent.
        customBadgeParent->offsetDescendantRectToMyCoords(anchorView, anchorRect);
    }
  
    calculateCenterAndBounds(anchorRect, anchorView);
  
    BadgeUtils::updateBadgeBounds(mBadgeBounds, mBadgeCenterX, mBadgeCenterY, mHalfBadgeWidth, mHalfBadgeHeight);
 
    if(mCornerRadius!=BADGE_RADIUS_NOT_SPECIFIED){
        mShapeDrawable->setCornerRadius(mCornerRadius);
    }
    if (tmpRect!=mBadgeBounds) {
        mShapeDrawable->setBounds(mBadgeBounds);
    }
}

int BadgeDrawable::getTotalVerticalOffsetForState() const{
    int vOffset = mState->getVerticalOffsetWithoutText();
    if (hasBadgeContent()) {
        vOffset = mState->getVerticalOffsetWithText();
        if (mContext != nullptr) {
             float progress =
              AnimationUtils::lerp(0.f, 1.f,FONT_SCALE_THRESHOLD, 1.f, 1.f/*MaterialResources.getFontScale(mContext)*/ - 1.f);
              vOffset = AnimationUtils::lerp(vOffset, vOffset - mState->getLargeFontVerticalOffsetAdjustment(), progress);
        }
    }

    // If the offset alignment mode is at the edge of the anchor, we want to move the badge
    // so that its origin is at the edge.
    if (mState->mOffsetAlignmentMode == OFFSET_ALIGNMENT_MODE_EDGE) {
        vOffset -= std::round(mHalfBadgeHeight);
    }
    return vOffset + mState->getAdditionalVerticalOffset();
}

int BadgeDrawable::getTotalHorizontalOffsetForState() const{
    int hOffset = hasBadgeContent()
          ? mState->getHorizontalOffsetWithText()
          : mState->getHorizontalOffsetWithoutText();
    // If the offset alignment mode is legacy, then we want to add the legacy inset to the offset.
    if (mState->mOffsetAlignmentMode == OFFSET_ALIGNMENT_MODE_LEGACY) {
        hOffset += hasBadgeContent() ? mState->mHorizontalInsetWithText : mState->mHorizontalInset;
    }
    return hOffset + mState->getAdditionalHorizontalOffset();
}

void BadgeDrawable::calculateCenterAndBounds(const Rect& anchorRect, View* anchorView) {
    mCornerRadius = hasBadgeContent()?mState->mBadgeWithTextRadius:mState->mBadgeRadius;
    if (mCornerRadius != BADGE_RADIUS_NOT_SPECIFIED) {
        mHalfBadgeWidth = mCornerRadius;
        mHalfBadgeHeight = mCornerRadius;
    } else {
        mHalfBadgeWidth =
          std::round(hasBadgeContent() ? mState->mBadgeWithTextWidth / 2 : mState->mBadgeWidth / 2);
        mHalfBadgeHeight =
          std::round(hasBadgeContent() ? mState->mBadgeWithTextHeight / 2 : mState->mBadgeHeight / 2);
    }

    // If the badge has a number, we want to make sure that the badge is at least tall/wide
    // enough to encompass the text with padding.
    if(hasBadgeContent()){
        std::string badgeContent=getBadgeContent();
        mTextLayout->setText(badgeContent);
        mTextLayout->relayout(1);
        mHalfBadgeWidth = std::max(mHalfBadgeWidth,
              mTextLayout->getMaxLineWidth() / 2.f
                  + mState->getBadgeHorizontalPadding());

        mHalfBadgeHeight =std::max(mHalfBadgeHeight,
              mTextLayout->getLineHeight(0)/ 2.f
                  + mState->getBadgeVerticalPadding());

        // If the badge has text, it should at least have the same width as it does height
        mHalfBadgeWidth = std::max(mHalfBadgeWidth, mHalfBadgeHeight);
    }

    const int totalVerticalOffset = getTotalVerticalOffsetForState();
    switch (mState->getBadgeGravity()) {
    case BOTTOM_END:
    case BOTTOM_START:
        mBadgeCenterY = anchorRect.bottom() - totalVerticalOffset;
        break;
    case TOP_END:
    case TOP_START:
    default:
        mBadgeCenterY = anchorRect.top + totalVerticalOffset;
        break;
    }
  
    const int totalHorizontalOffset = getTotalHorizontalOffsetForState();
    // Update the centerX based on the badge width and 'inset' from start or end boundary of anchor.
    switch (mState->getBadgeGravity()) {
    case BOTTOM_START:
    case TOP_START:
        mBadgeCenterX = mState->mBadgeFixedEdge == BADGE_FIXED_EDGE_START
            ? (anchorView->getLayoutDirection() == View::LAYOUT_DIRECTION_LTR
              ? anchorRect.left + mHalfBadgeWidth -(mHalfBadgeHeight*2 -totalHorizontalOffset)
              : anchorRect.right() - mHalfBadgeWidth +(mHalfBadgeHeight*2 - totalHorizontalOffset))
            : (anchorView->getLayoutDirection() == View::LAYOUT_DIRECTION_LTR
                ? anchorRect.left - mHalfBadgeWidth + totalHorizontalOffset
                : anchorRect.right() + mHalfBadgeWidth - totalHorizontalOffset);
      break;
    case BOTTOM_END:
    case TOP_END:
    default:
        mBadgeCenterX = mState->mBadgeFixedEdge ==BADGE_FIXED_EDGE_START
            ? (anchorView->getLayoutDirection() == View::LAYOUT_DIRECTION_LTR
              ? anchorRect.right() + mHalfBadgeWidth - totalHorizontalOffset
              : anchorRect.left - mHalfBadgeWidth + totalHorizontalOffset)
            : (anchorView->getLayoutDirection() == View::LAYOUT_DIRECTION_RTL
              ? anchorRect.right() - mHalfBadgeWidth + (mHalfBadgeHeight * 2 - totalHorizontalOffset)
              : anchorRect.left + mHalfBadgeWidth - (mHalfBadgeHeight * 2 - totalHorizontalOffset)); 

        break;
    }
    if (mState->isAutoAdjustedToGrandparentBounds()) {
        autoAdjustWithinGrandparentBounds(anchorView);
    } else {
        autoAdjustWithinViewBounds(anchorView, nullptr);
    }
}

void BadgeDrawable::autoAdjustWithinViewBounds(View* anchorView, View* ancestorView) {
    // The top of the badge may be cut off by the anchor view's ancestor view if clipChildren is
    // false (eg. in the case of the bottom navigation bar). If that is the case, we should adjust
    // the position of the badge.
  
    float totalAnchorYOffset;
    float totalAnchorXOffset;
    ViewGroup* anchorParent;
    // If there is a custom badge parent, we should use its coordinates instead of the anchor
    // view's parent.
    ViewGroup* customAnchorParent = getCustomBadgeParent();
    if (customAnchorParent == nullptr) {
        totalAnchorYOffset = anchorView->getY();
        totalAnchorXOffset = anchorView->getX();
        anchorParent = anchorView->getParent();
    } else {
        totalAnchorYOffset = 0;
        totalAnchorXOffset = 0;
        anchorParent = customAnchorParent;
    }
  
    ViewGroup* currentViewParent = anchorParent;
    while (/*currentViewParent instanceof View &&*/currentViewParent&& currentViewParent != ancestorView) {
        ViewGroup* viewGrandparent = currentViewParent->getParent();
        if (viewGrandparent==nullptr|| viewGrandparent->getClipChildren()) {
            break;
        }
        View* currentViewGroup = currentViewParent;
        totalAnchorYOffset += currentViewGroup->getY();
        totalAnchorXOffset += currentViewGroup->getX();
        currentViewParent = currentViewParent->getParent();
    }
  
    // If currentViewParent is not a View, all ancestor Views did not clip their children
    /*if (!(currentViewParent instanceof View)) {
        return;
    }*/
  
    float topCutOff = getTopCutOff(totalAnchorYOffset);
    float leftCutOff = getLeftCutOff(totalAnchorXOffset);
    float bottomCutOff = getBottomCutOff(currentViewParent->getHeight(), totalAnchorYOffset);
    float rightCutOff = getRightCutoff(currentViewParent->getWidth(), totalAnchorXOffset);
  
    // If there's any part of the badge that is cut off, we move the badge accordingly.
    if (topCutOff < 0) {
        mBadgeCenterY += std::abs(topCutOff);
    }
    if (leftCutOff < 0) {
        mBadgeCenterX += std::abs(leftCutOff);
    }
    if (bottomCutOff > 0) {
        mBadgeCenterY -= std::abs(bottomCutOff);
    }
    if (rightCutOff > 0) {
        mBadgeCenterX -= std::abs(rightCutOff);
    }
}

/** Adjust the badge placement so it is within its anchor's grandparent view. */
void BadgeDrawable::autoAdjustWithinGrandparentBounds(View* anchorView) {
    // If there is a custom badge parent, we should use its coordinates instead of the anchor
    // view's parent.
    ViewGroup* customAnchor = getCustomBadgeParent();
    ViewGroup* anchorParent = nullptr;
    if (customAnchor == nullptr) {
        anchorParent = anchorView->getParent();
    } else {
        anchorParent = customAnchor;
    }
    if (1/*anchorParent instanceof View && anchorParent.getParent() instanceof View*/) {
        autoAdjustWithinViewBounds(anchorView, anchorParent->getParent());
    }
}

float BadgeDrawable::getTopCutOff(float totalAnchorYOffset) const{
    return mBadgeCenterY - mHalfBadgeHeight + totalAnchorYOffset;
}

float BadgeDrawable::getLeftCutOff(float totalAnchorXOffset) const{
    return mBadgeCenterX - mHalfBadgeWidth + totalAnchorXOffset;
}

float BadgeDrawable::getBottomCutOff(float ancestorHeight, float totalAnchorYOffset) const{
    return mBadgeCenterY + mHalfBadgeHeight - ancestorHeight + totalAnchorYOffset;
}

float BadgeDrawable::getRightCutoff(float ancestorWidth, float totalAnchorXOffset) const{
    return mBadgeCenterX + mHalfBadgeWidth - ancestorWidth + totalAnchorXOffset;
}

void BadgeDrawable::drawBadgeContent(Canvas& canvas) {
    Rect textBounds;
    const std::string badgeContent = getBadgeContent();
    const int textBoundHeight= mTextLayout->getHeight()/2;
    const int textBoundWidth = mTextLayout->getLineWidth(0);
    canvas.move_to(mBadgeCenterX-textBoundWidth/2,/*mBadgeCenterY + textBoundHeight+*/mTextLayout->getLineBaseline(0));
    canvas.set_color(mState->getBadgeTextColor());
    canvas.set_font_size(mTextLayout->getFontSize());
    //canvas.show_text(badgeText);
    canvas.draw_text(mBadgeBounds,badgeContent,Gravity::CENTER);
}

bool BadgeDrawable::hasBadgeContent() const{
    return hasText() || hasNumber();
}

std::string BadgeDrawable::getBadgeContent() const{
    if (hasText()) {
        return getTextBadgeText();
    } else if (hasNumber()) {
        return getNumberBadgeText();
    } else {
        return "";
    }
}

std::string BadgeDrawable::getTextBadgeText() const{
    std::string text = getText();
    const int maxCharacterCount = getMaxCharacterCount();
    if (maxCharacterCount == BADGE_CONTENT_NOT_TRUNCATED) {
        return text;
    }
    if(text.length()>maxCharacterCount){
        if(mContext==nullptr){
            return "";
        }
        text = text.substr(0,maxCharacterCount-1);
        return text + DEFAULT_EXCEED_MAX_BADGE_TEXT_SUFFIX;
    } else {
        return text;
    }
}

std::string BadgeDrawable::getNumberBadgeText() const{
    // If number exceeds max count, show badgeMaxCount+ instead of the number.
    if (mMaxBadgeNumber == BADGE_CONTENT_NOT_TRUNCATED || getNumber() <= mMaxBadgeNumber) {
        return std::to_string(getNumber());//NumberFormat.getInstance(state.getNumberLocale()).format(getNumber());
    } else {
        if(mContext==nullptr){
            return "";
        }
        return std::to_string(mMaxBadgeNumber) + DEFAULT_EXCEED_MAX_BADGE_NUMBER_SUFFIX;
        /*return String.format(
            state.getNumberLocale(),
            context.getString(R.string.mtrl_exceed_max_badge_number_suffix),
            maxBadgeNumber,DEFAULT_EXCEED_MAX_BADGE_NUMBER_SUFFIX);
        */
    }
}

void BadgeDrawable::onBadgeContentUpdated() {
    //textDrawableHelper.setTextSizeDirty(true);
    onBadgeShapeAppearanceUpdated();
    updateCenterAndBounds();
    invalidateSelf();
}

void BadgeDrawable::updateMaxBadgeNumber() {
    if(getMaxCharacterCount() != BADGE_CONTENT_NOT_TRUNCATED){
        mMaxBadgeNumber = (int) std::pow(10.0d, (double) getMaxCharacterCount() - 1) - 1;
    }else{
        mMaxBadgeNumber = getMaxNumber();
    }
}
}/*endof namespace*/
