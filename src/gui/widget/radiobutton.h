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
#ifndef __RADIO_BUTTON_H__
#define __RADIO_BUTTON_H__
#include <widget/compoundbutton.h>
namespace cdroid{
class RadioButton:public CompoundButton{
public:
    RadioButton(const std::string&,int w,int h);
    RadioButton(Context*ctx,const AttributeSet& attrs);
#if !(defined(FUNCTION_AS_CHECKABLE)&&FUNCTION_AS_CHECKABLE)
    void toggle()override;
#endif
    std::string getAccessibilityClassName()const override;
};
}/*endof namespace*/
#endif
