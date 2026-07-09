#include "unicode/ubrk.h"
#include "unicode/utf16.h"
#include "unicode/uchar.h"
#include "unicode/uscript.h"
#include "unicode_range_data.h"
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
    WBP_E_BASE,
    WBP_E_MODIFIER,
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

// ============================================================================
// 使用 UnicodeRange 数据表实现词和行断行属性查询
// ============================================================================

// 辅助函数：判断是否是 Indic 脚本
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

// 辅助函数：判断是否是东南亚脚本
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

// 辅助函数：获取通用类别
static inline int32_t getGeneralCategory(UChar32 c) {
    return u_getIntPropertyValue(c, UCHAR_GENERAL_CATEGORY);
}

// 获取词断行属性
static WBProperty getWBProperty(UChar32 c) {
    // Derive WBP from the table (category + script) — no hardcoded ASCII cases:
    // findUnicodeRange now returns correct category/script for basic Latin, so the
    // cat checks below (SPACE_SEPARATOR→BREAK, UPPERCASE/LOWERCASE_LETTER→ALPHA,
    // DECIMAL_DIGIT_NUMBER→NUMERIC) and the script switch handle ASCII correctly.
    const auto* range = findUnicodeRange(c);

    // 特殊字符优先处理
    if (c == '\r') return WBP_CR;
    if (c == '\n') return WBP_LF;
    // UAX#29 WBP Newline: VT(000B), FF(000C), NEL(0085), LS(2028), PS(2029) — break around.
    if (c == 0x000B || c == 0x000C || c == 0x0085 || c == 0x2028 || c == 0x2029) return WBP_NEWLINE;
    if (c == 0x200D) return WBP_ZWJ;  // ZWJ
    if (c == '\'' || c == 0x2019) return WBP_SINGLE_QUOTE;
    if (c == '"' || c == 0x201D) return WBP_DOUBLE_QUOTE;
    if (c == '_') return WBP_LINK;
    // UAX#29 Word_Break for ASCII punctuation (verified against UCD WordBreakProperty.txt):
    //   ':' Colon = MidLetter; ',' Comma, ';' Semicolon = MidNum; '.' Full Stop = MidNumLet.
    //   '-' Hyphen-Minus and '/' Solidus are WBP_Other (not listed in UCD) → fall through.
    if (c == ':') return WBP_MID_LETTER;
    if (c == ',' || c == ';') return WBP_MID_NUM;
    if (c == '.') return WBP_MID_NUMLET;
    
    // 使用数据表中的属性
    uint8_t cat = range->category;
    uint8_t script = range->script;
    uint8_t gcb = range->gcb;
    
    // 空白和分隔符
    if (cat == U_SPACE_SEPARATOR || cat == U_LINE_SEPARATOR || 
        cat == U_PARAGRAPH_SEPARATOR || cat == U_CONTROL) {
        return WBP_BREAK;
    }
    
    // 标记 - Extend 类别（包括组合标记）
    if (cat == U_NON_SPACING_MARK || cat == U_ENCLOSING_MARK || 
        cat == U_COMBINING_SPACING_MARK || gcb == U_GCB_EXTEND) {
        return WBP_EXTEND;
    }
    
    // 格式字符
    if (cat == U_FORMAT) {
        return WBP_FORMAT;
    }
    
    // 特殊脚本处理
    switch (script) {
        case USCRIPT_ARABIC:
            // Arabic/Persian letters with joining behavior
            return WBP_ARABIC;
            
        case USCRIPT_HEBREW:
            // Hebrew letters with joining behavior
            return WBP_HEBREW;
            
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
            // Indic scripts
            return WBP_INDIC;
            
        case USCRIPT_THAI:
        case USCRIPT_LAO:
        case USCRIPT_KHMER:
        case USCRIPT_MYANMAR:
            // Southeast Asian
            return WBP_SE_ASIAN;
            
        case USCRIPT_TIBETAN:
            return WBP_TIBETAN;
            
        case USCRIPT_ETHIOPIC:
            return WBP_ETHIOPIC;
            
        case USCRIPT_MONGOLIAN:
            return WBP_MONGOLIAN;
            
        case USCRIPT_KATAKANA:
        case USCRIPT_KATAKANA_OR_HIRAGANA:
            return WBP_KATAKANA;
            
        case USCRIPT_HAN:
            return WBP_IDEOGRAPHIC;
            
        default:
            break;
    }
    
    // 字母
    if (cat >= U_UPPERCASE_LETTER && cat <= U_OTHER_LETTER) {
        return WBP_ALPHA;
    }
    
    // 数字
    if (cat == U_DECIMAL_DIGIT_NUMBER) {
        return WBP_NUMERIC;
    }
    
    return WBP_OTHER;
}

// 获取行断行属性
static LBProperty getLBProperty(UChar32 c) {
    const auto* range = findUnicodeRange(c);
    
    // 特殊字符优先处理
    if (c == '\r') return LBP_CR;
    if (c == '\n') return LBP_LF;
    if (c == ' ') return LBP_WS;
    if (c == '\t') return LBP_WS;
    
    uint8_t cat = range->category;
    uint8_t script = range->script;
    uint8_t lb = range->lb;
    
    // 使用数据表中的行断行属性
    if (lb != U_LB_UNKNOWN && lb != U_LB_AMBIGUOUS) {
        // 将 ULineBreak 转换为 LBProperty
        switch (lb) {
            case U_LB_ALPHABETIC: return LBP_AL;
            case U_LB_NUMERIC: return LBP_NU;
            case U_LB_IDEOGRAPHIC: return LBP_ID;
            case U_LB_INSEPARABLE: return LBP_INS;
            case U_LB_HYPHEN: return LBP_HY;
            case U_LB_BREAK_AFTER: return LBP_BA;
            case U_LB_BREAK_BEFORE: return LBP_BB;
            case U_LB_BREAK_BOTH: return LBP_B2;
            case U_LB_ZWSPACE: return LBP_ZW;
            case U_LB_COMBINING_MARK: return LBP_CM;
            case U_LB_WORD_JOINER: return LBP_WJ;
            case U_LB_LV: return LBP_H2;
            case U_LB_LVT: return LBP_H3;
            case U_LB_JL: return LBP_JL;
            case U_LB_JV: return LBP_JV;
            case U_LB_JT: return LBP_JT;
            case U_LB_CONTINGENT_BREAK: return LBP_CN;
            case U_LB_CARRIAGE_RETURN: return LBP_CR;
            case U_LB_LINE_FEED: return LBP_LF;
            case U_LB_NEXT_LINE: return LBP_NL;
            case U_LB_MANDATORY_BREAK: return LBP_BK;
            case U_LB_COMPLEX_CONTEXT: return LBP_SA;
            case U_LB_HEBREW_LETTER: return LBP_HE;
            case U_LB_ARABIC_LETTER: return LBP_ALM;
            case U_LB_SPACE: return LBP_WS;
            case U_LB_BREAK_SYMBOLS: return LBP_SY;
            case U_LB_INFIX_NUMERIC: return LBP_IS;
            case U_LB_OPEN_PUNCTUATION: return LBP_OP;
            case U_LB_CLOSE_PUNCTUATION: return LBP_CL;
            case U_LB_QUOTATION: return LBP_QU;
            case U_LB_EXCLAMATION: return LBP_EX;
            case U_LB_PREFIX_NUMERIC: return LBP_PR;
            case U_LB_POSTFIX_NUMERIC: return LBP_PO;
            default:
                break;
        }
    }
    
    // 特殊脚本处理（仅当数据表中没有明确指定 lb 属性时使用）
    // 注意：大部分脚本的 lb 属性已经在数据表中定义了，这里只处理特殊情况
    if (lb == U_LB_UNKNOWN || lb == U_LB_AMBIGUOUS) {
        switch (script) {
            case USCRIPT_ARABIC:
                return LBP_ALM;
                
            case USCRIPT_HEBREW:
                return LBP_HE;
                
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
                return LBP_IN;
                
            case USCRIPT_THAI:
            case USCRIPT_LAO:
            case USCRIPT_KHMER:
            case USCRIPT_MYANMAR:
                return LBP_SA;
                
            default:
                break;
        }
    }
    
    // 字母
    if (cat >= U_UPPERCASE_LETTER && cat <= U_OTHER_LETTER) {
        return LBP_AL;
    }
    
    // 数字
    if (cat == U_DECIMAL_DIGIT_NUMBER) {
        return LBP_NU;
    }
    
    // CJK 统一表意文字
    if (script == USCRIPT_HAN) {
        return LBP_ID;
    }
    
    // Hangul 音节
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
    
    // 标点符号
    if (c == '(' || c == '[' || c == '{' || c == '<') return LBP_OP;
    if (c == ')' || c == ']' || c == '}' || c == '>') return LBP_CL;
    if (c == '"' || c == '\'' || c == 0x201C || c == 0x201D ||
        c == 0x2018 || c == 0x2019) return LBP_QU;
    if (c == '!' || c == '?') return LBP_EX;
    if (c == '-' || c == 0x2010 || c == 0x2011) return LBP_HY;
    
    // 符号
    if (c == ',' || c == ';' || c == ':' || c == '/') {
        return LBP_SY;
    }
    if (c == '.') return LBP_PO;
    
    return LBP_AL;
}

// ============================================================================
// WORD BREAK 规则实现（UAX #29）
// ============================================================================

// 判断是否是字母类（用于 WB6-WB9 规则）
static inline bool isLetterLike(WBProperty prop) {
    return prop == WBP_ALPHA || prop == WBP_ARABIC || prop == WBP_HEBREW || 
           prop == WBP_INDIC || prop == WBP_SE_ASIAN || prop == WBP_TIBETAN ||
           prop == WBP_ETHIOPIC || prop == WBP_MONGOLIAN;
}

// WB4 helper: walk backward from `pos` over Extend/Format/ZWJ to the previous base code point.
// Returns the base char, or (UChar32)-1 if none (reached start through only-ignorable chars).
static UChar32 prevBaseChar(const UChar* text, int32_t length, int32_t pos, int32_t* basePos) {
    int32_t i = pos;
    while (i > 0) {
        UChar32 c = text[i - 1];
        int32_t cl = 1;
        if (U16_IS_TRAIL(c) && i > 1 && U16_IS_LEAD(text[i - 2])) {
            c = U16_GET_SUPPLEMENTARY(text[i - 2], text[i - 1]);
            cl = 2;
        }
        WBProperty p = getWBProperty(c);
        if (p != WBP_EXTEND && p != WBP_FORMAT && p != WBP_ZWJ) {
            *basePos = i - cl;
            return c;
        }
        i -= cl;
    }
    return (UChar32)-1;
}

// WB4 helper: walk forward from `pos` over Extend/Format/ZWJ to the next base code point.
static UChar32 nextBaseChar(const UChar* text, int32_t length, int32_t pos, int32_t* basePos) {
    int32_t i = pos;
    while (i < length) {
        UChar32 c = text[i];
        int32_t cl = 1;
        if (U16_IS_LEAD(c) && i + 1 < length && U16_IS_TRAIL(text[i + 1])) {
            c = U16_GET_SUPPLEMENTARY(text[i], text[i + 1]);
            cl = 2;
        }
        WBProperty p = getWBProperty(c);
        if (p != WBP_EXTEND && p != WBP_FORMAT && p != WBP_ZWJ) {
            *basePos = i;
            return c;
        }
        i += cl;
    }
    return (UChar32)-1;
}

// Check word boundary according to UAX #29 rules with script-specific handling
static UBool isWordBoundary(const UChar* text, int32_t length, int32_t pos) {
    if (pos <= 0 || pos >= length) {
        return true;
    }
    
    // 获取前后字符
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
    
    // 获取属性（使用数据表）
    WBProperty propPrev = getWBProperty(prev);
    WBProperty propCurr = getWBProperty(curr);

    // WB1: Break at start and end of text
    if (pos == 0 || pos == length) return true;
    
    // WB3: CR × LF — keep CRLF together (do not break between CR and LF).
    if (propPrev == WBP_CR && propCurr == WBP_LF) return false;
    // WB3a/b: Break before/after newlines (CR, LF, Newline).
    if (propPrev == WBP_CR || propPrev == WBP_LF || propPrev == WBP_NEWLINE) return true;
    
    // WB4: Don't break within surrogate pairs
    if (U16_IS_LEAD(prev) && U16_IS_TRAIL(curr)) return false;

    // WB4 (× Extend/Format/ZWJ): an Extend/Format/ZWJ attaches to the preceding char, so never
    // break BEFORE one. (The boundary, if any, is decided at the position after the run.)
    if (propCurr == WBP_EXTEND || propCurr == WBP_FORMAT || propCurr == WBP_ZWJ) return false;
    // WB4: if prev is Extend/Format/ZWJ, evaluate the rules using the base char before the run.
    if (propPrev == WBP_EXTEND || propPrev == WBP_FORMAT || propPrev == WBP_ZWJ) {
        int32_t pp = 0;
        UChar32 pb = prevBaseChar(text, length, pos, &pp);
        if (pb == (UChar32)-1) return true;  // only ignorables before pos → break
        prev = pb;
        propPrev = getWBProperty(prev);
    }
    
    // WB6: Don't break between letters（使用辅助函数简化）
    bool prevIsLetter = isLetterLike(propPrev);
    bool currIsLetter = isLetterLike(propCurr);
    if (prevIsLetter && currIsLetter) {
        return false;
    }
    
    // WB7: Don't break within sequences of Katakana
    if (propPrev == WBP_KATAKANA && propCurr == WBP_KATAKANA) return false;
    
    // WB10: Don't break between numbers
    if (propPrev == WBP_NUMERIC && propCurr == WBP_NUMERIC) return false;

    // WB11: Don't break between letter and number
    if ((prevIsLetter && propCurr == WBP_NUMERIC) ||
        (propPrev == WBP_NUMERIC && currIsLetter)) {
        return false;
    }

    // WB6/7 + MidNum/MidNumLet: a MidChar only prevents a break when it sits BETWEEN two matching
    // chars (ALetter×MidLetter×ALetter, Numeric×MidNum×Numeric, X×MidNumLet×X, X×SingleQuote×X,
    // X×ExtendNumLet(Link)×X). The previous pairwise rules over-bound when the MidChar was at the
    // end. Look at the char on the OTHER side of the MidChar (Extend/Format/ZWJ-skipping).
    auto nextProp = [&](int32_t from) -> WBProperty {
        int32_t np;
        UChar32 n = nextBaseChar(text, length, from, &np);
        return (n == (UChar32)-1) ? WBP_OTHER : getWBProperty(n);
    };
    auto prevBaseProp = [&](int32_t from) -> WBProperty {
        int32_t pp;
        UChar32 p = prevBaseChar(text, length, from, &pp);
        return (p == (UChar32)-1) ? WBP_OTHER : getWBProperty(p);
    };
    const bool prevIsAlphaOrNum = prevIsLetter || propPrev == WBP_NUMERIC;
    const bool currIsAlphaOrNum = currIsLetter || propCurr == WBP_NUMERIC;
    // curr is the MidChar → check the char after it.
    if (propCurr == WBP_MID_LETTER || propCurr == WBP_SINGLE_QUOTE) {
        if (prevIsLetter && isLetterLike(nextProp(pos + currLen))) return false;
    } else if (propCurr == WBP_MID_NUM) {
        if (propPrev == WBP_NUMERIC && nextProp(pos + currLen) == WBP_NUMERIC) return false;
    } else if (propCurr == WBP_MID_NUMLET || propCurr == WBP_LINK) {
        if (prevIsAlphaOrNum && (isLetterLike(nextProp(pos + currLen)) || nextProp(pos + currLen) == WBP_NUMERIC))
            return false;
    }
    // prev is the MidChar → check the char before it.
    if (propPrev == WBP_MID_LETTER || propPrev == WBP_SINGLE_QUOTE) {
        if (currIsLetter && isLetterLike(prevBaseProp(pos - prevLen))) return false;
    } else if (propPrev == WBP_MID_NUM) {
        if (propCurr == WBP_NUMERIC && prevBaseProp(pos - prevLen) == WBP_NUMERIC) return false;
    } else if (propPrev == WBP_MID_NUMLET || propPrev == WBP_LINK) {
        if (currIsAlphaOrNum && (isLetterLike(prevBaseProp(pos - prevLen)) || prevBaseProp(pos - prevLen) == WBP_NUMERIC))
            return false;
    }

    // 特殊脚本处理：Indic 辅音丛（使用数据表，避免 ICU 调用）
    if (propPrev == WBP_INDIC && propCurr == WBP_INDIC) {
        const auto* rangePrev = findUnicodeRange(prev);
        const auto* rangeCurr = findUnicodeRange(curr);
        
        // 同一 Indic 脚本内的字母不分割
        if (rangePrev->script == rangeCurr->script &&
            isIndicScript((UScriptCode)rangePrev->script)) {
            uint8_t catPrev = rangePrev->category;
            uint8_t catCurr = rangeCurr->category;
            
            // 字母 + 标记不分割
            bool prevIsLet = (catPrev >= U_UPPERCASE_LETTER && catPrev <= U_OTHER_LETTER);
            bool currIsLet = (catCurr >= U_UPPERCASE_LETTER && catCurr <= U_OTHER_LETTER);
            bool prevIsMark = (catPrev == U_NON_SPACING_MARK || catPrev == U_ENCLOSING_MARK ||
                               catPrev == U_COMBINING_SPACING_MARK);
            bool currIsMark = (catCurr == U_NON_SPACING_MARK || catCurr == U_ENCLOSING_MARK ||
                               catCurr == U_COMBINING_SPACING_MARK);
            
            if ((prevIsMark && currIsLet) || (prevIsLet && currIsMark) ||
                (prevIsLet && currIsLet)) {
                return false;
            }
        }
    }
    
    // 特殊脚本：东南亚文字（泰文、老挝文等）不分割
    if (propPrev == WBP_SE_ASIAN && propCurr == WBP_SE_ASIAN) {
        return false;
    }
    
    // 特殊脚本：藏文、埃塞俄比亚文、蒙古文不分割
    if ((propPrev == WBP_TIBETAN && propCurr == WBP_TIBETAN) ||
        (propPrev == WBP_ETHIOPIC && propCurr == WBP_ETHIOPIC) ||
        (propPrev == WBP_MONGOLIAN && propCurr == WBP_MONGOLIAN)) {
        return false;
    }
    
    // WB12-WB14: CJK 规则
    if (propPrev == WBP_IDEOGRAPHIC && propCurr == WBP_IDEOGRAPHIC) return true;
    if (propPrev == WBP_IDEOGRAPHIC && propCurr != WBP_IDEOGRAPHIC) return true;
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
            // First boundary STRICTLY after offset (ICU ubrk_following semantics).
            // The previous "find boundary, step past, find next" skipped a boundary
            // (e.g. for offset 2 in "HELLO WORLD" it returned 6, not 5), which broke
            // word selection. Single loop from offset+1.
            bi->current++;
            while (bi->current < bi->length && !isWordBoundary(bi->text, bi->length, bi->current)) {
                bi->current++;
            }
            break;
        case UBRK_LINE:
            bi->current++;
            while (bi->current < bi->length && !isLineBoundary(bi->text, bi->length, bi->current)) {
                bi->current++;
            }
            break;
        case UBRK_SENTENCE:
            bi->current++;
            while (bi->current < bi->length && !isSentenceBoundary(bi->text, bi->length, bi->current)) {
                bi->current++;
            }
            break;
        case UBRK_TITLE:
            bi->current++;
            while (bi->current < bi->length && !isTitleBoundary(bi->text, bi->length, bi->current)) {
                bi->current++;
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
