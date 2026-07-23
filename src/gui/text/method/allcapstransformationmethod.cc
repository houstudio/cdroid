/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * (LGPL v2.1+) — C++ port of Android's AllCapsTransformationMethod.
 *********************************************************************************/
#include <text/method/allcapstransformationmethod.h>
#include <text/textutils.h>
#include <text/spannablestring.h>   // Spanned (for copySpans test)

namespace cdroid{

AllCapsTransformationMethod::AllCapsTransformationMethod(Context* /*context*/) {
    // Android reads the display Locale from context.getResources().getConfiguration()
    // here for locale-aware upper-casing. CDROID has no Locale wiring yet, so v1 is
    // locale-insensitive (TextUtils::toUpperCase uses Character::toUpperCase).
}

CharSequence* AllCapsTransformationMethod::getTransformation(CharSequence& source, View& /*view*/) {
    if (!mEnabled) {
        // Android logs: "Caller did not enable length changes; not transforming text".
        return &source;
    }
    // Spanned sources keep their spans (cloned into the result by toUpperCase, so
    // source and result never share an owned span).
    const bool copySpans = (dynamic_cast<Spanned*>(&source) != nullptr);
    return TextUtils::toUpperCase(&source, copySpans);
}

void AllCapsTransformationMethod::onFocusChanged(View& /*view*/, CharSequence& /*sourceText*/,
        bool /*focused*/, int /*direction*/, const Rect& /*previouslyFocusedRect*/) {
    // No-op, matching Android.
}

void AllCapsTransformationMethod::setLengthChangesAllowed(bool allowLengthChanges) {
    mEnabled = allowLengthChanges;
}

}/*endof namespace*/
