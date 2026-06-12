/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef __ANDROID_BIDI_H__
#define __ANDROID_BIDI_H__
#include <cstdint>
#include <vector>
#include <text/layout.h>
namespace cdroid{
class AndroidBidi {
public:
    static int bidi(int dir, const std::vector<char16_t>& chs, std::vector<uint8_t>& chInfo);

    static const Directions* directions(int dir,const std::vector<uint8_t>& levels, int lstart,
            const std::vector<char16_t>& chars, int cstart, int len);
};
}/*end namespace*/
#endif/*__ANDROID_BIDI_H__*/
