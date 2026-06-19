namespace cdroid{
/**
 * InputFilters can be attached to {@link Editable}s to constrain the
 * changes that can be made to them.
 */
class InputFilter{
    /**
     * This method is called when the buffer is going to replace the
     * range <code>dstart &hellip; dend</code> of <code>dest</code>
     * with the new text from the range <code>start &hellip; end</code>
     * of <code>source</code>.  Return the CharSequence that you would
     * like to have placed there instead, including an empty string
     * if appropriate, or <code>null</code> to accept the original
     * replacement.  Be careful to not to reject 0-length replacements,
     * as this is what happens when you delete text.  Also beware that
     * you should not attempt to make any changes to <code>dest</code>
     * from this method; you may only examine it for context.
     *
     * Note: If <var>source</var> is an instance of {@link Spanned} or
     * {@link Spannable}, the span objects in the <var>source</var> should be
     * copied into the filtered result (i.e. the non-null return value).
     * {@link TextUtils#copySpansFrom} can be used for convenience if the
     * span boundary indices would be remaining identical relative to the source.
     */
public:
    CharSequence* filter(CharSequence* source, int start, int end, Spanned* dest, int dstart, int dend)=0;

    /**
     * This filter will capitalize all the lowercase and titlecase letters that are added
     * through edits. (Note that if there are no lowercase or titlecase letters in the input, the
     * text would not be transformed, even if the result of capitalization of the string is
     * different from the string.)
     */
    class AllCaps:public InputFilter {
        private final Locale mLocale;

        public AllCaps() {
            mLocale = nullptr;
        }

        /**
         * Constructs a locale-specific AllCaps filter, to make sure capitalization rules of that
         * locale are used for transforming the sequence.
         */
        public AllCaps(@NonNull Locale locale) {
            mLocale = locale;
        }

        public CharSequence* filter(CharSequence source, int start, int end,
                                   Spanned dest, int dstart, int dend) {
            CharSequence* wrapper = new CharSequenceWrapper(source, start, end);

            bool lowerOrTitleFound = false;
            const int length = end - start;
            for (int i = 0, cp; i < length; i += Character::charCount(cp)) {
                // We access 'wrapper' instead of 'source' to make sure no code unit beyond 'end' is
                // ever accessed.
                cp = Character::codePointAt(wrapper, i);
                if (Character::isLowerCase(cp) || Character::isTitleCase(cp)) {
                    lowerOrTitleFound = true;
                    break;
                }
            }
            if (!lowerOrTitleFound) {
                return null; // keep original
            }

            const bool copySpans = source instanceof Spanned;
            CharSequence* upper = TextUtils::toUpperCase(mLocale, wrapper, copySpans);
            if (upper == wrapper) {
                // Nothing was changed in the uppercasing operation. This is weird, since
                // we had found at least one lowercase or titlecase character. But we can't
                // do anything better than keeping the original in this case.
                return nullptr; // keep original
            }
            // Return a SpannableString or String for backward compatibility.
            return copySpans ? new SpannableString(upper) : upper.toString();
        }

        private:
        class CharSequenceWrapper:public CharSequence, Spanned {
            private CharSequence* mSource;
            private int mStart, mEnd;
            private int mLength;

            CharSequenceWrapper(CharSequence* source, int start, int end) {
                mSource = source;
                mStart = start;
                mEnd = end;
                mLength = end - start;
            }

            public int length() const{
                return mLength;
            }

            public char16_t charAt(int index) const{
                if (index < 0 || index >= mLength) {
                    throw new IndexOutOfBoundsException();
                }
                return mSource->charAt(mStart + index);
            }

            public CharSequence* subSequence(int start, int end) {
                if (start < 0 || end < 0 || end > mLength || start > end) {
                    throw new IndexOutOfBoundsException();
                }
                return new CharSequenceWrapper(mSource, mStart + start, mStart + end);
            }

            public String toString() {
                return mSource.subSequence(mStart, mEnd).toString();
            }

            public <T> T[] getSpans(int start, int end, Class<T> type) {
                return ((Spanned) mSource).getSpans(mStart + start, mStart + end, type);
            }

            public int getSpanStart(Object tag) {
                return ((Spanned) mSource).getSpanStart(tag) - mStart;
            }

            public int getSpanEnd(Object tag) {
                return ((Spanned) mSource).getSpanEnd(tag) - mStart;
            }

            public int getSpanFlags(Object tag) {
                return ((Spanned) mSource).getSpanFlags(tag);
            }

            public int nextSpanTransition(int start, int limit, Class type) {
                return ((Spanned) mSource).nextSpanTransition(mStart + start, mStart + limit, type)
                        - mStart;
            }
        };
    };

    /**
     * This filter will constrain edits not to make the length of the text
     * greater than the specified length.
     */
    class LengthFilter:public InputFilter {
    private:
        int mMax;
    public:
        LengthFilter(int max) {
            mMax = max;
        }
        CharSequence* filter(CharSequence source, int start, int end, Spanned dest,
                int dstart, int dend) {
            int keep = mMax - (dest.length() - (dend - dstart));
            if (keep <= 0) {
                return "";
            } else if (keep >= end - start) {
                return nullptr; // keep original
            } else {
                keep += start;
                if (Character::isHighSurrogate(source.charAt(keep - 1))) {
                    --keep;
                    if (keep == start) {
                        return "";
                    }
                }
                return source.subSequence(start, keep);
            }
        }
        int getMax() const{
            return mMax;
        }
    };
};
}/*endof namespace*/
