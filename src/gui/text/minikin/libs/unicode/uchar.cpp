#include "unicode/uchar.h"
#include "unicode/uscript.h"
#include "unicode/ubidi.h"
#include "unicode_range_data.h"
#include <algorithm>

// Unicode 范围数据表（精简版，覆盖常用字符）

const UnicodeRange g_unicodeRanges[] = {
    // 控制字符 (0x0000-0x001F)
    {0x0000, 0x001F, U_OTHER_NEUTRAL, U_CONTROL, U_GCB_CONTROL, U_LB_MANDATORY_BREAK, 0, USCRIPT_COMMON, 0},
    
    // 空格 (0x0020)
    {0x0020, 0x0020, U_WHITE_SPACE_NEUTRAL, U_SPACE_SEPARATOR, U_GCB_OTHER, U_LB_SPACE, 0, USCRIPT_COMMON, (1u << 0)},  // U_WHITE_SPACE = 0
    
    // 标点符号 (!") (0x0021-0x0022)
    {0x0021, 0x0022, U_OTHER_NEUTRAL, U_OTHER_PUNCTUATION, U_GCB_OTHER, U_LB_EXCLAMATION, 0, USCRIPT_COMMON, 0},
    
    // #$%
    {0x0023, 0x0023, U_OTHER_NEUTRAL, U_OTHER_PUNCTUATION, U_GCB_OTHER, U_LB_ALPHABETIC, 0, USCRIPT_COMMON, 0},
    {0x0024, 0x0024, U_EUROPEAN_NUMBER_TERMINATOR, U_CURRENCY_SYMBOL, U_GCB_OTHER, U_LB_PREFIX_NUMERIC, 0, USCRIPT_COMMON, 0},
    {0x0025, 0x0025, U_EUROPEAN_NUMBER_TERMINATOR, U_OTHER_PUNCTUATION, U_GCB_OTHER, U_LB_POSTFIX_NUMERIC, 0, USCRIPT_COMMON, 0},
    
    // &'
    {0x0026, 0x0027, U_OTHER_NEUTRAL, U_OTHER_PUNCTUATION, U_GCB_OTHER, U_LB_ALPHABETIC, 0, USCRIPT_COMMON, 0},
    
    // 括号 ()
    {0x0028, 0x0028, U_OTHER_NEUTRAL, U_OPEN_PUNCTUATION, U_GCB_OTHER, U_LB_OPEN_PUNCTUATION, 0, USCRIPT_COMMON, (1u << 1)},
    {0x0029, 0x0029, U_OTHER_NEUTRAL, U_CLOSE_PUNCTUATION, U_GCB_OTHER, U_LB_CLOSE_PARENTHESIS, 0, USCRIPT_COMMON, (1u << 1)},
    
    // *+
    {0x002A, 0x002A, U_OTHER_NEUTRAL, U_MATH_SYMBOL, U_GCB_OTHER, U_LB_ALPHABETIC, 0, USCRIPT_COMMON, 0},
    {0x002B, 0x002B, U_EUROPEAN_NUMBER_SEPARATOR, U_MATH_SYMBOL, U_GCB_OTHER, U_LB_INFIX_NUMERIC, 0, USCRIPT_COMMON, 0},
    
    // ,
    {0x002C, 0x002C, U_COMMON_NUMBER_SEPARATOR, U_OTHER_PUNCTUATION, U_GCB_OTHER, U_LB_INFIX_NUMERIC, 0, USCRIPT_COMMON, 0},
    
    // -
    {0x002D, 0x002D, U_EUROPEAN_NUMBER_SEPARATOR, U_DASH_PUNCTUATION, U_GCB_OTHER, U_LB_HYPHEN, 0, USCRIPT_COMMON, 0},
    
    // .
    {0x002E, 0x002E, U_COMMON_NUMBER_SEPARATOR, U_OTHER_PUNCTUATION, U_GCB_OTHER, U_LB_INFIX_NUMERIC, 0, USCRIPT_COMMON, 0},
    
    // /
    {0x002F, 0x002F, U_OTHER_NEUTRAL, U_OTHER_PUNCTUATION, U_GCB_OTHER, U_LB_BREAK_SYMBOLS, 0, USCRIPT_COMMON, 0},
    
    // 数字 0-9
    {0x0030, 0x0039, U_EUROPEAN_NUMBER, U_DECIMAL_DIGIT_NUMBER, U_GCB_OTHER, U_LB_NUMERIC, 0, USCRIPT_COMMON, 0},
    
    // :;
    {0x003A, 0x003A, U_EUROPEAN_NUMBER_SEPARATOR, U_OTHER_PUNCTUATION, U_GCB_OTHER, U_LB_INFIX_NUMERIC, 0, USCRIPT_COMMON, 0},
    {0x003B, 0x003B, U_OTHER_NEUTRAL, U_OTHER_PUNCTUATION, U_GCB_OTHER, U_LB_INFIX_NUMERIC, 0, USCRIPT_COMMON, 0},
    
    // <=>
    {0x003C, 0x003E, U_OTHER_NEUTRAL, U_MATH_SYMBOL, U_GCB_OTHER, U_LB_ALPHABETIC, 0, USCRIPT_COMMON, (1u << 1)},
    
    // ?@
    {0x003F, 0x0040, U_OTHER_NEUTRAL, U_OTHER_PUNCTUATION, U_GCB_OTHER, U_LB_ALPHABETIC, 0, USCRIPT_COMMON, 0},
    
    // 大写字母 A-Z
    {0x0041, 0x005A, U_LEFT_TO_RIGHT, U_UPPERCASE_LETTER, U_GCB_OTHER, U_LB_ALPHABETIC, 0, USCRIPT_LATIN, 0},
    
    // [\]^_`
    {0x005B, 0x0060, U_OTHER_NEUTRAL, U_OPEN_PUNCTUATION, U_GCB_OTHER, U_LB_OPEN_PUNCTUATION, 0, USCRIPT_COMMON, 0},
    
    // 小写字母 a-z
    {0x0061, 0x007A, U_LEFT_TO_RIGHT, U_LOWERCASE_LETTER, U_GCB_OTHER, U_LB_ALPHABETIC, 0, USCRIPT_LATIN, 0},
    
    // {|}~
    {0x007B, 0x007E, U_OTHER_NEUTRAL, U_CLOSE_PUNCTUATION, U_GCB_OTHER, U_LB_CLOSE_PUNCTUATION, 0, USCRIPT_COMMON, 0},
    
    // DEL 和控制字符 (0x007F-0x009F)
    {0x007F, 0x009F, U_OTHER_NEUTRAL, U_CONTROL, U_GCB_CONTROL, U_LB_MANDATORY_BREAK, 0, USCRIPT_COMMON, 0},
    
    // 不间断空格 (0x00A0)
    {0x00A0, 0x00A0, U_WHITE_SPACE_NEUTRAL, U_SPACE_SEPARATOR, U_GCB_OTHER, U_LB_BREAK_BOTH, 0, USCRIPT_COMMON, (1u << 0)},  // U_WHITE_SPACE = 0
    
    // 上标、下标等
    {0x00AA, 0x00AA, U_OTHER_NEUTRAL, U_OTHER_LETTER, U_GCB_OTHER, U_LB_ALPHABETIC, 0, USCRIPT_LATIN, 0},
    {0x00B5, 0x00B5, U_OTHER_NEUTRAL, U_OTHER_LETTER, U_GCB_OTHER, U_LB_ALPHABETIC, 0, USCRIPT_LATIN, 0},
    
    // 中间点
    {0x00B7, 0x00B7, U_OTHER_NEUTRAL, U_OTHER_PUNCTUATION, U_GCB_OTHER, U_LB_ALPHABETIC, 0, USCRIPT_COMMON, 0},
    
    {0x00BA, 0x00BA, U_OTHER_NEUTRAL, U_OTHER_LETTER, U_GCB_OTHER, U_LB_ALPHABETIC, 0, USCRIPT_LATIN, 0},
    
    // 组合变音符号 (0x0300-0x036F)
    {0x0300, 0x036F, U_DIR_NON_SPACING_MARK, U_NON_SPACING_MARK, U_GCB_EXTEND, U_LB_COMBINING_MARK, 0, USCRIPT_INHERITED, 0},
    
    // 希腊文 (0x0370-0x03FF)
    {0x0370, 0x03FF, U_LEFT_TO_RIGHT, U_OTHER_LETTER, U_GCB_OTHER, U_LB_ALPHABETIC, 0, USCRIPT_GREEK, 0},
    
    // 西里尔文 (0x0400-0x04FF)
    {0x0400, 0x04FF, U_LEFT_TO_RIGHT, U_OTHER_LETTER, U_GCB_OTHER, U_LB_ALPHABETIC, 0, USCRIPT_CYRILLIC, 0},
    
    // 希伯来文 (0x0590-0x05FF)
    {0x0590, 0x05FF, U_RIGHT_TO_LEFT, U_OTHER_LETTER, U_GCB_OTHER, U_LB_HEBREW_LETTER, 0, USCRIPT_HEBREW, 0},
    
    // 阿拉伯文 (0x0600-0x06FF)
    {0x0600, 0x06FF, U_RIGHT_TO_LEFT_ARABIC, U_OTHER_LETTER, U_GCB_OTHER, U_LB_ARABIC_LETTER, 0, USCRIPT_ARABIC, 0},
    
    // CJK 统一表意文字 (0x4E00-0x9FFF)
    {0x4E00, 0x9FFF, U_LEFT_TO_RIGHT, U_OTHER_LETTER, U_GCB_OTHER, U_LB_IDEOGRAPHIC, 0, USCRIPT_HAN, 0},
    
    // 平假名 (0x3040-0x309F)
    {0x3040, 0x309F, U_LEFT_TO_RIGHT, U_OTHER_LETTER, U_GCB_OTHER, U_LB_IDEOGRAPHIC, 0, USCRIPT_HIRAGANA, 0},
    
    // 片假名 (0x30A0-0x30FF)
    {0x30A0, 0x30FF, U_LEFT_TO_RIGHT, U_OTHER_LETTER, U_GCB_OTHER, U_LB_IDEOGRAPHIC, 0, USCRIPT_KATAKANA, 0},
    
    // Hangul 音节 (0xAC00-0xD7AF)
    {0xAC00, 0xD7AF, U_LEFT_TO_RIGHT, U_OTHER_LETTER, U_GCB_LV, U_LB_LV, 0, USCRIPT_HANGUL, 0},
    
    // Emoji (0x1F600-0x1F64F)
    {0x1F600, 0x1F64F, U_OTHER_NEUTRAL, U_OTHER_SYMBOL, U_GCB_OTHER, U_LB_IDEOGRAPHIC, 0, USCRIPT_COMMON, 0},
    
    // Emoji (0x1F300-0x1F5FF)
    {0x1F300, 0x1F5FF, U_OTHER_NEUTRAL, U_OTHER_SYMBOL, U_GCB_OTHER, U_LB_IDEOGRAPHIC, 0, USCRIPT_COMMON, 0},
    
    // 区域指示符号 (0x1F1E0-0x1F1FF)
    {0x1F1E0, 0x1F1FF, U_OTHER_NEUTRAL, U_OTHER_SYMBOL, U_GCB_REGIONAL_INDICATOR, U_LB_REGIONAL_INDICATOR, 0, USCRIPT_COMMON, 0},
};

const int g_unicodeRangesCount = sizeof(g_unicodeRanges) / sizeof(g_unicodeRanges[0]);

// ============================================================================
// 查找函数实现
// ============================================================================

const UnicodeRange* findUnicodeRange(UChar32 c) {
    // 处理无效码点
    if (c < 0 || c > 0x10FFFF) {
        static const UnicodeRange invalidRange = {
            0, 0, U_OTHER_NEUTRAL, U_UNASSIGNED, U_GCB_OTHER, U_LB_AMBIGUOUS, 0, USCRIPT_UNKNOWN, 0
        };
        return &invalidRange;
    }
    
    // 二分查找
    int left = 0;
    int right = g_unicodeRangesCount - 1;
    
    while (left <= right) {
        int mid = left + (right - left) / 2;
        const auto& range = g_unicodeRanges[mid];
        
        if (c >= range.start && c <= range.end) {
            return &range;
        } else if (c < range.start) {
            right = mid - 1;
        } else {
            left = mid + 1;
        }
    }
    
    // 返回默认范围
    static const UnicodeRange defaultRange = {
        0, 0, U_LEFT_TO_RIGHT, U_OTHER_LETTER, U_GCB_OTHER, U_LB_ALPHABETIC, 0, USCRIPT_UNKNOWN, 0
    };
    return &defaultRange;
}

// ============================================================================
// 统一的属性查询接口
// ============================================================================

U_CAPI UCharDirection U_EXPORT2 u_charDirection(UChar32 c) {
    const auto* range = findUnicodeRange(c);
    return (UCharDirection)range->directionality;
}

U_CAPI UBool U_EXPORT2 u_isMirrored(UChar32 c) {
    const auto* range = findUnicodeRange(c);
    return range->hasBinaryProperty(1);  // UCHAR_BIDI_MIRRORED = 1
}

U_CAPI UChar32 U_EXPORT2 u_charMirror(UChar32 c) {
    if (u_isMirrored(c)) {
        return c;
    }
    return c;
}

U_CAPI int32_t U_EXPORT2 u_getIntPropertyValue(UChar32 c, uint32_t property) {
    const auto* range = findUnicodeRange(c);
    
    switch (property) {
        case UCHAR_BIDI_CLASS:
            return range->directionality;
        
        case UCHAR_GENERAL_CATEGORY:
            return range->category;
        
        case UCHAR_GRAPHEME_CLUSTER_BREAK:
            return range->gcb;
        
        case UCHAR_LINE_BREAK:
            return range->lb;
        
        case UCHAR_CANONICAL_COMBINING_CLASS:
            return range->ccc;
        
        case UCHAR_EMOJI:
            return range->hasBinaryProperty(8) ? 1 : 0;
        
        case UCHAR_EXTENDED_PICTOGRAPHIC:
            return range->hasBinaryProperty(151) ? 1 : 0;
        
        case UCHAR_EMOJI_MODIFIER:
            return range->hasBinaryProperty(149) ? 1 : 0;
        
        case UCHAR_EMOJI_MODIFIER_BASE:
            return range->hasBinaryProperty(150) ? 1 : 0;
        
        default:
            return 0;
    }
}

U_CAPI UBool U_EXPORT2 u_hasBinaryProperty(UChar32 c, uint32_t property) {
    const auto* range = findUnicodeRange(c);
    return range->hasBinaryProperty(property);
}

U_CAPI UScriptCode U_EXPORT2 u_getScript(UChar32 c, UErrorCode* pErrorCode) {
    if (pErrorCode && U_FAILURE(*pErrorCode)) {
        return USCRIPT_INVALID_CODE;
    }
    
    const auto* range = findUnicodeRange(c);
    
    if (pErrorCode) {
        *pErrorCode = U_ZERO_ERROR;
    }
    
    return (UScriptCode)range->script;
}

U_CAPI int32_t U_EXPORT2 u_getCombiningClass(UChar32 c) {
    const auto* range = findUnicodeRange(c);
    return range->ccc;
}

U_CAPI int8_t U_EXPORT2 u_charType(UChar32 c) {
    const auto* range = findUnicodeRange(c);
    return (int8_t)range->category;
}

// 辅助函数
U_CAPI UBool U_EXPORT2 u_isbase(UChar32 c) {
    const auto* range = findUnicodeRange(c);
    int32_t cat = range->category;
    return (cat >= U_UPPERCASE_LETTER && cat <= U_OTHER_LETTER) ||
           (cat >= U_DECIMAL_DIGIT_NUMBER && cat <= U_OTHER_NUMBER);
}

U_CAPI UBool U_EXPORT2 u_isUAlphabetic(UChar32 c) {
    const auto* range = findUnicodeRange(c);
    return (range->category >= U_UPPERCASE_LETTER && range->category <= U_OTHER_LETTER);
}

U_CAPI UBool U_EXPORT2 u_isULowercase(UChar32 c) {
    const auto* range = findUnicodeRange(c);
    return range->category == U_LOWERCASE_LETTER;
}

U_CAPI UBool U_EXPORT2 u_isUUppercase(UChar32 c) {
    const auto* range = findUnicodeRange(c);
    return range->category == U_UPPERCASE_LETTER;
}

U_CAPI UBool U_EXPORT2 u_isWhitespace(UChar32 c) {
    const auto* range = findUnicodeRange(c);
    return range->hasBinaryProperty(40);  // U_WHITE_SPACE = 40
}

U_CAPI UBool U_EXPORT2 u_isSpaceChar(UChar32 c) {
    const auto* range = findUnicodeRange(c);
    return range->category == U_SPACE_SEPARATOR;
}

U_CAPI UBool U_EXPORT2 u_isPrintable(UChar32 c) {
    const auto* range = findUnicodeRange(c);
    return range->category != U_CONTROL && range->category != U_FORMAT && 
           range->category != U_UNASSIGNED && range->category != U_PRIVATE_USE &&
           range->category != U_SURROGATE;
}

U_CAPI UBool U_EXPORT2 u_isDefined(UChar32 c) {
    const auto* range = findUnicodeRange(c);
    return range->category != U_UNASSIGNED;
}

U_CAPI UBool U_EXPORT2 u_isLetter(UChar32 c) {
    const auto* range = findUnicodeRange(c);
    return (range->category >= U_UPPERCASE_LETTER && range->category <= U_OTHER_LETTER);
}

U_CAPI UBool U_EXPORT2 u_isDigit(UChar32 c) {
    const auto* range = findUnicodeRange(c);
    return range->category == U_DECIMAL_DIGIT_NUMBER;
}

U_CAPI UBool U_EXPORT2 u_isPunct(UChar32 c) {
    const auto* range = findUnicodeRange(c);
    return (range->category >= U_CONTROL && range->category <= U_FORMAT);
}

U_CAPI UBool U_EXPORT2 u_isSymbol(UChar32 c) {
    const auto* range = findUnicodeRange(c);
    return range->category >= U_CURRENCY_SYMBOL;
}

U_CAPI UBool U_EXPORT2 u_isMark(UChar32 c) {
    const auto* range = findUnicodeRange(c);
    return (range->category >= U_NON_SPACING_MARK && range->category <= U_COMBINING_SPACING_MARK);
}

U_CAPI UBool U_EXPORT2 u_isNumber(UChar32 c) {
    const auto* range = findUnicodeRange(c);
    return (range->category >= U_DECIMAL_DIGIT_NUMBER && range->category <= U_OTHER_NUMBER);
}

U_CAPI UBool U_EXPORT2 u_isSeparator(UChar32 c) {
    const auto* range = findUnicodeRange(c);
    return (range->category >= U_SPACE_SEPARATOR && range->category <= U_PARAGRAPH_SEPARATOR);
}

// ============================================================================
// 字符大小写转换函数
// ============================================================================

U_CAPI UChar32 U_EXPORT2 u_tolower(UChar32 c) {
    if (c >= 'A' && c <= 'Z') {
        return c + ('a' - 'A');
    }
    return c;
}

U_CAPI UChar32 U_EXPORT2 u_toupper(UChar32 c) {
    if (c >= 'a' && c <= 'z') {
        return c - ('a' - 'A');
    }
    return c;
}

U_CAPI UChar32 U_EXPORT2 u_totitle(UChar32 c) {
    if (c >= 'a' && c <= 'z') {
        return c - ('a' - 'A');
    }
    return c;
}

// ============================================================================
// 数字相关函数
// ============================================================================

U_CAPI int32_t U_EXPORT2 u_digit(UChar32 c, int32_t radix) {
    if (radix < 2 || radix > 36) {
        return -1;
    }
    
    if (c >= '0' && c <= '9') {
        int digit = c - '0';
        return (digit < radix) ? digit : -1;
    }
    
    if (c >= 'A' && c <= 'Z') {
        int digit = 10 + (c - 'A');
        return (digit < radix) ? digit : -1;
    }
    
    if (c >= 'a' && c <= 'z') {
        int digit = 10 + (c - 'a');
        return (digit < radix) ? digit : -1;
    }
    
    return -1;
}

U_CAPI double U_EXPORT2 u_getNumericValue(UChar32 c) {
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    return -1.0;
}

// ============================================================================
// 字符名称函数
// ============================================================================

U_CAPI int32_t U_EXPORT2 u_charName(UChar32 c, int32_t choice, 
                                      char* result, int32_t resultLength, 
                                      UErrorCode* pErrorCode) {
    if (pErrorCode && U_FAILURE(*pErrorCode)) {
        return 0;
    }
    
    if (resultLength > 0) {
        result[0] = '\0';
    }
    
    if (pErrorCode) {
        *pErrorCode = U_ZERO_ERROR;
    }
    
    return 0;
}
