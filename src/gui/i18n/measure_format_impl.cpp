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

#include "i18n_memory_adapter.h"
#include "str_util.h"
#include "measure_format_impl.h"

using namespace OHOS::I18N;

MeasureFormatImpl::MeasureFormatImpl(LocaleInfo &localeinfo, I18nStatus &status)
{
    if (localeinfo.GetId() == nullptr) {
        status = I18nStatus::IERROR;
        return;
    }
    locale = localeinfo;
    pluralFormat = new (std::nothrow) PluralFormat(localeinfo, status);
    if (status != I18nStatus::ISUCCESS || pluralFormat == nullptr) {
        status = I18nStatus::IERROR;
        return;
    }
    int numberFormatStatus = 0;
    numberFormat = new (std::nothrow) NumberFormat(localeinfo, numberFormatStatus);
    if (numberFormatStatus || numberFormat == nullptr) {
        status = I18nStatus::IERROR;
        return;
    }
}

MeasureFormatImpl::~MeasureFormatImpl()
{
    DeallocateMemory();
    if (pluralFormat != nullptr) {
        delete pluralFormat;
        pluralFormat = nullptr;
    }
    if (numberFormat != nullptr) {
        delete numberFormat;
        numberFormat = nullptr;
    }
}

std::string MeasureFormatImpl::Format(double num, std::string &unit, MeasureFormatType type, I18nStatus &status)
{
    formattedDouble = num;
    return FormatInner(false, unit, type, status);
}

std::string MeasureFormatImpl::Format(int num, std::string &unit, MeasureFormatType type, I18nStatus &status)
{
    formattedInteger = num;
    return FormatInner(true, unit, type, status);
}

std::string MeasureFormatImpl::FormatInner(bool isInteger, std::string &unit, MeasureFormatType type,
    I18nStatus &status)
{
    int unitIndex = SearchUnits(unit, status);
    if (status != I18nStatus::ISUCCESS) {
        return "";
    }
    int pluralIndex = ComputePluralIndex(isInteger, status);
    if (status != I18nStatus::ISUCCESS) {
        return "";
    }
    std::string formattedUnit = ComputeFormattedUnit(unitIndex, pluralIndex, type, status);
    if (status != I18nStatus::ISUCCESS) {
        return "";
    }
    std::string formattedNumber = ComputeFormattedNumber(isInteger, status);
    if (status != I18nStatus::ISUCCESS) {
        return "";
    }

    char result[MAX_MEASURE_FORMAT_LENGTH];
    int concatenate = 0;
    if (isNumberFirst) {
        concatenate = snprintf(result, MAX_MEASURE_FORMAT_LENGTH, pattern.c_str(), formattedNumber.c_str(),
            formattedUnit.c_str());
    } else {
        concatenate = snprintf(result, MAX_MEASURE_FORMAT_LENGTH, pattern.c_str(), formattedUnit.c_str(),
            formattedNumber.c_str());
    }
    if (concatenate < 0) {
        status = I18nStatus::IERROR;
        return "";
    }
    return result;
}

int MeasureFormatImpl::SearchUnits(std::string &target, I18nStatus &status)
{
    for (int i = 0; i < unitCount; i++) {
        if (target.compare(units[i]) == 0) {
            return i;
        }
    }
    status = I18nStatus::IERROR;
    return -1;
}

int MeasureFormatImpl::ComputePluralIndex(bool isInteger, I18nStatus &status)
{
    if (isInteger) {
        return pluralFormat->GetPluralRuleIndex(formattedInteger, status);
    } else {
        return pluralFormat->GetPluralRuleIndex(formattedDouble, status);
    }
}

std::string MeasureFormatImpl::ComputeFormattedUnit(int unitIndex, int pluralIndex, MeasureFormatType type,
    I18nStatus &status)
{
    int index = unitIndex * MEASURE_FORMAT_TYPE_NUM + (int)type;
    std::string formattedUnit = formattedUnits[index][pluralIndex];
    if (formattedUnit.length() != 0) {
        return formattedUnit;
    }
    formattedUnit = formattedUnits[index][MEASURE_PLURAL_NUM - 1];
    if (formattedUnit.length() != 0) {
        return formattedUnit;
    }
    type = ConvertType(type, status);
    if (status != I18nStatus::ISUCCESS) {
        return "";
    }
    formattedUnit = ComputeFormattedUnit(unitIndex, pluralIndex, type, status);
    if (status != I18nStatus::ISUCCESS) {
        return "";
    }
    return formattedUnit;
}

MeasureFormatType MeasureFormatImpl::ConvertType(MeasureFormatType type, I18nStatus &status)
{
    if (type == MeasureFormatType::MEASURE_SHORT) {
        return MeasureFormatType::MEASURE_MEDIUM;
    } else if (type == MeasureFormatType::MEASURE_FULL) {
        return MeasureFormatType::MEASURE_LONG;
    } else if (type == MeasureFormatType::MEASURE_LONG) {
        return MeasureFormatType::MEASURE_MEDIUM;
    } else {
        status = I18nStatus::IERROR;
        return MeasureFormatType::MEASURE_MEDIUM;
    }
}

std::string MeasureFormatImpl::ComputeFormattedNumber(bool isInteger, I18nStatus &status)
{
    std::string formattedNumber;
    int numberFormatStatus = 0;
    if (isInteger) {
        formattedNumber = numberFormat->Format(formattedInteger, NumberFormatType::DECIMAL, numberFormatStatus);
    } else {
        formattedNumber = numberFormat->Format(formattedDouble, NumberFormatType::DECIMAL, numberFormatStatus);
    }
    if (numberFormatStatus) {
        status = I18nStatus::IERROR;
        return "";
    }
    return formattedNumber;
}

bool MeasureFormatImpl::Init(const DataResource &resource)
{
    if (units != nullptr || formattedUnits != nullptr) {
        DeallocateMemory();
    }
    std::string unprocessedMeasureData = resource.GetString(DataResourceType::MEASURE_FORMAT_PATTERNS);
    bool status = InitMeasureFormat(unprocessedMeasureData);
    if (!status) {
        return false;
    }
    return true;
}

bool MeasureFormatImpl::InitMeasureFormat(std::string &unprocessedMeasureData)
{
    int end = 0;
    while (end < unprocessedMeasureData.length() && unprocessedMeasureData[end] != PLURAL_SEP) {
        end++;
    }
    unitCount = std::stoi(std::string(unprocessedMeasureData, 0, end));
    int itemCount = MEASURE_BASE_ITEM_COUNT + unitCount * MEASURE_SINGLE_UNIT_COUNT;
    std::string *items = new std::string[itemCount];
    if (items == nullptr) {
        return false;
    }
    Split(unprocessedMeasureData, items, itemCount, PLURAL_SEP);
    // items[1] is unit list, such as h|min|kcal|time...
    if (!ParseUnits(items[1])) {
        delete[] items;
        return false;
    }
    // items[2] is pattern
    pattern = items[2];
    // items[3] represent number and unit which comes first.
    if (strcmp(items[3].c_str(), "#") == 0) {
        isNumberFirst = true;
    } else {
        isNumberFirst = false;
    }
    formattedUnitCount = unitCount * MEASURE_FORMAT_TYPE_NUM;
    if (!AllocaFormattedUnits()) {
        delete[] items;
        return false;
    }
    int itemStartIndex = MEASURE_BASE_ITEM_COUNT;
    for (int i = 0; i < unitCount; i++) {
        for (int j = 0; j < MEASURE_FORMAT_TYPE_NUM; j++) {
            FillFormattedUnits(i, j, items, itemStartIndex);
            itemStartIndex += MEASURE_PLURAL_NUM;
        }
    }
    if (items != nullptr) {
        delete[] items;
        items = nullptr;
    }
    return true;
}

bool MeasureFormatImpl::ParseUnits(std::string &unitsList)
{
    units = new std::string[unitCount];
    if (units == nullptr) {
        return false;
    }
    int begin = 0;
    int end = 0;
    int unitsLength = unitsList.length();
    int count = 0;
    while (count < unitCount) {
        while (end < unitsLength && unitsList[end] != MEASURE_UNIT_SEP) {
            end++;
        }
        if (end >= unitsLength) {
            break;
        }
        units[count] = std::string(unitsList, begin, end - begin);
        count++;
        end++;
        begin = end;
    }
    if (count < unitCount && end > begin) {
        units[count] = std::string(unitsList, begin, end - begin);
    }
    if ((count + 1 != unitCount) || end < unitsLength) {
        return false;
    }
    return true;
}

bool MeasureFormatImpl::AllocaFormattedUnits()
{
    formattedUnits = new std::string *[formattedUnitCount];
    if (formattedUnits == nullptr) {
        return false;
    }
    for (int i = 0; i < formattedUnitCount; i++) {
        formattedUnits[i] = new std::string[MEASURE_PLURAL_NUM];
        if (formattedUnits[i] == nullptr) {
            return false;
        }
    }
    return true;
}

void MeasureFormatImpl::FillFormattedUnits(int unitIndex, int typeIndex, std::string *items, int itemStartIndex)
{
    for (int i = 0; i < MEASURE_PLURAL_NUM; i++) {
        int index = unitIndex * MEASURE_FORMAT_TYPE_NUM + typeIndex;
        formattedUnits[index][i] = items[itemStartIndex + i];
    }
}

void MeasureFormatImpl::DeallocateMemory()
{
    if (units != nullptr) {
        delete[] units;
        units = nullptr;
    }
    if (formattedUnits != nullptr) {
        for (int i = 0; i < formattedUnitCount; i++) {
            if (formattedUnits[i] != nullptr) {
                delete[] formattedUnits[i];
            }
        }
        delete[] formattedUnits;
    }
}
