#ifndef __TEXTLAYOUT_TEXT_UTILS_H__
#define __TEXTLAYOUT_TEXT_UTILS_H__
#include <string>
#include <vector>
#include <stdio.h>
#include <text/textpaint.h>
#include <text/parcelablespan.h>
//#include <core/spannablestring.h>
#include <text/textdirectionheuristics.h>
namespace cdroid{
class CharSequence;
class Spanned;
class Spannable;
class TextUtils {
    // Zero-width character used to fill ellipsized strings when codepoint length must be preserved.
    static constexpr char16_t ELLIPSIS_FILLER = 0xFEFF;//ZERO WIDTH NO-BREAK SPACE
private:
    static constexpr int LINE_FEED_CODE_POINT = 10;
    static constexpr int NBSP_CODE_POINT = 160;
    static constexpr int PARCEL_SAFE_TEXT_LENGTH = 100000;
public:
    static constexpr int SAFE_STRING_FLAG_TRIM = 0x1;
    static constexpr int SAFE_STRING_FLAG_SINGLE_LINE = 0x2;
    static constexpr int SAFE_STRING_FLAG_FIRST_LINE = 0x4;
    static constexpr int ALIGNMENT_SPAN = 1;
    static constexpr int FIRST_SPAN = ALIGNMENT_SPAN;
    static constexpr int FOREGROUND_COLOR_SPAN = 2;
    static constexpr int RELATIVE_SIZE_SPAN = 3;
    static constexpr int SCALE_X_SPAN = 4;
    static constexpr int STRIKETHROUGH_SPAN = 5;
    static constexpr int UNDERLINE_SPAN = 6;
    static constexpr int STYLE_SPAN = 7;
    static constexpr int BULLET_SPAN = 8;
    static constexpr int QUOTE_SPAN = 9;
    static constexpr int LEADING_MARGIN_SPAN = 10;
    static constexpr int URL_SPAN = 11;
    static constexpr int BACKGROUND_COLOR_SPAN = 12;
    static constexpr int TYPEFACE_SPAN = 13;
    static constexpr int SUPERSCRIPT_SPAN = 14;
    static constexpr int SUBSCRIPT_SPAN = 15;
    static constexpr int ABSOLUTE_SIZE_SPAN = 16;
    static constexpr int TEXT_APPEARANCE_SPAN = 17;
    static constexpr int ANNOTATION = 18;
    static constexpr int SUGGESTION_SPAN = 19;
    static constexpr int SPELL_CHECK_SPAN = 20;
    static constexpr int SUGGESTION_RANGE_SPAN = 21;
    static constexpr int EASY_EDIT_SPAN = 22;
    static constexpr int LOCALE_SPAN = 23;
    static constexpr int TTS_SPAN = 24;
    static constexpr int ACCESSIBILITY_CLICKABLE_SPAN = 25;
    static constexpr int ACCESSIBILITY_URL_SPAN = 26;
    static constexpr int LINE_BACKGROUND_SPAN = 27;
    static constexpr int LINE_HEIGHT_SPAN = 28;
    static constexpr int ACCESSIBILITY_REPLACEMENT_SPAN = 29;
    static constexpr int LAST_SPAN = ACCESSIBILITY_REPLACEMENT_SPAN;

    using EllipsizeCallback =std::function<void(int,int)>;//void ellipsized(int start, int end);
    enum TruncateAt {
        NONE=0,
        START,
        MIDDLE,
        END,
        MARQUEE,
        END_SMALL
    };
private:
    static bool isNewline(int codePoint);
    static bool isWhiteSpace(int codePoint);
public:
    static bool isPunctuation(int codePoint);
    static std::wstring utf8tounicode(const std::string&);
    static std::string unicode2utf8(const std::wstring&);
    static std::string utf16_utf8(const uint16_t*utf16,size_t);
    static std::string utf16_utf8(const std::u16string&utf16);
    static const std::u16string utf8_utf16(const std::string&utf8);
    static CharSequence* stringOrSpannedString(CharSequence* source);
    static bool isEmpty(const CharSequence* str);
    static bool isEmpty(const std::string&);
    static std::string trim(std::string&);
    static long strtol(const std::string&value);
    static std::string& replace(std::string&str,const std::string&sfind,const std::string&sreplace);
    static bool startWith(const std::string&str,const std::string&head);
    static bool endWith(const std::string&str,const std::string&tail);
    static void stringAppendV(std::string& dst, const char* format, va_list ap);
    static std::string stringPrintf(const char* fmt, ...);
    static void stringAppendF(std::string& dst, const char* format, ...);
    static std::string formatTime(const std::string& fmt, int64_t seconds);

    static std::string getEllipsisString(TextUtils::TruncateAt method);
    static void getChars(const CharSequence* s, int start, int end, char16_t* dest, int destoff);

    static int indexOf(const CharSequence* s, char16_t ch) {
        return indexOf(s, ch, 0);
    }

    static int indexOf(const CharSequence* s, char16_t ch, int start);
    static int indexOf(const CharSequence* s, char16_t ch, int start, int end);

    static int lastIndexOf(const CharSequence* s, char16_t ch);
    static int lastIndexOf(const CharSequence* s, char16_t ch, int last);
    static int lastIndexOf(const CharSequence* s, char16_t ch, int start, int last);

    static int indexOf(const CharSequence* s,const CharSequence* needle);
    static int indexOf(const CharSequence* s, const CharSequence* needle, int start);
    static int indexOf(const CharSequence* s,const CharSequence* needle, int start, int end);

    static bool regionMatches(const CharSequence* one, int toffset,const CharSequence* two, int ooffset, int len);

    static std::string substring(const CharSequence* source, int start, int end);
    static std::vector<std::string> split(const std::string& text, const std::string& delim);
    static std::vector<std::string> split(const std::string& text,int delim);
    static int getTrimmedLength(const CharSequence* s);
    static bool equals(const CharSequence* a,const CharSequence* b);

    /**
     * Flatten a CharSequence and whatever styles can be copied across processes
     * into the parcel.
     */
    static int getOffsetBefore(const CharSequence* text, int offset);
    static int getOffsetAfter(const CharSequence* text, int offset);

    static void copySpansFrom(const Spanned* source, int start, int end,
                                     const SpanFilter& kind, Spannable* dest, int destoff);

    static CharSequence* ellipsize(CharSequence* text, TextPaint& p, float avail, TruncateAt where) {
        return ellipsize(text, p, avail, where, false, nullptr);
    }

    static CharSequence* ellipsize(CharSequence* text, TextPaint& paint,
                float avail, TruncateAt where, bool preserveLength,const  EllipsizeCallback& callback) {
        return ellipsize(text, paint, avail, where, preserveLength, callback,
                TextDirectionHeuristics::FIRSTSTRONG_LTR, getEllipsisString(where));
    }

    static CharSequence* ellipsize(CharSequence* text, TextPaint& paint, float avail, TruncateAt where,
            bool preserveLength,const EllipsizeCallback& callback,
            const TextDirectionHeuristic* textDir,const std::string& ellipsis);

    static CharSequence* commaEllipsize(CharSequence* text,TextPaint& p, float avail,
            const std::string& oneMore, const std::string& more) {
        return commaEllipsize(text, p, avail, oneMore, more, TextDirectionHeuristics::FIRSTSTRONG_LTR);
    }

    static CharSequence* commaEllipsize(CharSequence* text, TextPaint& p, float avail,
            const std::string& oneMore, const std::string& more, const TextDirectionHeuristic* textDir);

    static bool couldAffectRtl(char16_t c);

    static bool doesNotNeedBidi(const std::vector<char16_t>& text, int start, int len);

    static std::string htmlEncode(const std::string& s);
    static CharSequence* concat(const std::vector<CharSequence*>&text);

    static bool isGraphic(const CharSequence* str);
    static bool isGraphic(char16_t c);
    static bool isDigitsOnly(const CharSequence* str);
    static bool isPrintableAscii(char16_t c);
    static bool isPrintableAsciiOnly(const CharSequence* str);

    static int getCapsMode(const CharSequence* cs, int off, int reqModes);

    static void removeEmptySpans(std::vector<const ParcelableSpan*>&spans,const Spanned* spanned,const SpanFilter&type);
    static int64_t packRangeInLong(int start, int end) {
        return (((long) start) << 32) | end;
    }

    static int unpackRangeStartFromLong(int64_t range) {
        return (int) (range >> 32);
    }

    static int unpackRangeEndFromLong(int64_t range) {
        return (int) (range & 0x00000000FFFFFFFFL);
    }

    static bool hasStyleSpan(const Spanned* spanned);
    static const CharSequence* trimNoCopySpans(const CharSequence* charSequence);
};
}/*endof namespace*/
#endif/*__TEXTLAYOUT_TEXT_UTILS_H__*/
