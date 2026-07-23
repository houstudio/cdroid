#ifndef __UNORM2_H__
#define __UNORM2_H__
#include "unicode/utypes.h"
#include "unicode/uset.h"
#include "unicode/utf16.h"

struct UNormalizer2;
typedef struct UNormalizer2 UNormalizer2;

U_CAPI const UNormalizer2* U_EXPORT2
unorm2_getNFDInstance(UErrorCode* status);

U_CAPI int32_t U_EXPORT2
unorm2_getRawDecomposition(const UNormalizer2* n2, UChar32 c, UChar* dest, int32_t destCapacity, UErrorCode* status);

#define U16_NEXT_UNSAFE(s, i, c) UPRV_BLOCK_MACRO_BEGIN { \
    (c)=(s)[(i)++]; \
    if(U16_IS_LEAD(c)) { \
        (c)=U16_GET_SUPPLEMENTARY((c), (s)[(i)++]); \
    } \
} UPRV_BLOCK_MACRO_END

#endif