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

DWORD nglDispInit();
DWORD nglDispSetResolution(int res);
DWORD nglDispSetAspectRatio(int ratio);
DWORD nglDispGetAspectRatio(int *ratio);
DWORD nglDispSetMatchMode(int md);
/*value 0=100*/
DWORD nglDispSetBrightNess(int value);
DWORD nglDispSetContrast(int value);
DWORD nglDispSetSaturation(int value);

END_DECLS

#endif
