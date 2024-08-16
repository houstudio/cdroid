#ifndef __NGL_DISP_H__
#define __NGL_DISP_H__

BEGIN_DECLS
typedef enum{
    DISP_RES_1080P,
    DISP_RES_1080I,
    DISP_RES_720P,
    DISP_RES_720I,
    DISP_RES_576P,
    DISP_RES_576I,
    DISP_RES_480I
}DISP_RESOLUTION;

typedef enum{
    DISP_APR_AUTO,
    DISP_APR_4_3,
    DISP_APR_16_9
}DISP_ASPECT_RATIO;

typedef enum{
    DISP_MM_PANSCAN,
    DISP_MM_LETTERBOX,
    DISP_MM_PILLBOX,
    DISP_MM_NORMAL_SCALE,
    DISP_COMBINED_SCALE
}DISP_SCALE_MATCH;

typedef struct{
    uint16_t RedOffset;
    uint16_t GreenOffset;
    uint16_t BlueOffset;
    uint16_t RedColor; // 00~FF, 0x80 is no change
    uint16_t GreenColor;// 00~FF, 0x80 is no change
    uint16_t BlueColor; // 00~FF, 0x80 is no change
}DISP_ColorTemperature;

int32_t DispInit();
int32_t DispSetResolution(int dev,int res);
int32_t DispSetAspectRatio(int dev,int ratio);
int32_t DispGetAspectRatio(int dev,int *ratio);
int32_t DispSetMatchMode(int dev,int md);

/*value 0=100*/
int32_t DispSetBrightNess(int dev,int value);
int32_t DispSetContrast(int dev,int value);
int32_t DispSetSaturation(int dev,int value);

int32_t DispSetColorTemp(int dev,const DISP_ColorTemperature*);
int32_t DispGetColorTemp(int dev,DISP_ColorTemperature*);

/*DispSetGamma
 * 
 * entryNumber Gamma Entry number,-1(0xFFFFFFFF):disable gamma
 * */
int32_t DispSetGamma(int dev,uint8_t*r,uint8_t*g,uint8_t*b,uint32_t entryNumber);
int32_t DispGetGamma(int dev,uint8_t*r,uint8_t*g,uint8_t*b,uint32_t*entryNumber);
END_DECLS

#endif
