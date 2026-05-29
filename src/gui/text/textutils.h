public class TextUtils {
    private static final String TAG = "TextUtils";

    // Zero-width character used to fill ellipsized strings when codepoint length must be preserved.
    static constexpr char ELLIPSIS_FILLER = 0xFEFF; // ZERO WIDTH NO-BREAK SPACE

    private static final String ELLIPSIS_NORMAL = "\u2026"; // HORIZONTAL ELLIPSIS (…)
    private static final String ELLIPSIS_TWO_DOTS = "\u2025"; // TWO DOT LEADER (‥)

    private static constexpr int LINE_FEED_CODE_POINT = 10;
    private static constexpr int NBSP_CODE_POINT = 160;

    public static constexpr int SAFE_STRING_FLAG_TRIM = 0x1;
    public static constexpr int SAFE_STRING_FLAG_SINGLE_LINE = 0x2;
    public static constexpr int SAFE_STRING_FLAG_FIRST_LINE = 0x4;

    public static String getEllipsisString(TextUtils::TruncateAt method) {
        return (method == TextUtils::TruncateAt::END_SMALL) ? ELLIPSIS_TWO_DOTS : ELLIPSIS_NORMAL;
    }


    private TextUtils() { /* cannot be instantiated */ }

    public static void getChars(CharSequence s, int start, int end,  char[] dest, int destoff);

    public static int indexOf(CharSequence s, char ch) {
        return indexOf(s, ch, 0);
    }

    public static int indexOf(CharSequence s, char ch, int start);

    public static int indexOf(CharSequence s, char ch, int start, int end);

    public static int lastIndexOf(CharSequence s, char ch) {
        return lastIndexOf(s, ch, s.length() - 1);
    }

    public static int lastIndexOf(CharSequence s, char ch, int last) {
        Class<? extends CharSequence> c = s.getClass();

        if (c == String.class)
            return ((String) s).lastIndexOf(ch, last);

        return lastIndexOf(s, ch, 0, last);
    }

    public static int lastIndexOf(CharSequence s, char ch, int start, int last);
    public static int indexOf(CharSequence s, CharSequence needle) {
        return indexOf(s, needle, 0, s.length());
    }

    public static int indexOf(CharSequence s, CharSequence needle, int start) {
        return indexOf(s, needle, start, s.length());
    }

    public static int indexOf(CharSequence s, CharSequence needle, int start, int end);

    public static boolean regionMatches(CharSequence one, int toffset,
                                        CharSequence two, int ooffset,
                                        int len);

    public static String substring(CharSequence source, int start, int end);
    public static String join(@NonNull CharSequence delimiter, @NonNull Object[] tokens);

    public static String join(@NonNull CharSequence delimiter, @NonNull Iterable tokens);

    public static String[] split(String text, String expression) {
        if (text.length() == 0) {
            return EMPTY_STRING_ARRAY;
        } else {
            return text.split(expression, -1);
        }
    }

    public static String[] split(String text, Pattern pattern) {
        if (text.length() == 0) {
            return EMPTY_STRING_ARRAY;
        } else {
            return pattern.split(text, -1);
        }
    }

    public interface StringSplitter extends Iterable<String> {
        public void setString(String string);
    }

    public static class SimpleStringSplitter implements StringSplitter, Iterator<String> {
        private String mString;
        private char mDelimiter;
        private int mPosition;
        private int mLength;

        public SimpleStringSplitter(char delimiter) {
            mDelimiter = delimiter;
        }

        public void setString(String string) {
            mString = string;
            mPosition = 0;
            mLength = mString.length();
        }

        public Iterator<String> iterator() {
            return this;
        }

        public boolean hasNext() {
            return mPosition < mLength;
        }

        public String next() {
            int end = mString.indexOf(mDelimiter, mPosition);
            if (end == -1) {
                end = mLength;
            }
            String nextString = mString.substring(mPosition, end);
            mPosition = end + 1; // Skip the delimiter.
            return nextString;
        }

        public void remove() {
            throw new UnsupportedOperationException();
        }
    }

    public static CharSequence stringOrSpannedString(CharSequence source) {
        if (source == null)
            return null;
        if (source instanceof SpannedString)
            return source;
        if (source instanceof Spanned)
            return new SpannedString(source);

        return source.toString();
    }

    public static boolean isEmpty(@Nullable CharSequence str) {
        return str == null || str.length() == 0;
    }

    /** {@hide} */
    public static String nullIfEmpty(@Nullable String str) {
        return isEmpty(str) ? null : str;
    }

    /** {@hide} */
    public static String emptyIfNull(@Nullable String str) {
        return str == null ? "" : str;
    }

    /** {@hide} */
    public static String firstNotEmpty(@Nullable String a, @NonNull String b) {
        return !isEmpty(a) ? a : Preconditions.checkStringNotEmpty(b);
    }

    /** {@hide} */
    public static int length(@Nullable String s) {
        return s != null ? s.length() : 0;
    }

    public static String safeIntern(String s) {
        return (s != null) ? s.intern() : null;
    }

    public static int getTrimmedLength(CharSequence s) {
        int len = s.length();

        int start = 0;
        while (start < len && s.charAt(start) <= ' ') {
            start++;
        }

        int end = len;
        while (end > start && s.charAt(end - 1) <= ' ') {
            end--;
        }

        return end - start;
    }

    public static boolean equals(CharSequence a, CharSequence b) {
        if (a == b) return true;
        int length;
        if (a != null && b != null && (length = a.length()) == b.length()) {
            if (a instanceof String && b instanceof String) {
                return a.equals(b);
            } else {
                for (int i = 0; i < length; i++) {
                    if (a.charAt(i) != b.charAt(i)) return false;
                }
                return true;
            }
        }
        return false;
    }

    public static CharSequence getReverse(CharSequence source, int start, int end) {
        return new Reverser(source, start, end);
    }

    private static class Reverser  implements CharSequence, GetChars
    {
        public Reverser(CharSequence source, int start, int end) {
            mSource = source;
            mStart = start;
            mEnd = end;
        }

        public int length() {
            return mEnd - mStart;
        }

        public CharSequence subSequence(int start, int end) {
            char[] buf = new char[end - start];

            getChars(start, end, buf, 0);
            return new String(buf);
        }

        @Override
        public String toString() {
            return subSequence(0, length()).toString();
        }

        public char charAt(int off) {
            return (char) UCharacter.getMirror(mSource.charAt(mEnd - 1 - off));
        }

        @SuppressWarnings("deprecation")
        public void getChars(int start, int end, char[] dest, int destoff) {
            TextUtils.getChars(mSource, start + mStart, end + mStart,
                               dest, destoff);
            AndroidCharacter.mirror(dest, 0, end - start);

            int len = end - start;
            int n = (end - start) / 2;
            for (int i = 0; i < n; i++) {
                char tmp = dest[destoff + i];

                dest[destoff + i] = dest[destoff + len - i - 1];
                dest[destoff + len - i - 1] = tmp;
            }
        }

        private CharSequence mSource;
        private int mStart;
        private int mEnd;
    }

    /** @hide */
    public static final int ALIGNMENT_SPAN = 1;
    /** @hide */
    public static final int FIRST_SPAN = ALIGNMENT_SPAN;
    /** @hide */
    public static final int FOREGROUND_COLOR_SPAN = 2;
    /** @hide */
    public static final int RELATIVE_SIZE_SPAN = 3;
    /** @hide */
    public static final int SCALE_X_SPAN = 4;
    /** @hide */
    public static final int STRIKETHROUGH_SPAN = 5;
    /** @hide */
    public static final int UNDERLINE_SPAN = 6;
    /** @hide */
    public static final int STYLE_SPAN = 7;
    /** @hide */
    public static final int BULLET_SPAN = 8;
    /** @hide */
    public static final int QUOTE_SPAN = 9;
    /** @hide */
    public static final int LEADING_MARGIN_SPAN = 10;
    /** @hide */
    public static final int URL_SPAN = 11;
    /** @hide */
    public static final int BACKGROUND_COLOR_SPAN = 12;
    /** @hide */
    public static final int TYPEFACE_SPAN = 13;
    /** @hide */
    public static final int SUPERSCRIPT_SPAN = 14;
    /** @hide */
    public static final int SUBSCRIPT_SPAN = 15;
    /** @hide */
    public static final int ABSOLUTE_SIZE_SPAN = 16;
    /** @hide */
    public static final int TEXT_APPEARANCE_SPAN = 17;
    /** @hide */
    public static final int ANNOTATION = 18;
    /** @hide */
    public static final int SUGGESTION_SPAN = 19;
    /** @hide */
    public static final int SPELL_CHECK_SPAN = 20;
    /** @hide */
    public static final int SUGGESTION_RANGE_SPAN = 21;
    /** @hide */
    public static final int EASY_EDIT_SPAN = 22;
    /** @hide */
    public static final int LOCALE_SPAN = 23;
    /** @hide */
    public static final int TTS_SPAN = 24;
    /** @hide */
    public static final int ACCESSIBILITY_CLICKABLE_SPAN = 25;
    /** @hide */
    public static final int ACCESSIBILITY_URL_SPAN = 26;
    /** @hide */
    public static final int LINE_BACKGROUND_SPAN = 27;
    /** @hide */
    public static final int LINE_HEIGHT_SPAN = 28;
    /** @hide */
    public static final int ACCESSIBILITY_REPLACEMENT_SPAN = 29;
    /** @hide */
    public static final int LAST_SPAN = ACCESSIBILITY_REPLACEMENT_SPAN;

    /**
     * Flatten a CharSequence and whatever styles can be copied across processes
     * into the parcel.
     */
    public static void writeToParcel(@Nullable CharSequence cs, @NonNull Parcel p,
            int parcelableFlags);
    private static void writeWhere(Parcel p, Spanned sp, Object o) {
        p.writeInt(sp.getSpanStart(o));
        p.writeInt(sp.getSpanEnd(o));
        p.writeInt(sp.getSpanFlags(o));
    }

    public static final Parcelable.Creator<CharSequence> CHAR_SEQUENCE_CREATOR

    public static void dumpSpans(CharSequence cs, Printer printer, String prefix) {
        if (cs instanceof Spanned) {
            Spanned sp = (Spanned) cs;
            Object[] os = sp.getSpans(0, cs.length(), Object.class);

            for (int i = 0; i < os.length; i++) {
                Object o = os[i];
                printer.println(prefix + cs.subSequence(sp.getSpanStart(o),
                        sp.getSpanEnd(o)) + ": "
                        + Integer.toHexString(System.identityHashCode(o))
                        + " " + o.getClass().getCanonicalName()
                         + " (" + sp.getSpanStart(o) + "-" + sp.getSpanEnd(o)
                         + ") fl=#" + sp.getSpanFlags(o));
            }
        } else {
            printer.println(prefix + cs + ": (no spans)");
        }
    }

    public static CharSequence replace(CharSequence template,
                                       String[] sources,
                                       CharSequence[] destinations);
    public static CharSequence expandTemplate(CharSequence template,
                                              CharSequence... values);

    public static int getOffsetBefore(CharSequence text, int offset);
    public static int getOffsetAfter(CharSequence text, int offset);

    private static void readSpan(Parcel p, Spannable sp, Object o) {
        sp.setSpan(o, p.readInt(), p.readInt(), p.readInt());
    }

    public static void copySpansFrom(Spanned source, int start, int end,
                                     Class kind,
                                     Spannable dest, int destoff) {
        if (kind == null) {
            kind = Object.class;
        }

        Object[] spans = source.getSpans(start, end, kind);

        for (int i = 0; i < spans.length; i++) {
            int st = source.getSpanStart(spans[i]);
            int en = source.getSpanEnd(spans[i]);
            int fl = source.getSpanFlags(spans[i]);

            if (st < start)
                st = start;
            if (en > end)
                en = end;

            dest.setSpan(spans[i], st - start + destoff, en - start + destoff,
                         fl);
        }
    }

    public static CharSequence toUpperCase(@Nullable Locale locale, @NonNull CharSequence source,
            boolean copySpans);

    // helper method for toUpperCase()
    private static int toUpperMapToDest(Edits.Iterator iterator, int sourceIndex);
    public enum TruncateAt {
        START,
        MIDDLE,
        END,
        MARQUEE,
        /**
         * @hide
         */
        @UnsupportedAppUsage
        END_SMALL
    }

    public interface EllipsizeCallback {
        public void ellipsized(int start, int end);
    }

    public static CharSequence ellipsize(CharSequence text,
                                         TextPaint p,
                                         float avail, TruncateAt where) {
        return ellipsize(text, p, avail, where, false, null);
    }

    public static CharSequence ellipsize(CharSequence text,
                                         TextPaint paint,
                                         float avail, TruncateAt where,
                                         boolean preserveLength,
                                         @Nullable EllipsizeCallback callback) {
        return ellipsize(text, paint, avail, where, preserveLength, callback,
                TextDirectionHeuristics.FIRSTSTRONG_LTR,
                getEllipsisString(where));
    }

    public static CharSequence ellipsize(CharSequence text,
            TextPaint paint,
            float avail, TruncateAt where,
            boolean preserveLength,
            @Nullable EllipsizeCallback callback,
            TextDirectionHeuristic textDir, String ellipsis);

    public static CharSequence listEllipsize(@Nullable Context context,
            @Nullable List<CharSequence> elements, @NonNull String separator,
            @NonNull TextPaint paint, @FloatRange(from=0.0,fromInclusive=false) float avail,
            @PluralsRes int moreId);
    public static CharSequence commaEllipsize(CharSequence text,
                                              TextPaint p, float avail,
                                              String oneMore,
                                              String more) {
        return commaEllipsize(text, p, avail, oneMore, more,
                TextDirectionHeuristics.FIRSTSTRONG_LTR);
    }

    public static CharSequence commaEllipsize(CharSequence text, TextPaint p,
         float avail, String oneMore, String more, TextDirectionHeuristic textDir);

    static boolean couldAffectRtl(char c) {
        return (0x0590 <= c && c <= 0x08FF) ||  // RTL scripts
                c == 0x200E ||  // Bidi format character
                c == 0x200F ||  // Bidi format character
                (0x202A <= c && c <= 0x202E) ||  // Bidi format characters
                (0x2066 <= c && c <= 0x2069) ||  // Bidi format characters
                (0xD800 <= c && c <= 0xDFFF) ||  // Surrogate pairs
                (0xFB1D <= c && c <= 0xFDFF) ||  // Hebrew and Arabic presentation forms
                (0xFE70 <= c && c <= 0xFEFE);  // Arabic presentation forms
    }

    static boolean doesNotNeedBidi(char[] text, int start, int len) {
        final int end = start + len;
        for (int i = start; i < end; i++) {
            if (couldAffectRtl(text[i])) {
                return false;
            }
        }
        return true;
    }

    /* package */ static char[] obtain(int len) {
        char[] buf;

        synchronized (sLock) {
            buf = sTemp;
            sTemp = null;
        }

        if (buf == null || buf.length < len)
            buf = ArrayUtils.newUnpaddedCharArray(len);

        return buf;
    }

    /* package */ static void recycle(char[] temp) {
        if (temp.length > 1000)
            return;

        synchronized (sLock) {
            sTemp = temp;
        }
    }

    public static String htmlEncode(String s);
    public static CharSequence concat(CharSequence... text);

    public static boolean isGraphic(CharSequence str);
    public static boolean isGraphic(char c);
    public static boolean isDigitsOnly(CharSequence str);
    public static boolean isPrintableAscii(final char c);
    public static boolean isPrintableAsciiOnly(final CharSequence str);

    public static final int CAP_MODE_CHARACTERS
            = InputType.TYPE_TEXT_FLAG_CAP_CHARACTERS;

    public static final int CAP_MODE_WORDS
            = InputType.TYPE_TEXT_FLAG_CAP_WORDS;

    public static final int CAP_MODE_SENTENCES
            = InputType.TYPE_TEXT_FLAG_CAP_SENTENCES;

    public static int getCapsMode(CharSequence cs, int off, int reqModes);

    public static boolean delimitedStringContains(
            String delimitedString, char delimiter, String item);

    public static <T> T[] removeEmptySpans(T[] spans, Spanned spanned, Class<T> klass);
    public static long packRangeInLong(int start, int end) {
        return (((long) start) << 32) | end;
    }

    public static int unpackRangeStartFromLong(long range) {
        return (int) (range >>> 32);
    }

    public static int unpackRangeEndFromLong(long range) {
        return (int) (range & 0x00000000FFFFFFFFL);
    }

    public static int getLayoutDirectionFromLocale(Locale locale) {
        return ((locale != null && !locale.equals(Locale.ROOT)
                        && ULocale.forLocale(locale).isRightToLeft())
                // If forcing into RTL layout mode, return RTL as default
                || DisplayProperties.debug_force_rtl().orElse(false))
            ? View.LAYOUT_DIRECTION_RTL
            : View.LAYOUT_DIRECTION_LTR;
    }

    public static CharSequence formatSelectedCount(int count) {
        return Resources.getSystem().getQuantityString(R.plurals.selected_count, count, count);
    }

    public static boolean hasStyleSpan(@NonNull Spanned spanned) {
        Preconditions.checkArgument(spanned != null);
        final Class<?>[] styleClasses = {
                CharacterStyle.class, ParagraphStyle.class, UpdateAppearance.class};
        for (Class<?> clazz : styleClasses) {
            if (spanned.nextSpanTransition(-1, spanned.length(), clazz) < spanned.length()) {
                return true;
            }
        }
        return false;
    }

    public static CharSequence trimNoCopySpans(@Nullable CharSequence charSequence) {
        if (charSequence != null && charSequence instanceof Spanned) {
            // SpannableStringBuilder copy constructor trims NoCopySpans.
            return new SpannableStringBuilder(charSequence);
        }
        return charSequence;
    }

    public static void wrap(StringBuilder builder, String start, String end) {
        builder.insert(0, start);
        builder.append(end);
    }

    private static final int PARCEL_SAFE_TEXT_LENGTH = 100000;

    public static <T extends CharSequence> T trimToParcelableSize(@Nullable T text) {
        return trimToSize(text, PARCEL_SAFE_TEXT_LENGTH);
    }

    public static <T extends CharSequence> T trimToSize(@Nullable T text,
            @IntRange(from = 1) int size) {
        Preconditions.checkArgument(size > 0);
        if (TextUtils.isEmpty(text) || text.length() <= size) return text;
        if (Character.isHighSurrogate(text.charAt(size - 1))
                && Character.isLowSurrogate(text.charAt(size))) {
            size = size - 1;
        }
        return (T) text.subSequence(0, size);
    }

    public static <T extends CharSequence> T trimToLengthWithEllipsis(@Nullable T text,
            @IntRange(from = 1) int size) {
        T trimmed = trimToSize(text, size);
        if (trimmed.length() < text.length()) {
            trimmed = (T) (trimmed.toString() + "...");
        }
        return trimmed;
    }

    private static boolean isNewline(int codePoint) {
        int type = Character.getType(codePoint);
        return type == Character.PARAGRAPH_SEPARATOR || type == Character.LINE_SEPARATOR
                || codePoint == LINE_FEED_CODE_POINT;
    }

    private static boolean isWhiteSpace(int codePoint) {
        return Character.isWhitespace(codePoint) || codePoint == NBSP_CODE_POINT;
    }

    public static String withoutPrefix(@Nullable String prefix, @Nullable String str) {
        if (prefix == null || str == null) return str;
        return str.startsWith(prefix) ? str.substring(prefix.length()) : str;
    }

    public static @NonNull CharSequence makeSafeForPresentation(@NonNull String unclean,
            @IntRange(from = 0) int maxCharactersToConsider,
            @FloatRange(from = 0) float ellipsizeDip, @SafeStringFlags int flags);

    private static class StringWithRemovedChars {
        /** The original string */
        private final String mOriginal;
        private BitSet mRemovedChars;

        StringWithRemovedChars(@NonNull String original) {
            mOriginal = original;
        }

        void removeRange(int firstRemoved, int firstNonRemoved) {
            if (mRemovedChars == null) {
                mRemovedChars = new BitSet(mOriginal.length());
            }

            mRemovedChars.set(firstRemoved, firstNonRemoved);
        }

        void removeAllCharBefore(int firstNonRemoved) {
            if (mRemovedChars == null) {
                mRemovedChars = new BitSet(mOriginal.length());
            }

            mRemovedChars.set(0, firstNonRemoved);
        }

        void removeAllCharAfter(int firstRemoved) {
            if (mRemovedChars == null) {
                mRemovedChars = new BitSet(mOriginal.length());
            }

            mRemovedChars.set(firstRemoved, mOriginal.length());
        }

        @Override
        public String toString() {
            // Common case, no chars removed
            if (mRemovedChars == null) {
                return mOriginal;
            }

            StringBuilder sb = new StringBuilder(mOriginal.length());
            for (int i = 0; i < mOriginal.length(); i++) {
                if (!mRemovedChars.get(i)) {
                    sb.append(mOriginal.charAt(i));
                }
            }

            return sb.toString();
        }

        int length() {
            return mOriginal.length();
        }

        int codePointAt(int offset) {
            return mOriginal.codePointAt(offset);
        }
    }

    private static Object sLock = new Object();
    private static char[] sTemp = null;
    private static String[] EMPTY_STRING_ARRAY = new String[]{};
}

