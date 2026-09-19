#include <content/i18nbridge.h>

#ifdef ENABLE_I18N
#include <content/i18n/number_format.h>
#include <content/i18n/measure_format.h>
#include <cctype>

namespace cdroid {

i18n::LocaleInfo I18nBridge::toLocaleInfo(const Locale& locale)
{
    i18n::I18nStatus status = i18n::I18nStatus::ISUCCESS;
    return i18n::LocaleInfo::ForLanguageTag(locale.toLanguageTag().c_str(), status);
}

namespace {
// True for digit code points the number engine can emit: ASCII 0-9 plus the
// Arabic-Indic (U+0660-0669) and Eastern Arabic-Indic (U+06F0-06F9) blocks.
bool isDigitCp(uint32_t cp)
{
    return (cp >= 0x30 && cp <= 0x39) || (cp >= 0x660 && cp <= 0x669)
           || (cp >= 0x6F0 && cp <= 0x6F9);
}

// Length in bytes of the UTF-8 sequence starting at s[b] (1 on invalid lead).
size_t utf8LenAt(const std::string& s, size_t b)
{
    const unsigned char c = static_cast<unsigned char>(s[b]);
    if ((c & 0x80u) == 0) return 1;
    if ((c & 0xE0u) == 0xC0) return 2;
    if ((c & 0xF0u) == 0xE0) return 3;
    if ((c & 0xF8u) == 0xF0) return 4;
    return 1;
}

uint32_t utf8CpAt(const std::string& s, size_t b, size_t len)
{
    const auto* p = reinterpret_cast<const unsigned char*>(s.data()) + b;
    switch (len) {
    case 2: return ((p[0] & 0x1Fu) << 6) | (p[1] & 0x3Fu);
    case 3: return ((p[0] & 0x0Fu) << 12) | ((p[1] & 0x3Fu) << 6) | (p[2] & 0x3Fu);
    case 4: return ((p[0] & 0x07u) << 18) | ((p[1] & 0x3Fu) << 12)
                  | ((p[2] & 0x3Fu) << 6) | (p[3] & 0x3Fu);
    default: return p[0];
    }
}

// Strip the leading and trailing digit runs (code-point aware); what sits
// between them is the separator ("0,1" → ","; "1.234" → "."; ar renders
// Arabic-Indic digits and a multi-byte separator: "٠٫١" → "٫"). Empty when
// the probe carried no separator.
std::string middleNonDigits(const std::string& s)
{
    size_t b = 0;
    size_t e = s.size();
    while (b < e) {
        const size_t n = utf8LenAt(s, b);
        if (b + n > e || !isDigitCp(utf8CpAt(s, b, n))) break;
        b += n;
    }
    while (e > b) {
        // Walk back one code point from e.
        size_t st = e - 1;
        while (st > b && (static_cast<unsigned char>(s[st]) & 0xC0u) == 0x80) --st;
        const size_t n = e - st;
        if (n > 4 || !isDigitCp(utf8CpAt(s, st, n))) break;
        e = st;
    }
    return s.substr(b, e - b);
}
} // namespace

std::string I18nBridge::decimalSeparator(const Locale& locale)
{
    int status = 0;
    i18n::LocaleInfo info = toLocaleInfo(locale);
    i18n::NumberFormat engine(info, status);
    if (status != 0) return std::string();
    // "0.1" in the locale's own symbols: exactly one separator, between the
    // two digit runs.
    return middleNonDigits(engine.Format(0.1, i18n::NumberFormatType::DECIMAL, status));
}

std::string I18nBridge::groupingSeparator(const Locale& locale)
{
    int status = 0;
    i18n::LocaleInfo info = toLocaleInfo(locale);
    i18n::NumberFormat engine(info, status);
    if (status != 0) return std::string();
    // 1234 groups exactly once ("1,234"): exactly one separator, in the middle.
    return middleNonDigits(engine.Format(1234, status));
}

namespace {
// The DataResource mask chain always terminates on the en-US default, so a
// locale with no entry of its own (jv) silently resolves to the en-US value
// ("English"/"United States"). For display names that default is wrong —
// treat "equal to en-US's own entry while not being an en locale" as a miss
// so callers fall back to the raw codes.
std::string displayValueGuarded(const Locale& locale, i18n::DataResourceType type)
{
    i18n::LocaleInfo info = I18nBridge::toLocaleInfo(locale);
    i18n::DataResource resource(&info);
    if (!resource.Init()) return std::string();
    std::string out;
    resource.GetString(type, out);
    if (out.empty() || locale.getLanguage() == "en") return out;

    i18n::LocaleInfo enUS("en", "US");
    i18n::DataResource enRes(&enUS);
    std::string enOut;
    if (enRes.Init()) enRes.GetString(type, enOut);
    return (out == enOut) ? std::string() : out;
}
} // namespace

std::string I18nBridge::languageDisplayName(const Locale& locale)
{
    return displayValueGuarded(locale, i18n::LANGUAGES_DISPLAY);
}

std::string I18nBridge::regionDisplayName(const Locale& locale)
{
    return displayValueGuarded(locale, i18n::TERRITORIES_DISPLAY);
}

std::string I18nBridge::measureUnitShort(const Locale& locale, int value, const char* unit)
{
    i18n::I18nStatus status = i18n::I18nStatus::ISUCCESS;
    i18n::LocaleInfo info = toLocaleInfo(locale);
    i18n::MeasureFormat engine(info, status);
    if (status != 0) return std::string();
    std::string unitStr(unit);
    const std::string out = engine.Format(value, unitStr, status, i18n::MEASURE_SHORT);
    if (status != 0 || out.empty()) return std::string();
    return out;
}

} // namespace cdroid
#endif // ENABLE_I18N
