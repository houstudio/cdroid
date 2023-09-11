#include "gfxdrm.h"
#include "cdgraph.h"
#include "cdtypes.h"
#include "cdlog.h"

static GFXDrm *drm=nullptr;//("/dev/dri/card0");

INT GFXInit() {
    drm=new GFXDrm("/dev/dri/card0");
    std::vector<GFXDrm::GFXMode>modes;
    GFXDrm::GFXFB fb;
    drm->fetchModes(modes);
    for(int i=0; i<modes.size(); i++) {
        LOGD("mode[%d]=%dx%dx%d fps",i,modes[i].width,modes[i].height,modes[i].mode.vrefresh);
    }
    return 0;
}

INT GFXGetDisplaySize(int dispid,UINT*width,UINT*height) {
    return 0;
}

INT GFXGetDisplayCount(){
    return 1;
}

INT GFXLockSurface(HANDLE surface,void**buffer,UINT*pitch) {
    return 0;
}

INT GFXGetSurfaceInfo(HANDLE surface,UINT*width,UINT*height,INT *format) {
    return 0;
}

INT GFXUnlockSurface(HANDLE surface) {
    return 0;
}

INT GFXSurfaceSetOpacity(HANDLE surface,BYTE alpha) {
    return 0;
}

INT GFXFillRect(HANDLE surface,const GFXRect*rect,UINT color) {
    return 0;
}

INT GFXFlip(HANDLE surface) {
    return 0;
}

INT GFXCreateSurface(int,HANDLE*surface,UINT width,UINT height,INT format,BOOL hwsurface) {
    return 0;
}

INT GFXBlit(HANDLE dstsurface,int dx,int dy,HANDLE srcsurface,const GFXRect*srcrect) {
    return 0;
}

INT GFXDestroySurface(HANDLE surface) {
    return 0;
}

