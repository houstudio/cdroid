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

#ifndef LOCINFO_H
#define LOCINFO_H

/**
* @addtogroup i18n
* @{
*
* @brief Provides functions related to internationalization (i18n), with which you can format date, time and numbers.
*
* @since 2.2
* @version 1.0
*/

/**
* @file locale_info.h
*
* @brief Declares functions for obtaining locale information, including language, script, and country/region.
*
* Example code: \n
* Creating a <b>LocaleInfo</b> instance: \n
*      {@code LocaleInfo locale("zh", "Hans", "CN");}
* Obtaining the language: \n
*      {@code const char *language = locale.GetLanguage();}
* Output: \n
*     zh
*
* @since 2.2
* @version 1.0
*/

#include <cstdint>
#include <string>
#include <set>
#include "types.h"

namespace cdroid {
namespace i18n {
class LocaleInfo {
public:
    /**
    * @brief A constructor used to create a <b>LocaleInfo</b> instance with specified language,
    *   script, and country/region.
    *
    * @param lang Indicates the pointer to the specified language.
    * @param script Indicates the pointer to the specified script.
    * @param region Indicates the pointer to the specified country/region.
    * @since 2.2
    * @version 1.0
    */
    LocaleInfo(const char *lang, const char *script, const char *region);

    /**
    * @brief A constructor used to create a <b>LocaleInfo</b> instance with specified language and country/region.
    *
    * @param lang Indicates the pointer to the specified language.
    * @param region Indicates the pointer to the specified country/region.
    * @since 2.2
    * @version 1.0
    */
    LocaleInfo(const char *lang, const char *region);

    /**
    * @brief A constructor used to create a <b>LocaleInfo</b> instance by copying a specified one.
    *
    * @param locale Indicates the specified <b>LocaleInfo</b> instance.
    * @since 2.2
    * @version 1.0
    */
    LocaleInfo(const LocaleInfo& locale);

    /**
    * @brief Default constructor used to create a <b>LocaleInfo</b> instance.
    *
    * @since 2.2
    * @version 1.0
    */
    LocaleInfo();

    /**
    * @brief A destructor used to delete the <b>LocaleInfo</b> instance.
    *
    * @since 2.2
    * @version 1.0
    */
    virtual ~LocaleInfo();

    /**
    * @brief Checks whether this <b>LocaleInfo</b> object equals a specified one.
    *
    * @param other Indicates the <b>LocaleInfo</b> object to compare.
    * @return Returns <b>true</b> if the two objects are equal; returns <b>false</b> otherwise.
    * @since 2.2
    * @version 1.0
    */
    virtual bool operator ==(const LocaleInfo &other) const;

    /**
    * @brief Creates a new <b>LocaleInfo</b> object based on a specified one.
    *
    * @param other Indicates the specified <b>LocaleInfo</b> object.
    * @return Returns the new <b>LocaleInfo</b> object.
    * @since 2.2
    * @version 1.0
    */
    virtual LocaleInfo &operator =(const LocaleInfo &other);

    /**
    * @brief Obtains the ID of this <b>LocaleInfo</b> object, which consists of the language,
    *   script, and country/region.
    *
    * @return Returns the ID.
    * @since 2.2
    * @version 1.0
    */
    const char *GetId() const;

    /**
    * @brief Obtains the language specified in this <b>LocaleInfo</b> object.
    *
    * @return Returns the language.
    * @since 2.2
    * @version 1.0
    */
    const char *GetLanguage() const;

    /**
    * @brief Obtains the script specified in this <b>LocaleInfo</b> object.
    *
    * @return Returns the script.
    * @since 2.2
    * @version 1.0
    */
    const char *GetScript() const;

    /**
    * @brief Obtains the country/region specified in this <b>LocaleInfo</b> object.
    *
    * @return Returns the country/region.
    * @since 2.2
    * @version 1.0
    */
    const char *GetRegion() const;

    /**
    * @brief Obtains the mask of this <b>LocaleInfo</b> object.
    *
    * @return Returns the mask.
    * @since 2.2
    * @version 1.0
    */
    uint32_t GetMask() const;

    /**
    * @brief Checks whether this <b>LocaleInfo</b> object represents the default locale (en-US).
    *
    * @return Returns <b>true</b> if the <b>LocaleInfo</b> object represents the default locale;
    *   returns <b>false</b> otherwise.
    * @since 2.2
    * @version 1.0
    */
    bool IsDefaultLocale() const;

    /**
    * @brief Parse a language tag, and returns an associated <b>LocaleInfo</b> instance.
    *
    * @param languageTag Indicates the language tag, which is to be parsed.
    * @param status Indicates the status of the creating process.
    * @return Returns the associated LocaleInfo instances.
    */
    static LocaleInfo ForLanguageTag(const char *languageTag, I18nStatus &status);

    /**
    * @brief Get extension subtag associated with the key.
    *
    * @param key Get the extension subtag using the key.
    * @return Returns the subtag
    */
    const char *GetExtension(const char *key);
private:
    bool ChangeLanguageCode(char *lang, const int32_t dstSize, const char *src, const int32_t srcSize) const;
    void FreeResource();
    static void ProcessExtension(LocaleInfo &locale, const char *key, const char *value);
    static void ConfirmTagType(const char *start, size_t length, uint8_t &type, const char* &key, const char* &value);
    static void ParseLanguageTag(LocaleInfo &locale, const char *languageTag, I18nStatus &status);
    static bool ParseNormalSubTag(LocaleInfo &locale, const char *start, size_t tagLength, uint16_t &options,
        uint8_t &type);
    static bool IsLanguage(const char *start, uint8_t length);
    static bool IsScript(const char *start, uint8_t length);
    static bool IsRegion(const char *start, uint8_t length);
    void InitIdstr();
    char *language = nullptr;
    char *script = nullptr;
    char *region = nullptr;
    char *id = nullptr;
    char *numberDigits = nullptr;
    bool isSucc = true;
    bool IsSuccess();
    void SetFail();
    void Init(const char *lang, const char *script, const char *region, int &status);
    const int CHAR_OFF = 48;
    static const std::set<std::string> SCRIPTS;
    static constexpr uint16_t OPT_LANG = 0x0001;
    static constexpr uint16_t OPT_SCRIPT = 0x0002;
    static constexpr uint16_t OPT_REGION = 0x0004;
    static constexpr uint16_t OPT_EXTENSION = 0x0008;
    static constexpr uint8_t TAG_COMMON = 0;
    static constexpr uint8_t TAG_U = 1;
    static constexpr uint8_t TAG_KEY = 2;
    static constexpr uint8_t TAG_VALUE = 3;
    static constexpr int LANGUAGE_MIN_LENGTH = 2;
    static constexpr int LANGUAGE_MAX_LENGTH = 3;
    static constexpr int REGION_LENGTH = 2;
    static constexpr int SCRIPT_LENGTH = 4;
};

enum ESupportScript {
    NOKOWN = 0x0,
    LATN = 0x1,
    HANS = 0x2,
    HANT = 0x3,
    QAAG = 0x4,
    CYRL = 0x5,
    DEVA = 0x6,
    GURU = 0x7
};

enum EMask {
    REGION_FIRST_LETTER = 7,
    SCRIPT_BEGIN = 14,
    LANG_SECOND_BEGIN = 18,
    LANG_FIRST_BEGIN = 25
};
} // namespace i18n
} // namespace cdroid
/** @} */
#endif
