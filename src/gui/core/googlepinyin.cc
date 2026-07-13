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
#include <core/googlepinyin.h>
#ifdef ENABLE_PINYIN2HZ
#include <utf16char.h>
#include <utils/textutils.h>
#include <porting/cdtypes.h>
#include <porting/cdlog.h>
#include <pinyinime.h>
using namespace ime_pinyin;

namespace cdroid{

GooglePinyin::GooglePinyin()
 :InputMethod(),handle(nullptr){
}

GooglePinyin::~GooglePinyin(){
    if(handle)
        im_close_decoder(handle);
}

bool GooglePinyin::loadDicts(const std::string&sys,const std::string&user){
    sysdict = sys;
    userdict= user;
    handle  = im_open_decoder(sysdict.c_str(),userdict.c_str());
    LOGD("%p's dict=%s,%s",handle,sys.c_str(),user.c_str());
    return handle!=nullptr;
}

/*pinyin to chinese words*/
int GooglePinyin::search(const std::string&pinyin,std::vector<std::string>&candidates){
    const size_t num = im_search(handle,pinyin.c_str(),pinyin.length());
    size_t spslen=0;
    const char* sps = im_get_sps_str(handle,&spslen);
    LOGD("[py] search in='%s' -> cands=%d sps='%.*s' fixed=%zu",
         pinyin.c_str(),(int)num,(int)spslen,sps,im_get_fixed_len(handle));
    return fillCandidates(candidates,num);
}

/* Fetch `num` candidates from the current search/choose workspace into the
 * vector (shared by search/choose/cancelLastChoice). */
int GooglePinyin::fillCandidates(std::vector<std::string>&candidates,size_t num){
    char16 canbuf[64];
    for(size_t i = 0; i < num ; i++){/*pinyin to HANZI*/
        char16*scan = im_get_candidate(handle,i,canbuf,32);
        std::string u8s = TextUtils::utf16_utf8(scan,utf16_strlen(scan));
        if(u8s.size())candidates.push_back(u8s);
    }
    return num;
}

void GooglePinyin::closeSearch(){
    im_reset_search(handle);
}

//predictive input
int GooglePinyin::getPredicts(const std::string&history,std::vector<std::string>&predicts){
    char16 (*predict_buf)[kMaxPredictSize + 1];
    const std::u16string u16txt=TextUtils::utf8_utf16(history);
    const int num = im_get_predicts(handle,(const char16*)u16txt.c_str(),predict_buf);
    LOGV("kMaxPredictSize=%d num=%d",kMaxPredictSize,num);
    for(int i = 0;i< num ;i++){
        std::string u8s = TextUtils::utf16_utf8(predict_buf[i],utf16_strlen(predict_buf[i]));
        if(u8s.size())predicts.push_back(u8s);
    }
    return num;
}

}/*endof namespace*/
#endif //ENABLE_PINYIN2HZ
