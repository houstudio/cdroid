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
#include <widget/internal_R.h>
#include <core/context.h>
#include <widget/compoundbutton.h>
#include <widget/radiogroup.h>
#include <widget/framework_styleable.h>
#include <widget/checkbox.h>
#include <widget/radiobutton.h>
#include <porting/cdlog.h>
namespace cdroid{
using namespace cdroid::internal;

DECLARE_WIDGET2(CompoundButton, "android.widget.CompoundButton");

CompoundButton::CompoundButton(Context*ctx)
    :CompoundButton(ctx,nullptr){}

CompoundButton::CompoundButton(Context*ctx,const AttributeSet* attrs):CompoundButton(ctx,attrs,0){}

CompoundButton::CompoundButton(Context*ctx,const AttributeSet* pAttrs,int defStyleAttr)
  :Button(ctx,pAttrs, defStyleAttr){
    initCompoundButton();
    // AOSP CompoundButton ctor: obtainStyledAttributes(pAttrs, styleable, defStyleAttr, 0);
    // reads button, buttonTintMode, buttonTint, checked in that order, then applyButtonTint().
    auto ta = ctx->obtainStyledAttributes(pAttrs, R::styleable::CompoundButton, defStyleAttr, 0);
    Drawable* d = ta->getDrawable(R::styleable::CompoundButton_button);
    if (d) setButtonDrawable(d);

    if (ta->hasValue(R::styleable::CompoundButton_buttonTintMode)) {
        mButtonBlendMode = Drawable::parseTintMode(ta->getInt(
                R::styleable::CompoundButton_buttonTintMode, -1), mButtonBlendMode);
        mHasButtonBlendMode = true;
    }

    if (ta->hasValue(R::styleable::CompoundButton_buttonTint)) {
        mButtonTintList = ta->getColorStateList(R::styleable::CompoundButton_buttonTint);
        mHasButtonTint = true;
    }

    setChecked(ta->getBoolean(R::styleable::CompoundButton_checked, false));
    mCheckedFromResource = true;

    applyButtonTint();
}

void CompoundButton::initCompoundButton(){
    mChecked = false;
    mBroadcasting = false;
    mButtonDrawable = nullptr;
    mButtonTintList = nullptr;
    mButtonBlendMode = PorterDuff::Mode::NOOP;
    mHasButtonTint = false;
    mHasButtonBlendMode = false;
    mOnCheckedChangeListener = nullptr;
    mOnCheckedChangeWidgetListener = nullptr;
    mCheckedFromResource = false;
}

void CompoundButton::setChecked(bool checked){
    if (mChecked != checked) {
        mCheckedFromResource = false;
        mChecked = checked;
        refreshDrawableState();
        notifyViewAccessibilityStateChangedIfNeeded(AccessibilityEvent::CONTENT_CHANGE_TYPE_UNDEFINED);
        // Avoid infinite recursions if setChecked() is called from a listener
        if (mBroadcasting)return;

        mBroadcasting = true;
        if (mOnCheckedChangeListener) mOnCheckedChangeListener(*this, mChecked);
        if (mOnCheckedChangeWidgetListener ) mOnCheckedChangeWidgetListener(*this, mChecked);
        //final AutofillManager afm = mContext.getSystemService(AutofillManager.class);
        //if (afm != null)  afm.notifyValueChanged(this);
        mBroadcasting = false;
    }
    setDefaultStateDescription();
}

bool CompoundButton::isChecked()const{
    return mChecked;
}

void CompoundButton::toggle(){
    setChecked(!mChecked); 
}

CompoundButton::~CompoundButton(){
    delete mButtonDrawable;
}

std::string CompoundButton::getButtonStateDescription() {
    if (isChecked()) {
        return mContext->getString(R::string::checked);
    } else {
        return mContext->getString(R::string::not_checked);
    }
}

void CompoundButton::setStateDescription(const std::string&stateDescription) {
    mCustomStateDescription = stateDescription;
    if (stateDescription.empty()) {
        setDefaultStateDescription();
    } else {
        View::setStateDescription(stateDescription);
    }
}

/** @hide **/
void CompoundButton::setDefaultStateDescription() {
    if (mCustomStateDescription.empty()) {
        View::setStateDescription(getButtonStateDescription());
    }
}

std::vector<int>CompoundButton::onCreateDrawableState(int extraSpace){
    std::vector<int>drawableState = Button::onCreateDrawableState(extraSpace);
    if (isChecked()) {
        mergeDrawableStates(drawableState,{cdroid::internal::R::attr::state_checked});
    }
    return drawableState;
}

void CompoundButton::drawableStateChanged() {
    Button::drawableStateChanged();
    if (mButtonDrawable  && mButtonDrawable->isStateful()
            && mButtonDrawable->setState(getDrawableState())) {
        invalidateDrawable(*mButtonDrawable);
    }
}

void CompoundButton::drawableHotspotChanged(float x,float y){
    Button::drawableHotspotChanged(x,y);
    if(mButtonDrawable)mButtonDrawable->setHotspot(x,y);
}

bool CompoundButton::performClick(){
    toggle();
    const bool handled = Button::performClick();
    if (!handled) {
        // View only makes a sound effect if the onClickListener was
        // called, so we'll need to make one here instead.
        playSoundEffect(SoundEffectConstants::CLICK);
    }
    return handled;
}

void CompoundButton::setButtonDrawable(int resid){
    Drawable* d= getContext()->getDrawable(resid);
    setButtonDrawable(d);
}

void CompoundButton::setButtonDrawable(Drawable*drawable){
    if (mButtonDrawable != drawable) {
        if (mButtonDrawable != nullptr) {
            mButtonDrawable->setCallback(nullptr);
            unscheduleDrawable(*mButtonDrawable);
        }
        delete mButtonDrawable;
        mButtonDrawable = drawable;

        if (drawable != nullptr) {
            drawable->setCallback(this);
            drawable->setLayoutDirection(getLayoutDirection());
            if (drawable->isStateful()) drawable->setState(getDrawableState());

            drawable->setVisible(getVisibility() == VISIBLE, false);
            setMinHeight(drawable->getIntrinsicHeight());
            applyButtonTint();
        }
    } 
}

Drawable* CompoundButton::getButtonDrawable()const{
    return mButtonDrawable;
}

bool CompoundButton::verifyDrawable(Drawable* who)const{
    return Button::verifyDrawable(who) || (who == mButtonDrawable);
}

void CompoundButton::onDetachedFromWindow(){
    if(mButtonDrawable)
        unscheduleDrawable(*mButtonDrawable);
}

void CompoundButton::jumpDrawablesToCurrentState(){
    Button::jumpDrawablesToCurrentState();
    if (mButtonDrawable!=nullptr){
        mButtonDrawable->jumpToCurrentState();
    }
}

void CompoundButton::setButtonTintList(const cdroid::RefPtr<ColorStateList>& tint) {
    mButtonTintList = tint;
    mHasButtonTint = true;

    applyButtonTint();
}

/**
 * @return the tint applied to the button drawable
 * @attr ref android.R.styleable#CompoundButton_buttonTint
 * @see #setButtonTintList(const cdroid::RefPtr<ColorStateList>)
 */
const cdroid::RefPtr<ColorStateList> CompoundButton::getButtonTintList() const{
    return mButtonTintList;
}

void CompoundButton::setButtonTintMode(PorterDuffMode tintMode){
    // AOSP: setButtonTintBlendMode(tintMode != null ? BlendMode.fromValue(tintMode.nativeInt) : null);
    setButtonTintBlendMode(tintMode);
}

PorterDuffMode CompoundButton::getButtonTintMode() const {
    // AOSP: mButtonBlendMode != null ? BlendMode.blendModeToPorterDuffMode(mButtonBlendMode) : null;
    return getButtonTintBlendMode();
}

void CompoundButton::setButtonTintBlendMode(PorterDuffMode tintMode){
    mButtonBlendMode = tintMode;
    mHasButtonBlendMode = true;

    applyButtonTint();
}

PorterDuffMode CompoundButton::getButtonTintBlendMode() const{
    return mButtonBlendMode;
}

void CompoundButton::applyButtonTint() {
    if (mButtonDrawable != nullptr && (mHasButtonTint || mHasButtonBlendMode)) {
        mButtonDrawable = mButtonDrawable->mutate();

        if (mHasButtonTint) {
            mButtonDrawable->setTintList(mButtonTintList);
        }

        if (mHasButtonBlendMode) {
            mButtonDrawable->setTintMode(mButtonBlendMode);
        }

        // The drawable (or one of its children) may not have been
        // stateful before applying the tint, so let's try again.
        if (mButtonDrawable->isStateful()) {
            mButtonDrawable->setState(getDrawableState());
        }
    }
}

std::string CompoundButton::getAccessibilityClassName()const{
    return "CompoundButton";
}

void CompoundButton::onInitializeAccessibilityEventInternal(AccessibilityEvent& event){
    Button::onInitializeAccessibilityEventInternal(event);
    event.setChecked(mChecked);
}

void CompoundButton::onInitializeAccessibilityNodeInfoInternal(AccessibilityNodeInfo& info){
    Button::onInitializeAccessibilityNodeInfoInternal(info);
    info.setCheckable(true);
    info.setChecked(mChecked);
}

void CompoundButton::setOnCheckedChangeListener(const OnCheckedChangeListener& listener) {
    mOnCheckedChangeListener = listener;
}

void CompoundButton::setOnCheckedChangeWidgetListener(const OnCheckedChangeListener& listener) {
    mOnCheckedChangeWidgetListener = listener;
}

int CompoundButton::getCompoundPaddingLeft() const{
    int padding = Button::getCompoundPaddingLeft();
    if ((false==isLayoutRtl()) && mButtonDrawable) {
        padding += mButtonDrawable->getIntrinsicWidth();
    }
    return padding;
}

int CompoundButton::getCompoundPaddingRight() const{
    int padding = Button::getCompoundPaddingRight();
    if (isLayoutRtl() && mButtonDrawable) {
        padding += mButtonDrawable->getIntrinsicWidth();
    }
    return padding;
}

int CompoundButton::getHorizontalOffsetForDrawables()const{
    return (mButtonDrawable == nullptr) ? 0 : mButtonDrawable->getIntrinsicWidth();
}

void CompoundButton::onDraw(Canvas&canvas){
    if (mButtonDrawable != nullptr) {
        const int verticalGravity = getGravity() & Gravity::VERTICAL_GRAVITY_MASK;
        const int drawableHeight = mButtonDrawable->getIntrinsicHeight();
        const int drawableWidth = mButtonDrawable->getIntrinsicWidth();

        int top;
        switch (verticalGravity) {
        case Gravity::BOTTOM         : top = getHeight() - drawableHeight;         break;
        case Gravity::CENTER_VERTICAL: top = (getHeight() - drawableHeight) / 2;   break;
        default:           top = 0;
        }

        const int left = isLayoutRtl() ? getWidth() - drawableWidth : 0;
        mButtonDrawable->setBounds(left,top, drawableWidth ,drawableHeight);
        Drawable* background = getBackground();
        if (background != nullptr) {
            background->setHotspotBounds(left, top,drawableWidth,drawableHeight);
        }
    }

    Button::onDraw(canvas);
    if (mButtonDrawable != nullptr) {
        if (mScrollX == 0 && mScrollY == 0) {
            mButtonDrawable->draw(canvas);
        } else {
            canvas.translate(mScrollX, mScrollY);
            mButtonDrawable->draw(canvas);
            canvas.translate(-mScrollX, -mScrollY);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////
DECLARE_WIDGET2(CheckBox, "android.widget.CheckBox");
CheckBox::CheckBox(Context*ctx):CheckBox(ctx,nullptr){}

CheckBox::CheckBox(Context*ctx,const AttributeSet* attrs)
    :CheckBox(ctx,attrs,cdroid::internal::R::attr::checkboxStyle){
}

CheckBox::CheckBox(Context*ctx,const AttributeSet* attrs,int defStyleAttr)
    :CompoundButton(ctx,attrs,defStyleAttr){
}

std::string CheckBox::getAccessibilityClassName()const{
    return "CheckBox";
}
//////////////////////////////////////////////////////////////
//class RadioButton:public CompoundButton

DECLARE_WIDGET2(RadioButton, "android.widget.RadioButton");
RadioButton::RadioButton(Context*ctx):RadioButton(ctx,nullptr){}

RadioButton::RadioButton(Context*ctx,const AttributeSet* attrs)
   :RadioButton(ctx,attrs, R::attr::radioButtonStyle){
}

RadioButton::RadioButton(Context*ctx,const AttributeSet* attrs,int defStyleAttr)
   :CompoundButton(ctx,attrs,defStyleAttr){
}

void RadioButton::toggle(){
    if(!isChecked())CompoundButton::toggle();
}

std::string RadioButton::getAccessibilityClassName()const{
    return "RadioButton";
}

// AOSP RadioButton.onInitializeAccessibilityNodeInfo: inside a RadioGroup the
// button reports its collection item info (row/column per group orientation,
// selection flag from the checked state).
void RadioButton::onInitializeAccessibilityNodeInfo(AccessibilityNodeInfo& info) {
    CompoundButton::onInitializeAccessibilityNodeInfo(info);
    if (dynamic_cast<RadioGroup*>(getParent())) {
        RadioGroup* radioGroup = (RadioGroup*) getParent();
        if (radioGroup->getOrientation() == LinearLayout::HORIZONTAL) {
            info.setCollectionItemInfo(AccessibilityNodeInfo::CollectionItemInfo::obtain(0, 1,
                    radioGroup->getIndexWithinVisibleButtons(this), 1, false, isChecked()));
        } else {
            info.setCollectionItemInfo(AccessibilityNodeInfo::CollectionItemInfo::obtain(
                    radioGroup->getIndexWithinVisibleButtons(this), 1, 0, 1,
                    false, isChecked()));
        }
    }
}

}/*endof namespace*/

