/*
 * Ported from Android android.text.method.QwertyKeyListener (Apache 2.0).
 * Standard alphabetic key listener for qwerty/full keyboards: resolves the key
 * to a Unicode char (event.getUnicodeChar(meta)) and inserts it, with optional
 * auto-capitalization and the two-spaces→period shortcut.
 *
 * Phase-1 deferrals: dead-key (COMBINING_ACCENT) composition, HEX_INPUT,
 * PICKER_DIALOG_INPUT / CharacterPickerDialog, and AutoText replacement (+ its
 * DEL undo via the Replaced span). These branches are simply not entered on a
 * desktop keyboard with no AutoText/picker wired.
 */
#ifndef CDROID_QWERTYKEYLISTENER_H
#define CDROID_QWERTYKEYLISTENER_H

#include <text/method/basekeylistener.h>

namespace cdroid {

class QwertyKeyListener : public BaseKeyListener {
public:
    static QwertyKeyListener* getInstance(bool autoText, Capitalize cap);
    static QwertyKeyListener* getInstanceForFullKeyboard();

    int getInputType() const override;
    bool onKeyDown(View& view, Editable& content, int keyCode, const KeyEvent& event) override;

private:
    QwertyKeyListener(Capitalize cap, bool autoText, bool fullKeyboard);

    Capitalize mAutoCap;
    bool mAutoText;
    bool mFullKeyboard;
};

} // namespace cdroid

#endif // CDROID_QWERTYKEYLISTENER_H
