#include "gfxdrm.h"
#include "cdgraph.h"
#include "cdtypes.h"
#include "cdlog.h"

static GFXDrm *drm=nullptr;//("/dev/dri/card0");

DWORD GFXInit(){
    drm=new GFXDrm("/dev/dri/card0");
    std::vector<GFXDrm::GFXMode>modes;
    GFXDrm::GFXFB fb;
    drm->fetchModes(modes);
    for(int i=0;i<modes.size();i++){
        LOGD("mode[%d]=(%dx%d)",i,modes[i].width,modes[i].height);
    }
    return 0;
}

DWORD GFXGetScreenSize(UINT*width,UINT*height){
    return 0;
}

DWORD GFXLockSurface(HANDLE surface,void**buffer,UINT*pitch){
    return 0;
}

DWORD GFXGetSurfaceInfo(HANDLE surface,UINT*width,UINT*height,INT *format){
    return 0;
}

DWORD GFXUnlockSurface(HANDLE surface){
    return 0;
}

DWORD GFXSurfaceSetOpacity(HANDLE surface,BYTE alpha){
    return 0;
}

DWORD GFXFillRect(HANDLE surface,const GFXRect*rect,UINT color){
    return 0;
}

DWORD GFXFlip(HANDLE surface){
    return 0;
}

DWORD GFXCreateSurface(HANDLE*surface,UINT width,UINT height,INT format,BOOL hwsurface){
    return 0;
}

DWORD GFXBlit(HANDLE dstsurface,int dx,int dy,HANDLE srcsurface,const GFXRect*srcrect){
    return 0;
}

DWORD GFXDestroySurface(HANDLE surface){
    return 0;
}

