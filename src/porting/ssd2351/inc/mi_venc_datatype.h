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
#ifndef __MI_VENC_DATATYPE_
#define __MI_VENC_DATATYPE_

#include "mi_common_datatype.h"
#include "mi_sys_datatype.h"

// device id
#define MI_VENC_DEV_ID_H264_H265_0 0
#define MI_VENC_DEV_ID_H264_H265_1 (MI_VENC_DEV_ID_H264_H265_0 + 1)
#define MI_VENC_DEV_ID_JPEG_0      (8)
#define MI_VENC_DEV_ID_JPEG_1      (MI_VENC_DEV_ID_JPEG_0 + 1)

#define VENC_CUST_MAP_NUM          (2)

#define MI_DEF_VENC_ERR(err) MI_DEF_ERR(E_MI_MODULE_ID_VENC, E_MI_ERR_LEVEL_ERROR, err)
/* invalid device ID */
#define MI_ERR_VENC_INVALID_DEVID MI_DEF_VENC_ERR(E_MI_ERR_INVALID_DEVID)
/* invalid channel ID */
#define MI_ERR_VENC_INVALID_CHNID MI_DEF_VENC_ERR(E_MI_ERR_INVALID_CHNID)
/* at lease one parameter is illegal, e.g, an illegal enumeration value  */
#define MI_ERR_VENC_ILLEGAL_PARAM MI_DEF_VENC_ERR(E_MI_ERR_ILLEGAL_PARAM)
/* channel exists */
#define MI_ERR_VENC_EXIST MI_DEF_VENC_ERR(E_MI_ERR_EXIST)
/*UN exist*/
#define MI_ERR_VENC_UNEXIST MI_DEF_VENC_ERR(E_MI_ERR_UNEXIST)
/* using a NULL point */
#define MI_ERR_VENC_NULL_PTR MI_DEF_VENC_ERR(E_MI_ERR_NULL_PTR)
/* try to enable or initialize system,device or channel, before configuring attribute */
#define MI_ERR_VENC_NOT_CONFIG MI_DEF_VENC_ERR(E_MI_ERR_NOT_CONFIG)
/* operation is not supported by NOW */
#define MI_ERR_VENC_NOT_SUPPORT MI_DEF_VENC_ERR(E_MI_ERR_NOT_SUPPORT)
/* operation is not permitted, e.g, try to change static attribute */
#define MI_ERR_VENC_NOT_PERM MI_DEF_VENC_ERR(E_MI_ERR_NOT_PERM)
/* failure caused by malloc memory */
#define MI_ERR_VENC_NOMEM MI_DEF_VENC_ERR(E_MI_ERR_NOMEM)
/* failure caused by malloc buffer */
#define MI_ERR_VENC_NOBUF MI_DEF_VENC_ERR(E_MI_ERR_NOBUF)
/* no data in buffer */
#define MI_ERR_VENC_BUF_EMPTY MI_DEF_VENC_ERR(E_MI_ERR_BUF_EMPTY)
/* no buffer for new data */
#define MI_ERR_VENC_BUF_FULL MI_DEF_VENC_ERR(E_MI_ERR_BUF_FULL)
/* System is not ready,maybe not initialed or loaded.
 * Returning the error code when opening a device file failed.
 */
#define MI_ERR_VENC_NOTREADY MI_DEF_VENC_ERR(E_MI_ERR_SYS_NOTREADY)

/* bad address, e.g. used for copy_from_user & copy_to_user */
#define MI_ERR_VENC_BADADDR MI_DEF_VENC_ERR(E_MI_ERR_BADADDR)
/* resource is busy, e.g. destroy a VENC channel without unregistering it */
#define MI_ERR_VENC_BUSY MI_DEF_VENC_ERR(E_MI_ERR_BUSY)

/* channel not start*/
#define MI_ERR_VENC_CHN_NOT_STARTED MI_DEF_VENC_ERR(E_MI_ERR_CHN_NOT_STARTED)
/* channel not stop*/
#define MI_ERR_VENC_CHN_NOT_STOPPED MI_DEF_VENC_ERR(E_MI_ERR_CHN_NOT_STOPED)

#define MI_ERR_VENC_PRIVATE_START MI_DEF_VENC_ERR(MI_VENC_INITIAL_ERROR_CODE)
/* to be removed later */
#define MI_ERR_VENC_UNDEFINED MI_DEF_VENC_ERR(E_MI_ERR_FAILED)


/* =======================================================
 * Missing definitions
 * =======================================================*/
#define RC_TEXTURE_THR_SIZE 1 // Fixme

/* =======================================================
 * Data Types
 * =======================================================*/

//==== Enumerates ====
typedef enum
{
    E_MI_VENC_MODTYPE_VENC = 1, //!< E_MI_VENC_MODTYPE_VENC
    E_MI_VENC_MODTYPE_H264E,    //!< E_MI_VENC_MODTYPE_H264E
    E_MI_VENC_MODTYPE_H265E,    //!< E_MI_VENC_MODTYPE_H265E
    E_MI_VENC_MODTYPE_JPEGE,    //!< E_MI_VENC_MODTYPE_JPEGE
    E_MI_VENC_MODTYPE_AV1,      //!< E_MI_VENC_MODTYPE_AV1
    E_MI_VENC_MODTYPE_MAX       //!< E_MI_VENC_MODTYPE_MAX
} MI_VENC_ModType_e;

typedef enum
{
    E_MI_VENC_BASE_IDR = 0,
    E_MI_VENC_BASE_P_REFTOIDR,
    E_MI_VENC_BASE_P_REFBYBASE,
    E_MI_VENC_BASE_P_REFBYENHANCE,
    E_MI_VENC_ENHANCE_P_REFBYENHANCE,
    E_MI_VENC_ENHANCE_P_NOTFORREF,
    E_MI_VENC_BASE_SWITCH_FRAME,
    E_MI_VENC_REF_TYPE_MAX
} MI_VENC_H264eRefType_e;
typedef MI_VENC_H264eRefType_e MI_VENC_H265eRefType_e;
typedef MI_VENC_H264eRefType_e MI_VENC_Av1eRefType_e;

typedef enum
{
    E_MI_VENC_H264E_NALU_PSLICE  = 1,
    E_MI_VENC_H264E_NALU_ISLICE  = 5,
    E_MI_VENC_H264E_NALU_SEI     = 6,
    E_MI_VENC_H264E_NALU_SPS     = 7,
    E_MI_VENC_H264E_NALU_PPS     = 8,
    E_MI_VENC_H264E_NALU_IPSLICE = 9,
    E_MI_VENC_H264E_NALU_PREFIX  = 14,
    E_MI_VENC_H264E_NALU_MAX
} MI_VENC_H264eNaluType_e;

typedef enum
{
    E_MI_VENC_H264E_REFSLICE_FOR_1X = 1,
    E_MI_VENC_H264E_REFSLICE_FOR_2X = 2,
    E_MI_VENC_H264E_REFSLICE_FOR_4X,
    E_MI_VENC_H264E_REFSLICE_FOR_MAX = 5
} MI_VENC_H264eRefSliceType_e;

typedef enum
{
    E_MI_VENC_JPEGE_PACK_ECS = 5,
    E_MI_VENC_JPEGE_PACK_APP = 6,
    E_MI_VENC_JPEGE_PACK_VDO = 7,
    E_MI_VENC_JPEGE_PACK_PIC = 8,
    E_MI_VENC_JPEGE_PACK_MAX
} MI_VENC_JpegePackType_e;

typedef enum
{
    E_MI_VENC_H265E_NALU_PSLICE = 1,
    E_MI_VENC_H265E_NALU_ISLICE = 19,
    E_MI_VENC_H265E_NALU_VPS    = 32,
    E_MI_VENC_H265E_NALU_SPS    = 33,
    E_MI_VENC_H265E_NALU_PPS    = 34,
    E_MI_VENC_H265E_NALU_SEI    = 39,
    E_MI_VENC_H265E_NALU_MAX
} MI_VENC_H265eNaluType_e;

typedef enum
{
    E_MI_VENC_RC_MODE_H264CBR = 1,
    E_MI_VENC_RC_MODE_H264VBR,
    E_MI_VENC_RC_MODE_H264FIXQP,
    E_MI_VENC_RC_MODE_H264AVBR,
    E_MI_VENC_RC_MODE_H264UBR,
    E_MI_VENC_RC_MODE_MJPEGCBR,
    E_MI_VENC_RC_MODE_MJPEGVBR,
    E_MI_VENC_RC_MODE_MJPEGFIXQP,
    E_MI_VENC_RC_MODE_H265CBR,
    E_MI_VENC_RC_MODE_H265VBR,
    E_MI_VENC_RC_MODE_H265FIXQP,
    E_MI_VENC_RC_MODE_H265AVBR,
    E_MI_VENC_RC_MODE_H265UBR,
    E_MI_VENC_RC_MODE_AV1CBR,
    E_MI_VENC_RC_MODE_AV1VBR,
    E_MI_VENC_RC_MODE_AV1FIXQP,
    E_MI_VENC_RC_MODE_AV1AVBR,
    E_MI_VENC_RC_MODE_AV1UBR,
    E_MI_VENC_RC_MODE_CBR,
    E_MI_VENC_RC_MODE_VBR,
    E_MI_VENC_RC_MODE_AVBR,
    E_MI_VENC_RC_MODE_UBR,
    E_MI_VENC_RC_MODE_CVBR,
    E_MI_VENC_RC_MODE_FIXQP,
    E_MI_VENC_RC_MODE_MAX
} MI_VENC_RcMode_e;

typedef enum
{
    E_MI_VENC_SUPERFRM_NONE,
    E_MI_VENC_SUPERFRM_DISCARD,
    E_MI_VENC_SUPERFRM_REENCODE,
    E_MI_VENC_SUPERFRM_MAX
} MI_VENC_SuperFrmMode_e;

typedef enum
{
    E_MI_VENC_FRMLOST_NORMAL,
    E_MI_VENC_FRMLOST_PSKIP,
    E_MI_VENC_FRMLOST_MAX,
} MI_VENC_FrameLostMode_e;

typedef enum
{
    E_MI_VENC_RC_PRIORITY_BITRATE_FIRST = 1,
    E_MI_VENC_RC_PRIORITY_FRAMEBITS_FIRST,
    E_MI_VENC_RC_PRIORITY_MAX,
} MI_VENC_RcPriority_e;

typedef enum
{
    E_MI_VENC_INPUT_MODE_NORMAL_FRMBASE = 0, /*Handshake with input by about 3 buffers in frame mode*/
    E_MI_VENC_INPUT_MODE_RING_ONE_FRM,       /*Handshake with input by one buffer in ring mode*/
    E_MI_VENC_INPUT_MODE_RING_HALF_FRM,      /*Handshake with input by half buffer in ring mode*/
    E_MI_VENC_INPUT_MODE_HW_AUTO_SYNC,       /*Handshake with input by hw auto sync mode */
    E_MI_VENC_INPUT_MODE_RING_UNIFIED_DMA,   /*Multiple venc main channel handshake with input by one
                                               unified buffer in ring mode*/
    E_MI_VENC_INPUT_MODE_MAX
} MI_VENC_InputSrcBufferMode_e;

typedef enum
{
    E_MI_VENC_AV1_CP_RESERVED_0   = 0,  /**< For future use */
    E_MI_VENC_AV1_CP_BT_709       = 1,  /**< BT.709 */
    E_MI_VENC_AV1_CP_UNSPECIFIED  = 2,  /**< Unspecified */
    E_MI_VENC_AV1_CP_RESERVED_3   = 3,  /**< For future use */
    E_MI_VENC_AV1_CP_BT_470_M     = 4,  /**< BT.470 System M (historical) */
    E_MI_VENC_AV1_CP_BT_470_B_G   = 5,  /**< BT.470 System B, G (historical) */
    E_MI_VENC_AV1_CP_BT_601       = 6,  /**< BT.601 */
    E_MI_VENC_AV1_CP_SMPTE_240    = 7,  /**< SMPTE 240 */
    E_MI_VENC_AV1_CP_GENERIC_FILM = 8,  /**< Generic film (color filters using illuminant C) */
    E_MI_VENC_AV1_CP_BT_2020      = 9,  /**< BT.2020, BT.2100 */
    E_MI_VENC_AV1_CP_XYZ          = 10, /**< SMPTE 428 (CIE 1921 XYZ) */
    E_MI_VENC_AV1_CP_SMPTE_431    = 11, /**< SMPTE RP 431-2 */
    E_MI_VENC_AV1_CP_SMPTE_432    = 12, /**< SMPTE EG 432-1  */
    E_MI_VENC_AV1_CP_RESERVED_13  = 13, /**< For future use (values 13 - 21)  */
    E_MI_VENC_AV1_CP_EBU_3213     = 22, /**< EBU Tech. 3213-E  */
    E_MI_VENC_AV1_CP_RESERVED_23  = 23, /**< For future use (values 23 - 255)  */
    E_MI_VENC_AV1_CP_MAX
} MI_VENC_Av1ColorPrimaries_e;

typedef enum
{
    E_MI_VENC_AV1_TC_RESERVED_0     = 0,  /**< For future use */
    E_MI_VENC_AV1_TC_BT_709         = 1,  /**< BT.709 */
    E_MI_VENC_AV1_TC_UNSPECIFIED    = 2,  /**< Unspecified */
    E_MI_VENC_AV1_TC_RESERVED_3     = 3,  /**< For future use */
    E_MI_VENC_AV1_TC_BT_470_M       = 4,  /**< BT.470 System M (historical)  */
    E_MI_VENC_AV1_TC_BT_470_B_G     = 5,  /**< BT.470 System B, G (historical) */
    E_MI_VENC_AV1_TC_BT_601         = 6,  /**< BT.601 */
    E_MI_VENC_AV1_TC_SMPTE_240      = 7,  /**< SMPTE 240 M */
    E_MI_VENC_AV1_TC_LINEAR         = 8,  /**< Linear */
    E_MI_VENC_AV1_TC_LOG_100        = 9,  /**< Logarithmic (100 : 1 range) */
    E_MI_VENC_AV1_TC_LOG_100_SQRT10 = 10, /**< Logarithmic (100 * Sqrt(10) : 1 range) */
    E_MI_VENC_AV1_TC_IEC_61966      = 11, /**< IEC 61966-2-4 */
    E_MI_VENC_AV1_TC_BT_1361        = 12, /**< BT.1361 */
    E_MI_VENC_AV1_TC_SRGB           = 13, /**< sRGB or sYCC*/
    E_MI_VENC_AV1_TC_BT_2020_10_BIT = 14, /**< BT.2020 10-bit systems */
    E_MI_VENC_AV1_TC_BT_2020_12_BIT = 15, /**< BT.2020 12-bit systems */
    E_MI_VENC_AV1_TC_SMPTE_2084     = 16, /**< SMPTE ST 2084, ITU BT.2100 PQ */
    E_MI_VENC_AV1_TC_SMPTE_428      = 17, /**< SMPTE ST 428 */
    E_MI_VENC_AV1_TC_HLG            = 18, /**< BT.2100 HLG, ARIB STD-B67 */
    E_MI_VENC_AV1_TC_RESERVED_19    = 19, /**< For future use (values 19-255) */
    E_MI_VENC_AV1_TC_MAX
} MI_VENC_Av1TransferCharacteristics_e;

typedef enum
{
    E_MI_VENC_AV1_MC_IDENTITY    = 0,  /**< Identity matrix */
    E_MI_VENC_AV1_MC_BT_709      = 1,  /**< BT.709 */
    E_MI_VENC_AV1_MC_UNSPECIFIED = 2,  /**< Unspecified */
    E_MI_VENC_AV1_MC_RESERVED_3  = 3,  /**< For future use */
    E_MI_VENC_AV1_MC_FCC         = 4,  /**< US FCC 73.628 */
    E_MI_VENC_AV1_MC_BT_470_B_G  = 5,  /**< BT.470 System B, G (historical) */
    E_MI_VENC_AV1_MC_BT_601      = 6,  /**< BT.601 */
    E_MI_VENC_AV1_MC_SMPTE_240   = 7,  /**< SMPTE 240 M */
    E_MI_VENC_AV1_MC_SMPTE_YCGCO = 8,  /**< YCgCo */
    E_MI_VENC_AV1_MC_BT_2020_NCL = 9,  /**< BT.2020 non-constant luminance, BT.2100 YCbCr  */
    E_MI_VENC_AV1_MC_BT_2020_CL  = 10, /**< BT.2020 constant luminance */
    E_MI_VENC_AV1_MC_SMPTE_2085  = 11, /**< SMPTE ST 2085 YDzDx */
    E_MI_VENC_AV1_MC_CHROMAT_NCL = 12, /**< Chromaticity-derived non-constant luminance */
    E_MI_VENC_AV1_MC_CHROMAT_CL  = 13, /**< Chromaticity-derived constant luminance */
    E_MI_VENC_AV1_MC_ICTCP       = 14, /**< BT.2100 ICtCp */
    E_MI_VENC_AV1_MC_RESERVED_15 = 15, /**< For future use (values 15-255)  */
    E_MI_VENC_AV1_MC_MAX
} MI_VENC_Av1MatrixCoefficients_e;

typedef enum
{
    E_MI_VENC_AV1_OBU_SEQUENCE_HEADER    = 1,
    E_MI_VENC_AV1_OBU_TEMPORAL_DELIMITER = 2,
    E_MI_VENC_AV1_OBU_METADATA           = 5,
    E_MI_VENC_AV1_OBU_FRAME              = 6,
    E_MI_VENC_AV1_OBU_MAX                = 16
} MI_VENC_Av1ObuType_e;

typedef enum
{
    E_MI_VENC_UBR_MODE_DEFAULT,         /**< default mode, will enable block-rc and enable gradient qp*/
    E_MI_VENC_UBR_MODE_PIC_BIT_DISABLE, /**< disable pit target bit, will disable block-rc and enable gradient qp*/
    E_MI_VENC_UBR_MODE_FIXED_BLOCK_QP,  /**< use fixed block qp, will disable block-rc and disable gradient qp*/
    E_MI_VENC_UBR_MODE_MAX
} MI_VENC_UbrMode_e;

//==== Structures ====

typedef struct MI_VENC_Rect_s
{
    MI_U32 u32Left;
    MI_U32 u32Top;
    MI_U32 u32Width;
    MI_U32 u32Height;
} MI_VENC_Rect_t;

typedef union MI_VENC_DataType_s
{
    MI_VENC_H264eNaluType_e eH264EType;
    MI_VENC_JpegePackType_e eJPEGEType;
    MI_VENC_H265eNaluType_e eH265EType;
    MI_VENC_Av1ObuType_e    eAv1Type;
} MI_VENC_DataType_t;

typedef struct MI_VENC_PackInfo_s
{
    MI_VENC_DataType_t stPackType;
    MI_U32             u32PackOffset;
    MI_U32             u32PackLength;
    MI_U32             u32SliceId;
} MI_VENC_PackInfo_t;

typedef struct MI_VENC_Pack_s
{
    MI_PHY             phyAddr;
    MI_U8 *            pu8Addr;
    MI_U32             u32Len;
    MI_U64             u64PTS;
    MI_BOOL            bFrameEnd;
    MI_VENC_DataType_t stDataType;
    MI_U32             u32Offset;
    MI_U32             u32DataNum;
    MI_U8              u8FrameQP;
    MI_S32             s32PocNum;
    MI_U32             u32Gradient;
    MI_VENC_PackInfo_t asackInfo[8];
} MI_VENC_Pack_t;

typedef struct MI_VENC_StreamInfoH264_s
{
    MI_U32                      u32PicBytesNum;
    MI_U32                      u32PSkipMbNum;
    MI_U32                      u32IpcmMbNum;
    MI_U32                      u32Inter16x8MbNum;
    MI_U32                      u32Inter16x16MbNum;
    MI_U32                      u32Inter8x16MbNum;
    MI_U32                      u32Inter8x8MbNum;
    MI_U32                      u32Intra16MbNum;
    MI_U32                      u32Intra8MbNum;
    MI_U32                      u32Intra4MbNum;
    MI_VENC_H264eRefSliceType_e eRefSliceType;
    MI_VENC_H264eRefType_e      eRefType;
    MI_U32                      u32UpdateAttrCnt;
    MI_U32                      u32StartQp;
} MI_VENC_StreamInfoH264_t;

typedef struct MI_VENC_StreamInfoJpeg_s
{
    MI_U32 u32PicBytesNum;
    MI_U32 u32UpdateAttrCnt;
    MI_U32 u32Qfactor;
} MI_VENC_StreamInfoJpeg_t;

typedef struct MI_VENC_StreamInfoH265_s
{
    MI_U32                 u32PicBytesNum;
    MI_U32                 u32Inter64x64CuNum;
    MI_U32                 u32Inter32x32CuNum;
    MI_U32                 u32Inter16x16CuNum;
    MI_U32                 u32Inter8x8CuNum;
    MI_U32                 u32Intra32x32CuNum;
    MI_U32                 u32Intra16x16CuNum;
    MI_U32                 u32Intra8x8CuNum;
    MI_U32                 u32Intra4x4CuNum;
    MI_VENC_H265eRefType_e eRefType;
    MI_U32                 u32UpdateAttrCnt;
    MI_U32                 u32StartQp;
} MI_VENC_StreamInfoH265_t;

typedef struct MI_VENC_StreamInfoAv1_s
{
    MI_U32                u32PicBytesNum;
    MI_U32                u32Inter64x64CuNum;
    MI_U32                u32Inter32x32CuNum;
    MI_U32                u32Inter16x16CuNum;
    MI_U32                u32Inter8x8CuNum;
    MI_U32                u32Intra32x32CuNum;
    MI_U32                u32Intra16x16CuNum;
    MI_U32                u32Intra8x8CuNum;
    MI_U32                u32Intra4x4CuNum;
    MI_VENC_Av1eRefType_e eRefType;
    MI_U32                u32UpdateAttrCnt;
    MI_U32                u32StartQp;
} MI_VENC_StreamInfoAv1_t;

typedef struct MI_VENC_Stream_s
{
    MI_VENC_Pack_t *  pstPack;
    MI_U32            u32PackCount;
    MI_U32            u32Seq;
    MI_SYS_BUF_HANDLE hMiSys;
    union
    {
        MI_VENC_StreamInfoH264_t stH264Info;
        MI_VENC_StreamInfoJpeg_t stJpegInfo;
        MI_VENC_StreamInfoH265_t stH265Info;
        MI_VENC_StreamInfoAv1_t  stAv1Info;
    };
} MI_VENC_Stream_t;

typedef struct MI_VENC_AttrVenc_s
{
    MI_U32  u32MaxPicWidth;
    MI_U32  u32MaxPicHeight;
    MI_U32  u32BufSize;
    MI_U32  u32Profile;
    MI_BOOL bByFrame;
    MI_U32  u32PicWidth;
    MI_U32  u32PicHeight;
    MI_U32  u32BFrameNum;
    MI_U32  u32RefNum;
} MI_VENC_AttrVenc_t;
typedef MI_VENC_AttrVenc_t MI_VENC_AttrH264_t;
typedef MI_VENC_AttrVenc_t MI_VENC_AttrH265_t;


typedef struct MI_VENC_AttrAv1_s
{
    MI_U32  u32MaxPicWidth;
    MI_U32  u32MaxPicHeight;
    MI_U32  u32BufSize;
    MI_U32  u32Profile;
    MI_BOOL bByFrame;
    MI_U32  u32PicWidth;
    MI_U32  u32PicHeight;
    MI_U32  u32RefNum;
    MI_BOOL bEnableSwitchFrame;
} MI_VENC_AttrAv1_t;

typedef struct MI_VENC_AttrJpeg_s
{
    MI_U32  u32MaxPicWidth;
    MI_U32  u32MaxPicHeight;
    MI_U32  u32BufSize;
    MI_BOOL bByFrame;
    MI_U32  u32PicWidth;
    MI_U32  u32PicHeight;
    MI_BOOL bSupportDCF;
    MI_U32  u32RestartMakerPerRowCnt;
} MI_VENC_AttrJpeg_t;

typedef struct MI_VENC_Attr_s
{
    MI_VENC_ModType_e eType;
    union
    {
        MI_VENC_AttrH264_t stAttrH264e;
        MI_VENC_AttrJpeg_t stAttrJpeg;
        MI_VENC_AttrH265_t stAttrH265e;
        MI_VENC_AttrAv1_t  stAttrAv1;
    };
} MI_VENC_Attr_t;

typedef struct MI_VENC_ChnStat_s
{
    MI_U32 u32LeftPics;
    MI_U32 u32LeftStreamBytes;
    MI_U32 u32LeftStreamFrames;
    MI_U32 u32LeftStreamMillisec;
    MI_U32 u32CurPacks;
    MI_U32 u32LeftRecvPics;
    MI_U32 u32LeftEncPics;
    MI_U32 u32FrmRateNum;
    MI_U32 u32FrmRateDen;
    MI_U32 u32BitRate;
} MI_VENC_ChnStat_t;

typedef struct MI_VENC_ParamH264SliceSplit_s
{
    MI_BOOL bSplitEnable;
    MI_U32  u32SliceRowCount;
} MI_VENC_ParamH264SliceSplit_t;

typedef struct MI_VENC_ParamH264Trans_s
{
    MI_U32 u32IntraTransMode;
    MI_U32 u32InterTransMode;
    MI_S32 s32ChromaQpIndexOffset;
} MI_VENC_ParamH264Trans_t;

typedef struct MI_VENC_ParamH264Entropy_s
{
    MI_U32 u32EntropyEncModeI;
    MI_U32 u32EntropyEncModeP;
} MI_VENC_ParamH264Entropy_t;

typedef struct MI_VENC_ParamH265Trans_s
{
    MI_U32 u32IntraTransMode;
    MI_U32 u32InterTransMode;
    MI_S32 s32ChromaQpIndexOffset;
} MI_VENC_ParamH265Trans_t;

typedef struct MI_VENC_ParamH264Dblk_s
{
    MI_U32 disable_deblocking_filter_idc; // special naming for CODEC ISO SPEC.
    MI_S32 slice_alpha_c0_offset_div2;    // special naming for CODEC ISO SPEC.
    MI_S32 slice_beta_offset_div2;        // special naming for CODEC ISO SPEC.
} MI_VENC_ParamH264Dblk_t;

typedef struct MI_VENC_ParamH264VuiAspectRatio_s
{
    MI_U8  u8AspectRatioInfoPresentFlag;
    MI_U8  u8AspectRatioIdc;
    MI_U8  u8OverscanInfoPresentFlag;
    MI_U8  u8OverscanAppropriateFlag;
    MI_U16 u16SarWidth;
    MI_U16 u16SarHeight;
} MI_VENC_ParamH264VuiAspectRatio_t;

typedef struct MI_VENC_ParamH264VuiTimeInfo_s
{
    MI_U8  u8TimingInfoPresentFlag;
    MI_U8  u8FixedFrameRateFlag;
    MI_U32 u32NumUnitsInTick;
    MI_U32 u32TimeScale;
} MI_VENC_ParamH264VuiTimeInfo_t;

typedef struct MI_VENC_ParamH264VuiVideoSignal_s
{
    MI_U8 u8VideoSignalTypePresentFlag;
    MI_U8 u8VideoFormat;
    MI_U8 u8VideoFullRangeFlag;
    MI_U8 u8ColourDescriptionPresentFlag;
    MI_U8 u8ColourPrimaries;
    MI_U8 u8TransferCharacteristics;
    MI_U8 u8MatrixCoefficients;
} MI_VENC_ParamH264VuiVideoSignal_t;

typedef struct MI_VENC_ParamH264Vui_s
{
    MI_VENC_ParamH264VuiAspectRatio_t stVuiAspectRatio;
    MI_VENC_ParamH264VuiTimeInfo_t    stVuiTimeInfo;
    MI_VENC_ParamH264VuiVideoSignal_t stVuiVideoSignal;
} MI_VENC_ParamH264Vui_t;

typedef struct MI_VENC_ParamH265VuiAspectRatio_s
{
    MI_U8  u8AspectRatioInfoPresentFlag;
    MI_U8  u8AspectRatioIdc;
    MI_U8  u8OverscanInfoPresentFlag;
    MI_U8  u8OverscanAppropriateFlag;
    MI_U16 u16SarWidth;
    MI_U16 u16SarHeight;
} MI_VENC_ParamH265VuiAspectRatio_t;

typedef struct MI_VENC_ParamH265VuiTimeInfo_s
{
    MI_U8 u8TimingInfoPresentFlag;
    MI_U32 u32NumUnitsInTick;
    MI_U32 u32TimeScale;
} MI_VENC_ParamH265VuiTimeInfo_t;

typedef struct MI_VENC_ParamH265VuiVideoSignal_s
{
    MI_U8 u8VideoSignalTypePresentFlag;
    MI_U8 u8VideoFormat;
    MI_U8 u8VideoFullRangeFlag;
    MI_U8 u8ColourDescriptionPresentFlag;
    MI_U8 u8ColourPrimaries;
    MI_U8 u8TransferCharacteristics;
    MI_U8 u8MatrixCoefficients;
} MI_VENC_ParamH265VuiVideoSignal_t;

typedef struct MI_VENC_ParamH265Vui_s
{
    MI_VENC_ParamH265VuiAspectRatio_t stVuiAspectRatio;
    MI_VENC_ParamH265VuiTimeInfo_t    stVuiTimeInfo;
    MI_VENC_ParamH265VuiVideoSignal_t stVuiVideoSignal;
} MI_VENC_ParamH265Vui_t;

typedef struct MI_VENC_ParamH265SliceSplit_s
{
    MI_BOOL bSplitEnable;
    MI_U32  u32SliceRowCount;
} MI_VENC_ParamH265SliceSplit_t;

typedef struct MI_VENC_ParamH265Dblk_s
{
    MI_U32 disable_deblocking_filter_idc; // special naming for CODEC ISO SPEC.
    MI_S32 slice_tc_offset_div2;          // special naming for CODEC ISO SPEC.
    MI_S32 slice_beta_offset_div2;        // special naming for CODEC ISO SPEC.
} MI_VENC_ParamH265Dblk_t;

typedef struct MI_VENC_ParamJpeg_s
{
    MI_U32 u32Qfactor;
    MI_U8  au8YQt[64];
    MI_U8  au8CbCrQt[64];
    MI_U32 u32McuPerEcs;
} MI_VENC_ParamJpeg_t;

typedef struct MI_VENC_RoiCfg_s
{
    MI_U32         u32Index;
    MI_BOOL        bEnable;
    MI_BOOL        bAbsQp;
    MI_S32         s32Qp;
    MI_VENC_Rect_t stRect;
} MI_VENC_RoiCfg_t;

typedef struct MI_VENC_RoiBgFrameRate_s
{
    MI_S32 s32SrcFrmRate;
    MI_S32 s32DstFrmRate;
} MI_VENC_RoiBgFrameRate_t;

typedef struct MI_VENC_ParamRef_s
{
    MI_U32  u32Base;
    MI_U32  u32Enhance;
    MI_BOOL bEnablePred;
} MI_VENC_ParamRef_t;

typedef struct MI_VENC_AttrCbr_s
{
    MI_U32 u32Gop;
    MI_U32 u32StatTime;
    MI_U32 u32SrcFrmRateNum;
    MI_U32 u32SrcFrmRateDen;
    MI_U32 u32BitRate;
    MI_U32 u32FluctuateLevel;
} MI_VENC_AttrCbr_t;
typedef MI_VENC_AttrCbr_t MI_VENC_AttrH264Cbr_t;
typedef MI_VENC_AttrCbr_t MI_VENC_AttrH265Cbr_t;
typedef MI_VENC_AttrCbr_t MI_VENC_AttrAv1Cbr_t;

typedef struct MI_VENC_AttrVbr_s
{
    MI_U32 u32Gop;
    MI_U32 u32StatTime;
    MI_U32 u32SrcFrmRateNum;
    MI_U32 u32SrcFrmRateDen;
    MI_U32 u32MaxBitRate;
    MI_U32 u32MaxQp;
    MI_U32 u32MinQp;
} MI_VENC_AttrVbr_t;
typedef MI_VENC_AttrVbr_t MI_VENC_AttrH264Vbr_t;
typedef MI_VENC_AttrVbr_t MI_VENC_AttrH265Vbr_t;
typedef MI_VENC_AttrVbr_t MI_VENC_AttrAv1Vbr_t;

typedef struct MI_VENC_AttrFixQp_s
{
    MI_U32 u32Gop;
    MI_U32 u32SrcFrmRateNum;
    MI_U32 u32SrcFrmRateDen;
    MI_U32 u32IQp;
    MI_U32 u32PQp;
} MI_VENC_AttrFixQp_t;
typedef MI_VENC_AttrFixQp_t MI_VENC_AttrH264FixQp_t;
typedef MI_VENC_AttrFixQp_t MI_VENC_AttrH265FixQp_t;
typedef MI_VENC_AttrFixQp_t MI_VENC_AttrAv1FixQp_t;


typedef struct MI_VENC_AttrAvbr_s
{
    MI_U32 u32Gop;
    MI_U32 u32StatTime;
    MI_U32 u32SrcFrmRateNum;
    MI_U32 u32SrcFrmRateDen;
    MI_U32 u32MaxBitRate;
    MI_U32 u32MaxQp;
    MI_U32 u32MinQp;
} MI_VENC_AttrAvbr_t;
typedef MI_VENC_AttrAvbr_t MI_VENC_AttrH264Avbr_t;
typedef MI_VENC_AttrAvbr_t MI_VENC_AttrH265Avbr_t;
typedef MI_VENC_AttrAvbr_t MI_VENC_AttrAv1Avbr_t;

typedef struct MI_VENC_AttrUbr_s
{
    MI_U32 u32Gop;
    MI_U32 u32StatTime;
    MI_U32 u32SrcFrmRateNum;
    MI_U32 u32SrcFrmRateDen;
    MI_U32 u32MaxBitRate;
    MI_U32 u32MaxQp;
    MI_U32 u32MinQp;
    MI_VENC_UbrMode_e eUbrMode;
} MI_VENC_AttrUbr_t;
typedef MI_VENC_AttrUbr_t MI_VENC_AttrH264Ubr_t;
typedef MI_VENC_AttrUbr_t MI_VENC_AttrH265Ubr_t;
typedef MI_VENC_AttrUbr_t MI_VENC_AttrAv1Ubr_t;

typedef struct MI_VENC_AttrCvbr_s
{
    MI_U32 u32Gop;
    MI_U32 u32StatTime;
    MI_U32 u32SrcFrmRateNum;
    MI_U32 u32SrcFrmRateDen;
    MI_U32 u32MaxBitRate;
    MI_U32 u32ShortTermStatsTime;
    MI_U32 u32LongTermStatsTime;
    MI_U32 u32LongTermMaxBitRate;
    MI_U32 u32LongTermMinBitRate;
} MI_VENC_AttrCvbr_t;

typedef struct MI_VENC_AttrMjpegCbr_s
{
    MI_U32 u32BitRate;
    MI_U32 u32SrcFrmRateNum;
    MI_U32 u32SrcFrmRateDen;
} MI_VENC_AttrMjpegCbr_t;

typedef struct MI_VENC_AttrMjpegVbr_s
{
    MI_U32 u32MaxBitRate;
    MI_U32 u32SrcFrmRateNum;
    MI_U32 u32SrcFrmRateDen;
} MI_VENC_AttrMjpegVbr_t;

typedef struct MI_VENC_AttrMjpegFixQp_s
{
    MI_U32 u32SrcFrmRateNum;
    MI_U32 u32SrcFrmRateDen;
    MI_U32 u32Qfactor;
} MI_VENC_AttrMjpegFixQp_t;

typedef struct MI_VENC_RcAttr_s
{
    MI_VENC_RcMode_e eRcMode;
    union
    {
        MI_VENC_AttrH264Cbr_t    stAttrH264Cbr;
        MI_VENC_AttrH264Vbr_t    stAttrH264Vbr;
        MI_VENC_AttrH264FixQp_t  stAttrH264FixQp;
        MI_VENC_AttrH264Avbr_t   stAttrH264Avbr;
        MI_VENC_AttrH264Ubr_t    stAttrH264Ubr;
        MI_VENC_AttrH265Cbr_t    stAttrH265Cbr;
        MI_VENC_AttrH265Vbr_t    stAttrH265Vbr;
        MI_VENC_AttrH265FixQp_t  stAttrH265FixQp;
        MI_VENC_AttrH265Avbr_t   stAttrH265Avbr;
        MI_VENC_AttrH265Ubr_t    stAttrH265Ubr;
        MI_VENC_AttrAv1Cbr_t     stAttrAv1Cbr;
        MI_VENC_AttrAv1Vbr_t     stAttrAv1Vbr;
        MI_VENC_AttrAv1FixQp_t   stAttrAv1FixQp;
        MI_VENC_AttrAv1Avbr_t    stAttrAv1Avbr;
        MI_VENC_AttrAv1Ubr_t     stAttrAv1Ubr;
        MI_VENC_AttrMjpegCbr_t   stAttrMjpegCbr;
        MI_VENC_AttrMjpegVbr_t   stAttrMjpegVbr;
        MI_VENC_AttrMjpegFixQp_t stAttrMjpegFixQp;
        MI_VENC_AttrCbr_t        stAttrCbr;
        MI_VENC_AttrVbr_t        stAttrVbr;
        MI_VENC_AttrFixQp_t      stAttrFixQp;
        MI_VENC_AttrAvbr_t       stAttrAvbr;
        MI_VENC_AttrUbr_t        stAttrUbr;
        MI_VENC_AttrCvbr_t       stAttrCvbr;
    };
} MI_VENC_RcAttr_t;

typedef struct MI_VENC_ChnAttr_s
{
    MI_VENC_Attr_t   stVeAttr;
    MI_VENC_RcAttr_t stRcAttr;
} MI_VENC_ChnAttr_t;

typedef struct MI_VENC_OutPortParam_s
{
    MI_U32  u32Width;
    MI_U32  u32Height;
} MI_VENC_OutPortParam_t;

typedef struct MI_VENC_ParamVbr_s
{
    MI_U32 u32MaxQp;
    MI_U32 u32MinQp;
    MI_S32 s32IPQPDelta;
    MI_S32 s32ChangePos;
    MI_U32 u32MaxIQp;
    MI_U32 u32MinIQp;
    MI_U32 u32MaxIPProp;
    MI_U32 u32MaxISize;
    MI_U32 u32MaxPSize;
} MI_VENC_ParamVbr_t;
typedef MI_VENC_ParamVbr_t MI_VENC_ParamH264Vbr_t;
typedef MI_VENC_ParamVbr_t MI_VENC_ParamH265Vbr_t;
typedef MI_VENC_ParamVbr_t MI_VENC_ParamAv1Vbr_t;

typedef struct MI_VENC_ParamCbr_s
{
    MI_U32 u32MaxQp;
    MI_U32 u32MinQp;
    MI_S32 s32IPQPDelta;
    MI_U32 u32MaxIQp;
    MI_U32 u32MinIQp;
    MI_U32 u32MaxIPProp;
    MI_U32 u32MaxISize;
    MI_U32 u32MaxPSize;
} MI_VENC_ParamCbr_t;
typedef MI_VENC_ParamCbr_t MI_VENC_ParamH264Cbr_t;
typedef MI_VENC_ParamCbr_t MI_VENC_ParamH265Cbr_t;
typedef MI_VENC_ParamCbr_t MI_VENC_ParamAv1Cbr_t;

typedef struct MI_VENC_ParamAvbr_s
{
    MI_U32 u32MaxQp;
    MI_U32 u32MinQp;
    MI_S32 s32IPQPDelta;
    MI_S32 s32ChangePos;
    MI_U32 u32MaxIQp;
    MI_U32 u32MinIQp;
    MI_U32 u32MaxIPProp;
    MI_U32 u32MaxISize;
    MI_U32 u32MaxPSize;
    MI_U32 u32MinStillPercent;
    MI_U32 u32MaxStillQp;
    MI_U32 u32MotionSensitivity;
} MI_VENC_ParamAvbr_t;
typedef MI_VENC_ParamAvbr_t MI_VENC_ParamH264Avbr_t;
typedef MI_VENC_ParamAvbr_t MI_VENC_ParamH265Avbr_t;
typedef MI_VENC_ParamAvbr_t MI_VENC_ParamAv1Avbr_t;

typedef struct MI_VENC_ParamCvbr_s {
    MI_U32 u32MaxQp;
    MI_U32 u32MinQp;
    MI_S32 s32IPQPDelta;
    MI_U32 u32MaxIQp;
    MI_U32 u32MinIQp;
    MI_U32 u32MaxIPProp;
    MI_U32 u32MaxISize;
    MI_U32 u32MaxPSize;
} MI_VENC_ParamCvbr_t;

typedef struct MI_VENC_ParamMjpegVbr_s
{
    MI_U32 u32MaxQfactor;
    MI_U32 u32MinQfactor;
    MI_S32 s32ChangePos;
} MI_VENC_ParamMjpegVbr_t;

typedef struct MI_VENC_ParamMjpegCbr_s
{
    MI_U32 u32MaxQfactor;
    MI_U32 u32MinQfactor;
} MI_VENC_ParamMjpegCbr_t;

typedef struct MI_VENC_RcParam_s
{
    MI_U32 au32ThrdI[RC_TEXTURE_THR_SIZE];
    MI_U32 au32ThrdP[RC_TEXTURE_THR_SIZE];
    MI_U32 u32RowQpDelta;
    union
    {
        MI_VENC_ParamCbr_t  stParamCbr;
        MI_VENC_ParamVbr_t  stParamVBR;
        MI_VENC_ParamAvbr_t stParamAvbr;
        MI_VENC_ParamCvbr_t stParamCvbr;
        MI_VENC_ParamH264Cbr_t  stParamH264Cbr;
        MI_VENC_ParamH264Vbr_t  stParamH264VBR;
        MI_VENC_ParamH264Avbr_t stParamH264Avbr;
        MI_VENC_ParamMjpegCbr_t stParamMjpegCbr;
        MI_VENC_ParamMjpegVbr_t stParamMjpegVbr;
        MI_VENC_ParamH265Cbr_t  stParamH265Cbr;
        MI_VENC_ParamH265Vbr_t  stParamH265Vbr;
        MI_VENC_ParamH265Avbr_t stParamH265Avbr;
        MI_VENC_ParamAv1Cbr_t   stParamAv1Cbr;
        MI_VENC_ParamAv1Vbr_t   stParamAv1Vbr;
        MI_VENC_ParamAv1Avbr_t  stParamAv1Avbr;
    };
} MI_VENC_RcParam_t;

typedef struct MI_VENC_CropCfg_s
{
    MI_BOOL        bEnable; /* Crop region enable */
    MI_VENC_Rect_t stRect;  /* Crop region, note: s32X must be multi of 16 */
} MI_VENC_CropCfg_t;

typedef struct MI_VENC_RecvPicParam_s
{
    MI_S32 s32RecvPicNum;
} MI_VENC_RecvPicParam_t;

typedef struct MI_VENC_DeBreathCfg_s
{
    MI_BOOL bEnable;
    MI_U8 u8Strength0;
    MI_U8 u8Strength1;
} MI_VENC_DeBreathCfg_t;


typedef struct MI_VENC_ParamFrameLost_s
{
    MI_BOOL                 bFrmLostOpen;
    MI_U32                  u32FrmLostBpsThr;
    MI_VENC_FrameLostMode_e eFrmLostMode;
    MI_U32                  u32EncFrmGaps;
} MI_VENC_ParamFrameLost_t;

typedef struct MI_VENC_SuperFrameCfg_s
{
    MI_VENC_SuperFrmMode_e eSuperFrmMode;
    MI_U32                 u32SuperIFrmBitsThr;
    MI_U32                 u32SuperPFrmBitsThr;
    MI_U32                 u32SuperBFrmBitsThr;
} MI_VENC_SuperFrameCfg_t;

typedef struct MI_VENC_InputSourceConfig_s
{
    MI_VENC_InputSrcBufferMode_e eInputSrcBufferMode;
} MI_VENC_InputSourceConfig_t;

typedef struct MI_VENC_FrameHistoStaticInfo_s
{
    MI_U8  u8PicSkip;
    MI_U16 u16PicType;
    MI_U32 u32PicPoc;
    MI_U32 u32PicSliNum;
    MI_U32 u32PicNumIntra;
    MI_U32 u32PicNumMerge;
    MI_U32 u32PicNumSkip;
    MI_U32 u32PicAvgCtuQp;
    MI_U32 u32PicByte;
    MI_U32 u32GopPicIdx;
    MI_U32 u32PicNum;
    MI_U32 u32PicDistLow;
    MI_U32 u32PicDistHigh;
} MI_VENC_FrameHistoStaticInfo_t;

typedef struct MI_VENC_AdvCustRcAttr_s
{
    MI_BOOL bEnableQPMap;
    MI_BOOL bAbsQP;
    MI_BOOL bEnableModeMap;
    MI_BOOL bEnabelHistoStaticInfo;
} MI_VENC_AdvCustRcAttr_t;

typedef struct MI_VENC_IntraRefresh_s
{
    MI_BOOL bEnable;
    MI_U32  u32RefreshLineNum;
    MI_U32  u32ReqIQp;
} MI_VENC_IntraRefresh_t;

typedef struct MI_VENC_InitParam_s
{
    MI_U32 u32MaxWidth;
    MI_U32 u32MaxHeight;
} MI_VENC_InitParam_t;

typedef struct MI_VENC_ParamAv1TileSplit_s
{
    MI_BOOL bSplitEnable;
    MI_U32  u32TileRowCount;
} MI_VENC_ParamAv1TileSplit_t;

typedef struct MI_VENC_ParamAv1Dblk_s
{
    MI_U32 u32DblkMode; /**< 0:disable loopfilter. 1:enable loopfilter and use internal param. 2:enable loopfilter and
                          use customer param */
    MI_U32 u32LumaHorzLevel; /**< specify the loopfilter parameter level for luma horizontal */
    MI_U32 u32LumaVertLevel; /**< specify the loopfilter parameter level for luma vertical */
    MI_U32 u32ChromaLevel;   /**< specify the loopfilter parameter level for chroma */
    MI_U32 u32Sharpness;     /**< specify the loopfilter parameter sharpess.  */
} MI_VENC_ParamAv1Dblk_t;

typedef struct MI_VENC_ParamAv1VuiColorConfig_s
{
    MI_U32                               u32ColorDescriptionPresentFlag;
    MI_VENC_Av1ColorPrimaries_e          eColorPrimaries;
    MI_VENC_Av1TransferCharacteristics_e eTransferCharacteristics;
    MI_VENC_Av1MatrixCoefficients_e      eMatrixCofficients;
} MI_VENC_ParamAv1VuiColorConfig_t;

typedef struct MI_VENC_ParamAv1VuiTimingInfo_s
{
    MI_U32 u32TimingInfoPresentFlag;
    MI_U32 u32NumUnitsInDisplayTick;
    MI_U32 u32TimeScale;
    MI_U32 u32EqualPictureInterval;
    MI_U32 u32NumTicksPerPictureMinus1;
} MI_VENC_ParamAv1VuiTimingInfo_t;

typedef struct MI_VENC_ParamAv1Vui_s
{
    MI_VENC_ParamAv1VuiColorConfig_t stVuiColorConfig;
    MI_VENC_ParamAv1VuiTimingInfo_t  stTimingInfo;
} MI_VENC_ParamAv1Vui_t;

/* =======================================================
 * Support UBR Mode
 * =======================================================*/
typedef MI_VENC_H264eRefType_e MI_VENC_UserRcRefType_e;

typedef struct MI_VENC_UserRcChnAttr_s
{
    MI_U32            u32Gop;
    MI_U32            u32PicWidth;
    MI_U32            u32PicHeight;
    MI_U32            u32SrcFrmRateNum;
    MI_U32            u32SrcFrmRateDen;
    MI_U32            u32Bitrate;
    MI_U32            u32MaxQp;
    MI_U32            u32MinQp;
    MI_VENC_UbrMode_e eUbrMode;
} MI_VENC_UserRcChnAttr_t;

typedef struct MI_VENC_UserRcFrameAttr_s
{
    MI_VENC_UserRcRefType_e eRefType;
    MI_U32                  u32FrameIdx;
    MI_BOOL                 bLtrFrame;
    MI_U32                  u32PreFrameBits;
} MI_VENC_UserRcFrameAttr_t;

typedef struct MI_VENC_UserRcEncParam_s
{
    MI_U8   u8FrameQp;
    MI_U32  u32PicTargetBits;
    MI_BOOL bDropCurrentFrame;
    MI_BOOL bForceSkip;
} MI_VENC_UserRcEncParam_t;

typedef struct MI_VENC_UserRcCallback_s
{
    MI_S32 (*OnVencUserRcInit)(MI_VENC_DEV VeDev, MI_VENC_CHN VeChn, MI_VENC_UserRcChnAttr_t *pstAttr);
    MI_S32 (*OnVencUserRcDeinit)(MI_VENC_DEV VeDev, MI_VENC_CHN VeChn);
    MI_S32(*OnVencUserRcCalc)
    (MI_VENC_DEV                      VeDev,        // devive id
     MI_VENC_CHN                      VeChn,        // channel id
     const MI_VENC_UserRcFrameAttr_t *pstFrameAttr, // the next frame info from encoder
     MI_VENC_UserRcEncParam_t *       pstEncParam);        // the next frame encode parameter
    MI_S32 (*OnVencUserRcAttrChange)(MI_VENC_DEV VeDev, MI_VENC_CHN VeChn, MI_VENC_UserRcChnAttr_t *pstAttr);
} MI_VENC_UserRcCallback_t;

#endif /* End of #ifndef __MI_VENC_DATATYPE_ */
