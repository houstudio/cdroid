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
 * Ported to C++ for CDROID from android.os.LocaleList (android-36).
 * See core/LocaleList.h for the ported-scope notes.
 */
#include <content/LocaleList.h>

#include <climits>
#include <mutex>

namespace cdroid {

namespace {

// Likely (default) script per language, standing in for
// ULocale.addLikelySubtags(...).getScript(). Languages absent from the table
// have no known script — matchesLanguageAndScript then falls back to the
// region comparison, exactly the path Android documents for the same case.
// zh is special-cased by region (Hans everywhere but TW/HK/MO).
struct LikelyScript { const char* language; const char* script; };
const LikelyScript kLikelyScripts[] = {
    {"am", "Ethi"}, {"ar", "Arab"}, {"be", "Cyrl"}, {"bg", "Cyrl"}, {"bn", "Beng"},
    {"bo", "Tibt"}, {"dv", "Thaa"}, {"dz", "Tibt"}, {"el", "Grek"}, {"fa", "Arab"},
    {"gu", "Gujr"}, {"he", "Hebr"}, {"hi", "Deva"}, {"hy", "Armn"}, {"ja", "Jpan"},
    {"ka", "Geor"}, {"km", "Khmr"}, {"kn", "Knda"}, {"ko", "Kore"}, {"ks", "Arab"},
    {"ky", "Cyrl"}, {"lo", "Laoo"}, {"mk", "Cyrl"}, {"ml", "Mlym"}, {"mn", "Cyrl"},
    {"mr", "Deva"}, {"my", "Mymr"}, {"ne", "Deva"}, {"or", "Orya"}, {"pa", "Guru"},
    {"ps", "Arab"}, {"ru", "Cyrl"}, {"sd", "Arab"}, {"si", "Sinh"}, {"sr", "Cyrl"},
    {"ta", "Taml"}, {"te", "Telu"}, {"tg", "Cyrl"}, {"th", "Thai"}, {"ti", "Ethi"},
    {"ug", "Arab"}, {"uk", "Cyrl"}, {"ur", "Arab"}, {"yi", "Hebr"}, {"yue", "Hant"},
};

const char STRING_EN_XA[] = "en-XA";
const char STRING_AR_XB[] = "ar-XB";
constexpr int NUM_PSEUDO_LOCALES = 2;

// Process-wide default-list state (Java: sLock-guarded statics).
struct DefaultState {
    std::mutex lock;
    LocaleList lastExplicitlySetLocaleList;
    LocaleList defaultLocaleList;
    LocaleList defaultAdjustedLocaleList;
    Locale lastDefaultLocale;
    bool hasDefaultLocaleList = false;
};
DefaultState& defaultState() {
    static DefaultState s;
    return s;
}

} // anonymous namespace

// ---- construction -----------------------------------------------------------

LocaleList::LocaleList() = default;

LocaleList::LocaleList(const std::vector<Locale>& list) {
    std::string sb;
    for (size_t i = 0; i < list.size(); i++) {
        const Locale& l = list[i];
        bool duplicate = false;
        for (const Locale& kept : mList) { // Java: HashSet<Locale> + equals
            if (kept == l) { duplicate = true; break; }
        }
        if (duplicate) {
            // Dropping duplicated locale entries.
        } else {
            mList.push_back(l);
            sb += l.toLanguageTag();
            if (i < list.size() - 1) sb += ',';
        }
    }
    mStringRepresentation = sb;
}

LocaleList::LocaleList(const Locale& topLocale, const LocaleList* otherLocales) {
    const int inputLength = (otherLocales == nullptr) ? 0 : (int)otherLocales->mList.size();
    int topLocaleIndex = -1;
    for (int i = 0; i < inputLength; i++) {
        if (topLocale == otherLocales->mList[i]) {
            topLocaleIndex = i;
            break;
        }
    }

    const int outputLength = inputLength + (topLocaleIndex == -1 ? 1 : 0);
    std::vector<Locale> localeList((size_t)outputLength);
    localeList[0] = topLocale;
    if (topLocaleIndex == -1) {
        // topLocale was not in otherLocales
        for (int i = 0; i < inputLength; i++) localeList[(size_t)i + 1] = otherLocales->mList[(size_t)i];
    } else {
        for (int i = 0; i < topLocaleIndex; i++) localeList[(size_t)i + 1] = otherLocales->mList[(size_t)i];
        for (int i = topLocaleIndex + 1; i < inputLength; i++) localeList[(size_t)i] = otherLocales->mList[(size_t)i];
    }

    std::string sb;
    for (int i = 0; i < outputLength; i++) {
        sb += localeList[(size_t)i].toLanguageTag();
        if (i < outputLength - 1) sb += ',';
    }
    mList = std::move(localeList);
    mStringRepresentation = sb;
}

// ---- basic accessors ---------------------------------------------------------

Locale LocaleList::get(int index) const {
    return (0 <= index && index < (int)mList.size()) ? mList[(size_t)index] : Locale();
}

bool LocaleList::isEmpty() const { return mList.empty(); }

int LocaleList::size() const { return (int)mList.size(); }

int LocaleList::indexOf(const Locale& locale) const {
    for (size_t i = 0; i < mList.size(); i++) {
        if (mList[i] == locale) return (int)i;
    }
    return -1;
}

bool LocaleList::operator==(const LocaleList& other) const {
    return mList == other.mList;
}

int LocaleList::hashCode() const {
    int result = 1;
    for (const Locale& l : mList) result = 31 * result + l.hashCode();
    return result;
}

std::string LocaleList::toString() const {
    std::string sb = "[";
    for (size_t i = 0; i < mList.size(); i++) {
        sb += mList[i].toString();
        if (i < mList.size() - 1) sb += ',';
    }
    sb += "]";
    return sb;
}

std::vector<Locale> LocaleList::getIntersection(const LocaleList& other) const {
    std::vector<Locale> intersection;
    for (const Locale& l1 : mList) {
        for (const Locale& l2 : other.mList) {
            if (matchesLanguageAndScript(l2, l1)) {
                intersection.push_back(l1);
                break;
            }
        }
    }
    return intersection;
}

// ---- factory / pseudo-locale helpers -----------------------------------------

const LocaleList& LocaleList::getEmptyLocaleList() {
    static const LocaleList sEmpty;
    return sEmpty;
}

LocaleList LocaleList::forLanguageTags(const std::string& list) {
    if (list.empty()) return getEmptyLocaleList();
    std::vector<Locale> localeArray;
    size_t start = 0;
    while (true) {
        const size_t comma = list.find(',', start);
        const std::string tag = list.substr(start,
                comma == std::string::npos ? std::string::npos : comma - start);
        localeArray.push_back(Locale::forLanguageTag(tag));
        if (comma == std::string::npos) break;
        start = comma + 1;
    }
    return LocaleList(localeArray);
}

std::string LocaleList::getLikelyScript(const Locale& locale) {
    const std::string script = locale.getScript();
    if (!script.empty()) return script;
    const std::string language = locale.getLanguage();
    if (language.empty()) return std::string();
    if (language == "zh") {
        const std::string& country = locale.getCountry();
        return (country == "TW" || country == "HK" || country == "MO") ? "Hant" : "Hans";
    }
    for (const LikelyScript& ls : kLikelyScripts) {
        if (language == ls.language) return ls.script;
    }
    // CLDR gives every language a likely script; unlisted languages default
    // to Latin (matches [en-US]/[en-CA] the way AOSP's addLikelySubtags does).
    return "Latn";
}

bool LocaleList::isPseudoLocale(const Locale& locale) {
    return (locale == Locale("en", "XA")) || (locale == Locale("ar", "XB"));
}

bool LocaleList::matchesLanguageAndScript(const Locale& supported, const Locale& desired) {
    if (supported == desired) {
        return true;  // return early so we don't do unnecessary computation
    }
    if (supported.getLanguage() != desired.getLanguage()) {
        return false;
    }
    if (isPseudoLocale(supported) || isPseudoLocale(desired)) {
        // The locales are not the same, but the languages are the same, and one of
        // the locales is a pseudo-locale. So this is not a match.
        return false;
    }
    const std::string supportedScr = getLikelyScript(supported);
    if (supportedScr.empty()) {
        // If we can't guess a script, we don't know enough about the locales'
        // language to find if the locales match. So we fall back to old behavior
        // of matching, which considered locales with different regions different.
        const std::string supportedRegion = supported.getCountry();
        return supportedRegion.empty() || supportedRegion == desired.getCountry();
    }
    const std::string desiredScr = getLikelyScript(desired);
    // There is no match if the two locales use different scripts. This will most
    // importantly take care of traditional vs simplified Chinese.
    return supportedScr == desiredScr;
}

// ---- first-match -------------------------------------------------------------

int LocaleList::findFirstMatchIndex(const Locale& supportedLocale) const {
    for (size_t idx = 0; idx < mList.size(); idx++) {
        if (matchesLanguageAndScript(supportedLocale, mList[idx])) return (int)idx;
    }
    return INT_MAX;
}

int LocaleList::computeFirstMatchIndex(const std::vector<std::string>& supportedLocales,
                                       bool assumeEnglishIsSupported) const {
    if (mList.size() == 1) {  // just one locale, perhaps the most common scenario
        return 0;
    }
    if (mList.empty()) {  // empty locale list
        return -1;
    }

    int bestIndex = INT_MAX;
    // Try English first, so we can return early if it's in the LocaleList
    if (assumeEnglishIsSupported) {
        const int idx = findFirstMatchIndex(Locale::forLanguageTag("en-Latn"));
        if (idx == 0) { // We have a match on the first locale, which is good enough
            return 0;
        } else if (idx < bestIndex) {
            bestIndex = idx;
        }
    }
    for (const std::string& languageTag : supportedLocales) {
        const Locale supportedLocale = Locale::forLanguageTag(languageTag);
        // We expect the average length of locale lists used for locale resolution
        // to be smaller than three, so it's OK to do this as an O(mn) algorithm.
        const int idx = findFirstMatchIndex(supportedLocale);
        if (idx == 0) { // We have a match on the first locale, which is good enough
            return 0;
        } else if (idx < bestIndex) {
            bestIndex = idx;
        }
    }
    if (bestIndex == INT_MAX) {
        // no match was found, so we fall back to the first locale in the locale list
        return 0;
    } else {
        return bestIndex;
    }
}

Locale LocaleList::computeFirstMatch(const std::vector<std::string>& supportedLocales,
                                     bool assumeEnglishIsSupported) const {
    const int bestIndex = computeFirstMatchIndex(supportedLocales, assumeEnglishIsSupported);
    return bestIndex == -1 ? Locale() : mList[(size_t)bestIndex];
}

Locale LocaleList::getFirstMatch(const std::vector<std::string>& supportedLocales) const {
    return computeFirstMatch(supportedLocales, false /* assume English is not supported */);
}

int LocaleList::getFirstMatchIndex(const std::vector<std::string>& supportedLocales) const {
    return computeFirstMatchIndex(supportedLocales, false /* assume English is not supported */);
}

Locale LocaleList::getFirstMatchWithEnglishSupported(
        const std::vector<std::string>& supportedLocales) const {
    return computeFirstMatch(supportedLocales, true /* assume English is supported */);
}

int LocaleList::getFirstMatchIndexWithEnglishSupported(
        const std::vector<std::string>& supportedLocales) const {
    return computeFirstMatchIndex(supportedLocales, true /* assume English is supported */);
}

bool LocaleList::isPseudoLocalesOnly(const std::vector<std::string>* supportedLocales) {
    if (supportedLocales == nullptr) return true;
    if ((int)supportedLocales->size() > NUM_PSEUDO_LOCALES + 1) {
        // This is for optimization. Since there's no repetition in the input, if
        // we have more than the number of pseudo-locales plus one for the empty
        // string, it's guaranteed that we have some meaningful locale in the
        // collection, so the list is not "practically empty".
        return false;
    }
    for (const std::string& locale : *supportedLocales) {
        if (!locale.empty() && locale != STRING_EN_XA && locale != STRING_AR_XB) {
            return false;
        }
    }
    return true;
}

// ---- process-wide default -----------------------------------------------------

LocaleList LocaleList::getDefault() {
    const Locale defaultLocale = Locale::getDefault();
    DefaultState& s = defaultState();
    std::lock_guard<std::mutex> lock(s.lock);
    if (!(defaultLocale == s.lastDefaultLocale)) {
        s.lastDefaultLocale = defaultLocale;
        // It's either the first time someone has asked for the default locale
        // list, or someone has called Locale.setDefault() since we last set or
        // adjusted the default locale list. So let's recalculate the locale list.
        if (s.hasDefaultLocaleList && defaultLocale == s.defaultLocaleList.get(0)) {
            // The default Locale has changed, but it happens to be the first
            // locale in the default locale list, so we don't need to construct a
            // new locale list.
            return s.defaultLocaleList;
        }
        s.defaultLocaleList = LocaleList(defaultLocale, &s.lastExplicitlySetLocaleList);
        s.defaultAdjustedLocaleList = s.defaultLocaleList;
        s.hasDefaultLocaleList = true;
    }
    return s.defaultLocaleList;
}

LocaleList LocaleList::getAdjustedDefault() {
    getDefault(); // to recalculate the default locale list, if necessary
    DefaultState& s = defaultState();
    std::lock_guard<std::mutex> lock(s.lock);
    return s.defaultAdjustedLocaleList;
}

void LocaleList::setDefault(const LocaleList& locales, int localeIndex) {
    DefaultState& s = defaultState();
    std::lock_guard<std::mutex> lock(s.lock);
    s.lastDefaultLocale = locales.get(localeIndex);
    Locale::setDefault(s.lastDefaultLocale);
    s.lastExplicitlySetLocaleList = locales;
    s.defaultLocaleList = locales;
    s.hasDefaultLocaleList = true;
    if (localeIndex == 0) {
        s.defaultAdjustedLocaleList = s.defaultLocaleList;
    } else {
        s.defaultAdjustedLocaleList = LocaleList(s.lastDefaultLocale, &s.defaultLocaleList);
    }
}

} // namespace cdroid
