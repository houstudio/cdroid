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

#ifndef NUMBER_DATA_H
#define NUMBER_DATA_H

#define NUMBER_SIGN {                                                    \
        "0", "1", "2", "3", "4", "5", "6", "7", "8", "9" \
    }
#define ARABIC_NOBREAK_ONE_MINUS 2
#define ARABIC_NOBREAK_TWO_MINUS 3
#define PERCENT_SIGN_INDEX 2
#define SIGNS_SIZE 3
#define PERCENT_INFO_SIZE 3

#include "types.h"
#include "data_resource.h"

namespace OHOS {
namespace I18N {
struct StyleData {
    int decLen = 0;
    int intLen = 0;
    int percentIntLen = 0;
    int maxDecimalLength = -1;
    int minDecimalLength = -1;
    char *numFormat = nullptr; // number format style
    char *entireFormat = nullptr;
    bool isTwoGroup = false;
    StyleData() = default;
    ~StyleData();
    StyleData(const StyleData &data);
    StyleData& operator=(const StyleData &data);
};

class NumberData {
public:
    static const char NUMBER_DECIMAL = '.';
    static const char NUMBER_GROUPSIGN = ',';
    static const char NUMBER_PERCENT = '%';
    static const int NUMBER_GROUP = 3;
    static const int TWO_GROUP = 2;
    static constexpr int NUM_SIZE = 10;
    static constexpr int INFO_SIZE = 3;
    std::string nativeNums[NUM_SIZE] = {}; // used to store 0-9 letters in current language
    char *decimal = nullptr;
    char *group = nullptr;
    char *percent = nullptr;
    std::string minusSign;
    bool isNative = false;
    StyleData style;
    friend class NumberFormatImpl;
    NumberData();
    NumberData(const char *pat, const char *percentPat, std::string decSign, std::string groupSign,
        std::string perSign);
    virtual ~NumberData();
    void SetNumSystem(std::string *numSym, const int numSize);
    void SetMinDecimalLength(int length);
    void SetMaxDecimalLength(int length);
    void SetMinusSign(const std::string &minus);
    std::string GetMinusSign();

private:
    static void GetNumberingSystem(const char *numberingSystem, std::string &numberFormatString,
        std::string &digitsRet);
    static bool IsNoBreakSpace(const char *pattern, const int len, bool order);
    void Init(const char *pat, int patLen, const char *percentPat, int perPatLen);
    void InitSign(const std::string *signs, const int signLength);
    void ParsePattern(const char *pattern, const int len);
    void ParsePercentPattern(const char *pattern, const int len);
    void ParseOtherPerPattern(const char *pattern, const int len, const int perSignPos,
        const int space, const int hasSpace);
    void CalculateIntLength(int intEndPos, const char *pattern, const int len, bool isDec);
    bool CalculateDecLength(const char *pattern, const int len);
    bool IsSuccess();
    void UpdateNumberFormat();
    int GetNumberFormatLength();
    char *numberFormatPattern = nullptr;
    char *percentFormatPattern = nullptr;
    bool isSucc = true;
    bool isPercent = false;
    int maxDecimalLength = -1;
    const char *NUMBER_FORMAT = "%%.%df";
    const int NUMBER_FORMAT_LENGTH = 5;
    static const int ARABIC_NOBREAK_ONE = -96;
    static const int ARABIC_NOBREAK_TWO = -62;
};

enum EPercentLocation {
    UNKNOWN = 0,
    LEFT = 1,
    RIGHT = 2
};
} // namespace I18N
} // namespace OHOS
#endif
