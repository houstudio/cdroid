/*
 * Ported from Android android.text.method.DialerKeyListener (Apache 2.0).
 *
 * For dialing-only text entry. Accepts the dialer character set (digits plus the
 * phone symbols) and, via the overridden lookup(), prefers the key's NUMBER label
 * — so on a keyboard where an alphabetic key doubles as a phone digit (ALT-Q →
 * '1' etc.) the digit is typed without forcing the modifier. That number-preferring
 * lookup is what distinguishes DialerKeyListener from DigitsKeyListener.
 *
 * As with all KeyListener implementations, this class only concerns itself with
 * hardware keyboards; software input methods have no obligation to trigger it.
 */
#ifndef CDROID_DIALERKEYLISTENER_H
#define CDROID_DIALERKEYLISTENER_H

#include <text/method/numberkeylistener.h>
#include <string>

namespace cdroid {

class DialerKeyListener : public NumberKeyListener {
public:
    // The characters accepted by a dialer field (Android: CHARACTERS).
    static const std::u16string CHARACTERS;

    // Cached singleton (Android's sInstance; never deleted).
    static DialerKeyListener* getInstance();

    int getInputType() const override;
    std::u16string getAcceptedChars() const override;

protected:
    // Android DialerKeyListener.lookup: prefer the key's number when no
    // ALT/SHIFT is active, else the accepted-char match; if a meta is active but
    // yields nothing, try the other meta variants via the key's KeyData; finally
    // fall back to the number.
    int lookup(const KeyEvent& event, Spannable& content) override;

private:
    static DialerKeyListener* sInstance;
};

} // namespace cdroid

#endif // CDROID_DIALERKEYLISTENER_H
