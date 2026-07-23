/*
 * Ported from Android android.text.method.MultiTapKeyListener (Apache 2.0).
 *
 * Standard key listener for alphabetic input on 12-key (multi-tap) keyboards:
 * press KEYCODE_2 once for 'a', twice for 'b', three times for 'c', etc. The
 * sRecs table maps each digit key to its character cycle.
 *
 * STATUS: API surface + the sRecs table + getInstance/getInputType are ported.
 * The onKeyDown multi-tap state machine and the Timeout (2s inactivity timer,
 * which Android hangs off android.os.Handler and stores as a span — its
 * lifetime needs care in C++ since there's no GC) are DEFERRED (onKeyDown
 * currently delegates to BaseKeyListener). Wire when 12-key multi-tap input is
 * actually needed. Depends on TextKeyListener.{ACTIVE,LAST_TYPED,AUTO_CAP,
 * shouldCap,getPrefs} + BaseKeyListener.{OLD_SEL_START,makeTextContentType},
 * all of which CDROID has.
 */
#ifndef CDROID_MULTITAPKEYLISTENER_H
#define CDROID_MULTITAPKEYLISTENER_H

#include <text/method/basekeylistener.h>
#include <text/method/textkeylistener.h>
#include <text/spanwatcher.h>
#include <map>
#include <string>

namespace cdroid {

class MultiTapKeyListener : public BaseKeyListener, public SpanWatcher {
public:
    MultiTapKeyListener(TextKeyListener::Capitalize cap, bool autotext);

    static MultiTapKeyListener* getInstance(bool autotext, TextKeyListener::Capitalize cap);

    int getInputType() const override;
    bool onKeyDown(View& view, Editable& content, int keyCode, const KeyEvent& event) override;

    // SpanWatcher.
    void onSpanAdded(Spannable& s, const ParcelableSpan* what, int start, int end) override {}
    void onSpanRemoved(Spannable& s, const ParcelableSpan* what, int start, int end) override {}
    void onSpanChanged(Spannable& s, const ParcelableSpan* what,
            int ostart, int oend, int nstart, int nend) override;

    // The digit-key -> character-cycle table (Android's sRecs), keyed by keycode.
    static const std::map<int, std::u16string>& recs();

private:
    static MultiTapKeyListener* sInstance[];  // [Capitalize count * 2]
    TextKeyListener::Capitalize mCapitalize;
    bool mAutoText;
};

} // namespace cdroid

#endif // CDROID_MULTITAPKEYLISTENER_H
