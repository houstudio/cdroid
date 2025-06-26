/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef MEASUREFORMAT_IMPL_H
#define MEASUREFORMAT_IMPL_H

#include <string>
#include "data_resource.h"
#include "locale_info.h"
#include "number_format.h"
#include "plural_format.h"
#include "securec.h"
#include "types.h"

namespace OHOS {
namespace I18N {
class MeasureFormatImpl {
public:
    MeasureFormatImpl(LocaleInfo &localeinfo, I18nStatus &status);
    virtual ~MeasureFormatImpl();
    std::string Format(double num, std::string &unit, MeasureFormatType type, I18nStatus &status);
    std::string Format(int num, std::string &unit, MeasureFormatType type, I18nStatus &status);
    bool Init(const DataResource &resource);
private:
    std::string FormatInner(bool isInteger, std::string &unit, MeasureFormatType type, I18nStatus &status);
    int SearchUnits(std::string &target, I18nStatus &status);
    int ComputePluralIndex(bool isInteger, I18nStatus &status);
    std::string ComputeFormattedUnit(int unitIndex, int pluralIndex, MeasureFormatType type, I18nStatus &status);
    std::string ComputeFormattedNumber(bool isInteger, I18nStatus &status);
    MeasureFormatType ConvertType(MeasureFormatType type, I18nStatus &status);
    bool InitMeasureFormat(std::string &unprocessedMeasureData);
    bool ParseUnits(std::string &unitsList);
    bool AllocaFormattedUnits();
    void FillFormattedUnits(int unitIndex, int typeIndex, std::string *items, int itemStartIndex);
    void DeallocateMemory();
    LocaleInfo locale;
    PluralFormat *pluralFormat;
    NumberFormat *numberFormat;
    // # represets %d %s
    std::string pattern;
    bool isNumberFirst = true;
    // units array, size is unitCount
    int unitCount = 0;
    std::string *units = nullptr;
    // formatted units array, size is (unitCount * MEASURE_FORMAT_TYPE_NUM) * MEASURE_PLURAL_NUM
    int formattedUnitCount = 0;
    std::string **formattedUnits = nullptr;
    // number to be formatted
    int formattedInteger = 0;
    double formattedDouble = 0;
};
} // namespace I18N
} // namespace OHOS
#endif
