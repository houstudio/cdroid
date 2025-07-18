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
#ifndef _MI_SYS_DATATYPE_H_
#define _MI_SYS_DATATYPE_H_

#include "mi_common.h"
#define MI_SYS_MAX_INPUT_PORT_CNT   (3)
#define MI_SYS_MAX_OUTPUT_PORT_CNT  (9)
#define MI_SYS_MAX_DEV_CHN_CNT      (72)
#define MI_SYS_INVLAID_SEQUENCE_NUM (~(MI_U32)0)
#define MI_SYS_MAX_SUB_PLANE_CNT    (8)
#define MI_SYS_MAX_METADATA_CNT (4)

// ensure that sizeof(MI_VB_PoolListConf_t)  is less that 4096 !!!
#define MI_MAX_MMA_HEAP_LENGTH  (32)
#define MI_VB_POOL_LIST_MAX_CNT (8)

#define MI_SYS_INVALID_PTS (~(MI_U64)0)

#define MI_VB_BLK_HANDLE_INVALID  (-1)
#define MI_VB_POOL_HANDLE_INVALID (-1)

#define MI_SYS_MAP_VA        0x8
#define MI_SYS_MAP_CPU_READ  0X2
#define MI_SYS_MAP_CPU_WRITE 0X1

#define MI_SYS_INVLAID_SEQUENCE_NUM (~(MI_U32)0)

#define MI_SYS_SUCCESS MI_SUCCESS // do not use MI_DEF_ERR(E_MI_MODULE_ID_SYS, E_MI_ERR_LEVEL_ERROR, MI_SUCCESS)   !!!!

/* SYS Module ErrorCode */
#define MI_ERR_SYS_INVALID_DEVID   MI_DEF_ERR(E_MI_MODULE_ID_SYS, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_INVALID_DEVID)
#define MI_ERR_SYS_INVALID_CHNID   MI_DEF_ERR(E_MI_MODULE_ID_SYS, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_INVALID_CHNID)
#define MI_ERR_SYS_ILLEGAL_PARAM   MI_DEF_ERR(E_MI_MODULE_ID_SYS, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_ILLEGAL_PARAM)
#define MI_ERR_SYS_EXIST           MI_DEF_ERR(E_MI_MODULE_ID_SYS, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_EXIST)
#define MI_ERR_SYS_UNEXIST         MI_DEF_ERR(E_MI_MODULE_ID_SYS, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_UNEXIST)
#define MI_ERR_SYS_NULL_PTR        MI_DEF_ERR(E_MI_MODULE_ID_SYS, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_NULL_PTR)
#define MI_ERR_SYS_NOT_CONFIG      MI_DEF_ERR(E_MI_MODULE_ID_SYS, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_NOT_CONFIG)
#define MI_ERR_SYS_NOT_SUPPORT     MI_DEF_ERR(E_MI_MODULE_ID_SYS, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_NOT_SUPPORT)
#define MI_ERR_SYS_NOT_PERM        MI_DEF_ERR(E_MI_MODULE_ID_SYS, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_NOT_PERM)
#define MI_ERR_SYS_NOMEM           MI_DEF_ERR(E_MI_MODULE_ID_SYS, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_NOMEM)
#define MI_ERR_SYS_NOBUF           MI_DEF_ERR(E_MI_MODULE_ID_SYS, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_NOBUF)
#define MI_ERR_SYS_BUF_EMPTY       MI_DEF_ERR(E_MI_MODULE_ID_SYS, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_BUF_EMPTY)
#define MI_ERR_SYS_BUF_FULL        MI_DEF_ERR(E_MI_MODULE_ID_SYS, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_BUF_FULL)
#define MI_ERR_SYS_SYS_NOTREADY    MI_DEF_ERR(E_MI_MODULE_ID_SYS, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_SYS_NOTREADY)
#define MI_ERR_SYS_BADADDR         MI_DEF_ERR(E_MI_MODULE_ID_SYS, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_BADADDR)
#define MI_ERR_SYS_BUSY            MI_DEF_ERR(E_MI_MODULE_ID_SYS, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_BUSY)
#define MI_ERR_SYS_CHN_NOT_STARTED MI_DEF_ERR(E_MI_MODULE_ID_SYS, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_CHN_NOT_STARTED)
#define MI_ERR_SYS_CHN_NOT_STOPED  MI_DEF_ERR(E_MI_MODULE_ID_SYS, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_CHN_NOT_STOPED)
#define MI_ERR_SYS_NOT_INIT        MI_DEF_ERR(E_MI_MODULE_ID_SYS, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_NOT_INIT)
#define MI_ERR_SYS_INITED          MI_DEF_ERR(E_MI_MODULE_ID_SYS, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_INITED)
#define MI_ERR_SYS_NOT_ENABLE      MI_DEF_ERR(E_MI_MODULE_ID_SYS, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_NOT_ENABLE)
#define MI_ERR_SYS_NOT_DISABLE     MI_DEF_ERR(E_MI_MODULE_ID_SYS, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_NOT_DISABLE)
#define MI_ERR_SYS_TIMEOUT         MI_DEF_ERR(E_MI_MODULE_ID_SYS, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_SYS_TIMEOUT)
#define MI_ERR_SYS_DEV_NOT_STARTED MI_DEF_ERR(E_MI_MODULE_ID_SYS, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_DEV_NOT_STARTED)
#define MI_ERR_SYS_DEV_NOT_STOPED  MI_DEF_ERR(E_MI_MODULE_ID_SYS, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_DEV_NOT_STOPED)
#define MI_ERR_SYS_CHN_NO_CONTENT  MI_DEF_ERR(E_MI_MODULE_ID_SYS, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_CHN_NO_CONTENT)
#define MI_ERR_SYS_NOVASAPCE       MI_DEF_ERR(E_MI_MODULE_ID_SYS, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_NOVASPACE)
#define MI_ERR_SYS_NOITEM          MI_DEF_ERR(E_MI_MODULE_ID_SYS, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_NOITEM)
#define MI_ERR_SYS_FAILED          MI_DEF_ERR(E_MI_MODULE_ID_SYS, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_FAILED)

typedef MI_HANDLE MI_VB_POOL_HANDLE;
typedef MI_HANDLE MI_VB_BLK_HANDLE;
typedef MI_HANDLE MI_SYS_BUF_HANDLE;

/*
****************************************************TODO Refine*****************************************
*/

#if !defined(TRUE) && !defined(FALSE)
/// definition for TRUE
#define TRUE 1
/// definition for FALSE
#define FALSE 0
#endif

typedef struct MI_VB_BufBlkInfo_s
{
    MI_VB_POOL_HANDLE poolHandle;
    MI_U32            u32OffsetInVBPool;
    MI_U32            u32BlkSize;
    MI_PHY            phySicalAddr;
    union
    {
        MI_PTR   pVirtualAddress;
        MI_PTR64 p64VirtualAddress;
    };
} MI_VB_BufBlkInfo_t;

typedef struct MI_VB_PoolConf_s
{
    MI_U32 u32BlkSize;
    MI_U32 u32BlkCnt;
    MI_U8  u8MMAHeapName[MI_MAX_MMA_HEAP_LENGTH];
} MI_VB_PoolConf_t;

typedef struct MI_VB_PoolListConf_s
{
    MI_U32           u32PoolListCnt;
    MI_VB_PoolConf_t stPoolConf[MI_VB_POOL_LIST_MAX_CNT];
} MI_VB_PoolListConf_t;

typedef enum
{
    E_MI_SYS_DATA_PRECISION_8BPP,
    E_MI_SYS_DATA_PRECISION_10BPP,
    E_MI_SYS_DATA_PRECISION_12BPP,
    E_MI_SYS_DATA_PRECISION_14BPP,
    E_MI_SYS_DATA_PRECISION_16BPP,
    E_MI_SYS_DATA_PRECISION_MAX,
} MI_SYS_DataPrecision_e;

typedef enum
{
    E_MI_SYS_PIXEL_BAYERID_RG,
    E_MI_SYS_PIXEL_BAYERID_GR,
    E_MI_SYS_PIXEL_BAYERID_BG,
    E_MI_SYS_PIXEL_BAYERID_GB,
    E_MI_SYS_PIXEL_RGBIR_R0,
    E_MI_SYS_PIXEL_RGBIR_G0,
    E_MI_SYS_PIXEL_RGBIR_B0,
    E_MI_SYS_PIXEL_RGBIR_G1,
    E_MI_SYS_PIXEL_RGBIR_G2,
    E_MI_SYS_PIXEL_RGBIR_I0,
    E_MI_SYS_PIXEL_RGBIR_G3,
    E_MI_SYS_PIXEL_RGBIR_I1,
    E_MI_SYS_PIXEL_BAYERID_MAX,
} MI_SYS_BayerId_e;

typedef enum
{
    E_MI_SYS_PIXEL_FRAME_YUV422_YUYV = 0,
    E_MI_SYS_PIXEL_FRAME_ARGB8888,
    E_MI_SYS_PIXEL_FRAME_ABGR8888,
    E_MI_SYS_PIXEL_FRAME_BGRA8888,

    E_MI_SYS_PIXEL_FRAME_RGB565,
    E_MI_SYS_PIXEL_FRAME_ARGB1555,
    E_MI_SYS_PIXEL_FRAME_ARGB4444,
    E_MI_SYS_PIXEL_FRAME_I2,
    E_MI_SYS_PIXEL_FRAME_I4,
    E_MI_SYS_PIXEL_FRAME_I8,

    E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_422,
    E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420,
    E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420_NV21,
    E_MI_SYS_PIXEL_FRAME_YUV_TILE_420,
    E_MI_SYS_PIXEL_FRAME_YUV422_UYVY,
    E_MI_SYS_PIXEL_FRAME_YUV422_YVYU,
    E_MI_SYS_PIXEL_FRAME_YUV422_VYUY,

    E_MI_SYS_PIXEL_FRAME_YUV422_PLANAR,
    E_MI_SYS_PIXEL_FRAME_YUV420_PLANAR,

    E_MI_SYS_PIXEL_FRAME_FBC_420,

    E_MI_SYS_PIXEL_FRAME_RGB_BAYER_BASE,
    E_MI_SYS_PIXEL_FRAME_RGB_BAYER_NUM =
        E_MI_SYS_PIXEL_FRAME_RGB_BAYER_BASE + E_MI_SYS_DATA_PRECISION_MAX * E_MI_SYS_PIXEL_BAYERID_MAX - 1,

    E_MI_SYS_PIXEL_FRAME_RGB888,
    E_MI_SYS_PIXEL_FRAME_BGR888,
    E_MI_SYS_PIXEL_FRAME_GRAY8,
    E_MI_SYS_PIXEL_FRAME_RGB101010,
    E_MI_SYS_PIXEL_FRAME_RGB888_PLANAR,

    E_MI_SYS_PIXEL_FRAME_FORMAT_MAX,
} MI_SYS_PixelFormat_e;

#define RGB_BAYER_PIXEL(BitMode, PixelID) \
    (E_MI_SYS_PIXEL_FRAME_RGB_BAYER_BASE + BitMode * E_MI_SYS_PIXEL_BAYERID_MAX + PixelID)

typedef enum
{
    E_MI_SYS_COMPRESS_MODE_NONE,  // no compress
    E_MI_SYS_COMPRESS_MODE_SEG,   // compress unit is 256 bytes as a segment
    E_MI_SYS_COMPRESS_MODE_LINE,  // compress unit is the whole line
    E_MI_SYS_COMPRESS_MODE_FRAME, // compress unit is the whole frame
    E_MI_SYS_COMPRESS_MODE_TO_8BIT,
    E_MI_SYS_COMPRESS_MODE_TO_6BIT,
    E_MI_SYS_COMPRESS_MODE_IFC,
    E_MI_SYS_COMPRESS_MODE_SFBC0,
    E_MI_SYS_COMPRESS_MODE_SFBC1,
    E_MI_SYS_COMPRESS_MODE_SFBC2,
    E_MI_SYS_COMPRESS_MODE_BUTT,  // number
} MI_SYS_CompressMode_e;

typedef enum
{
    E_MI_SYS_FRAME_SCAN_MODE_PROGRESSIVE = 0x0, // progessive.
    E_MI_SYS_FRAME_SCAN_MODE_INTERLACE   = 0x1, // interlace.
    E_MI_SYS_FRAME_SCAN_MODE_MAX,
} MI_SYS_FrameScanMode_e;

typedef enum
{
    E_MI_SYS_FRAME_TILE_MODE_NONE = 0,
    E_MI_SYS_FRAME_TILE_MODE_16x16, // tile mode 16x16
    E_MI_SYS_FRAME_TILE_MODE_16x32, // tile mode 16x32
    E_MI_SYS_FRAME_TILE_MODE_32x16, // tile mode 32x16
    E_MI_SYS_FRAME_TILE_MODE_32x32, // tile mode 32x32
    E_MI_SYS_FRAME_TILE_MODE_128x32, // tile mode 128x32
    E_MI_SYS_FRAME_TILE_MODE_128x64, // tile mode 128x64
    E_MI_SYS_FRAME_TILE_MODE_64x4,   // tile mode 64x4
    E_MI_SYS_FRAME_TILE_MODE_MAX
} MI_SYS_FrameTileMode_e;

typedef enum
{
    E_MI_SYS_FIELDTYPE_NONE,   //< no field.
    E_MI_SYS_FIELDTYPE_TOP,    //< Top field only.
    E_MI_SYS_FIELDTYPE_BOTTOM, //< Bottom field only.
    E_MI_SYS_FIELDTYPE_BOTH,   //< Both fields.
    E_MI_SYS_FIELDTYPE_NUM
} MI_SYS_FieldType_e;

typedef enum
{
    E_MI_SYS_BUFDATA_RAW = 0,
    E_MI_SYS_BUFDATA_FRAME,
    E_MI_SYS_BUFDATA_META,
    E_MI_SYS_BUFDATA_MULTIPLANE,
    E_MI_SYS_BUFDATA_FBC,
    E_MI_SYS_BUFDATA_MAX
} MI_SYS_BufDataType_e;

typedef enum
{
    E_MI_SYS_ROTATE_NONE, // Rotate 0 degrees
    E_MI_SYS_ROTATE_90,   // Rotate 90 degrees
    E_MI_SYS_ROTATE_180,  // Rotate 180 degrees
    E_MI_SYS_ROTATE_270,  // Rotate 270 degrees
    E_MI_SYS_ROTATE_NUM,
} MI_SYS_Rotate_e;

typedef enum
{
    E_MI_SYS_BIND_TYPE_FRAME_BASE     = 0x00000001,
    E_MI_SYS_BIND_TYPE_SW_LOW_LATENCY = 0x00000002,
    E_MI_SYS_BIND_TYPE_REALTIME       = 0x00000004,
    E_MI_SYS_BIND_TYPE_HW_AUTOSYNC    = 0x00000008,
    E_MI_SYS_BIND_TYPE_HW_RING        = 0x00000010
} MI_SYS_BindType_e;

typedef enum
{
    E_MI_SYS_VPE_TO_VENC_PRIVATE_RING_POOL = 0,
    E_MI_SYS_PER_CHN_PRIVATE_POOL          = 1,
    E_MI_SYS_PER_DEV_PRIVATE_POOL          = 2,
    E_MI_SYS_PER_CHN_PORT_OUTPUT_POOL      = 3,
    E_MI_SYS_PER_DEV_PRIVATE_RING_POOL     = 4,
} MI_SYS_InsidePrivatePoolType_e;

typedef enum
{
    E_MI_SYS_FRAME_ISP_INFO_TYPE_NONE,
    E_MI_SYS_FRAME_ISP_INFO_TYPE_GLOBAL_GRADIENT
} MI_SYS_FrameIspInfoType_e;

typedef enum
{
    E_MI_SYS_MEMCPY_DRAM_TO_DRAM,
    E_MI_SYS_MEMCPY_DRAM_TO_PM_SRAM,
    E_MI_SYS_MEMCPY_PM_SRAM_TO_DRAM,
    E_MI_SYS_MEMCPY_DRAM_TO_PM_PSRAM,
    E_MI_SYS_MEMCPY_PM_PSRAM_TO_DRAM,
    E_MI_SYS_MEMCPY_DIRECT_MAX,
} MI_SYS_MemcpyDirect_e;

typedef struct MI_SYS_ChnPort_s
{
    MI_ModuleId_e eModId;
    MI_U32        u32DevId;
    MI_U32        u32ChnId;
    MI_U32        u32PortId;

} MI_SYS_ChnPort_t;

typedef struct MI_SYS_WindowRect_s
{
    MI_U16 u16X;
    MI_U16 u16Y;
    MI_U16 u16Width;
    MI_U16 u16Height;
} MI_SYS_WindowRect_t;

typedef struct MI_SYS_WindowSize_s
{
    MI_U16 u16Width;
    MI_U16 u16Height;
} MI_SYS_WindowSize_t;

typedef struct MI_SYS_RawData_s
{
    union
    {
        MI_PTR   pVirAddr;
        MI_PTR64 p64VirAddr;
    };
    MI_PHY phyAddr; // notice that this is miu bus addr,not cpu bus addr.
    MI_U32 u32BufSize;

    MI_U32  u32ContentSize;
    MI_BOOL bEndOfFrame;
    MI_U64  u64SeqNum;
} MI_SYS_RawData_t;

typedef struct MI_SYS_MetaData_s
{
    union
    {
        MI_PTR   pVirAddr;
        MI_PTR64 p64VirAddr;
    };
    MI_PHY phyAddr; // notice that this is miu bus addr,not cpu bus addr.

    MI_U32        u32Size;
    MI_U32        u32ExtraData; /*driver special flag*/
    MI_ModuleId_e eDataFromModule;
} MI_SYS_MetaData_t;

typedef enum
{
    REALTIME_FRAME_DATA,
    RINGBUF_FRAME_DATA,
    NORMAL_FRAME_DATA,
    AUTOSYNC_FRAME_DATA,
} MI_SYS_FrameData_PhySignalType;

#define MI_SYS_REALTIME_MAGIC_PADDR ((MI_PHY)0x46414B45) //"FAKE"
#define MI_SYS_REALTIME_MAGIC_VADDR ((void*)0x46414B45)  //"FAKE"
#define MI_SYS_REALTIME_MAGIC_PITCH ((MI_U32)0x46414B45) //"FAKE"

typedef struct MI_SYS_FrameIspInfo_s
{
    MI_SYS_FrameIspInfoType_e eType;
    union
    {
        MI_U32 u32GlobalGradient;
    } uIspInfo;
} MI_SYS_FrameIspInfo_t;

typedef struct MI_SYS_FbcData_s
{
    MI_SYS_FrameTileMode_e eTileMode;
    MI_SYS_PixelFormat_e   ePixelFormat;
    MI_SYS_CompressMode_e  eCompressMode;

    MI_SYS_FrameData_PhySignalType ePhylayoutType;

    MI_U16 u16Width;
    MI_U16 u16Height;

    union
    {
        MI_PTR   pVirAddr[2];
        MI_PTR64 p64VirAddr[2];
    };
    MI_PHY phyAddr[2]; // notice that this is miu bus addr,not cpu bus addr.
    MI_U32 u32Stride[2];
    MI_U32 u32BufSize; // total size that allocated for this buffer,include consider alignment.

    /*
     *                           ----------------- ----------------
     *                           |               |              ^
     *                           |               |              |
     *   u16RingBufStartLine --> |               |----          |
     *                           |               | ^            |
     *                           |     Ring      | |            |
     *                           |               | u16Height    |
     *                           |     Heap      | |            |
     *                           |               | |       u16RingHeapTotalLines
     *                           |               | v            |
     *   u16RingBufEndLine -->   |               |----          |
     *                           |               |              |
     *                           |               |              v
     *                           ----------------- ----------------
     */

    MI_U16 u16RingBufStartLine;
    MI_U16 u16RingBufEndLine;
    MI_U16 u16RingHeapTotalLines;

    MI_PHY phyFbcTableAddr[2];
    MI_U32 u32FbcTableSize[2];

    MI_SYS_FrameIspInfo_t stFrameIspInfo;
} MI_SYS_FbcData_t;

// N.B. in MI_SYS_FrameData_t should never support u32Size,
// for other values are enough,and not support u32Size is general standard method.
typedef struct MI_SYS_FrameData_s
{
    MI_SYS_FrameTileMode_e         eTileMode;
    MI_SYS_PixelFormat_e           ePixelFormat;
    MI_SYS_CompressMode_e          eCompressMode;
    MI_SYS_FrameScanMode_e         eFrameScanMode;
    MI_SYS_FieldType_e             eFieldType;
    MI_SYS_FrameData_PhySignalType ePhylayoutType;

    MI_U16 u16Width;
    MI_U16 u16Height;
    // in case ePhylayoutType equal to REALTIME_FRAME_DATA, pVirAddr would be MI_SYS_REALTIME_MAGIC_PADDR and phyAddr
    // would be MI_SYS_REALTIME_MAGIC_VADDR

    union
    {
        MI_PTR   pVirAddr[3];
        MI_PTR64 p64VirAddr[3];
    };
    MI_PHY phyAddr[3]; // notice that this is miu bus addr,not cpu bus addr.
    MI_U32 u32Stride[3];
    MI_U32 u32BufSize; // total size that allocated for this buffer,include consider alignment.

    MI_U16
    u16RingBufRealTotalHeight; /// Valid in case RINGBUF_FRAME_DATA, u16RingBufRealTotalHeight must be LGE than u16Height

    MI_SYS_FrameIspInfo_t stFrameIspInfo; // isp info of each frame
    MI_SYS_WindowRect_t   stContentCropWindow;

    MI_U32                     u32MetaDataTypeMask;
    MI_PHY                     phyMetaDataAddr[MI_SYS_MAX_METADATA_CNT];
    MI_U32                     u32MetaDataSize[MI_SYS_MAX_METADATA_CNT];
} MI_SYS_FrameData_t;

typedef struct MI_SYS_FrameDataSubPlane_s
{
    MI_SYS_PixelFormat_e  ePixelFormat;
    MI_SYS_CompressMode_e eCompressMode;
    MI_U64                u64FrameId;

    MI_U16 u16Width;
    MI_U16 u16Height;
    union
    {
        MI_PTR   pVirAddr[2];
        MI_PTR64 p64VirAddr[2];
    };
    MI_PHY phyAddr[2];
    MI_U16 u16Stride[2];
    MI_U32 u32BufSize;
    MI_SYS_FrameData_PhySignalType ePhySignalType;
} MI_SYS_FrameDataSubPlane_t;

typedef struct MI_SYS_FrameDataMultiPlane_s
{
    MI_U8                      u8SubPlaneNum;
    MI_SYS_FrameDataSubPlane_t stSubPlanes[MI_SYS_MAX_SUB_PLANE_CNT];
} MI_SYS_FrameDataMultiPlane_t;

typedef struct MI_SYS_BufInfo_s
{
    MI_U64               u64Pts;
    MI_U64               u64SidebandMsg;
    MI_SYS_BufDataType_e eBufType;
    MI_U32               u32SequenceNumber;
    MI_U32 bEndOfStream : 1;
    MI_U32 bUsrBuf : 1;
    MI_U32 bDrop : 1;
    MI_U32 bCrcCheck : 1;
    MI_U32 u32IrFlag : 2; // For Window Hello Usage: 0x00/off, 0x01/on, 0x02/invalid
    MI_U32 u32Reserved : 26;
    union
    {
        MI_SYS_FrameData_t           stFrameData;
        MI_SYS_RawData_t             stRawData;
        MI_SYS_MetaData_t            stMetaData;
        MI_SYS_FrameDataMultiPlane_t stFrameDataMultiPlane;
        MI_SYS_FbcData_t             stFbcData;
    };
    MI_ModuleId_e              eDataSource;
} MI_SYS_BufInfo_t;

typedef struct MI_SYS_FrameBufExtraConfig_s
{
    // Buf alignment requirement in horizontal
    MI_U16 u16BufHAlignment;
    // Buf alignment requirement in vertical
    MI_U16 u16BufVAlignment;
    // Buf alignment requirement in chroma
    MI_U16 u16BufChromaAlignment;
    // Buf alignment requirement after compress
    MI_U16 u16BufCompressAlignment;
    // Buf Extra add after alignment
    MI_U16 u16BufExtraSize;
    // Clear padding flag
    MI_BOOL bClearPadding;
} MI_SYS_FrameBufExtraConfig_t;

typedef struct MI_SYS_BufFrameConfig_s
{
    MI_U16                 u16Width;
    MI_U16                 u16Height;
    MI_SYS_FrameScanMode_e eFrameScanMode; //
    MI_SYS_PixelFormat_e   eFormat;
    MI_SYS_CompressMode_e  eCompressMode;
    MI_U32                 u32MetaDataTypeMask;
    MI_U32                 u32MetaDataSize[MI_SYS_MAX_METADATA_CNT];
} MI_SYS_BufFrameConfig_t;

typedef struct MI_SYS_BufRawConfig_s
{
    MI_U32 u32Size;
} MI_SYS_BufRawConfig_t;

typedef struct MI_SYS_MetaDataConfig_s
{
    MI_U32 u32Size;
} MI_SYS_MetaDataConfig_t;

typedef struct MI_SYS_BufFrameMultiPlaneConfig_s
{
    MI_U8                   u8SubPlaneNum;
    MI_SYS_BufFrameConfig_t stFrameCfgs[MI_SYS_MAX_SUB_PLANE_CNT];
} MI_SYS_BufFrameMultiPlaneConfig_t;

typedef struct MI_SYS_BufFrameMetaConfig_s
{
    MI_U16                 u16Width;
    MI_U16                 u16Height;
    MI_SYS_FrameScanMode_e eFrameScanMode; //
    MI_SYS_PixelFormat_e   eFormat;
    MI_SYS_CompressMode_e  eCompressMode;

    union
    {
        MI_PTR   pVirAddr[3];
        MI_PTR64 p64VirAddr[3];
    };
    MI_PHY phyAddr[3];
    MI_U16 u32Stride[3];
    MI_U32 u32BufSize;
} MI_SYS_BufFrameMetaConfig_t;

typedef MI_SYS_FbcData_t MI_SYS_FbcDataConfig_t;

typedef struct MI_SYS_BufConf_s
{
    MI_SYS_BufDataType_e eBufType;
    MI_U32               u32Flags; // 0 or MI_SYS_MAP_VA
    MI_U64               u64TargetPts;
    MI_BOOL              bDirectBuf; // Direct alloc buffer by Others.
    MI_BOOL              bCrcCheck;
    union
    {
        MI_SYS_BufFrameConfig_t           stFrameCfg;
        MI_SYS_BufRawConfig_t             stRawCfg;
        MI_SYS_MetaDataConfig_t           stMetaCfg;
        MI_SYS_BufFrameMultiPlaneConfig_t stMultiPlaneCfg;
        MI_SYS_BufFrameMetaConfig_t       stFrameMetaCfg; // support frame buffer has alloced by Others.
        MI_SYS_FbcDataConfig_t            stFbcCfg;
    };
} MI_SYS_BufConf_t;

#define MI_SYS_DMABUF_STATUS_INVALID 0x00000001
#define MI_SYS_DMABUF_STATUS_DROP    0x00000002
#define MI_SYS_DMABUF_STATUS_DONE    0x00000004

typedef MI_SYS_FrameIspInfo_t MI_SYS_FrameExtInfo_t;

typedef struct MI_SYS_DmaBufInfo_s
{
    MI_U16 u16Width;
    MI_U16 u16Height;

    MI_U32 bEndOfStream;
    MI_U32 u32SequenceNumber;
    MI_U64 u64Pts;

    MI_SYS_PixelFormat_e   eFormat;
    MI_SYS_CompressMode_e  eCompressMode;
    MI_SYS_FrameScanMode_e eFrameScanMode;
    MI_SYS_FrameTileMode_e eTileMode;

    MI_SYS_WindowRect_t   stContentCropWindow;
    MI_SYS_FrameExtInfo_t stFrameExtInfo;

    MI_S32 s32Fd[3];
    MI_U32 u32Stride[3];
    MI_U32 u32DataOffset[3];

    MI_U32 u32Status;
} MI_SYS_DmaBufInfo_t;

typedef struct MI_SYS_Version_s
{
    MI_U8 u8Version[128];
} MI_SYS_Version_t;

typedef struct MI_PerChnPrivHeapConf_s
{
    MI_ModuleId_e eModule;
    MI_U32        u32Devid;
    MI_U32        u32Channel;
    MI_U8         u8MMAHeapName[MI_MAX_MMA_HEAP_LENGTH];
    MI_U32        u32PrivateHeapSize;
} MI_SYS_PerChnPrivHeapConf_t;

typedef struct MI_PerDevPrivHeapConf_s
{
    MI_ModuleId_e eModule;
    MI_U32        u32Devid;
    MI_U32        u32Reserve;
    MI_U8         u8MMAHeapName[MI_MAX_MMA_HEAP_LENGTH];
    MI_U32        u32PrivateHeapSize;
} MI_SYS_PerDevPrivHeapConf_t;

typedef struct MI_SYS_PerVpe2VencRingPoolConf_s
{
    MI_U32 u32VencInputRingPoolStaticSize;
    MI_U8  u8MMAHeapName[MI_MAX_MMA_HEAP_LENGTH];
} MI_SYS_PerVpe2VencRingPoolConf_t;

typedef struct MI_SYS_PerChnPortOutputPool_s
{
    MI_ModuleId_e eModule;
    MI_U32        u32Devid;
    MI_U32        u32Channel;
    MI_U32        u32Port;
    MI_U8         u8MMAHeapName[MI_MAX_MMA_HEAP_LENGTH];
    MI_U32        u32PrivateHeapSize;
} MI_SYS_PerChnPortOutputPool_t;

typedef struct MI_SYS_PerDevPrivRingPoolConf_s
{
    MI_ModuleId_e eModule;
    MI_U32        u32Devid;
    MI_U16        u16MaxWidth;
    MI_U16        u16MaxHeight;
    MI_U16        u16RingLine;
    MI_U8         u8MMAHeapName[MI_MAX_MMA_HEAP_LENGTH];
}MI_SYS_PerDevPrivRingPoolConf_t;

typedef struct MI_SYS_GlobalPrivPoolConfig_s
{
    MI_SYS_InsidePrivatePoolType_e eConfigType;
    MI_BOOL                        bCreate;
    union
    {
        MI_SYS_PerChnPrivHeapConf_t      stPreChnPrivPoolConfig;
        MI_SYS_PerDevPrivHeapConf_t      stPreDevPrivPoolConfig;
        MI_SYS_PerVpe2VencRingPoolConf_t stPreVpe2VencRingPrivPoolConfig;
        MI_SYS_PerChnPortOutputPool_t    stPreChnPortOutputPrivPool;
        MI_SYS_PerDevPrivRingPoolConf_t  stpreDevPrivRingPoolConfig;
    } uConfig;
} MI_SYS_GlobalPrivPoolConfig_t;

typedef struct MI_SYS_InitParam_s
{
    MI_U32 u32DevId;
    MI_U8* u8Data;
} MI_SYS_InitParam_t;

typedef struct MI_SYS_UserPictureInfo_s
{
    MI_U32 u32SrcFrc;
} MI_SYS_UserPictureInfo_t;

typedef struct MI_SYS_ChnPortState_s
{
    MI_BOOL bEnable;
} MI_SYS_ChnPortState_t;

typedef enum
{
    E_MI_SYS_FRC_STRATEGY_BY_RATIO,
    E_MI_SYS_FRC_STRATEGY_BY_PERIOD,
    E_MI_SYS_FRC_STRATEGY_BY_PTS,
    E_MI_SYS_FRC_STRATEGY_MAX,
} MI_SYS_FrcStrategy_e;

typedef enum
{
    E_MI_SYS_FRC_TYPE_PIPELINE,
    E_MI_SYS_FRC_TYPE_USERINJECT,
    E_MI_SYS_FRC_TYPE_MAX,
} MI_SYS_FrcType_e;

typedef struct MI_SYS_ChnPortFrcAttr_s
{
    MI_SYS_FrcType_e eType;
    MI_SYS_FrcStrategy_e eStrategy;

    MI_U32 u32SrcFrameRate;
    MI_U32 u32DstFrameRate;
} MI_SYS_ChnPortFrcAttr_t;

typedef struct MI_SYS_BindAttr_s
{
    MI_SYS_BindType_e eBindType;
} MI_SYS_BindAttr_t;

typedef struct MI_SYS_SidebandData_s
{
    union
    {
        MI_PTR   pVirAddr;
        MI_PTR64 p64VirAddr;
    };
    MI_U32       u32Size;
} MI_SYS_SidebandData_t;

enum __anonEnumSizeCheck
{
    __E_ANON_ENUM_SIZE_CHECK_NONE
};
#ifdef __cplusplus
static_assert
#else
_Static_assert
#endif
    (sizeof(enum __anonEnumSizeCheck) == sizeof(unsigned int), "sizeof(enum) must be 32bit!!!");



#endif ///_MI_SYS_DATATYPE_H_
