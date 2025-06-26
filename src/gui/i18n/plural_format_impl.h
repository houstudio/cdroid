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

#ifndef PLURALFORMAT_IMPL_H
#define PLURALFORMAT_IMPL_H

#include "data_resource.h"
#include "locale_info.h"
#include "plural_rules.h"
#include "types.h"

namespace OHOS {
namespace I18N {
class PluralFormatImpl {
public:
    PluralFormatImpl(LocaleInfo &locale, I18nStatus &status);
    virtual ~PluralFormatImpl();
    int GetPluralRuleIndex(int number, I18nStatus status) const;
    int GetPluralRuleIndex(double number, I18nStatus status) const;
    bool Init(const DataResource &resource);

private:
    PluralRules *mPluralRules = nullptr;
    PluralRules *mDecimalPluralRules = nullptr;
    LocaleInfo mLocale;
    PluralRules *GetPluralData(I18nStatus status) const;
    bool ParseRule(const std::string &rule, const int ruleSize, const int number) const;
    bool ParseFormula(const std::string &rule, const int ruleSize, int &index, const int number) const;
    bool CompareResult(const std::string &rule, const int ruleSize, int &index, const int number) const;
    bool CompareNotEqualResult(const std::string &rule, const int ruleSize, int &index, const int number) const;
    int ParseNumber(const std::string &rule, const int ruleSize, int &index) const;
    bool ParseDecimalRule(const std::string &rule, const int ruleSize, const int *numberInfo,
        const int numberInfoSize) const;
    bool ParseDecimalFormula(const std::string &rule, const int ruleSize, int &index, const int *numberInfo,
        const int numberInfoSize) const;
    void ComputeDecimalInfo(double number, int integerNumber, int *numberInfo, const int numberInfoSize) const;
    PluralRules *InitPluralRules(std::string unprocessedPluralData);
    bool CheckContainsIntegerRule() const;
    const int SYMBOL_LENGTH = 1;
    const int SKIP_SYMBOL_LENGTH = 2;
    const char EQUAL = '=';
    const char NOT_EQUAL = '!';
    const char MOD = '%';
    const char AND = 'a';
    const char OR = 'o';
    const char TO = '<';
    const char COMMA = ',';
    const char NUM_OF_FRACTION = 'v';
    const char FRACTION_NUMBER = 'f';
    const char FRACTION_NUMBER_WITH_ZERO = 't';
    const int INTEGER_NUMBER_INDEX = 0;
    const int FRACTION_NUMBER_INDEX = 1;
    const int NUM_OF_FRACTION_INDEX = 2;
    static constexpr int NUMBER_INFO_SIZE = 3;
    const double EPS = 1e-6;
    const int MAX_FRACTION_NUMBERS = 6;
    const int DECIMALISM = 10;
};
} // namespace I18N
} // namespace OHOS

#endif