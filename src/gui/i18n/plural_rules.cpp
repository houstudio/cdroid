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

#include "plural_rules.h"

using namespace OHOS::I18N;

PluralRules::PluralRules(std::string *rules, const int rulesLength, const int *ruleSize, const int sizesLength)
{
    if ((rules != nullptr) && (ruleSize != nullptr) &&
        (rulesLength > PluralRuleType::OTHER) && (sizesLength > PluralRuleType::OTHER)) {
        mZeroRule = rules[PluralRuleType::ZERO];
        mZeroRuleSize = ruleSize[PluralRuleType::ZERO];
        mOneRule = rules[PluralRuleType::ONE];
        mOneRuleSize = ruleSize[PluralRuleType::ONE];
        mTwoRule = rules[PluralRuleType::TWO];
        mTwoRuleSize = ruleSize[PluralRuleType::TWO];
        mFewRule = rules[PluralRuleType::FEW];
        mFewRuleSize = ruleSize[PluralRuleType::FEW];
        mManyRule = rules[PluralRuleType::MANY];
        mManyRuleSize = ruleSize[PluralRuleType::MANY];
        mOtherRule = rules[PluralRuleType::OTHER];
        mOtherRuleSize = ruleSize[PluralRuleType::OTHER];
    }
}

PluralRules::~PluralRules() {}