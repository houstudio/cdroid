/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef STR_UTIL_H
#define STR_UTIL_H
#include <string>
#include <cstring>
#include "types.h"

#define I18N_STRING_LENGTH_MAX 512

namespace cdroid {
namespace i18n {
int ReplaceAndCountOff(std::string &content, const int index, const char *sign, const int off);

void ArrayCopy(std::string *target, const int targetSize, const std::string *source, const int sourceSize);

char *NewArrayAndCopy(const char *source, const int len);

char *I18nNewCharString(const char *source, const int len);

bool CleanCharArray(char *target, const int len);

int LenCharArray(const char *target);

void Split(const std::string &src, std::string *dst, const int32_t size, const char &sep);

bool CompareLocaleItem(const char *src, const char *dst);

std::string Parse(const char *str, int32_t count);
} // namespace i18n
} // namespace cdroid
#endif
