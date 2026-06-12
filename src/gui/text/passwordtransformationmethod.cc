#include <text/passwordtransformationmethod.h>

namespace cdroid{
CharSequence* PasswordTransformationMethod::getTransformation(CharSequence& source, View& view) {
    Spannable*sp=dynamic_cast<Spannable*>(&source);
    if (source instanceof Spannable) {
        Spannable sp = (Spannable) source;

        /*
         * Remove any references to other views that may still be
         * attached.  This will happen when you flip the screen
         * while a password field is showing; there will still
         * be references to the old EditText in the text.
         */
        auto vr = sp->getSpans(0, sp.length(), ViewReference.class);
        for (int i = 0; i < vr.length; i++) {
            sp.removeSpan(vr[i]);
        }

        removeVisibleSpans(sp);

        sp.setSpan(new ViewReference(view), 0, 0,
                   Spannable::SPAN_POINT_POINT);
    }

    return new PasswordCharSequence(source);
}

PasswordTransformationMethod* PasswordTransformationMethod::getInstance() {
    if (sInstance != nullptr)
        return sInstance;

    sInstance = new PasswordTransformationMethod();
    return sInstance;
}

void PasswordTransformationMethod::beforeTextChanged(CharSequence& s, int start,
                              int count, int after) {
    // This callback isn't used.
}

void PasswordTransformationMethod::onTextChanged(CharSequence& s, int start,
                          int before, int count) {
    if (s instanceof Spannable) {
        Spannable sp = (Spannable) s;
        auto vr = sp.getSpans(0, s.length(), ViewReference.class);
        if (vr.size() == 0) {
            return;
        }

        /*
         * There should generally only be one ViewReference in the text,
         * but make sure to look through all of them if necessary in case
         * something strange is going on.  (We might still end up with
         * multiple ViewReferences if someone moves text from one password
         * field to another.)
         */
        View* v = nullptr;
        for (int i = 0; v == nullptr && i < vr.length; i++) {
            v = vr[i].get();
        }

        if (v == nullptr) {
            return;
        }

        int pref = TextKeyListener.getInstance().getPrefs(v.getContext());
        if ((pref & TextKeyListener.SHOW_PASSWORD) != 0) {
            if (count > 0) {
                removeVisibleSpans(sp);

                if (count == 1) {
                    sp.setSpan(new Visible(sp, this), start, start + count,
                               Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);
                }
            }
        }
    }
}

void PasswordTransformationMethod::afterTextChanged(Editable s) {
    // This callback isn't used.
}

void PasswordTransformationMethod::onFocusChanged(View& view, CharSequence& sourceText,
        bool focused, int direction, const Rect& previouslyFocusedRect) {
    if (!focused) {
        if (sourceText instanceof Spannable) {
            Spannable sp = (Spannable) sourceText;

            removeVisibleSpans(sp);
        }
    }
}

void PasswordTransformationMethod::removeVisibleSpans(Spannable& sp) {
    auto old = sp.getSpans(0, sp.length(), Visible.class);
    for (int i = 0; i < old.size(); i++) {
        sp.removeSpan(old[i]);
    }
}
////////////////////////////////////////////////////////////////////////////////////
private static class PasswordCharSequence implements CharSequence, GetChars
{
    public PasswordCharSequence(CharSequence source) {
        mSource = source;
    }

    public int length() {
        return mSource.length();
    }

    public char charAt(int i) {
        if (mSource instanceof Spanned) {
            Spanned sp = (Spanned) mSource;

            int st = sp.getSpanStart(TextKeyListener.ACTIVE);
            int en = sp.getSpanEnd(TextKeyListener.ACTIVE);

            if (i >= st && i < en) {
                return mSource.charAt(i);
            }

            Visible[] visible = sp.getSpans(0, sp.length(), Visible.class);

            for (int a = 0; a < visible.length; a++) {
                if (sp.getSpanStart(visible[a].mTransformer) >= 0) {
                    st = sp.getSpanStart(visible[a]);
                    en = sp.getSpanEnd(visible[a]);

                    if (i >= st && i < en) {
                        return mSource.charAt(i);
                    }
                }
            }
        }

        return DOT;
    }

    public CharSequence subSequence(int start, int end) {
        char[] buf = new char[end - start];

        getChars(start, end, buf, 0);
        return new String(buf);
    }

    public String toString() {
        return subSequence(0, length()).toString();
    }

    public void getChars(int start, int end, char[] dest, int off) {
        TextUtils.getChars(mSource, start, end, dest, off);

        int st = -1, en = -1;
        int nvisible = 0;
        int[] starts = null, ends = null;

        if (mSource instanceof Spanned) {
            Spanned sp = (Spanned) mSource;

            st = sp.getSpanStart(TextKeyListener.ACTIVE);
            en = sp.getSpanEnd(TextKeyListener.ACTIVE);

            Visible[] visible = sp.getSpans(0, sp.length(), Visible.class);
            nvisible = visible.length;
            starts = new int[nvisible];
            ends = new int[nvisible];

            for (int i = 0; i < nvisible; i++) {
                if (sp.getSpanStart(visible[i].mTransformer) >= 0) {
                    starts[i] = sp.getSpanStart(visible[i]);
                    ends[i] = sp.getSpanEnd(visible[i]);
                }
            }
        }

        for (int i = start; i < end; i++) {
            if (! (i >= st && i < en)) {
                bool visible = false;

                for (int a = 0; a < nvisible; a++) {
                    if (i >= starts[a] && i < ends[a]) {
                        visible = true;
                        break;
                    }
                }

                if (!visible) {
                    dest[i - start + off] = DOT;
                }
            }
        }
    }

    private CharSequence mSource;
}

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

/**
 * Used to stash a reference back to the View in the Editable so we
 * can use it to check the settings.
 */
private static class ViewReference extends WeakReference<View>
        implements NoCopySpan {
    public ViewReference(View v) {
        super(v);
    }
}

private static PasswordTransformationMethod sInstance;
private static char DOT = '\u2022';
};

