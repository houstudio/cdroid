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

#include "i18n_memory_adapter.h"
#include "str_util.h"
#include "number_data.h"

using namespace cdroid::i18n;

StyleData::StyleData(const StyleData &data)
{
    decLen = data.decLen;
    intLen = data.intLen;
    percentIntLen = data.percentIntLen;
    isTwoGroup = data.isTwoGroup;
    if (data.numFormat != nullptr) {
        int len = strlen(data.numFormat);
        numFormat = NewArrayAndCopy(data.numFormat, len);
    }
    if (data.entireFormat != nullptr) {
        int len = strlen(data.entireFormat);
        entireFormat = I18nNewCharString(data.entireFormat, len);
    }
}

StyleData::~StyleData()
{
    I18nFree(static_cast<void *>(numFormat));
    I18nFree(static_cast<void *>(entireFormat));
}

StyleData &StyleData::operator=(const StyleData &data)
{
    decLen = data.decLen;
    intLen = data.intLen;
    percentIntLen = data.percentIntLen;
    isTwoGroup = data.isTwoGroup;
    if (data.numFormat != nullptr) {
        int len = strlen(data.numFormat);
        numFormat = I18nNewCharString(data.numFormat, len);
    }
    if (data.entireFormat != nullptr) {
        int len = strlen(data.entireFormat);
        entireFormat = I18nNewCharString(data.entireFormat, len);
    }
    return *this;
}

void NumberData::SetNumSystem(std::string *numSym, const int numSize)
{
    if (numSym == nullptr || numSize <= 0) {
        return;
    }
    ArrayCopy(nativeNums, NUM_SIZE, numSym, numSize);
    if (!(numSym[0]).empty() && (numSym[0][0] != '0')) {
        isNative = true;
    }
}

void NumberData::Init(const char *pat, int patLen, const char *percentPat, int perPatLen)
{
    numberFormatPattern = I18nNewCharString(pat, patLen);
    percentFormatPattern = I18nNewCharString(percentPat, perPatLen);
    ParsePattern(pat, patLen);
    ParsePercentPattern(percentPat, perPatLen);
}

void NumberData::InitSign(const std::string *signs, int size)
{
    if (signs == nullptr || size < PERCENT_SIGN_INDEX) {
        return;
    }
    std::string decSign = signs[0]; // use array to store num data, first is decimal sign
    std::string groupSign = signs[1]; // use array to store num data, second is group sign
    std::string perSign = signs[PERCENT_SIGN_INDEX]; // use array to store num data, third is percent sign
    const char *td = decSign.c_str();
    decimal = I18nNewCharString(td, strlen(td));
    const char *tdg = groupSign.c_str();
    group = I18nNewCharString(tdg, strlen(tdg));
    const char *tdp = perSign.c_str();
    percent = I18nNewCharString(tdp, strlen(tdp));
}

void NumberData::ParsePattern(const char *pattern, const int len)
{
    bool isDec = CalculateDecLength(pattern, len);
    CalculateIntLength(len - style.decLen, pattern, len, isDec);
    if ((pattern != nullptr) && (strcmp(pattern, "#,##,##0.###")) == 0) {
        style.isTwoGroup = true;
    }
    UpdateNumberFormat();
}

bool NumberData::CalculateDecLength(const char *pattern, const int len)
{
    if (pattern == nullptr || len <= 0) {
        return false;
    }
    bool isDec = false;
    int decLen = 0;
    for (int i = 0; i < len; i++) { // calculate the format after decimal sign
        char temp = pattern[i];
        if (temp == '.') {
            isDec = true;
            continue;
        }
        if (isDec && ((temp == '#') || (temp == '0'))) {
            decLen++;
        }
    }
    style.decLen = decLen;
    return isDec;
}

void NumberData::CalculateIntLength(int intEndPos, const char *pattern, const int len, bool isDec)
{
    if (pattern == nullptr || len <= 0) {
        return;
    }
    if (isDec) {
        --intEndPos;
    }
    int intLen = 0;
    for (; intEndPos > 0; --intEndPos) {
        if (pattern[intEndPos - 1] != '0') {
            break;
        }
        ++intLen;
    }
    style.intLen = intLen;
}

void NumberData::ParsePercentPattern(const char *pattern, const int len)
{
    if (pattern == nullptr || len <= 0) {
        return;
    }
    int perSignPos = 0; // 0 : no percent 1:left 2:right;
    int space = 0; // 0 = 0020, 1 = c2a0
    int hasSpace = 0;
    if (pattern[0] == '%') {
        perSignPos = LEFT;
        if ((len >= 2) && pattern[1] == ' ') { // length >= 2 guarantees that we can safely get second byte
            hasSpace = 1;
        } else if (IsNoBreakSpace(pattern, len, true)) {
            hasSpace = 1;
            space = 1;
        }
    } else if (pattern[len - 1] == '%') {
        perSignPos = RIGHT;
        if ((len >= 2) && (pattern[len - 2] == ' ')) { // len - 2 position has a spacce
            hasSpace = 1;
        } else if (IsNoBreakSpace(pattern, len, false)) {
            hasSpace = 1;
            space = 1;
        }
    }
    ParseOtherPerPattern(pattern, len, perSignPos, space, hasSpace);
}

bool NumberData::IsNoBreakSpace(const char *pattern, const int len, bool order)
{
    if (len < 3) { // pattern should at least have 3 bytes
        return false;
    }
    int firstPosition = order ? 2 : (len - 2); // 2 is the offset to find ARABIC_NOBREAK_ONE
    int secondPosition = order ? 1 : (len - 3); // 3 is the offset to find ARABIC_NOBREAK_TWO
    if ((static_cast<signed char>(pattern[firstPosition]) == ARABIC_NOBREAK_ONE) &&
        (static_cast<signed char>(pattern[secondPosition]) == ARABIC_NOBREAK_TWO)) {
        return true;
    }
    return false;
}

void NumberData::ParseOtherPerPattern(const char *pattern, const int len, const int perSignPos,
    const int space, const int hasSpace)
{
    // 2 is the minimal length of pattern
    if (pattern == nullptr || len < 2) {
        return;
    }
    std::string type;
    if (perSignPos > 0) {
        if (perSignPos == 1) {
            type = "%%%s";
            if ((hasSpace > 0) && (space == 0)) {
                type = "%% %s";
            } else if ((hasSpace > 0) && (space == 1)) {
                unsigned char typeChars[] = { 0x25, 0x25, 0xC2, 0xA0, 0x25, 0x73, 0x0 }; // %%\uc2a0%s
                type = reinterpret_cast<char const *>(typeChars);
            } else {
                // do nothing
            }
        } else {
            type = "%s%%";
            if ((hasSpace > 0) && (space == 0)) {
                type = "%s %%";
            } else if ((hasSpace > 0) && (space == 1)) {
                unsigned char typeChars[] = { 0x25, 0x73, 0xC2, 0xA0, 0x25, 0x25, 0x0 }; // %s\uc2a0%%
                type = reinterpret_cast<char const *>(typeChars);
            } else {
                // do nothing
            }
        }
    } else {
        type = "%s%%";
    }
    I18nFree(static_cast<void *>(style.entireFormat));
    int typeLen = type.size();
    style.entireFormat = I18nNewCharString(type.data(), typeLen);
}

void NumberData::SetMinDecimalLength(int length)
{
    style.minDecimalLength = length;
}

NumberData::NumberData(const char *pat, const char *percentPat, std::string decSign,
    std::string groupSign, std::string perSign)
{
    if (pat != nullptr || percentPat != nullptr) {
        std::string nums[NUM_SIZE] = NUMBER_SIGN;
        SetNumSystem(nums, NUM_SIZE);
        std::string signs[3] = { decSign, groupSign, perSign }; // use string array contain number data
        int len = -1;
        int patLen = -1;
        if (pat != nullptr) {
            len = strlen(pat);
        }
        if (percentPat != nullptr) {
            patLen = strlen(percentPat);
        }
        Init(pat, len, percentPat, patLen);
        InitSign(signs, SIGNS_SIZE);
    }
}

NumberData::NumberData()
{
    isNative = false;
    std::string signs[3] = { ".", ",", "%" }; // use string array contain number data
    const char *enNumberPattern = "#,##0.###";
    const char *percentPattern = "#,##0%";
    Init(enNumberPattern, strlen(enNumberPattern), percentPattern, strlen(percentPattern));
    InitSign(signs, SIGNS_SIZE);
}

NumberData::~NumberData()
{
    I18nFree(static_cast<void *>(group));
    I18nFree(static_cast<void *>(percent));
    I18nFree(static_cast<void *>(decimal));
    I18nFree(static_cast<void *>(numberFormatPattern));
    I18nFree(static_cast<void *>(percentFormatPattern));
}

bool NumberData::IsSuccess()
{
    bool r = isSucc;
    isSucc = true;
    return r;
}

void NumberData::SetMaxDecimalLength(int length)
{
    style.maxDecimalLength = length;
}

void NumberData::GetNumberingSystem(const char *numberingSystem, std::string &numberFormatString,
    std::string &digitsRet)
{
    numberFormatString = "#,##0.###_#,##0%_._,_%";
    digitsRet = "0;1;2;3;4;5;6;7;8;9";
    if (numberingSystem == nullptr || (strcmp(numberingSystem, "latn") == 0)) {
        return;
    }
    if (strcmp(numberingSystem, "arab") == 0) {
        signed char arabFormatArray[] = {
            35, 44, 35, 35, 48, 46, 35, 35, 35, 95, 35, 44, 35, 35, 48, 37, -30, -128, -113, 95, -39, -85, 95, -39, -84,
            95, -39, -86, -40, -100, 0
        };
        numberFormatString = reinterpret_cast<char *>(arabFormatArray);
        signed char localeDigitsArab[] = {
            -39, -96, 59, -39, -95, 59, -39, -94, 59, -39, -93, 59, -39, -92, 59, -39, -91, 59, -39, -90, 59, -39, -89,
            59, -39, -88, 59, -39, -87, 0
        };
        digitsRet = reinterpret_cast<char *>(localeDigitsArab);
    } else if (strcmp(numberingSystem, "arabext") == 0) {
        signed char extFormatArray[] = {
            35, 44, 35, 35, 48, 46, 35, 35, 35, 95, 35, 44, 35, 35, 48, 37, 95, -39, -85, 95, -39, -84, 95, -39, -86, 0
        };
        numberFormatString = reinterpret_cast<char *>(extFormatArray);
        signed char localeDigitsArabext[] = {
            -37, -80, 59, -37, -79, 59, -37, -78, 59, -37, -77, 59, -37, -76, 59, -37, -75, 59, -37, -74, 59, -37, -73,
            59, -37, -72, 59, -37, -71, 0
        };
        digitsRet = reinterpret_cast<char *>(localeDigitsArabext);
    } else if (strcmp(numberingSystem, "beng") == 0) {
        numberFormatString = "#,##,##0.###_#,##0%_._,_%";
        signed char localeDigitsBeng[] = {
            -32, -89, -90, 59, -32, -89, -89, 59, -32, -89, -88, 59, -32, -89, -87, 59, -32, -89, -86, 59, -32, -89,
            -85, 59, -32, -89, -84, 59, -32, -89, -83, 59, -32, -89, -82, 59, -32, -89, -81, 0
        };
        digitsRet = reinterpret_cast<char *>(localeDigitsBeng);
    } else if (strcmp(numberingSystem, "deva") == 0) {
        numberFormatString = "#,##,##0.###_#,##,##0%_._,_%";
        signed char localeDigitsDeva[] = {
            -32, -91, -90, 59, -32, -91, -89, 59, -32, -91, -88, 59, -32, -91, -87, 59, -32, -91, -86, 59, -32, -91,
            -85, 59, -32, -91, -84, 59, -32, -91, -83, 59, -32, -91, -82, 59, -32, -91, -81, 0
        };
        digitsRet = reinterpret_cast<char *>(localeDigitsDeva);
    } else if (strcmp(numberingSystem, "mymr") == 0) {
        signed char localeDigitsMymr[] = {
            -31, -127, -128, 59, -31, -127, -127, 59, -31, -127, -126, 59, -31, -127, -125, 59, -31, -127, -124, 59,
            -31, -127, -123, 59, -31, -127, -122, 59, -31, -127, -121, 59, -31, -127, -120, 59, -31, -127, -119, 0
        };
        digitsRet = reinterpret_cast<char *>(localeDigitsMymr);
    } else {
        // do noting
    }
}

void NumberData::UpdateNumberFormat()
{
    // reset the style's number pattern which is used to format a decimal number
    char *format = reinterpret_cast<char *>(I18nMalloc(NUMBER_FORMAT_LENGTH));
    if (format == nullptr) {
        isSucc = false;
        return;
    }
    int finalDecLength = GetNumberFormatLength();
    if (snprintf(format,NUMBER_FORMAT_LENGTH , NUMBER_FORMAT, finalDecLength) == -1) {
        isSucc = false;
        I18nFree(static_cast<void *>(format));
        return;
    }
    I18nFree(static_cast<void *>(style.numFormat));
    style.numFormat = format;
}

int NumberData::GetNumberFormatLength()
{
    if (style.minDecimalLength < 0) {
        if (style.maxDecimalLength < 0) {
            return style.decLen;
        }
        return style.maxDecimalLength;
    } else {
        if (style.maxDecimalLength < 0) {
            return (style.minDecimalLength > style.decLen) ? style.minDecimalLength : style.decLen;
        } else {
            return style.maxDecimalLength;
        }
    }
}

void NumberData::SetMinusSign(const std::string &minus)
{
    this->minusSign = minus;
}

std::string NumberData::GetMinusSign()
{
    return this->minusSign;
}

