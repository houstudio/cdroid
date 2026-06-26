/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 * (LGPL v2.1+) — ported from Android's android.text.method.ArrowKeyMovementMethod.
 * The default movement method for editable text: cursor movement + selection via
 * arrow/page/home/end keys (shift = select, ctrl = word, alt = line/edge/paragraph).
 *********************************************************************************/
#ifndef __ARROW_KEY_MOVEMENT_METHOD_H__
#define __ARROW_KEY_MOVEMENT_METHOD_H__
#include <text/method/basemovementmethod.h>
namespace cdroid {

class Layout;

class ArrowKeyMovementMethod : public BaseMovementMethod {
public:
    bool onKeyDown(TextView& widget, Spannable& text, int keyCode, KeyEvent& event) override;
    bool onTouchEvent(TextView& widget, Spannable& text, MotionEvent& event) override;
    bool canSelectArbitrarily() const override { return true; }
    void initialize(TextView& widget, Spannable& text) override;
    void onTakeFocus(TextView& widget, Spannable& text, int direction) override;

    static MovementMethod* getInstance();

protected:
    bool left(TextView&, Spannable&) override;
    bool right(TextView&, Spannable&) override;
    bool up(TextView&, Spannable&) override;
    bool down(TextView&, Spannable&) override;
    bool pageUp(TextView&, Spannable&) override;
    bool pageDown(TextView&, Spannable&) override;
    bool top(TextView&, Spannable&) override;
    bool bottom(TextView&, Spannable&) override;
    bool lineStart(TextView&, Spannable&) override;
    bool lineEnd(TextView&, Spannable&) override;
    bool home(TextView&, Spannable&) override;
    bool end(TextView&, Spannable&) override;
    bool previousParagraph(TextView&, Spannable&) override;
    bool nextParagraph(TextView&, Spannable&) override;

private:
    // Android tracks selection mode in the Spannable (MetaKeyKeyListener spans).
    // CDROID reads SHIFT off the key event in onKeyDown — enough for shift-select.
    bool isSelecting() const { return mSelecting; }
    static int getCurrentLineTop(Spannable& buffer, Layout* layout);
    static int getPageHeight(TextView& widget);
    bool mSelecting = false;
};

}  // namespace cdroid
#endif  // __ARROW_KEY_MOVEMENT_METHOD_H__
