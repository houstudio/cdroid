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
#include <core/inputmethod.h>
#include <text/inputtype.h>
#include <widget/internal_R.h>

namespace cdroid{

InputMethod::InputMethod(){
}

int InputMethod::getKeyboardLayout(int inputType)const{
    // System-default keyboard set. The bundled English/Pinyin methods inherit
    // this unchanged (they do not override), so they return the system layouts
    // explicitly rather than relying on the IME's magic fallback. A product
    // subclass overrides to ship its own keyboards. POPUP -> 0 so the
    // KeyboardView's android:popupLayout (keyboard_popup_keyboard.xml) supplies
    // the accent popup container. The number/phone/datetime classes ship no
    // framework keyboard resource (0 keeps the empty keyboard, exactly like the
    // old unresolvable "@cdroid:xml/keyboard_*.xml" string refs did).
    if(inputType == POPUP) return 0;
    switch(inputType & InputType::TYPE_MASK_CLASS){
    case InputType::TYPE_CLASS_NUMBER:
    case InputType::TYPE_CLASS_PHONE:
    case InputType::TYPE_CLASS_DATETIME:
        return 0;
    default: return cdroid::internal::R::xml::qwerty; // TYPE_CLASS_TEXT
    }
}

int InputMethod::getIMEWindowLayout()const{
    // 0 = the built-in ime_pinyin_keyboard container (candidate strip on top,
    // full-width KeyboardView below). Bundled methods keep it; see the header
    // for the product-override contract (keyboardview id required, the rest
    // optional).
    return 0;
}

InputMethod::~InputMethod(){
}

bool InputMethod::loadDicts(const std::string&sys,const std::string&user){
    sysdict = sys;
    userdict= user;
    return true;
}

const std::string& InputMethod::getSysDict()const{
    return sysdict;
}

const std::string& InputMethod::getUserDict()const{
    return userdict;
}

int InputMethod::search(const std::string&pinyin,std::vector<std::string>&candidates){
    return -1;
}

void InputMethod::closeSearch(){
}

int InputMethod::getPredicts(const std::string&history,std::vector<std::string>&predicts){
    return -1;
}

}/*endof namespace*/
