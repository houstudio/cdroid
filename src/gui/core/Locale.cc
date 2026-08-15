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
#include <core/Locale.h>

#include <algorithm>
#include <cctype>
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
// One shared default behind a mutex (getDefault/setDefault must observe the
// same slot; ojluni keeps a per-Category static).
Locale& defaultLocaleSlot() {
    static Locale sDefault = Locale("en", "US"); // ojluni initDefault fallback
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
