#ifndef __UBIDI_H__
#define __UBIDI_H__
#include "unicode/utypes.h"
#include "unicode/uchar.h"

typedef uint8_t UBiDiLevel;

#define UBIDI_DEFAULT_LTR 0xfe
#define UBIDI_DEFAULT_RTL 0xff

#define UBIDI_MAX_EXPLICIT_LEVEL 125
#define UBIDI_LEVEL_OVERRIDE 0x80
#define UBIDI_MAP_NOWHERE   (-1)

enum UBiDiDirection {
  /** Left-to-right text. This is a 0 value.
   * <ul>
   * <li>As return value for <code>ubidi_getDirection()</code>, it means
   *     that the source string contains no right-to-left characters, or
   *     that the source string is empty and the paragraph level is even.
   * <li> As return value for <code>ubidi_getBaseDirection()</code>, it
   *      means that the first strong character of the source string has
   *      a left-to-right direction.
   * </ul>
   * @stable ICU 2.0
   */
  UBIDI_LTR,
  /** Right-to-left text. This is a 1 value.
   * <ul>
   * <li>As return value for <code>ubidi_getDirection()</code>, it means
   *     that the source string contains no left-to-right characters, or
   *     that the source string is empty and the paragraph level is odd.
   * <li> As return value for <code>ubidi_getBaseDirection()</code>, it
   *      means that the first strong character of the source string has
   *      a right-to-left direction.
   * </ul>
   * @stable ICU 2.0
   */
  UBIDI_RTL,
  /** Mixed-directional text.
   * <p>As return value for <code>ubidi_getDirection()</code>, it means
   *    that the source string contains both left-to-right and
   *    right-to-left characters.
   * @stable ICU 2.0
   */
  UBIDI_MIXED,
  /** No strongly directional text.
   * <p>As return value for <code>ubidi_getBaseDirection()</code>, it means
   *    that the source string is missing or empty, or contains neither left-to-right
   *    nor right-to-left characters.
   * @stable ICU 4.6
   */
  UBIDI_NEUTRAL
};

/** @stable ICU 2.0 */
typedef enum UBiDiDirection UBiDiDirection;

struct UBiDi;

/** @stable ICU 2.0 */
typedef struct UBiDi UBiDi;

U_CAPI UBiDi * U_EXPORT2
ubidi_open(void);

U_CAPI UBiDi * U_EXPORT2
ubidi_openSized(int32_t maxLength, int32_t maxRunCount, UErrorCode *pErrorCode);

U_CAPI void U_EXPORT2
ubidi_close(UBiDi *pBiDi);

U_CAPI void U_EXPORT2
ubidi_setInverse(UBiDi *pBiDi, UBool isInverse);

U_CAPI UBool U_EXPORT2
ubidi_isInverse(UBiDi *pBiDi);

U_CAPI void U_EXPORT2
ubidi_orderParagraphsLTR(UBiDi *pBiDi, UBool orderParagraphsLTR);

U_CAPI UBool U_EXPORT2
ubidi_isOrderParagraphsLTR(UBiDi *pBiDi);


typedef enum UBiDiReorderingMode {
    /** Regular Logical to Visual Bidi algorithm according to Unicode.
      * This is a 0 value.
      * @stable ICU 3.6 */
    UBIDI_REORDER_DEFAULT = 0,
    /** Logical to Visual algorithm which handles numbers in a way which
      * mimics the behavior of Windows XP.
      * @stable ICU 3.6 */
    UBIDI_REORDER_NUMBERS_SPECIAL,
    /** Logical to Visual algorithm grouping numbers with adjacent R characters
      * (reversible algorithm).
      * @stable ICU 3.6 */
    UBIDI_REORDER_GROUP_NUMBERS_WITH_R,
    /** Reorder runs only to transform a Logical LTR string to the Logical RTL
      * string with the same display, or vice-versa.<br>
      * If this mode is set together with option
      * <code>#UBIDI_OPTION_INSERT_MARKS</code>, some Bidi controls in the source
      * text may be removed and other controls may be added to produce the
      * minimum combination which has the required display.
      * @stable ICU 3.6 */
    UBIDI_REORDER_RUNS_ONLY,
    /** Visual to Logical algorithm which handles numbers like L
      * (same algorithm as selected by <code>ubidi_setInverse(true)</code>.
      * @see ubidi_setInverse
      * @stable ICU 3.6 */
    UBIDI_REORDER_INVERSE_NUMBERS_AS_L,
    /** Visual to Logical algorithm equivalent to the regular Logical to Visual
      * algorithm.
      * @stable ICU 3.6 */
    UBIDI_REORDER_INVERSE_LIKE_DIRECT,
    /** Inverse Bidi (Visual to Logical) algorithm for the
      * <code>UBIDI_REORDER_NUMBERS_SPECIAL</code> Bidi algorithm.
      * @stable ICU 3.6 */
    UBIDI_REORDER_INVERSE_FOR_NUMBERS_SPECIAL,
#ifndef U_HIDE_DEPRECATED_API
    /**
     * Number of values for reordering mode.
     * @deprecated ICU 58 The numeric value may change over time, see ICU ticket #12420.
     */
    UBIDI_REORDER_COUNT
#endif  // U_HIDE_DEPRECATED_API
} UBiDiReorderingMode;

U_CAPI void U_EXPORT2
ubidi_setReorderingMode(UBiDi *pBiDi, UBiDiReorderingMode reorderingMode);

U_CAPI UBiDiReorderingMode U_EXPORT2
ubidi_getReorderingMode(UBiDi *pBiDi);

typedef enum UBiDiReorderingOption {
    UBIDI_OPTION_DEFAULT = 0,
    UBIDI_OPTION_INSERT_MARKS = 1,
    UBIDI_OPTION_REMOVE_CONTROLS = 2,
    UBIDI_OPTION_STREAMING = 4
} UBiDiReorderingOption;

U_CAPI void U_EXPORT2
ubidi_setReorderingOptions(UBiDi *pBiDi, uint32_t reorderingOptions);

U_CAPI uint32_t U_EXPORT2
ubidi_getReorderingOptions(UBiDi *pBiDi);

U_CAPI void U_EXPORT2
ubidi_setContext(UBiDi *pBiDi,
                 const UChar *prologue, int32_t proLength,
                 const UChar *epilogue, int32_t epiLength,
                 UErrorCode *pErrorCode);

U_CAPI void U_EXPORT2
ubidi_setPara(UBiDi *pBiDi, const UChar *text, int32_t length,
              UBiDiLevel paraLevel, UBiDiLevel *embeddingLevels,
              UErrorCode *pErrorCode);

U_CAPI void U_EXPORT2
ubidi_setLine(const UBiDi *pParaBiDi,
              int32_t start, int32_t limit,
              UBiDi *pLineBiDi,
              UErrorCode *pErrorCode);

U_CAPI UBiDiDirection U_EXPORT2
ubidi_getDirection(const UBiDi *pBiDi);

U_CAPI UBiDiDirection U_EXPORT2
ubidi_getBaseDirection(const UChar *text,  int32_t length );

U_CAPI const UChar * U_EXPORT2
ubidi_getText(const UBiDi *pBiDi);

U_CAPI int32_t U_EXPORT2
ubidi_getLength(const UBiDi *pBiDi);

U_CAPI UBiDiLevel U_EXPORT2
ubidi_getParaLevel(const UBiDi *pBiDi);

U_CAPI int32_t U_EXPORT2
ubidi_countParagraphs(UBiDi *pBiDi);

U_CAPI int32_t U_EXPORT2
ubidi_getParagraph(const UBiDi *pBiDi, int32_t charIndex, int32_t *pParaStart,
                   int32_t *pParaLimit, UBiDiLevel *pParaLevel,
                   UErrorCode *pErrorCode);

U_CAPI void U_EXPORT2
ubidi_getParagraphByIndex(const UBiDi *pBiDi, int32_t paraIndex,
                          int32_t *pParaStart, int32_t *pParaLimit,
                          UBiDiLevel *pParaLevel, UErrorCode *pErrorCode);

U_CAPI UBiDiLevel U_EXPORT2
ubidi_getLevelAt(const UBiDi *pBiDi, int32_t charIndex);

U_CAPI const UBiDiLevel * U_EXPORT2
ubidi_getLevels(UBiDi *pBiDi, UErrorCode *pErrorCode);

U_CAPI void U_EXPORT2
ubidi_getLogicalRun(const UBiDi *pBiDi, int32_t logicalPosition,
                    int32_t *pLogicalLimit, UBiDiLevel *pLevel);

U_CAPI int32_t U_EXPORT2
ubidi_countRuns(UBiDi *pBiDi, UErrorCode *pErrorCode);

U_CAPI UBiDiDirection U_EXPORT2
ubidi_getVisualRun(UBiDi *pBiDi, int32_t runIndex,
                   int32_t *pLogicalStart, int32_t *pLength);

U_CAPI int32_t U_EXPORT2
ubidi_getVisualIndex(UBiDi *pBiDi, int32_t logicalIndex, UErrorCode *pErrorCode);

U_CAPI int32_t U_EXPORT2
ubidi_getLogicalIndex(UBiDi *pBiDi, int32_t visualIndex, UErrorCode *pErrorCode);

U_CAPI void U_EXPORT2
ubidi_getLogicalMap(UBiDi *pBiDi, int32_t *indexMap, UErrorCode *pErrorCode);

U_CAPI void U_EXPORT2
ubidi_getVisualMap(UBiDi *pBiDi, int32_t *indexMap, UErrorCode *pErrorCode);

U_CAPI void U_EXPORT2
ubidi_reorderLogical(const UBiDiLevel *levels, int32_t length, int32_t *indexMap);

U_CAPI void U_EXPORT2
ubidi_reorderVisual(const UBiDiLevel *levels, int32_t length, int32_t *indexMap);

U_CAPI void U_EXPORT2
ubidi_invertMap(const int32_t *srcMap, int32_t *destMap, int32_t length);




U_CAPI int32_t U_EXPORT2
ubidi_getProcessedLength(const UBiDi *pBiDi);

typedef UCharDirection U_CALLCONV
UBiDiClassCallback(const void *context, UChar32 c);

U_CAPI UCharDirection U_EXPORT2
ubidi_getCustomizedClass(UBiDi *pBiDi, UChar32 c);

U_CAPI void U_EXPORT2
ubidi_setClassCallback(UBiDi *pBiDi, UBiDiClassCallback *newFn,
                       const void *newContext, UBiDiClassCallback **oldFn,
                       const void **oldContext, UErrorCode *pErrorCode);

#endif
