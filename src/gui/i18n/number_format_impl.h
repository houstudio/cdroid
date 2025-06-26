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

#ifndef NUMBERFORMAT_IMPL_H
#define NUMBERFORMAT_IMPL_H

#include "types.h"
#include "securec.h"
#include "locale_info.h"
#include "data_resource.h"
#include "number_data.h"

namespace OHOS {
namespace I18N {
class NumberFormatImpl {
public:
    NumberFormatImpl(LocaleInfo &locale, int &status);
    virtual ~NumberFormatImpl();
    std::string Format(double num, NumberFormatType type, int &status) const;
    std::string Format(int num, int &status) const;
    std::string FormatNoGroup(double num, NumberFormatType type, int &status) const;
    std::string FormatNoGroup(int num, int &status) const;
    bool Init(const DataResource &resource);
    bool SetMaxDecimalLength(int length);
    bool SetMinDecimalLength(int length);
private:
    NumberData *defaultData = nullptr;
    LocaleInfo mLocale;
    int maxDecimalLength = -1;
    int minDecimalLength = -1;
    void CheckStatus(int rc, int &status) const;
    int DelMoreZero(const StyleData &style, int decLen, int lastLen, bool hasDec, char *&result) const;
    std::string InnerFormat(double num, bool hasDec, bool isShowGroup, bool isPercent, int &status) const;
    std::string ConvertSignAndNum(const char *content, int len, NumberData *data, StyleData &style) const;
    int ConvertNum(std::string &strContent, char currentChar, const NumberData *data, int index, int off) const;
    int DelZero(char *target, int len, int delNum, bool onlyZero) const;
    void AddGroup(char *targetAndSource[], const int len[], const char *decimal, bool hasDec, int decLen) const;
    int CountGroupNum(int intLength, bool isTwoGrouped) const;
    char *FillMinDecimal(const char *target, int len, int addSize, bool isDec) const;
    bool DealWithPercent(char *buff, char *&result, int &status, StyleData &style, int &lastLen) const;
    static constexpr int NUMBER_MAX = 50;
    static constexpr int NO_SET = -1;
};
} // namespace I18N
} // namespace OHOS
#endif
