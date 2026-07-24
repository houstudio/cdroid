/*
 * Ported from Android android.text.method.MultiTapKeyListener (Apache 2.0).
 *
 * Full port: the sRecs multi-tap table, the onKeyDown state machine (cycle the
 * active char within the key's char cycle, track ACTIVE/LAST_TYPED/OLD_SEL_START
 * spans, store the sRecs index in the ACTIVE span's SPAN_USER bits), and the
 * Timeout (2s inactivity timer). CDROID adaptation: Android's Timeout extends
 * android.os.Handler; CDROID has no Handler, so Timeout holds the View* passed
 * to onKeyDown and arms/cancels via View::postDelayed/removeCallbacks (the same
 * mechanism Editor's Blink uses). Timeout is stored as a NoCopySpan (borrowed)
 * and is deleted either when its timer fires (run) or when removeTimeouts cancels
 * it — safe single-threaded, since removeCallbacks cancels the pending fire.
 */
#include <text/method/multitapkeylistener.h>
#include <text/method/textkeylistener.h>
#include <text/selection.h>
#include <text/spannablestring.h>      // Spanned constants + SpannedString
#include <text/spannablestringbuilder.h>
#include <text/character.h>
#include <text/parcelablespan.h>
#include <view/keyevent.h>
#include <view/view.h>
#include <core/callbackbase.h>
#include <vector>

namespace cdroid {
namespace {

// Android's sRecs as an ordered vector so indexOfKey/valueAt give stable indices
// (Android's SparseArray is key-sorted; we keep insertion order and provide our
// own indexOf/valueAt, which is internally consistent).
const std::vector<std::pair<int, std::u16string>>& sRecsVec() {
    static const std::vector<std::pair<int, std::u16string>> sRecs = {
        { KeyEvent::KEYCODE_1,     u".,1!@#$%^&*:/?'=()" },
        { KeyEvent::KEYCODE_2,     u"abc2ABC" },
        { KeyEvent::KEYCODE_3,     u"def3DEF" },
        { KeyEvent::KEYCODE_4,     u"ghi4GHI" },
        { KeyEvent::KEYCODE_5,     u"jkl5JKL" },
        { KeyEvent::KEYCODE_6,     u"mno6MNO" },
        { KeyEvent::KEYCODE_7,     u"pqrs7PQRS" },
        { KeyEvent::KEYCODE_8,     u"tuv8TUV" },
        { KeyEvent::KEYCODE_9,     u"wxyz9WXYZ" },
        { KeyEvent::KEYCODE_0,     u"0+" },
        { KeyEvent::KEYCODE_POUND, u" " },
    };
    return sRecs;
}

int recsIndexOfKey(int keyCode) {
    const auto& r = sRecsVec();
    for (size_t i = 0; i < r.size(); i++) if (r[i].first == keyCode) return (int)i;
    return -1;
}
const std::u16string& recsValueAt(int i) { return sRecsVec().at(i).second; }

// A 2s inactivity timer for the active multi-tap session. Borrowed span (NoCopySpan):
// the Spannable never frees it; it is deleted in run() (timer fired) or in
// removeTimeouts (cancelled). Single-threaded + removeCallbacks-cancel makes that safe.
class Timeout : public NoCopySpan {
public:
    Timeout(Editable* buffer, View* view) : mBuffer(buffer), mView(view) {
        mBuffer->setSpan(this, 0, (int)mBuffer->length(), Spanned::SPAN_INCLUSIVE_INCLUSIVE);
        mRunnable = [this]() -> bool { run(); return true; };
        if (mView != nullptr) mView->postDelayed(mRunnable, 2000);
    }

    // Cancel the pending fire (if any), detach from the buffer, and free. Idempotent
    // via mBuffer==nullptr (set here so a late fire no-ops — though removeCallbacks
    // already cancelled it).
    void cancel() {
        if (mView != nullptr) mView->removeCallbacks(mRunnable);
        mBuffer = nullptr;
    }

    void run() {
        Editable* buf = mBuffer;
        mBuffer = nullptr;
        if (buf != nullptr) {
            const int st = Selection::getSelectionStart(buf);
            const int en = Selection::getSelectionEnd(buf);
            const int start = buf->getSpanStart(TextKeyListener::ACTIVE);
            const int end = buf->getSpanEnd(TextKeyListener::ACTIVE);
            if (st == start && en == end) {
                Selection::setSelection(buf, Selection::getSelectionEnd(buf));
            }
            buf->removeSpan(this);
        }
        delete this;   // owned nowhere else (borrowed span); single-threaded, fire already cancelled if removed
    }

private:
    Editable* mBuffer;
    View* mView;
    Runnable mRunnable;
};

void removeTimeouts(Spannable* buf) {
    auto timeouts = buf->getSpans(0, (int)buf->length(), make_span_filter<Timeout>());
    for (const ParcelableSpan* p : timeouts) {
        Timeout* t = const_cast<Timeout*>(dynamic_cast<const Timeout*>(p));
        if (t != nullptr) {
            t->cancel();
            buf->removeSpan(t);
            delete t;
        }
    }
}

// Build a fresh single-char CharSequence (owned) for Editable::replace.
SpannedString* oneChar(char16_t ch) {
    return new SpannedString(std::u16string(1, ch));
}

} // namespace

MultiTapKeyListener* MultiTapKeyListener::sInstance[8] = {};

MultiTapKeyListener::MultiTapKeyListener(TextKeyListener::Capitalize cap, bool autotext)
    : mCapitalize(cap), mAutoText(autotext) {
}

MultiTapKeyListener* MultiTapKeyListener::getInstance(bool autotext, TextKeyListener::Capitalize cap) {
    using Cap = TextKeyListener::Capitalize;
    const int capOff = (cap == Cap::CHARACTERS) ? 3
                     : (cap == Cap::WORDS)      ? 2
                     : (cap == Cap::SENTENCES)  ? 1 : 0;
    const int off = capOff * 2 + (autotext ? 1 : 0);
    if (sInstance[off] == nullptr) {
        sInstance[off] = new MultiTapKeyListener(cap, autotext);
    }
    return sInstance[off];
}

int MultiTapKeyListener::getInputType() const {
    return BaseKeyListener::makeTextContentType(mCapitalize, mAutoText);
}

bool MultiTapKeyListener::onKeyDown(View& view, Editable& content, int keyCode, const KeyEvent& event) {
    int pref = 0;
    // Android: TextKeyListener.getInstance().getPrefs(view.getContext()); CDROID's
    // getPrefs() returns the Android default bitmask (no ContentResolver).
    pref = TextKeyListener::getInstance(false, TextKeyListener::Capitalize::NONE)->getPrefs();

    int a = Selection::getSelectionStart(&content);
    int b = Selection::getSelectionEnd(&content);
    int selStart = std::min(a, b);
    int selEnd = std::max(a, b);

    int activeStart = content.getSpanStart(TextKeyListener::ACTIVE);
    int activeEnd = content.getSpanEnd(TextKeyListener::ACTIVE);

    int rec = (content.getSpanFlags(TextKeyListener::ACTIVE) & Spanned::SPAN_USER) >> Spanned::SPAN_USER_SHIFT;

    // Try to increment the char we were working on, if same key & still active.
    if (activeStart == selStart && activeEnd == selEnd && selEnd - selStart == 1
            && rec >= 0 && rec < (int)sRecsVec().size()) {
        if (keyCode == KeyEvent::KEYCODE_STAR) {
            const char16_t current = (char16_t)content.charAt(selStart);
            if (Character::isLowerCase(current)) {
                SpannedString* up = oneChar(Character::toUpperCase(current));
                content.replace(selStart, selEnd, *up);
                delete up;
                removeTimeouts(&content);
                new Timeout(&content, &view);
                return true;
            }
            if (Character::isUpperCase(current)) {
                SpannedString* down = oneChar(Character::toLowerCase(current));
                content.replace(selStart, selEnd, *down);
                delete down;
                removeTimeouts(&content);
                new Timeout(&content, &view);
                return true;
            }
        }

        if (recsIndexOfKey(keyCode) == rec) {
            const std::u16string& val = recsValueAt(rec);
            const char16_t ch = (char16_t)content.charAt(selStart);
            int ix = (int)val.find(ch);
            if (ix >= 0) {
                ix = (ix + 1) % (int)val.length();
                SpannedString* rep = new SpannedString(val);
                content.replace(selStart, selEnd, *rep, ix, ix + 1);
                delete rep;
                removeTimeouts(&content);
                new Timeout(&content, &view);
                return true;
            }
        }

        // Known key but changed/mismatched: move selection so next press inserts.
        rec = recsIndexOfKey(keyCode);
        if (rec >= 0) {
            Selection::setSelection(&content, selEnd, selEnd);
            selStart = selEnd;
        }
    } else {
        rec = recsIndexOfKey(keyCode);
    }

    if (rec >= 0) {
        const std::u16string& val = recsValueAt(rec);

        int off = 0;
        if ((pref & TextKeyListener::AUTO_CAP) != 0
                && TextKeyListener::shouldCap(mCapitalize, content, selStart)) {
            for (int i = 0; i < (int)val.length(); i++) {
                if (Character::isUpperCase(val[i])) { off = i; break; }
            }
        }

        if (selStart != selEnd) {
            Selection::setSelection(&content, selEnd);
        }

        content.setSpan(BaseKeyListener::OLD_SEL_START, selStart, selStart, Spanned::SPAN_MARK_MARK);

        SpannedString* first = new SpannedString(val);
        content.replace(selStart, selEnd, *first, off, off + 1);
        delete first;

        int oldStart = content.getSpanStart(BaseKeyListener::OLD_SEL_START);
        selEnd = Selection::getSelectionEnd(&content);

        if (selEnd != oldStart) {
            Selection::setSelection(&content, oldStart, selEnd);
            content.setSpan(TextKeyListener::LAST_TYPED, oldStart, selEnd, Spanned::SPAN_EXCLUSIVE_EXCLUSIVE);
            content.setSpan(TextKeyListener::ACTIVE, oldStart, selEnd,
                    Spanned::SPAN_EXCLUSIVE_EXCLUSIVE | (rec << Spanned::SPAN_USER_SHIFT));
        }

        removeTimeouts(&content);
        new Timeout(&content, &view);

        // Attach ourselves as a SpanWatcher so we can clear the active session on
        // cursor move. Remove any other KeyListener spans first (only one active).
        if (content.getSpanStart(this) < 0) {
            auto methods = content.getSpans(0, (int)content.length(), make_span_filter<KeyListener>());
            for (const ParcelableSpan* m : methods) {
                content.removeSpan(m);
            }
            content.setSpan(this, 0, (int)content.length(), Spanned::SPAN_INCLUSIVE_INCLUSIVE);
        }
        return true;
    }

    return BaseKeyListener::onKeyDown(view, content, keyCode, event);
}

void MultiTapKeyListener::onSpanChanged(Spannable& s, const ParcelableSpan* what,
        int /*ostart*/, int /*oend*/, int /*nstart*/, int /*nend*/) {
    if (what == Selection::SELECTION_END) {
        s.removeSpan(TextKeyListener::ACTIVE);
        removeTimeouts(&s);
    }
}

} // namespace cdroid
