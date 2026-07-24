/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * (LGPL v2.1+) — ported from Android's android.text.method.BaseMovementMethod.
 * Base class: decodes movement keys (arrows/page/home/end + ctrl/alt modifiers)
 * into movement actions (left/right/up/down/…) and provides default scroll
 * helpers. Subclasses (ArrowKeyMovementMethod, ScrollingMovementMethod) override
 * the movement actions.
 *
 * Pragmatic CDROID adaptation: Android tracks SHIFT/ALT/CTRL meta in the
 * Spannable buffer via MetaKeyKeyListener (a large span-based meta store, not
 * ported). CDROID reads the meta state straight off the KeyEvent — enough for
 * the common modifier cases (shift=select, ctrl=word, alt=line/edge).
 *********************************************************************************/
#ifndef __BASE_MOVEMENT_METHOD_H__
#define __BASE_MOVEMENT_METHOD_H__
#include <text/method/movementmethod.h>
namespace cdroid {

class BaseMovementMethod : public MovementMethod {
public:
    bool onKeyDown(TextView& widget, Spannable& text, int keyCode, KeyEvent& event) override;
    bool onKeyOther(TextView& widget, Spannable& text, KeyEvent& event) override;
    bool canSelectArbitrarily() const override { return false; }

protected:
    // Meta used for movement (modifiers present on this key event, SHIFT masked
    // out since movement ignores it — selection uses it via isSelecting()).
    virtual int getMovementMetaState(Spannable& buffer, KeyEvent& event);
    // Decodes the key into a movement action. Returns true if handled.
    virtual bool handleMovementKey(TextView& widget, Spannable& buffer, int keyCode,
                                   int movementMetaState, KeyEvent& event);

    // Movement actions — default implementations do nothing (return false).
    // ArrowKeyMovementMethod / ScrollingMovementMethod override these.
    virtual bool left(TextView& widget, Spannable& buffer)          { return false; }
    virtual bool right(TextView& widget, Spannable& buffer)         { return false; }
    virtual bool up(TextView& widget, Spannable& buffer)            { return false; }
    virtual bool down(TextView& widget, Spannable& buffer)          { return false; }
    virtual bool pageUp(TextView& widget, Spannable& buffer)        { return false; }
    virtual bool pageDown(TextView& widget, Spannable& buffer)      { return false; }
    virtual bool top(TextView& widget, Spannable& buffer)           { return false; }
    virtual bool bottom(TextView& widget, Spannable& buffer)        { return false; }
    virtual bool lineStart(TextView& widget, Spannable& buffer)     { return false; }
    virtual bool lineEnd(TextView& widget, Spannable& buffer)       { return false; }
    virtual bool leftWord(TextView& widget, Spannable& buffer)      { return false; }
    virtual bool rightWord(TextView& widget, Spannable& buffer)     { return false; }
    virtual bool home(TextView& widget, Spannable& buffer)          { return false; }
    virtual bool end(TextView& widget, Spannable& buffer)           { return false; }
    virtual bool previousParagraph(TextView& widget, Spannable& buffer) { return false; }
    virtual bool nextParagraph(TextView& widget, Spannable& buffer)     { return false; }

    // Scroll helpers (used by ScrollingMovementMethod and mouse-wheel scroll).
    bool scrollLeft(TextView& widget, Spannable& buffer, int amount);
    bool scrollRight(TextView& widget, Spannable& buffer, int amount);
    bool scrollUp(TextView& widget, Spannable& buffer, int amount);
    bool scrollDown(TextView& widget, Spannable& buffer, int amount);
    bool scrollPageUp(TextView& widget, Spannable& buffer);
    bool scrollPageDown(TextView& widget, Spannable& buffer);
    bool scrollTop(TextView& widget, Spannable& buffer);
    bool scrollBottom(TextView& widget, Spannable& buffer);
    bool scrollLineStart(TextView& widget, Spannable& buffer);
    bool scrollLineEnd(TextView& widget, Spannable& buffer);
};

}  // namespace cdroid
#endif  // __BASE_MOVEMENT_METHOD_H__
