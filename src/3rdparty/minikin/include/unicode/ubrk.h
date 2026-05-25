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

#define UBRK_DONE (-1)

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
