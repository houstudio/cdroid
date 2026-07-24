#ifndef __UBRK_H__
#define __UBRK_H__
#include "unicode/utypes.h"
#include "unicode/utext.h"
#include "unicode/uloc.h"

struct UBreakIterator;
typedef struct UBreakIterator UBreakIterator;

enum UBreakIteratorType {
  /** Character breaks  @stable ICU 2.0 */
  UBRK_CHARACTER = 0,
  /** Word breaks @stable ICU 2.0 */
  UBRK_WORD = 1,
  /** Line breaks @stable ICU 2.0 */
  UBRK_LINE = 2,
  /** Sentence breaks @stable ICU 2.0 */
  UBRK_SENTENCE = 3,
  UBRK_TITLE = 4,
  UBRK_COUNT = 5
};

#define UBRK_DONE ((int32_t) -1)

/* Word break status values returned by ubrk_getRuleStatus() */
#define UBRK_WORD_NONE           0
#define UBRK_WORD_NONE_LIMIT     100
#define UBRK_WORD_NUMBER         100
#define UBRK_WORD_NUMBER_LIMIT   200
#define UBRK_WORD_LETTER         200
#define UBRK_WORD_LETTER_LIMIT   300
#define UBRK_WORD_KANA           300
#define UBRK_WORD_KANA_LIMIT     400
#define UBRK_WORD_IDEO           400
#define UBRK_WORD_IDEO_LIMIT     500

/* Line break status values returned by ubrk_getRuleStatus() */
#define UBRK_LINE_SOFT            0
#define UBRK_LINE_SOFT_LIMIT      100
#define UBRK_LINE_HARD            100
#define UBRK_LINE_HARD_LIMIT      200

/* Sentence break status values returned by ubrk_getRuleStatus() */
#define UBRK_SENTENCE_TERM       0
#define UBRK_SENTENCE_TERM_LIMIT 100
#define UBRK_SENTENCE_SEP        100
#define UBRK_SENTENCE_SEP_LIMIT  200

U_CAPI UBreakIterator* U_EXPORT2
ubrk_open(int32_t type, const char* locale, const UChar* text, int32_t length, UErrorCode* status);

U_CAPI UBreakIterator* U_EXPORT2
ubrk_openUText(int32_t type, const char* locale, UText* text, UErrorCode* status);

U_CAPI void U_EXPORT2
ubrk_close(UBreakIterator* bi);

U_CAPI int32_t U_EXPORT2
ubrk_first(UBreakIterator* bi);

U_CAPI int32_t U_EXPORT2
ubrk_last(UBreakIterator* bi);

U_CAPI int32_t U_EXPORT2
ubrk_next(UBreakIterator* bi);

U_CAPI int32_t U_EXPORT2
ubrk_previous(UBreakIterator* bi);

U_CAPI int32_t U_EXPORT2
ubrk_current(UBreakIterator* bi);

U_CAPI int32_t U_EXPORT2
ubrk_preceding(UBreakIterator* bi, int32_t offset);

U_CAPI int32_t U_EXPORT2
ubrk_following(UBreakIterator* bi, int32_t offset);

U_CAPI UBool U_EXPORT2
ubrk_isBoundary(UBreakIterator* bi, int32_t offset);

U_CAPI void U_EXPORT2
ubrk_setUText(UBreakIterator* bi, UText* text, UErrorCode* status);

U_CAPI UText* U_EXPORT2
utext_openUChars(UText* fillIn, const UChar* chars, int32_t length, UErrorCode* status);

U_CAPI void U_EXPORT2
utext_close(UText* ut);

#endif
