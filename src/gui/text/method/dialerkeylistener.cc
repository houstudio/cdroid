/*
 * Ported from Android android.text.method.DialerKeyListener (Apache 2.0).
 */
#include <text/method/dialerkeylistener.h>
#include <text/method/metakeylistener.h>
#include <text/inputtype.h>
#include <view/keyevent.h>

namespace cdroid {

// Android CHARACTERS = { '0'..'9', '#', '*', '+', '-', '(', ')', ',', '/', 'N',
//                        '.', ' ', ';' } — the dialer/symbol set.
const std::u16string DialerKeyListener::CHARACTERS = u"0123456789#*+-(),/N. ;";

DialerKeyListener* DialerKeyListener::sInstance = nullptr;

DialerKeyListener* DialerKeyListener::getInstance() {
    if (sInstance == nullptr) {
        sInstance = new DialerKeyListener();
    }
    return sInstance;
}

int DialerKeyListener::getInputType() const {
    return InputType::TYPE_CLASS_PHONE;
}

std::u16string DialerKeyListener::getAcceptedChars() const {
    return CHARACTERS;
}

int DialerKeyListener::lookup(const KeyEvent& event, Spannable& content) {
    const int meta = getMetaState(content, event);
    const char16_t number = event.getNumber();

    /*
     * Prefer number if no meta key is active, or if it produces something
     * valid and the meta lookup does not.
     */
    if ((meta & (MetaKeyKeyListener::META_ALT_ON | MetaKeyKeyListener::META_SHIFT_ON)) == 0) {
        if (number != 0) {
            return number;
        }
    }

    const int match = NumberKeyListener::lookup(event, content);
    if (match != 0) {
        return match;
    }

    /*
     * If a meta key is active but the lookup with the meta key did not produce
     * anything, try some other meta keys, because the user might have pressed
     * SHIFT when they meant ALT, or vice versa.
     */
    if (meta != 0) {
        const std::u16string accepted = getAcceptedChars();
        KeyEvent::KeyData kd;
        if (event.getKeyData(kd)) {
            for (int i = 1; i < KeyEvent::KeyData::META_LENGTH; i++) {
                if (ok(accepted, kd.meta[i])) {
                    return kd.meta[i];
                }
            }
        }
    }

    /*
     * Otherwise, use the number associated with the key, since whatever they
     * wanted to do with the meta key does not seem to be valid here.
     */
    return number;
}

} // namespace cdroid
