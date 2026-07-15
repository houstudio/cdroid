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

namespace cdroid{

InputMethod::InputMethod(){
}

std::string InputMethod::getKeyboardLayout(int inputType)const{
    // System-default keyboard set. The bundled English/Pinyin methods inherit
    // this unchanged (they do not override), so they return the system layouts
    // explicitly rather than relying on the IME's magic fallback. A product
    // subclass overrides to ship its own keyboards. POPUP -> empty so the
    // KeyboardView's android:popupLayout (keyboard_popup_keyboard.xml) supplies
    // the accent popup container.
    if(inputType == POPUP) return {};
    switch(inputType & InputType::TYPE_MASK_CLASS){
    case InputType::TYPE_CLASS_NUMBER:  return "@cdroid:xml/keyboard_number.xml";
    case InputType::TYPE_CLASS_PHONE:   return "@cdroid:xml/keyboard_phone.xml";
    case InputType::TYPE_CLASS_DATETIME:return "@cdroid:xml/keyboard_datetime.xml";
    default: return "@cdroid:xml/qwerty.xml"; // TYPE_CLASS_TEXT
    }
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
