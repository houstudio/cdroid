#include <cdtypes.h>
#include <cdlog.h>
#include <fcntl.h>
#include <sys/prctl.h>
#include <sys/ioctl.h>
#include "hi_common.h"
#include "hi_comm_sys.h"
#include "hi_comm_vb.h"
#include "hi_comm_vi.h"
#include "hi_comm_vo.h"
#include "hi_comm_venc.h"
#include "hi_comm_vpss.h"
#include "hi_comm_vdec.h"
#include "hi_comm_vda.h"
#include "hi_comm_region.h"
#include "hi_comm_adec.h"
#include "hi_comm_aenc.h"
#include "hi_comm_ai.h"
#include "hi_comm_ao.h"
#include "hi_comm_aio.h"
#include "hi_comm_hdmi.h"
#include "hi_defines.h"
#include "hifb.h"
#include "hi_tde_type.h"
#include "mpi_sys.h"
#include "mpi_vb.h"
#include "mpi_vi.h"
#include "mpi_vo.h"
#include "mpi_venc.h"
#include "mpi_vpss.h"
#include "mpi_vdec.h"
#include "mpi_vda.h"
#include "mpi_region.h"
#include "mpi_adec.h"
#include "mpi_aenc.h"
#include "mpi_ai.h"
#include "mpi_ao.h"
#include "mpi_hdmi.h"

#define CEILING_2_POWER(x,a) ( ((x) + ((a) - 1) ) & ( ~((a) - 1) ) )
#define SAMPLE_SYS_ALIGN_WIDTH  16
#define GRAPHICS_LAYER_HC0 3
#define GRAPHICS_LAYER_G0  0
#define GRAPHICS_LAYER_G1  1

HI_S32 HISI_SYS_Init(int screenWidth,int screnHeight) {
    VB_CONF_S stVbConf;
    memset(&stVbConf,0,sizeof(VB_CONF_S));
    stVbConf.u32MaxPoolCnt = 128;
    stVbConf.astCommPool[0].u32BlkSize = CEILING_2_POWER(screenWidth,SAMPLE_SYS_ALIGN_WIDTH)
                                         * CEILING_2_POWER(screnHeight,SAMPLE_SYS_ALIGN_WIDTH) *2;
    stVbConf.astCommPool[0].u32BlkCnt =  6;

    MPP_SYS_CONF_S stSysConf = {0};
    HI_S32 s32Ret = HI_FAILURE;
    HI_S32 i;

    HI_MPI_SYS_Exit();

    for(i=0; i<VB_MAX_USER; i++) {
        HI_MPI_VB_ExitModCommPool(i);
    }
    for(i=0; i<VB_MAX_POOLS; i++) {
        HI_MPI_VB_DestroyPool(i);
    }
    HI_MPI_VB_Exit();

    s32Ret = HI_MPI_VB_SetConf(&stVbConf);
    if (HI_SUCCESS != s32Ret) {
        LOGE("HI_MPI_VB_SetConf failed!");
        return HI_FAILURE;
    }

    s32Ret = HI_MPI_VB_Init();
    if (HI_SUCCESS != s32Ret) {
        LOGE("HI_MPI_VB_Init failed!");
        return HI_FAILURE;
    }

    stSysConf.u32AlignWidth = SAMPLE_SYS_ALIGN_WIDTH;
    s32Ret = HI_MPI_SYS_SetConf(&stSysConf);
    if (HI_SUCCESS != s32Ret) {
        LOGE("HI_MPI_SYS_SetConf failed");
        return HI_FAILURE;
    }

    s32Ret = HI_MPI_SYS_Init();
    if (HI_SUCCESS != s32Ret) {
        LOGE("HI_MPI_SYS_Init failed!");
        return HI_FAILURE;
    }
    return HI_SUCCESS;
}

static HI_S32 VO_GetWH(VO_INTF_SYNC_E enIntfSync, HI_U32 *pu32W,HI_U32 *pu32H, HI_U32 *pu32Frm) {
    switch (enIntfSync) {
    case VO_OUTPUT_PAL       :
        *pu32W = 720;
        *pu32H = 576;
        *pu32Frm = 25;
        break;
    case VO_OUTPUT_NTSC      :
        *pu32W = 720;
        *pu32H = 480;
        *pu32Frm = 30;
        break;
    case VO_OUTPUT_576P50    :
        *pu32W = 720;
        *pu32H = 576;
        *pu32Frm = 50;
        break;
    case VO_OUTPUT_480P60    :
        *pu32W = 720;
        *pu32H = 480;
        *pu32Frm = 60;
        break;
    case VO_OUTPUT_800x600_60:
        *pu32W = 800;
        *pu32H = 600;
        *pu32Frm = 60;
        break;
    case VO_OUTPUT_720P50    :
        *pu32W = 1280;
        *pu32H = 720;
        *pu32Frm = 50;
        break;
    case VO_OUTPUT_720P60    :
        *pu32W = 1280;
        *pu32H = 720;
        *pu32Frm = 60;
        break;
    case VO_OUTPUT_1080I50   :
        *pu32W = 1920;
        *pu32H = 1080;
        *pu32Frm = 25;
        break;
    case VO_OUTPUT_1080I60   :
        *pu32W = 1920;
        *pu32H = 1080;
        *pu32Frm = 30;
        break;
    case VO_OUTPUT_1080P24   :
        *pu32W = 1920;
        *pu32H = 1080;
        *pu32Frm = 24;
        break;
    case VO_OUTPUT_1080P25   :
        *pu32W = 1920;
        *pu32H = 1080;
        *pu32Frm = 25;
        break;
    case VO_OUTPUT_1080P30   :
        *pu32W = 1920;
        *pu32H = 1080;
        *pu32Frm = 30;
        break;
    case VO_OUTPUT_1080P50   :
        *pu32W = 1920;
        *pu32H = 1080;
        *pu32Frm = 50;
        break;
    case VO_OUTPUT_1080P60   :
        *pu32W = 1920;
        *pu32H = 1080;
        *pu32Frm = 60;
        break;
    case VO_OUTPUT_1024x768_60:
        *pu32W = 1024;
        *pu32H = 768;
        *pu32Frm = 60;
        break;
    case VO_OUTPUT_1280x1024_60:
        *pu32W = 1280;
        *pu32H = 1024;
        *pu32Frm = 60;
        break;
    case VO_OUTPUT_1366x768_60:
        *pu32W = 1366;
        *pu32H = 768;
        *pu32Frm = 60;
        break;
    case VO_OUTPUT_1440x900_60:
        *pu32W = 1440;
        *pu32H = 900;
        *pu32Frm = 60;
        break;
    case VO_OUTPUT_1280x800_60:
        *pu32W = 1280;
        *pu32H = 800;
        *pu32Frm = 60;
        break;
    case VO_OUTPUT_1600x1200_60:
        *pu32W = 1600;
        *pu32H = 1200;
        *pu32Frm = 60;
        break;
    case VO_OUTPUT_1680x1050_60:
        *pu32W = 1680;
        *pu32H = 1050;
        *pu32Frm = 60;
        break;
    case VO_OUTPUT_1920x1200_60:
        *pu32W = 1920;
        *pu32H = 1200;
        *pu32Frm = 60;
        break;
    case VO_OUTPUT_3840x2160_30:
        *pu32W = 3840;
        *pu32H = 2160;
        *pu32Frm = 30;
        break;
    case VO_OUTPUT_3840x2160_60:
        *pu32W = 3840;
        *pu32H = 2160;
        *pu32Frm = 60;
        break;
    case VO_OUTPUT_USER    :
        *pu32W = 720;
        *pu32H = 576;
        *pu32Frm = 25;
        break;
    default:
        LOGD("vo enIntfSync not support!\n");
        return HI_FAILURE;
    }
    return HI_SUCCESS;
}

static HI_S32 HISI_VO_StartDev(VO_DEV VoDev, VO_PUB_ATTR_S *pstPubAttr) {
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = HI_MPI_VO_SetPubAttr(VoDev, pstPubAttr);
    if (s32Ret != HI_SUCCESS) {
        LOGE("failed with %#x!", s32Ret);
        return HI_FAILURE;
    }
    s32Ret = HI_MPI_VO_Enable(VoDev);
    if (s32Ret != HI_SUCCESS) {
        LOGE("failed with %#x!", s32Ret);
        return HI_FAILURE;
    }
    return s32Ret;
}
static HI_S32 HISI_VO_StartLayer(VO_LAYER VoLayer,const VO_VIDEO_LAYER_ATTR_S *pstLayerAttr) {
    HI_S32 s32Ret = HI_SUCCESS;
    s32Ret = HI_MPI_VO_SetVideoLayerAttr(VoLayer, pstLayerAttr);
    if (s32Ret != HI_SUCCESS) {
        LOGE("failed with %#x!", s32Ret);
        return HI_FAILURE;
    }
    s32Ret = HI_MPI_VO_EnableVideoLayer(VoLayer);
    if (s32Ret != HI_SUCCESS) {
        LOGE("failed with %#x!", s32Ret);
        return HI_FAILURE;
    }
    return s32Ret;
}
static HI_S32 HISI_VO_HdmiStart(VO_INTF_SYNC_E enIntfSync) {
    HI_HDMI_ATTR_S      stAttr;
    HI_HDMI_VIDEO_FMT_E enVideoFmt;

    //SAMPLE_COMM_VO_HdmiConvertSync(enIntfSync, &enVideoFmt);

    HI_MPI_HDMI_Init();

    HI_MPI_HDMI_Open(HI_HDMI_ID_0);

    HI_MPI_HDMI_GetAttr(HI_HDMI_ID_0, &stAttr);

    stAttr.bEnableHdmi = HI_TRUE;

    stAttr.bEnableVideo = HI_TRUE;
    stAttr.enVideoFmt = enVideoFmt;

    stAttr.enVidOutMode = HI_HDMI_VIDEO_MODE_YCBCR444;
    stAttr.enDeepColorMode = HI_HDMI_DEEP_COLOR_OFF;
    stAttr.bxvYCCMode = HI_FALSE;

    stAttr.bEnableAudio = HI_FALSE;
    stAttr.enSoundIntf = HI_HDMI_SND_INTERFACE_I2S;
    stAttr.bIsMultiChannel = HI_FALSE;

    stAttr.enBitDepth = HI_HDMI_BIT_DEPTH_16;

    stAttr.bEnableAviInfoFrame = HI_TRUE;
    stAttr.bEnableAudInfoFrame = HI_TRUE;
    stAttr.bEnableSpdInfoFrame = HI_FALSE;
    stAttr.bEnableMpegInfoFrame = HI_FALSE;

    stAttr.bDebugFlag = HI_FALSE;
    stAttr.bHDCPEnable = HI_FALSE;

    stAttr.b3DEnable = HI_FALSE;

    HI_MPI_HDMI_SetAttr(HI_HDMI_ID_0, &stAttr);

    HI_MPI_HDMI_Start(HI_HDMI_ID_0);

    LOGD("HDMI start success.");
    return HI_SUCCESS;
}

HI_S32 HISI_VO_Init() {
    HI_S32 s32Ret=0;
    VO_DEV VoDev = 0;//SAMPLE_VO_DEV_DHD0;
    VO_LAYER VoLayer = 0;
    SIZE_S  stSize;
    HI_U32 u32VoFrmRate;
    VO_PUB_ATTR_S stPubAttr;
    VO_VIDEO_LAYER_ATTR_S stLayerAttr;
    /*s32Ret=HI_MPI_VO_BindGraphicLayer(GRAPHICS_LAYER_HC0, VoDev);
    if(HI_SUCCESS != s32Ret) {
        LOGE("HI_MPI_VO_BindGraphicLayer failed");
        //return HI_FAILURE;
    }*/
    stPubAttr.enIntfSync = VO_OUTPUT_720P50;
    stPubAttr.enIntfType = VO_INTF_HDMI;//VGA;
    stPubAttr.u32BgColor = 0x0000FF;

    stLayerAttr.bClusterMode = HI_FALSE;
    stLayerAttr.bDoubleFrame = HI_FALSE;
    stLayerAttr.enPixFormat = PIXEL_FORMAT_RGB_8888;//PIXEL_FORMAT_YUV_SEMIPLANAR_420;
    s32Ret = VO_GetWH(stPubAttr.enIntfSync,&stSize.u32Width, &stSize.u32Height,&u32VoFrmRate);
    LOGE_IF(s32Ret!=HI_SUCCESS,"get vo wh failed with %d!", s32Ret);

    memcpy(&stLayerAttr.stImageSize,&stSize,sizeof(stSize));

    stLayerAttr.u32DispFrmRt = 30 ;
    stLayerAttr.stDispRect.s32X = 0;
    stLayerAttr.stDispRect.s32Y = 0;
    stLayerAttr.stDispRect.u32Width = stSize.u32Width;
    stLayerAttr.stDispRect.u32Height = stSize.u32Height;
    s32Ret = HISI_VO_StartDev(VoDev, &stPubAttr);
    LOGE_IF(s32Ret!=HI_SUCCESS,"start vo dev failed with %d!", s32Ret);
    s32Ret = HISI_VO_StartLayer(VoLayer, &stLayerAttr);
    LOGE_IF(s32Ret!=HI_SUCCESS,"start vo layer failed with %d!", s32Ret);
    if (stPubAttr.enIntfType & VO_INTF_HDMI) {
#ifndef HI_FPGA
        s32Ret = HISI_VO_HdmiStart(stPubAttr.enIntfSync);
        if (HI_SUCCESS != s32Ret) {
            LOGE("start HDMI failed with %d!", s32Ret);
        }
#endif
    }
    return HI_SUCCESS;
}
#if 0
void* HIFB_PANDISPLAY(void *pData) {
    HI_S32 i,x,y,s32Ret;
    HI_S32/*TDE_HANDLE*/ s32Handle;
    struct fb_fix_screeninfo fix;
    struct fb_var_screeninfo var;
    HI_U32 u32FixScreenStride = 0;
    HI_U8 *pShowScreen;
    HI_U8 *pHideScreen;
    HI_U32 u32HideScreenPhy = 0;
    HI_U16 *pShowLine;
    HI_U16 *ptemp = NULL;
    HIFB_ALPHA_S stAlpha= {0};
    HIFB_POINT_S stPoint = {40, 112};
    HI_CHAR file[12] = "/dev/fb0";

    HI_CHAR image_name[128];
    HI_U8 *pDst = NULL;
    HI_BOOL bShow;
    PTHREAD_HIFB_SAMPLE_INFO *pstInfo;
    HIFB_COLORKEY_S stColorKey;
    TDE2_RECT_S stSrcRect,stDstRect;
    TDE2_SURFACE_S stSrc,stDst;
    //HI_U32 Phyaddr;
    HI_VOID *Viraddr;

    prctl(PR_SET_NAME, "hi_PANDISPLAY", 0, 0, 0);

    if(HI_NULL == pData) {
        return HI_NULL;
    }
    pstInfo = (PTHREAD_HIFB_SAMPLE_INFO *)pData;
    switch (pstInfo->layer) {
    case GRAPHICS_LAYER_G0 :  strcpy(file, "/dev/fb0"); break;
    case GRAPHICS_LAYER_HC0 : strcpy(file, "/dev/fb3"); break;/*CURSOR LAYER*/
    default:      strcpy(file, "/dev/fb0");     break;
    }

    /* 1. open framebuffer device overlay 0 */
    pstInfo->fd = open(file, O_RDWR, 0);
    if(pstInfo->fd < 0) {
        LOGE("open %s failed!\n",file);
        return HI_NULL;
    }

    bShow = HI_FALSE;
    if (ioctl(pstInfo->fd, FBIOPUT_SHOW_HIFB, &bShow) < 0) {
        LOGE("FBIOPUT_SHOW_HIFB failed!\n");
        return HI_NULL;
    }
    /* 2. set the screen original position */
    switch(pstInfo->ctrlkey) {
    case GRAPHICS_LAYER_HC0:
        stPoint.s32XPos = 150;
        stPoint.s32YPos = 150;
        break;
    default:
        stPoint.s32XPos = 0;
        stPoint.s32YPos = 0;
        break;
    }

    if (ioctl(pstInfo->fd, FBIOPUT_SCREEN_ORIGIN_HIFB, &stPoint) < 0) {
        LOGE("set screen original show position failed!\n");
        close(pstInfo->fd);
        return HI_NULL;
    }

    /* 3. get the variable screen info */
    if (ioctl(pstInfo->fd, FBIOGET_VSCREENINFO, &var) < 0) {
        LOGE("Get variable screen info failed!\n");
        close(pstInfo->fd);
        return HI_NULL;
    }

    /* 4. modify the variable screen info
          the screen size: IMAGE_WIDTH*IMAGE_HEIGHT
          the virtual screen size: VIR_SCREEN_WIDTH*VIR_SCREEN_HEIGHT
          (which equals to VIR_SCREEN_WIDTH*(IMAGE_HEIGHT*2))
          the pixel format: ARGB1555  */
    usleep(4*1000*1000);
    switch(pstInfo->ctrlkey) {
    case GRAPHICS_LAYER_HC0:
        var.xres_virtual = 48;
        var.yres_virtual = 48;
        var.xres = 48;      var.yres = 48;
        break;
    default:
        var.xres_virtual = WIDTH;
        var.yres_virtual = HEIGHT*2;
        var.xres = WIDTH;   var.yres = HEIGHT;
        break;
    }

    var.transp= s_a16;
    var.red = s_r16;
    var.green = s_g16;
    var.blue = s_b16;
    var.bits_per_pixel = 16;
    var.activate = FB_ACTIVATE_NOW;

    /* 5. set the variable screeninfo */
    if (ioctl(pstInfo->fd, FBIOPUT_VSCREENINFO, &var) < 0) {
        LOGE("Put variable screen info failed!\n");
        close(pstInfo->fd);
        return HI_NULL;
    }

    /* 6. get the fix screen info */
    if (ioctl(pstInfo->fd, FBIOGET_FSCREENINFO, &fix) < 0) {
        LOGE("Get fix screen info failed!\n");
        close(pstInfo->fd);
        return HI_NULL;
    }
    u32FixScreenStride = fix.line_length;   /*fix screen stride*/

    /* 7. map the physical video memory for user use */
    pShowScreen = mmap(HI_NULL, fix.smem_len, PROT_READ|PROT_WRITE, MAP_SHARED, pstInfo->fd, 0);
    if(MAP_FAILED == pShowScreen) {
        LOGE("mmap framebuffer failed!\n");
        close(pstInfo->fd);
        return HI_NULL;
    }

    memset(pShowScreen, 0x00, fix.smem_len);

    /* time to play*/
    bShow = HI_TRUE;
    if (ioctl(pstInfo->fd, FBIOPUT_SHOW_HIFB, &bShow) < 0) {
        LOGE("FBIOPUT_SHOW_HIFB failed!\n");
        munmap(pShowScreen, fix.smem_len);
        close(pstInfo->fd);
        return HI_NULL;
    }
#if 0
    if (GRAPHICS_LAYER_HC0 != pstInfo->ctrlkey) {
        for (i = 0; i < 1; i++) {
            var.yoffset = (i % 2)?var.yres:0;
            ptemp = (HI_U16*)(pShowScreen + var.yres * u32FixScreenStride * (i % 2));
            for (y = 100; y < 300; y++) {
                for (x = 0; x < 300; x++) {
                    *(ptemp + y * var.xres + x) = HIFB_RED_1555;
                }
            }
            LOGE("expected: the red box will appear!\n");
            sleep(2);

            /* BVTGA-684 */
            if (ioctl(pstInfo->fd, FBIOGET_ALPHA_HIFB,  &stAlpha)) {
                LOGE("Get alpha failed!\n");
                close(pstInfo->fd);
                return HI_NULL;
            }

            stAlpha.bAlphaEnable = HI_TRUE;
            stAlpha.u8Alpha0 = 0x0;
            stAlpha.u8Alpha1 = 0x0;
            if (ioctl(pstInfo->fd, FBIOPUT_ALPHA_HIFB,  &stAlpha) < 0) {
                LOGE("Set alpha failed!\n");
                close(pstInfo->fd);
                return HI_NULL;
            }
            LOGE("expected: after set alpha = 0, the red box will disappear!\n");
            sleep(2);

            stAlpha.u8Alpha0 = 0;
            stAlpha.u8Alpha1 = 0xFF;
            if (ioctl(pstInfo->fd, FBIOPUT_ALPHA_HIFB,  &stAlpha) < 0) {
                LOGE("Set alpha failed!\n");
                close(pstInfo->fd);
                return HI_NULL;
            }
            LOGE("expected:after set set alpha = 0xFF, the red box will appear again!\n");
            sleep(2);

            LOGE("expected: the red box will erased by colorkey!\n");
            stColorKey.bKeyEnable = HI_TRUE;
            stColorKey.u32Key = HIFB_RED_1555;
            s32Ret = ioctl(pstInfo->fd, FBIOPUT_COLORKEY_HIFB, &stColorKey);
            if (s32Ret < 0) {
                LOGE("FBIOPUT_COLORKEY_HIFB failed!\n");
                close(pstInfo->fd);
                return HI_NULL;
            }
            sleep(2);
            LOGE("expected: the red box will appear again!\n");
            stColorKey.bKeyEnable = HI_FALSE;
            s32Ret = ioctl(pstInfo->fd, FBIOPUT_COLORKEY_HIFB, &stColorKey);
            if (s32Ret < 0) {
                LOGE("FBIOPUT_COLORKEY_HIFB failed!\n");
                close(pstInfo->fd);
                return HI_NULL;
            }
            sleep(2);
	}
    }
#endif
    /* unmap the physical memory */
    munmap(pShowScreen, fix.smem_len);
    bShow = HI_FALSE;
    if (ioctl(pstInfo->fd, FBIOPUT_SHOW_HIFB, &bShow) < 0) {
        LOGE("FBIOPUT_SHOW_HIFB failed!\n");
        close(pstInfo->fd);
        return HI_NULL;
    }
    close(pstInfo->fd);
    return HI_NULL;
}
#endif
void HISI_SYS_Exit(void) {

    HI_S32 i;

    HI_MPI_SYS_Exit();
    for(i=0; i<VB_MAX_USER; i++) {
        HI_MPI_VB_ExitModCommPool(i);
    }
    for(i=0; i<VB_MAX_POOLS; i++) {
        HI_MPI_VB_DestroyPool(i);
    }
    HI_MPI_VB_Exit();
    return;
}

