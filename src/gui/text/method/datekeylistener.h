/*
 * Ported from Android android.text.method.DateKeyListener (Apache 2.0).
 *
 * For entering dates in a text field.
 *
 * Phase-1: the compatibility (locale=null) path — Android's deprecated no-arg
 * ctor / getInstance(null), using the static CHARACTERS set (digits + the date
 * separators '/', '-', '.'). The locale-aware path (NumberKeyListener.addDigits /
 * addFormatCharsFromSkeleton, the mNeedsAdvancedInput -> TYPE_CLASS_TEXT fallback)
 * defers with the rest of the locale-aware NumberKeyListener work; see
 * numberkeylistener.h.
 *
 * As with all KeyListener implementations, only hardware keyboards are concerned.
 */
#ifndef CDROID_DATEKEYLISTENER_H
#define CDROID_DATEKEYLISTENER_H

#include <text/method/numberkeylistener.h>
#include <string>

namespace cdroid {

class DateKeyListener : public NumberKeyListener {
public:
    // The characters accepted by a date field (Android: CHARACTERS).
    static const std::u16string CHARACTERS;

    // Cached singleton (Android's sInstance; never deleted).
    static DateKeyListener* getInstance();

    int getInputType() const override;
    std::u16string getAcceptedChars() const override;

private:
    static DateKeyListener* sInstance;
};

} // namespace cdroid

#endif // CDROID_DATEKEYLISTENER_H
