/*
 * Ported from Android android.text.method.DigitsKeyListener (Apache 2.0).
 * Phase-1: the compatibility (locale=null) path — ASCII digits 0-9, optionally
 * '+'/'-' sign and/or '.' decimal point — which is exactly what EditText with
 * TYPE_CLASS_NUMBER uses. The locale-aware path (DecimalFormatSymbols /
 * NumberKeyListener.addDigits) defers with the Date/Time/Dialer listeners.
 */
#ifndef CDROID_DIGITSKEYLISTENER_H
#define CDROID_DIGITSKEYLISTENER_H

#include <text/method/numberkeylistener.h>
#include <string>

namespace cdroid {

class DigitsKeyListener : public NumberKeyListener {
public:
    DigitsKeyListener();
    DigitsKeyListener(bool sign, bool decimal);
    explicit DigitsKeyListener(const std::u16string& accepted);

    static DigitsKeyListener* getInstance();
    static DigitsKeyListener* getInstance(bool sign, bool decimal);
    static DigitsKeyListener* getInstance(const std::u16string& accepted);

    int getInputType() const override;
    std::u16string getAcceptedChars() const override;
    CharSequence* filter(CharSequence* source, int start, int end,
                         Spanned* dest, int dstart, int dend) override;

private:
    bool isSignChar(char16_t c) const;
    bool isDecimalPointChar(char16_t c) const;

    std::u16string mAccepted;
    const bool mSign;
    const bool mDecimal;
    const bool mStringMode;
    std::u16string mDecimalPointChars;   // "."
    std::u16string mSignChars;           // "-+"
};

} // namespace cdroid

#endif // CDROID_DIGITSKEYLISTENER_H
