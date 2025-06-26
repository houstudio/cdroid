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

#include "date_time_format.h"
#include "data_resource.h"
#include "date_time_format_impl.h"
#include "number_format_impl.h"

using namespace OHOS::I18N;
using namespace std;

/**
 * construct a DateTimeFormat object with request pattern and locale.
 * now we only support patterns defined in AvailableDateTimeFormatPatterns.
 * locale, locale information to retrieve datetime resource form icu data.
 */
DateTimeFormat::DateTimeFormat(AvailableDateTimeFormatPattern requestPattern, const LocaleInfo &locale)
{
    this->locale = locale;
    this->requestPattern = requestPattern;
}

DateTimeFormat::~DateTimeFormat()
{
    if (impl != nullptr) {
        delete impl;
        impl = nullptr;
    }
}

bool DateTimeFormat::Init()
{
    if (impl != nullptr) {
        delete impl;
        impl = nullptr;
    }
    impl = new(nothrow) DateTimeFormatImpl(requestPattern, locale);
    if (impl == nullptr) {
        return false;
    }
    DataResource resource(&locale);
    bool isSuccess = resource.Init();
    if (!isSuccess) {
        return false;
    }
    isSuccess = impl->Init(resource);
    if (!isSuccess) {
        return false;
    }
    int status = I18nStatus::ISUCCESS;
    NumberFormatImpl *numberFormatter = new(nothrow) NumberFormatImpl(locale, status);
    if (numberFormatter == nullptr) {
        delete impl;
        impl = nullptr;
        return false;
    }
    if (status != I18nStatus::ISUCCESS) {
        delete numberFormatter;
        numberFormatter = nullptr;
        delete impl;
        impl = nullptr;
        return false;
    }
    isSuccess = numberFormatter->Init(resource);
    if (!isSuccess) {
        delete numberFormatter;
        numberFormatter = nullptr;
        delete impl;
        impl = nullptr;
        return false;
    }
    impl->SetNumberFormatter(numberFormatter);
    return true;
}

/**
 * parse a time (represent by the seconds elapsed from UTC 1970, January 1 00:00:00) to its text format.
 * cal, seconds from from UTC 1970, January 1 00:00:00
 * zoneInfoOffest, string representation of offset such as "+01:45"
 * appendTo, output of this method.
 */
void DateTimeFormat::Format(const time_t &cal, const string &zoneInfo, string &appendTo, I18nStatus &status)
{
    if (impl == nullptr) {
        bool isSuccess = Init();
        if (!isSuccess) {
            status = IERROR;
            return;
        }
    }
    impl->Format(cal, zoneInfo, appendTo, status);
}

void DateTimeFormat::ApplyPattern(const AvailableDateTimeFormatPattern &requestPattern)
{
    if (this->requestPattern == requestPattern) {
        return;
    }
    this->requestPattern = requestPattern;
    if (this->impl != nullptr) {
        this->impl->ApplyPattern(requestPattern);
    } else {
        Init();
    }
}

std::string DateTimeFormat::GetWeekName(const int32_t &index, DateTimeDataType type)
{
    if (impl == nullptr) {
        bool isSuccess = Init();
        if (!isSuccess) {
            return "";
        }
    }
    return impl->GetWeekName(index, type);
}

std::string DateTimeFormat::GetMonthName(const int32_t &index, DateTimeDataType type)
{
    if (impl == nullptr) {
        bool isSuccess = Init();
        if (!isSuccess) {
            return "";
        }
    }
    return impl->GetMonthName(index, type);
}

std::string DateTimeFormat::GetAmPmMarker(const int32_t &index, DateTimeDataType type)
{
    if (impl == nullptr) {
        bool isSuccess = Init();
        if (!isSuccess) {
            return "";
        }
    }
    return impl->GetAmPmMarker(index, type);
}

int8_t DateTimeFormat::Get12HourTimeWithoutAmpm(const time_t &cal, const std::string &zoneInfo,
    std::string &appendTo, I18nStatus &status)
{
    if (impl == nullptr) {
        bool isSuccess = Init();
        if (!isSuccess) {
            status = IERROR;
            return 0;
        }
    }
    return impl->Get12HourTimeWithoutAmpm(cal, zoneInfo, appendTo, status);
}

std::string DateTimeFormat::FormatElapsedDuration(int32_t milliseconds, ElapsedPatternType type,
    I18nStatus &status)
{
    if (impl == nullptr) {
        bool isSuccess = Init();
        if (!isSuccess) {
            status = IERROR;
            return "";
        }
    }
    return impl->FormatElapsedDuration(milliseconds, type, status);
}

std::string DateTimeFormat::GetTimeSeparator()
{
    if (impl == nullptr) {
        bool isSuccess = Init();
        if (!isSuccess) {
            return "";
        }
    }
    return impl->GetTimeSeparator();
}