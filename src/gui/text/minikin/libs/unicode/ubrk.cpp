#include "unicode/ubrk.h"
#include "unicode/utf16.h"
#include "unicode/uchar.h"
#include "unicode/uscript.h"
#include <cstdlib>
#include <cstring>

struct UBreakIterator {
    const UChar* text;
    int32_t length;
    int32_t current;
    int32_t type;
};

// Word Break Rule Types (from Unicode UAX #29)
typedef enum {
    WBP_OTHER,
    WBP_CR,
    WBP_LF,
    WBP_NEWLINE,
    WBP_ALPHA,
    WBP_NUMERIC,
    WBP_KATAKANA,
    WBP_IDEOGRAPHIC,
    WBP_EXTEND,
    WBP_FORMAT,
    WBP_LINK,
    WBP_DOUBLE_QUOTE,
    WBP_SINGLE_QUOTE,
    WBP_MID_NUMLET,
    WBP_MID_LETTER,
    WBP_MID_NUM,
    WBP_COMPLEX_CONTEXT,
    WBP_E_BASE,
    WBP_E_MODIFIER,
    WBP_GLUE_AFTER_ZWJ,
    WBP_ZWJ,
    WBP_REGIONAL_INDICATOR,
    WBP_BREAK,
    WBP_NON_BREAK,
    WBP_ARABIC,          // Arabic/Persian with joining behavior
    WBP_HEBREW,          // Hebrew with joining behavior
    WBP_INDIC,           // Indic scripts (Devanagari, Bengali, etc.)
    WBP_SE_ASIAN,        // Southeast Asian (Thai, Lao, Khmer)
    WBP_TIBETAN,         // Tibetan
    WBP_ETHIOPIC,        // Ethiopic (Ge'ez)
    WBP_MONGOLIAN,       // Mongolian
} WBProperty;

// Line Break Rule Types (from Unicode UAX #14)
typedef enum {
    LBP_UNKNOWN,
    LBP_AL,      // Alphabetic
    LBP_NU,      // Numeric
    LBP_ID,      // Ideographic
    LBP_IDS,     // Ideographic with Special Line Breaking
    LBP_INS,     // Inseparable Characters
    LBP_HY,      // Hyphen
    LBP_BA,      // Break After
    LBP_BB,      // Break Before
    LBP_B2,      // Break on Either Side (but not pair)
    LBP_ZW,      // Zero Width Space
    LBP_CM,      // Combining Mark
    LBP_WJ,      // Word Joiner
    LBP_H2,      // Hangul LV
    LBP_H3,      // Hangul LVT
    LBP_JL,      // Hangul L
    LBP_JV,      // Hangul V
    LBP_JT,      // Hangul T
    LBP_CN,      // Contingent Break
    LBP_CR,      // Carriage Return
    LBP_LF,      // Line Feed
    LBP_NL,      // Next Line
    LBP_BK,      // Mandatory Break
    LBP_SA,      // South-East Asian
    LBP_ALM,     // Arabic Letter
    LBP_HE,      // Hebrew Letter
    LBP_IN,      // Indic
    LBP_WS,      // Whitespace
    LBP_INS_SEP, // Infix Numeric Separator
    LBP_SY,      // Symbols Allowing Break After
    LBP_IS,      // Infix Numeric Symbol
    LBP_OP,      // Open Punctuation
    LBP_CL,      // Close Punctuation
    LBP_QU,      // Quotation
    LBP_EX,      // Exclamation/Interrogation
    LBP_PR,      // Prefix Numeric
    LBP_PO,      // Postfix Numeric
    LBP_ST,      // Suffix Numeric
    LBP_AI,      // Ambiguous (Alphabetic or Ideographic)
    LBP_FW,      // Fixed Width
    LBP_NB,      // Non-Breaking ("Glue")
    LBP_SG,      // Surrogate
} LBProperty;

// Helper to get general category
static inline int32_t getGeneralCategory(UChar32 c) {
    return u_getIntPropertyValue(c, UCHAR_GENERAL_CATEGORY);
}

// Helper to get joining type for Arabic/Hebrew script
static inline int32_t getJoiningType(UChar32 c) {
    return u_getIntPropertyValue(c, UCHAR_JOINING_TYPE);
}

// Check if script is Indic
static inline UBool isIndicScript(UScriptCode script) {
    switch (script) {
        case USCRIPT_DEVANAGARI:
        case USCRIPT_BENGALI:
        case USCRIPT_GURMUKHI:
        case USCRIPT_GUJARATI:
        case USCRIPT_ORIYA:
        case USCRIPT_TAMIL:
        case USCRIPT_TELUGU:
        case USCRIPT_KANNADA:
        case USCRIPT_MALAYALAM:
        case USCRIPT_SINHALA:
            return true;
        default:
            return false;
    }
}

// Check if script is Southeast Asian
static inline UBool isSEAsianScript(UScriptCode script) {
    switch (script) {
        case USCRIPT_THAI:
        case USCRIPT_LAO:
        case USCRIPT_KHMER:
        case USCRIPT_MYANMAR:
            return true;
        default:
            return false;
    }
}

// Get Word Break property for a character
static WBProperty getWBProperty(UChar32 c) {
    // Newline and separators
    if (c == '\r') return WBP_CR;
    if (c == '\n') return WBP_LF;
    
    int32_t cat = getGeneralCategory(c);
    
    // Whitespace and control
    if (cat == U_SPACE_SEPARATOR || cat == U_LINE_SEPARATOR || 
        cat == U_PARAGRAPH_SEPARATOR || cat == U_CONTROL) {
        return WBP_BREAK;
    }
    
    // Marks - Extend category
    if (cat == U_NON_SPACING_MARK || cat == U_ENCLOSING_MARK || 
        cat == U_COMBINING_SPACING_MARK) {
        return WBP_EXTEND;
    }
    
    // Format characters
    if (cat == U_FORMAT) {
        return WBP_FORMAT;
    }
    
    // Zero-Width Joiner
    if (c == 0x200D) return WBP_ZWJ;
    
    // Use uscript_getScript to determine script
    UErrorCode status = U_ZERO_ERROR;
    UScriptCode script = uscript_getScript(c, &status);
    
    // Arabic/Persian letters with joining behavior
    if (script == USCRIPT_ARABIC) {
        int32_t joinType = getJoiningType(c);
        if (joinType == U_JT_RIGHT_JOINING || joinType == U_JT_DUAL_JOINING || 
            joinType == U_JT_JOIN_CAUSING) {
            return WBP_ARABIC;
        }
        return WBP_ALPHA;
    }
    
    // Hebrew letters with joining behavior
    if (script == USCRIPT_HEBREW) {
        return WBP_HEBREW;
    }
    
    // Indic scripts (Devanagari, Bengali, etc.)
    if (isIndicScript(script)) {
        return WBP_INDIC;
    }
    
    // Southeast Asian scripts (Thai, Lao, Khmer)
    if (isSEAsianScript(script)) {
        return WBP_SE_ASIAN;
    }
    
    // Tibetan
    if (script == USCRIPT_TIBETAN) {
        return WBP_TIBETAN;
    }
    
    // Ethiopic
    if (script == USCRIPT_ETHIOPIC) {
        return WBP_ETHIOPIC;
    }
    
    // Mongolian
    if (script == USCRIPT_MONGOLIAN) {
        return WBP_MONGOLIAN;
    }
    
    // Letters
    if (cat >= U_UPPERCASE_LETTER && cat <= U_OTHER_LETTER) {
        return WBP_ALPHA;
    }
    
    // Digits
    if (cat == U_DECIMAL_DIGIT_NUMBER) {
        return WBP_NUMERIC;
    }
    
    // Katakana
    if (script == USCRIPT_KATAKANA || script == USCRIPT_KATAKANA_OR_HIRAGANA) {
        return WBP_KATAKANA;
    }
    
    // CJK ideographs
    if (script == USCRIPT_HAN) {
        return WBP_IDEOGRAPHIC;
    }
    
    // Special characters
    if (c == '\'' || c == 0x2019) return WBP_SINGLE_QUOTE;
    if (c == '"' || c == 0x201D) return WBP_DOUBLE_QUOTE;
    if (c == '_') return WBP_LINK;
    if (c == '-') return WBP_MID_LETTER;
    
    // Mid characters
    if (c == '.' || c == ',' || c == ';' || c == ':' || c == '/') {
        return WBP_MID_NUMLET;
    }
    
    return WBP_OTHER;
}

// Get Line Break property for a character
static LBProperty getLBProperty(UChar32 c) {
    // Newline
    if (c == '\r') return LBP_CR;
    if (c == '\n') return LBP_LF;
    
    // Whitespace
    if (c == ' ') return LBP_WS;
    if (c == '\t') return LBP_WS;
    
    int32_t cat = getGeneralCategory(c);
    
    // Use uscript_getScript to determine script
    UErrorCode status = U_ZERO_ERROR;
    UScriptCode script = uscript_getScript(c, &status);
    
    // Arabic letters
    if (script == USCRIPT_ARABIC) {
        return LBP_ALM;
    }
    
    // Hebrew letters
    if (script == USCRIPT_HEBREW) {
        return LBP_HE;
    }
    
    // Indic scripts
    if (isIndicScript(script)) {
        return LBP_IN;
    }
    
    // Southeast Asian
    if (isSEAsianScript(script)) {
        return LBP_SA;
    }
    
    // Letters
    if (cat >= U_UPPERCASE_LETTER && cat <= U_OTHER_LETTER) {
        return LBP_AL;
    }
    
    // Digits
    if (cat == U_DECIMAL_DIGIT_NUMBER) {
        return LBP_NU;
    }
    
    // CJK ideographs
    if (script == USCRIPT_HAN) {
        return LBP_ID;
    }
    
    // Hangul
    if (script == USCRIPT_HANGUL || script == USCRIPT_JAMO) {
        if (c >= 0xAC00 && c <= 0xD7AF) {
            uint32_t s = c - 0xAC00;
            uint32_t t = s % 28;
            if (t == 0) {
                uint32_t v = (s / 28) % 21;
                if (v == 0) return LBP_JL;
                return LBP_H2;
            }
            return LBP_H3;
        }
        if (c >= 0x1100 && c <= 0x115F) return LBP_JL;
        if (c >= 0x1160 && c <= 0x11A7) return LBP_JV;
        if (c >= 0x11A8 && c <= 0x11FF) return LBP_JT;
    }
    
    // Punctuation
    if (c == '(' || c == '[' || c == '{' || c == '<') return LBP_OP;
    if (c == ')' || c == ']' || c == '}' || c == '>') return LBP_CL;
    if (c == '"' || c == '\'' || c == 0x201C || c == 0x201D ||
        c == 0x2018 || c == 0x2019) return LBP_QU;
    if (c == '!' || c == '?') return LBP_EX;
    if (c == '-' || c == 0x2010 || c == 0x2011) return LBP_HY;
    
    // Symbols
    if (c == ',' || c == ';' || c == ':' || c == '/') {
        return LBP_SY;
    }
    if (c == '.') return LBP_PO;
    
    return LBP_AL;
}

// Check word boundary according to UAX #29 rules with script-specific handling
static UBool isWordBoundary(const UChar* text, int32_t length, int32_t pos) {
    if (pos <= 0 || pos >= length) {
        return true;
    }
    UChar32 prev = text[pos - 1];
    int32_t prevLen = 1;
    if (U16_IS_TRAIL(prev) && pos > 1) {
        prev = U16_GET_SUPPLEMENTARY(text[pos - 2], text[pos - 1]);
        prevLen = 2;
    }
    
    UChar32 curr = text[pos];
    int32_t currLen = 1;
    if (U16_IS_LEAD(curr) && pos + 1 < length) {
        curr = U16_GET_SUPPLEMENTARY(text[pos], text[pos + 1]);
        currLen = 2;
    }
    
    WBProperty propPrev = getWBProperty(prev);
    WBProperty propCurr = getWBProperty(curr);
    
    // WB1: Break at start and end of text
    if (pos == 0 || pos == length) return true;
    
    // WB2 & WB3: Newline handling
    if (propPrev == WBP_CR && propCurr != WBP_LF) return true;
    if (propPrev == WBP_LF || propPrev == WBP_CR) return true;
    
    // WB4: Don't break within surrogate pairs
    if (U16_IS_LEAD(prev) && U16_IS_TRAIL(curr)) return false;
    
    // WB5: Ignore format and extend characters
    if (propPrev == WBP_EXTEND || propPrev == WBP_FORMAT) {
        return isWordBoundary(text, length, pos - prevLen);
    }
    if (propCurr == WBP_EXTEND || propCurr == WBP_FORMAT) {
        return isWordBoundary(text, length, pos + currLen);
    }
    
    // WB5a: Ignore ZWJ
    if (propPrev == WBP_ZWJ) {
        return isWordBoundary(text, length, pos - prevLen);
    }
    if (propCurr == WBP_ZWJ) {
        return isWordBoundary(text, length, pos + currLen);
    }
    
    // WB6: Don't break between letters (including Arabic, Hebrew, Indic, etc.)
    bool prevIsLetter = (propPrev == WBP_ALPHA || propPrev == WBP_ARABIC || 
                         propPrev == WBP_HEBREW || propPrev == WBP_INDIC ||
                         propPrev == WBP_SE_ASIAN || propPrev == WBP_TIBETAN ||
                         propPrev == WBP_ETHIOPIC || propPrev == WBP_MONGOLIAN);
    bool currIsLetter = (propCurr == WBP_ALPHA || propCurr == WBP_ARABIC || 
                         propCurr == WBP_HEBREW || propCurr == WBP_INDIC ||
                         propCurr == WBP_SE_ASIAN || propCurr == WBP_TIBETAN ||
                         propCurr == WBP_ETHIOPIC || propCurr == WBP_MONGOLIAN);
    
    if (prevIsLetter && currIsLetter) {
        return false;
    }
    
    // WB7: Don't break within sequences of Katakana
    if (propPrev == WBP_KATAKANA && propCurr == WBP_KATAKANA) return false;
    
    // WB8: Don't break between letters and apostrophes
    if ((prevIsLetter && propCurr == WBP_SINGLE_QUOTE) ||
        (propPrev == WBP_SINGLE_QUOTE && currIsLetter)) {
        return false;
    }
    
    // WB9: Don't break between letters and link
    if ((prevIsLetter && propCurr == WBP_LINK) ||
        (propPrev == WBP_LINK && currIsLetter)) {
        return false;
    }
    
    // WB9a: Don't break between letter and mid-letter
    if ((prevIsLetter && propCurr == WBP_MID_LETTER) ||
        (propPrev == WBP_MID_LETTER && currIsLetter)) {
        return false;
    }
    
    // WB10: Don't break between numbers
    if (propPrev == WBP_NUMERIC && propCurr == WBP_NUMERIC) return false;
    
    // WB11: Don't break between letter and number
    if ((prevIsLetter && propCurr == WBP_NUMERIC) ||
        (propPrev == WBP_NUMERIC && currIsLetter)) {
        return false;
    }
    
    // Special: Indic consonant clusters - don't break within clusters
    if (propPrev == WBP_INDIC && propCurr == WBP_INDIC) {
        UErrorCode status = U_ZERO_ERROR;
        UScriptCode prevScript = uscript_getScript(prev, &status);
        UScriptCode currScript = uscript_getScript(curr, &status);
        
        // Check if both are in the same Indic script
        if (isIndicScript(prevScript) && isIndicScript(currScript)) {
            // For Indic, consider characters to be part of the same word
            // unless there's explicit punctuation or space
            int32_t prevCat = getGeneralCategory(prev);
            int32_t currCat = getGeneralCategory(curr);
            
            // Allow break after combining marks, punctuation, etc.
            if ((prevCat == U_NON_SPACING_MARK || prevCat == U_ENCLOSING_MARK ||
                 prevCat == U_COMBINING_SPACING_MARK) &&
                (currCat >= U_UPPERCASE_LETTER && currCat <= U_OTHER_LETTER)) {
                // Don't break between combining mark and letter
                return false;
            }
            if ((prevCat >= U_UPPERCASE_LETTER && prevCat <= U_OTHER_LETTER) &&
                (currCat == U_NON_SPACING_MARK || currCat == U_ENCLOSING_MARK ||
                 currCat == U_COMBINING_SPACING_MARK)) {
                // Don't break between letter and combining mark
                return false;
            }
            // For Indic consonant clusters, we don't break
            if ((prevCat >= U_UPPERCASE_LETTER && prevCat <= U_OTHER_LETTER) &&
                (currCat >= U_UPPERCASE_LETTER && currCat <= U_OTHER_LETTER)) {
                return false;
            }
        }
    }
    
    // Special: Southeast Asian - don't break between letters (no word boundaries)
    if (propPrev == WBP_SE_ASIAN && propCurr == WBP_SE_ASIAN) {
        return false;
    }
    
    // Special: Tibetan - don't break between letters
    if (propPrev == WBP_TIBETAN && propCurr == WBP_TIBETAN) {
        return false;
    }
    
    // Special: Ethiopic - don't break between letters
    if (propPrev == WBP_ETHIOPIC && propCurr == WBP_ETHIOPIC) {
        return false;
    }
    
    // Special: Mongolian - don't break between letters
    if (propPrev == WBP_MONGOLIAN && propCurr == WBP_MONGOLIAN) {
        return false;
    }
    
    // WB12: CJK rule - break between ideographs
    if (propPrev == WBP_IDEOGRAPHIC && propCurr == WBP_IDEOGRAPHIC) return true;
    
    // WB13: CJK rule - break after ideograph before non-ideograph
    if (propPrev == WBP_IDEOGRAPHIC && propCurr != WBP_IDEOGRAPHIC) return true;
    
    // WB14: CJK rule - break before ideograph after non-ideograph
    if (propPrev != WBP_IDEOGRAPHIC && propCurr == WBP_IDEOGRAPHIC) return true;
    
    // Default: break
    return true;
}

// Check line boundary according to UAX #14 rules with script-specific handling
static UBool isLineBoundary(const UChar* text, int32_t length, int32_t pos) {
    if (pos <= 0 || pos >= length) {
        return true;
    }
    UChar32 prev = text[pos - 1];
    if (U16_IS_TRAIL(prev) && pos > 1) {
        prev = U16_GET_SUPPLEMENTARY(text[pos - 2], text[pos - 1]);
    }
    UChar32 curr = text[pos];
    if (U16_IS_LEAD(curr) && pos + 1 < length) {
        curr = U16_GET_SUPPLEMENTARY(text[pos], text[pos + 1]);
    }
    LBProperty propPrev = getLBProperty(prev);
    LBProperty propCurr = getLBProperty(curr);
    // LB1: Mandatory breaks
    if (propPrev == LBP_CR && propCurr != LBP_LF) return true;
    if (propPrev == LBP_LF || propPrev == LBP_BK || propPrev == LBP_NL) return true;
    
    // LB2: Never break at surrogate pairs
    if (U16_IS_LEAD(prev) && U16_IS_TRAIL(curr)) return false;
    
    // LB3: Don't break before/after ZWJ
    if (prev == 0x200D || curr == 0x200D) return false;
    
    // LB4: Zero-width space allows break
    if (prev == 0x200B || curr == 0x200B) return true;
    
    // LB5: Don't break between Hangul syllables
    if ((propPrev == LBP_JL && (propCurr == LBP_JL || propCurr == LBP_JV || propCurr == LBP_H2)) ||
        ((propPrev == LBP_H2 || propPrev == LBP_JV) && (propCurr == LBP_JV || propCurr == LBP_JT || propCurr == LBP_H3)) ||
        ((propPrev == LBP_H3 || propPrev == LBP_JT) && propCurr == LBP_JT)) {
        return false;
    }
    
    // Don't break between alphabetic characters
    if (propPrev == LBP_AL && propCurr == LBP_AL) return false;
    
    // Don't break between Arabic letters
    if (propPrev == LBP_ALM && propCurr == LBP_ALM) return false;
    
    // Don't break between Arabic and alphabetic
    if ((propPrev == LBP_ALM && propCurr == LBP_AL) ||
        (propPrev == LBP_AL && propCurr == LBP_ALM)) {
        return false;
    }
    
    // Don't break between Hebrew letters
    if (propPrev == LBP_HE && propCurr == LBP_HE) return false;
    
    // Don't break between Indic letters
    if (propPrev == LBP_IN && propCurr == LBP_IN) {
        return false;
    }
    
    // Don't break between Southeast Asian letters
    if (propPrev == LBP_SA && propCurr == LBP_SA) {
        return false;
    }
    
    // Don't break between alphabetic and numeric
    if ((propPrev == LBP_AL && propCurr == LBP_NU) ||
        (propPrev == LBP_NU && propCurr == LBP_AL)) {
        return false;
    }
    
    // Don't break between numeric characters
    if (propPrev == LBP_NU && propCurr == LBP_NU) return false;
    
    // LB6: Break after hyphens
    if (propPrev == LBP_HY) return true;
    
    // LB7: Break after whitespace
    if (propPrev == LBP_WS) return true;
    
    // LB8: Don't break before closing punctuation
    if (propCurr == LBP_CL) return false;
    
    // LB9: Don't break after opening punctuation
    if (propPrev == LBP_OP) return false;
    
    // LB12: Break between ideographic characters
    if (propPrev == LBP_ID && propCurr == LBP_ID) return true;
    
    // LB13: Break after ideograph before non-ideograph
    if (propPrev == LBP_ID && propCurr == LBP_AL) return true;
    
    // LB14: Break before ideograph after non-ideograph
    if (propPrev == LBP_AL && propCurr == LBP_ID) return true;
    
    // Break after symbols
    if (propPrev == LBP_SY) return true;
    
    // Break after exclamation/question
    if (propPrev == LBP_EX) return true;
    
    // Break after postfix numeric (period)
    if (propPrev == LBP_PO) return true;
    
    // Default: don't allow break (conservative)
    return false;
}

// Check sentence boundary
static UBool isSentenceBoundary(const UChar* text, int32_t length, int32_t pos) {
    if (pos <= 0 || pos >= length) {
        return true;
    }
    UChar32 prev = text[pos - 1];
    if (U16_IS_TRAIL(prev) && pos > 1) {
        prev = U16_GET_SUPPLEMENTARY(text[pos - 2], text[pos - 1]);
    }
    if (prev == '.' || prev == '?' || prev == '!' || prev == 0x2026 || prev == 0x203C) {
        int32_t skip = pos;
        while (skip < length) {
            UChar32 c = text[skip];
            if (U16_IS_LEAD(c) && skip + 1 < length) {
                c = U16_GET_SUPPLEMENTARY(text[skip], text[skip + 1]);
            }
            if (!(c == ')' || c == ']' || c == '}' || c == '"' || c == '\'' ||
                  c == 0x201D || c == 0x2019)) {
                break;
            }
            skip++;
        }
        if (skip >= length) {
            return true;
        }
        UChar32 next = text[skip];
        if (U16_IS_LEAD(next) && skip + 1 < length) {
            next = U16_GET_SUPPLEMENTARY(text[skip], text[skip + 1]);
        }
        int32_t cat = getGeneralCategory(next);
        if (cat == U_SPACE_SEPARATOR || cat == U_LINE_SEPARATOR || 
            cat == U_PARAGRAPH_SEPARATOR || next == '\n' || next == '\r') {
            return true;
        }
    }
    return false;
}

static UBool isTitleBoundary(const UChar* text, int32_t length, int32_t pos) {
    if (pos <= 0 || pos >= length) {
        return true;
    }
    if (isWordBoundary(text, length, pos)) {
        return true;
    }
    UChar32 curr = text[pos];
    if (U16_IS_LEAD(curr) && pos + 1 < length) {
        curr = U16_GET_SUPPLEMENTARY(text[pos], text[pos + 1]);
    }
    int32_t cat = getGeneralCategory(curr);
    if (cat == U_LOWERCASE_LETTER || cat == U_TITLECASE_LETTER) {
        UChar32 prev = text[pos - 1];
        if (U16_IS_TRAIL(prev) && pos > 1) {
            prev = U16_GET_SUPPLEMENTARY(text[pos - 2], text[pos - 1]);
        }
        int32_t prevCat = getGeneralCategory(prev);
        if (prevCat == U_UPPERCASE_LETTER || prevCat == U_TITLECASE_LETTER) {
            return true;
        }
    }
    return false;
}

U_CAPI UBreakIterator* U_EXPORT2 ubrk_open(int32_t type, const char* locale, const UChar* text, int32_t length, UErrorCode* status) {
    (void)locale;
    if (status == nullptr) {
        return nullptr;
    }
    if (*status > U_ZERO_ERROR) {
        return nullptr;
    }
    if (type < UBRK_CHARACTER || type > UBRK_TITLE) {
        *status = U_ILLEGAL_ARGUMENT_ERROR;
        return nullptr;
    }
    UBreakIterator* bi = (UBreakIterator*)malloc(sizeof(UBreakIterator));
    if (bi == nullptr) {
        *status = U_MEMORY_ALLOCATION_ERROR;
        return nullptr;
    }
    bi->text = text;
    bi->length = length;
    bi->current = 0;
    bi->type = type;
    return bi;
}

U_CAPI UBreakIterator* U_EXPORT2 ubrk_openUText(int32_t type, const char* locale, UText* text, UErrorCode* status) {
    if (text == nullptr) {
        return ubrk_open(type, locale, nullptr, 0, status);
    }
    return ubrk_open(type, locale, text->chars, text->length, status);
}

U_CAPI void U_EXPORT2 ubrk_close(UBreakIterator* bi) {
    if (bi != nullptr) {
        free(bi);
    }
}

U_CAPI int32_t U_EXPORT2 ubrk_first(UBreakIterator* bi) {
    if (bi == nullptr) return UBRK_DONE;
    bi->current = 0;
    return bi->current;
}

U_CAPI int32_t U_EXPORT2 ubrk_last(UBreakIterator* bi) {
    if (bi == nullptr) return UBRK_DONE;
    bi->current = bi->length;
    return bi->current;
}

U_CAPI int32_t U_EXPORT2 ubrk_next(UBreakIterator* bi) {
    if (bi == nullptr) return UBRK_DONE;
    if (bi->current >= bi->length) {
        return UBRK_DONE;
    }
    switch (bi->type) {
        case UBRK_CHARACTER: {
            UChar32 c = bi->text[bi->current];
            if (U16_IS_LEAD(c) && bi->current + 1 < bi->length && U16_IS_TRAIL(bi->text[bi->current + 1])) {
                bi->current += 2;
            } else {
                bi->current += 1;
            }
            break;
        }
        case UBRK_WORD: {
            bi->current++;
            while (bi->current < bi->length && !isWordBoundary(bi->text, bi->length, bi->current)) {
                UChar32 c = bi->text[bi->current];
                if (U16_IS_LEAD(c) && bi->current + 1 < bi->length && U16_IS_TRAIL(bi->text[bi->current + 1])) {
                    bi->current += 2;
                } else {
                    bi->current += 1;
                }
            }
            break;
        }
        case UBRK_LINE: {
            bi->current++;
            while (bi->current < bi->length && !isLineBoundary(bi->text, bi->length, bi->current)) {
                UChar32 c = bi->text[bi->current];
                if (U16_IS_LEAD(c) && bi->current + 1 < bi->length && U16_IS_TRAIL(bi->text[bi->current + 1])) {
                    bi->current += 2;
                } else {
                    bi->current += 1;
                }
            }
            break;
        }
        case UBRK_SENTENCE: {
            bi->current++;
            while (bi->current < bi->length && !isSentenceBoundary(bi->text, bi->length, bi->current)) {
                UChar32 c = bi->text[bi->current];
                if (U16_IS_LEAD(c) && bi->current + 1 < bi->length && U16_IS_TRAIL(bi->text[bi->current + 1])) {
                    bi->current += 2;
                } else {
                    bi->current += 1;
                }
            }
            break;
        }
        case UBRK_TITLE: {
            bi->current++;
            while (bi->current < bi->length && !isTitleBoundary(bi->text, bi->length, bi->current)) {
                UChar32 c = bi->text[bi->current];
                if (U16_IS_LEAD(c) && bi->current + 1 < bi->length && U16_IS_TRAIL(bi->text[bi->current + 1])) {
                    bi->current += 2;
                } else {
                    bi->current += 1;
                }
            }
            break;
        }
        default:
            bi->current++;
            break;
    }
    if (bi->current > bi->length) {
        return UBRK_DONE;
    }
    return bi->current;
}

U_CAPI int32_t U_EXPORT2 ubrk_previous(UBreakIterator* bi) {
    if (bi == nullptr) return UBRK_DONE;
    if (bi->current <= 0) {
        return UBRK_DONE;
    }
    switch (bi->type) {
        case UBRK_CHARACTER: {
            if (bi->current > 1 && U16_IS_TRAIL(bi->text[bi->current - 1]) && U16_IS_LEAD(bi->text[bi->current - 2])) {
                bi->current -= 2;
            } else {
                bi->current -= 1;
            }
            break;
        }
        case UBRK_WORD: {
            bi->current--;
            while (bi->current > 0 && !isWordBoundary(bi->text, bi->length, bi->current)) {
                bi->current--;
            }
            break;
        }
        case UBRK_LINE: {
            bi->current--;
            while (bi->current > 0 && !isLineBoundary(bi->text, bi->length, bi->current)) {
                bi->current--;
            }
            break;
        }
        case UBRK_SENTENCE: {
            bi->current--;
            while (bi->current > 0 && !isSentenceBoundary(bi->text, bi->length, bi->current)) {
                bi->current--;
            }
            break;
        }
        case UBRK_TITLE: {
            bi->current--;
            while (bi->current > 0 && !isTitleBoundary(bi->text, bi->length, bi->current)) {
                bi->current--;
            }
            break;
        }
        default:
            bi->current--;
            break;
    } 
    return bi->current;
}

U_CAPI int32_t U_EXPORT2 ubrk_current(UBreakIterator* bi) {
    if (bi == nullptr) return UBRK_DONE;
    if (bi->current >= bi->length) {
        return UBRK_DONE;
    }
    return bi->current;
}

U_CAPI int32_t U_EXPORT2 ubrk_preceding(UBreakIterator* bi, int32_t offset) {
    if (bi == nullptr || offset <= 0) return UBRK_DONE;
    if (offset > bi->length) {
        offset = bi->length;
    }
    bi->current = offset - 1;
    switch (bi->type) {
        case UBRK_WORD:
            while (bi->current > 0 && !isWordBoundary(bi->text, bi->length, bi->current)) {
                bi->current--;
            }
            break;
        case UBRK_LINE:
            while (bi->current > 0 && !isLineBoundary(bi->text, bi->length, bi->current)) {
                bi->current--;
            }
            break;
        case UBRK_SENTENCE:
            while (bi->current > 0 && !isSentenceBoundary(bi->text, bi->length, bi->current)) {
                bi->current--;
            }
            break;
        case UBRK_TITLE:
            while (bi->current > 0 && !isTitleBoundary(bi->text, bi->length, bi->current)) {
                bi->current--;
            }
            break;
        default:
            if (bi->current > 1 && U16_IS_TRAIL(bi->text[bi->current]) && U16_IS_LEAD(bi->text[bi->current - 1])) {
                bi->current--;
            }
            break;
    }
    return bi->current;
}

U_CAPI int32_t U_EXPORT2 ubrk_following(UBreakIterator* bi, int32_t offset) {
    if (bi == nullptr || offset < 0) return UBRK_DONE;
    if (offset >= bi->length) {
        bi->current = bi->length;
        return UBRK_DONE;
    }
    bi->current = offset;
    switch (bi->type) {
        case UBRK_WORD:
            while (bi->current < bi->length && !isWordBoundary(bi->text, bi->length, bi->current)) {
                bi->current++;
            }
            if (bi->current < bi->length) {
                bi->current++;
                while (bi->current < bi->length && !isWordBoundary(bi->text, bi->length, bi->current)) {
                    bi->current++;
                }
            }
            break;
        case UBRK_LINE:
            while (bi->current < bi->length && !isLineBoundary(bi->text, bi->length, bi->current)) {
                bi->current++;
            }
            if (bi->current < bi->length) {
                bi->current++;
                while (bi->current < bi->length && !isLineBoundary(bi->text, bi->length, bi->current)) {
                    bi->current++;
                }
            }
            break;
        case UBRK_SENTENCE:
            while (bi->current < bi->length && !isSentenceBoundary(bi->text, bi->length, bi->current)) {
                bi->current++;
            }
            if (bi->current < bi->length) {
                bi->current++;
                while (bi->current < bi->length && !isSentenceBoundary(bi->text, bi->length, bi->current)) {
                    bi->current++;
                }
            }
            break;
        case UBRK_TITLE:
            while (bi->current < bi->length && !isTitleBoundary(bi->text, bi->length, bi->current)) {
                bi->current++;
            }
            if (bi->current < bi->length) {
                bi->current++;
                while (bi->current < bi->length && !isTitleBoundary(bi->text, bi->length, bi->current)) {
                    bi->current++;
                }
            }
            break;
        default:
            UChar32 c = bi->text[bi->current];
            if (U16_IS_LEAD(c) && bi->current + 1 < bi->length && U16_IS_TRAIL(bi->text[bi->current + 1])) {
                bi->current += 2;
            } else {
                bi->current += 1;
            }
            break;
    }
    
    if (bi->current > bi->length) {
        bi->current = bi->length;
        return UBRK_DONE;
    }
    return bi->current;
}

U_CAPI UBool U_EXPORT2 ubrk_isBoundary(UBreakIterator* bi, int32_t offset) {
    if (bi == nullptr || offset < 0 || offset > bi->length) {
        return false;
    }
    if (offset == 0 || offset == bi->length) {
        return true;
    }
    switch (bi->type) {
        case UBRK_CHARACTER: {
            if (offset > 0 && U16_IS_TRAIL(bi->text[offset - 1])) {
                return false;
            }
            return true;
        }
        case UBRK_WORD:
            return isWordBoundary(bi->text, bi->length, offset);
        case UBRK_LINE:
            return isLineBoundary(bi->text, bi->length, offset);
        case UBRK_SENTENCE:
            return isSentenceBoundary(bi->text, bi->length, offset);
        case UBRK_TITLE:
            return isTitleBoundary(bi->text, bi->length, offset);
        default:
            return true;
    }
}

U_CAPI void U_EXPORT2 ubrk_setUText(UBreakIterator* bi, UText* text, UErrorCode* status) {
    if (status == nullptr) {
        return;
    }
    if (*status > U_ZERO_ERROR) {
        return;
    }
    if (bi == nullptr) {
        *status = U_ILLEGAL_ARGUMENT_ERROR;
        return;
    }
    if (text == nullptr) {
        *status = U_ILLEGAL_ARGUMENT_ERROR;
        return;
    }
    bi->text = text->chars;
    bi->length = text->length;
    bi->current = 0;
}

U_CAPI UText* U_EXPORT2 utext_openUChars(UText* fillIn, const UChar* chars, int32_t length, UErrorCode* status) {
    if (status == nullptr) {
        return fillIn;
    }
    if (*status > U_ZERO_ERROR) {
        return fillIn;
    }
    UText* ut = fillIn;
    if (ut == nullptr) {
        ut = (UText*)malloc(sizeof(UText));
        if (ut == nullptr) {
            *status = U_MEMORY_ALLOCATION_ERROR;
            return nullptr;
        }
    }
    ut->chars = chars;
    ut->length = length;
    return ut;
}

U_CAPI void U_EXPORT2 utext_close(UText* ut) {
    if (ut != nullptr) {
        free(ut);
    }
}