/*
 * Ported from Android android.text.method.TextKeyListener (Apache 2.0).
 * The key listener for normal text: delegates per key event to the keyboard-
 * appropriate listener (QwertyKeyListener on a full/alpha keyboard).
 *
 * Phase-1 deferrals: SpanWatcher (onSpanChanged clears the ACTIVE dead-key span
 * — dead keys aren't wired), the SettingsObserver/getPrefs(Context) (CDROID has
 * no ContentResolver — getPrefs() returns the Android default bitmask), and the
 * multi-tap NUMERIC branch (getKeyListener always uses the full-keyboard Qwerty
 * listener, which is the desktop case).
 */
#ifndef CDROID_TEXTKEYLISTENER_H
#define CDROID_TEXTKEYLISTENER_H

#include <text/method/basekeylistener.h>

namespace cdroid {

class QwertyKeyListener;

class TextKeyListener : public BaseKeyListener {
public:
    // Faithful alias for Android's TextKeyListener.Capitalize (hosted on
    // BaseKeyListener to avoid a header cycle).
    using Capitalize = BaseKeyListener::Capitalize;

    // Span markers used by the QwertyKeyListener composition/autocap machinery.
    static const NoCopySpan* ACTIVE;
    static const NoCopySpan* CAPPED;
    static const NoCopySpan* INHIBIT_REPLACEMENT;
    static const NoCopySpan* LAST_TYPED;

    // Preference bits (Android package-private constants).
    static const int AUTO_CAP;
    static const int AUTO_TEXT;
    static const int AUTO_PERIOD;
    static const int SHOW_PASSWORD;

    TextKeyListener(Capitalize cap, bool autotext);

    static TextKeyListener* getInstance(bool autotext, Capitalize cap);
    static TextKeyListener* getInstance();

    static bool shouldCap(Capitalize cap, CharSequence& cs, int off);
    static void clear(Editable& e);

    int getInputType() const override;
    bool onKeyDown(View& view, Editable& content, int keyCode, const KeyEvent& event) override;
    bool onKeyUp(View& view, Editable& content, int keyCode, const KeyEvent& event) override;
    bool onKeyOther(View& view, Editable& content, const KeyEvent& event) override;

    // CDROID has no Settings.System; returns the stock-device default bitmask.
    int getPrefs();

private:
    KeyListener* getKeyListener(const KeyEvent& event);

    Capitalize mAutoCap;
    bool mAutoText;
};

} // namespace cdroid

#endif // CDROID_TEXTKEYLISTENER_H
