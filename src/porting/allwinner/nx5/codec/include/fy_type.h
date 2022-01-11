#ifndef __FY_TYPE_H__
#define __FY_TYPE_H__


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

/*----------------------------------------------*
 * The common data type, will be used in the whole project.*
 *----------------------------------------------*/

typedef unsigned char           FY_U8;
typedef unsigned short          FY_U16;
typedef unsigned int            FY_U32;
typedef double                  FY_DOUBLE;
typedef signed char             FY_S8;
typedef short                   FY_S16;
typedef int                     FY_S32;

#ifndef _M_IX86
    typedef unsigned long long  FY_U64;
    typedef long long           FY_S64;
#else
    typedef __int64             FY_U64;
    typedef __int64             FY_S64;
#endif

typedef char                    FY_CHAR;
#define FY_VOID                 void

/*----------------------------------------------*
 * const defination                             *
 *----------------------------------------------*/
typedef enum {
    FY_FALSE = 0,
    FY_TRUE  = 1,
} FY_BOOL;

#ifndef NULL
    #define NULL    0L
#endif

#define FY_NULL     0L
#define FY_SUCCESS  0
#define FY_FAILURE  (-1)


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __FY_TYPE_H__ */

