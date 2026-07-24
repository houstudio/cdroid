/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * (LGPL v2.1+) — ported from Android's android.text.method.AllCapsTransformationMethod.
 *
 * Upper-cases the displayed text. Android takes a Context to resolve the display
 * Locale for locale-aware uppercasing; CDROID has no Java-style Locale wiring yet,
 * so v1 is locale-insensitive (Character::toUpperCase / u_toupper). The Context
 * parameter is kept on the constructor for signature parity and future locale work.
 *
 * Like the Android original this implements TransformationMethod2: until
 * setLengthChangesAllowed(true) is called (TextView::setTransformationMethod does
 * this automatically via dynamic_cast), getTransformation() returns the source
 * unchanged. It does NOT implement OffsetMapping (matches Android); for the
 * length-preserving upper-casing that holds for the vast majority of text the
 * cursor maps 1:1 anyway.
 *********************************************************************************/
#ifndef __ALL_CAPS_TRANSFORMATION_METHOD_H__
#define __ALL_CAPS_TRANSFORMATION_METHOD_H__
#include <text/method/transformationmethod.h>

namespace cdroid{

class Context;

class AllCapsTransformationMethod : public TransformationMethod2 {
public:
    explicit AllCapsTransformationMethod(Context* context);

    CharSequence* getTransformation(CharSequence& source, View& view) override;
    void onFocusChanged(View& view, CharSequence& sourceText, bool focused,
                        int direction, const Rect& previouslyFocusedRect) override;
    void setLengthChangesAllowed(bool allowLengthChanges) override;

private:
    // Mirrors Android's mEnabled: upper-casing only happens when the host has
    // allowed length changes (some locales change string length when upper-cased).
    bool mEnabled = false;
};

}/*endof namespace*/
#endif/*__ALL_CAPS_TRANSFORMATION_METHOD_H__*/
