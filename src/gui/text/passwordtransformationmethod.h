#ifndef __PASSWORD_TRANSFORMATION_METHOD_H__
#define __PASSWORD_TRANSFORMATION_METHOD_H__
#include <text/transformationmethod.h>
namespace cdroid{
class PasswordTransformationMethod :public TransformationMethod, TextWatcher{
private:
    static PasswordTransformationMethod sInstance;
    static char16_t DOT = 0x2022;
public:
    CharSequence* getTransformation(CharSequence& source, View& view)override;

    static PasswordTransformationMethod* getInstance();

    pvoid beforeTextChanged(CharSequence& s, int start, int count, int after) override;

    void onTextChanged(CharSequence& s, int start, int before, int count)override;

    void afterTextChanged(Editable* s) override;
    void onFocusChanged(View& view, CharSequence& sourceText, bool focused,
            int direction, const Rect& previouslyFocusedRect)override;
#if 0
    private static void removeVisibleSpans(Spannable sp) {
        Visible[] old = sp.getSpans(0, sp.length(), Visible.class);
        for (int i = 0; i < old.length; i++) {
            sp.removeSpan(old[i]);
        }
    }

    private static class PasswordCharSequence implements CharSequence, GetChars
    {
        public PasswordCharSequence(CharSequence source) {
            mSource = source;
        }

        public int length() {
            return mSource.length();
        }

        public char16_t charAt(int i);

        public CharSequence subSequence(int start, int end);

        public String toString() {
            return subSequence(0, length()).toString();
        }

        public void getChars(int start, int end, char16_t* dest, int off);

        private CharSequence mSource;
    };

    private static class Visible extends Handler implements UpdateLayout, Runnable
    {
        public Visible(Spannable sp, PasswordTransformationMethod ptm) {
            mText = sp;
            mTransformer = ptm;
            postAtTime(this, SystemClock.uptimeMillis() + 1500);
        }

        public void run() {
            mText.removeSpan(this);
        }

        private Spannable mText;
        private PasswordTransformationMethod mTransformer;
    }
#endif
};
}/*endof namespace*/
#endif/*__PASSWORD_TRANSFORMATION_METHOD_H__*/

