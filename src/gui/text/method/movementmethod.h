/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * (LGPL v2.1+) — ported from Android's android.text.method.MovementMethod.
 * Provides cursor positioning, scrolling and text selection functionality; the
 * TextView delegates key/trackball/touch events to it for content navigation.
 *********************************************************************************/
#ifndef __MOVEMENT_METHOD_H__
#define __MOVEMENT_METHOD_H__
namespace cdroid {

class TextView;
class Spannable;
class KeyEvent;
class MotionEvent;

/** Port of android.text.method.MovementMethod. */
class MovementMethod {
public:
    virtual ~MovementMethod() = default;
    virtual void initialize(TextView& widget, Spannable& text) {}
    virtual bool onKeyDown(TextView& widget, Spannable& text, int keyCode, KeyEvent& event) = 0;
    virtual bool onKeyUp(TextView& widget, Spannable& text, int keyCode, KeyEvent& event) { return false; }
    /** Android: handle "other" key events (e.g. ACTION_MULTIPLE repeats). */
    virtual bool onKeyOther(TextView& widget, Spannable& text, KeyEvent& event) { return false; }
    virtual void onTakeFocus(TextView& widget, Spannable& text, int direction) {}
    virtual bool onTrackballEvent(TextView& widget, Spannable& text, MotionEvent& event) { return false; }
    virtual bool onTouchEvent(TextView& widget, Spannable& text, MotionEvent& event) { return false; }
    virtual bool onGenericMotionEvent(TextView& widget, Spannable& text, MotionEvent& event) { return false; }
    /** True if arbitrary text selection is allowed (Select All menu gated on this). */
    virtual bool canSelectArbitrarily() const { return false; }
};

}  // namespace cdroid
#endif  // __MOVEMENT_METHOD_H__
