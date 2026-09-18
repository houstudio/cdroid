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

#include "i18n_memory_adapter.h"
#include "str_util.h"
#include "types.h"
#include "locale_info.h"

using namespace cdroid::i18n;

const std::set<std::string> LocaleInfo::SCRIPTS = {
    "Hans", "Latn", "Hant", "Qaag", "Cyrl", "Deva", "Guru"
};

void LocaleInfo::Init(const char *newLang, const char *newScript, const char *newRegion, int &status)
{
    id = nullptr;
    status = IERROR;
    if (newLang == nullptr) {
        return;
    }
    int langLength = LenCharArray(newLang);
    // language consists of two or three letters
    if ((langLength > LANGUAGE_MAX_LENGTH) || (langLength < LANGUAGE_MIN_LENGTH)) {
        return;
    }
    I18nFree(static_cast<void *>(language));
    language = NewArrayAndCopy(newLang, langLength);
    if (newScript != nullptr) {
        int scriptLength = LenCharArray(newScript);
        if (scriptLength == SCRIPT_LENGTH) {
            script = NewArrayAndCopy(newScript, scriptLength);
        }
    }
    if (newRegion != nullptr) {
        int regionLength = LenCharArray(newRegion);
        if (regionLength == REGION_LENGTH) {
            region = NewArrayAndCopy(newRegion, regionLength);
        }
    }
    InitIdstr();
    status = ISUCCESS;
}

void LocaleInfo::InitIdstr()
{
    if (language == nullptr) {
        return;
    }
    std::string idStr(language);
    // script consists of four letters
    if ((script != nullptr) && (LenCharArray(script) > 0)) {
        idStr = idStr + "-" + script;
    }
    if ((region != nullptr) && (LenCharArray(region) > 0)) {
        idStr = idStr + "-" + region;
    }
    I18nFree(static_cast<void *>(id));
    id = NewArrayAndCopy(idStr.data(), idStr.size());
}

LocaleInfo::LocaleInfo(const char *newLang, const char *newScript, const char *newRegion)
{
    int status = ISUCCESS;
    Init(newLang, newScript, newRegion, status);
    if (status != ISUCCESS) {
        SetFail();
    }
}

bool LocaleInfo::IsDefaultLocale() const
{
    if ((GetLanguage() == nullptr) || (GetRegion() == nullptr)) {
        return false;
    }
    return ((strcmp(GetLanguage(), "en") == 0) && (strcmp(GetRegion(), "US") == 0));
}

LocaleInfo::LocaleInfo(const char *newLang, const char *newRegion)
{
    int status = ISUCCESS;
    Init(newLang, nullptr, newRegion, status);
    if (status != ISUCCESS) {
        SetFail();
    }
}

LocaleInfo::LocaleInfo()
{
    id = nullptr;
    SetFail();
}

LocaleInfo::LocaleInfo(const LocaleInfo &o)
{
    int status = ISUCCESS;
    Init(o.language, o.script, o.region, status);
    if (status != ISUCCESS) {
        SetFail();
    }
}

LocaleInfo::~LocaleInfo()
{
    FreeResource();
}

void LocaleInfo::FreeResource()
{
    I18nFree(static_cast<void *>(language));
    I18nFree(static_cast<void *>(script));
    I18nFree(static_cast<void *>(region));
    I18nFree(static_cast<void *>(id));
    I18nFree(static_cast<void *>(numberDigits));
}

bool LocaleInfo::operator ==(const LocaleInfo &other) const
{
    bool ret = CompareLocaleItem(language, other.language);
    if (!ret) {
        return false;
    }
    ret = CompareLocaleItem(script, other.script);
    if (!ret) {
        return false;
    }
    ret = CompareLocaleItem(region, other.region);
    return ret;
}

LocaleInfo &LocaleInfo::operator =(const LocaleInfo &o)
{
    if (&o == this) {
        return *this;
    }
    FreeResource();
    if (o.language != nullptr) {
        language = NewArrayAndCopy(o.language, strlen(o.language));
    }
    if (o.script != nullptr) {
        script = NewArrayAndCopy(o.script, strlen(o.script));
    }
    if (o.region != nullptr) {
        region = NewArrayAndCopy(o.region, strlen(o.region));
    }
    if (o.id != nullptr) {
        id = NewArrayAndCopy(o.id, LenCharArray(o.id));
    }
    if (o.numberDigits != nullptr) {
        numberDigits = NewArrayAndCopy(o.numberDigits, LenCharArray(o.numberDigits));
    }
    return *this;
}

const char *LocaleInfo::GetLanguage() const
{
    return language;
}

const char *LocaleInfo::GetScript() const
{
    return script;
}

const char *LocaleInfo::GetRegion() const
{
    return region;
}

const char *LocaleInfo::GetId() const
{
    const char *rid = id;
    return rid;
}

bool LocaleInfo::IsSuccess()
{
    bool r = isSucc;
    isSucc = true;
    return r;
}

void LocaleInfo::SetFail()
{
    isSucc = false;
}

bool LocaleInfo::ChangeLanguageCode(char *lang, const int32_t dstSize, const char *src, const int32_t srcSize) const
{
    if (lang == nullptr || src == nullptr) {
        return false;
    }
    if (srcSize == (LANGUAGE_MIN_LENGTH + 1)) { // three letter language only support fil and mai
        if (memcmp(src, "fil", srcSize) == 0) {
            lang[0] = 't';
            lang[1] = 'l';
        } else if (memcmp(src, "mai", srcSize) == 0) {
            lang[0] = 'm';
            lang[1] = 'd';
        } else {
            return false;
        }
        return true;
    } else if (srcSize == LANGUAGE_MIN_LENGTH) {
        if (memcmp(src, "he", srcSize) == 0) {
            lang[0] = 'i';
            lang[1] = 'w';
        } else if (memcmp(src, "id", srcSize) == 0) {
            lang[0] = 'i';
            lang[1] = 'n';
        } else {
            strncpy(lang, src,dstSize);
        }
        return true;
    }
    return false;
}

uint32_t LocaleInfo::GetMask() const
{
    if (language == nullptr) {
        return 0;
    }
    char lang[LANGUAGE_MAX_LENGTH];
    bool isRight = ChangeLanguageCode(lang, LANGUAGE_MAX_LENGTH, language, LenCharArray(language));
    if (!isRight) {
        return 0;
    }
    // use 7bit to represent an English letter,
    // 32--- language ---18--- script ---14--- region ---0
    uint32_t tempLangFirst = (lang[0] - CHAR_OFF);
    uint32_t tempLangSecond = (lang[1] - CHAR_OFF);
    uint32_t mask = (tempLangFirst << LANG_FIRST_BEGIN) | (tempLangSecond << LANG_SECOND_BEGIN);
    if ((script != nullptr) && (LenCharArray(script) > 0)) {
        if (strcmp(script, "Hans") == 0) {
            mask = mask | (HANS << SCRIPT_BEGIN);
        } else if (strcmp(script, "Hant") == 0) {
            mask = mask | (HANT << SCRIPT_BEGIN);
        } else if (strcmp(script, "Latn") == 0) {
            mask = mask | (LATN << SCRIPT_BEGIN);
        } else if (strcmp(script, "Qaag") == 0) {
            mask = mask | (QAAG << SCRIPT_BEGIN);
        } else if (strcmp(script, "Cyrl") == 0) {
            mask = mask | (CYRL << SCRIPT_BEGIN);
        } else if (strcmp(script, "Deva") == 0) {
            mask = mask | (DEVA << SCRIPT_BEGIN);
        } else if (strcmp(script, "Guru") == 0) {
            mask = mask | (GURU << SCRIPT_BEGIN);
        }
    }
    if ((region != nullptr) && (LenCharArray(region) > 1)) {
        uint32_t tempRegion = (region[0] - CHAR_OFF);
        uint32_t tempRegionSecond = (region[1] - CHAR_OFF);
        mask = mask | (tempRegion << REGION_FIRST_LETTER) | (tempRegionSecond);
    }
    return mask;
}

LocaleInfo LocaleInfo::ForLanguageTag(const char *languageTag, I18nStatus &status)
{
    LocaleInfo locale;
    if (languageTag == nullptr) {
        status = IERROR;
        return locale;
    }
    ParseLanguageTag(locale, languageTag, status);
    locale.InitIdstr();
    return locale;
}

void LocaleInfo::ParseLanguageTag(LocaleInfo &locale, const char *languageTag, I18nStatus &status)
{
    const char *tag = languageTag;
    uint16_t options = OPT_LANG;
    const char *key = nullptr;
    const char *value = nullptr;
    uint8_t type = 0;
    while (tag) {
        const char *start = tag;
        const char *end = tag;
        while (*end) {
            if (*end == '-') {
                break;
            }
            ++end;
        }
        tag = end + 1;
        if (*end == '\0') {
            tag = nullptr;
        }
        auto tagLength = end - start;
        ConfirmTagType(start, tagLength, type, key, value);
        if (!ParseNormalSubTag(locale, start, tagLength, options, type)) {
            if ((options & OPT_EXTENSION) && (type == TAG_VALUE)) {
                ProcessExtension(locale, key, value);
                type = TAG_COMMON;
            }
        }
    }
    I18nFree(static_cast<void *>(const_cast<char *>(key)));
    I18nFree(static_cast<void *>(const_cast<char *>(value)));
}

bool LocaleInfo::ParseNormalSubTag(LocaleInfo &locale, const char *start, size_t tagLength, uint16_t &options,
    uint8_t &type)
{
    if ((start == nullptr) || (tagLength == 0)) {
        return false;
    }
    if ((options & OPT_LANG) && (type == TAG_COMMON)) {
        if (IsLanguage(start, tagLength)) {
            locale.language = I18nNewCharString(start, tagLength);
            options &= ~OPT_LANG;
            options |= OPT_SCRIPT | OPT_REGION | OPT_EXTENSION;
            return true;
        }
    }
    if ((options & OPT_SCRIPT) && (type == TAG_COMMON)) {
        if (IsScript(start, tagLength)) {
            options &= ~OPT_SCRIPT;
            locale.script = I18nNewCharString(start, tagLength);
            return true;
        }
    }
    if ((options & OPT_REGION) && (type == TAG_COMMON)) {
        if (IsRegion(start, tagLength)) {
            options &= ~OPT_REGION;
            options &= ~OPT_SCRIPT;
            locale.region = I18nNewCharString(start, tagLength);
            return true;
        }
    }
    return false;
}

void LocaleInfo::ConfirmTagType(const char *start, size_t length, uint8_t &type, const char* &key, const char* &value)
{
    if (start == nullptr) {
        return;
    }
    switch (type) {
        case TAG_COMMON: {
            if ((length == 1) && (*start == 'u')) {
                type = TAG_U;
            }
            return;
        }
        case TAG_U: {
            type = TAG_KEY;
            I18nFree(static_cast<void *>(const_cast<char *>(key)));
            key = I18nNewCharString(start, length);
            return;
        }
        case TAG_KEY: {
            type = TAG_VALUE;
            I18nFree(static_cast<void *>(const_cast<char *>(value)));
            value = I18nNewCharString(start, length);
            return;
        }
        default: {
            type = TAG_COMMON;
            return;
        }
    }
}

void LocaleInfo::ProcessExtension(LocaleInfo &locale, const char *key, const char *value)
{
    if (key == nullptr || value == nullptr) {
        return;
    }
    // now we only support numbering systems in extensions
    if (strcmp(key, "nu") == 0) {
        locale.numberDigits = NewArrayAndCopy(value, strlen(value));
        return;
    }
}

bool LocaleInfo::IsLanguage(const char *start, uint8_t length)
{
    if ((length != LANGUAGE_MAX_LENGTH) && (length != LANGUAGE_MIN_LENGTH)) {
        return false;
    }
    for (uint8_t i = 0; i < length; ++i) {
        const char ch = *(start + i);
        if (ch < 'a' || ch > 'z') {
            return false;
        }
    }
    return true;
}

bool LocaleInfo::IsScript(const char *start, uint8_t length)
{
    // all scripts's length is 4,
    // now we support Latn, Hans, Hant, Qaag, Cyrl, Deva, Guru
    if (length != SCRIPT_LENGTH || start == nullptr) {
        return false;
    }
    if (SCRIPTS.find(std::string(start, length)) != SCRIPTS.end()) {
        return true;
    } else {
        return false;
    }
}

bool LocaleInfo::IsRegion(const char *start, uint8_t length)
{
    if (length != REGION_LENGTH) {
        return false;
    }
    for (uint8_t i = 0; i < length; ++i) {
        const char ch = *(start + i);
        if (ch < 'A' || ch > 'Z') { // region characters should all be upper case.
            return false;
        }
    }
    return true;
}

const char *LocaleInfo::GetExtension(const char *key)
{
    if (strcmp(key, "nu") == 0) {
        return numberDigits;
    }
    return nullptr;
}
