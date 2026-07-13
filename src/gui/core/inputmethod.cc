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

namespace cdroid{

InputMethod::InputMethod(){
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

/* Base InputMethod does no candidate search and no in-composition selection:
 * search() returns <0 (the typed char is committed directly) and choose/
 * cancelLastChoice are unsupported. Concrete engines live in their own files
 * (googlepinyin.{h,cc}, englishinputmethod.{h,cc}). */
int InputMethod::choose(size_t,std::vector<std::string>&){
   return -1;
}

int InputMethod::cancelLastChoice(std::vector<std::string>&){
   return -1;
}

}/*endof namespace*/
