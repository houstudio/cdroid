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
    UCHAR_GENERAL_CATEGORY = 0,
    UCHAR_CANONICAL_COMBINING_CLASS = 3,
    UCHAR_BIDI_CLASS = 4,
    UCHAR_LINE_BREAK = 13,
    UCHAR_JOINING_TYPE = 14,
    UCHAR_GRAPHEME_CLUSTER_BREAK = 41,
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
    U_LB_JT = 34,               /*[JT]*/
    U_LB_JV = 35,               /*[JV]*/
    U_LB_CLOSE_PARENTHESIS = 36,/*[CP]*/
    U_LB_CONDITIONAL_JAPANESE_STARTER = 37,/*[CJ]*/
    U_LB_HEBREW_LETTER = 38,    /*[HL]*/
    U_LB_REGIONAL_INDICATOR = 39,/*[RI]*/
    U_LB_E_BASE = 40,           /*[EB]*/
    U_LB_E_MODIFIER = 41,       /*[EM]*/
    U_LB_ZWJ = 42,              /*[ZWJ]*/
    U_LB_AKSARA = 43,           /*[AK]*/
    U_LB_AKSARA_PREBASE = 44,   /*[AP]*/
    U_LB_AKSARA_START = 45,     /*[AS]*/
    U_LB_VIRAMA_FINAL = 46,     /*[VF]*/
    U_LB_VIRAMA = 47,           /*[VI]*/
    U_LB_UNAMBIGUOUS_HYPHEN = 48,/*[HH]*/
    U_LB_COUNT = 49
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


#endif
