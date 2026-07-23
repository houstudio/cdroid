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
#ifndef __GOOGLE_PINYIN_H__
#define __GOOGLE_PINYIN_H__
#include <string>
#include <vector>
#include <gui_features.h>
#include <core/inputmethod.h>

/* Pinyin input method backed by the vendored Google Pinyin engine
 * (src/3rdparty/pinyin, treated as third-party). The whole class only exists
 * when ENABLE_PINYIN2HZ is on, so the base InputMethod / EnglishInputMethod in
 * inputmethod.h stay free of any pinyin dependency. It is a conversion method
 * (isConversionMethod()==true): typed letters are pinyin, candidates are hanzi. */
#ifdef ENABLE_PINYIN2HZ
namespace cdroid{

class GooglePinyin:public InputMethod{
protected:
   void*handle;
private:
   int fillCandidates(std::vector<std::string>&candidates,size_t num);
public:
   GooglePinyin();
   ~GooglePinyin()override;
   bool loadDicts(const std::string&sys,const std::string&user)override;
   int search(const std::string&pinyin,std::vector<std::string>&candidates)override;
   void closeSearch()override;
   int getPredicts(const std::string&history,std::vector<std::string>&predicts)override;
   bool isConversionMethod()const override{return true;}
};

}/*endof namespace*/
#endif //ENABLE_PINYIN2HZ
#endif
