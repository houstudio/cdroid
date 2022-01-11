#ifndef __FY_COMM_SYS_H__
#define __FY_COMM_SYS_H__

#include "fy_type.h"
#include "fy_errno.h"
#include "fy_debug.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

typedef struct fyMPP_SYS_CONF_S
{
    /* stride of picture buffer must be aligned with this value.
     * you can choose a value from 1 to 1024, and it must be multiple of 16.
     */
    FY_U32 u32AlignWidth;  

}MPP_SYS_CONF_S;

typedef enum fyEN_SYS_ERR_CODE_E
{
    ERR_SYS_NOHEARTBEAT = 0x40,

    ERR_SYS_BUTT
}EN_SYS_ERR_CODE_E;


#define FY_ERR_SYS_NULL_PTR         FY_DEF_ERR(FY_ID_SYS, EN_ERR_LEVEL_ERROR, EN_ERR_NULL_PTR)
#define FY_ERR_SYS_NOTREADY         FY_DEF_ERR(FY_ID_SYS, EN_ERR_LEVEL_ERROR, EN_ERR_SYS_NOTREADY)
#define FY_ERR_SYS_NOT_PERM         FY_DEF_ERR(FY_ID_SYS, EN_ERR_LEVEL_ERROR, EN_ERR_NOT_PERM)
#define FY_ERR_SYS_NOMEM            FY_DEF_ERR(FY_ID_SYS, EN_ERR_LEVEL_ERROR, EN_ERR_NOMEM)
#define FY_ERR_SYS_ILLEGAL_PARAM    FY_DEF_ERR(FY_ID_SYS, EN_ERR_LEVEL_ERROR, EN_ERR_ILLEGAL_PARAM)
#define FY_ERR_SYS_BUSY             FY_DEF_ERR(FY_ID_SYS, EN_ERR_LEVEL_ERROR, EN_ERR_BUSY)
#define FY_ERR_SYS_NOT_SUPPORT      FY_DEF_ERR(FY_ID_SYS, EN_ERR_LEVEL_ERROR, EN_ERR_NOT_SUPPORT)
#define FY_ERR_SYS_NOHEARTBEAT      FY_DEF_ERR(FY_ID_SYS, EN_ERR_LEVEL_ERROR, ERR_SYS_NOHEARTBEAT)


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif  /* __FY_COMM_SYS_H__ */

