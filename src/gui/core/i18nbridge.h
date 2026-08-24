#ifndef CDROID_CORE_I18NBRIDGE_H
#define CDROID_CORE_I18NBRIDGE_H
/*
 * CDROID-owned adapter between cdroid::Locale (the java.util.Locale port —
 * the API face) and the vendored OHOS i18n engine's LocaleInfo. The i18n/
 * tree stays untouched: every Android-shaped entry point converts through
 * here instead of teaching the engine about cdroid::Locale.
 *
 * The conversion is a BCP-47 round-trip: Locale::toLanguageTag() →
 * LocaleInfo::ForLanguageTag(). Extensions ("-u-ca-…") ride along in the tag
 * and are parsed by ForLanguageTag (GetExtension) even though cdroid::Locale
 * itself drops them — the bridge preserves more than the Locale keeps.
 */
#include <string>
#include <gui_features.h>

#ifdef ENABLE_I18N
#include <core/Locale.h>
#include <i18n/locale_info.h>

namespace cdroid {

class I18nBridge {
public:
    static i18n::LocaleInfo toLocaleInfo(const Locale& locale);

    // Locale separators straight out of the CLDR number-format data, as UTF-8
    // strings (NOT char: CLDR separators can be multi-byte — ar decimal "٫"
    // U+066B, fr grouping U+202F narrow no-break space). Derived through the
    // engine's own Format() output so the vendored code needs no new getters.
    // Empty when the engine has no data for the locale (caller keeps default).
    static std::string decimalSeparator(const Locale& locale);
    static std::string groupingSeparator(const Locale& locale);
};

} // namespace cdroid
#endif // ENABLE_I18N
#endif // CDROID_CORE_I18NBRIDGE_H
