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

#ifndef PLURALFORMAT_H
#define PLURALFORMAT_H

/**
* @addtogroup I18N
* @{
*
* @brief Provides APIs related to internationalization (i18n), with which you can
* format date, time, and numbers and obtain plural rules.
*
* @since 2.2
* @version 1.0
*/

/**
* @file plural_format.h
*
* @brief Provides functions that are used to obtain plural rules.
*
* @since 2.2
* @version 1.0
*/

#include "types.h"
#include "locale_info.h"
#include "plural_format_impl.h"

namespace OHOS {
namespace I18N {
/**
* Obtains plural rules.
*
* @since 2.2
* @version 1.0
*/
class PluralFormat {
public:
    /**
    * @brief A constructor used to create a <b>PluralFormat</b> object based on the specified locale.
    *
    * @param locale Indicates the specified locale.
    * @param status Indicates the result of creating the <b>PluralFormat</b> object. I18nStatus::ISUCCESS
    *               indicates that the operation is successful; I18nStatus::IERROR indicates that the
    *               operation has failed.
    * @since 2.2
    * @version 1.0
    */
    PluralFormat(LocaleInfo &locale, I18nStatus &status);

    /**
    * @brief A destructor used to delete the <b>PluralFormat</b> object.
    *
    * @since 2.2
    * @version 1.0
    */
    virtual ~PluralFormat();

    /**
    * @brief Obtains the index value of the plural rule for the specified number.
    *
    * @param num Indicates the number for which the plural rule is obtained.
    * @param status Indicates the status of the process for obtaining the plural rule.
    * @return Returns the index value of the plural rule; returns <b>-1</b> otherwise.
    * @since 2.2
    * @version 1.0
    */
    int GetPluralRuleIndex(int number, I18nStatus status);

    /**
    * @brief Obtains the index value of the plural rule for the specified decimal number.
    *
    * @param num Indicates the decimal number for which the plural rule is obtained.
    * @param status Indicates the status of the process for obtaining the plural rule.
    * @return Returns the index value of the plural rule; returns <b>-1</b> otherwise.
    * @version 1.0
    */
    int GetPluralRuleIndex(double number, I18nStatus status);
private:
    bool Init();
    PluralFormatImpl *impl = nullptr;
    LocaleInfo mLocale;
};
} // namespace I18N
} // namespace OHOS

#endif