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

#ifndef _MI_IPU_DATATYPE_H_
#define _MI_IPU_DATATYPE_H_

#include "mi_common.h"
#include "mi_sys_datatype.h"
#include "cam_os_wrapper.h"

typedef struct MI_IPU_FW_s {
    MI_U8 *pcharFWName;
    MI_U8 *pcharFWVersion;
    MI_U32 u32FWSize;
    MI_PHY pFWBuf;
    void *pVirAddrFW;
    MI_U8 charURL[128];
    SerializedReadFunc pFileReader;
    MI_PHY variableTensorBufPhyAddr;
    MI_PHY heap_start, heap_end;
} MI_IPU_FW_t;

typedef struct MI_IPU_TensorDesc_s {
    MI_U32 u32TensorDim;
    MI_IPU_ELEMENT_FORMAT eElmFormat;
    MI_U32 u32TensorShape[MI_IPU_MAX_TENSOR_DIM]; // 3,299,299
    MI_S8 name[MAX_TENSOR_NAME_LEN];
} MI_IPU_TensorDesc_t;

typedef struct MI_IPU_SubNet_InputOutputDesc_s {
    MI_U32 u32InputTensorCount;
    MI_U32 u32OutputTensorCount;
    MI_IPU_TensorDesc_t astMI_InputTensorDescs[MI_IPU_MAX_INPUT_TENSOR_CNT];
    MI_IPU_TensorDesc_t astMI_OutputTensorDescs[MI_IPU_MAX_OUTPUT_TENSOR_CNT];
} MI_IPU_SubNet_InputOutputDesc_t;

typedef struct MI_IPU_Tensor_s {
    //MI_IPU_TensorShape_t *pstTensorShape;
    //MI_BOOL bRewiseRGBOrder;//only valid under U8 & C=3,4 or NV12
    void      *ptTensorData[2];
    MI_PHY  phyTensorAddr[2];//notice that this is miu bus addr,not cpu bus addr.
} MI_IPU_Tensor_t;

typedef struct MI_IPU_TensorVector_s {
    MI_U32 u32TensorCount;
    MI_IPU_Tensor_t  astArrayTensors[MI_IPU_MAX_TENSOR_CNT];
} MI_IPU_TensorVector_t;

typedef struct MI_IPU_DevAttr_s {
    MI_U32 u32MaxVariableBufSize;
    MI_U32 u32YUV420_W_Pitch_Alignment; //default is 16
    MI_U32 u32YUV420_H_Pitch_Alignment; //default is 2
    MI_U32 u32XRGB_W_Pitch_Alignment;   //default is 16
} MI_IPU_DevAttr_t;

typedef struct MI_IPUChnAttr_s {
    MI_U32 u32SubNetId;
    MI_U32 u32OutputBufDepth;
    MI_U32 u32InputBufDepth;
} MI_IPUChnAttr_t;

typedef struct MI_IPU_Dev_s {
    MI_IPU_FW_t stFW;
    MI_IPU_DevAttr_t stDevAttr;
    CamOsMutex_t CHNLock;
    MI_BOOL beCreated;
} MI_IPU_Dev_t;

typedef struct MI_IPU_FirmwareAttr_s {
    MI_IPU_FW_t stFW;
    MI_IPU_DevAttr_t stDevAttr;
} MI_IPU_FirmwareAttr_t;


#endif // !_MI_IPU_DATATYPE_H_
