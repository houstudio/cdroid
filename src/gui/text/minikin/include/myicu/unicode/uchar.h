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
    /* Binary properties start at 0 */
    UCHAR_ALPHABETIC=0,
    UCHAR_BINARY_START=UCHAR_ALPHABETIC,
    UCHAR_ASCII_HEX_DIGIT=1,
    UCHAR_BIDI_CONTROL=2,
    UCHAR_BIDI_MIRRORED=3,
    UCHAR_DASH=4,
    UCHAR_DEFAULT_IGNORABLE_CODE_POINT=5,
    UCHAR_DEPRECATED=6,
    UCHAR_DIACRITIC=7,
    UCHAR_EXTENDER=8,
    UCHAR_FULL_COMPOSITION_EXCLUSION=9,
    UCHAR_GRAPHEME_BASE=10,
    UCHAR_GRAPHEME_EXTEND=11,
    UCHAR_GRAPHEME_LINK=12,
    UCHAR_HEX_DIGIT=13,
    UCHAR_HYPHEN=14,
    UCHAR_ID_CONTINUE=15,
    UCHAR_ID_START=16,
    UCHAR_IDEOGRAPHIC=17,
    UCHAR_IDS_BINARY_OPERATOR=18,
    UCHAR_IDS_TRINARY_OPERATOR=19,
    UCHAR_JOIN_CONTROL=20,
    UCHAR_LOGICAL_ORDER_EXCEPTION=21,
    UCHAR_LOWERCASE=22,
    UCHAR_MATH=23,
    UCHAR_NONCHARACTER_CODE_POINT=24,
    UCHAR_QUOTATION_MARK=25,
    UCHAR_RADICAL=26,
    UCHAR_SOFT_DOTTED=27,
    UCHAR_TERMINAL_PUNCTUATION=28,
    UCHAR_UNIFIED_IDEOGRAPH=29,
    UCHAR_UPPERCASE=30,
    UCHAR_WHITE_SPACE=31,
    UCHAR_XID_CONTINUE=32,
    UCHAR_XID_START=33,
    UCHAR_CASE_SENSITIVE=34,
    UCHAR_S_TERM=35,
    UCHAR_VARIATION_SELECTOR=36,
    UCHAR_NFD_INERT=37,
    UCHAR_NFKD_INERT=38,
    UCHAR_NFC_INERT=39,
    UCHAR_NFKC_INERT=40,
    UCHAR_SEGMENT_STARTER=41,
    UCHAR_PATTERN_SYNTAX=42,
    UCHAR_PATTERN_WHITE_SPACE=43,
    UCHAR_POSIX_ALNUM=44,
    UCHAR_POSIX_BLANK=45,
    UCHAR_POSIX_GRAPH=46,
    UCHAR_POSIX_PRINT=47,
    UCHAR_POSIX_XDIGIT=48,
    UCHAR_CASED=49,
    UCHAR_CASE_IGNORABLE=50,
    UCHAR_CHANGES_WHEN_LOWERCASED=51,
    UCHAR_CHANGES_WHEN_UPPERCASED=52,
    UCHAR_CHANGES_WHEN_TITLECASED=53,
    UCHAR_CHANGES_WHEN_CASEFOLDED=54,
    UCHAR_CHANGES_WHEN_CASEMAPPED=55,
    UCHAR_CHANGES_WHEN_NFKC_CASEFOLDED=56,
    UCHAR_EMOJI=57,
    UCHAR_EMOJI_PRESENTATION=58,
    UCHAR_EMOJI_MODIFIER=59,
    UCHAR_EMOJI_MODIFIER_BASE=60,
    UCHAR_EMOJI_COMPONENT=61,
    UCHAR_REGIONAL_INDICATOR=62,
    UCHAR_PREPENDED_CONCATENATION_MARK=63,
    UCHAR_EXTENDED_PICTOGRAPHIC=64,
    UCHAR_BINARY_LIMIT=72,
    
    /* Enumerated/catalog properties */
    UCHAR_BIDI_CLASS=0x1000,
    UCHAR_INT_START=UCHAR_BIDI_CLASS,
    UCHAR_BLOCK=0x1001,
    UCHAR_CANONICAL_COMBINING_CLASS=0x1002,
    UCHAR_DECOMPOSITION_TYPE=0x1003,
    UCHAR_EAST_ASIAN_WIDTH=0x1004,
    UCHAR_GENERAL_CATEGORY=0x1005,
    UCHAR_JOINING_GROUP=0x1006,
    UCHAR_JOINING_TYPE=0x1007,
    UCHAR_LINE_BREAK=0x1008,
    UCHAR_NUMERIC_TYPE=0x1009,
    UCHAR_SCRIPT=0x100A,
    UCHAR_HANGUL_SYLLABLE_TYPE=0x100B,
    UCHAR_NFD_QUICK_CHECK=0x100C,
    UCHAR_NFKD_QUICK_CHECK=0x100D,
    UCHAR_NFC_QUICK_CHECK=0x100E,
    UCHAR_NFKC_QUICK_CHECK=0x100F,
    UCHAR_LEAD_CANONICAL_COMBINING_CLASS=0x1010,
    UCHAR_TRAIL_CANONICAL_COMBINING_CLASS=0x1011,
    UCHAR_BIDI_MIRRORED_TYPE=0x1012,
    UCHAR_NUMERIC_VALUE=0x1013,
    UCHAR_BIDI_PAIRED_BRACKET=0x1014,
    UCHAR_BIDI_PAIRED_BRACKET_TYPE=0x1015,
    UCHAR_JAVA_LOWERCASE=0x1016,
    UCHAR_JAVA_UPPERCASE=0x1017,
    UCHAR_JAVA_WHITESPACE=0x1018,
    UCHAR_GRAPHEME_CLUSTER_BREAK=0x1019,
    UCHAR_WORD_BREAK=0x101A,
    UCHAR_SENTENCE_BREAK=0x101B,
    UCHAR_UPPERCASE_MAPPING=0x101C,
    UCHAR_LOWERCASE_MAPPING=0x101D,
    UCHAR_TITLECASE_MAPPING=0x101E,
    UCHAR_CASE_FOLDING=0x101F,
    UCHAR_SIMPLE_CASE_FOLDING=0x1020,
    UCHAR_ENUM_LIMIT=0x2000,
    
    /* Mask properties */
    UCHAR_GENERAL_CATEGORY_MASK=0x2000,
    UCHAR_MASK_START=UCHAR_GENERAL_CATEGORY_MASK,
    UCHAR_SCRIPT_MASK=0x2001,
    UCHAR_MASK_LIMIT=0x3000,
    
    /* String properties */
    UCHAR_NAME=0x3000,
    UCHAR_STRING_START=UCHAR_NAME,
    UCHAR_NAME_ALIAS=0x3001,
    UCHAR_UNICODE_1_NAME=0x3002,
    UCHAR_STRING_LIMIT=0x4000,
    
    /* Reserved */
    UCHAR_RESERVED_0=0xFFFF
} UProperty;

typedef enum UCharCategory {
    U_UNASSIGNED              = 0,
    U_GENERAL_OTHER_TYPES     = 0,
    U_UPPERCASE_LETTER        = 1,
    U_LOWERCASE_LETTER        = 2,
    U_TITLECASE_LETTER        = 3,
    U_MODIFIER_LETTER         = 4,
    U_OTHER_LETTER            = 5,
    U_NON_SPACING_MARK        = 6,
    U_ENCLOSING_MARK          = 7,
    U_COMBINING_SPACING_MARK  = 8,
    U_DECIMAL_DIGIT_NUMBER    = 9,
    U_LETTER_NUMBER           = 10,
    U_OTHER_NUMBER            = 11,
    U_SPACE_SEPARATOR         = 12,
    U_LINE_SEPARATOR          = 13,
    U_PARAGRAPH_SEPARATOR     = 14,
    U_CONTROL_CHAR            = 15,
    U_FORMAT_CHAR             = 16,
    U_PRIVATE_USE_CHAR        = 17,
    U_SURROGATE               = 18,
    U_DASH_PUNCTUATION        = 19,
    U_START_PUNCTUATION       = 20,
    U_END_PUNCTUATION         = 21,
    U_CONNECTOR_PUNCTUATION   = 22,
    U_OTHER_PUNCTUATION       = 23,
    U_MATH_SYMBOL             = 24,
    U_CURRENCY_SYMBOL         = 25,
    U_MODIFIER_SYMBOL         = 26,
    U_OTHER_SYMBOL            = 27,
    U_INITIAL_PUNCTUATION     = 28,
    U_FINAL_PUNCTUATION       = 29,
    U_CHAR_CATEGORY_COUNT     = 30
} UCharCategory;

/* Compatibility aliases */
#define U_CONTROL U_CONTROL_CHAR
#define U_FORMAT U_FORMAT_CHAR
#define U_PRIVATE_USE U_PRIVATE_USE_CHAR
#define U_OPEN_PUNCTUATION U_START_PUNCTUATION
#define U_CLOSE_PUNCTUATION U_END_PUNCTUATION

typedef enum UJoiningType {
    U_JT_NON_JOINING=0,        /*[U]*/
    U_JT_JOIN_CAUSING=1,       /*[C]*/
    U_JT_DUAL_JOINING=2,       /*[D]*/
    U_JT_LEFT_JOINING=3,       /*[L]*/
    U_JT_RIGHT_JOINING=4,      /*[R]*/
    U_JT_TRANSPARENT=5,        /*[T]*/
#ifndef U_HIDE_DEPRECATED_API
    U_JT_COUNT=6
#endif
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

#define U_GET_GC_MASK(c) (1 << u_charType(c))

#define U_GC_CN_MASK    (1 << U_UNASSIGNED)
#define U_GC_LU_MASK    (1 << U_UPPERCASE_LETTER)
#define U_GC_LL_MASK    (1 << U_LOWERCASE_LETTER)
#define U_GC_LT_MASK    (1 << U_TITLECASE_LETTER)
#define U_GC_LM_MASK    (1 << U_MODIFIER_LETTER)
#define U_GC_LO_MASK    (1 << U_OTHER_LETTER)
#define U_GC_MN_MASK    (1 << U_NON_SPACING_MARK)
#define U_GC_ME_MASK    (1 << U_ENCLOSING_MARK)
#define U_GC_MC_MASK    (1 << U_COMBINING_SPACING_MARK)
#define U_GC_ND_MASK    (1 << U_DECIMAL_DIGIT_NUMBER)
#define U_GC_NL_MASK    (1 << U_LETTER_NUMBER)
#define U_GC_NO_MASK    (1 << U_OTHER_NUMBER)
#define U_GC_ZS_MASK    (1 << U_SPACE_SEPARATOR)
#define U_GC_ZL_MASK    (1 << U_LINE_SEPARATOR)
#define U_GC_ZP_MASK    (1 << U_PARAGRAPH_SEPARATOR)
#define U_GC_CC_MASK    (1 << U_CONTROL_CHAR)
#define U_GC_CF_MASK    (1 << U_FORMAT_CHAR)
#define U_GC_CO_MASK    (1 << U_PRIVATE_USE_CHAR)
#define U_GC_CS_MASK    (1 << U_SURROGATE)
#define U_GC_PD_MASK    (1 << U_DASH_PUNCTUATION)
#define U_GC_PS_MASK    (1 << U_START_PUNCTUATION)
#define U_GC_PE_MASK    (1 << U_END_PUNCTUATION)
#define U_GC_PC_MASK    (1 << U_CONNECTOR_PUNCTUATION)
#define U_GC_PO_MASK    (1 << U_OTHER_PUNCTUATION)
#define U_GC_SM_MASK    (1 << U_MATH_SYMBOL)
#define U_GC_SC_MASK    (1 << U_CURRENCY_SYMBOL)
#define U_GC_SK_MASK    (1 << U_MODIFIER_SYMBOL)
#define U_GC_SO_MASK    (1 << U_OTHER_SYMBOL)
#define U_GC_PI_MASK    (1 << U_INITIAL_PUNCTUATION)
#define U_GC_PF_MASK    (1 << U_FINAL_PUNCTUATION)

#define U_GC_L_MASK     (U_GC_LU_MASK | U_GC_LL_MASK | U_GC_LT_MASK | U_GC_LM_MASK | U_GC_LO_MASK)
#define U_GC_M_MASK     (U_GC_MN_MASK | U_GC_ME_MASK | U_GC_MC_MASK)
#define U_GC_N_MASK     (U_GC_ND_MASK | U_GC_NL_MASK | U_GC_NO_MASK)
#define U_GC_Z_MASK     (U_GC_ZS_MASK | U_GC_ZL_MASK | U_GC_ZP_MASK)
#define U_GC_C_MASK     (U_GC_CC_MASK | U_GC_CF_MASK | U_GC_CO_MASK | U_GC_CS_MASK | U_GC_CN_MASK)
#define U_GC_P_MASK     (U_GC_PD_MASK | U_GC_PS_MASK | U_GC_PE_MASK | U_GC_PC_MASK | U_GC_PO_MASK | U_GC_PI_MASK | U_GC_PF_MASK)
#define U_GC_S_MASK     (U_GC_SM_MASK | U_GC_SC_MASK | U_GC_SK_MASK | U_GC_SO_MASK)

typedef enum UCharDirection {
    U_LEFT_TO_RIGHT               = 0,
    U_RIGHT_TO_LEFT               = 1,
    U_EUROPEAN_NUMBER             = 2,
    U_EUROPEAN_NUMBER_SEPARATOR   = 3,
    U_EUROPEAN_NUMBER_TERMINATOR  = 4,
    U_ARABIC_NUMBER               = 5,
    U_COMMON_NUMBER_SEPARATOR     = 6,
    U_BLOCK_SEPARATOR             = 7,
    U_SEGMENT_SEPARATOR           = 8,
    U_WHITE_SPACE_NEUTRAL         = 9,
    U_OTHER_NEUTRAL               = 10,
    U_LEFT_TO_RIGHT_EMBEDDING     = 11,
    U_LEFT_TO_RIGHT_OVERRIDE      = 12,
    U_RIGHT_TO_LEFT_ARABIC        = 13,
    U_RIGHT_TO_LEFT_EMBEDDING     = 14,
    U_RIGHT_TO_LEFT_OVERRIDE      = 15,
    U_POP_DIRECTIONAL_FORMAT      = 16,
    U_DIR_NON_SPACING_MARK        = 17,
    U_BOUNDARY_NEUTRAL            = 18,
    U_FIRST_STRONG_ISOLATE        = 19,
    U_LEFT_TO_RIGHT_ISOLATE       = 20,
    U_RIGHT_TO_LEFT_ISOLATE       = 21,
    U_POP_DIRECTIONAL_ISOLATE     = 22,
#ifndef U_HIDE_DEPRECATED_API
    U_CHAR_DIRECTION_COUNT        = 23
#endif
} UCharDirection;

/* Functions */
UCharDirection u_charDirection(UChar32 c);
UCharCategory u_charType(UChar32 c);
UBool u_hasBinaryProperty(UChar32 c, UProperty prop);
UBool u_isMirrored(UChar32 c);
UBool u_isWhiteSpace(UChar32 c);
UChar32 u_tolower(UChar32 c);
UChar32 u_toupper(UChar32 c);

int32_t u_getIntPropertyValue(UChar32 c, UProperty prop);
const char* u_charName(UChar32 c, int32_t nameChoice);

#endif // __UCHAR_H__