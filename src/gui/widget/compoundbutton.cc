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
#include <widget/compoundbutton.h>
#include <widget/checkbox.h>
#include <widget/radiobutton.h>
#include <cdlog.h>
namespace cdroid{

DECLARE_WIDGET(CompoundButton)

CompoundButton::CompoundButton(Context*ctx,const AttributeSet& attrs)
  :Button(ctx,attrs){
    initCompoundButton();
    setButtonDrawable(attrs.getString("button"));
    setChecked(attrs.getBoolean("checked"));
    mButtonTintList = attrs.getColorStateList("buttonTint");
    applyButtonTint();
}

CompoundButton::CompoundButton(const std::string&txt,int width,int height)
    :Button(txt,width,height){
    initCompoundButton();
}

void CompoundButton::initCompoundButton(){
    mChecked = false;
    mBroadcasting = false;
    mCheckedFromResource = false;
    mButtonDrawable = nullptr;
    mOnCheckedChangeListener = nullptr;
    mOnCheckedChangeWidgetListener = nullptr;
    mButtonTintMode = PorterDuff::Mode::NOOP;
    mButtonTintList = nullptr;
#if defined(FUNCTION_AS_CHECKABLE)&&FUNCTION_AS_CHECKABLE
    isChecked = [this]()->bool{
        return mChecked;
    };
    toggle = [this](){
        doSetChecked(!mChecked);
    };
    setChecked = [this](bool checked){
        doSetChecked(checked);
    };
#endif
}

#if !(defined(FUNCTION_AS_CHECKABLE)&&FUNCTION_AS_CHECKABLE)
void CompoundButton::setChecked(bool checked){
    doSetChecked(checked);
}

bool CompoundButton::isChecked()const{
    return mChecked;
}

void CompoundButton::toggle(){
    doSetChecked(!mChecked); 
}
#endif

CompoundButton::~CompoundButton(){
    delete mButtonDrawable;
    //delete mButtonTintList;tintlist cant be deleted
}

std::string CompoundButton::getButtonStateDescription() {
    if (isChecked()) {
        return mContext->getString("cdroid:string/checked");
    } else {
        return mContext->getString("cdroid:string/not_checked");
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
        mergeDrawableStates(drawableState,StateSet::get(StateSet::VIEW_STATE_CHECKED));
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
    doSetChecked(!mChecked);//toggle();
    const bool handled = Button::performClick();
    if (!handled) {
        // View only makes a sound effect if the onClickListener was
        // called, so we'll need to make one here instead.
        playSoundEffect(SoundEffectConstants::CLICK);
    }
    return handled;
}

void CompoundButton::setButtonDrawable(const std::string&resid){
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

void CompoundButton::setButtonTintList(const ColorStateList* tint) {
    if(mButtonTintList!=tint){
        mButtonTintList = tint;
        applyButtonTint();
    }
}

/**
 * @return the tint applied to the button drawable
 * @attr ref android.R.styleable#CompoundButton_buttonTint
 * @see #setButtonTintList(const ColorStateList)
 */
const ColorStateList* CompoundButton::getButtonTintList() const{
    return mButtonTintList;
}

void CompoundButton::setButtonTintMode(PorterDuffMode tintMode){
    mButtonTintMode = tintMode;
    applyButtonTint();
}

PorterDuffMode CompoundButton::getButtonTintMode() const {
    return (PorterDuffMode)mButtonTintMode;
}

void CompoundButton::applyButtonTint() {
    if (mButtonDrawable  && (mButtonTintList || mButtonTintMode!=PorterDuff::Mode::NOOP)) {
        mButtonDrawable = mButtonDrawable->mutate();

        if (mButtonTintList) {
            mButtonDrawable->setTintList(mButtonTintList);
        }

        if (mButtonTintMode!=PorterDuff::Mode::NOOP) {
            mButtonDrawable->setTintMode(mButtonTintMode);
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

void CompoundButton::doSetChecked(bool checked){
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
DECLARE_WIDGET2(CheckBox,"cdroid:attr/checkboxStyle")
CheckBox::CheckBox(Context*ctx,const AttributeSet& attrs)
    :CompoundButton(ctx,attrs){
}

CheckBox::CheckBox(const std::string&txt,int w,int h)
    :CompoundButton(txt,w,h){
    setButtonDrawable("cdroid:drawable/btn_check.xml");
}

std::string CheckBox::getAccessibilityClassName()const{
    return "CheckBox";
}
//////////////////////////////////////////////////////////////
//class RadioButton:public CompoundButton

DECLARE_WIDGET2(RadioButton,"cdroid:attr/radioButtonStyle")
RadioButton::RadioButton(const std::string&txt,int w,int h)
  :CompoundButton(txt,w,h){
#if defined(FUNCTION_AS_CHECKABLE)&&FUNCTION_AS_CHECKABLE
    toggle = [this](){
        if(!isChecked())doSetChecked(true);
    };
#endif
}

RadioButton::RadioButton(Context*ctx,const AttributeSet& attrs)
   :CompoundButton(ctx,attrs){
#if defined(FUNCTION_AS_CHECKABLE)&&FUNCTION_AS_CHECKABLE
    toggle = [this](){
        if(!isChecked())doSetChecked(true);
    };
#endif
}

#if !(defined(FUNCTION_AS_CHECKABLE)&&FUNCTION_AS_CHECKABLE)
void RadioButton::toggle(){
    if(!isChecked())CompoundButton::toggle();
}
#endif
std::string RadioButton::getAccessibilityClassName()const{
    return "RadioButton";
}

}/*endof namespace*/

