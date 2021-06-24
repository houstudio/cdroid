#ifndef __UTF_8_H__
#define __UTF_8_H__
#include<cdtypes.h>
typedef unsigned int unichar;
#define UTF8_SKIP(c)     (((BYTE)(c) < 0xc0) ? 1 : __utf8_skip[(BYTE)(c)&0x3f])

#define UTF8_GET_CHAR(p) (*(const BYTE*)(p) < 0xc0 ? \
                                 *(const BYTE*)(p) : __utf8_get_char((const BYTE*)(p)))


/*
 *  Actually the last two fields used to be zero since they indicate an
 *  invalid UTF-8 string. Changed it to 1 to avoid endless looping on
 *  invalid input.
 */
static const char __utf8_skip[64] = {
     2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
     3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,5,5,5,5,6,6,1,1
};
static __inline__ unichar __utf8_get_char( const BYTE *p )
{
     int              len;
     register unichar result = p[0];
     if (result < 0xc0)
          return result;
     if (result > 0xfd)
          return (unichar) -1;
     len = __utf8_skip[result & 0x3f];
     result &= 0x7c >> len;
     while (--len) {
          int c = *(++p);
          if ((c & 0xc0) != 0x80)
               return (unichar) -1;
          result = (result << 6) | (c & 0x3f);
     }
     return result;
}

#endif
