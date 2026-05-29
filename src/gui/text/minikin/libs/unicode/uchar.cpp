#include "unicode/uchar.h"
#include <cstdint>

static int32_t getGraphemeClusterBreak(UChar32 c) {
    if (c == 0x000D) return U_GCB_CR;
    if (c == 0x000A) return U_GCB_LF;
    if ((c >= 0x0000 && c <= 0x001F) || (c >= 0x007F && c <= 0x009F)) {
        return U_GCB_CONTROL;
    }
    if ((c >= 0x0300 && c <= 0x036F) || (c >= 0x1DC0 && c <= 0x1DFF) ||
        (c >= 0x20D0 && c <= 0x20FF) || (c >= 0xFE20 && c <= 0xFE2F)) {
        return U_GCB_EXTEND;
    }
    if (c == 0x200D) return U_GCB_ZWJ;
    if (c >= 0x1F1E6 && c <= 0x1F1FF) return U_GCB_REGIONAL_INDICATOR;
    if (c >= 0x0E40 && c <= 0x0E4F) return U_GCB_PREPEND;
    if (c >= 0x09BE && c <= 0x09CC) return U_GCB_SPACING_MARK;
    if (c >= 0xAC00 && c <= 0xD7AF) {
        uint32_t s = c - 0xAC00;
        uint32_t t = s % 28;
        uint32_t v = (s / 28) % 21;
        if (v == 0 && t == 0) return U_GCB_L;
        if (v > 0 && t == 0) return U_GCB_LV;
        if (t > 0) return U_GCB_LVT;
    }
    if (c >= 0x1100 && c <= 0x115F) return U_GCB_L;
    if (c >= 0x1160 && c <= 0x11A7) return U_GCB_V;
    if (c >= 0x11A8 && c <= 0x11FF) return U_GCB_T;
    return U_GCB_OTHER;
}

static int32_t getCanonicalCombiningClass(UChar32 c) {
    if (c >= 0x0300 && c <= 0x0345) return c - 0x0300 + 1;
    if (c >= 0x034B && c <= 0x034D) return c - 0x034B + 0x20;
    if (c >= 0x0350 && c <= 0x0352) return c - 0x0350 + 0x23;
    if (c >= 0x0359 && c <= 0x036F) return c - 0x0359 + 0x2A;
    if (c >= 0x1DC0 && c <= 0x1DF9) return c - 0x1DC0 + 0x200;
    return 0;
}

static int32_t getGeneralCategory(UChar32 c) {
    if (c >= 'A' && c <= 'Z') return U_UPPERCASE_LETTER;
    if (c >= 'a' && c <= 'z') return U_LOWERCASE_LETTER;
    if (c >= '0' && c <= '9') return U_DECIMAL_DIGIT_NUMBER;
    if (c == ' ') return U_SPACE_SEPARATOR;
    if (c == '\t') return U_SPACE_SEPARATOR;
    if (c == '\n') return U_CONTROL;
    if (c == '\r') return U_CONTROL;
    if ((c >= 0x0000 && c <= 0x001F) || (c >= 0x007F && c <= 0x009F)) {
        return U_CONTROL;
    }
    if (c >= 0x0020 && c <= 0x007E) return U_UNASSIGNED;
    if (c >= 0x4E00 && c <= 0x9FFF) return U_OTHER_LETTER;
    if (c >= 0x3040 && c <= 0x309F) return U_LOWERCASE_LETTER;
    if (c >= 0x30A0 && c <= 0x30FF) return U_UPPERCASE_LETTER;
    if (c >= 0xAC00 && c <= 0xD7AF) return U_OTHER_LETTER;
    if (c >= 0x0600 && c <= 0x06FF) return U_OTHER_LETTER;
    if (c >= 0x0590 && c <= 0x05FF) return U_OTHER_LETTER;
    if (c >= 0x0900 && c <= 0x097F) return U_OTHER_LETTER;
    if (c >= 0x0980 && c <= 0x09FF) return U_OTHER_LETTER;
    if (c >= 0x0A00 && c <= 0x0A7F) return U_OTHER_LETTER;
    if (c >= 0x0A80 && c <= 0x0AFF) return U_OTHER_LETTER;
    if (c >= 0x0B00 && c <= 0x0B7F) return U_OTHER_LETTER;
    if (c >= 0x0B80 && c <= 0x0BFF) return U_OTHER_LETTER;
    if (c >= 0x0C00 && c <= 0x0C7F) return U_OTHER_LETTER;
    if (c >= 0x0C80 && c <= 0x0CFF) return U_OTHER_LETTER;
    if (c >= 0x0D00 && c <= 0x0D7F) return U_OTHER_LETTER;
    if (c >= 0x0E00 && c <= 0x0E7F) return U_OTHER_LETTER;
    if (c >= 0x0300 && c <= 0x036F) return U_NON_SPACING_MARK;
    if (c >= 0x1DC0 && c <= 0x1DFF) return U_NON_SPACING_MARK;
    if (c >= 0x20D0 && c <= 0x20FF) return U_NON_SPACING_MARK;
    if (c >= 0xFE20 && c <= 0xFE2F) return U_COMBINING_SPACING_MARK;
    return U_UNASSIGNED;
}

static int32_t getJoiningType(UChar32 c) {
    if (c >= 0x0621 && c <= 0x063A) return U_JT_RIGHT_JOINING;
    if (c >= 0x0641 && c <= 0x064A) return U_JT_RIGHT_JOINING;
    if (c >= 0x066E && c <= 0x066F) return U_JT_RIGHT_JOINING;
    if (c >= 0x0671 && c <= 0x06D3) return U_JT_RIGHT_JOINING;
    if (c >= 0x06D5 && c <= 0x06DC) return U_JT_RIGHT_JOINING;
    if (c >= 0x06DE && c <= 0x06E8) return U_JT_RIGHT_JOINING;
    if (c >= 0x06EA && c <= 0x06ED) return U_JT_RIGHT_JOINING;
    if (c >= 0xFB50 && c <= 0xFDFF) return U_JT_RIGHT_JOINING;
    if (c >= 0xFE70 && c <= 0xFEFE) return U_JT_RIGHT_JOINING;
    if (c == 0x0622 || c == 0x0623 || c == 0x0625 || c == 0x0624) {
        return U_JT_DUAL_JOINING;
    }
    if (c == 0x0640) return U_JT_JOIN_CAUSING;
    if ((c >= 0x0300 && c <= 0x036F) || (c >= 0x20D0 && c <= 0x20FF)) {
        return U_JT_TRANSPARENT;
    }
    return U_JT_NON_JOINING;
}

static int32_t getLineBreak(UChar32 c) {
    if (c == '\r') return U_LB_CARRIAGE_RETURN;
    if (c == '\n') return U_LB_LINE_FEED;
    if (c == ' ' || c == '\t') return U_LB_SPACE;
    if ((c >= 0x0000 && c <= 0x001F) || (c >= 0x007F && c <= 0x009F)) {
        return U_LB_COMBINING_MARK;
    }
    if (c >= 'A' && c <= 'Z') return U_LB_ALPHABETIC;
    if (c >= 'a' && c <= 'z') return U_LB_ALPHABETIC;
    if (c >= '0' && c <= '9') return U_LB_NUMERIC;
    if (c >= 0x4E00 && c <= 0x9FFF) return U_LB_IDEOGRAPHIC;
    if (c >= 0xAC00 && c <= 0xD7AF) return U_LB_HEBREW_LETTER;
    if (c >= 0x0600 && c <= 0x06FF) return U_LB_ALPHABETIC;
    if (c >= 0x0590 && c <= 0x05FF) return U_LB_ALPHABETIC;
    if (c >= 0x0900 && c <= 0x0D7F) return U_LB_IDEOGRAPHIC;
    if (c >= 0x0E00 && c <= 0x0E7F) return U_LB_COMPLEX_CONTEXT;
    if (c == '(' || c == '[' || c == '{' || c == '<') return U_LB_OPEN_PUNCTUATION;
    if (c == ')' || c == ']' || c == '}' || c == '>') return U_LB_CLOSE_PUNCTUATION;
    if (c == '"' || c == '\'' || c == 0x201C || c == 0x201D) return U_LB_QUOTATION;
    if (c == '-' || c == 0x2010 || c == 0x2011) return U_LB_HYPHEN;
    if (c == '.' || c == ',' || c == ';' || c == ':' || c == '!') {
        return U_LB_POSTFIX_NUMERIC;
    }
    return U_LB_UNKNOWN;
}

U_CAPI int32_t U_EXPORT2 u_getIntPropertyValue(UChar32 c, uint32_t property) {
    switch (property) {
        case UCHAR_GRAPHEME_CLUSTER_BREAK:
            return getGraphemeClusterBreak(c);
        case UCHAR_CANONICAL_COMBINING_CLASS:
            return getCanonicalCombiningClass(c);
        case UCHAR_GENERAL_CATEGORY:
            return getGeneralCategory(c);
        case UCHAR_JOINING_TYPE:
            return getJoiningType(c);
        case UCHAR_LINE_BREAK:
            return getLineBreak(c);
        default:
            return 0;
    }
}

U_CAPI UBool U_EXPORT2 u_isbase(UChar32 c) {
    int32_t cat = getGeneralCategory(c);
    return (cat >= U_UPPERCASE_LETTER && cat <= U_OTHER_LETTER) ||
           (cat >= U_DECIMAL_DIGIT_NUMBER && cat <= U_OTHER_NUMBER);
}

U_CAPI UCharDirection U_EXPORT2 u_charDirection(UChar32 c) {
    if (c >= 0x0600 && c <= 0x06FF) return U_RIGHT_TO_LEFT_ARABIC;
    if (c >= 0x0590 && c <= 0x05FF) return U_RIGHT_TO_LEFT;
    if (c >= 0x07B0 && c <= 0x07FF) return U_RIGHT_TO_LEFT_ARABIC;
    if (c >= 0xFB50 && c <= 0xFDFF) return U_RIGHT_TO_LEFT_ARABIC;
    if (c >= 0xFE70 && c <= 0xFEFF) return U_RIGHT_TO_LEFT_ARABIC;
    if (c >= '0' && c <= '9') return U_EUROPEAN_NUMBER;
    if (c == '.' || c == ',') return U_COMMON_NUMBER_SEPARATOR;
    if (c == ' ') return U_WHITE_SPACE_NEUTRAL;
    return U_LEFT_TO_RIGHT;
}

U_CAPI UBool U_EXPORT2 u_isMirrored(UChar32 c) {
    switch (c) {
        case '(': case ')':
        case '[': case ']':
        case '{': case '}':
        case '<': case '>':
        case 0x00AB: case 0x00BB:
        case 0x2039: case 0x203A:
        case 0x2018: case 0x2019:
        case 0x201C: case 0x201D:
            return true;
        default:
            return false;
    }
}

U_CAPI UChar32 U_EXPORT2 u_charMirror(UChar32 c) {
    switch (c) {
        case '(': return ')';
        case ')': return '(';
        case '[': return ']';
        case ']': return '[';
        case '{': return '}';
        case '}': return '{';
        case '<': return '>';
        case '>': return '<';
        case 0x00AB: return 0x00BB;
        case 0x00BB: return 0x00AB;
        case 0x2039: return 0x203A;
        case 0x203A: return 0x2039;
        default:
            return c;
    }
}

U_CAPI UChar32 U_EXPORT2 u_getBidiPairedBracket(UChar32 c) {
    switch (c) {
        case '(': return ')';
        case ')': return '(';
        case '[': return ']';
        case ']': return '[';
        case '{': return '}';
        case '}': return '{';
        case '<': return '>';
        case '>': return '<';
        case 0x00AB: return 0x00BB;
        case 0x00BB: return 0x00AB;
        case 0x2018: return 0x2019;
        case 0x2019: return 0x2018;
        case 0x201C: return 0x201D;
        case 0x201D: return 0x201C;
        case 0x2039: return 0x203A;
        case 0x203A: return 0x2039;
        default:
            return c;
    }
}

U_CAPI int8_t U_EXPORT2 u_charType(UChar32 c) {
    return getGeneralCategory(c);
}

U_CAPI UBool U_EXPORT2 u_hasBinaryProperty(UChar32 c, uint32_t property) {
    switch (property) {
        case UCHAR_EMOJI:
            return (c >= 0x2600 && c <= 0x26FF) ||
                   (c >= 0x2700 && c <= 0x27BF) ||
                   (c >= 0x1F300 && c <= 0x1F5FF) ||
                   (c >= 0x1F600 && c <= 0x1F64F) ||
                   (c >= 0x1F680 && c <= 0x1F6FF) ||
                   (c >= 0x1F900 && c <= 0x1F9FF);
        case UCHAR_EMOJI_MODIFIER:
            return (c >= 0x1F3FB && c <= 0x1F3FF);
        case UCHAR_EMOJI_MODIFIER_BASE:
            return (c >= 0x1F466 && c <= 0x1F469) ||
                   (c >= 0x1F910 && c <= 0x1F917) ||
                   (c >= 0x1F930 && c <= 0x1F939) ||
                   (c >= 0x1F9B0 && c <= 0x1F9B9);
        case UCHAR_EXTENDED_PICTOGRAPHIC:
            return (c >= 0x2600 && c <= 0x26FF) ||
                   (c >= 0x2700 && c <= 0x27BF) ||
                   (c >= 0x1F000 && c <= 0x1F0FF) ||
                   (c >= 0x1F100 && c <= 0x1F1FF) ||
                   (c >= 0x1F200 && c <= 0x1F2FF) ||
                   (c >= 0x1F300 && c <= 0x1F5FF) ||
                   (c >= 0x1F600 && c <= 0x1F64F) ||
                   (c >= 0x1F680 && c <= 0x1F6FF) ||
                   (c >= 0x1F700 && c <= 0x1F77F) ||
                   (c >= 0x1F900 && c <= 0x1F9FF);
        default:
            return false;
    }
}

U_CAPI UBool U_EXPORT2 u_iscntrl(UChar32 c) {
    return (c >= 0x0000 && c <= 0x001F) || (c >= 0x007F && c <= 0x009F);
}