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

#include "measure_format.h"

using namespace OHOS::I18N;

MeasureFormat::MeasureFormat(LocaleInfo &localeinfo, I18nStatus &status)
{
    if (localeinfo.GetId() == nullptr) {
        status = I18nStatus::IERROR;
        return;
    }
    locale = localeinfo;
}

bool MeasureFormat::Init()
{
    I18nStatus status = I18nStatus::ISUCCESS;
    if (measureFormatImpl != nullptr) {
        delete measureFormatImpl;
    }
    measureFormatImpl = new MeasureFormatImpl(locale, status);
    if (measureFormatImpl == nullptr || status != I18nStatus::ISUCCESS) {
        return false;
    }
    DataResource resource(&locale);
    bool isSuccess = resource.Init();
    if (!isSuccess) {
        return false;
    }
    return measureFormatImpl->Init(resource);
}

MeasureFormat::~MeasureFormat()
{
    if (measureFormatImpl != nullptr) {
        delete measureFormatImpl;
        measureFormatImpl = nullptr;
    }
}

std::string MeasureFormat::Format(double num, std::string unit, I18nStatus &status, MeasureFormatType type)
{
    if (!ReInitImpl()) {
        status = I18nStatus::IERROR;
        return "";
    }
    return measureFormatImpl->Format(num, unit, type, status);
}

std::string MeasureFormat::Format(int num, std::string unit, I18nStatus &status, MeasureFormatType type)
{
    if (!ReInitImpl()) {
        status = I18nStatus::IERROR;
        return "";
    }
    return measureFormatImpl->Format(num, unit, type, status);
}

bool MeasureFormat::ReInitImpl()
{
    if (measureFormatImpl == nullptr) {
        bool isSuccess = Init();
        if (!isSuccess) {
            return false;
        }
    }
    return true;
}