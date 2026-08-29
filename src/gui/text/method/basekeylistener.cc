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
#include <text/character.h>
#include <text/paint.h>
#include <text/method/worditerator.h>
#include <widget/textview.h>
#include <view/keyevent.h>
#include <unicode/uchar.h>
#include <algorithm>
#include <stdexcept>

namespace cdroid {

const NoCopySpan* BaseKeyListener::OLD_SEL_START = new NoCopySpan();

// BreakIterator.DONE equivalent for WordIterator results.
static constexpr int BI_DONE = -1;

// Port of android.text.Emoji (@hide) — the emoji predicates the delete state
// machine below runs on. Values come from the myicu binary properties (UCD
// emoji-data.txt); the two hand-picked modifier bases follow AOSP.
namespace {
struct Emoji {
    static constexpr int COMBINING_ENCLOSING_KEYCAP = 0x20E3;
    static constexpr int ZERO_WIDTH_JOINER = 0x200D;
    static constexpr int VARIATION_SELECTOR_16 = 0xFE0F;
    static constexpr int CANCEL_TAG = 0xE007F;

    static bool isRegionalIndicatorSymbol(int codePoint) {
        return 0x1F1E6 <= codePoint && codePoint <= 0x1F1FF;
    }
    static bool isEmojiModifier(int codePoint) {
        return u_hasBinaryProperty(codePoint, UCHAR_EMOJI_MODIFIER);
    }
    static bool isEmojiModifierBase(int c) {
        // Removed from Emoji_Modifier_Base in Emoji 4.0 but still treated as
        // bases for compatibility with existing fonts and text (AOSP note).
        if (c == 0x1F91D || c == 0x1F93C) {
            return true;
        }
        return u_hasBinaryProperty(c, UCHAR_EMOJI_MODIFIER_BASE);
    }
    static bool isEmoji(int codePoint) {
        return u_hasBinaryProperty(codePoint, UCHAR_EMOJI);
    }
    // True if the character can be a base of COMBINING ENCLOSING KEYCAP.
    static bool isKeycapBase(int codePoint) {
        return ('0' <= codePoint && codePoint <= '9') || codePoint == '#' || codePoint == '*';
    }
    // True if the character can be part of tag_spec in an emoji tag sequence.
    // 0xE007F (CANCEL TAG) is not included.
    static bool isTagSpecChar(int codePoint) {
        return 0xE0020 <= codePoint && codePoint <= 0xE007E;
    }
};

// Returns true if the given code point is a variation selector.
bool isVariationSelector(int codepoint) {
    return u_hasBinaryProperty(codepoint, UCHAR_VARIATION_SELECTOR);
}
}  // namespace

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
    int end;
    if (isForwardDelete) {
        // Android resolves the paint from the hosting TextView, falling back to
        // a cached bare Paint guarded by a lock. CDROID's UI thread is single,
        // so the plain function-local static is enough.
        static Paint sCachedPaint;
        TextView* textView = dynamic_cast<TextView*>(&view);
        const Paint* paint = (textView != nullptr)
                ? static_cast<const Paint*>(&textView->getPaint()) : &sCachedPaint;
        end = getOffsetForForwardDeleteKey(content, start, *paint);
    } else {
        end = getOffsetForBackspaceKey(content, start);
    }
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

int BaseKeyListener::adjustReplacementSpan(CharSequence& /*text*/, int offset, bool /*moveToStart*/) {
    // Android shifts the offset onto a ReplacementSpan edge. CDROID's
    // ReplacementSpan is not wired into editing yet; leave the offset unchanged.
    return offset;
}

// Returns the start offset to be deleted by a backspace key from the given offset.
int BaseKeyListener::getOffsetForBackspaceKey(CharSequence& text, int offset) {
    if (offset <= 1) {
        return 0;
    }

    enum State {
        STATE_START = 0,               // Initial state
        STATE_LF = 1,                  // The offset is immediately before line feed.
        STATE_BEFORE_KEYCAP = 2,       // .. before a KEYCAP.
        STATE_BEFORE_VS_AND_KEYCAP = 3,// .. before a variation selector and a KEYCAP.
        STATE_BEFORE_EMOJI_MODIFIER = 4,            // .. before an emoji modifier.
        STATE_BEFORE_VS_AND_EMOJI_MODIFIER = 5,     // .. before a VS and an emoji modifier.
        STATE_BEFORE_VS = 6,           // .. before a variation selector.
        STATE_BEFORE_EMOJI = 7,        // .. before an emoji.
        STATE_BEFORE_ZWJ = 8,          // .. before a ZWJ seen before a ZWJ emoji.
        STATE_BEFORE_VS_AND_ZWJ = 9,   // .. before a VS and a ZWJ seen before a ZWJ emoji.
        STATE_ODD_NUMBERED_RIS = 10,   // The number of following RIS code points is odd.
        STATE_EVEN_NUMBERED_RIS = 11,  // .. even.
        STATE_IN_TAG_SEQUENCE = 12,    // The offset is in an emoji tag sequence.
        STATE_FINISHED = 13,           // The state machine has been stopped.
    };
    static constexpr int LINE_FEED = 0x0A;
    static constexpr int CARRIAGE_RETURN = 0x0D;

    int deleteCharCount = 0;      // Char count to be deleted by backspace.
    int lastSeenVSCharCount = 0;  // Char count of previous variation selector.

    int state = STATE_START;

    int tmpOffset = offset;
    do {
        const int codePoint = Character::codePointBefore(&text, tmpOffset);
        tmpOffset -= Character::charCount(codePoint);

        switch (state) {
        case STATE_START:
            deleteCharCount = Character::charCount(codePoint);
            if (codePoint == LINE_FEED) {
                state = STATE_LF;
            } else if (isVariationSelector(codePoint)) {
                state = STATE_BEFORE_VS;
            } else if (Emoji::isRegionalIndicatorSymbol(codePoint)) {
                state = STATE_ODD_NUMBERED_RIS;
            } else if (Emoji::isEmojiModifier(codePoint)) {
                state = STATE_BEFORE_EMOJI_MODIFIER;
            } else if (codePoint == Emoji::COMBINING_ENCLOSING_KEYCAP) {
                state = STATE_BEFORE_KEYCAP;
            } else if (Emoji::isEmoji(codePoint)) {
                state = STATE_BEFORE_EMOJI;
            } else if (codePoint == Emoji::CANCEL_TAG) {
                state = STATE_IN_TAG_SEQUENCE;
            } else {
                state = STATE_FINISHED;
            }
            break;
        case STATE_LF:
            if (codePoint == CARRIAGE_RETURN) {
                ++deleteCharCount;
            }
            state = STATE_FINISHED;
            break;
        case STATE_ODD_NUMBERED_RIS:
            if (Emoji::isRegionalIndicatorSymbol(codePoint)) {
                deleteCharCount += 2;  // char count of RIS
                state = STATE_EVEN_NUMBERED_RIS;
            } else {
                state = STATE_FINISHED;
            }
            break;
        case STATE_EVEN_NUMBERED_RIS:
            if (Emoji::isRegionalIndicatorSymbol(codePoint)) {
                deleteCharCount -= 2;  // char count of RIS
                state = STATE_ODD_NUMBERED_RIS;
            } else {
                state = STATE_FINISHED;
            }
            break;
        case STATE_BEFORE_KEYCAP:
            if (isVariationSelector(codePoint)) {
                lastSeenVSCharCount = Character::charCount(codePoint);
                state = STATE_BEFORE_VS_AND_KEYCAP;
                break;
            }
            if (Emoji::isKeycapBase(codePoint)) {
                deleteCharCount += Character::charCount(codePoint);
            }
            state = STATE_FINISHED;
            break;
        case STATE_BEFORE_VS_AND_KEYCAP:
            if (Emoji::isKeycapBase(codePoint)) {
                deleteCharCount += lastSeenVSCharCount + Character::charCount(codePoint);
            }
            state = STATE_FINISHED;
            break;
        case STATE_BEFORE_EMOJI_MODIFIER:
            if (isVariationSelector(codePoint)) {
                lastSeenVSCharCount = Character::charCount(codePoint);
                state = STATE_BEFORE_VS_AND_EMOJI_MODIFIER;
                break;
            } else if (Emoji::isEmojiModifierBase(codePoint)) {
                deleteCharCount += Character::charCount(codePoint);
                state = STATE_BEFORE_EMOJI;
                break;
            }
            state = STATE_FINISHED;
            break;
        case STATE_BEFORE_VS_AND_EMOJI_MODIFIER:
            if (Emoji::isEmojiModifierBase(codePoint)) {
                deleteCharCount += lastSeenVSCharCount + Character::charCount(codePoint);
            }
            state = STATE_FINISHED;
            break;
        case STATE_BEFORE_VS:
            if (Emoji::isEmoji(codePoint)) {
                deleteCharCount += Character::charCount(codePoint);
                state = STATE_BEFORE_EMOJI;
                break;
            }
            if (!isVariationSelector(codePoint) &&
                    u_getIntPropertyValue(codePoint, UCHAR_CANONICAL_COMBINING_CLASS) == 0) {
                deleteCharCount += Character::charCount(codePoint);
            }
            state = STATE_FINISHED;
            break;
        case STATE_BEFORE_EMOJI:
            if (codePoint == Emoji::ZERO_WIDTH_JOINER) {
                state = STATE_BEFORE_ZWJ;
            } else {
                state = STATE_FINISHED;
            }
            break;
        case STATE_BEFORE_ZWJ:
            if (Emoji::isEmoji(codePoint)) {
                deleteCharCount += Character::charCount(codePoint) + 1;  // +1 for ZWJ.
                state = Emoji::isEmojiModifier(codePoint)
                        ? STATE_BEFORE_EMOJI_MODIFIER : STATE_BEFORE_EMOJI;
            } else if (isVariationSelector(codePoint)) {
                lastSeenVSCharCount = Character::charCount(codePoint);
                state = STATE_BEFORE_VS_AND_ZWJ;
            } else {
                state = STATE_FINISHED;
            }
            break;
        case STATE_BEFORE_VS_AND_ZWJ:
            if (Emoji::isEmoji(codePoint)) {
                // +1 for ZWJ.
                deleteCharCount += lastSeenVSCharCount + 1 + Character::charCount(codePoint);
                lastSeenVSCharCount = 0;
                state = STATE_BEFORE_EMOJI;
            } else {
                state = STATE_FINISHED;
            }
            break;
        case STATE_IN_TAG_SEQUENCE:
            if (Emoji::isTagSpecChar(codePoint)) {
                deleteCharCount += 2;  // char count of emoji tag spec character
                // Keep the same state.
            } else if (Emoji::isEmoji(codePoint)) {
                deleteCharCount += Character::charCount(codePoint);
                state = STATE_FINISHED;
            } else {
                // Couldn't find tag_base character. Delete the last tag_term character.
                deleteCharCount = 2;  // for U+E007F
                state = STATE_FINISHED;
            }
            // TODO: Need handle emoji variation selectors. Issue 35224297
            break;
        default:
            throw std::invalid_argument("unknown backspace state machine state");
        }
    } while (tmpOffset > 0 && state != STATE_FINISHED);

    return adjustReplacementSpan(text, offset - deleteCharCount, true /* move to the start */);
}

// Returns the end offset to be deleted by a forward delete key from the given offset.
int BaseKeyListener::getOffsetForForwardDeleteKey(CharSequence& text, int offset,
        const Paint& paint) {
    const int len = (int)text.length();

    if (offset >= len - 1) {
        return len;
    }

    offset = (int)paint.getTextRunCursor(&text, offset, len, false /* LTR, not used */,
            offset, Paint::CURSOR_AFTER);

    return adjustReplacementSpan(text, offset, false /* move to the end */);
}

} // namespace cdroid
