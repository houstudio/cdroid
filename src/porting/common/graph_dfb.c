#include <cdtypes.h>
#include <ngl_os.h>
#include <cdgraph.h>
#include <cdlog.h>
#include <directfb.h>
#include <directfb_util.h>
#include <direct/messages.h>
#include <cdlog.h>

static IDirectFB *directfb=NULL;
static IDirectFBSurface*primarySurface;
static IDirectFBDisplayLayer *primaryLayer;
static GFXRect screenMargin= {0,0,0,0};

static void DFBDisplayLayerCBK(DFBDisplayLayerID layer_id, DFBDisplayLayerDescription  desc, void *callbackdata) {
    LOGD("Layer %d[%s] type:%x surface.caps=%x accessor=%x",layer_id,desc.name,desc.type,desc.surface_caps,desc.surface_accessor);
}

int32_t GFXInit() {
    DFBDisplayLayerConfig dispCfg;
    const char*cfg=getenv("DIRECTFB_CONFIGFILE");
    if(directfb!=NULL)return E_OK;
    DirectFBInit (NULL,NULL);
    if(cfg) {
        LOGI("directfbrc=%s",cfg);
        DirectFBSetOption("config-file", cfg);
    }
    int ret=DirectFBCreate (&directfb);
    LOGI("DirectFBCreate=%x dfb=%p cfg=%s",ret,directfb,cfg);
    directfb->GetDisplayLayer(directfb, DLID_PRIMARY, &primaryLayer);
    primaryLayer->GetConfiguration(primaryLayer, &dispCfg );
    directfb->EnumDisplayLayers(directfb,DFBDisplayLayerCBK,NULL);
    directfb->SetCooperativeLevel (directfb, DFSCL_FULLSCREEN);

    DFBSurfaceDescription   desc;
    memset(&desc,0,sizeof(DFBSurfaceDescription));
    desc.flags=DSDESC_CAPS;
    desc.caps=DSCAPS_PRIMARY;//|DSCAPS_FLIPPING;
    LOGI("DirectFB ScreenSize %dx%d",dispCfg.width,dispCfg.height);
    directfb->SetCooperativeLevel( directfb, DFSCL_FULLSCREEN );
    directfb->CreateSurface( directfb, &desc,&primarySurface);
    return E_OK;
}

int32_t GFXGetDisplayCount() {
    return 1;
}

int32_t GFXGetDisplaySize(int32_t disp,uint32_t*width,uint32_t*height) {
    DFBDisplayLayerConfig dispCfg;
    primaryLayer->GetConfiguration(primaryLayer, &dispCfg );
    *width =dispCfg.width  - screenMargin.x - screenMargin.w;
    *height=dispCfg.height- screenMargin.y - screenMargin.h;
    LOGV("screensize=%d,%d margin=(%d,%d,%d,%d)",*width,*height,
         screenMargin.x,screenMargin.y,screenMargin.w,screenMargin.h);
    return E_OK;
}

GFXHANDLE GFXCreateCursor(const GFXCursorImage*cursorImage){
    return (GFXHANDLE)0;
}

void GFXAttachCursor(GFXHANDLE cursorHandle){
}
void GFXMoveCursor(int32_t xPos,int32_t yPos){
}

void GFXDestroyCursor(GFXHANDLE cursorHandle){
}

int32_t GFXLockSurface(GFXHANDLE surface,void**buffer,uint32_t*pitch) {
    IDirectFBSurface*surf=(IDirectFBSurface*)surface;
    int ret=surf->Lock(surf,DSLF_READ | DSLF_WRITE,buffer,pitch);
    LOGV_IF(ret,"surface=%p buffer=%p pitch=%d",surf,buffer,*pitch);
    return ret;
}

int32_t GFXGetSurfaceInfo(GFXHANDLE surface,uint32_t*width,uint32_t*height,int32_t *format) {
    IDirectFBSurface*surf=(IDirectFBSurface*)surface;
    if(NULL==width||NULL==height)
        return E_INVALID_PARA;
    int ret=surf->GetSize(surf,width,height);
    if(format)*format=GPF_ARGB;
    LOGV("surface=%p,format=%d ret=%d",(surface?surface:0),(format?*format:0),ret);
    return ret==0?E_OK:E_ERROR;
}

int32_t GFXUnlockSurface(GFXHANDLE surface) {
    IDirectFBSurface*surf=(IDirectFBSurface*)surface;
    int ret=surf->Unlock(surf);
    LOGV_IF(ret,"surface=%p ret=%d",surface,ret);
    return ret;
}

int32_t GFXSurfaceSetOpacity(GFXHANDLE surface,uint8_t alpha) {
    LOGV("setopacity=%d",alpha);
    IDirectFBSurface*surf=(IDirectFBSurface*)surface;
    IDirectFBDisplayLayer *dispLayer;
    directfb->GetDisplayLayer( directfb, DLID_PRIMARY, &dispLayer );
    LOGE_IF(NULL==surface,"g_hw_layer is null,global alpha setting failed");
    return dispLayer->SetOpacity(dispLayer,alpha);
}

int32_t GFXFillRect(GFXHANDLE surface,const GFXRect*rec,uint32_t color) {
    IDirectFBSurface*surf=(IDirectFBSurface*)surface;
    GFXRect r= {0,0,0,0};
    if(NULL==rec)
        surf->GetSize(surf,&r.w,&r.h);
    else
        r=*rec;
    surf->SetColor(surf,(color>>24),(color>>16),(color>>8),color);
    surf->FillRectangle(surf,r.x,r.y,r.w,r.h);
    return E_OK;
}

int32_t GFXFlip(GFXHANDLE surface) {
    IDirectFBSurface*dfbsrc=(IDirectFBSurface*)surface;
    IDirectFBSurface*dfbdst=primarySurface;
    DFBRegion clip;
    primarySurface->GetClip(primarySurface,&clip);
    const int ret=0;//primarySurface->Flip(primarySurface,&clip, DSFLIP_ONSYNC);
    return ret;
}

int32_t GFXCreateSurface(int32_t dispid,GFXHANDLE*surface,uint32_t width,uint32_t height,int32_t format,bool hwsurface) {
    int i,ret;
    DFBSurfaceDescription   desc;
    IDirectFBSurface*dfbsurface;
    DFBSurfaceID surface_id;
    void*data;
    int pitch;
    memset(&desc,0,sizeof(DFBSurfaceDescription));
    if(hwsurface) {
        DFBDisplayLayerConfig dispCfg;
        primaryLayer->GetConfiguration(primaryLayer, &dispCfg );
        desc.flags = (DSDESC_CAPS|DSDESC_WIDTH | DSDESC_HEIGHT | DSDESC_PIXELFORMAT);//|DSDESC_PREALLOCATED);
        desc.caps = DSCCAPS_NONE;//DSCAPS_FLIPPING;
        width = dispCfg.width;
        height= dispCfg.height;
    } else {
        desc.flags=(DSDESC_CAPS| DSDESC_WIDTH | DSDESC_HEIGHT|DSDESC_PIXELFORMAT);
        desc.caps =DSCAPS_NONE;
    }
    desc.width=width;
    desc.height=height;
    desc.pixelformat=DSPF_ARGB;
    ret = directfb->CreateSurface( directfb, &desc,&dfbsurface);
    LOGD_IF(ret,"surface=%x  ishw=%d",dfbsurface,hwsurface);
    if(!hwsurface)dfbsurface->MakeClient(dfbsurface);
    dfbsurface->Clear(dfbsurface,0,0,0,0);
    *surface = (GFXHANDLE)dfbsurface;
    return E_OK;
}

int32_t GFXSetSurfaceColorKey(GFXHANDLE surface,uint32_t color) {
    IDirectFBSurface*dfbsrc=(IDirectFBSurface*)surface;
}

int32_t GFXBlit(GFXHANDLE dstsurface,int dx,int dy,GFXHANDLE srcsurface,const GFXRect*srcrect) {
    int ret,dstwidth,dstheight;
    GFXRect rs = {0,0},rd;
    IDirectFBSurface*dfbsrc = (IDirectFBSurface*)srcsurface;
    IDirectFBSurface*dfbdst = (IDirectFBSurface*)dstsurface;
    DFBRegion region;

    dfbdst = primarySurface;
    dfbdst->GetSize(dfbdst,&dstwidth,&dstheight);
    dfbsrc->GetSize(dfbsrc,(int*)&rs.w,(int*)&rs.h);

    if(srcrect)rs=*srcrect;

    if(dx<0) {
        rs.x-=dx;
        rs.w=(int)rs.w+dx;
        dx=0;
    }
    if(dy<0) {
        rs.y-=dy;
        rs.h=(int)rs.h+dy;
        dy=0;
    }
    if(dx+rs.w > dstwidth - screenMargin.x - screenMargin.w)
        rs.w = dstwidth - screenMargin.x - screenMargin.w -dx;
    if(dy+rs.h > dstheight- screenMargin.y - screenMargin.h)
        rs.h = dstheight- screenMargin.y - screenMargin.h -dy;

    dfbdst->SetPorterDuff(dfbdst,DSPD_SRC);
    const int ox=dx,oy=dy;
    rd = rs;
    dx += screenMargin.x;
    dy += screenMargin.y;
    dfbdst->Blit(dfbdst,dfbsrc,&rs,dx,dy);
    region.x1= dx;
    region.y1= dy;
    region.x2= dx + rd.w;
    region.y2= dy + rd.h;
    //dfbdst->SetClip(dfbdst, &region);
    LOGV("dstsurface=%p/primarySurface=%p srcsurface=%p (%d,%d,%d,%d) to pos(%d,%d)/(%d,%d)",
         dstsurface,primarySurface,srcsurface,rs.x,rs.y,rs.w,rs.h,ox,oy,dx,dy);
    return ret;
}

int32_t GFXDestroySurface(GFXHANDLE surface) {
    LOGV("surface=%p",surface);
    IDirectFBSurface*dfbsurface = (IDirectFBSurface*)surface;
    return dfbsurface->Release(dfbsurface);
}
