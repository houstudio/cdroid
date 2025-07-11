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
#include <widget/button.h>

namespace cdroid{

DECLARE_WIDGET2(Button,"cdroid:attr/buttonStyle")

Button::Button(Context*ctx,const AttributeSet& attrs):TextView(ctx,attrs){
}

Button::Button(int32_t w, int32_t h):Button(std::string(),w,h){
}

Button::Button(const std::string& text, int32_t w, int32_t h)
  :TextView(text, w, h){
    setGravity(Gravity::CENTER);
}

Button::~Button() {
}

PointerIcon* Button::onResolvePointerIcon(MotionEvent& event, int pointerIndex) {
    if ((getPointerIcon() == nullptr) && isClickable() && isEnabled()) {
        return PointerIcon::getSystemIcon(getContext(), PointerIcon::TYPE_HAND);
    }
    return TextView::onResolvePointerIcon(event, pointerIndex);
}

std::string Button::getAccessibilityClassName()const{
    return "Button";
}
}//endof namespace

