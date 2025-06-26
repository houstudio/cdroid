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

#include "plural_format_impl.h"
#include <cmath>
#include "str_util.h"

using namespace OHOS::I18N;

PluralFormatImpl::PluralFormatImpl(LocaleInfo &locale, I18nStatus &status)
{
    if (locale.GetId() == nullptr) {
        status = IERROR;
        return;
    }
    mLocale = locale;
}

PluralFormatImpl::~PluralFormatImpl()
{
    if (mPluralRules != nullptr) {
        delete mPluralRules;
        mPluralRules = nullptr;
    }
    if (mDecimalPluralRules != nullptr) {
        delete mDecimalPluralRules;
        mDecimalPluralRules = nullptr;
    }
}

bool PluralFormatImpl::Init(const DataResource &resource)
{
    std::string unprocessedPluralData;
    resource.GetString(DataResourceType::PLURAL, unprocessedPluralData);
    std::string unprocessedDecimalPluralData;
    resource.GetString(DataResourceType::DECIMAL_PLURAL, unprocessedDecimalPluralData);
    mPluralRules = InitPluralRules(unprocessedPluralData);
    mDecimalPluralRules = InitPluralRules(unprocessedDecimalPluralData);
    if (mPluralRules == nullptr || mDecimalPluralRules == nullptr) {
        return false;
    }
    return true;
}

PluralRules *PluralFormatImpl::InitPluralRules(std::string unprocessedPluralData)
{
    std::string rules[RULES_NUM];
    Split(unprocessedPluralData, rules, RULES_NUM, PLURAL_SEP);
    int zeroRuleSize = static_cast<int>(rules[PluralRuleType::ZERO].size());
    int oneRuleSize = static_cast<int>(rules[PluralRuleType::ONE].size());
    int twoRuleSize = static_cast<int>(rules[PluralRuleType::TWO].size());
    int fewRuleSize = static_cast<int>(rules[PluralRuleType::FEW].size());
    int manyRuleSize = static_cast<int>(rules[PluralRuleType::MANY].size());
    int otherRuleSize = static_cast<int>(rules[PluralRuleType::OTHER].size());
    int ruleSizes[RULES_NUM] = { zeroRuleSize, oneRuleSize, twoRuleSize, fewRuleSize, manyRuleSize, otherRuleSize };
    return new PluralRules(rules, RULES_NUM, ruleSizes, RULES_NUM);
}

PluralRules *PluralFormatImpl::GetPluralData(I18nStatus status) const
{
    if (status == IERROR) {
        return nullptr;
    }
    return mPluralRules;
}

int PluralFormatImpl::GetPluralRuleIndex(double number, I18nStatus status) const
{
    if (status == IERROR) {
        return -1;
    }
    if (GetPluralData(status) == nullptr) {
        return PluralRuleType::OTHER;
    }
    int integerNumber = (int)number;
    int numberInfo[NUMBER_INFO_SIZE];
    ComputeDecimalInfo(number, integerNumber, numberInfo, NUMBER_INFO_SIZE);
    if ((mDecimalPluralRules->mZeroRuleSize > 0) && ParseDecimalRule(mDecimalPluralRules->mZeroRule,
        mDecimalPluralRules->mZeroRuleSize, numberInfo, NUMBER_INFO_SIZE)) {
        return PluralRuleType::ZERO;
    } else if ((mDecimalPluralRules->mOneRuleSize > 0) && ParseDecimalRule(mDecimalPluralRules->mOneRule,
        mDecimalPluralRules->mOneRuleSize, numberInfo, NUMBER_INFO_SIZE)) {
        return PluralRuleType::ONE;
    } else if ((mDecimalPluralRules->mTwoRuleSize > 0) && ParseDecimalRule(mDecimalPluralRules->mTwoRule,
        mDecimalPluralRules->mTwoRuleSize, numberInfo, NUMBER_INFO_SIZE)) {
        return PluralRuleType::TWO;
    } else if ((mDecimalPluralRules->mFewRuleSize > 0) && ParseDecimalRule(mDecimalPluralRules->mFewRule,
        mDecimalPluralRules->mFewRuleSize, numberInfo, NUMBER_INFO_SIZE)) {
        return PluralRuleType::FEW;
    } else if ((mDecimalPluralRules->mManyRuleSize > 0) && ParseDecimalRule(mDecimalPluralRules->mManyRule,
        mDecimalPluralRules->mManyRuleSize, numberInfo, NUMBER_INFO_SIZE)) {
        return PluralRuleType::MANY;
    } else {
        if (!CheckContainsIntegerRule() && numberInfo[FRACTION_NUMBER_INDEX] == 0) {
            return GetPluralRuleIndex(integerNumber, status);
        }
        return PluralRuleType::OTHER;
    }
}

bool PluralFormatImpl::CheckContainsIntegerRule() const
{
    if ((mDecimalPluralRules->mZeroRuleSize > 0) &&
        (mDecimalPluralRules->mZeroRule.find("v = 0") != std::string::npos)) {
        return true;
    } else if ((mDecimalPluralRules->mOneRuleSize > 0) &&
        (mDecimalPluralRules->mOneRule.find("v = 0") != std::string::npos)) {
        return true;
    } else if ((mDecimalPluralRules->mTwoRuleSize > 0) &&
        (mDecimalPluralRules->mTwoRule.find("v = 0") != std::string::npos)) {
        return true;
    } else if ((mDecimalPluralRules->mFewRuleSize > 0) &&
        (mDecimalPluralRules->mFewRule.find("v = 0") != std::string::npos)) {
        return true;
    } else if ((mDecimalPluralRules->mManyRuleSize > 0) &&
        (mDecimalPluralRules->mManyRule.find("v = 0") != std::string::npos)) {
        return true;
    } else {
        return false;
    }
}

void PluralFormatImpl::ComputeDecimalInfo(double number, int integerNumber, int *numberInfo,
    const int numberInfoSize) const
{
    int fractionNumber = 0;
    int numOfFraction = 0;
    for (int i = 1; i <= MAX_FRACTION_NUMBERS; i++) {
        // Calculate number of fraction digits in the decimal number by judging whether
        // multiplying by 10 is an integer.
        double temp = number * pow(10, i);
        if (i == MAX_FRACTION_NUMBERS && temp - ((int)temp) >= EPS) {
            temp = round(temp);
        }
        if (temp - ((int)temp) < EPS) {
            while (i > 1 && (int) temp % DECIMALISM == 0) { // 10 means decimal
                i--;
                temp /= DECIMALISM; // 10 means decimal
            }
            // 10 is base
            fractionNumber = (int)(temp - integerNumber * pow(10, i));
            numOfFraction = i;
            break;
        }
    }
    numberInfo[INTEGER_NUMBER_INDEX] = integerNumber;
    numberInfo[FRACTION_NUMBER_INDEX] = fractionNumber;
    numberInfo[NUM_OF_FRACTION_INDEX] = numOfFraction;
}

bool PluralFormatImpl::ParseDecimalRule(const std::string &rule, const int ruleSize, const int *numberInfo,
    const int numberInfoSize) const
{
    bool tempResult = true;
    for (int i = 0; i < ruleSize; i++) {
        bool curResult = ParseDecimalFormula(rule, ruleSize, i, numberInfo, numberInfoSize);
        int nextSymbolIndex = i + SYMBOL_LENGTH;
        if (curResult && tempResult) {
            // If next symbol is or and current result and temp result are true, the final result should be true.
            if ((nextSymbolIndex < ruleSize) && (rule[nextSymbolIndex] == OR)) {
                return true;
            // If next symbol is and and current result and temp result are true, set the temp result to true and
            // skip to next formula.
            } else if ((nextSymbolIndex < ruleSize) && (rule[nextSymbolIndex] == AND)) {
                i += SKIP_SYMBOL_LENGTH;
                tempResult = true;
            // If there is no symbol after this formula and current result and temp result are true,
            // the final result should be true.
            } else if (nextSymbolIndex >= ruleSize) {
                return true;
            }
        } else {
            // If next symbol is or and current result or temp result is false, skip to next formula.
            if ((nextSymbolIndex < ruleSize) && (rule[nextSymbolIndex] == OR)) {
                i += SKIP_SYMBOL_LENGTH;
                tempResult = true;
            // If next symbol is and and current result or temp result is false, skip to next formula and
            // set temp result to false.
            } else if ((nextSymbolIndex < ruleSize) && (rule[nextSymbolIndex] == AND)) {
                i += SKIP_SYMBOL_LENGTH;
                tempResult = false;
            } else if ((nextSymbolIndex >= ruleSize) &&
                        !ParseDecimalFormula(rule, ruleSize, i, numberInfo, numberInfoSize)) {
                tempResult = false;
            }
        }
    }
    return tempResult;
}

bool PluralFormatImpl::ParseDecimalFormula(const std::string &rule, const int ruleSize, int &index,
    const int *numberInfo, const int numberInfoSize) const
{
    int currentNumber = 0;
    if ((index < ruleSize) && (rule[index] == NUM_OF_FRACTION)) {
        currentNumber = numberInfo[NUM_OF_FRACTION_INDEX];
    } else if ((index < ruleSize) &&
        ((rule[index] == FRACTION_NUMBER) || (rule[index] == FRACTION_NUMBER_WITH_ZERO))) {
        currentNumber = numberInfo[FRACTION_NUMBER_INDEX];
    } else {
        currentNumber = numberInfo[INTEGER_NUMBER_INDEX];
    }
    index += SKIP_SYMBOL_LENGTH;
    return ParseFormula(rule, ruleSize, index, currentNumber);
}

int PluralFormatImpl::GetPluralRuleIndex(int number, I18nStatus status) const
{
    if (status == IERROR) {
        return -1;
    }
    if (GetPluralData(status) == nullptr) {
        return PluralRuleType::OTHER;
    }
    if ((mPluralRules->mZeroRuleSize > 0) && ParseRule(mPluralRules->mZeroRule, mPluralRules->mZeroRuleSize, number)) {
        return PluralRuleType::ZERO;
    } else if ((mPluralRules->mOneRuleSize > 0) &&
        ParseRule(mPluralRules->mOneRule, mPluralRules->mOneRuleSize, number)) {
        return PluralRuleType::ONE;
    } else if ((mPluralRules->mTwoRuleSize > 0) &&
        ParseRule(mPluralRules->mTwoRule, mPluralRules->mTwoRuleSize, number)) {
        return PluralRuleType::TWO;
    } else if ((mPluralRules->mFewRuleSize > 0) &&
        ParseRule(mPluralRules->mFewRule, mPluralRules->mFewRuleSize, number)) {
        return PluralRuleType::FEW;
    } else if ((mPluralRules->mManyRuleSize > 0) &&
        ParseRule(mPluralRules->mManyRule, mPluralRules->mManyRuleSize, number)) {
        return PluralRuleType::MANY;
    } else {
        return PluralRuleType::OTHER;
    }
}

bool PluralFormatImpl::ParseRule(const std::string &rule, const int ruleSize, const int number) const
{
    bool tempResult = true;
    for (int i = 0; i < ruleSize; i++) {
        bool curResult = ParseFormula(rule, ruleSize, i, number);
        int nextSymbolIndex = i + SYMBOL_LENGTH;
        if (curResult && tempResult) {
            // If next symbol is or and current result and temp result are true, the final result should be true.
            if ((nextSymbolIndex < ruleSize) && (rule[nextSymbolIndex] == OR)) {
                return true;
            // If next symbol is and and current result and temp result are true, set the temp result to true and
            // skip to next formula.
            } else if ((nextSymbolIndex < ruleSize) && (rule[nextSymbolIndex] == AND)) {
                i += SKIP_SYMBOL_LENGTH;
                tempResult = true;
            // If there is no symbol after this formula and current result and temp result are true,
            // the final result should be true.
            } else if (nextSymbolIndex >= ruleSize) {
                return true;
            }
        } else {
            // If next symbol is or and current result or temp result is false, skip to next formula.
            if ((nextSymbolIndex < ruleSize) && (rule[nextSymbolIndex] == OR)) {
                i += SKIP_SYMBOL_LENGTH;
                tempResult = true;
            // If next symbol is and and current result or temp result is false, skip to next formula and
            // set temp result to false.
            } else if ((nextSymbolIndex < ruleSize) && (rule[nextSymbolIndex] == AND)) {
                i += SKIP_SYMBOL_LENGTH;
                tempResult = false;
            } else if ((nextSymbolIndex >= ruleSize) && !ParseFormula(rule, ruleSize, i, number)) {
                tempResult = false;
            }
        }
    }
    return tempResult;
}

bool PluralFormatImpl::ParseFormula(const std::string &rule, const int ruleSize, int &index, const int number) const
{
    int currentNumber = number;
    if ((index < ruleSize) && (rule[index] == MOD)) {
        // Skip the module symbol and obtain the divisor number.
        index += SKIP_SYMBOL_LENGTH;
        int divisor = ParseNumber(rule, ruleSize, index);
        if (divisor == 0) {
            divisor = 1;
        }
        currentNumber = number % divisor;
        index++;
    }
    // Compare the result with the equation
    if (CompareResult(rule, ruleSize, index, currentNumber)) {
        return true;
    }
    return false;
}

bool PluralFormatImpl::CompareResult(const std::string &rule, const int ruleSize, int &index, const int number) const
{
    if (!((index < ruleSize) && (rule[index] == EQUAL))) {
        return CompareNotEqualResult(rule, ruleSize, index, number);
    }

    index += SKIP_SYMBOL_LENGTH;
    int num = ParseNumber(rule, ruleSize, index);
    bool temp = false;

    // Obtain all numbers in the formula
    while ((index < ruleSize) && ((rule[index] == COMMA) || (rule[index] == TO))) {
        if (rule[index] == TO) {
            // If the symbol is "to", it indicates a number range.
            int rangeStart = num;
            index++;
            int rangeEnd = ParseNumber(rule, ruleSize, index);
            if ((number >= rangeStart) && (number <= rangeEnd)) {
                temp = true;
            }
        } else {
            // Compare the input number with each number in the equation.
            if (number == num) {
                temp = true;
            }
            index++;
            num = ParseNumber(rule, ruleSize, index);
        }
    }
    if (number == num) {
        temp = true;
    }
    return temp;
}

bool PluralFormatImpl::CompareNotEqualResult(const std::string &rule, const int ruleSize, int &index,
    const int number) const
{
    if (!((index < ruleSize) && (rule[index] == NOT_EQUAL))) {
        return false;
    }

    index += SKIP_SYMBOL_LENGTH;
    int num = ParseNumber(rule, ruleSize, index);
    bool temp = true;

    // Obtain all numbers in the formula
    while ((index < ruleSize) && ((rule[index] == COMMA) || (rule[index] == TO))) {
        if (rule[index] == TO) {
            // If the symbol is "to", it indicates a number range.
            index++;
            int rangeStart = num;
            int rangeEnd = ParseNumber(rule, ruleSize, index);
            if ((number >= rangeStart) && (number <= rangeEnd)) {
                temp = false;
            }
        } else {
            // Compare the input number with each number in the equation.
            if (number == num) {
                temp = false;
            }
            index++;
            num = ParseNumber(rule, ruleSize, index);
        }
    }
    if (number == num) {
        temp = false;
    }
    return temp;
}

int PluralFormatImpl::ParseNumber(const std::string &rule, const int ruleSize, int &index) const
{
    int num = 0;

    // Parse number in the formula.
    while ((index < ruleSize) && (rule[index] != ' ') && (rule[index] != TO) && (rule[index] != COMMA)) {
        num *= DECIMALISM; // 10 means decimal. Calculate decimal value of the number.
        num += rule[index] - '0';
        index++;
    }
    return num;
}
