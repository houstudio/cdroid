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
#ifndef __TOGGLE_BUTTON_H__
#define __TOGGLE_BUTTON_H__
#include <widget/compoundbutton.h>
namespace cdroid{
class ToggleButton:public CompoundButton{
private:
    float mDisabledAlpha;
    std::string mTextOn;
    std::string mTextOff;
    Drawable*mIndicatorDrawable;
    void syncTextState();
    void updateReferenceToIndicatorDrawable(Drawable* backgroundDrawable);
protected:
    void doSetChecked(bool checked)override;
    void drawableStateChanged()override;
public:
    ToggleButton(int w,int h);
    ToggleButton(Context*ctx,const AttributeSet& attrs);
    const std::string getTextOn()const;
    void setTextOn(const std::string& textOn);
    const std::string getTextOff()const;
    void setTextOff(const std::string& textOff);
    float getDisabledAlpha() const;
    void setBackground(Drawable* d)override;
    std::string getAccessibilityName()const;
    std::string getButtonStateDescription()override;
};
}
#endif
