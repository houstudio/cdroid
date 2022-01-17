#ifndef __FY_COMM_VGS_H__
#define __FY_COMM_VGS_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#include "fy_type.h"
#include "fy_common.h"
#include "fy_errno.h"
#include "fy_comm_video.h"

/* failure caused by malloc buffer */
#define FY_ERR_VGS_NOBUF           FY_DEF_ERR(FY_ID_VGS, EN_ERR_LEVEL_ERROR, EN_ERR_NOBUF)
#define FY_ERR_VGS_BUF_EMPTY       FY_DEF_ERR(FY_ID_VGS, EN_ERR_LEVEL_ERROR, EN_ERR_BUF_EMPTY)
#define FY_ERR_VGS_NULL_PTR        FY_DEF_ERR(FY_ID_VGS, EN_ERR_LEVEL_ERROR, EN_ERR_NULL_PTR)
#define FY_ERR_VGS_ILLEGAL_PARAM   FY_DEF_ERR(FY_ID_VGS, EN_ERR_LEVEL_ERROR, EN_ERR_ILLEGAL_PARAM)
#define FY_ERR_VGS_BUF_FULL        FY_DEF_ERR(FY_ID_VGS, EN_ERR_LEVEL_ERROR, EN_ERR_BUF_FULL)
#define FY_ERR_VGS_SYS_NOTREADY    FY_DEF_ERR(FY_ID_VGS, EN_ERR_LEVEL_ERROR, EN_ERR_SYS_NOTREADY)
#define FY_ERR_VGS_NOT_SUPPORT     FY_DEF_ERR(FY_ID_VGS, EN_ERR_LEVEL_ERROR, EN_ERR_NOT_SUPPORT)
#define FY_ERR_VGS_NOT_PERMITTED   FY_DEF_ERR(FY_ID_VGS, EN_ERR_LEVEL_ERROR, EN_ERR_NOT_PERM)

typedef FY_S32      VGS_HANDLE;

typedef struct fyVGS_TASK_ATTR_S
{
    VIDEO_FRAME_INFO_S      stImgIn;        	/* input picture */
    VIDEO_FRAME_INFO_S      stImgOut;       	/* output picture */
    FY_U32                  au32privateData[4]; /* task's private data */
    FY_U32                  reserved;       	/* save current picture's state while debug */
}VGS_TASK_ATTR_S;

typedef struct fyVGS_LINE_S
{
    POINT_S 				stStartPoint;	/* line start point [-8191, 8191] */
	POINT_S 				stEndPoint;     /* line end point [-8191, 8191] */
	FY_U32  				u32Thick;       /* width of line [0, 14] */
	FY_U32  				u32Color;       /* color of line */
}VGS_LINE_S;

typedef struct fyVGS_COMBINE_S
{
    FY_S32          handle;
    VIDEO_FRAME_S   stVFrameIn;
    RECT_S          stRectIn;
    VIDEO_FRAME_S   stVFrameOut;
    RECT_S          stRectOut;
}VGS_COMBINE_S;

typedef struct fyVGS_ROTATE_S
{
    FY_BOOL          mirrorEn;
    FY_BOOL          flipEn;
    FY_BOOL          rotateEn;
}VGS_ROTATE_S;

typedef enum fyVGS_COVER_TYPE_E
{
    COVER_RECT = 0,
    COVER_QUAD_RANGLE,
    COVER_BUTT
} VGS_COVER_TYPE_E;

typedef struct fyVGS_QUADRANGLE_COVER_S
{
	FY_BOOL bSolid;            /* solid or hollow cover */
    FY_U32  u32Thick;          /* width of hollow cover */
    POINT_S stPoint[4];        /* four coordinate of quadrangle */
} VGS_QUADRANGLE_COVER_S;

typedef struct fyVGS_COVER_S
{
	VGS_COVER_TYPE_E        	enCoverType;
	union
    {
        RECT_S              	stDstRect;  	/* config of rectrangle */
        VGS_QUADRANGLE_COVER_S  stQuadRangle;   /* config of quadrangle */
    };

	FY_U32  u32Color;          					/* color of cover */
}VGS_COVER_S;

typedef struct fyVGS_OSD_S
{
    RECT_S 					stRect;			/* start point, width and height of osd [0, 8192] */
	FY_U32 					u32BgColor;     /* background color of osd */
	fyPIXEL_FORMAT_E 			enPixelFmt;     /* pixel format of osd */
	FY_U32                  u32PhyAddr;
	FY_U32                  u32Stride;
	FY_U32                  u32BgAlpha;
	FY_U32                  u32FgAlpha;
}VGS_OSD_S;

typedef enum fyVGS_ROTATE_E
{
    VGS_ROTATE_NONE = 0,               /* no rotate */
    VGS_ROTATE_90   = 1,               /* 90 degrees clockwise */
    VGS_ROTATE_180  = 2,               /* 180 degrees clockwise */
    VGS_ROTATE_270  = 3,               /* 270 degrees clockwise */
    VGS_ROTATE_BUTT
} VGS_ROTATE_E;

typedef struct FY_VGS_BORDER_S
{
    FY_U32  u32Width[4]; 	/* width of 4 frames,0:L,1:R,2:B,3:T */
    FY_U32  u32Color; 							/* color of 4 frames,R/G/B */
}VGS_BORDER_S;

typedef struct fyVGS_ASPECTRATIO_S
{
	RECT_S  stVideoRect;
	FY_U32 	u32CoverData;
 }VGS_ASPECTRATIO_S;


typedef struct fyVGS_ONLINE_S
{
    FY_BOOL                 bCrop;              /* if enable crop */
    RECT_S          		stCropRect;
    FY_BOOL                 bHSharpen;          /* if enable sharpen */
    FY_U32                  u32LumaGain;
    FY_BOOL                 bBorder;            /* if enable Border */
    VGS_BORDER_S        	stBorderOpt;
	FY_BOOL    				bAspectRatio;		/* if enable aspect ratio */
	VGS_ASPECTRATIO_S 		stAspectRatioOpt;

    FY_BOOL                 bForceHFilt;        /* if enable horizontal filter */
    FY_BOOL                 bForceVFilt;        /* if enable vertical filter */
    FY_BOOL                 bDeflicker;        	/* if enable deflicker */
}VGS_ONLINE_S;



#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __FY_COMM_VGS_H__ */
