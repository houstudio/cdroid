#include "unicode/uchar.h"
#include "unicode/uscript.h"
#include "unicode/ubidi.h"
#include "unicode_range_data.h"
#include <algorithm>

U_CAPI UCharDirection U_EXPORT2 u_charDirection(UChar32 c) {
    const auto* range = findUnicodeRange(c);
    return (UCharDirection)range->directionality;
}

U_CAPI UBool U_EXPORT2 u_isMirrored(UChar32 c) {
    const auto* range = findUnicodeRange(c);
    // Named constant, not a literal: bit 1 is UCHAR_ASCII_HEX_DIGIT in the
    // myicu numbering (BIDI_MIRRORED = 3), so the old hardcoded 1 made
    // isMirrored true only for hex digits.
    return range->hasBinaryProperty(UCHAR_BIDI_MIRRORED);
}

U_CAPI UChar32 U_EXPORT2 u_charMirror(UChar32 c) {
    if (u_isMirrored(c)) {
        return c;
    }
    return c;
}

U_CAPI int32_t U_EXPORT2 u_getIntPropertyValue(UChar32 c, UProperty property) {
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

        // UAX#29 Word_Break: authoritative in the data table (the wbp field
        // ubrk.cpp reads) — the old default swallowed it to 0, leaving
        // worditerator.cc's mid-word-punctuation logic dead.
        case UCHAR_WORD_BREAK:
            return range->wbp;

        // Binary properties: pass the myicu enum value itself — the earlier
        // hardcoded 8/151/149/150 were upstream ICU numbering, which indexes
        // unrelated bits (and 151 is beyond the 0..127 bitmap, always false).
        case UCHAR_EMOJI:
        case UCHAR_EMOJI_PRESENTATION:
        case UCHAR_EMOJI_MODIFIER:
        case UCHAR_EMOJI_MODIFIER_BASE:
        case UCHAR_EMOJI_COMPONENT:
        case UCHAR_REGIONAL_INDICATOR:
        case UCHAR_EXTENDED_PICTOGRAPHIC:
            return range->hasBinaryProperty(property) ? 1 : 0;

        default:
            return 0;
    }
}

U_CAPI UBool U_EXPORT2 u_hasBinaryProperty(UChar32 c, UProperty property) {
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

U_CAPI UCharCategory U_EXPORT2 u_charType(UChar32 c) {
    const auto* range = findUnicodeRange(c);
    return (UCharCategory)range->category;
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

U_CAPI UBool U_EXPORT2 u_isWhiteSpace(UChar32 c) {
    const auto* range = findUnicodeRange(c);
    // Same disease as u_isMirrored: 40 is not WHITE_SPACE (= 31) in the
    // myicu numbering — Character::isWhitespace answered by the wrong bit.
    return range->hasBinaryProperty(UCHAR_WHITE_SPACE);
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
    int32_t cat = range->category;
    return (cat >= U_DASH_PUNCTUATION && cat <= U_OTHER_PUNCTUATION) ||  // Pd Ps Pe Pc Po (19-23)
           cat == U_INITIAL_PUNCTUATION || cat == U_FINAL_PUNCTUATION;     // Pi Pf (28-29)
}

U_CAPI UBool U_EXPORT2 u_isSymbol(UChar32 c) {
    const auto* range = findUnicodeRange(c);
    return range->category >= U_MATH_SYMBOL;  // Sm(24) Sc(25) Sk(26) So(27)
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
    return toLowerFull(c);
}

U_CAPI UChar32 U_EXPORT2 u_toupper(UChar32 c) {
    return toUpperFull(c);
}

U_CAPI UChar32 U_EXPORT2 u_totitle(UChar32 c) {
    // 标题大小写通常等同于大写
    return toUpperFull(c);
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
