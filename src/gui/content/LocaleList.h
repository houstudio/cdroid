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
 *
 * An immutable list of Locales, typically used to keep an ordered list of user
 * preferences for locales. Java's null get()/computeFirstMatch results map to
 * the empty Locale sentinel; AOSP's String[] becomes std::vector<std::string>.
 * The Parcelable surface is not ported (CDROID has no Parcel). getLikelyScript
 * (AOSP-private) is public so TextUtils can share the likely-script table.
 *
 * NB: this is a framework-public header — it must not include anything from
 * androidfw/ (the ResTable_config packing stays in ResourcesImpl).
 */
#ifndef CDROID_CORE_LOCALELIST_H
#define CDROID_CORE_LOCALELIST_H

#include <content/Locale.h>

#include <string>
#include <vector>

namespace cdroid {

class LocaleList {
public:
    // The empty list (Java: new LocaleList() with zero args).
    LocaleList();
    // Duplicates are dropped, like Java.
    explicit LocaleList(const std::vector<Locale>& list);
    // topLocale moved to the front of otherLocales (added if absent);
    // a null otherLocales maps to a single-locale list.
    LocaleList(const Locale& topLocale, const LocaleList* otherLocales);

    Locale get(int index) const;               // out-of-range → empty Locale (Java null)
    bool isEmpty() const;
    int size() const;
    int indexOf(const Locale& locale) const;
    bool operator==(const LocaleList& other) const;
    bool operator!=(const LocaleList& other) const { return !(*this == other); }
    int hashCode() const;
    std::string toString() const;              // "[zh-CN,en-US]"
    // Comma-separated language tags, built at construction.
    const std::string& toLanguageTags() const { return mStringRepresentation; }

    // Locales present in both lists, in this list's order (matching is
    // matchesLanguageAndScript, not exact equality).
    std::vector<Locale> getIntersection(const LocaleList& other) const;

    static const LocaleList& getEmptyLocaleList();
    static LocaleList forLanguageTags(const std::string& list);

    // en-XA / ar-XB pseudo-locales.
    static bool isPseudoLocale(const Locale& locale);
    static bool isPseudoLocalesOnly(const std::vector<std::string>* supportedLocales);

    // Language+script match (script inferred via getLikelyScript):
    // [zh-HK] matches [zh-Hant]; [en-US] matches [en-CA].
    static bool matchesLanguageAndScript(const Locale& supported, const Locale& desired);

    // First match against an unordered set of supported BCP-47 tags; the empty
    // Locale (Java null) means "no match / empty list".
    Locale getFirstMatch(const std::vector<std::string>& supportedLocales) const;
    int getFirstMatchIndex(const std::vector<std::string>& supportedLocales) const;
    Locale getFirstMatchWithEnglishSupported(const std::vector<std::string>& supportedLocales) const;
    int getFirstMatchIndexWithEnglishSupported(const std::vector<std::string>& supportedLocales) const;

    // ---- process-wide default (recalculated from Locale::getDefault()) ----
    static LocaleList getDefault();
    static LocaleList getAdjustedDefault();
    static void setDefault(const LocaleList& locales, int localeIndex = 0);

    // The locale's script, inferring the likely script for the language when
    // unset (ULocale.addLikelySubtags upstream; a compact table here).
    static std::string getLikelyScript(const Locale& locale);

private:
    int findFirstMatchIndex(const Locale& supportedLocale) const;
    int computeFirstMatchIndex(const std::vector<std::string>& supportedLocales,
                               bool assumeEnglishIsSupported) const;
    Locale computeFirstMatch(const std::vector<std::string>& supportedLocales,
                             bool assumeEnglishIsSupported) const;

    std::vector<Locale> mList;
    std::string mStringRepresentation;
};

} // namespace cdroid

#endif // CDROID_CORE_LOCALELIST_H
