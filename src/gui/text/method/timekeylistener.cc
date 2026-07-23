/*
 * Ported from Android android.text.method.TimeKeyListener (Apache 2.0).
 * Compatibility (locale=null) path only; see header.
 */
#include <text/method/timekeylistener.h>
#include <text/inputtype.h>

namespace cdroid {

// Android CHARACTERS = { '0'..'9', 'a', 'm', 'p', ':' }
const std::u16string TimeKeyListener::CHARACTERS = u"0123456789amp:";

TimeKeyListener* TimeKeyListener::sInstance = nullptr;

TimeKeyListener* TimeKeyListener::getInstance() {
    if (sInstance == nullptr) {
        sInstance = new TimeKeyListener();
    }
    return sInstance;
}

int TimeKeyListener::getInputType() const {
    // Compat path: mNeedsAdvancedInput is false, so the time datetime type.
    // (The locale-aware mNeedsAdvancedInput -> TYPE_CLASS_TEXT path is deferred.)
    return InputType::TYPE_CLASS_DATETIME | InputType::TYPE_DATETIME_VARIATION_TIME;
}

std::u16string TimeKeyListener::getAcceptedChars() const {
    return CHARACTERS;
}

} // namespace cdroid
