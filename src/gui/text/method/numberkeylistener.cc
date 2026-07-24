/*
 * Ported from Android android.text.method.NumberKeyListener (Apache 2.0).
 */
#include <text/method/numberkeylistener.h>
#include <text/editable.h>
#include <text/spannablestringbuilder.h>
#include <text/selection.h>
#include <view/keyevent.h>
#include <algorithm>

namespace cdroid {

int NumberKeyListener::lookup(const KeyEvent& event, Spannable& content) {
    const std::u16string accept = getAcceptedChars();
    return event.getMatch(accept.data(), (int)accept.size(), getMetaState(content, event));
}

bool NumberKeyListener::ok(const std::u16string& accept, char16_t c) {
    for (size_t i = accept.size(); i-- > 0;) {
        if (accept[i] == c) return true;
    }
    return false;
}

bool NumberKeyListener::onKeyDown(View& view, Editable& content, int keyCode, const KeyEvent& event) {
    int selStart, selEnd;

    {
        const int a = Selection::getSelectionStart(&content);
        const int b = Selection::getSelectionEnd(&content);
        selStart = std::min(a, b);
        selEnd = std::max(a, b);
    }

    if (selStart < 0 || selEnd < 0) {
        selStart = selEnd = 0;
        Selection::setSelection(&content, 0);
    }

    const int i = lookup(event, content);
    const int repeatCount = event.getRepeatCount();
    if (repeatCount == 0) {
        if (i != 0) {
            if (selStart != selEnd) {
                Selection::setSelection(&content, selEnd);
            }
            content.replace(selStart, selEnd, SpannableStringBuilder(std::u16string(1, (char16_t)i)));
            // CDROID's adjustSpansForReplace does not advance a cursor at the
            // insertion offset (unlike Android's SpannableStringBuilder), so move
            // the caret explicitly to after the inserted character.
            Selection::setSelection(&content, selStart + 1);
            adjustMetaAfterKeypress(content);
            return true;
        }
    } else if (i == '0' && repeatCount == 1) {
        // Pretty hackish: replaces the 0 with '+'.
        if (selStart == selEnd && selEnd > 0 && content.charAt(selStart - 1) == '0') {
            content.replace(selStart - 1, selEnd, SpannableStringBuilder(std::u16string(1, u'+')));
            // Replaced [selStart-1, selEnd) with one char; caret lands at selStart.
            Selection::setSelection(&content, selStart);
            adjustMetaAfterKeypress(content);
            return true;
        }
    }

    adjustMetaAfterKeypress(content);
    return BaseKeyListener::onKeyDown(view, content, keyCode, event);
}

CharSequence* NumberKeyListener::filter(CharSequence* source, int start, int end,
                                         Spanned* /*dest*/, int /*dstart*/, int /*dend*/) {
    const std::u16string accept = getAcceptedChars();

    int i;
    for (i = start; i < end; i++) {
        if (!ok(accept, (char16_t)source->charAt(i))) {
            break;
        }
    }

    if (i == end) {
        // It was all OK.
        return nullptr;
    }

    if (end - start == 1) {
        // It was not OK, and there is only one char, so nothing remains.
        // (Android returns ""; CDROID has no empty-CharSequence singleton, so
        // return a freshly built empty SpannableStringBuilder.)
        return new SpannableStringBuilder();
    }

    auto* filtered = new SpannableStringBuilder();
    for (int k = start; k < end; k++) {
        filtered->append((char16_t)source->charAt(k));
    }
    i -= start;
    end -= start;

    // Only count down to i because the chars before that were all OK.
    for (int j = end - 1; j >= i; j--) {
        if (!ok(accept, (char16_t)filtered->charAt(j))) {
            filtered->Delete(j, j + 1);
        }
    }

    return filtered;
}

} // namespace cdroid
