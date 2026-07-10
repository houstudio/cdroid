/*
 * Ported from Android android.text.method.BaseKeyListener (Apache 2.0).
 * See basekeylistener.h for the CDROID keycode note (112=delete-left,
 * 67=delete-right) and the Phase-1 grapheme stubs.
 */
#include <text/method/basekeylistener.h>
#include <text/editable.h>
#include <text/spannablestringbuilder.h>
#include <text/selection.h>
#include <text/layout.h>
#include <text/inputtype.h>
#include <text/method/worditerator.h>
#include <widget/textview.h>
#include <view/keyevent.h>
#include <algorithm>

namespace cdroid {

const NoCopySpan* BaseKeyListener::OLD_SEL_START = new NoCopySpan();

// BreakIterator.DONE equivalent for WordIterator results.
static constexpr int BI_DONE = -1;

int BaseKeyListener::makeTextContentType(Capitalize caps, bool autoText) {
    int contentType = InputType::TYPE_CLASS_TEXT;
    switch (caps) {
        case Capitalize::CHARACTERS: contentType |= InputType::TYPE_TEXT_FLAG_CAP_CHARACTERS; break;
        case Capitalize::WORDS:      contentType |= InputType::TYPE_TEXT_FLAG_CAP_WORDS;      break;
        case Capitalize::SENTENCES:  contentType |= InputType::TYPE_TEXT_FLAG_CAP_SENTENCES;  break;
        case Capitalize::NONE:       break;
    }
    if (autoText) contentType |= InputType::TYPE_TEXT_FLAG_AUTO_CORRECT;
    return contentType;
}

bool BaseKeyListener::backspace(View& view, Editable& content, int keyCode, const KeyEvent& event) {
    return backspaceOrForwardDelete(view, content, keyCode, event, false);
}

bool BaseKeyListener::forwardDelete(View& view, Editable& content, int keyCode, const KeyEvent& event) {
    return backspaceOrForwardDelete(view, content, keyCode, event, true);
}

bool BaseKeyListener::backspaceOrForwardDelete(View& view, Editable& content, int /*keyCode*/,
        const KeyEvent& event, bool isForwardDelete) {
    // Ensure the key event does not have modifiers except ALT or SHIFT or CTRL.
    if (!KeyEvent::metaStateHasNoModifiers(event.getMetaState()
            & ~(KeyEvent::META_SHIFT_MASK | KeyEvent::META_ALT_MASK | KeyEvent::META_CTRL_MASK))) {
        return false;
    }

    // If there is a current selection, delete it.
    if (deleteSelection(view, content)) {
        return true;
    }

    // MetaKeyKeyListener doesn't track control key state. Check the KeyEvent.
    const bool isCtrlActive  = ((event.getMetaState() & KeyEvent::META_CTRL_ON) != 0);
    const bool isShiftActive = (getMetaState(content, META_SHIFT_ON, event) == 1);
    const bool isAltActive   = (getMetaState(content, META_ALT_ON, event) == 1);

    if (isCtrlActive) {
        if (isAltActive || isShiftActive) {
            // Ctrl+Alt, Ctrl+Shift, Ctrl+Alt+Shift should not delete any characters.
            return false;
        }
        return deleteUntilWordBoundary(view, content, isForwardDelete);
    }

    // Alt+Backspace or Alt+ForwardDelete deletes the current line, if possible.
    if (isAltActive && deleteLineFromCursor(view, content, isForwardDelete)) {
        return true;
    }

    // Delete a character. CDROID's SpannableStringBuilder.Delete auto-adjusts the
    // Selection spans (adjustSpansForReplace shifts/collapses them), so no explicit
    // setSelection is needed — matching Android's Editable contract.
    const int start = Selection::getSelectionEnd(&content);
    const int end = isForwardDelete ? getOffsetForForwardDeleteKey(content, start)
                                    : getOffsetForBackspaceKey(content, start);
    if (start != end) {
        content.Delete(std::min(start, end), std::max(start, end));
        return true;
    }
    return false;
}

bool BaseKeyListener::deleteUntilWordBoundary(View& view, Editable& content, bool isForwardDelete) {
    const int currentCursorOffset = Selection::getSelectionStart(&content);

    // If there is a selection, do nothing.
    if (currentCursorOffset != Selection::getSelectionEnd(&content)) {
        return false;
    }

    // Early exit if there is no contents to delete.
    if ((!isForwardDelete && currentCursorOffset == 0) ||
        (isForwardDelete && currentCursorOffset == (int)content.length())) {
        return false;
    }

    // Android uses ((TextView)view).getWordIterator() when available; CDROID's
    // TextView does not expose it, so use a default-locale WordIterator.
    WordIterator wordIterator;

    int deleteFrom;
    int deleteTo;

    if (isForwardDelete) {
        deleteFrom = currentCursorOffset;
        wordIterator.setCharSequence(&content, deleteFrom, (int)content.length());
        deleteTo = wordIterator.following(currentCursorOffset);
        if (deleteTo == BI_DONE) {
            deleteTo = (int)content.length();
        }
    } else {
        deleteTo = currentCursorOffset;
        wordIterator.setCharSequence(&content, 0, deleteTo);
        deleteFrom = wordIterator.preceding(currentCursorOffset);
        if (deleteFrom == BI_DONE) {
            deleteFrom = 0;
        }
    }
    content.Delete(deleteFrom, deleteTo);
    return true;
}

bool BaseKeyListener::deleteSelection(View& /*view*/, Editable& content) {
    int selectionStart = Selection::getSelectionStart(&content);
    int selectionEnd = Selection::getSelectionEnd(&content);
    if (selectionEnd < selectionStart) {
        std::swap(selectionEnd, selectionStart);
    }
    if (selectionStart != selectionEnd) {
        content.Delete(selectionStart, selectionEnd);
        return true;
    }
    return false;
}

bool BaseKeyListener::deleteLineFromCursor(View& view, Editable& content, bool forward) {
    TextView* textView = dynamic_cast<TextView*>(&view);
    if (textView != nullptr) {
        const int selectionStart = Selection::getSelectionStart(&content);
        const int selectionEnd = Selection::getSelectionEnd(&content);
        const int selectionMin = std::min(selectionStart, selectionEnd);
        const int selectionMax = std::max(selectionStart, selectionEnd);

        Layout* layout = textView->getLayout();
        // Android also gates on !textView.isOffsetMappingAvailable(); CDROID's
        // TextView has no offset mapping, so that check is omitted.
        if (layout != nullptr) {
            const int line = layout->getLineForOffset(Selection::getSelectionStart(&content));
            const int start = layout->getLineStart(line);
            const int end = layout->getLineEnd(line);

            if (forward) {
                content.Delete(selectionMin, end);
            } else {
                content.Delete(start, selectionMax);
            }
            return true;
        }
    }
    return false;
}

bool BaseKeyListener::onKeyDown(View& view, Editable& content, int keyCode, const KeyEvent& event) {
    bool handled = false;
    switch (keyCode) {
        case KeyEvent::KEYCODE_DEL:
            // Android KEYCODE_DEL(67) = backspace, deletes to the LEFT.
            handled = backspace(view, content, keyCode, event);
            break;
        case KeyEvent::KEYCODE_FORWARD_DEL:
            // Android KEYCODE_FORWARD_DEL(112) = forward delete, deletes to the RIGHT.
            handled = forwardDelete(view, content, keyCode, event);
            break;
        default:
            handled = false;
            break;
    }

    if (handled) {
        adjustMetaAfterKeypress(content);
        return true;
    }

    return MetaKeyKeyListener::onKeyDown(view, content, keyCode, event);
}

bool BaseKeyListener::onKeyOther(View& /*view*/, Editable& /*content*/, const KeyEvent& event) {
    // Android inserts event.getCharacters() here for ACTION_MULTIPLE/KEYCODE_UNKNOWN.
    // CDROID's desktop input layer never generates those, and KeyEvent has no
    // getCharacters(), so this path is unreachable. Return false (not handled).
    if (event.getAction() != KeyEvent::ACTION_MULTIPLE
            || event.getKeyCode() != KeyEvent::KEYCODE_UNKNOWN) {
        return false;
    }
    return false;
}

// --- Phase-1 grapheme stubs ---

int BaseKeyListener::adjustReplacementSpan(CharSequence& /*text*/, int offset, bool /*moveToStart*/) {
    // Android shifts the offset onto a ReplacementSpan edge. CDROID's
    // ReplacementSpan is not wired into editing yet; leave the offset unchanged.
    return offset;
}

int BaseKeyListener::getOffsetForBackspaceKey(CharSequence& text, int offset) {
    if (offset <= 1) {
        return 0;
    }
    // Phase-1 stub: delete one BMP char to the left. The full Android version
    // runs a grapheme/emoji (ZWJ/VS/RIS/keycap/tag) state machine via ICU Emoji
    // helpers, which CDROID does not have. For BMP text this matches the prior
    // hand-written Editor behavior exactly.
    return adjustReplacementSpan(text, offset - 1, true);
}

int BaseKeyListener::getOffsetForForwardDeleteKey(CharSequence& text, int offset) {
    const int len = (int)text.length();
    if (offset >= len - 1) {
        return len;
    }
    // Phase-1 stub: delete one BMP char to the right (no Paint.getTextRunCursor).
    return adjustReplacementSpan(text, offset + 1, false);
}

} // namespace cdroid
