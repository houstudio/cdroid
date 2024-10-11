#ifndef __NGL_FRONT_PANNEL_H__
#define __NGL_FRONT_PANNEL_H__
#include<cdtypes.h>

BEGIN_DECLS

DWORD nglFPInit();
DWORD nglFPShowText(const char*text,int len);
DWORD nglFPSetBrightness(int value);
END_DECLS
#endif

