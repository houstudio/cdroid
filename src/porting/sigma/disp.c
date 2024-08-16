#include<cdtypes.h>
#include<cdlog.h>
#include<cddisp.h>
#include <string.h>
#include "mi_disp.h"
#include "mi_disp_datatype.h"

int32_t DispInit(){
    return E_OK;
}

int32_t DispSetResolution(int dev,int res){
    int rc;
    unsigned int tve_src=0;
    unsigned long output_width = 1280;
    unsigned long output_height = 720;
    BOOL progressive=FALSE;
    switch(res){
    case DISP_RES_1080I:
    case DISP_RES_1080P:
	    output_width = 1920;
	    output_height = 1080;
	    progressive =res==DISP_RES_1080P;
        break;
    case DISP_RES_720I:
    case DISP_RES_720P:
	    output_width = 1280;
	    output_height = 720;
	    progressive = res==DISP_RES_720P;
        break;
    case DISP_RES_576I:
    case DISP_RES_576P:
	    output_width = 720;
	    output_height = 576;
	    progressive = true;
    case DISP_RES_480I:
        output_width=720;
        output_height=480;
        progressive =FALSE;
    default:break;
    } 
    return E_OK;
}

int32_t DispSetAspectRatio(int dev,int ratio){
    int rc;
    switch(ratio){
    case DISP_APR_AUTO:break;
    case DISP_APR_4_3: break;
    case DISP_APR_16_9:break;
    default:LOGD("Invalid value %d",ratio);return E_ERROR;
    }    
    LOGD("ratio=%d rc=%d",ratio,rc);
    return rc;
}

int32_t DispGetAspectRatio(int dev,int*ratio){
    return E_OK;
}

int32_t DispSetMatchMode(int dev,int md){
    int rc;
    switch(md){
    case DISP_MM_PANSCAN :break;
    case DISP_MM_LETTERBOX:break;
    case DISP_MM_PILLBOX :break;
    case DISP_MM_NORMAL_SCALE:break;
    case DISP_COMBINED_SCALE:break;
    return E_ERROR;
    }
    LOGD("md=%d rc=%d",md,rc);
    return rc;
}

int32_t DispSetBrightNess(int dev,int value){
    int rc;
    MI_DISP_LcdParam_t st_lcdParam_t;
    memset(&st_lcdParam_t, 0, sizeof(MI_DISP_LcdParam_t));
    MI_DISP_GetLcdParam(0,&st_lcdParam_t);
    st_lcdParam_t.stCsc.u32Luma = value;
    rc=MI_DISP_SetLcdParam(0,&st_lcdParam_t);
    LOGD("rc=%d value=%d",rc,value);
    return rc;
}

int32_t DispSetContrast(int dev,int value){
    int rc;
    MI_DISP_LcdParam_t st_lcdParam_t;
    memset(&st_lcdParam_t, 0, sizeof(MI_DISP_LcdParam_t));
    MI_DISP_GetLcdParam(0,&st_lcdParam_t);
    st_lcdParam_t.stCsc.eCscMatrix = value;
    rc = MI_DISP_SetLcdParam(0,&st_lcdParam_t);
    LOGD("rc=%d value=%d",rc,value);
    return rc;
}

int32_t DispSetSaturation(int dev,int value){
    int rc;
    MI_DISP_LcdParam_t st_lcdParam_t;
    memset(&st_lcdParam_t, 0, sizeof(MI_DISP_LcdParam_t));
    MI_DISP_GetLcdParam(0,&st_lcdParam_t);
    st_lcdParam_t.stCsc.u32Saturation = value;
    MI_DISP_SetLcdParam(0,&st_lcdParam_t);
    LOGD("rc=%d value=%d ",value,rc);
    return rc;
}

int32_t DispSetColorTemp(int dev,const DISP_ColorTemperature*colorTemp){
    return 0;
}

int32_t DispGetColorTemp(int dev,DISP_ColorTemperature*colorTemp){
    return 0;
}

/*DispSetGamma
 *
 * entryNumber Gamma Entry number,-1(0xFFFFFFFF):disable gamma
 * */
int32_t DispSetGamma(int dev,uint8_t*r,uint8_t*g,uint8_t*b,uint32_t entryNumber){
    return 0;
}

int32_t DispGetGamma(int dev,uint8_t*r,uint8_t*g,uint8_t*b,uint32_t *entryNumber){
    return 0;
}
