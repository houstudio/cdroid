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

#ifndef PLURALRULE_H
#define PLURALRULE_H

#include "types.h"
#include "locale_info.h"

namespace OHOS {
namespace I18N {
class PluralRules {
public:
    PluralRules(std::string *rules, const int rulesLength, const int *rulesSize, const int sizesLength);
    virtual ~PluralRules();
    std::string mZeroRule = "";
    int mZeroRuleSize = 0;
    std::string mOneRule = "";
    int mOneRuleSize = 0;
    std::string mTwoRule = "";
    int mTwoRuleSize = 0;
    std::string mFewRule = "";
    int mFewRuleSize = 0;
    std::string mManyRule = "";
    int mManyRuleSize = 0;
    std::string mOtherRule = "";
    int mOtherRuleSize = 0;
};
} // namespace I18N
} // namespace OHOS

#endif