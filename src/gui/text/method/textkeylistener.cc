/*
 * Ported from Android android.text.method.TextKeyListener (Apache 2.0).
 * See header for Phase-1 deferrals (SpanWatcher, SettingsObserver, multi-tap).
 */
#include <text/method/textkeylistener.h>
#include <text/method/qwertykeylistener.h>
#include <text/editable.h>
#include <text/selection.h>
#include <text/textutils.h>
#include <text/inputtype.h>
#include <view/keyevent.h>

namespace cdroid {

// --- span markers + pref bits ---
const NoCopySpan* TextKeyListener::ACTIVE             = new NoCopySpan();
const NoCopySpan* TextKeyListener::CAPPED             = new NoCopySpan();
const NoCopySpan* TextKeyListener::INHIBIT_REPLACEMENT= new NoCopySpan();
const NoCopySpan* TextKeyListener::LAST_TYPED         = new NoCopySpan();

const int TextKeyListener::AUTO_CAP     = 1;
const int TextKeyListener::AUTO_TEXT    = 2;
const int TextKeyListener::AUTO_PERIOD  = 4;
const int TextKeyListener::SHOW_PASSWORD= 8;

namespace {
// Android's TextKeyListener.NullKeyListener: handles nothing.
class NullKeyListener : public KeyListener {
public:
    int getInputType() const override { return InputType::TYPE_NULL; }
    bool onKeyDown(View&, Editable&, int, const KeyEvent&) override { return false; }
    bool onKeyUp(View&, Editable&, int, const KeyEvent&) override { return false; }
    bool onKeyOther(View&, Editable&, const KeyEvent&) override { return false; }
    void clearMetaKeyState(View&, Editable&, int) override {}
    static NullKeyListener* getInstance() {
        static NullKeyListener sInstance;
        return &sInstance;
    }
};
} // namespace

TextKeyListener::TextKeyListener(Capitalize cap, bool autotext)
    : mAutoCap(cap), mAutoText(autotext) {
}

TextKeyListener* TextKeyListener::getInstance(bool autotext, Capitalize cap) {
    static TextKeyListener* sInstance[8] = { nullptr, nullptr, nullptr, nullptr,
                                             nullptr, nullptr, nullptr, nullptr };
    const int off = (int)cap * 2 + (autotext ? 1 : 0);
    if (sInstance[off] == nullptr) {
        sInstance[off] = new TextKeyListener(cap, autotext);
    }
    return sInstance[off];
}

TextKeyListener* TextKeyListener::getInstance() {
    return getInstance(false, Capitalize::NONE);
}

bool TextKeyListener::shouldCap(Capitalize cap, CharSequence& cs, int off) {
    if (cap == Capitalize::NONE) {
        return false;
    }
    if (cap == Capitalize::CHARACTERS) {
        return true;
    }
    // NOTE: CDROID's TextUtils::getCapsMode is currently a stub (#if 0 body), so
    // WORDS/SENTENCES cap detection is not yet effective — but this path is only
    // reached when mAutoCap != NONE, which the full-keyboard listener never sets.
    return TextUtils::getCapsMode(&cs, off,
            cap == Capitalize::WORDS ? TextUtils::CAP_MODE_WORDS
                                     : TextUtils::CAP_MODE_SENTENCES) != 0;
}

void TextKeyListener::clear(Editable& e) {
    e.clear();
    e.removeSpan(ACTIVE);
    e.removeSpan(CAPPED);
    e.removeSpan(INHIBIT_REPLACEMENT);
    e.removeSpan(LAST_TYPED);
    // QwertyKeyListener.Replaced spans deferred (autotext not wired).
}

int TextKeyListener::getInputType() const {
    return makeTextContentType(mAutoCap, mAutoText);
}

int TextKeyListener::getPrefs() {
    // CDROID has no Settings.System; return the stock-device default (all on).
    return AUTO_CAP | AUTO_TEXT | AUTO_PERIOD | SHOW_PASSWORD;
}

KeyListener* TextKeyListener::getKeyListener(const KeyEvent& /*event*/) {
    // Android branches on event.getKeyCharacterMap().getKeyboardType(): ALPHA→
    // QwertyKeyListener, NUMERIC→MultiTapKeyListener, FULL/SPECIAL_FUNCTION→
    // QwertyKeyListener.getInstanceForFullKeyboard(). CDROID can't query the
    // per-event keyboard type and runs on a full keyboard, so always use the
    // full-keyboard Qwerty listener (the desktop case). ALPHA/NUMERIC/multi-tap
    // detection is deferred with MultiTapKeyListener.
    return QwertyKeyListener::getInstanceForFullKeyboard();
}

bool TextKeyListener::onKeyDown(View& view, Editable& content, int keyCode, const KeyEvent& event) {
    KeyListener* im = getKeyListener(event);
    return im->onKeyDown(view, content, keyCode, event);
}

bool TextKeyListener::onKeyUp(View& view, Editable& content, int keyCode, const KeyEvent& event) {
    KeyListener* im = getKeyListener(event);
    return im->onKeyUp(view, content, keyCode, event);
}

bool TextKeyListener::onKeyOther(View& view, Editable& content, const KeyEvent& event) {
    KeyListener* im = getKeyListener(event);
    return im->onKeyOther(view, content, event);
}

} // namespace cdroid
