#ifndef __FY_DEBUG_H__
#define __FY_DEBUG_H__

#ifndef __KERNEL__
#include <stdarg.h>
#endif

#include "fy_type.h"
#include "fy_common.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#define _EX__FILE_LINE(fxx,lxx) "[File]:"fxx"\n[Line]:"#lxx"\n[Info]:"
#define EX__FILE_LINE(fxx,lxx) _EX__FILE_LINE(fxx,lxx)
#define __FILE_LINE__ EX__FILE_LINE(__FILE__, __LINE__)

#define FY_DBG_EMERG      0   /* system is unusable                   */
#define FY_DBG_ALERT      1   /* action must be taken immediately     */
#define FY_DBG_CRIT       2   /* critical conditions                  */
#define FY_DBG_ERR        3   /* error conditions                     */
#define FY_DBG_WARN       4   /* warning conditions                   */
#define FY_DBG_NOTICE     5   /* normal but significant condition     */
#define FY_DBG_INFO       6   /* informational                        */
#define FY_DBG_DEBUG      7   /* debug-level messages                 */

typedef struct fyLOG_LEVEL_CONF_S
{
    MOD_ID_E  enModId;
    FY_S32    s32Level;
    FY_CHAR   cModName[16];
} LOG_LEVEL_CONF_S;

#ifndef __KERNEL__ 
/******************************************************************************
** For User Mode : FY_PRINT, FY_ASSERT, FY_TRACE
******************************************************************************/

#define FY_PRINT printf

/* #ifdef FY_DEBUG */
#if 1
    /* Using samples:   FY_ASSERT(x>y); */
    #define FY_ASSERT(expr)               \
    do{                                   \
        if (!(expr)) {                    \
            printf("\nASSERT failed at:\n"\
                   "  >File name: %s\n"   \
                   "  >Function : %s\n"   \
                   "  >Line No. : %d\n"   \
                   "  >Condition: %s\n",  \
                    __FILE__,__FUNCTION__, __LINE__, #expr);\
            _exit(-1);\
        } \
    }while(0)

    /* Using samples: 
    ** FY_TRACE(FY_DBG_DEBUG, FY_ID_CMPI, "Test %d, %s\n", 12, "Test");
    **/
    #define FY_TRACE(level, enModId, fmt...) fprintf(stderr,##fmt)
#else
    #define FY_ASSERT(expr)
    #define FY_TRACE(level, enModId, fmt...)
#endif

#else
/******************************************************************************
** For Linux Kernel : FY_PRINT, FY_ASSERT, FY_TRACE
******************************************************************************/

#define FY_PRINT printk

extern FY_S32 FY_ChkLogLevel(FY_S32 s32Levle, MOD_ID_E enModId);
asmlinkage int FY_LOG(FY_S32 level, MOD_ID_E enModId,const char *fmt, ...);

/* #ifdef FY_DEBUG */
#if 1
    /* Using samples:   FY_ASSERT(x>y); */
    #define FY_ASSERT(expr)               \
    do{                                   \
        if (!(expr)) {                    \
            panic("\nASSERT failed at:\n" \
                  "  >File name: %s\n"    \
                  "  >Function : %s\n"    \
                  "  >Line No. : %d\n"    \
                  "  >Condition: %s\n",   \
                    __FILE__,__FUNCTION__, __LINE__, #expr);\
        } \
    }while(0)

    /* Using samples: 
    ** FY_TRACE(FY_DBG_DEBUG, FY_ID_CMPI, "Test %d, %s\n", 12, "Test");
    **/
    #define FY_TRACE FY_LOG
#else
    #define FY_ASSERT(expr)
    #define FY_TRACE(level, enModId, fmt...)
#endif

#endif  /* end of __KERNEL__ */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __FY_DEBUG_H__ */

