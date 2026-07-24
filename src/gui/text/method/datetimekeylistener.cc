/*
 * Ported from Android android.text.method.DateTimeKeyListener (Apache 2.0).
 * Compatibility (locale=null) path only; see header.
 */
#include <text/method/datetimekeylistener.h>
#include <text/inputtype.h>

namespace cdroid {

// Android CHARACTERS = { '0'..'9', 'a', 'm', 'p', ':', '/', '-', ' ' }
const std::u16string DateTimeKeyListener::CHARACTERS = u"0123456789amp:/- ";

DateTimeKeyListener* DateTimeKeyListener::sInstance = nullptr;

DateTimeKeyListener* DateTimeKeyListener::getInstance() {
    if (sInstance == nullptr) {
        sInstance = new DateTimeKeyListener();
    }
    return sInstance;
}

int DateTimeKeyListener::getInputType() const {
    // Compat path: mNeedsAdvancedInput is false, so the datetime (normal) type.
    // (The locale-aware mNeedsAdvancedInput -> TYPE_CLASS_TEXT path is deferred.)
    return InputType::TYPE_CLASS_DATETIME | InputType::TYPE_DATETIME_VARIATION_NORMAL;
}

std::u16string DateTimeKeyListener::getAcceptedChars() const {
    return CHARACTERS;
}

} // namespace cdroid
