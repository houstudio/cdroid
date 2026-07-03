/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * (LGPL v2.1+) — ported from Android's android.text.PasswordTransformationMethod.
 * Minimal port: transforms the source into a run of DOT (•) characters for
 * display. The "show the last typed character briefly" refinement (Android's
 * Visible span + TextKeyListener.SHOW_PASSWORD) is NOT ported yet — every
 * character renders as a dot.
 *********************************************************************************/
#ifndef __PASSWORD_TRANSFORMATION_METHOD_H__
#define __PASSWORD_TRANSFORMATION_METHOD_H__
#include <text/method/transformationmethod.h>
#include <text/textwatcher.h>
#include <text/parcelablespan.h>   // CharSequence (for PasswordCharSequence)

namespace cdroid{

class PasswordTransformationMethod : public TransformationMethod, public TextWatcher {
public:
    static constexpr char16_t DOT = 0x2022;   // '•'

    CharSequence* getTransformation(CharSequence& source, View& view) override;
    static PasswordTransformationMethod* getInstance();
    void onFocusChanged(View& view, CharSequence& sourceText, bool focused,
                        int direction, const Rect& previouslyFocusedRect) override;

    // TextWatcher callbacks are inherited (std::function members). The minimal
    // port leaves them empty (no show-last-char handling); they remain so the
    // type can be attached as a TextWatcher span like Android's.

    // A CharSequence wrapper that renders every char of the source as DOT. It
    // reads the source live, so edits to the underlying Editable show up without
    // re-running getTransformation.
    class PasswordCharSequence : public CharSequence {
    public:
        explicit PasswordCharSequence(CharSequence* source) : mSource(source) {}
        size_t length() const override { return mSource ? mSource->length() : 0; }
        int charAt(int /*i*/) const override { return DOT; }
        void getChars(int start, int end, char16_t* dest, int destPos) const override;
        std::string toString() const override;
        std::u16string toU16String() const override;
    private:
        CharSequence* mSource;
    };

private:
    static PasswordTransformationMethod* sInstance;
};

}/*endof namespace*/
#endif/*__PASSWORD_TRANSFORMATION_METHOD_H__*/
