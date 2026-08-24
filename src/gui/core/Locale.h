/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *********************************************************************************/

/*
 * Ported to C++ for CDROID from java.util.Locale (ojluni / android-36).
 *
 * The BCP-47 core: language / script / country / variant with Android's case
 * normalization, language-tag round-trip, the well-known Locale constants and
 * the process-wide default. NOT ported (upstream leans on ICU):
 *   - Unicode locale extensions / keywords ("-u-ca-buddhist", "-x-…") — parsed
 *     tags carrying extensions keep language/script/region and drop the rest.
 *   - Locale.Builder / Locale.Category(FORMAT/DISPLAY) variants.
 *   - ISO3 conversions (getISO3Language/getISO3Country) — need the ISO tables.
 * The empty Locale (all fields "") is Java's Locale.ROOT; get() out-of-range
 * and not-found results use it as the null sentinel (no std::optional).
 *
 * NB: the filename is capitalized (Locale.h, like vendored minikin's own
 * Locale.h/LocaleList.h) because core/ is on the bare include path and a
 * lowercase locale.h would SHADOW glibc's <locale.h> pulled in by libstdc++.
 */
#ifndef CDROID_CORE_LOCALE_H
#define CDROID_CORE_LOCALE_H

#include <string>

namespace cdroid {

class Locale {
public:
    // ---- Java's well-known constants -------------------------------------
    static const Locale ROOT;                 // ""       (the null-ish empty Locale)
    static const Locale ENGLISH;              // en
    static const Locale FRENCH;               // fr
    static const Locale GERMAN;               // de
    static const Locale ITALIAN;              // it
    static const Locale JAPANESE;             // ja
    static const Locale KOREAN;               // ko
    static const Locale CHINESE;              // zh
    static const Locale SIMPLIFIED_CHINESE;   // zh-Hans-CN
    static const Locale TRADITIONAL_CHINESE;  // zh-Hant-TW
    static const Locale FRANCE;               // fr-FR
    static const Locale GERMANY;              // de-DE
    static const Locale ITALY;                // it-IT
    static const Locale JAPAN;                // ja-JP
    static const Locale KOREA;                // ko-KR
    static const Locale CHINA;                // zh-Hans-CN
    static const Locale PRC;                  // zh-Hans-CN
    static const Locale TAIWAN;               // zh-Hant-TW
    static const Locale UK;                   // en-GB
    static const Locale US;                   // en-US
    static const Locale CANADA;               // en-CA
    static const Locale CANADA_FRENCH;        // fr-CA

    // Construct from (deprecated-in-Java) language/country/variant. Script is
    // only set by forLanguageTag()/explicit assignment (Java needs the Builder).
    Locale();
    explicit Locale(const std::string& language);
    Locale(const std::string& language, const std::string& country);
    Locale(const std::string& language, const std::string& country, const std::string& variant);

    // ---- process-wide default (Java: user.language/user.region) ----------
    static Locale getDefault();
    static void setDefault(const Locale& locale);

    // Parse a BCP-47 tag ("zh-Hant-TW"). Malformed subtags are skipped; the
    // "und"/empty language yields the empty Locale (Java: Locale.UND).
    static Locale forLanguageTag(const std::string& tag);

    // "language[-script][-region][-variant…]"; all-empty → "und" (Java behavior).
    std::string toLanguageTag() const;
    // Java toString(): "language_COUNTRY_VARIANT" ("" for the empty Locale).
    std::string toString() const;

    std::string getLanguage() const { return mLanguage; }   // ISO-639, lower-case
    std::string getScript()   const { return mScript;   }   // ISO-15924, title-case
    std::string getCountry()  const { return mCountry;  }   // ISO-3166/UN M49, upper-case
    std::string getVariant()  const { return mVariant;  }   // '_'-joined, upper-case

    // AOSP getDisplayName family. Names come from i18n.dat's
    // LANGUAGES_DISPLAY/TERRITORIES_DISPLAY slots (native/self names, mined
    // from the glibc locale database) via I18nBridge; a miss falls back to
    // the code-based string exactly like AOSP's ICU-miss path. Cross-language
    // display (showing zh's name in German) is NOT carried by the data — the
    // inLocale overloads accept the AOSP signature but return the native
    // name (the common language-menu presentation).
    std::string getDisplayName() const;                    // inLocale = getDefault()
    std::string getDisplayName(const Locale& inLocale) const;
    std::string getDisplayLanguage() const;
    std::string getDisplayLanguage(const Locale& inLocale) const;
    std::string getDisplayCountry() const;
    std::string getDisplayCountry(const Locale& inLocale) const;
    std::string getDisplayScript() const;
    std::string getDisplayScript(const Locale& inLocale) const;
    std::string getDisplayVariant() const;
    std::string getDisplayVariant(const Locale& inLocale) const;

    bool operator==(const Locale& other) const;
    bool operator!=(const Locale& other) const { return !(*this == other); }
    // Java-shaped (language, country, variant) mix — script is not part of the
    // Java equals/hashCode contract, kept faithful here.
    int hashCode() const;

private:
    std::string mLanguage;
    std::string mScript;
    std::string mCountry;
    std::string mVariant;
};

} // namespace cdroid

#endif // CDROID_CORE_LOCALE_H
