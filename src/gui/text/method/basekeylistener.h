/*
 * Ported from Android android.text.method.BaseKeyListener (Apache 2.0).
 * Abstract base above the concrete text key listeners: handles DEL/FORWARD_DEL
 * (delete selection / char / Alt-line / Ctrl-word) and exposes makeTextContentType
 * plus the Capitalize enum shared by QwertyKeyListener/TextKeyListener.
 *
 * IMPORTANT CDROID keycode note (see AGENTS.md plan): the input layer maps the
 * physical BackSpace key to KEYCODE_BACKSPACE(112) [deletes LEFT] and the
 * physical Delete key to KEYCODE_DEL(67) [deletes RIGHT] — the LEFT/RIGHT
 * sense is opposite to Android's *names* (Android KEYCODE_DEL=backspace,
 * KEYCODE_FORWARD_DEL=forward) but is physically correct here. This class
 * therefore routes KEYCODE_BACKSPACE→backspace() and KEYCODE_DEL→forwardDelete()
 * to preserve existing editor behavior; do NOT copy Android's literal switch.
 */
#ifndef CDROID_BASEKEYLISTENER_H
#define CDROID_BASEKEYLISTENER_H

#include <text/method/metakeylistener.h>

namespace cdroid {

class TextView;

class BaseKeyListener : public MetaKeyKeyListener {
public:
    // Capitalization rules (Android defines this nested in TextKeyListener;
    // CDROID hosts it here so BaseKeyListener.makeTextContentType and the
    // concrete listeners can share it without a header cycle).
    enum class Capitalize { NONE, SENTENCES, WORDS, CHARACTERS };

    // Marker span for the saved pre-edit selection start (Android's OLD_SEL_START).
    static const NoCopySpan* OLD_SEL_START;

    bool onKeyDown(View& view, Editable& content, int keyCode, const KeyEvent& event) override;
    bool onKeyOther(View& view, Editable& content, const KeyEvent& event) override;

    bool backspace(View& view, Editable& content, int keyCode, const KeyEvent& event);
    bool forwardDelete(View& view, Editable& content, int keyCode, const KeyEvent& event);

    static int makeTextContentType(Capitalize caps, bool autoText);

protected:
    // Used by Editor/TextView to prime the autotext-undo span region. Ported but
    // inert until the autotext (Replaced span) machinery is wired (later phase).
    // Kept declared for API parity; definition in the .cc.

private:
    bool backspaceOrForwardDelete(View& view, Editable& content, int keyCode,
                                  const KeyEvent& event, bool isForwardDelete);
    bool deleteSelection(View& view, Editable& content);
    bool deleteLineFromCursor(View& view, Editable& content, bool forward);
    bool deleteUntilWordBoundary(View& view, Editable& content, bool isForwardDelete);

    // Phase-1 stubs: CDROID has no ICU Emoji / Paint.getTextRunCursor, so these
    // reduce to BMP single-char offsets (identical to prior hand-written Editor
    // behavior for BMP text). Full grapheme/emoji state machine = later phase.
    static int getOffsetForBackspaceKey(CharSequence& text, int offset);
    static int getOffsetForForwardDeleteKey(CharSequence& text, int offset);
    static int adjustReplacementSpan(CharSequence& text, int offset, bool moveToStart);
};

} // namespace cdroid

#endif // CDROID_BASEKEYLISTENER_H
