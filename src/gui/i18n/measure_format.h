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

#ifndef MEASUREFORMAT_H
#define MEASUREFORMAT_H
/**
 * @addtogroup I18N
 * @{
 *
 * @brief Provides functions related to internationalization (i18n), with which you can format measure.
 *
 * @since 2.2
 * @version 1.0
 */

/**
 * @file measure_format.h
 *
 * @brief Declares functions for formatting measures.
 *
 * Example code: \n
 * Creating a <b>LocaleInfo</b> instance: \n
 *      {@code LocaleInfo locale("en", "Latn", "");}
 * Creating a <b>MeasureFormat</b> instance: \n
 * {@code
 * int status = 0;
 * MeasureFormat formatter(locale, status);
 * Formatting data: \n
 * {@code
 * std::string out = formatter.Format(12, "kcal", MeasureType::MEDIUM);}
 * Output: \n
 *     12 kcal
 *
 * @since 2.2
 * @version 1.0
 */

#include "locale_info.h"
#include "measure_format_impl.h"
#include "string.h"
#include "types.h"

namespace OHOS {
namespace I18N {
class MeasureFormat {
public:
    /**
     * @brief A constructor used to create a <b>MeasureFormat</b> instance with specified locale information.
     *
     * @param localeinfo Indicates the specified locale information.
     * @param status Specifies whether a <b>MeasureFormat</b> instance is created.
     *   The value <b>0</b> indicates that a <b>NumberFormat</b> instance is created,
     *   and the value <b>1</b> indicates the opposite case.
     * @since 2.2
     * @version 1.0
     */
    MeasureFormat(LocaleInfo &localeinfo, I18nStatus &status);

    /**
     * @brief A destructor used to delete the <b>MeasureFormat</b> instance.
     *
     * @since 2.2
     * @version 1.0
     */
    virtual ~MeasureFormat();

    /**
     * @brief Formats a measure.
     *
     * @param num Indicates the int number to format.
     * @param unit Indicates the measure unit to format.
     * @param status Specifies whether the formatting is successful.
     *   The value <b>0</b> indicates that the formatting is successful,
     *   and <b>1</b> indicates that the formatting fails.
     * @param type Indicates the type the int number is formatted into.
     *   The value can be <b>SHORT</b>, <b>MEDIUM</b>, <b>LONG</b>, <b>FULL</b>.
     * @return Returns a string representation of the formatted measure.
     * @since 2.2
     * @version 1.0
     */
    std::string Format(int num, std::string unit, I18nStatus &status,
        MeasureFormatType type = MeasureFormatType::MEASURE_SHORT);

    /**
    * @brief Formats a number with measure.
    *
    * @param num Indicates the double number to format.
    * @param unit Indicates the measure unit to format.
    * @param status Specifies whether the formatting is successful.
    *   The value <b>0</b> indicates that the formatting is successful,
    *   and <b>1</b> indicates that the formatting fails.
    * @param type Indicates the type the double number is formatted into.
    *   The value can be <b>SHORT</b>, <b>MEDIUM</b>, <b>LONG</b>, <b>FULL</b>.
    * @return Returns a string representation of the formatted measure.
    * @since 2.2
    * @version 1.0
    */
    std::string Format(double num, std::string unit, I18nStatus &status,
        MeasureFormatType type = MeasureFormatType::MEASURE_SHORT);

    /**
    * @brief Performs an initialization to load data.
    *
    * @return Returns <b>true</b> if the initialization is successful; returns <b>false</b> otherwise.
    * @since 2.2
    * @version 1.0
    */
    bool Init();

private:
    bool ReInitImpl();
    MeasureFormatImpl *measureFormatImpl = nullptr;
    LocaleInfo locale;
};
} // namespace I18N
} // namespace OHOS
/** @} */
#endif