#ifndef __DTV_UTILS_H__
#define __DTV_UTILS_H__
#include <porting/cdtypes.h>
#include <ngl_tuner.h>

namespace tvutils{

int GetTuningParams(NGLTunerParam&tp,int*lnb=nullptr,int*_22k=nullptr);

}

#endif
