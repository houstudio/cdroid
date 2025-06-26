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

#include "week_info.h"
#include <cstring>
#include "data_resource.h"

using namespace OHOS::I18N;

static constexpr uint8_t FIRST_DAY_OF_WEEK_INDEX = 0;
static constexpr uint8_t MINIMAL_DAYS_INFIRST_WEEK_INDEX = 1;
static constexpr uint8_t WEEKEND_ONSET_INDEX = 2;
static constexpr uint8_t WEEKEND_CEASE_INDEX = 3;
static constexpr uint8_t WEEK_DATA_LAST_INDEX = 4;
static constexpr uint8_t SINGLE_DATA_LENGTH = 2;

WeekInfo::WeekInfo(const LocaleInfo &localeInfo, I18nStatus &status)
{
    locale = localeInfo;
    Init(status);
}

void WeekInfo::Init(I18nStatus &status)
{
    DataResource resource(&locale);
    bool isSuccess = resource.Init();
    if (!isSuccess) {
        status = IERROR;
        return;
    }
    char *weekData = resource.GetString(WEEK_DATA);
    if (weekData == nullptr) {
        status = IERROR;
        return;
    }
    ProcessWeekData(weekData, status);
}

void WeekInfo::ProcessWeekData(const char *data, I18nStatus &status)
{
    if (data == nullptr) {
        status = IERROR;
        return;
    }
    size_t length = strlen(data);
    if (length != SINGLE_DATA_LENGTH * WEEK_DATA_LAST_INDEX - 1) {
        status = IERROR;
        return;
    }
    firstDayOfWeek = static_cast<uint8_t>(data[FIRST_DAY_OF_WEEK_INDEX * SINGLE_DATA_LENGTH]) - '0';
    minimalDaysInFirstWeek = static_cast<uint8_t>(data[MINIMAL_DAYS_INFIRST_WEEK_INDEX * SINGLE_DATA_LENGTH]) - '0';
    firstDayOfWeekend = static_cast<uint8_t>(data[WEEKEND_ONSET_INDEX * SINGLE_DATA_LENGTH]) - '0';
    lastDayOfWeekend = static_cast<uint8_t>(data[WEEKEND_CEASE_INDEX * SINGLE_DATA_LENGTH]) - '0';
}

uint8_t WeekInfo::GetFirstDayOfWeek()
{
    return firstDayOfWeek;
}

uint8_t WeekInfo::GetMinimalDaysInFirstWeek()
{
    return minimalDaysInFirstWeek;
}

uint8_t WeekInfo::GetFirstDayOfWeekend()
{
    return firstDayOfWeekend;
}

uint8_t WeekInfo::GetLastDayOfWeekend()
{
    return lastDayOfWeekend;
}
