#ifndef __UCHAR_H__
#define __UCHAR_H__
#include "unicode/utypes.h"

typedef enum UGraphemeClusterBreak {
    /*
     * Note: UGraphemeClusterBreak constants are parsed by preparseucd.py.
     * It matches lines like
     *     U_GCB_<Unicode Grapheme_Cluster_Break value name>
     */

    U_GCB_OTHER = 0,            /*[XX]*/
    U_GCB_CONTROL = 1,          /*[CN]*/
    U_GCB_CR = 2,               /*[CR]*/
    U_GCB_EXTEND = 3,           /*[EX]*/
    U_GCB_L = 4,                /*[L]*/
    U_GCB_LF = 5,               /*[LF]*/
    U_GCB_LV = 6,               /*[LV]*/
    U_GCB_LVT = 7,              /*[LVT]*/
    U_GCB_T = 8,                /*[T]*/
    U_GCB_V = 9,                /*[V]*/
    /** @stable ICU 4.0 */
    U_GCB_SPACING_MARK = 10,    /*[SM]*/
    /** @stable ICU 4.0 */
    U_GCB_PREPEND = 11,         /*[PP]*/
    /** @stable ICU 50 */
    U_GCB_REGIONAL_INDICATOR = 12,  /*[RI]*/
    /** @stable ICU 58 */
    U_GCB_E_BASE = 13,          /*[EB]*/
    /** @stable ICU 58 */
    U_GCB_E_BASE_GAZ = 14,      /*[EBG]*/
    /** @stable ICU 58 */
    U_GCB_E_MODIFIER = 15,      /*[EM]*/
    /** @stable ICU 58 */
    U_GCB_GLUE_AFTER_ZWJ = 16,  /*[GAZ]*/
    /** @stable ICU 58 */
    U_GCB_ZWJ = 17,             /*[ZWJ]*/

#ifndef U_HIDE_DEPRECATED_API
    /**
     * One more than the highest normal UGraphemeClusterBreak value.
     * The highest value is available via u_getIntPropertyMaxValue(UCHAR_GRAPHEME_CLUSTER_BREAK).
     *
     * @deprecated ICU 58 The numeric value may change over time, see ICU ticket #12420.
     */
    U_GCB_COUNT = 18
#endif  // U_HIDE_DEPRECATED_API
} UGraphemeClusterBreak;

typedef enum UProperty {
    // 通用属性
    UCHAR_GENERAL_CATEGORY = 0,
    UCHAR_CANONICAL_COMBINING_CLASS = 3,
    UCHAR_BIDI_CLASS = 4,
    
    // 双向属性
    UCHAR_BIDI_PAIRED_BRACKET_TYPE = 7,
    UCHAR_BIDI_PAIRED_BRACKET = 8,
    
    // 脚本属性
    UCHAR_SCRIPT = 10,
    
    // 行断属性
    UCHAR_LINE_BREAK = 13,
    
    // 连接类型
    UCHAR_JOINING_TYPE = 14,
    UCHAR_JOINING_GROUP = 15,
    
    // 分解类型
    UCHAR_DECOMPOSITION_TYPE = 18,
    
    // 数值属性
    UCHAR_NUMERIC_VALUE = 21,
    
    // 大小写映射
    UCHAR_LOWERCASE_MAPPING = 24,
    UCHAR_UPPERCASE_MAPPING = 25,
    UCHAR_TITLECASE_MAPPING = 26,
    
    // 字符名称
    UCHAR_NAME = 28,
    UCHAR_NAME_ALIAS = 29,
    
    // 块属性
    UCHAR_BLOCK = 30,
    
    // 字素聚类
    UCHAR_GRAPHEME_CLUSTER_BREAK = 41,
    
    // 词断属性
    UCHAR_WORD_BREAK = 42,
    
    // 句子断属性
    UCHAR_SENTENCE_BREAK = 43,
    
    // 二进制属性起始
    UCHAR_BINARY_START = 0x100,
    UCHAR_WHITE_SPACE = 0x100,
    UCHAR_BIDI_MIRRORED = 0x101,
    UCHAR_ALPHABETIC = 0x102,
    UCHAR_NUMERIC = 0x103,
    UCHAR_BIDI_CONTROL = 0x104,
    UCHAR_JOIN_CONTROL = 0x105,
    UCHAR_DASH = 0x106,
    UCHAR_HYPHEN = 0x107,
    UCHAR_LOWERCASE = 0x108,
    UCHAR_UPPERCASE = 0x109,
    UCHAR_TITLECASE = 0x10A,
    UCHAR_IDEOGRAPHIC = 0x10B,
    UCHAR_DIACRITIC = 0x10C,
    UCHAR_EXTENDER = 0x10D,
    UCHAR_BREAK_WHITESPACE = 0x10E,
    UCHAR_HEX_DIGIT = 0x10F,
    UCHAR_ASCII_HEX_DIGIT = 0x110,
    UCHAR_ALPHANUMERIC = 0x111,
    UCHAR_PUNCTUATION = 0x112,
    UCHAR_MATH = 0x113,
    UCHAR_LOWERCASE_LETTER = 0x114,
    UCHAR_UPPERCASE_LETTER = 0x115,
    UCHAR_IDEOGRAPHIC_LETTER = 0x116,
    UCHAR_NONCHARACTER_CODE_POINT = 0x117,
    UCHAR_DEFAULT_IGNORABLE_CODE_POINT = 0x118,
    UCHAR_DEPRECATED = 0x119,
    UCHAR_SOFTHYPHEN = 0x11A,
    UCHAR_QUOTATION_MARK = 0x11B,
    UCHAR_TERMINAL_PUNCTUATION = 0x11C,
    UCHAR_SEGMENT_STARTER = 0x11D,
    UCHAR_XID_START = 0x11E,
    UCHAR_XID_CONTINUE = 0x11F,
    UCHAR_GRAPHEME_EXTEND = 0x120,
    UCHAR_GRAPHEME_LINK = 0x121,
    UCHAR_IDS_BINARY_OPERATOR = 0x122,
    UCHAR_IDS_TRINARY_OPERATOR = 0x123,
    UCHAR_RADICAL = 0x124,
    UCHAR_UNIFIED_IDEOGRAPH = 0x125,
    UCHAR_VARIATION_SELECTOR = 0x126,
    UCHAR_PATTERN_SYNTAX = 0x127,
    UCHAR_PATTERN_WHITE_SPACE = 0x128,
    UCHAR_PREPENDED_CONCATENATION_MARK = 0x129,
    
    // Emoji 属性
    UCHAR_EMOJI = 148,
    UCHAR_EMOJI_MODIFIER = 149,
    UCHAR_EMOJI_MODIFIER_BASE = 150,
    UCHAR_EXTENDED_PICTOGRAPHIC = 151,
} UProperty;

typedef enum UCharCategory {
    U_UNASSIGNED = 0,
    U_UPPERCASE_LETTER = 1,
    U_LOWERCASE_LETTER = 2,
    U_TITLECASE_LETTER = 3,
    U_MODIFIER_LETTER = 4,
    U_OTHER_LETTER = 5,
    U_NON_SPACING_MARK = 6,
    U_ENCLOSING_MARK = 7,
    U_COMBINING_SPACING_MARK = 8,
    U_DECIMAL_DIGIT_NUMBER = 9,
    U_LETTER_NUMBER = 10,
    U_OTHER_NUMBER = 11,
    U_SPACE_SEPARATOR = 12,
    U_LINE_SEPARATOR = 13,
    U_PARAGRAPH_SEPARATOR = 14,
    U_CONTROL = 15,
    U_FORMAT = 16,
    U_PRIVATE_USE = 17,
    U_SURROGATE = 18,
    U_DASH_PUNCTUATION = 19,
    U_OPEN_PUNCTUATION = 20,
    U_CLOSE_PUNCTUATION = 21,
    U_CONNECTOR_PUNCTUATION = 22,
    U_OTHER_PUNCTUATION = 23,
    U_MATH_SYMBOL = 24,
    U_CURRENCY_SYMBOL = 25,
    U_MODIFIER_SYMBOL = 26,
    U_OTHER_SYMBOL = 27,
    U_INITIAL_PUNCTUATION = 28,
    U_FINAL_PUNCTUATION = 29,
} UCharCategory;

typedef enum UJoiningType {
    /*
     * Note: UJoiningType constants are parsed by preparseucd.py.
     * It matches lines like
     *     U_JT_<Unicode Joining_Type value name>
     */

    U_JT_NON_JOINING,       /*[U]*/
    U_JT_JOIN_CAUSING,      /*[C]*/
    U_JT_DUAL_JOINING,      /*[D]*/
    U_JT_LEFT_JOINING,      /*[L]*/
    U_JT_RIGHT_JOINING,     /*[R]*/
    U_JT_TRANSPARENT,       /*[T]*/
#ifndef U_HIDE_DEPRECATED_API
    /**
     * One more than the highest normal UJoiningType value.
     * The highest value is available via u_getIntPropertyMaxValue(UCHAR_JOINING_TYPE).
     *
     * @deprecated ICU 58 The numeric value may change over time, see ICU ticket #12420.
     */
    U_JT_COUNT /* 6 */
#endif  // U_HIDE_DEPRECATED_API
} UJoiningType;

typedef enum ULineBreak {
    U_LB_UNKNOWN = 0,           /*[XX]*/
    U_LB_AMBIGUOUS = 1,         /*[AI]*/
    U_LB_ALPHABETIC = 2,        /*[AL]*/
    U_LB_BREAK_BOTH = 3,        /*[B2]*/
    U_LB_BREAK_AFTER = 4,       /*[BA]*/
    U_LB_BREAK_BEFORE = 5,      /*[BB]*/
    U_LB_MANDATORY_BREAK = 6,   /*[BK]*/
    U_LB_CONTINGENT_BREAK = 7,  /*[CB]*/
    U_LB_CLOSE_PUNCTUATION = 8, /*[CL]*/
    U_LB_COMBINING_MARK = 9,    /*[CM]*/
    U_LB_CARRIAGE_RETURN = 10,  /*[CR]*/
    U_LB_EXCLAMATION = 11,      /*[EX]*/
    U_LB_GLUE = 12,             /*[GL]*/
    U_LB_HYPHEN = 13,           /*[HY]*/
    U_LB_IDEOGRAPHIC = 14,      /*[ID]*/
    U_LB_INSEPARABLE = 15,      /*[IN]*/
    U_LB_INSEPERABLE = U_LB_INSEPARABLE,
    U_LB_INFIX_NUMERIC = 16,    /*[IS]*/
    U_LB_LINE_FEED = 17,        /*[LF]*/
    U_LB_NONSTARTER = 18,       /*[NS]*/
    U_LB_NUMERIC = 19,          /*[NU]*/
    U_LB_OPEN_PUNCTUATION = 20, /*[OP]*/
    U_LB_POSTFIX_NUMERIC = 21,  /*[PO]*/
    U_LB_PREFIX_NUMERIC = 22,   /*[PR]*/
    U_LB_QUOTATION = 23,        /*[QU]*/
    U_LB_COMPLEX_CONTEXT = 24,  /*[SA]*/
    U_LB_SURROGATE = 25,        /*[SG]*/
    U_LB_SPACE = 26,            /*[SP]*/
    U_LB_BREAK_SYMBOLS = 27,    /*[SY]*/
    U_LB_ZWSPACE = 28,          /*[ZW]*/
    U_LB_NEXT_LINE = 29,        /*[NL]*/
    U_LB_WORD_JOINER = 30,      /*[WJ]*/
    U_LB_H2 = 31,               /*[H2]*/
    U_LB_H3 = 32,               /*[H3]*/
    U_LB_JL = 33,               /*[JL]*/
    U_LB_JV = 34,               /*[JV]*/
    U_LB_JT = 35,               /*[JT]*/
    U_LB_LV = 36,               /*[LV]*/
    U_LB_LVT = 37,              /*[LVT]*/
    U_LB_CLOSE_PARENTHESIS = 38,/*[CP]*/
    U_LB_CONDITIONAL_JAPANESE_STARTER = 39,/*[CJ]*/
    U_LB_HEBREW_LETTER = 40,    /*[HL]*/
    U_LB_REGIONAL_INDICATOR = 41,/*[RI]*/
    U_LB_E_BASE = 42,           /*[EB]*/
    U_LB_E_MODIFIER = 43,       /*[EM]*/
    U_LB_ZWJ = 44,              /*[ZWJ]*/
    U_LB_AKSARA = 45,           /*[AK]*/
    U_LB_AKSARA_PREBASE = 46,   /*[AP]*/
    U_LB_AKSARA_START = 47,     /*[AS]*/
    U_LB_VIRAMA_FINAL = 48,     /*[VF]*/
    U_LB_VIRAMA = 49,           /*[VI]*/
    U_LB_UNAMBIGUOUS_HYPHEN = 50,/*[HH]*/
    U_LB_ARABIC_LETTER = 51,    /*[AL]*/
    U_LB_COUNT = 52
} ULineBreak;

#define UCHAR_JOINING_TYPE 14
#define UCHAR_LINE_BREAK 13

#define U_GET_GC_MASK(c) (1 << u_charType(c))

#define U_GC_ZS_MASK (1 << U_SPACE_SEPARATOR)
#define U_GC_P_MASK (1 << U_ENCLOSING_MARK)
#define U_GC_CC_MASK (1 << U_CONTROL)
#define U_GC_M_MASK ((1 << U_NON_SPACING_MARK) | (1 << U_ENCLOSING_MARK) | (1 << U_COMBINING_SPACING_MARK))

typedef enum UCharDirection {
    /*
     * Note: UCharDirection constants and their API comments are parsed by preparseucd.py.
     * It matches pairs of lines like
     *     / ** <Unicode 1..3-letter Bidi_Class value> comment... * /
     *     U_<[A-Z_]+> = <integer>,
     */

    /** L @stable ICU 2.0 */
    U_LEFT_TO_RIGHT               = 0,
    /** R @stable ICU 2.0 */
    U_RIGHT_TO_LEFT               = 1,
    /** EN @stable ICU 2.0 */
    U_EUROPEAN_NUMBER             = 2,
    /** ES @stable ICU 2.0 */
    U_EUROPEAN_NUMBER_SEPARATOR   = 3,
    /** ET @stable ICU 2.0 */
    U_EUROPEAN_NUMBER_TERMINATOR  = 4,
    /** AN @stable ICU 2.0 */
    U_ARABIC_NUMBER               = 5,
    /** CS @stable ICU 2.0 */
    U_COMMON_NUMBER_SEPARATOR     = 6,
    /** B @stable ICU 2.0 */
    U_BLOCK_SEPARATOR             = 7,
    /** S @stable ICU 2.0 */
    U_SEGMENT_SEPARATOR           = 8,
    /** WS @stable ICU 2.0 */
    U_WHITE_SPACE_NEUTRAL         = 9,
    /** ON @stable ICU 2.0 */
    U_OTHER_NEUTRAL               = 10,
    /** LRE @stable ICU 2.0 */
    U_LEFT_TO_RIGHT_EMBEDDING     = 11,
    /** LRO @stable ICU 2.0 */
    U_LEFT_TO_RIGHT_OVERRIDE      = 12,
    /** AL @stable ICU 2.0 */
    U_RIGHT_TO_LEFT_ARABIC        = 13,
    /** RLE @stable ICU 2.0 */
    U_RIGHT_TO_LEFT_EMBEDDING     = 14,
    /** RLO @stable ICU 2.0 */
    U_RIGHT_TO_LEFT_OVERRIDE      = 15,
    /** PDF @stable ICU 2.0 */
    U_POP_DIRECTIONAL_FORMAT      = 16,
    /** NSM @stable ICU 2.0 */
    U_DIR_NON_SPACING_MARK        = 17,
    /** BN @stable ICU 2.0 */
    U_BOUNDARY_NEUTRAL            = 18,
    /** FSI @stable ICU 52 */
    U_FIRST_STRONG_ISOLATE        = 19,
    /** LRI @stable ICU 52 */
    U_LEFT_TO_RIGHT_ISOLATE       = 20,
    /** RLI @stable ICU 52 */
    U_RIGHT_TO_LEFT_ISOLATE       = 21,
    /** PDI @stable ICU 52 */
    U_POP_DIRECTIONAL_ISOLATE     = 22,
#ifndef U_HIDE_DEPRECATED_API
    /**
     * One more than the highest UCharDirection value.
     * The highest value is available via u_getIntPropertyMaxValue(UCHAR_BIDI_CLASS).
     *
     * @deprecated ICU 58 The numeric value may change over time, see ICU ticket #12420.
     */
    U_CHAR_DIRECTION_COUNT
#endif  // U_HIDE_DEPRECATED_API
} UCharDirection;


U_CAPI UBool U_EXPORT2
u_isbase(UChar32 c);

U_CAPI UCharDirection U_EXPORT2
u_charDirection(UChar32 c);

U_CAPI UBool U_EXPORT2
u_isMirrored(UChar32 c);

U_CAPI UChar32 U_EXPORT2
u_charMirror(UChar32 c);

U_CAPI UChar32 U_EXPORT2
u_getBidiPairedBracket(UChar32 c);

U_CAPI int8_t U_EXPORT2
u_charType(UChar32 c);

U_CAPI int32_t U_EXPORT2
u_getIntPropertyValue(UChar32 c, uint32_t property);

U_CAPI UBool U_EXPORT2
u_hasBinaryProperty(UChar32 c, uint32_t property);

U_CAPI UBool U_EXPORT2
u_iscntrl(UChar32 c);

U_CAPI UBool U_EXPORT2
u_isdigit(UChar32 c);

U_CAPI UBool U_EXPORT2
u_isWhitespace(UChar32 c);

U_CAPI int32_t U_EXPORT2
u_getCombiningClass(UChar32 c);

U_CAPI UChar32 U_EXPORT2
u_tolower(UChar32 c);
U_CAPI UChar32 U_EXPORT2
u_toupper(UChar32 c);
#endif
