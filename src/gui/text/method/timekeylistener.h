/*
 * Ported from Android android.text.method.TimeKeyListener (Apache 2.0).
 *
 * For entering times in a text field.
 *
 * Phase-1: the compatibility (locale=null) path — Android's deprecated no-arg
 * ctor / getInstance(null), using the static CHARACTERS set (digits + am/pm +
 * ':'). The locale-aware path (NumberKeyListener.addDigits / addAmPmChars /
 * addFormatCharsFromSkeleton, the mNeedsAdvancedInput -> TYPE_CLASS_TEXT
 * fallback) defers with the rest of the locale-aware NumberKeyListener work;
 * see numberkeylistener.h.
 *
 * As with all KeyListener implementations, only hardware keyboards are concerned.
 */
#ifndef CDROID_TIMEKEYLISTENER_H
#define CDROID_TIMEKEYLISTENER_H

#include <text/method/numberkeylistener.h>
#include <string>

namespace cdroid {

class TimeKeyListener : public NumberKeyListener {
public:
    // The characters accepted by a time field (Android: CHARACTERS).
    static const std::u16string CHARACTERS;

    // Cached singleton (Android's sInstance; never deleted).
    static TimeKeyListener* getInstance();

    int getInputType() const override;
    std::u16string getAcceptedChars() const override;

private:
    static TimeKeyListener* sInstance;
};

} // namespace cdroid

#endif // CDROID_TIMEKEYLISTENER_H
