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

#include "number_format.h"
#include "number_format_impl.h"

using namespace OHOS::I18N;

NumberFormat::NumberFormat(LocaleInfo &locale, int &status)
{
    if (locale.GetId() == nullptr) {
        status = IERROR;
        return;
    }
    mLocale = locale;
}

bool NumberFormat::Init()
{
    int status = I18nStatus::ISUCCESS;
    if (impl != nullptr) {
        delete impl;
    }
    impl = new NumberFormatImpl(mLocale, status);
    if (impl == nullptr) {
        return false;
    }
    if (status != I18nStatus::ISUCCESS) {
        return false;
    }
    DataResource resource(&mLocale);
    bool isSuccess = resource.Init();
    if (!isSuccess) {
        return false;
    }
    return impl->Init(resource);
}


NumberFormat::~NumberFormat()
{
    if (impl != nullptr) {
        delete impl;
        impl = nullptr;
    }
}


std::string NumberFormat::Format(double num, NumberFormatType type, int &status)
{
    if (!ReInitImpl()) {
        return "";
    }
    return impl->Format(num, type, status);
}

std::string NumberFormat::Format(int num, int &status)
{
    if (!ReInitImpl()) {
        return "";
    }
    return impl->Format(num, status);
}

std::string NumberFormat::FormatNoGroup(double num, NumberFormatType type, int &status)
{
    if (!ReInitImpl()) {
        return "";
    }
    return impl->FormatNoGroup(num, type, status);
}

std::string NumberFormat::FormatNoGroup(int num, int &status)
{
    if (!ReInitImpl()) {
        return "";
    }
    return impl->FormatNoGroup(num, status);
}

bool NumberFormat::SetMaxDecimalLength(int length)
{
    if (!ReInitImpl()) {
        return false;
    }
    return impl->SetMaxDecimalLength(length);
}

bool NumberFormat::SetMinDecimalLength(int length)
{
    if (!ReInitImpl()) {
        return false;
    }
    return impl->SetMinDecimalLength(length);
}

bool NumberFormat::ReInitImpl()
{
    if (impl == nullptr) {
        bool isSuccess = Init();
        if (!isSuccess) {
            return false;
        }
    }
    return true;
}