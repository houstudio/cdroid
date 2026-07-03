/*
 * Ported from Android android.text.method.MetaKeyKeyListener
 * (Apache 2.0). Tracks SHIFT/ALT/SYM (and the pseudo-meta SELECTING) state,
 * stored as NoCopySpan markers (CAP/ALT/SYM/SELECTING) with PRESSED/RELEASED/
 * USED/LOCKED span flags inside the Editable buffer. Two APIs are provided,
 * matching Android: a span-based one (onKeyDown/onKeyUp/clearMetaKeyState and
 * the static getMetaState(CharSequence…)/adjustMetaAfterKeypress(Spannable))
 * and a pure long-bitmask one (handleKeyDown/getMetaState(long)/…).
 */
#ifndef CDROID_METAKEYLISTENER_H
#define CDROID_METAKEYLISTENER_H

#include <text/method/keylistener.h>
#include <text/parcelablespan.h>

namespace cdroid {

class View;
class Editable;
class Spannable;
class CharSequence;
class KeyEvent;

class MetaKeyKeyListener : public virtual KeyListener {
public:
    // Public meta flags — aliases of KeyEvent's, kept for API parity with Android.
    static const int META_SHIFT_ON;
    static const int META_ALT_ON;
    static const int META_SYM_ON;
    static const int META_CAP_LOCKED;
    static const int META_ALT_LOCKED;
    static const int META_SYM_LOCKED;
    static const int META_SELECTING;

    // --- Span-based API (state stored in the Editable) ---

    static void resetMetaState(Spannable& text);
    static int getMetaState(CharSequence& text);
    static int getMetaState(CharSequence& text, const KeyEvent& event);
    static int getMetaState(CharSequence& text, int meta);
    static int getMetaState(CharSequence& text, int meta, const KeyEvent& event);

    static void adjustMetaAfterKeypress(Spannable& content);
    static bool isMetaTracker(CharSequence& text, const ParcelableSpan* what);
    static bool isSelectingMetaTracker(CharSequence& text, const ParcelableSpan* what);

    // Span markers (identity-compared; Android uses NoCopySpan.Concrete).
    static const NoCopySpan* CAP;
    static const NoCopySpan* ALT;
    static const NoCopySpan* SYM;
    static const NoCopySpan* SELECTING;

    // --- KeyListener overrides: handle the meta keys themselves ---

    bool onKeyDown(View& view, Editable& content, int keyCode, const KeyEvent& event) override;
    bool onKeyUp(View& view, Editable& content, int keyCode, const KeyEvent& event) override;
    void clearMetaKeyState(View& view, Editable& content, int states) override;

protected:
    static void resetLockedMeta(Spannable& content);
    static void startSelecting(View& view, Spannable& content);
    static void stopSelecting(View& view, Spannable& content);
    static void clearMetaKeyState(Editable& content, int states);

private:
    static int getActive(CharSequence& text, const ParcelableSpan* meta, int on, int lock);
    static void adjust(Spannable& content, const ParcelableSpan* what);
    static void resetLock(Spannable& content, const ParcelableSpan* what);
    void press(Editable& content, const ParcelableSpan* what);
    void release(Editable& content, const ParcelableSpan* what, const KeyEvent& event);

    // Span flag values for the meta markers (Android's private PRESSED/RELEASED/
    // USED/LOCKED). SPAN_MARK_MARK | (n << SPAN_USER_SHIFT).
    static const int PRESSED;
    static const int RELEASED;
    static const int USED;
    static const int LOCKED;
    static const int PRESSED_RETURN_VALUE;
    static const int LOCKED_RETURN_VALUE;

    // --- Long-bitmask API (caller-managed state) ---

public:
    static const long META_CAP_USED;
    static const long META_ALT_USED;
    static const long META_SYM_USED;
    static const long META_CAP_PRESSED;
    static const long META_ALT_PRESSED;
    static const long META_SYM_PRESSED;
    static const long META_CAP_RELEASED;
    static const long META_ALT_RELEASED;
    static const long META_SYM_RELEASED;
    static const long META_SHIFT_MASK;
    static const long META_ALT_MASK;
    static const long META_SYM_MASK;

    static int getMetaState(long state);
    static int getMetaState(long state, int meta);
    static long adjustMetaAfterKeypress(long state);
    static long handleKeyDown(long state, int keyCode, const KeyEvent& event);
    static long handleKeyUp(long state, int keyCode, const KeyEvent& event);
    long clearMetaKeyState(long state, int which);
    static long resetLockedMeta(long state);

private:
    static long press(long state, int what, long mask, long locked,
                      long pressed, long released, long used);
    static long release(long state, int what, long mask,
                        long pressed, long released, long used, const KeyEvent& event);
};

} // namespace cdroid

#endif // CDROID_METAKEYLISTENER_H
