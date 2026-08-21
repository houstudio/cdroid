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
#include <widget/togglebutton.h>
#include <widget/framework_styleable.h>
#include <core/assets.h>
#include <cdlog.h>

namespace cdroid{
using namespace cdroid::internal;
#define NO_ALPHA 0xFF

DECLARE_WIDGET2(ToggleButton,R::attr::buttonStyleToggle)

ToggleButton::ToggleButton(Context*ctx)
    :ToggleButton(ctx,nullptr){}

ToggleButton::ToggleButton(Context*ctx,const AttributeSet* attrs):ToggleButton(ctx,attrs,cdroid::internal::R::attr::buttonStyleToggle){}

ToggleButton::ToggleButton(Context*ctx,const AttributeSet* pAttrs,int defStyleAttr)
  :CompoundButton(ctx,pAttrs, defStyleAttr){
    mIndicatorDrawable=nullptr;
    // Phase 2: TypedArray (binary AXML typed resolution). ta=null → text XML fallback.
    auto ta = ctx->obtainStyledAttributes(pAttrs, R::styleable::ToggleButton, defStyleAttr);
    

    setTextOn(ta->getString(R::styleable::ToggleButton_textOn));
    setTextOff(ta->getString(R::styleable::ToggleButton_textOff));
    mDisabledAlpha= ta->getFloat(R::styleable::ToggleButton_disabledAlpha,0.5f);

}

void ToggleButton::setChecked(bool checked){
    CompoundButton::setChecked(checked);
    syncTextState();
}

const std::string ToggleButton::getTextOn()const{
    return mTextOn;
}

void ToggleButton::setTextOn(const std::string& textOn){
    mTextOn = textOn;
    syncTextState();
}

const std::string ToggleButton::getTextOff()const{
    return mTextOff;
}

void ToggleButton::setTextOff(const std::string& textOff){
    mTextOff=textOff;
    syncTextState();
}

float ToggleButton::getDisabledAlpha() const{
    return mDisabledAlpha;
}

void ToggleButton::syncTextState(){
    const bool checked = isChecked();
    if (checked && !mTextOn.empty()) {
        setText(mTextOn);
    } else if (!checked && !mTextOff.empty()) {
        setText(mTextOff);
    }
}

void ToggleButton::drawableStateChanged() {
    CompoundButton::drawableStateChanged();
    if (mIndicatorDrawable != nullptr) {
        mIndicatorDrawable->setAlpha(isEnabled() ? NO_ALPHA : (int) (NO_ALPHA * mDisabledAlpha));
    }
}

void ToggleButton::updateReferenceToIndicatorDrawable(Drawable* backgroundDrawable) {
    if (dynamic_cast<LayerDrawable*>(backgroundDrawable)) {
        LayerDrawable* layerDrawable = (LayerDrawable*) backgroundDrawable;
        mIndicatorDrawable =layerDrawable->findDrawableByLayerId(R::id::toggle);//com.android.internal.R.id.toggle);
    } else {
        mIndicatorDrawable = nullptr;
    }
}

void ToggleButton::setBackground(Drawable* d){
    CompoundButton::setBackground(d);
    updateReferenceToIndicatorDrawable(d);
}

std::string ToggleButton::getAccessibilityName()const{
    return "ToggleButton";
}

std::string ToggleButton::getButtonStateDescription() {
    if (isChecked()) {
        return mTextOn.empty() ? mContext->getString(R::string::capital_on) : mTextOn;
    } else {
        return mTextOff.empty() ? mContext->getString(R::string::capital_off) : mTextOff;
    }
}
}

