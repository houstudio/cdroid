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

#ifndef OHOS_I18N_WEEK_INFO_H
#define OHOS_I18N_WEEK_INFO_H

#include "locale_info.h"

namespace OHOS {
namespace I18N {
class WeekInfo {
public:
    /**
    * @brief A constructor used to create a <b>Calendar</b> instance with specified locale.
    *
    * @param locale Indicates the specified locale.
    * @param status Indicates the whether the <b>Calendar</b> instances is created correctly.
    */
    WeekInfo(const LocaleInfo &localeInfo, I18nStatus &status);

    /**
    * @brief A destructor used to delete the <b>WeekInfo</b> object.
    */
    virtual ~WeekInfo() = default;

    /**
    * @brief Get the index of the beginning day of a week.
    *
    * @return Returns the index of the beginning day of a week, 1 stands for Sunday and 7 stands for Saturday
    */
    uint8_t GetFirstDayOfWeek();

    /**
    * @brief Get the minimal number of days in the first week of a year.
    *
    * @return Returns the minimal number of days in the first week of a year.
    */
    uint8_t GetMinimalDaysInFirstWeek();

    /**
    * @brief Get the index of the beginning day of weekend.
    *
    * @return Returns the index of the beginning day of weekend, 1 stands for Sunday and 7 stands for Saturday
    */
    uint8_t GetFirstDayOfWeekend();

    /**
    * @brief Get the index of the end day of weekend.
    *
    * @return Returns the index of the end day of weekend, 1 stands for Sunday and 7 stands for Saturday
    */
    uint8_t GetLastDayOfWeekend();
private:
    void Init(I18nStatus &status);
    void ProcessWeekData(const char *data, I18nStatus &status);
    LocaleInfo locale;
    uint8_t firstDayOfWeek;
    uint8_t minimalDaysInFirstWeek;
    uint8_t firstDayOfWeekend;
    uint8_t lastDayOfWeekend;
};
} // namespace I18N
} // namespace OHOS
#endif