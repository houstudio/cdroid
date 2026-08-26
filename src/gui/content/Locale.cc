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
 * See core/Locale.h for the ported-scope notes.
 */
#include <content/Locale.h>
#include <gui_features.h>
#ifdef ENABLE_I18N
#include <content/i18nbridge.h>
#endif

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <mutex>
#include <vector>

namespace cdroid {

namespace {

std::string toLower(const std::string& s) {
    std::string out = s;
    std::transform(out.begin(), out.end(), out.begin(),
                   [](unsigned char c) { return (char)std::tolower(c); });
    return out;
}

std::string toUpper(const std::string& s) {
    std::string out = s;
    std::transform(out.begin(), out.end(), out.begin(),
                   [](unsigned char c) { return (char)std::toupper(c); });
    return out;
}

bool isAlpha(const std::string& s, size_t len) {
    if (s.size() != len) return false;
    for (char c : s) if (!std::isalpha((unsigned char)c)) return false;
    return true;
}

bool isAlphaNum(const std::string& s, size_t len) {
    if (s.size() != len) return false;
    for (char c : s) if (!std::isalnum((unsigned char)c)) return false;
    return true;
}

bool isDigits(const std::string& s, size_t len) {
    if (s.size() != len) return false;
    for (char c : s) if (!std::isdigit((unsigned char)c)) return false;
    return true;
}

// "hant" → "Hant" (ISO-15924 title case).
std::string titleCase(const std::string& s) {
    std::string out = toLower(s);
    if (!out.empty()) out[0] = (char)std::toupper((unsigned char)out[0]);
    return out;
}

} // anonymous namespace

// ---- well-known constants (Java: Locale.<name>) ---------------------------
const Locale Locale::ROOT;
const Locale Locale::ENGLISH("en");
const Locale Locale::FRENCH("fr");
const Locale Locale::GERMAN("de");
const Locale Locale::ITALIAN("it");
const Locale Locale::JAPANESE("ja");
const Locale Locale::KOREAN("ko");
const Locale Locale::CHINESE("zh");
const Locale Locale::SIMPLIFIED_CHINESE = Locale::forLanguageTag("zh-Hans-CN");
const Locale Locale::TRADITIONAL_CHINESE = Locale::forLanguageTag("zh-Hant-TW");
const Locale Locale::FRANCE("fr", "FR");
const Locale Locale::GERMANY("de", "DE");
const Locale Locale::ITALY("it", "IT");
const Locale Locale::JAPAN("ja", "JP");
const Locale Locale::KOREA("ko", "KR");
const Locale Locale::CHINA = Locale::SIMPLIFIED_CHINESE;
const Locale Locale::PRC = Locale::SIMPLIFIED_CHINESE;
const Locale Locale::TAIWAN = Locale::TRADITIONAL_CHINESE;
const Locale Locale::UK("en", "GB");
const Locale Locale::US("en", "US");
const Locale Locale::CANADA("en", "CA");
const Locale Locale::CANADA_FRENCH("fr", "CA");

// ---- constructors ----------------------------------------------------------
Locale::Locale() = default;

Locale::Locale(const std::string& language) {
    // Java normalizes case on construction (lower-case language, upper-case
    // country/variant); script is left to forLanguageTag().
    mLanguage = toLower(language);
}

Locale::Locale(const std::string& language, const std::string& country)
    : Locale(language) {
    mCountry = toUpper(country);
}

Locale::Locale(const std::string& language, const std::string& country, const std::string& variant)
    : Locale(language, country) {
    mVariant = toUpper(variant);
}

// ---- default (process-wide, Java: user.language / user.region) -------------
namespace {
// AOSP ojluni Locale.initDefault() reads the host's default locale from
// user.language/user.country properties. CDROID runs on Linux hosts, where
// the same information lives in the POSIX locale environment — glibc
// precedence LC_ALL > LC_MESSAGES > LANG, values like "zh_CN.UTF-8" or
// "en-US". Parses the language[_TERRITORY] part; anything unparseable
// ("C", "POSIX", empty) falls back to en_US like ojluni.
Locale initDefaultFromHostEnv() {
    const char* spec = nullptr;
    for (const char* name : {"LC_ALL", "LC_MESSAGES", "LANG"}) {
        const char* v = std::getenv(name);
        if (v && *v && std::strcmp(v, "C") != 0 && std::strcmp(v, "POSIX") != 0) {
            spec = v;
            break;
        }
    }
    if (spec == nullptr) return Locale("en", "US");

    std::string s(spec);
    // Strip codeset ("zh_CN.UTF-8") and modifier ("zh_CN.UTF-8@pinyin").
    const size_t dot = s.find('.');
    if (dot != std::string::npos) s.resize(dot);
    const size_t at = s.find('@');
    if (at != std::string::npos) s.resize(at);

    // BCP-47 dash form ("en-US") goes through forLanguageTag; the POSIX
    // underscore form ("zh_CN") splits here.
    if (s.find('-') != std::string::npos) {
        Locale tagged = Locale::forLanguageTag(s);
        return tagged;
    }
    const size_t us = s.find('_');
    if (us == std::string::npos) return Locale(s);
    const std::string language = s.substr(0, us);
    const std::string country  = s.substr(us + 1);
    if (country.empty()) return Locale(language);
    return Locale(language, country);
}

// One shared default behind a mutex (getDefault/setDefault must observe the
// same slot; ojluni keeps a per-Category static).
Locale& defaultLocaleSlot() {
    static Locale sDefault = initDefaultFromHostEnv();
    return sDefault;
}
std::mutex& defaultLocaleLock() {
    static std::mutex sLock;
    return sLock;
}
} // anonymous namespace

Locale Locale::getDefault() {
    std::lock_guard<std::mutex> lock(defaultLocaleLock());
    return defaultLocaleSlot();
}

void Locale::setDefault(const Locale& locale) {
    std::lock_guard<std::mutex> lock(defaultLocaleLock());
    defaultLocaleSlot() = locale;
}

// ---- BCP-47 -----------------------------------------------------------------
Locale Locale::forLanguageTag(const std::string& tag) {
    // Split on '-'; classify subtags the way the well-formed subset demands:
    //   language: first subtag, 2-3 alpha (singletons/extensions are skipped —
    //             unicode extensions are not ported, see header)
    //   script:   4 alpha
    //   region:   2 alpha or 3 digit
    //   variant:  5-8 alnum or (4 alnum after a well-formed subtag), '_'-joined
    Locale out;
    std::vector<std::string> subtags;
    std::string cur;
    for (char c : tag) {
        if (c == '-') { subtags.push_back(cur); cur.clear(); }
        else cur.push_back(c);
    }
    subtags.push_back(cur);

    bool haveLanguage = false, haveScript = false, haveRegion = false;
    std::string variant;
    for (size_t i = 0; i < subtags.size(); i++) {
        const std::string& sub = subtags[i];
        if (sub.empty()) continue;
        // Singleton subtags introduce extensions / private use — dropped (TODO).
        if (sub.size() == 1) {
            while (i + 1 < subtags.size() && subtags[i + 1].size() > 1) i++;
            continue;
        }
        if (!haveLanguage) {
            if (isAlpha(sub, 2) || isAlpha(sub, 3)) {
                const std::string lang = toLower(sub);
                if (lang != "und") out.mLanguage = lang;
                haveLanguage = true;
            }
            continue;
        }
        if (!haveScript && isAlpha(sub, 4)) {
            out.mScript = titleCase(sub);
            haveScript = true;
            continue;
        }
        if (!haveRegion && (isAlpha(sub, 2) || isDigits(sub, 3))) {
            // 2 alpha = ISO-3166, 3 digits = UN M49; both upper-case.
            out.mCountry = toUpper(sub);
            haveRegion = true;
            continue;
        }
        if ((sub.size() >= 5 && sub.size() <= 8 && isAlphaNum(sub, sub.size()))
                || (sub.size() == 4 && std::isdigit((unsigned char)sub[0]))) {
            if (!variant.empty()) variant += "_";
            variant += toUpper(sub);
        }
    }
    out.mVariant = variant;
    return out;
}

std::string Locale::toLanguageTag() const {
    if (mLanguage.empty() && mScript.empty() && mCountry.empty() && mVariant.empty()) {
        return "und"; // Java: the empty Locale serializes as "und"
    }
    std::string tag;
    if (!mLanguage.empty()) tag += mLanguage;
    if (!mScript.empty())  { if (!tag.empty()) tag += "-"; tag += mScript; }
    if (!mCountry.empty()) { if (!tag.empty()) tag += "-"; tag += mCountry; }
    if (!mVariant.empty()) {
        // '_'-joined variants re-split into '-'-separated subtags.
        std::string v = mVariant;
        for (auto& c : v) if (c == '_') c = '-';
        if (!tag.empty()) tag += "-";
        tag += v;
    }
    return tag;
}

std::string Locale::toString() const {
    std::string out;
    if (!mLanguage.empty()) out += mLanguage;
    if (!mCountry.empty())  { if (!out.empty()) out += "_"; out += mCountry; }
    if (!mVariant.empty())  { if (!out.empty()) out += "_"; out += mVariant; }
    return out;
}

// ---- AOSP getDisplayName family -------------------------------------------
// Names come from i18n.dat's LANGUAGES_DISPLAY / TERRITORIES_DISPLAY slots
// (native/self names via I18nBridge); a miss falls back to the code itself,
// the same shape as AOSP's ICU-miss path. The data carries only the SELF
// name, so the inLocale overloads return the native name (the common
// language-menu presentation) — see Locale.h.
namespace {
std::string displayOr(const std::string& name, const std::string& code) {
    return name.empty() ? code : name;
}
} // namespace

std::string Locale::getDisplayLanguage(const Locale&) const {
#ifdef ENABLE_I18N
    return displayOr(I18nBridge::languageDisplayName(*this), mLanguage);
#else
    return mLanguage;
#endif
}

std::string Locale::getDisplayLanguage() const {
    return getDisplayLanguage(getDefault());
}

std::string Locale::getDisplayCountry(const Locale&) const {
#ifdef ENABLE_I18N
    return displayOr(I18nBridge::regionDisplayName(*this), mCountry);
#else
    return mCountry;
#endif
}

std::string Locale::getDisplayCountry() const {
    return getDisplayCountry(getDefault());
}

std::string Locale::getDisplayScript(const Locale&) const {
    // No script-name table in the data — always the code (AOSP ICU-miss shape).
    return mScript;
}

std::string Locale::getDisplayScript() const {
    return getDisplayScript(getDefault());
}

std::string Locale::getDisplayVariant(const Locale&) const {
    // Variant names are rare enough that AOSP itself mostly shows the raw
    // code; no table carries them.
    return mVariant;
}

std::string Locale::getDisplayVariant() const {
    return getDisplayVariant(getDefault());
}

std::string Locale::getDisplayName(const Locale& inLocale) const {
    // Java.util.Locale.getDisplayName: language (country[,variant]) — the
    // parentheses parts appear only for the fields this locale carries.
    const std::string languageName = getDisplayLanguage(inLocale);
    if (mCountry.empty() && mVariant.empty()) return languageName;
    std::string out = languageName + " (";
    if (!mCountry.empty()) out += getDisplayCountry(inLocale);
    if (!mVariant.empty()) {
        if (!mCountry.empty()) out += ",";
        out += getDisplayVariant(inLocale);
    }
    out += ")";
    return out;
}

std::string Locale::getDisplayName() const {
    return getDisplayName(getDefault());
}

bool Locale::operator==(const Locale& other) const {
    return mLanguage == other.mLanguage && mCountry == other.mCountry
        && mVariant == other.mVariant;
}

int Locale::hashCode() const {
    // Java: language ^ country ^ variant over their (lower-cased) hashCodes.
    // C++ translation: the classic 31-mix over the three fields.
    int result = 0;
    for (const std::string* f : {&mLanguage, &mCountry, &mVariant}) {
        int h = 0;
        for (char c : *f) h = 31 * h + c;
        result ^= h;
    }
    return result;
}

} // namespace cdroid
