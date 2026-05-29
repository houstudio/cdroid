#ifndef __UTF16_H__
#define __UTF16_H__
#include "unicode/utypes.h"

#define U_IS_SURROGATE(c) (((c)&0xfffff800)==0xd800)

#define U16_IS_SINGLE(c) !U_IS_SURROGATE(c)

#define U16_IS_LEAD(c) (((c)&0xfffffc00)==0xd800)

#define U16_IS_TRAIL(c) (((c)&0xfffffc00)==0xdc00)

#define U16_IS_SURROGATE(c) U_IS_SURROGATE(c)

#define U16_IS_SURROGATE_LEAD(c) (((c)&0x400)==0)

#define U16_IS_SURROGATE_TRAIL(c) (((c)&0x400)!=0)

#define U16_SURROGATE_OFFSET ((0xd800<<10UL)+0xdc00-0x10000)

#define U16_GET_SUPPLEMENTARY(lead, trail) \
    (((UChar32)(lead)<<10UL)+(UChar32)(trail)-U16_SURROGATE_OFFSET)

#define U16_LEAD(supplementary) (UChar)(((supplementary)>>10)+0xd7c0)

#define U16_TRAIL(supplementary) (UChar)(((supplementary)&0x3ff)|0xdc00)

#define U16_LENGTH(c) ((uint32_t)(c)<=0xffff ? 1 : 2)


#define U16_PREV(s, start, i, c) UPRV_BLOCK_MACRO_BEGIN { \
    (c)=(s)[--(i)]; \
    if(U16_IS_TRAIL(c)) { \
        uint16_t __c2; \
        if((i)>(start) && U16_IS_LEAD(__c2=(s)[(i)-1])) { \
            --(i); \
            (c)=U16_GET_SUPPLEMENTARY(__c2, (c)); \
        } \
    } \
} UPRV_BLOCK_MACRO_END

#define U16_NEXT(s, i, length, c) UPRV_BLOCK_MACRO_BEGIN { \
    (c)=(s)[(i)++]; \
    if(U16_IS_LEAD(c)) { \
        uint16_t __c2; \
        if((i)!=(length) && U16_IS_TRAIL(__c2=(s)[(i)])) { \
            ++(i); \
            (c)=U16_GET_SUPPLEMENTARY((c), __c2); \
        } \
    } \
} UPRV_BLOCK_MACRO_END

#define U16_APPEND(s, i, capacity, c, isError) UPRV_BLOCK_MACRO_BEGIN { \
    if((uint32_t)(c)<=0xffff) { \
        (s)[(i)++]=(uint16_t)(c); \
    } else if((uint32_t)(c)<=0x10ffff && (i)+1<(capacity)) { \
        (s)[(i)++]=(uint16_t)(((c)>>10)+0xd7c0); \
        (s)[(i)++]=(uint16_t)(((c)&0x3ff)|0xdc00); \
    } else /* c>0x10ffff or not enough space */ { \
        (isError)=true; \
    } \
} UPRV_BLOCK_MACRO_END

#endif

