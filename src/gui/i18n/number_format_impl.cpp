/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "number_format_impl.h"
#include "i18n_memory_adapter.h"
#include "number_data.h"
#include "str_util.h"

using namespace cdroid::i18n;

std::string NumberFormatImpl::ConvertSignAndNum(const char *content, int len, NumberData *data, StyleData &style) const
{
    std::string strContent = content;
    int off = 0;
    for (int i = 0; i < len; i++) {
        switch (content[i]) {
            case NumberData::NUMBER_DECIMAL:
                off = ReplaceAndCountOff(strContent, i + off, data->decimal, off);
                break;
            case NumberData::NUMBER_GROUPSIGN:
                off = ReplaceAndCountOff(strContent, i + off, data->group, off);
                break;
            case NumberData::NUMBER_PERCENT:
                off = ReplaceAndCountOff(strContent, i + off, data->percent, off);
                break;
            default:
                break;
        }
        if (defaultData->isNative) {
            off = ConvertNum(strContent, content[i], data, i, off);
        }
    }
    return strContent;
}

int NumberFormatImpl::ConvertNum(std::string &strContent, char currentChar,
    const NumberData *data, int index, int off) const
{
    std::string numStr = "0123456789";
    size_t charPos = numStr.find(currentChar);
    return (charPos != std::string::npos) ?
        ReplaceAndCountOff(strContent, index + off, data->nativeNums[charPos].c_str(), off) : off;
}

NumberFormatImpl::NumberFormatImpl(LocaleInfo &locale, int &status)
{
    if (locale.GetId() == nullptr) {
        status = IERROR;
        return;
    }
    mLocale = locale;
}

bool NumberFormatImpl::Init(const DataResource &resource)
{
    std::string unprocessedNumberFormat;
    resource.GetString(DataResourceType::NUMBER_FORMAT, unprocessedNumberFormat);
    std::string unprocessedNumberDigit;
    resource.GetString(DataResourceType::NUMBER_DIGIT, unprocessedNumberDigit);
    std::string split[NUM_PATTERN_SIZE];
    Split(unprocessedNumberFormat, split, NUM_PATTERN_SIZE, NUM_PATTERN_SEP);
    std::string decSign = split[NUM_DEC_SIGN_INDEX];
    std::string groupSign = split[NUM_GROUP_SIGN_INDEX];
    std::string perSign = split[NUM_PERCENT_SIGN_INDEX];
    const char *numberDigits = mLocale.GetExtension("nu");
    if (numberDigits != nullptr) {
        std::string numebrSystemFormat;
        NumberData::GetNumberingSystem(numberDigits, numebrSystemFormat, unprocessedNumberDigit);
        std::string temp[NUM_PATTERN_SIZE];
        Split(numebrSystemFormat, temp, NUM_PATTERN_SIZE, NUM_PATTERN_SEP);
        decSign = temp[NUM_DEC_SIGN_INDEX];
        groupSign = temp[NUM_GROUP_SIGN_INDEX];
        perSign = temp[NUM_PERCENT_SIGN_INDEX];
    }
    std::string origin = split[NUM_PERCENT_PAT_INDEX];
    const char *pat = split[NUM_PAT_INDEX].c_str();
    int size = origin.size();
    std::string adjust = origin;
    // strip "0x80 0xe2 0x8f" these three bytes in pat
    if (size >= 3 && // check the last 3 character
        (static_cast<unsigned char>(origin.at(size - 1)) == 0x8f) && // check whether the last one is 0x8f
        (static_cast<unsigned char>(origin.at(size - 2)) == 0x80) && // check whether the index of size - 2 is 0x80
        (static_cast<unsigned char>(origin.at(size - 3)) == 0xe2)) { // check whether the index of size - 3 is 0xe2
        adjust = std::string(origin, 0, size - 3); // strip the last 3 chars
    }
    const char *percentPat = adjust.c_str();
    defaultData = new (std::nothrow) NumberData(pat, percentPat, decSign, groupSign, perSign);
    if (defaultData == nullptr) {
        return false;
    }
    if (unprocessedNumberDigit != "") {
        std::string splitDigit[NUM_DIGIT_SIZE];
        Split(unprocessedNumberDigit, splitDigit, NUM_DIGIT_SIZE, NUM_DIGIT_SEP);
        defaultData->SetNumSystem(splitDigit, NUM_DIGIT_SIZE);
    }
    // set minus sign
    std::string minus;
    resource.GetString(DataResourceType::MINUS_SIGN, minus);
    defaultData->SetMinusSign(minus);
    return true;
}


NumberFormatImpl::~NumberFormatImpl()
{
    if (defaultData != nullptr) {
        delete defaultData;
        defaultData = nullptr;
    }
}

std::string NumberFormatImpl::InnerFormat(double num, bool hasDec, bool isShowGroup, bool isPercent,
    int &status) const
{
    if (defaultData == nullptr) {
        return "";
    }
    char buff[NUMBER_MAX] = { 0 };
    bool isPercentDefault = isPercent && (defaultData->style.minDecimalLength < 0);
    double adjustNum = (num < 0) ? (-1 * num) : num;
    int len = isPercentDefault ? static_cast<int>(snprintf(buff, NUMBER_MAX, "%.f", adjustNum)) :
        static_cast<int>(snprintf(buff, NUMBER_MAX, defaultData->style.numFormat, adjustNum));
    // convert decimal to char and format
    if (len < 0) {
        status = IERROR;
        return "";
    }
    char *decimalNum = strchr(buff, NumberData::NUMBER_DECIMAL);
    int decLen = (decimalNum == nullptr) ? 0 : strlen(decimalNum);
    int lastLen = isShowGroup ? (len + CountGroupNum(len - decLen, defaultData->style.isTwoGroup)) : len;
    char *result = reinterpret_cast<char *>(I18nMalloc(lastLen + 1));
    if (result == nullptr) {
        status = IERROR;
        return "";
    }
    result[lastLen] = '\0';
    bool adjustHasDec = isPercentDefault ? false : hasDec;
    if (isShowGroup) {
        char *resultAndContent[] = { result, buff };
        int lengths[] = { lastLen, len, defaultData->style.isTwoGroup };
        AddGroup(resultAndContent, lengths, decimalNum, adjustHasDec, decLen);
    } else {
        strncpy(result,buff, lastLen + 1);
        /*if (rc != EOK) {
            I18nFree(static_cast<void *>(result));
            return "";
        }*/
    }
    // del more zero
    lastLen = DelMoreZero(defaultData->style, decLen, lastLen, adjustHasDec, result);
    // if percent
    if (isPercent && !DealWithPercent(buff, result, status, defaultData->style, lastLen)) {
        I18nFree(static_cast<void *>(result));
        return "";
    }
    // if have native number to convert
    std::string outStr = ConvertSignAndNum(result, lastLen, defaultData, defaultData->style);
    I18nFree(static_cast<void *>(result));
    if (num < 0) {
        outStr.insert(0, defaultData->GetMinusSign());
    }
    return outStr;
}

bool NumberFormatImpl::DealWithPercent(char *buff, char *&result, int &status, StyleData &style, int &lastLen) const
{
    if (style.entireFormat != nullptr) {
        bool cleanRet = CleanCharArray(buff, NUMBER_MAX);
        if (!cleanRet) {
            return false;
        }
        int len = static_cast<int>(snprintf(buff, NUMBER_MAX, style.entireFormat, result));
        if (len < 0) {
            status = IERROR;
            I18nFree(static_cast<void *>(result));
            return false;
        }
        char *perResult = reinterpret_cast<char *>(I18nMalloc(len + 1));
        if (perResult == nullptr) {
            return false;
        }
        strncpy(perResult,buff, len + 1);
        /*if (rc != EOK) {
            I18nFree(static_cast<void *>(perResult));
            return false;
        }*/
        perResult[len] = '\0';
        lastLen = len;
        I18nFree(static_cast<void *>(result));
        result = perResult;
        perResult = nullptr;
    }
    return true;
}


int NumberFormatImpl::DelMoreZero(const StyleData &style, int decLen, int lastLen, bool hasDec, char *&result) const
{
    int num = 0;
    if (decLen > 1) {
        int delNum = decLen - 1;
        num = DelZero(result, lastLen, delNum, true);
    }
    // delete more char
    if ((maxDecimalLength != NO_SET) && (maxDecimalLength < decLen - 1 - num)) {
        int delNum = decLen - 1 - num - maxDecimalLength;
        num = num + DelZero(result, lastLen - num, delNum, false);
    }
    // fill zero to min
    if (hasDec && (minDecimalLength != NO_SET) && (minDecimalLength > decLen - 1 - num)) {
        if (decLen - 1 - num < 0) {
            int add = minDecimalLength + 1;
            char *tempResult = FillMinDecimal(result, lastLen - num, add, false);
            if (result != nullptr) {
#ifdef I18N_PRODUCT
                (void)OhosFree(static_cast<void *>(result));
#else
                (void)free(result);
#endif
            }
            result = tempResult;
            num = num - add;
        } else {
            int add = minDecimalLength - decLen + num + 1;
            char *tempResult = FillMinDecimal(result, lastLen - num, add, true);
            if (result != nullptr) {
#ifdef I18N_PRODUCT
                (void)OhosFree(static_cast<void *>(result));
#else
                (void)free(result);
#endif
            }
            result = tempResult;
            num = num - add;
        }
    }
    return lastLen - num;
}

void NumberFormatImpl::CheckStatus(int rc, int &status) const
{
    if (rc != 0/*EOK*/) {
        status = IERROR;
    }
}

int NumberFormatImpl::CountGroupNum(int intLength, bool isTwoGrouped) const
{
    int groupNum;
    if (!isTwoGrouped) {
        groupNum = static_cast<int>(intLength / NumberData::NUMBER_GROUP);
        if ((intLength % NumberData::NUMBER_GROUP) == 0) {
            --groupNum;
        }
    } else {
        if (intLength <= NumberData::NUMBER_GROUP) {
            return 0;
        }
        intLength -= NumberData::NUMBER_GROUP;
        groupNum = 1 + static_cast<int>(intLength / NumberData::TWO_GROUP);
        if ((intLength % NumberData::TWO_GROUP) == 0) {
            --groupNum;
        }
    }
    return groupNum;
}

void NumberFormatImpl::AddGroup(char *targetAndSource[], const int len[], const char *decimal,
    bool hasDec, int decLen) const
{
    // The len array must have at least 3 elements and the targetAndSource array
    // must have at least 2 elements.
    if ((targetAndSource == nullptr) || (len == nullptr)) {
        return;
    }
    char *target = targetAndSource[0]; // use array to store target and source string, first is target string
    int targetLen = len[0]; // use array to store target length and source length, first is target length
    char *source = targetAndSource[1]; // use array to store target and source string, second is source string
    int sourceLen = len[1]; // use array to store target length and source length, second is source length
    int isTwoGroup = len[2]; // 2 is the index of group info
    int intLen = sourceLen - decLen;
    int addIndex = 0;
    for (int i = 0; (i < intLen) && (addIndex < targetLen); i++, addIndex++) {
        int index = intLen - i;
        // ADD GROUP SIGN
        if (isTwoGroup == 0) {
            if ((index % NumberData::NUMBER_GROUP == 0) && (i != 0)) {
                target[addIndex] = ',';
                addIndex++;
            }
            target[addIndex] = source[i];
        } else {
            if ((index == NumberData::NUMBER_GROUP) && (i != 0)) {
                target[addIndex] = ',';
                addIndex++;
                target[addIndex] = source[i];
            } else if ((index > NumberData::NUMBER_GROUP) &&
                ((index - NumberData::NUMBER_GROUP) % NumberData::TWO_GROUP == 0) && (i != 0)) {
                target[addIndex] = ',';
                addIndex++;
                target[addIndex] = source[i];
            } else {
                target[addIndex] = source[i];
            }
        }
    }
    if (decLen > 0) {
        target[addIndex] = hasDec ? '.' : '\0';
        for (int j = 1; (j < decLen) && (addIndex < targetLen); j++) {
            target[addIndex + j] = hasDec ? decimal[j] : '\0';
        }
    }
}

int NumberFormatImpl::DelZero(char *target, int len, int delNum, bool onlyZero) const
{
    int num = 0;
    for (int i = len - 1; (i > len - delNum - 1) && (i >= 0); i--) {
        if ((target[i] != '0') && onlyZero) {
            break;
        }
        target[i] = '\0';
        num++;
        if ((i - 1 > 0) && (target[i - 1] == '.')) {
            target[i - 1] = '\0';
            num++;
            break;
        }
    }
    return num;
}

std::string NumberFormatImpl::Format(double num, NumberFormatType type, int &status) const
{
    if (defaultData == nullptr) {
        status = IERROR;
        return "";
    }
    if (type == PERCENT) { // percent,the decimal needs to be multiplied by 100.
        return InnerFormat(num * 100, true, true, true, status);
    } else {
        return InnerFormat(num, true, true, false, status);
    }
}

std::string NumberFormatImpl::Format(int num, int &status) const
{
    if (defaultData == nullptr) {
        status = IERROR;
        return "";
    }
    return InnerFormat(num, false, true, false, status);
}

std::string NumberFormatImpl::FormatNoGroup(double num, NumberFormatType type, int &status) const
{
    if (defaultData == nullptr) {
        status = IERROR;
        return "";
    }
    if (type == PERCENT) { // percent,the decimal needs to be multiplied by 100.
        return InnerFormat(num * 100, true, false, true, status);
    } else {
        return InnerFormat(num, true, false, false, status);
    }
}

std::string NumberFormatImpl::FormatNoGroup(int num, int &status) const
{
    if (defaultData == nullptr) {
        status = IERROR;
        return "";
    }
    return InnerFormat(num, false, false, false, status);
}

bool NumberFormatImpl::SetMaxDecimalLength(int length)
{
    if (defaultData == nullptr) {
        return false;
    }
    int adjustValue = (length < 0) ? -1 : length;
    if ((minDecimalLength >= 0) && (minDecimalLength > adjustValue)) {
        minDecimalLength = adjustValue;
        defaultData->SetMinDecimalLength(adjustValue);
    }
    maxDecimalLength = adjustValue;
    defaultData->SetMaxDecimalLength(adjustValue);
    defaultData->UpdateNumberFormat();
    return true;
}

bool NumberFormatImpl::SetMinDecimalLength(int length)
{
    if (defaultData == nullptr) {
        return false;
    }
    int adjustValue = (length < 0) ? -1 : length;
    if ((maxDecimalLength >= 0) && (maxDecimalLength < adjustValue)) {
        maxDecimalLength = adjustValue;
        defaultData->SetMaxDecimalLength(maxDecimalLength);
    }
    minDecimalLength = adjustValue;
    defaultData->SetMinDecimalLength(adjustValue);
    defaultData->UpdateNumberFormat();
    return true;
}

char *NumberFormatImpl::FillMinDecimal(const char *target, int len, int addSize, bool isDec) const
{
    char *content = I18nNewCharString(target, len + addSize);
    if (content == nullptr) {
        return nullptr;
    }
    for (int i = 0; i < addSize; i++) {
        if ((!isDec) && (i == 0)) {
            content[len + i] = '.';
            continue;
        }
        content[len + i] = '0';
    }
    return content;
}
