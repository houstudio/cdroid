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

#ifndef NUMBERFORMAT_H
#define NUMBERFORMAT_H

/**
* @addtogroup I18N
* @{
*
* @brief Provides functions related to internationalization (i18n), with which you can format date, time and numbers.
*
* @since 2.2
* @version 1.0
*/

/**
* @file number_format.h
*
* @brief Declares functions for formatting integers and double numbers.
*
* Example code: \n
* Creating a <b>LocaleInfo</b> instance: \n
*      {@code LocaleInfo locale("en", "US");}
* Creating a <b>NumberFormat</b> instance: \n
* {@code
* int status = 0;
* NumberFormat formatter(locale, status);
* Formatting data: \n
* {@code
* int num = 1234
* std::string out = formatter.Format(num, status);}
* Output: \n
*     1,234
*
* @since 2.2
* @version 1.0
*/

#include "types.h"
#include "securec.h"
#include "number_format_impl.h"
#include "locale_info.h"

namespace OHOS {
namespace I18N {
class NumberFormat {
public:
    /**
    * @brief A constructor used to create a <b>NumberFormat</b> instance with specified locale information.
    *
    * @param status Specifies whether a <b>NumberFormat</b> instance is created.
    *   The value <b>0</b> indicates that a <b>NumberFormat</b> instance is created,
    *   and the value <b>1</b> indicates the opposite case.
    * @param locale Indicates the specified locale information.
    * @since 2.2
    * @version 1.0
    */
    NumberFormat(LocaleInfo &locale, int &status);

    /**
    * @brief A destructor used to delete the <b>NumberFormat</b> instance.
    *
    * @since 2.2
    * @version 1.0
    */
    virtual ~NumberFormat();

    /**
    * @brief Formats a double number.
    *
    * @param num Indicates the double number to format.
    * @param type Indicates the type the double number is formatted into.
    *   The value can be <b>DECIMAL</b> or <b>PERCENT</b>.
    * @param status Specifies whether the formatting is successful.
    *   The value <b>0</b> indicates that the formatting is successful,
    *   and <b>1</b> indicates that the formatting fails.
    * @return Returns a string representation of the formatted double number.
    * @since 2.2
    * @version 1.0
    */
    std::string Format(double num, NumberFormatType type, int &status);

    /**
    * @brief Formats an integer.
    *
    * @param num Indicates the integer to format.
    * @param status Specifies whether the formatting is successful.
    *   The value <b>0</b> indicates that the formatting is successful,
    *   and <b>1</b> indicates that the formatting fails.
    * @return Returns a string representation of the formatted integer.
    * @since 2.2
    * @version 1.0
    */
    std::string Format(int num, int &status);

    /**
    * @brief Formats a double number without grouping its integer part.
    *
    * @param num Indicates the double number to format.
    * @param type Indicates the type the double number is formatted into.
    *   The value can be <b>DECIMAL</b> or <b>PERCENT</b>.
    * @param status Specifies whether the formatting is successful. The value <b>0</b>
    *   indicates that the formatting is successful, and <b>1</b> indicates that the formatting fails.
    * @return Returns a string representation of the formatted double number.
    * @since 2.2
    * @version 1.0
    */
    std::string FormatNoGroup(double num, NumberFormatType type, int &status);

    /**
    * @brief Formats an integer without grouping.
    *
    * @param num Indicates the integer to format.
    * @param status Specifies whether the formatting is successful. The value <b>0</b> indicates that
    *   the formatting is successful, and <b>1</b> indicates that the formatting fails.
    * @return Returns a string representation of the formatted double integer.
    * @since 2.2
    * @version 1.0
    */
    std::string FormatNoGroup(int num, int &status);

    /**
    * @brief Performs an initialization to load data.
    *
    * @return Returns <b>true</b> if the initialization is successful; returns <b>false</b> otherwise.
    * @since 2.2
    * @version 1.0
    */
    bool Init();

    /**
    * @brief Sets the maximum length for the decimal part of a double number. The excess part will be truncated.
    *
    * @param length Indicates the maximum length to set.
    * @return Returns <b>true</b> if the setting is successful; returns <b>false</b> otherwise.
    * @since 2.2
    * @version 1.0
    */
    bool SetMaxDecimalLength(int length);

    /**
    * @brief Sets the minimum length for the decimal part of a double number.
    *   Zero padding is required if the minimum length is not reached.
    *
    * @param length Indicates the minimum length to set.
    * @return Returns <b>true</b> if the setting is successful; returns <b>false</b> otherwise.
    * @since 2.2
    * @version 1.0
    */
    bool SetMinDecimalLength(int length);
private:
    bool ReInitImpl();
    NumberFormatImpl *impl = nullptr;
    LocaleInfo mLocale;
};
} // namespace I18N
} // namespace OHOS
/** @} */
#endif