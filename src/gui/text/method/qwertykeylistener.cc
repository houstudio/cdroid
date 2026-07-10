/*
 * Ported from Android android.text.method.QwertyKeyListener (Apache 2.0).
 * See header for Phase-1 deferrals.
 */
#include <text/method/qwertykeylistener.h>
#include <text/method/textkeylistener.h>
#include <text/editable.h>
#include <text/spannablestringbuilder.h>
#include <text/selection.h>
#include <text/character.h>
#include <view/keyevent.h>
#include <algorithm>

namespace cdroid {

QwertyKeyListener::QwertyKeyListener(Capitalize cap, bool autoText, bool fullKeyboard)
    : mAutoCap(cap), mAutoText(autoText), mFullKeyboard(fullKeyboard) {
}

QwertyKeyListener* QwertyKeyListener::getInstance(bool autoText, Capitalize cap) {
    static QwertyKeyListener* sInstance[8] = { nullptr, nullptr, nullptr, nullptr,
                                               nullptr, nullptr, nullptr, nullptr };
    const int off = (int)cap * 2 + (autoText ? 1 : 0);
    if (sInstance[off] == nullptr) {
        sInstance[off] = new QwertyKeyListener(cap, autoText, false);
    }
    return sInstance[off];
}

QwertyKeyListener* QwertyKeyListener::getInstanceForFullKeyboard() {
    // Disables auto-capitalization, auto-text and character pickers — the desktop
    // case (Android's FULL / SPECIAL_FUNCTION keyboard branch).
    static QwertyKeyListener* sFullKeyboardInstance = nullptr;
    if (sFullKeyboardInstance == nullptr) {
        sFullKeyboardInstance = new QwertyKeyListener(Capitalize::NONE, false, true);
    }
    return sFullKeyboardInstance;
}

int QwertyKeyListener::getInputType() const {
    return makeTextContentType(mAutoCap, mAutoText);
}

bool QwertyKeyListener::onKeyDown(View& view, Editable& content, int keyCode, const KeyEvent& event) {
    int selStart, selEnd;
    int pref = 0;

    // Android: pref = TextKeyListener.getInstance().getPrefs(view.getContext()).
    // CDROID's getPrefs() takes no Context and returns the stock-device default.
    pref = TextKeyListener::getInstance()->getPrefs();

    {
        const int a = Selection::getSelectionStart(&content);
        const int b = Selection::getSelectionEnd(&content);
        selStart = std::min(a, b);
        selEnd = std::max(a, b);
        if (selStart < 0 || selEnd < 0) {
            selStart = selEnd = 0;
            Selection::setSelection(&content, 0, 0);
        }
    }

    const int activeStart = content.getSpanStart(TextKeyListener::ACTIVE);
    const int activeEnd   = content.getSpanEnd(TextKeyListener::ACTIVE);

    // QWERTY keyboard normal case.
    int i = event.getUnicodeChar(getMetaState(content, event));

    // Deferred (not entered on desktop): full-keyboard character-picker repeat
    // (mFullKeyboard), PICKER_DIALOG_INPUT, HEX_INPUT, dead-key COMBINING_ACCENT.

    if (i != 0) {
        // The ACTIVE composition block only runs in the dead-key path (deferred),
        // where activeStart/End == selStart/End; here they are -1 so this is inert.
        if (activeStart == selStart && activeEnd == selEnd) {
            // Dead-key compose would go here (event.getDeadChar) — skipped.
            Selection::setSelection(&content, selEnd);
            content.removeSpan(TextKeyListener::ACTIVE);
            selStart = selEnd;
        }

        // Auto-capitalize.
        if ((pref & TextKeyListener::AUTO_CAP) != 0
                && Character::isLowerCase((char16_t)i)
                && TextKeyListener::shouldCap(mAutoCap, content, selStart)) {
            int where = content.getSpanEnd(TextKeyListener::CAPPED);
            int flags = content.getSpanFlags(TextKeyListener::CAPPED);

            if (where == selStart && (((flags >> 16) & 0xFFFF) == i)) {
                content.removeSpan(TextKeyListener::CAPPED);
            } else {
                flags = i << 16;
                i = Character::toUpperCase((char16_t)i);

                if (selStart == 0) {
                    content.setSpan(TextKeyListener::CAPPED, 0, 0,
                                    Spanned::SPAN_MARK_MARK | flags);
                } else {
                    content.setSpan(TextKeyListener::CAPPED, selStart - 1, selStart,
                                    Spanned::SPAN_EXCLUSIVE_EXCLUSIVE | flags);
                }
            }
        }

        if (selStart != selEnd) {
            Selection::setSelection(&content, selEnd);
        }
        content.setSpan(OLD_SEL_START, selStart, selStart, Spanned::SPAN_MARK_MARK);

        content.replace(selStart, selEnd, SpannableStringBuilder(std::u16string(1, (char16_t)i)));

        // CDROID's adjustSpansForReplace does not advance a cursor at the insertion
        // offset (unlike Android's SpannableStringBuilder), so move the caret to
        // after the inserted character explicitly.
        Selection::setSelection(&content, selStart + 1);

        const int oldStart = content.getSpanStart(OLD_SEL_START);   // MARK_MARK → stays at selStart
        selEnd = Selection::getSelectionEnd(&content);              // == selStart + 1

        if (oldStart < selEnd) {
            content.setSpan(TextKeyListener::LAST_TYPED, oldStart, selEnd,
                            Spanned::SPAN_EXCLUSIVE_EXCLUSIVE);
            // dead-key ACTIVE span deferred
        }

        adjustMetaAfterKeypress(content);

        // AutoText replacement deferred (no AutoText dictionary).

        // Replace two spaces by a period and a space.
        if ((pref & TextKeyListener::AUTO_PERIOD) != 0 && mAutoText) {
            selEnd = Selection::getSelectionEnd(&content);
            if (selEnd - 3 >= 0) {
                if (content.charAt(selEnd - 1) == ' ' && content.charAt(selEnd - 2) == ' ') {
                    char16_t c = (char16_t)content.charAt(selEnd - 3);

                    for (int j = selEnd - 3; j > 0; j--) {
                        if (c == '"' ||
                            Character::getType(c) == Character::END_PUNCTUATION) {
                            c = (char16_t)content.charAt(j - 1);
                        } else {
                            break;
                        }
                    }

                    if (Character::isLetter(c) || Character::isDigit(c)) {
                        content.replace(selEnd - 2, selEnd - 1,
                                        SpannableStringBuilder(std::u16string(1, u'.')));
                    }
                }
            }
        }

        return true;
    } else if (keyCode == KeyEvent::KEYCODE_DEL
            && (event.hasNoModifiers() || event.hasModifiers(KeyEvent::META_ALT_ON))
            && selStart == selEnd) {
        // Special backspace case for undoing autotext. With AutoText deferred no
        // Replaced spans exist, so this always falls through to super (BaseKeyListener).
        return BaseKeyListener::onKeyDown(view, content, keyCode, event);
    } else if (keyCode == KeyEvent::KEYCODE_ESCAPE && event.hasNoModifiers()) {
        // Cancel dead-key composition — inert (no ACTIVE span without dead keys).
        if (activeStart == selStart && activeEnd == selEnd) {
            Selection::setSelection(&content, selEnd);
            content.removeSpan(TextKeyListener::ACTIVE);
            return true;
        }
    }

    // No character produced (BACKSPACE/DEL/navigation/meta) — defer to BaseKeyListener.
    return BaseKeyListener::onKeyDown(view, content, keyCode, event);
}

} // namespace cdroid
