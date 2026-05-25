#ifndef __UTF8_H__
#define __UTF8_H__
#include "unicode/utypes.h"

#define U8_COUNT_TRAIL_BYTES(leadByte) \
    (U8_IS_LEAD(leadByte) ? \
        ((uint8_t)(leadByte)>=0xe0)+((uint8_t)(leadByte)>=0xf0)+1 : 0)

#define U8_COUNT_TRAIL_BYTES_UNSAFE(leadByte) \
    (((uint8_t)(leadByte)>=0xc2)+((uint8_t)(leadByte)>=0xe0)+((uint8_t)(leadByte)>=0xf0))

#define U8_MASK_LEAD_BYTE(leadByte, countTrailBytes) ((leadByte)&=(1<<(6-(countTrailBytes)))-1)

#define U8_LEAD3_T1_BITS "\x20\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x10\x30\x30"

#define U8_IS_VALID_LEAD3_AND_T1(lead, t1) (U8_LEAD3_T1_BITS[(lead)&0xf]&(1<<((uint8_t)(t1)>>5)))

#define U8_LEAD4_T1_BITS "\x00\x00\x00\x00\x00\x00\x00\x00\x1E\x0F\x0F\x0F\x00\x00\x00\x00"


#define U8_IS_SINGLE(c) ((int8_t)(c)>=0)

#define U8_IS_LEAD(c) ((uint8_t)((c)-0xc2)<=0x32)

#define U8_IS_TRAIL(c) ((int8_t)(c)<-0x40)

#define U8_LENGTH(c) \
    ((uint32_t)(c)<=0x7f ? 1 : \
        ((uint32_t)(c)<=0x7ff ? 2 : \
            ((uint32_t)(c)<=0xd7ff ? 3 : \
                ((uint32_t)(c)<=0xdfff || (uint32_t)(c)>0x10ffff ? 0 : \
                    ((uint32_t)(c)<=0xffff ? 3 : 4)\
                ) \
            ) \
        ) \
    )


#define U8_APPEND(s, i, capacity, c, isError) UPRV_BLOCK_MACRO_BEGIN { \
    uint32_t __uc=(c); \
    if(__uc<=0x7f) { \
        (s)[(i)++]=(uint8_t)__uc; \
    } else if(__uc<=0x7ff && (i)+1<(capacity)) { \
        (s)[(i)++]=(uint8_t)((__uc>>6)|0xc0); \
        (s)[(i)++]=(uint8_t)((__uc&0x3f)|0x80); \
    } else if((__uc<=0xd7ff || (0xe000<=__uc && __uc<=0xffff)) && (i)+2<(capacity)) { \
        (s)[(i)++]=(uint8_t)((__uc>>12)|0xe0); \
        (s)[(i)++]=(uint8_t)(((__uc>>6)&0x3f)|0x80); \
        (s)[(i)++]=(uint8_t)((__uc&0x3f)|0x80); \
    } else if(0xffff<__uc && __uc<=0x10ffff && (i)+3<(capacity)) { \
        (s)[(i)++]=(uint8_t)((__uc>>18)|0xf0); \
        (s)[(i)++]=(uint8_t)(((__uc>>12)&0x3f)|0x80); \
        (s)[(i)++]=(uint8_t)(((__uc>>6)&0x3f)|0x80); \
        (s)[(i)++]=(uint8_t)((__uc&0x3f)|0x80); \
    } else { \
        (isError)=true; \
    } \
} UPRV_BLOCK_MACRO_END


#endif
