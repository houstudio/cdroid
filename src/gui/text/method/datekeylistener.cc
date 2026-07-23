/*
 * Ported from Android android.text.method.DateKeyListener (Apache 2.0).
 * Compatibility (locale=null) path only; see header.
 */
#include <text/method/datekeylistener.h>
#include <text/inputtype.h>

namespace cdroid {

// Android CHARACTERS = { '0'..'9', '/', '-', '.' }
const std::u16string DateKeyListener::CHARACTERS = u"0123456789/-.";

DateKeyListener* DateKeyListener::sInstance = nullptr;

DateKeyListener* DateKeyListener::getInstance() {
    if (sInstance == nullptr) {
        sInstance = new DateKeyListener();
    }
    return sInstance;
}

int DateKeyListener::getInputType() const {
    // Compat path: mNeedsAdvancedInput is false, so the date datetime type.
    // (The locale-aware mNeedsAdvancedInput -> TYPE_CLASS_TEXT path is deferred.)
    return InputType::TYPE_CLASS_DATETIME | InputType::TYPE_DATETIME_VARIATION_DATE;
}

std::u16string DateKeyListener::getAcceptedChars() const {
    return CHARACTERS;
}

} // namespace cdroid
