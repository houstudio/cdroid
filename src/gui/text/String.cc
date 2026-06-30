
public final class String
    implements java.io.Serializable, Comparable<String>, CharSequence,
               Constable, ConstantDesc {
    private final byte[] value;
    */
    private final int count;
    /** Cache the hash code for the string */
    private int hash; // Default to 0

    static final boolean COMPACT_STRINGS = true;

    // Android-added: Add a canonical empty string used by ART.
    /** @hide */
    public static final String EMPTY = "";

    @Native static final byte CODER_LATIN1 = 0;

    @Native static final byte CODER_UTF16 = 1;
    private static final ObjectStreamField[] serialPersistentFields =
        new ObjectStreamField[0];

    public String() {
        // BEGIN Android-changed: Implemented as compiler and runtime intrinsics.
        /*
        this.value = "".value;
        this.coder = "".coder;
         */
        throw new UnsupportedOperationException("Use StringFactory instead.");
        // END Android-changed: Implemented as compiler and runtime intrinsics.
    }

    public String(String original) {
        // BEGIN Android-changed: Implemented as compiler and runtime intrinsics.
        /*
        this.value = original.value;
        this.coder = original.coder;
        this.hash = original.hash;
         */
        throw new UnsupportedOperationException("Use StringFactory instead.");
        // END Android-changed: Implemented as compiler and runtime intrinsics.
    }

    public String(char value[]) {
        // BEGIN Android-changed: Implemented as compiler and runtime intrinsics.
        /*
        this(value, 0, value.length, null);
         */
        throw new UnsupportedOperationException("Use StringFactory instead.");
        // END Android-changed: Implemented as compiler and runtime intrinsics.
    }

    public String(char value[], int offset, int count) {
        // BEGIN Android-changed: Implemented as compiler and runtime intrinsics.
        throw new UnsupportedOperationException("Use StringFactory instead.");
        // END Android-changed: Implemented as compiler and runtime intrinsics.
    }

    public String(int[] codePoints, int offset, int count) {
        // BEGIN Android-changed: Implemented as compiler and runtime intrinsics.
        /*
        checkBoundsOffCount(offset, count, codePoints.length);
        if (count == 0) {
            this.value = "".value;
            this.coder = "".coder;
            return;
        }
        if (COMPACT_STRINGS) {
            byte[] val = StringLatin1.toBytes(codePoints, offset, count);
            if (val != null) {
                this.coder = LATIN1;
                this.value = val;
                return;
            }
        }
        this.coder = UTF16;
        this.value = StringUTF16.toBytes(codePoints, offset, count);
        */
        throw new UnsupportedOperationException("Use StringFactory instead.");
        // END Android-changed: Implemented as compiler and runtime intrinsics.
    }

    @Deprecated(since="1.1")
    public String(byte ascii[], int hibyte, int offset, int count) {
        // BEGIN Android-changed: Implemented as compiler and runtime intrinsics.
        /*
        checkBoundsOffCount(offset, count, ascii.length);
        if (count == 0) {
            this.value = "".value;
            this.coder = "".coder;
            return;
        }
        if (COMPACT_STRINGS && (byte)hibyte == 0) {
            this.value = Arrays.copyOfRange(ascii, offset, offset + count);
            this.coder = LATIN1;
        } else {
            hibyte <<= 8;
            byte[] val = StringUTF16.newBytesFor(count);
            for (int i = 0; i < count; i++) {
                StringUTF16.putChar(val, i, hibyte | (ascii[offset++] & 0xff));
            }
            this.value = val;
            this.coder = UTF16;
        }
        */
        throw new UnsupportedOperationException("Use StringFactory instead.");
        // END Android-changed: Implemented as compiler and runtime intrinsics.
    }

    @Deprecated(since="1.1")
    public String(byte ascii[], int hibyte) {
        // BEGIN Android-changed: Implemented as compiler and runtime intrinsics.
        /*
        this(ascii, hibyte, 0, ascii.length);
        */
        throw new UnsupportedOperationException("Use StringFactory instead.");
        // END Android-changed: Implemented as compiler and runtime intrinsics.
    }

    public String(byte bytes[], int offset, int length, String charsetName)
            throws UnsupportedEncodingException {
        // BEGIN Android-changed: Implemented as compiler and runtime intrinsics.
        /*
        if (charsetName == null)
            throw new NullPointerException("charsetName");
        checkBoundsOffCount(offset, length, bytes.length);
        StringCoding.Result ret =
            StringCoding.decode(charsetName, bytes, offset, length);
        this.value = ret.value;
        this.coder = ret.coder;
        */
        throw new UnsupportedOperationException("Use StringFactory instead.");
        // END Android-changed: Implemented as compiler and runtime intrinsics.
    }

    public String(byte bytes[], int offset, int length, Charset charset) {
        // BEGIN Android-changed: Implemented as compiler and runtime intrinsics.
        /*
        if (charset == null)
            throw new NullPointerException("charset");
        checkBoundsOffCount(offset, length, bytes.length);
        StringCoding.Result ret =
            StringCoding.decode(charset, bytes, offset, length);
        this.value = ret.value;
        this.coder = ret.coder;
        */
        throw new UnsupportedOperationException("Use StringFactory instead.");
        // END Android-changed: Implemented as compiler and runtime intrinsics.
    }

    public String(byte bytes[], String charsetName)
            throws UnsupportedEncodingException {
        // BEGIN Android-changed: Implemented as compiler and runtime intrinsics.
        /*
        this(bytes, 0, bytes.length, charsetName);
         */
        throw new UnsupportedOperationException("Use StringFactory instead.");
        // END Android-changed: Implemented as compiler and runtime intrinsics.
    }

    public String(byte bytes[], Charset charset) {
        // BEGIN Android-changed: Implemented as compiler and runtime intrinsics.
        /*
        this(bytes, 0, bytes.length, charset);
         */
        throw new UnsupportedOperationException("Use StringFactory instead.");
        // END Android-changed: Implemented as compiler and runtime intrinsics.
    }

    public String(byte bytes[], int offset, int length) {
        // BEGIN Android-changed: Implemented as compiler and runtime intrinsics.
        /*
        checkBoundsOffCount(offset, length, bytes.length);
        StringCoding.Result ret = StringCoding.decode(bytes, offset, length);
        this.value = ret.value;
        this.coder = ret.coder;
         */
        throw new UnsupportedOperationException("Use StringFactory instead.");
        // END Android-changed: Implemented as compiler and runtime intrinsics.
    }

    public String(byte[] bytes) {
        // BEGIN Android-changed: Implemented as compiler and runtime intrinsics.
        /*
        this(bytes, 0, bytes.length);
         */
        throw new UnsupportedOperationException("Use StringFactory instead.");
        // END Android-changed: Implemented as compiler and runtime intrinsics.
    }

    public String(StringBuffer buffer) {
        // BEGIN Android-changed: Implemented as compiler and runtime intrinsics.
        /*
        this(buffer.toString());
         */
        throw new UnsupportedOperationException("Use StringFactory instead.");
        // END Android-changed: Implemented as compiler and runtime intrinsics.
    }

    public String(StringBuilder builder) {
        // BEGIN Android-changed: Implemented as compiler and runtime intrinsics.
        /*
        this(builder, null);
         */
        throw new UnsupportedOperationException("Use StringFactory instead.");
        // END Android-changed: Implemented as compiler and runtime intrinsics.
    }

    @Deprecated
    String(int offset, int count, char[] value) {
        throw new UnsupportedOperationException("Use StringFactory instead.");
    }
    // END Android-added: Constructor for internal use.

    public int length() {
        // BEGIN Android-changed: Get length from count field rather than value array (see above).
        /*
        return value.length >> coder();
        */
        final boolean STRING_COMPRESSION_ENABLED = true;
        if (STRING_COMPRESSION_ENABLED) {
            // For the compression purposes (save the characters as 8-bit if all characters
            // are ASCII), the least significant bit of "count" is used as the compression flag.
            return (count >>> 1);
        } else {
            return count;
        }
        // END Android-changed: Get length from count field rather than value array (see above).
    }

    @Override
    public boolean isEmpty() {
        // BEGIN Android-changed: Get length from count field rather than value array (see above).
        // Empty string has {@code count == 0} with or without string compression enabled.
        /*
        return value.length == 0;
         */
        return count == 0;
        // END Android-changed: Get length from count field rather than value array (see above).
    }

    @FastNative
    public native char charAt(int index);
    public int codePointAt(int index) {
        // BEGIN Android-changed: delegate codePointAt() to Character class.
        /*
        if (isLatin1()) {
            checkIndex(index, value.length);
            return value[index] & 0xff;
        }
        int length = value.length >> 1;
        checkIndex(index, length);
        return StringUTF16.codePointAt(value, index, length);
         */
        checkIndex(index, length());
        return Character.codePointAt(this, index);
    }

    public int codePointBefore(int index) {
        int i = index - 1;
        if (i < 0 || i >= length()) {
            throw new StringIndexOutOfBoundsException(index);
        }
        // BEGIN Android-changed: delegate codePointBefore to Character class.
        /*
        if (isLatin1()) {
            return (value[i] & 0xff);
        }
        return StringUTF16.codePointBefore(value, index);
         */
        return Character.codePointBefore(this, index);
    }

    public int codePointCount(int beginIndex, int endIndex) {
        if (beginIndex < 0 || beginIndex > endIndex ||
            endIndex > length()) {
            throw new IndexOutOfBoundsException();
        }
        // BEGIN Android-changed: delegate codePointCount to Character class.
        /*
        if (isLatin1()) {
            return endIndex - beginIndex;
        }
        return StringUTF16.codePointCount(value, beginIndex, endIndex);
         */
        return Character.codePointCount(this, beginIndex, endIndex);
        // END Android-changed: delegate codePointCount to Character class.
    }

    public int offsetByCodePoints(int index, int codePointOffset) {
        if (index < 0 || index > length()) {
            throw new IndexOutOfBoundsException();
        }
        return Character.offsetByCodePoints(this, index, codePointOffset);
    }

    void getChars(char dst[], int dstBegin) {
        // Android-changed: Replace arraycopy with native call since chars are managed by runtime.
        // System.arraycopy(value, 0, dst, dstBegin, value.length);
        getCharsNoCheck(0, length(), dst, dstBegin);
    }

    public void getChars(int srcBegin, int srcEnd, char dst[], int dstBegin) {
        // BEGIN Android-added: Null pointer check.
        if (dst == null) {
            throw new NullPointerException("dst == null");
        }
        // END Android-added: Null pointer check.
        checkBoundsBeginEnd(srcBegin, srcEnd, length());
        // BEGIN Android-changed: Implement in terms of length() and native getCharsNoCheck method.
        /*
        checkBoundsOffCount(dstBegin, srcEnd - srcBegin, dst.length);
        if (isLatin1()) {
            StringLatin1.getChars(value, srcBegin, srcEnd, dst, dstBegin);
        } else {
            StringUTF16.getChars(value, srcBegin, srcEnd, dst, dstBegin);
        }
        */
        if (dstBegin < 0) {
            throw new ArrayIndexOutOfBoundsException("dstBegin < 0. dstBegin=" + dstBegin);
        }
        // dstBegin can be equal to dst.length, but only in the case where zero chars are to be
        // copied.
        if (dstBegin > dst.length) {
            throw new ArrayIndexOutOfBoundsException(
                    "dstBegin > dst.length. dstBegin=" + dstBegin + ", dst.length=" + dst.length);
        }

        int n = srcEnd - srcBegin;
        if (n > dst.length - dstBegin) {
            throw new ArrayIndexOutOfBoundsException(
                    "n > dst.length - dstBegin. n=" + n + ", dst.length=" + dst.length
                            + "dstBegin=" + dstBegin);
        }

        getCharsNoCheck(srcBegin, srcEnd, dst, dstBegin);
        // END Android-changed: Implement in terms of length() and native getCharsNoCheck method.
    }

    @FastNative
    native void getCharsNoCheck(int start, int end, char[] buffer, int index);
    // END Android-added: Native method to access char storage managed by runtime.

    @Deprecated(since="1.1")
    public void getBytes(int srcBegin, int srcEnd, byte dst[], int dstBegin) {
        checkBoundsBeginEnd(srcBegin, srcEnd, length());
        Objects.requireNonNull(dst);
        checkBoundsOffCount(dstBegin, srcEnd - srcBegin, dst.length);
        // BEGIN Android-changed: Implement in terms of charAt().
        /*
        if (isLatin1()) {
            StringLatin1.getBytes(value, srcBegin, srcEnd, dst, dstBegin);
        } else {
            StringUTF16.getBytes(value, srcBegin, srcEnd, dst, dstBegin);
        }
         */
        int j = dstBegin;
        int n = srcEnd;
        int i = srcBegin;

        while (i < n) {
            dst[j++] = (byte)charAt(i++);
        }
        // END Android-changed: Implement in terms of charAt().
    }

    public byte[] getBytes(String charsetName)
            throws UnsupportedEncodingException {
        if (charsetName == null) throw new NullPointerException();
        // BEGIN Android-changed: Skip StringCoding optimization that needs access to java chars.
        /*
        return StringCoding.encode(charsetName, coder(), value);
         */
        return getBytes(Charset.forNameUEE(charsetName));
        // END Android-changed: Skip StringCoding optimization that needs access to java chars.
    }

    public byte[] getBytes(Charset charset) {
        if (charset == null) throw new NullPointerException();
        // BEGIN Android-changed: Skip StringCoding optimization that needs access to java chars.
        /*
        return StringCoding.encode(charset, coder(), value);
        */
        final int len = length();
        final String name = charset.name();
        if ("UTF-8".equals(name)) {
            return CharsetUtils.toUtf8Bytes(this, 0, len);
        } else if ("ISO-8859-1".equals(name)) {
            return CharsetUtils.toIsoLatin1Bytes(this, 0, len);
        } else if ("US-ASCII".equals(name)) {
            return CharsetUtils.toAsciiBytes(this, 0, len);
        } else if ("UTF-16BE".equals(name)) {
            return CharsetUtils.toBigEndianUtf16Bytes(this, 0, len);
        }

        ByteBuffer buffer = charset.encode(this);
        byte[] bytes = new byte[buffer.limit()];
        buffer.get(bytes);
        return bytes;
        // END Android-changed: Skip StringCoding optimization that needs access to java chars.
    }

    public byte[] getBytes() {
        // BEGIN Android-changed: Skip StringCoding optimization that needs access to java chars.
        /*
        return StringCoding.encode(coder(), value);
         */
        return getBytes(Charset.defaultCharset());
        // END Android-changed: Skip StringCoding optimization that needs access to java chars.
    }

    public boolean equals(Object anObject) {
        if (this == anObject) {
            return true;
        }
        if (anObject instanceof String) {
            // BEGIN Android-changed: Implement in terms of charAt().
            /*
            String aString = (String)anObject;
            if (coder() == aString.coder()) {
                return isLatin1() ? StringLatin1.equals(value, aString.value)
                                  : StringUTF16.equals(value, aString.value);
            }
             */
            String anotherString = (String)anObject;
            int n = length();
            if (n == anotherString.length()) {
                int i = 0;
                while (n-- != 0) {
                    if (charAt(i) != anotherString.charAt(i))
                            return false;
                    i++;
                }
                return true;
            }
            // END Android-changed: Implement in terms of charAt().
        }
        return false;
    }

    public boolean contentEquals(StringBuffer sb) {
        return contentEquals((CharSequence)sb);
    }

    private boolean nonSyncContentEquals(AbstractStringBuilder sb) {
        int len = length();
        if (len != sb.length()) {
            return false;
        }
        // BEGIN Android-changed: Implement in terms of charAt().
        /*
        byte v1[] = value;
        byte v2[] = sb.getValue();
        if (coder() == sb.getCoder()) {
            int n = v1.length;
            for (int i = 0; i < n; i++) {
                if (v1[i] != v2[i]) {
                    return false;
                }
            }
        } else {
            if (!isLatin1()) {  // utf16 str and latin1 abs can never be "equal"
                return false;
            }
            return StringUTF16.contentEquals(v1, v2, len);
        }
         */
        for (int i = 0; i < len; i++) {
            if (charAt(i) != sb.charAt(i)) {
                return false;
            }
        }
        // END Android-changed: Implement in terms of charAt().
        return true;
    }

    public boolean contentEquals(CharSequence cs) {
        // Argument is a StringBuffer, StringBuilder
        if (cs instanceof AbstractStringBuilder) {
            if (cs instanceof StringBuffer) {
                synchronized(cs) {
                   return nonSyncContentEquals((AbstractStringBuilder)cs);
                }
            } else {
                return nonSyncContentEquals((AbstractStringBuilder)cs);
            }
        }
        // Argument is a String
        if (cs instanceof String) {
            return equals(cs);
        }
        // Argument is a generic CharSequence
        int n = cs.length();
        if (n != length()) {
            return false;
        }
        // BEGIN Android-changed: Implement in terms of charAt().
        /*
        byte[] val = this.value;
        if (isLatin1()) {
            for (int i = 0; i < n; i++) {
                if ((val[i] & 0xff) != cs.charAt(i)) {
                    return false;
                }
            }
        } else {
            if (!StringUTF16.contentEquals(val, cs, n)) {
         */
        for (int i = 0; i < n; i++) {
            if (charAt(i) != cs.charAt(i)) {
        // END Android-changed: Implement in terms of charAt().
                return false;
            }
        }
        return true;
    }

    public boolean equalsIgnoreCase(String anotherString) {
        // Android-added: Cache length() result so it's called once.
        final int len = length();
        return (this == anotherString) ? true
                : (anotherString != null)
                && (anotherString.length() == len)
                && regionMatches(true, 0, anotherString, 0, len);
    }

    // BEGIN Android-changed: Replace with implementation in runtime to access chars (see above).
    /*
    public int compareTo(String anotherString) {
        byte v1[] = value;
        byte v2[] = anotherString.value;
        if (coder() == anotherString.coder()) {
            return isLatin1() ? StringLatin1.compareTo(v1, v2)
                              : StringUTF16.compareTo(v1, v2);
        }
        return isLatin1() ? StringLatin1.compareToUTF16(v1, v2)
                          : StringUTF16.compareToLatin1(v1, v2);
     }
    */
    @FastNative
    public native int compareTo(String anotherString);
    // END Android-changed: Replace with implementation in runtime to access chars (see above).

    public static final Comparator<String> CASE_INSENSITIVE_ORDER
                                         = new CaseInsensitiveComparator();
    private static class CaseInsensitiveComparator
            implements Comparator<String>, java.io.Serializable {
        // use serialVersionUID from JDK 1.2.2 for interoperability
        private static final long serialVersionUID = 8575799808933029326L;

        public int compare(String s1, String s2) {
            // BEGIN Android-changed: Implement in terms of charAt().
            /*
            byte v1[] = s1.value;
            byte v2[] = s2.value;
            if (s1.coder() == s2.coder()) {
                return s1.isLatin1() ? StringLatin1.compareToCI(v1, v2)
                                     : StringUTF16.compareToCI(v1, v2);
            }
            return s1.isLatin1() ? StringLatin1.compareToCI_UTF16(v1, v2)
                                 : StringUTF16.compareToCI_Latin1(v1, v2);
             */
            int n1 = s1.length();
            int n2 = s2.length();
            int min = Math.min(n1, n2);
            for (int i = 0; i < min; i++) {
                char c1 = s1.charAt(i);
                char c2 = s2.charAt(i);
                if (c1 != c2) {
                    c1 = Character.toUpperCase(c1);
                    c2 = Character.toUpperCase(c2);
                    if (c1 != c2) {
                        c1 = Character.toLowerCase(c1);
                        c2 = Character.toLowerCase(c2);
                        if (c1 != c2) {
                            // No overflow because of numeric promotion
                            return c1 - c2;
                        }
                    }
                }
            }
            return n1 - n2;
            // END Android-changed: Implement in terms of charAt().
        }

        /** Replaces the de-serialized object. */
        private Object readResolve() { return CASE_INSENSITIVE_ORDER; }
    }

    public int compareToIgnoreCase(String str) {
        return CASE_INSENSITIVE_ORDER.compare(this, str);
    }

    public boolean regionMatches(int toffset, String other, int ooffset, int len) {
        // BEGIN Android-removed: Implement in terms of charAt().
        /*
        byte tv[] = value;
        byte ov[] = other.value;
         */
        // Note: toffset, ooffset, or len might be near -1>>>1.
        if ((ooffset < 0) || (toffset < 0) ||
             (toffset > (long)length() - len) ||
             (ooffset > (long)other.length() - len)) {
            return false;
        }
        // BEGIN Android-removed: Implement in terms of charAt().
        /*
        if (coder() == other.coder()) {
            if (!isLatin1() && (len > 0)) {
                toffset = toffset << 1;
                ooffset = ooffset << 1;
                len = len << 1;
            }
            while (len-- > 0) {
                if (tv[toffset++] != ov[ooffset++]) {
                    return false;
                }
            }
        } else {
            if (coder() == LATIN1) {
                while (len-- > 0) {
                    if (StringLatin1.getChar(tv, toffset++) !=
                        StringUTF16.getChar(ov, ooffset++)) {
                        return false;
                    }
                }
            } else {
                while (len-- > 0) {
                    if (StringUTF16.getChar(tv, toffset++) !=
                        StringLatin1.getChar(ov, ooffset++)) {
                        return false;
                    }
                }
         */
        while (len-- > 0) {
            if (charAt(toffset++) != other.charAt(ooffset++)) {
                return false;
        // END Android-removed: Implement in terms of charAt().
            }
        }
        return true;
    }

    public boolean regionMatches(boolean ignoreCase, int toffset,
            String other, int ooffset, int len) {
        if (!ignoreCase) {
            return regionMatches(toffset, other, ooffset, len);
        }
        // Note: toffset, ooffset, or len might be near -1>>>1.
        if ((ooffset < 0) || (toffset < 0)
                || (toffset > (long)length() - len)
                || (ooffset > (long)other.length() - len)) {
            return false;
        }
        // BEGIN Android-changed: Implement in terms of charAt().
        /*
        byte tv[] = value;
        byte ov[] = other.value;
        if (coder() == other.coder()) {
            return isLatin1()
              ? StringLatin1.regionMatchesCI(tv, toffset, ov, ooffset, len)
              : StringUTF16.regionMatchesCI(tv, toffset, ov, ooffset, len);
        }
        return isLatin1()
              ? StringLatin1.regionMatchesCI_UTF16(tv, toffset, ov, ooffset, len)
              : StringUTF16.regionMatchesCI_Latin1(tv, toffset, ov, ooffset, len);
         */
        while (len-- > 0) {
            char c1 = charAt(toffset++);
            char c2 = other.charAt(ooffset++);
            if (c1 == c2) {
                continue;
            }
            if (ignoreCase) {
                // If characters don't match but case may be ignored,
                // try converting both characters to uppercase.
                // If the results match, then the comparison scan should
                // continue.
                char u1 = Character.toUpperCase(c1);
                char u2 = Character.toUpperCase(c2);
                if (u1 == u2) {
                    continue;
                }
                // Unfortunately, conversion to uppercase does not work properly
                // for the Georgian alphabet, which has strange rules about case
                // conversion.  So we need to make one last check before
                // exiting.
                if (Character.toLowerCase(u1) == Character.toLowerCase(u2)) {
                    continue;
                }
            }
            return false;
        }
        return true;
        // END Android-changed: Implement in terms of charAt().
    }

    public boolean startsWith(String prefix, int toffset) {
        // Android-added: Cache length() result so it's called once.
        int pc = prefix.length();
        // Note: toffset might be near -1>>>1.
        if (toffset < 0 || toffset > length() - pc) {
            return false;
        }
        // BEGIN Android-changed: Implement in terms of charAt().
        /*
        byte ta[] = value;
        byte pa[] = prefix.value;
        int po = 0;
        int pc = pa.length;
        if (coder() == prefix.coder()) {
            int to = isLatin1() ? toffset : toffset << 1;
            while (po < pc) {
                if (ta[to++] != pa[po++]) {
                    return false;
                }
            }
        } else {
            if (isLatin1()) {  // && pcoder == UTF16
                return false;
            }
            // coder == UTF16 && pcoder == LATIN1)
            while (po < pc) {
                if (StringUTF16.getChar(ta, toffset++) != (pa[po++] & 0xff)) {
                    return false;
               }
            }
         */
        int po = 0;
        while (--pc >= 0) {
            if (charAt(toffset++) != prefix.charAt(po++)) {
                return false;
            }
        // END Android-changed: Implement in terms of charAt().
        }
        return true;
    }

    public boolean startsWith(String prefix) {
        return startsWith(prefix, 0);
    }

    public boolean endsWith(String suffix) {
        return startsWith(suffix, length() - suffix.length());
    }

    public int hashCode() {
        int h = hash;
        // BEGIN Android-changed: Implement in terms of charAt().
        /*
        if (h == 0 && value.length > 0) {
            hash = h = isLatin1() ? StringLatin1.hashCode(value)
                                  : StringUTF16.hashCode(value);
         */
        final int len = length();
        if (h == 0 && len > 0) {
            for (int i = 0; i < len; i++) {
                h = 31 * h + charAt(i);
            }
            hash = h;
        // END Android-changed: Implement in terms of charAt().
        }
        return h;
    }

    public int indexOf(int ch) {
        return indexOf(ch, 0);
    }

    public int indexOf(int ch, int fromIndex) {
    // BEGIN Android-changed: Implement in terms of charAt().
        /*
        return isLatin1() ? StringLatin1.indexOf(value, ch, fromIndex)
                          : StringUTF16.indexOf(value, ch, fromIndex);
         */
        final int max = length();
        if (fromIndex < 0) {
            fromIndex = 0;
        } else if (fromIndex >= max) {
            // Note: fromIndex might be near -1>>>1.
            return -1;
        }

        if (ch < Character.MIN_SUPPLEMENTARY_CODE_POINT) {
            // handle most cases here (ch is a BMP code point or a
            // negative value (invalid code point))
            for (int i = fromIndex; i < max; i++) {
                if (charAt(i) == ch) {
                    return i;
                }
            }
            return -1;
        } else {
            return indexOfSupplementary(ch, fromIndex);
        }
    }

    private int indexOfSupplementary(int ch, int fromIndex) {
        if (Character.isValidCodePoint(ch)) {
            final char hi = Character.highSurrogate(ch);
            final char lo = Character.lowSurrogate(ch);
            final int max = length() - 1;
            for (int i = fromIndex; i < max; i++) {
                if (charAt(i) == hi && charAt(i + 1) == lo) {
                    return i;
                }
            }
        }
        return -1;
    // END Android-changed: Implement in terms of charAt().
    }

    public int lastIndexOf(int ch) {
        return lastIndexOf(ch, length() - 1);
    }

    public int lastIndexOf(int ch, int fromIndex) {
    // BEGIN Android-changed: Implement in terms of charAt().
        /*
        return isLatin1() ? StringLatin1.lastIndexOf(value, ch, fromIndex)
                          : StringUTF16.lastIndexOf(value, ch, fromIndex);
         */
        if (ch < Character.MIN_SUPPLEMENTARY_CODE_POINT) {
            // handle most cases here (ch is a BMP code point or a
            // negative value (invalid code point))
            int i = Math.min(fromIndex, length() - 1);
            for (; i >= 0; i--) {
                if (charAt(i) == ch) {
                    return i;
                }
            }
            return -1;
        } else {
            return lastIndexOfSupplementary(ch, fromIndex);
        }
    }

    private int lastIndexOfSupplementary(int ch, int fromIndex) {
        if (Character.isValidCodePoint(ch)) {
            char hi = Character.highSurrogate(ch);
            char lo = Character.lowSurrogate(ch);
            int i = Math.min(fromIndex, length() - 2);
            for (; i >= 0; i--) {
                if (charAt(i) == hi && charAt(i + 1) == lo) {
                    return i;
                }
            }
        }
        return -1;
    // END Android-changed: Implement in terms of charAt().
    }

    @NeverInline
    public int indexOf(String str) {
        // BEGIN Android-changed: Implement with indexOf() method that takes String parameters.
        /*
        if (coder() == str.coder()) {
            return isLatin1() ? StringLatin1.indexOf(value, str.value)
                              : StringUTF16.indexOf(value, str.value);
        }
        if (coder() == LATIN1) {  // str.coder == UTF16
            return -1;
        }
        return StringUTF16.indexOfLatin1(value, str.value);
         */
        return indexOf(str, 0);
        // END Android-changed: Implement with indexOf() method that takes String parameters.
    }

    @NeverInline
    public int indexOf(String str, int fromIndex) {
        // BEGIN Android-changed: Implement with indexOf() method that takes String parameters.
        /*
        return indexOf(value, coder(), length(), str, fromIndex);
         */
        return indexOf(this, str, fromIndex);
        // END Android-changed: Implement with indexOf() method that takes String parameters.
    }

    // BEGIN Android-added: Private static indexOf method that takes String parameters.
    // The use of length(), charAt(), etc. makes it more efficient for compressed strings.
    private static int indexOf(String source, String target, int fromIndex) {
        final int sourceLength = source.length();
        final int targetLength = target.length();
        if (fromIndex >= sourceLength) {
            return (targetLength == 0 ? sourceLength : -1);
        }
        if (fromIndex < 0) {
            fromIndex = 0;
        }
        if (targetLength == 0) {
            return fromIndex;
        }

        char first = target.charAt(0);
        int max = (sourceLength - targetLength);

        for (int i = fromIndex; i <= max; i++) {
            /* Look for first character. */
            if (source.charAt(i)!= first) {
                while (++i <= max && source.charAt(i) != first);
            }

            /* Found first character, now look at the rest of v2 */
            if (i <= max) {
                int j = i + 1;
                int end = j + targetLength - 1;
                for (int k = 1; j < end && source.charAt(j)
                         == target.charAt(k); j++, k++);

                if (j == end) {
                    /* Found whole string. */
                    return i;
                }
            }
        }
        return -1;
    }
    // END Android-added: Private static indexOf method that takes String parameters.

    static int indexOf(byte[] src, byte srcCoder, int srcCount,
        String tgtStr, int fromIndex) {
        // byte[] tgt    = tgtStr.value;
        byte tgtCoder = tgtStr.coder();
        int tgtCount  = tgtStr.length();

        if (fromIndex >= srcCount) {
            return (tgtCount == 0 ? srcCount : -1);
        }
        if (fromIndex < 0) {
            fromIndex = 0;
        }
        if (tgtCount == 0) {
            return fromIndex;
        }
        if (tgtCount > srcCount) {
            return -1;
        }
        if (srcCoder == tgtCoder) {
            return srcCoder == CODER_LATIN1
                // Android-changed: libcore doesn't store String as Latin1 or UTF16 byte[] field.
                // ? StringLatin1.indexOf(src, srcCount, tgt, tgtCount, fromIndex)
                // : StringUTF16.indexOf(src, srcCount, tgt, tgtCount, fromIndex);
                ? StringLatin1.indexOf(src, srcCount, tgtStr, tgtCount, fromIndex)
                : StringUTF16.indexOf(src, srcCount, tgtStr, tgtCount, fromIndex);
        }
        if (srcCoder == CODER_LATIN1) {    //  && tgtCoder == UTF16
            // Android-changed: Latin1 AbstractStringBuilder has a larger range than Latin1 String.
            // return -1;
            return StringLatin1.indexOfUTF16(src, srcCount, tgtStr, tgtCount, fromIndex);
        }
        // srcCoder == UTF16 && tgtCoder == LATIN1) {
        // return StringUTF16.indexOfLatin1(src, srcCount, tgt, tgtCount, fromIndex);
        return StringUTF16.indexOfLatin1(src, srcCount, tgtStr, tgtCount, fromIndex);
    }

    public int lastIndexOf(String str) {
        return lastIndexOf(str, length());
    }

    public int lastIndexOf(String str, int fromIndex) {
        // BEGIN Android-changed: Implement with static lastIndexOf() that takes String parameters.
        /*
        return lastIndexOf(value, coder(), length(), str, fromIndex);
         */
        return lastIndexOf(this, str, fromIndex);
        // END Android-changed: Implement with static lastIndexOf() that takes String parameters.
    }

    // BEGIN Android-added: Private static lastIndexOf method that takes String parameters.
    // The use of length(), charAt(), etc. makes it more efficient for compressed strings.
    private static int lastIndexOf(String source, String target, int fromIndex) {
        /*
         * Check arguments; return immediately where possible. For
         * consistency, don't check for null str.
         */
        final int sourceLength = source.length();
        final int targetLength = target.length();
        int rightIndex = sourceLength - targetLength;
        if (fromIndex < 0) {
            return -1;
        }
        if (fromIndex > rightIndex) {
            fromIndex = rightIndex;
        }
        /* Empty string always matches. */
        if (targetLength == 0) {
            return fromIndex;
        }

        int strLastIndex = targetLength - 1;
        char strLastChar = target.charAt(strLastIndex);
        int min = targetLength - 1;
        int i = min + fromIndex;

        startSearchForLastChar:
        while (true) {
            while (i >= min && source.charAt(i) != strLastChar) {
                i--;
            }
            if (i < min) {
                return -1;
            }
            int j = i - 1;
            int start = j - (targetLength - 1);
            int k = strLastIndex - 1;

            while (j > start) {
                if (source.charAt(j--) != target.charAt(k--)) {
                    i--;
                    continue startSearchForLastChar;
                }
            }
            return start + 1;
        }
    }
    // END Android-added: Private static lastIndexOf method that takes String parameters.

    static int lastIndexOf(byte[] src, byte srcCoder, int srcCount,
        String tgtStr, int fromIndex) {
        // byte[] tgt = tgtStr.value;
        byte tgtCoder = tgtStr.coder();
        int tgtCount = tgtStr.length();
        /*
         * Check arguments; return immediately where possible. For
         * consistency, don't check for null str.
         */
        int rightIndex = srcCount - tgtCount;
        if (fromIndex > rightIndex) {
            fromIndex = rightIndex;
        }
        if (fromIndex < 0) {
            return -1;
        }
        /* Empty string always matches. */
        if (tgtCount == 0) {
            return fromIndex;
        }
        if (srcCoder == tgtCoder) {
            return srcCoder == CODER_LATIN1
                // Android-changed: libcore doesn't store String as Latin1 or UTF16 byte[] field.
                // ? StringLatin1.lastIndexOf(src, srcCount, tgt, tgtCount, fromIndex)
                // : StringUTF16.lastIndexOf(src, srcCount, tgt, tgtCount, fromIndex);
                ? StringLatin1.lastIndexOf(src, srcCount, tgtStr, tgtCount, fromIndex)
                : StringUTF16.lastIndexOf(src, srcCount, tgtStr, tgtCount, fromIndex);
        }
        if (srcCoder == CODER_LATIN1) {    // && tgtCoder == UTF16
            // Android-changed: Latin1 AbstractStringBuilder has a larger range than Latin1 String.
            // return -1;
            return StringLatin1.lastIndexOfUTF16(src, srcCount, tgtStr, tgtCount, fromIndex);
        }
        // srcCoder == UTF16 && tgtCoder == LATIN1
        // return StringUTF16.lastIndexOfLatin1(src, srcCount, tgt, tgtCount, fromIndex);
        return StringUTF16.lastIndexOfLatin1(src, srcCount, tgtStr, tgtCount, fromIndex);
    }

    static int lastIndexOf(char[] source, int sourceOffset, int sourceCount,
            char[] target, int targetOffset, int targetCount,
            int fromIndex) {
        /*
         * Check arguments; return immediately where possible. For
         * consistency, don't check for null str.
         */
        int rightIndex = sourceCount - targetCount;
        if (fromIndex < 0) {
            return -1;
        }
        if (fromIndex > rightIndex) {
            fromIndex = rightIndex;
        }
        /* Empty string always matches. */
        if (targetCount == 0) {
            return fromIndex;
        }

        int strLastIndex = targetOffset + targetCount - 1;
        char strLastChar = target[strLastIndex];
        int min = sourceOffset + targetCount - 1;
        int i = min + fromIndex;

    startSearchForLastChar:
        while (true) {
            while (i >= min && source[i] != strLastChar) {
                i--;
            }
            if (i < min) {
                return -1;
            }
            int j = i - 1;
            int start = j - (targetCount - 1);
            int k = strLastIndex - 1;

            while (j > start) {
                if (source[j--] != target[k--]) {
                    i--;
                    continue startSearchForLastChar;
                }
            }
            return start - sourceOffset + 1;
        }
    }

    public String substring(int beginIndex) {
        if (beginIndex < 0) {
            throw new StringIndexOutOfBoundsException(this, beginIndex);
        }
        int subLen = length() - beginIndex;
        if (subLen < 0) {
            throw new StringIndexOutOfBoundsException(this, beginIndex);
        }
        if (beginIndex == 0) {
            return this;
        }
        // BEGIN Android-changed: Use native fastSubstring instead of String constructor.
        /*
        return isLatin1() ? StringLatin1.newString(value, beginIndex, subLen)
                          : StringUTF16.newString(value, beginIndex, subLen);
         */
        return fastSubstring(beginIndex, subLen);
        // END Android-changed: Use native fastSubstring instead of String constructor.
    }

    public String substring(int beginIndex, int endIndex) {
        int length = length();
        checkBoundsBeginEnd(beginIndex, endIndex, length);
        int subLen = endIndex - beginIndex;
        if (beginIndex == 0 && endIndex == length) {
            return this;
        }

        // BEGIN Android-changed: Use native fastSubstring instead of String constructor.
        /*
        return isLatin1() ? StringLatin1.newString(value, beginIndex, subLen)
                          : StringUTF16.newString(value, beginIndex, subLen);
         */
        return fastSubstring(beginIndex, subLen);
        // END Android-changed: Use native fastSubstring instead of String constructor.
    }

    // BEGIN Android-added: Native method to access char storage managed by runtime.
    @FastNative
    private native String fastSubstring(int start, int length);
    // END Android-added: Native method to access char storage managed by runtime.

    public CharSequence subSequence(int beginIndex, int endIndex) {
        return this.substring(beginIndex, endIndex);
    }

    // BEGIN Android-changed: Replace with implementation in runtime to access chars (see above).
    /*
    public String concat(String str) {
        if (str.isEmpty()) {
            return this;
        }
        if (coder() == str.coder()) {
            byte[] val = this.value;
            byte[] oval = str.value;
            int len = val.length + oval.length;
            byte[] buf = Arrays.copyOf(val, len);
            System.arraycopy(oval, 0, buf, val.length, oval.length);
            return new String(buf, coder);
        }
        int len = length();
        int olen = str.length();
        byte[] buf = StringUTF16.newBytesFor(len + olen);
        getBytes(buf, 0, UTF16);
        str.getBytes(buf, len, UTF16);
        return new String(buf, UTF16);
    }
    */
    @FastNative
    public native String concat(String str);
    // END Android-changed: Replace with implementation in runtime to access chars (see above).

    public String replace(char oldChar, char newChar) {
        // BEGIN Android-changed: Replace with implementation using native doReplace method.
        if (oldChar != newChar) {
            /*
            String ret = isLatin1() ? StringLatin1.replace(value, oldChar, newChar)
                                    : StringUTF16.replace(value, oldChar, newChar);
            if (ret != null) {
                return ret;
            }
            */
            final int len = length();
            for (int i = 0; i < len; ++i) {
                if (charAt(i) == oldChar) {
                    return doReplace(oldChar, newChar);
                }
            }
        }
        // END Android-changed: Replace with implementation using native doReplace method.
        return this;
    }

    // BEGIN Android-added: Native method to access char storage managed by runtime.
    // Implementation of replace(char oldChar, char newChar) called when we found a match.
    @FastNative
    private native String doReplace(char oldChar, char newChar);
    // END Android-added: Native method to access char storage managed by runtime.

    public boolean matches(String regex) {
        return Pattern.matches(regex, this);
    }

    public boolean contains(CharSequence s) {
        return indexOf(s.toString()) >= 0;
    }

    public String replaceFirst(String regex, String replacement) {
        return Pattern.compile(regex).matcher(this).replaceFirst(replacement);
    }

    public String replaceAll(String regex, String replacement) {
        return Pattern.compile(regex).matcher(this).replaceAll(replacement);
    }

    public String replace(CharSequence target, CharSequence replacement) {
        // BEGIN Android-added: Additional null check for parameters.
        Objects.requireNonNull(target);
        Objects.requireNonNull(replacement);
        // END Android-added: Additional null check for parameters.

        String tgtStr = target.toString();
        String replStr = replacement.toString();
        int j = indexOf(tgtStr);
        if (j < 0) {
            return this;
        }
        int tgtLen = tgtStr.length();
        int tgtLen1 = Math.max(tgtLen, 1);
        int thisLen = length();

        int newLenHint = thisLen - tgtLen + replStr.length();
        if (newLenHint < 0) {
            throw new OutOfMemoryError();
        }
        StringBuilder sb = new StringBuilder(newLenHint);
        int i = 0;
        do {
            sb.append(this, i, j).append(replStr);
            i = j + tgtLen;
        } while (j < thisLen && (j = indexOf(tgtStr, j + tgtLen1)) > 0);
        return sb.append(this, i, thisLen).toString();
    }

    public String[] split(String regex, int limit) {
        // BEGIN Android-changed: Replace custom fast-path with use of new Pattern.fastSplit method.
        // Try fast splitting without allocating Pattern object
        /*
        /* fastpath if the regex is a
         (1)one-char String and this character is not one of the
            RegEx's meta characters ".$|()[{^?*+\\", or
         (2)two-char String and the first char is the backslash and
            the second is not the ascii digit or ascii letter.
         *
        char ch = 0;
        if (((regex.length() == 1 &&
             ".$|()[{^?*+\\".indexOf(ch = regex.charAt(0)) == -1) ||
             (regex.length() == 2 &&
              regex.charAt(0) == '\\' &&
              (((ch = regex.charAt(1))-'0')|('9'-ch)) < 0 &&
              ((ch-'a')|('z'-ch)) < 0 &&
              ((ch-'A')|('Z'-ch)) < 0)) &&
            (ch < Character.MIN_HIGH_SURROGATE ||
             ch > Character.MAX_LOW_SURROGATE))
        {
            int off = 0;
            int next = 0;
            boolean limited = limit > 0;
            ArrayList<String> list = new ArrayList<>();
            while ((next = indexOf(ch, off)) != -1) {
                if (!limited || list.size() < limit - 1) {
                    list.add(substring(off, next));
                    off = next + 1;
                } else {    // last one
                    //assert (list.size() == limit - 1);
                    int last = length();
                    list.add(substring(off, last));
                    off = last;
                    break;
                }
            }
            // If no match was found, return this
            if (off == 0)
                return new String[]{this};

            // Add remaining segment
            if (!limited || list.size() < limit)
                list.add(substring(off, length()));

            // Construct result
            int resultSize = list.size();
            if (limit == 0) {
                while (resultSize > 0 && list.get(resultSize - 1).isEmpty()) {
                    resultSize--;
                }
            }
            String[] result = new String[resultSize];
            return list.subList(0, resultSize).toArray(result);
        }
        */
        String[] fast = Pattern.fastSplit(regex, this, limit);
        if (fast != null) {
            return fast;
        }
        // END Android-changed: Replace custom fast-path with use of new Pattern.fastSplit method.
        return Pattern.compile(regex).split(this, limit);
    }

    public String[] split(String regex) {
        return split(regex, 0);
    }

    public static String join(CharSequence delimiter, CharSequence... elements) {
        Objects.requireNonNull(delimiter);
        Objects.requireNonNull(elements);
        // Number of elements not likely worth Arrays.stream overhead.
        StringJoiner joiner = new StringJoiner(delimiter);
        for (CharSequence cs: elements) {
            joiner.add(cs);
        }
        return joiner.toString();
    }

    public static String join(CharSequence delimiter,
            Iterable<? extends CharSequence> elements) {
        Objects.requireNonNull(delimiter);
        Objects.requireNonNull(elements);
        StringJoiner joiner = new StringJoiner(delimiter);
        for (CharSequence cs: elements) {
            joiner.add(cs);
        }
        return joiner.toString();
    }

    public String toLowerCase(Locale locale) {
        // BEGIN Android-changed: Replace custom code with call to new CaseMapper class.
        /*
        return isLatin1() ? StringLatin1.toLowerCase(this, value, locale)
                          : StringUTF16.toLowerCase(this, value, locale);
        */
        return CaseMapper.toLowerCase(locale, this);
        // END Android-changed: Replace custom code with call to new CaseMapper class.
    }

    public String toLowerCase() {
        return toLowerCase(Locale.getDefault());
    }

    public String toUpperCase(Locale locale) {
        // BEGIN Android-changed: Replace custom code with call to new CaseMapper class.
        /*
        return isLatin1() ? StringLatin1.toUpperCase(this, value, locale)
                          : StringUTF16.toUpperCase(this, value, locale);
        */
        return CaseMapper.toUpperCase(locale, this, length());
        // END Android-changed: Replace custom code with call to new CaseMapper class.
    }

    public String toUpperCase() {
        return toUpperCase(Locale.getDefault());
    }

    public String trim() {
        // BEGIN Android-changed: Implement in terms of charAt().
        /*
        String ret = isLatin1() ? StringLatin1.trim(value)
                                : StringUTF16.trim(value);
        return ret == null ? this : ret;
         */
        int len = length();
        int st = 0;

        while ((st < len) && (charAt(st) <= ' ')) {
            st++;
        }
        while ((st < len) && (charAt(len - 1) <= ' ')) {
            len--;
        }
        return ((st > 0) || (len < length())) ? substring(st, len) : this;
        // END Android-changed: Implement in terms of charAt().
    }

    public String strip() {
        // BEGIN Android-changed: Delegate to StringUTF16.
        /*
        String ret = isLatin1() ? StringLatin1.strip(value)
                                : StringUTF16.strip(value);
         */
        String ret = StringUTF16.strip(this);
        // END Android-changed: Delegate to StringUTF16.
        return ret == null ? this : ret;
    }

    public String stripLeading() {
        // BEGIN Android-changed: Delegate to StringUTF16.
        /*
        String ret = isLatin1() ? StringLatin1.stripLeading(value)
                                : StringUTF16.stripLeading(value);
         */
        String ret = StringUTF16.stripLeading(this);
        // END Android-changed: Delegate to StringUTF16.
        return ret == null ? this : ret;
    }

    public String stripTrailing() {
        // BEGIN Android-changed: Delegate to StringUTF16.
        /*
        String ret = isLatin1() ? StringLatin1.stripTrailing(value)
                                : StringUTF16.stripTrailing(value);
         */
        String ret = StringUTF16.stripTrailing(this);
        // END Android-changed: Delegate to StringUTF16.
        return ret == null ? this : ret;
    }

    public boolean isBlank() {
        return indexOfNonWhitespace() == length();
    }

    public Stream<String> lines() {
        // BEGIN Android-removed: Delegate to StringUTF16.
        /*
        return isLatin1() ? StringLatin1.lines(value)
                          : StringUTF16.lines(value);
         */
        return StringUTF16.lines(this);
        // END Android-removed: Delegate to StringUTF16.
    }

    public String indent(int n) {
        if (isEmpty()) {
            return "";
        }
        Stream<String> stream = lines();
        if (n > 0) {
            final String spaces = " ".repeat(n);
            stream = stream.map(s -> spaces + s);
        } else if (n == Integer.MIN_VALUE) {
            stream = stream.map(s -> s.stripLeading());
        } else if (n < 0) {
            stream = stream.map(s -> s.substring(Math.min(-n, s.indexOfNonWhitespace())));
        }
        return stream.collect(Collectors.joining("\n", "", "\n"));
    }

    private int indexOfNonWhitespace() {
        // BEGIN Android-removed: Delegate to StringUTF16.
        /*
        return isLatin1() ? StringLatin1.indexOfNonWhitespace(value)
                          : StringUTF16.indexOfNonWhitespace(value);
         */
        return StringUTF16.indexOfNonWhitespace(this);
        // END Android-removed: Delegate to StringUTF16.
    }

    private int lastIndexOfNonWhitespace() {
        // BEGIN Android-changed: Delegate to StringUTF16.
        /*
        return isLatin1() ? StringLatin1.lastIndexOfNonWhitespace(value)
                          : StringUTF16.lastIndexOfNonWhitespace(value);
        */
        return StringUTF16.lastIndexOfNonWhitespace(this);
        // END Android-changed: Delegate to StringUTF16.
    }

    public String stripIndent() {
        int length = length();
        if (length == 0) {
            return "";
        }
        char lastChar = charAt(length - 1);
        boolean optOut = lastChar == '\n' || lastChar == '\r';
        List<String> lines = lines().toList();
        final int outdent = optOut ? 0 : outdent(lines);
        return lines.stream()
            .map(line -> {
                int firstNonWhitespace = line.indexOfNonWhitespace();
                int lastNonWhitespace = line.lastIndexOfNonWhitespace();
                int incidentalWhitespace = Math.min(outdent, firstNonWhitespace);
                return firstNonWhitespace > lastNonWhitespace
                    ? "" : line.substring(incidentalWhitespace, lastNonWhitespace);
            })
            .collect(Collectors.joining("\n", "", optOut ? "\n" : ""));
    }

    private static int outdent(List<String> lines) {
        // Note: outdent is guaranteed to be zero or positive number.
        // If there isn't a non-blank line then the last must be blank
        int outdent = Integer.MAX_VALUE;
        for (String line : lines) {
            int leadingWhitespace = line.indexOfNonWhitespace();
            if (leadingWhitespace != line.length()) {
                outdent = Integer.min(outdent, leadingWhitespace);
            }
        }
        String lastLine = lines.get(lines.size() - 1);
        if (lastLine.isBlank()) {
            outdent = Integer.min(outdent, lastLine.length());
        }
        return outdent;
    }

    public String translateEscapes() {
        if (isEmpty()) {
            return "";
        }
        char[] chars = toCharArray();
        int length = chars.length;
        int from = 0;
        int to = 0;
        while (from < length) {
            char ch = chars[from++];
            if (ch == '\\') {
                ch = from < length ? chars[from++] : '\0';
                switch (ch) {
                case 'b':
                    ch = '\b';
                    break;
                case 'f':
                    ch = '\f';
                    break;
                case 'n':
                    ch = '\n';
                    break;
                case 'r':
                    ch = '\r';
                    break;
                case 's':
                    ch = ' ';
                    break;
                case 't':
                    ch = '\t';
                    break;
                case '\'':
                case '\"':
                case '\\':
                    // as is
                    break;
                case '0': case '1': case '2': case '3':
                case '4': case '5': case '6': case '7':
                    int limit = Integer.min(from + (ch <= '3' ? 2 : 1), length);
                    int code = ch - '0';
                    while (from < limit) {
                        ch = chars[from];
                        if (ch < '0' || '7' < ch) {
                            break;
                        }
                        from++;
                        code = (code << 3) | (ch - '0');
                    }
                    ch = (char)code;
                    break;
                case '\n':
                    continue;
                case '\r':
                    if (from < length && chars[from] == '\n') {
                        from++;
                    }
                    continue;
                default: {
                    String msg = String.format(
                        "Invalid escape sequence: \\%c \\\\u%04X",
                        ch, (int)ch);
                    throw new IllegalArgumentException(msg);
                }
                }
            }

            chars[to++] = ch;
        }

        return new String(chars, 0, to);
    }

    public <R> R transform(Function<? super String, ? extends R> f) {
        return f.apply(this);
    }

    public String toString() {
        return this;
    }

    @Override
    public IntStream chars() {
        return StreamSupport.intStream(
            // BEGIN Android-removed: Delegate to StringUTF16.
            /*
            isLatin1() ? new StringLatin1.CharsSpliterator(value, Spliterator.IMMUTABLE)
                       : new StringUTF16.CharsSpliterator(value, Spliterator.IMMUTABLE),
             */
            new StringUTF16.CharsSpliteratorForString(this, Spliterator.IMMUTABLE),
            // END Android-removed: Delegate to StringUTF16.
            false);
    }


    @Override
    public IntStream codePoints() {
        return StreamSupport.intStream(
            // BEGIN Android-removed: Delegate to StringUTF16.
            /*
            isLatin1() ? new StringLatin1.CharsSpliterator(value, Spliterator.IMMUTABLE)
                       : new StringUTF16.CodePointsSpliterator(value, Spliterator.IMMUTABLE),
             */
            new StringUTF16.CodePointsSpliteratorForString(this, Spliterator.IMMUTABLE),
            // END Android-removed: Delegate to StringUTF16.
            false);
    }

    @FastNative
    public native char[] toCharArray();
    // END Android-changed: Replace with implementation in runtime to access chars (see above).


    public static String format(String format, Object... args) {
        return new Formatter().format(format, args).toString();
    }

    public static String format(Locale l, String format, Object... args) {
        return new Formatter(l).format(format, args).toString();
    }

    public String formatted(Object... args) {
        return new Formatter().format(this, args).toString();
    }

    public static String valueOf(Object obj) {
        return (obj == null) ? "null" : obj.toString();
    }

    public static String valueOf(char data[]) {
        return new String(data);
    }

    public static String valueOf(char data[], int offset, int count) {
        return new String(data, offset, count);
    }

    public static String copyValueOf(char data[], int offset, int count) {
        return new String(data, offset, count);
    }

    public static String copyValueOf(char data[]) {
        return new String(data);
    }

     */
    public static String valueOf(boolean b) {
        return b ? "true" : "false";
    }

    public static String valueOf(char c) {
        // BEGIN Android-changed: Replace constructor call with call to StringFactory class.
        // There is currently no String(char[], boolean) on Android to call. http://b/79902155
        /*
        if (COMPACT_STRINGS && StringLatin1.canEncode(c)) {
            return new String(StringLatin1.toBytes(c), LATIN1);
        }
        return new String(StringUTF16.toBytes(c), UTF16);
         */
        return StringFactory.newStringFromChars(0, 1, new char[] { c });
        // END Android-changed: Replace constructor call with call to StringFactory class.
    }

    public static String valueOf(int i) {
        return Integer.toString(i);
    }

    public static String valueOf(long l) {
        return Long.toString(l);
    }

    public static String valueOf(float f) {
        return Float.toString(f);
    }

    public static String valueOf(double d) {
        return Double.toString(d);
    }

    // Android-added: Annotate native method as @FastNative.
    @FastNative
    public native String intern();

    public String repeat(int count) {
        if (count < 0) {
            throw new IllegalArgumentException("count is negative: " + count);
        }
        if (count == 1) {
            return this;
        }
        // Android-changed: Replace with implementation in runtime.
        // final int len = value.length;
        final int len = length();
        if (len == 0 || count == 0) {
            return "";
        }
        // BEGIN Android-changed: Replace with implementation in runtime.
        /*
        if (len == 1) {
            final byte[] single = new byte[count];
            Arrays.fill(single, value[0]);
            return new String(single, coder);
        }
        */
        // END Android-changed: Replace with implementation in runtime.
        if (Integer.MAX_VALUE / count < len) {
            throw new OutOfMemoryError("Repeating " + len + " bytes String " + count +
                    " times will produce a String exceeding maximum size.");
        }
        // BEGIN Android-changed: Replace with implementation in runtime.
        /*
        final int limit = len * count;
        final byte[] multiple = new byte[limit];
        System.arraycopy(value, 0, multiple, 0, len);
        int copied = len;
        for (; copied < limit - copied; copied <<= 1) {
            System.arraycopy(multiple, 0, multiple, copied, copied);
        }
        System.arraycopy(multiple, 0, multiple, copied, limit - copied);
        return new String(multiple, coder);
         */
        // END Android-changed: Replace with implementation in runtime.
        return doRepeat(count);
    }

    @FastNative
    private native String doRepeat(int count);

    ////////////////////////////////////////////////////////////////

    void fillBytes(byte dst[], int dstBegin, byte coder) {
        // We do bound check here before the native calls, because the upstream implementation does
        // the bound check in System.arraycopy and StringLatin1.inflate or throws an exception.
        if (coder == CODER_UTF16) {
            int fromIndex = dstBegin << 1;
            checkBoundsOffCount(fromIndex, length() << 1, dst.length);
            fillBytesUTF16(dst, fromIndex);
        } else {
            checkBoundsOffCount(dstBegin, length(), dst.length);
            fillBytesLatin1(dst, dstBegin);
        }
    }
    // END Android-changed: libcore doesn't store String as Latin1 or UTF16 byte[] field.

    // BEGIN Android-added: Implement fillBytes*() method natively.

    @FastNative
    private native void fillBytesLatin1(byte[] dst, int byteIndex);

    @FastNative
    private native void fillBytesUTF16(byte[] dst, int byteIndex);
    // END Android-added: Implement fillBytes*() method natively.

    String(byte[] value, byte coder) {
        // BEGIN Android-changed: Implemented as compiler and runtime intrinsics.
        // this.value = value;
        // this.coder = coder;
        throw new UnsupportedOperationException("Use StringFactory instead.");
        // END Android-changed: Implemented as compiler and runtime intrinsics.
    }

    byte coder() {
        // Android-changed: ART stores the flag in the count field.
        // return COMPACT_STRINGS ? coder : UTF16;
        // We assume that STRING_COMPRESSION_ENABLED is enabled here.
        // The flag has been true for 6+ years.
        return COMPACT_STRINGS ? ((byte) (count & 1)) : CODER_UTF16;
    }

    static void checkIndex(int index, int length) {
        if (index < 0 || index >= length) {
            throw new StringIndexOutOfBoundsException("index " + index +
                                                      ",length " + length);
        }
    }

    static void checkOffset(int offset, int length) {
        if (offset < 0 || offset > length) {
            throw new StringIndexOutOfBoundsException("offset " + offset +
                ",length " + length);
        }
    }

    static void checkBoundsOffCount(int offset, int count, int length) {
        if (offset < 0 || count < 0 || offset > length - count) {
            throw new StringIndexOutOfBoundsException(
                "offset " + offset + ", count " + count + ", length " + length);
        }
    }

    static String valueOfCodePoint(int codePoint) {
        if (COMPACT_STRINGS && StringLatin1.canEncode(codePoint)) {
            return new String(StringLatin1.toBytes((char)codePoint), CODER_LATIN1);
        } else if (Character.isBmpCodePoint(codePoint)) {
            return new String(StringUTF16.toBytes((char)codePoint), CODER_UTF16);
        } else if (Character.isSupplementaryCodePoint(codePoint)) {
            return new String(StringUTF16.toBytesSupplementary(codePoint), CODER_UTF16);
        }

        throw new IllegalArgumentException(
                format("Not a valid Unicode code point: 0x%X", codePoint));
    }

    static void checkBoundsBeginEnd(int begin, int end, int length) {
        if (begin < 0 || begin > end || end > length) {
            throw new StringIndexOutOfBoundsException(
                "begin " + begin + ", end " + end + ", length " + length);
        }
    }

    @Override
    public Optional<String> describeConstable() {
        return Optional.of(this);
    }

    @Override
    public String resolveConstantDesc(MethodHandles.Lookup lookup) {
        return this;
    }
}
