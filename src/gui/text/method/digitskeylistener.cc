/*
 * Ported from Android android.text.method.DigitsKeyListener (Apache 2.0).
 * Compatibility (locale=null) path only; see header.
 */
#include <text/method/digitskeylistener.h>
#include <text/spannablestringbuilder.h>
#include <text/inputtype.h>
#include <map>

namespace cdroid {

namespace {
// COMPATIBILITY_CHARACTERS[kind], kind = (sign?1:0)|(decimal?2:0).
const std::u16string COMPAT[4] = {
    u"0123456789",
    u"0123456789-+",
    u"0123456789.",
    u"0123456789-+."
};

// Cached singletons for the (sign, decimal) variants (Android's
// sLocaleInstanceCache for locale=null). Never deleted, like Android's.
DigitsKeyListener* sInstance[4] = { nullptr, nullptr, nullptr, nullptr };

// String-mode cache (Android's sStringInstanceCache).
std::map<std::u16string, DigitsKeyListener*>& stringCache() {
    static std::map<std::u16string, DigitsKeyListener*> c;
    return c;
}
} // namespace

DigitsKeyListener::DigitsKeyListener()
    : DigitsKeyListener(false, false) {
}

DigitsKeyListener::DigitsKeyListener(bool sign, bool decimal)
    : mSign(sign), mDecimal(decimal), mStringMode(false),
      mDecimalPointChars(u"."), mSignChars(u"-+") {
    const int kind = (sign ? 1 : 0) | (decimal ? 2 : 0);
    mAccepted = COMPAT[kind];
}

DigitsKeyListener::DigitsKeyListener(const std::u16string& accepted)
    : mAccepted(accepted), mSign(false), mDecimal(false), mStringMode(true),
      mDecimalPointChars(u"."), mSignChars(u"-+") {
}

DigitsKeyListener* DigitsKeyListener::getInstance() {
    return getInstance(false, false);
}

DigitsKeyListener* DigitsKeyListener::getInstance(bool sign, bool decimal) {
    const int kind = (sign ? 1 : 0) | (decimal ? 2 : 0);
    if (sInstance[kind] == nullptr) {
        sInstance[kind] = new DigitsKeyListener(sign, decimal);
    }
    return sInstance[kind];
}

DigitsKeyListener* DigitsKeyListener::getInstance(const std::u16string& accepted) {
    auto& cache = stringCache();
    auto it = cache.find(accepted);
    if (it != cache.end()) return it->second;
    DigitsKeyListener* result = new DigitsKeyListener(accepted);
    cache[accepted] = result;
    return result;
}

int DigitsKeyListener::getInputType() const {
    // Compat mode never needs advanced input, so this stays TYPE_CLASS_NUMBER.
    int contentType = InputType::TYPE_CLASS_NUMBER;
    if (mSign)    contentType |= InputType::TYPE_NUMBER_FLAG_SIGNED;
    if (mDecimal) contentType |= InputType::TYPE_NUMBER_FLAG_DECIMAL;
    return contentType;
}

std::u16string DigitsKeyListener::getAcceptedChars() const {
    return mAccepted;
}

bool DigitsKeyListener::isSignChar(char16_t c) const {
    return mSignChars.find(c) != std::u16string::npos;
}

bool DigitsKeyListener::isDecimalPointChar(char16_t c) const {
    return mDecimalPointChars.find(c) != std::u16string::npos;
}

CharSequence* DigitsKeyListener::filter(CharSequence* source, int start, int end,
                                         Spanned* dest, int dstart, int dend) {
    CharSequence* out = NumberKeyListener::filter(source, start, end, dest, dstart, dend);

    if (!mSign && !mDecimal) {
        return out;
    }

    if (out != nullptr) {
        source = out;
        start = 0;
        end = (int)out->length();
    }

    int sign = -1;
    int decimal = -1;
    const int dlen = (int)dest->length();

    // Find out if the existing text has sign or decimal point characters.
    for (int i = 0; i < dstart; i++) {
        const char16_t c = (char16_t)dest->charAt(i);
        if (isSignChar(c)) {
            sign = i;
        } else if (isDecimalPointChar(c)) {
            decimal = i;
        }
    }
    for (int i = dend; i < dlen; i++) {
        const char16_t c = (char16_t)dest->charAt(i);
        if (isSignChar(c)) {
            return new SpannableStringBuilder();   // Android returns "" — nothing before a sign.
        } else if (isDecimalPointChar(c)) {
            decimal = i;
        }
    }

    // Strip sign/decimal from source as needed (sign must be first char; only one
    // decimal). Reverse order so offsets stay stable.
    SpannableStringBuilder* stripped = nullptr;

    for (int i = end - 1; i >= start; i--) {
        const char16_t c = (char16_t)source->charAt(i);
        bool strip = false;

        if (isSignChar(c)) {
            if (i != start || dstart != 0) {
                strip = true;
            } else if (sign >= 0) {
                strip = true;
            } else {
                sign = i;
            }
        } else if (isDecimalPointChar(c)) {
            if (decimal >= 0) {
                strip = true;
            } else {
                decimal = i;
            }
        }

        if (strip) {
            if (end == start + 1) {
                return new SpannableStringBuilder();   // only one char, and it was stripped
            }
            if (stripped == nullptr) {
                stripped = new SpannableStringBuilder();
                for (int k = start; k < end; k++) {
                    stripped->append((char16_t)source->charAt(k));
                }
            }
            stripped->Delete(i - start, i + 1 - start);
        }
    }

    if (stripped != nullptr) {
        return stripped;
    } else if (out != nullptr) {
        return out;
    } else {
        return nullptr;
    }
}

} // namespace cdroid
