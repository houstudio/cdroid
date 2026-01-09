#include<cdtypes.h>
#include<cdlog.h>
#include<cddisp.h>
#include <string.h>

int32_t DispInit(){
    return E_OK;
}

int32_t DispSetResolution(int dev,int res){
    return E_OK;
}

int32_t DispSetAspectRatio(int dev,int ratio){
    int rc=0;
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
    int rc=0;
    LOGD("rc=%d value=%d",rc,value);
    return rc;
}

int32_t DispSetContrast(int dev,int value){
    int rc=0;
    LOGD("rc=%d value=%d",rc,value);
    return rc;
}

int32_t DispSetSaturation(int dev,int value){
    int rc=0;
    LOGD("rc=%d value=%d ",value,rc);
    return rc;
}

int32_t DispSetColorTemp(int dev,const DISP_ColorTemperature*colorTemp){
    int rc = 0;
    return 0;
}

int32_t DispGetColorTemp(int dev,DISP_ColorTemperature*colorTemp){
    int rc = 0;
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
    //MI_DISP_GammaParam_t gm;
    //MI_DISP_DeviceSetGammaParam(0,&gm);
    return 0;
}

