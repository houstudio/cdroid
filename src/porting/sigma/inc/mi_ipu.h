/* SigmaStar trade secret */
/* Copyright (c) [2019~2020] SigmaStar Technology.
All rights reserved.

Unless otherwise stipulated in writing, any and all information contained
herein regardless in any format shall remain the sole proprietary of
SigmaStar and be kept in strict confidence
(SigmaStar Confidential Information) by the recipient.
Any unauthorized act including without limitation unauthorized disclosure,
copying, use, reproduction, sale, distribution, modification, disassembling,
reverse engineering and compiling of the contents of SigmaStar Confidential
Information is unlawful and strictly prohibited. SigmaStar hereby reserves the
rights to any and all damages, losses, costs and expenses resulting therefrom.
*/

#ifndef _MI_IPU_H_
#define _MI_IPU_H_
#include "mi_common.h"
#include "mi_common_datatype.h"

#define MI_IPU_MAX_TENSOR_DIM 8
#define MI_IPU_CHN_MAX 64
#define MI_IPU_MAX_INPUT_TENSOR_CNT 16
#define MI_IPU_MAX_OUTPUT_TENSOR_CNT 16
#define MI_IPU_MAX_TENSOR_CNT \
                    ((MI_IPU_MAX_INPUT_TENSOR_CNT>MI_IPU_MAX_OUTPUT_TENSOR_CNT)? \
                    MI_IPU_MAX_INPUT_TENSOR_CNT:MI_IPU_MAX_OUTPUT_TENSOR_CNT)

#define MAX_TENSOR_NAME_LEN     256
#define MAX_IPU_INPUT_OUTPUT_BUF_DEPTH 3
#define MI_IPU_MAX_TIMEOUT  (10*60*1000)    //10 minutes
#define MI_IPU_BOOT_MAX_TIMEOUT (30*1000)   //30 seconds

#define YUV420_W_PITCH_ALIGNMENT    16
#define YUV420_H_PITCH_ALIGNMENT        2
#define XRGB_W_PITCH_ALIGNMENT          16

#if defined(__linux__)
//#define PERFORMANCE_DEBUG
#endif

//#define IPU_DEBUG
#define ipu_err(fmt, args...) CamOsPrintf(fmt, ##args)

#ifdef IPU_DEBUG
#define ipu_info(fmt, args...) CamOsPrintf(fmt,##args)
#else
#define ipu_info(fmt, args...)
#endif

typedef MI_U32 MI_IPU_CHN;
typedef int (*SerializedReadFunc)(void *dst_buf,int offset, int size, char *ctx);

typedef enum
{
    //MI_IPU_FORMAT_BGR,
    //MI_IPU_FORMAT_ARGB,
    //MI_IPU_FORMAT_ABGR,
    MI_IPU_FORMAT_U8,
    MI_IPU_FORMAT_NV12,
    MI_IPU_FORMAT_INT16,
    MI_IPU_FORMAT_INT32,
    MI_IPU_FORMAT_INT8,
    MI_IPU_FORMAT_FP32,
    MI_IPU_FORMAT_UNKNOWN,
} MI_IPU_ELEMENT_FORMAT;

typedef enum
{
    E_IPU_ERR_INVALID_CHNID = 1,        /* invalid channel ID */
    E_IPU_ERR_CHNID_EXIST,              /* channel exists */
    E_IPU_ERR_CHNID_UNEXIST,        /* channel unexists */
    E_IPU_ERR_NOMEM,        /* failure caused by malloc memory */
    E_IPU_ERR_NOBUF,            /* failure caused by malloc buffer */
    E_IPU_ERR_BADADDR,      /* bad address, buffer address doesn't get from IPU buffer allocator */
    E_IPU_ERR_SYS_TIMEOUT,  /* system timeout*/
    E_IPU_ERR_FILE_OPERATION,   /* file cannot be open or read or write */
    E_IPU_ERR_ILLEGAL_TENSOR_BUFFER_SIZE,   /* tensor buffer size cannot meet the requirement, usually less than requirement*/
    E_IPU_ERR_ILLEGAL_BUFFER_DEPTH,     /* input or output buffer depth quantum is more than maximum number */
    E_IPU_ERR_ILLEGAL_INPUT_OUTPUT_DESC,    /* network description is illegal, usually mean input or output buffer quantum is wrong */
    E_IPU_ERR_ILLEGAL_INPUT_OUTPUT_PARAM,   /* user's input or output buffer quantum isn't match network description */
    E_IPU_ERR_MAP,  /* address mapping error */
    E_IPU_ERR_INIT_FIRMWARE,    /* fail to init ipu firmware */
    E_IPU_ERR_CREATE_CHANNEL, /* fail to create channel */
    E_IPU_ERR_DESTROY_CHANNEL,  /* fail to destroy channel */
    E_IPU_ERR_INVOKE,   /* fail to invoke */
    E_IPU_ERR_SET_MALLOC_REGION,    /* fail to set malloc region for freertos */
    E_IPU_ERR_SET_IPU_PARAMETER,    /* fail to set IPU parameter */
    E_IPU_ERR_INVALID_PITCH_ALIGNMENT,  /* invalid pitch alignment */
    E_IPU_ERR_NO_CREATED_IPU_DEVICE,   /* there is no created IPU device */
    E_IPU_ERR_FAILED,   /* unexpected error */
    E_IPU_ERR_NO_AVAILABLE_CHNID = 1<<8,   /* there is no available channel */
}IPU_ErrCode_e;

#include "mi_ipu_datatype.h"

#define IPU_MAJOR_VERSION 1
#define IPU_SUB_VERSION 0
#define MACRO_TO_STR(macro) #macro
#define IPU_VERSION_STR(major_version,sub_version) ({char *tmp = sub_version/100 ? \
                                    "mi_ipu_version_" MACRO_TO_STR(major_version)"." MACRO_TO_STR(sub_version) : sub_version/10 ? \
                                    "mi_ipu_version_" MACRO_TO_STR(major_version)".0" MACRO_TO_STR(sub_version) : \
                                    "mi_ipu_version_" MACRO_TO_STR(major_version)".00" MACRO_TO_STR(sub_version);tmp;})
#define MI_IPU_API_VERSION IPU_VERSION_STR(IPU_MAJOR_VERSION,IPU_SUB_VERSION)

#ifdef __cplusplus
extern "C" {
#endif

MI_S32 MI_IPU_CreateDevice(MI_IPU_DevAttr_t *pstIPUDevAttr, SerializedReadFunc pReadFunc, char *pReadCtx, MI_U32 FWSize);
MI_S32 MI_IPU_DestroyDevice(void);
MI_S32 MI_IPU_CreateCHN(MI_IPU_CHN *ptChnId, MI_IPUChnAttr_t *pstIPUChnAttr,SerializedReadFunc pReadFunc, char *pReadCtx);
MI_S32 MI_IPU_DestroyCHN(MI_IPU_CHN u32ChnId);
MI_S32 MI_IPU_GetInOutTensorDesc(MI_IPU_CHN u32ChnId, MI_IPU_SubNet_InputOutputDesc_t *pstDesc);
MI_S32 MI_IPU_GetInputTensors(MI_IPU_CHN u32ChnId, MI_IPU_TensorVector_t *pstInputTensorVector);
MI_S32 MI_IPU_PutInputTensors(MI_IPU_CHN u32ChnId, MI_IPU_TensorVector_t *pstInputTensorVector);
MI_S32 MI_IPU_GetOutputTensors(MI_IPU_CHN u32ChnId, MI_IPU_TensorVector_t *pstInputTensorVector);
MI_S32 MI_IPU_PutOutputTensors(MI_IPU_CHN u32ChnId, MI_IPU_TensorVector_t *pstInputTensorVector);
MI_S32 MI_IPU_Invoke(MI_IPU_CHN u32ChnId, MI_IPU_TensorVector_t *pstInputTensorVector, MI_IPU_TensorVector_t *pstOuputTensorVector);


#ifdef __cplusplus
}
#endif

#endif
