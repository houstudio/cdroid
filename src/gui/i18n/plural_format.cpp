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

#include "plural_format.h"
#include "plural_format_impl.h"

using namespace OHOS::I18N;

PluralFormat::PluralFormat(LocaleInfo &locale, I18nStatus &status)
{
    if (locale.GetId() == nullptr) {
        status = IERROR;
        return;
    }
    mLocale = locale;
}

PluralFormat::~PluralFormat()
{
    if (impl != nullptr) {
        delete impl;
        impl = nullptr;
    }
}

bool PluralFormat::Init()
{
    if (impl != nullptr) {
        delete impl;
    }
    I18nStatus status = I18nStatus::ISUCCESS;
    impl = new PluralFormatImpl(mLocale, status);
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

int PluralFormat::GetPluralRuleIndex(int number, I18nStatus status)
{
    if (impl == nullptr) {
        bool isSuccess = Init();
        if (!isSuccess) {
            return -1;
        }
    }
    return impl->GetPluralRuleIndex(number, status);
}

int PluralFormat::GetPluralRuleIndex(double number, I18nStatus status)
{
    if (impl == nullptr) {
        bool isSuccess = Init();
        if (!isSuccess) {
            return -1;
        }
    }
    return impl->GetPluralRuleIndex(number, status);
}