/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * (LGPL v2.1+) — minimal C++ port of Android's PasswordTransformationMethod.
 *********************************************************************************/
#include <text/method/passwordtransformationmethod.h>
#include <text/textutils.h>

namespace cdroid{

PasswordTransformationMethod* PasswordTransformationMethod::sInstance = nullptr;

PasswordTransformationMethod* PasswordTransformationMethod::getInstance() {
    if (sInstance == nullptr) sInstance = new PasswordTransformationMethod();
    return sInstance;
}

CharSequence* PasswordTransformationMethod::getTransformation(CharSequence& source, View& /*view*/) {
    // Wrap the source so every character renders as a DOT. The wrapper holds the
    // source pointer and reads it live, so edits to the underlying Editable are
    // reflected without re-running getTransformation (the Layout reflows on edit
    // via the host TextView's ChangeWatcher).
    return new PasswordCharSequence(&source);
}

void PasswordTransformationMethod::onFocusChanged(View& /*view*/, CharSequence& /*sourceText*/,
        bool /*focused*/, int /*direction*/, const Rect& /*previouslyFocusedRect*/) {
    // Android removes the Visible (show-last-char) spans here on focus loss.
    // Not applicable to the minimal port (no Visible spans).
}

// ---- PasswordCharSequence ----

void PasswordTransformationMethod::PasswordCharSequence::getChars(int start, int end, char16_t* dest, int destPos) const {
    // Every visible character is the DOT, regardless of the source.
    for (int i = 0; i < end - start; i++) {
        dest[destPos + i] = DOT;
    }
}

std::string PasswordTransformationMethod::PasswordCharSequence::toString() const {
    return TextUtils::utf16_utf8(toU16String());
}

std::u16string PasswordTransformationMethod::PasswordCharSequence::toU16String() const {
    return std::u16string(length(), DOT);
}

}/*endof namespace*/
