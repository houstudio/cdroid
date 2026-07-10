/*
 * Ported from Android android.text.method.BaseKeyListener (Apache 2.0).
 * Abstract base above the concrete text key listeners: handles DEL/FORWARD_DEL
 * (delete selection / char / Alt-line / Ctrl-word) and exposes makeTextContentType
 * plus the Capitalize enum shared by QwertyKeyListener/TextKeyListener.
 *
 * Delete keycodes are aligned with Android (2026-07-10): KEYCODE_DEL(67)=backspace
 * (delete LEFT), KEYCODE_FORWARD_DEL(112)=forward delete (delete RIGHT). The old
 * CDROID-specific KEYCODE_BACKSPACE (which collided with FORWARD_DEL at 112) has
 * been removed. The keylayout files map the physical Backspace key (linux 14) via
 * `key 14 DEL` -> KEYCODE_DEL, matching Android's Generic.kl, so PC Backspace
 * deletes left and PC Delete (linux 111, `key 111 FORWARD_DEL`) deletes right.
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
