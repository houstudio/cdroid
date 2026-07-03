/*
 * Ported from Android android.text.method.NumberKeyListener (Apache 2.0).
 * Abstract base for numeric key listeners: resolves a key to one of the
 * subclass-supplied accepted characters (lookup → KeyEvent.getMatch) and
 * inserts it; also an InputFilter that strips non-accepted chars.
 *
 * The locale-aware static helpers (addDigits/addFormatChars…, which pull in
 * DecimalFormatSymbols/DateFormat) are deferred with the Date/Time/Dialer
 * listeners (Phase 2); DigitsKeyListener builds its accepted set directly.
 */
#ifndef CDROID_NUMBERKEYLISTENER_H
#define CDROID_NUMBERKEYLISTENER_H

#include <text/method/basekeylistener.h>
#include <text/inputfilter.h>

namespace cdroid {

class Spannable;

class NumberKeyListener : public BaseKeyListener, public InputFilter {
public:
    // KeyListener: resolve the key to an accepted char and insert it.
    bool onKeyDown(View& view, Editable& content, int keyCode, const KeyEvent& event) override;

    // InputFilter: strip characters not in the accepted set.
    CharSequence* filter(CharSequence* source, int start, int end,
                         Spanned* dest, int dstart, int dend) override;

    // The characters this listener accepts (subclass-supplied).
    virtual std::u16string getAcceptedChars() const = 0;

protected:
    int lookup(const KeyEvent& event, Spannable& content);
    static bool ok(const std::u16string& accept, char16_t c);
};

} // namespace cdroid

#endif // CDROID_NUMBERKEYLISTENER_H
