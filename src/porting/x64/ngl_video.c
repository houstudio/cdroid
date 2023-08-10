#include <cdtypes.h>
#include <ngl_video.h>
#include <cdlog.h>
#include <ngl_disp.h>

typedef struct {
} NGLAV;

#define NB_DEMUX 4

static NGLAV sAvPlayers[NB_DEMUX];


INT nglAvInit() {
}

INT nglAvPlay(int dmxid,int vid,int vtype,int aid,int atype,int pcr) {
    int rc=0;
    LOGD("aui_av_start=%d video=%d/%d audio=%d/%d pcr=%d",rc,vid,vtype,aid,atype,pcr);
    return rc;
}

INT nglAvStop(int dmxid) {
    return E_OK;
}

INT nglAvSetVideoWindow(int dmxid,const GFXRect*inRect,const GFXRect*outRect) {
    NGLAV*av=&sAvPlayers[dmxid];
    GFXRect src= {0,0,1280,720};
    GFXRect dst= {0,0,1280,720};
    if(inRect)src=*inRect;
    if(outRect)dst=*outRect;
    src.x=src.x*720/1280;
    src.y=src.y*2880/720;
    src.w=src.w*720/1280;
    src.h=src.h*2880/720;

    dst.x=dst.x*720/1280;
    dst.y=dst.y*2880/720;
    dst.w=dst.w*720/1280;
    dst.h=dst.h*2880/720;
    int rc;
    return rc;
}
