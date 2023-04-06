#include <cdtypes.h>
#include <ngl_os.h>
#include <cdgraph.h>
#include <cdlog.h>
#include <directfb.h>
#include <directfb_util.h>
#include <direct/messages.h>
#include <cdlog.h>

NGL_MODULE(GRAPH);

static IDirectFB *directfb=NULL;
static IDirectFBSurface*primarySurface;
static IDirectFBDisplayLayer *primaryLayer;

static void DFBDisplayLayerCBK(DFBDisplayLayerID layer_id, DFBDisplayLayerDescription  desc, void *callbackdata){
    LOGD("Layer %d[%s] type:%x surface.caps=%x accessor=%x",layer_id,desc.name,desc.type,desc.surface_caps,desc.surface_accessor);
}

INT GFXInit()
{
    DFBDisplayLayerConfig dispCfg;
    char*cfg=getenv("DIRECTFB_CONFIGFILE");
    if(directfb!=NULL)return E_OK;
    DirectFBInit (NULL,NULL);
    if(cfg){
	 LOGI("directfbrc=%s",cfg);
	 DirectFBSetOption("config-file", cfg);
    }
    DirectFBCreate (&directfb);
    directfb->GetDisplayLayer(directfb, DLID_PRIMARY, &primaryLayer);
    primaryLayer->GetConfiguration(primaryLayer, &dispCfg );
    directfb->EnumDisplayLayers(directfb,DFBDisplayLayerCBK,NULL);
    directfb->SetCooperativeLevel (directfb, DFSCL_FULLSCREEN);
    
    DFBSurfaceDescription   desc;
    memset(&desc,0,sizeof(DFBSurfaceDescription));
    desc.flags=DSDESC_CAPS;
    desc.caps=DSCAPS_PRIMARY;
    LOGI("DirectFB ScreenSize %dx%d",dispCfg.width,dispCfg.height);
    directfb->CreateSurface( directfb, &desc,&primarySurface);

    return E_OK;
}
static int displayRotations[8];
INT GFXGetDisplayCount(){
    return 1;
}

INT GFXGetDisplaySize(INT disp,UINT*width,UINT*height){
    DFBDisplayLayerConfig dispCfg;
    GFXInit();
    primaryLayer->GetConfiguration(primaryLayer, &dispCfg );
    *width=dispCfg.width;
    *height=dispCfg.height;
    LOGV("screensize=%d,%d",*width,*height);
    return E_OK;
}

INT GFXSetRotation(int dispid,GFX_ROTATION rotation){
    switch(rotation){
    case ROTATE_0  : primaryLayer->SetRotation(primaryLayer, 0) ; break;
    case ROTATE_90 : primaryLayer->SetRotation(primaryLayer, 90); break;
    case ROTATE_180: primaryLayer->SetRotation(primaryLayer,180); break;
    case ROTATE_270: primaryLayer->SetRotation(primaryLayer,270); break;		     
    }
    if( (rotation>=ROTATE_0) && (rotation<=ROTATE_270) &&
        (dispid>=0) && (dispid<GFXGetDisplayCount()) )
        displayRotations[dispid]=rotation;
    return E_OK;
}

GFX_ROTATION GFXGetRotation(int dispid){
    if((dispid>=0)&&(dispid<GFXGetDisplayCount()))
	return displayRotations[dispid];
    return 0;
}
INT GFXLockSurface(HANDLE surface,void**buffer,UINT*pitch){
    IDirectFBSurface*surf=(IDirectFBSurface*)surface;
    int ret=surf->Lock(surf,DSLF_READ | DSLF_WRITE,buffer,pitch);
    LOGV_IF(ret,"surface=%p buffer=%p pitch=%d",surf,buffer,*pitch);
    return ret;
}

INT GFXGetSurfaceInfo(HANDLE surface,UINT*width,UINT*height,INT *format)
{
    IDirectFBSurface*surf=(IDirectFBSurface*)surface;
    if(NULL==width||NULL==height)
       return E_INVALID_PARA;
    int ret=surf->GetSize(surf,width,height);
    if(format)*format=GPF_ARGB;
    LOGV("surface=%p,format=%d ret=%d",(surface?surface:0),(format?*format:0),ret);
    return ret==0?E_OK:E_ERROR;
}

INT GFXUnlockSurface(HANDLE surface){
    IDirectFBSurface*surf=(IDirectFBSurface*)surface;
    int ret=surf->Unlock(surf);
    LOGV_IF(ret,"surface=%p ret=%d",surface,ret);
    return ret;
}

INT GFXSurfaceSetOpacity(HANDLE surface,BYTE alpha){
    LOGV("setopacity=%d",alpha);
    IDirectFBSurface*surf=(IDirectFBSurface*)surface;
    IDirectFBDisplayLayer *dispLayer;
    directfb->GetDisplayLayer( directfb, DLID_PRIMARY, &dispLayer );
    LOGE_IF(NULL==surface,"g_hw_layer is null,global alpha setting failed");
    return dispLayer->SetOpacity(dispLayer,alpha);
}

INT GFXFillRect(HANDLE surface,const GFXRect*rec,UINT color){
    IDirectFBSurface*surf=(IDirectFBSurface*)surface;
    GFXRect r={0,0,0,0};
    if(NULL==rec)
        surf->GetSize(surf,&r.w,&r.h);
    else
        r=*rec;
    surf->SetColor(surf,(color>>24),(color>>16),(color>>8),color);
    surf->FillRectangle(surf,r.x,r.y,r.w,r.h);
    return E_OK;
}

INT GFXFlip(HANDLE surface){
    IDirectFBSurface*surf=(IDirectFBSurface*)surface;
    int ret=surf->Flip( surf, NULL, DSFLIP_NONE);//ONSYNC);
    return ret;
}

INT GFXCreateSurface(INT dispid,HANDLE*surface,UINT width,UINT height,INT format,BOOL hwsurface)
{
     int i,ret;
     DFBSurfaceDescription   desc;
     IDirectFBSurface*dfbsurface;
     DFBSurfaceID surface_id;
     void*data;
     int pitch;
     memset(&desc,0,sizeof(DFBSurfaceDescription));
     if(hwsurface){
         desc.flags=(DSDESC_CAPS|DSDESC_WIDTH | DSDESC_HEIGHT | DSDESC_PIXELFORMAT);//|DSDESC_PREALLOCATED);
         desc.caps=DSCCAPS_NONE;//DSCAPS_FLIPPING;
     }else{
         desc.flags=(DSDESC_CAPS| DSDESC_WIDTH | DSDESC_HEIGHT|DSDESC_PIXELFORMAT);
         desc.caps =DSCAPS_NONE;
     }
     desc.width=width;
     desc.height=height;
     desc.pixelformat=DSPF_ARGB;
     ret=directfb->CreateSurface( directfb, &desc,&dfbsurface);
     LOGD_IF(ret,"surface=%x  ishw=%d",dfbsurface,hwsurface);
     if(!hwsurface)dfbsurface->MakeClient(dfbsurface);
     dfbsurface->Clear(dfbsurface,0,0,0,0);
     *surface=(HANDLE)dfbsurface;
     return E_OK;
}

INT GFXSetSurfaceColorKey(HANDLE surface,UINT color){
      IDirectFBSurface*dfbsrc=(IDirectFBSurface*)surface;
}

INT GFXBlit(HANDLE dstsurface,int dx,int dy,HANDLE srcsurface,const GFXRect*srcrect)
{
     int ret,dstwidth,dstheight;
     GFXRect rs={0,0};
     IDirectFBSurface*dfbsrc=(IDirectFBSurface*)srcsurface;
     IDirectFBSurface*dfbdst=(IDirectFBSurface*)dstsurface;

     dfbdst = primarySurface;
     dfbdst->GetSize(dfbdst,&dstwidth,&dstheight);
     dfbsrc->GetSize(dfbsrc,(int*)&rs.w,(int*)&rs.h);

     if(srcrect)rs=*srcrect;

     dfbdst->SetPorterDuff(dfbdst,DSPD_SRC_OVER);
     const int ox=dx,oy=dy;
     switch(GFXGetRotation(0)){/*directfb's rotation is clockwise*/
     case ROTATE_0 : dfbdst->SetBlittingFlags(dfbdst,DSBLIT_NOFX);
		     break;
     case ROTATE_90: dx = oy; 
		     dy = dstheight -ox - rs.w;  
		     dfbdst->SetBlittingFlags(dfbdst,DSBLIT_ROTATE90);
		     break;
     case ROTATE_180:dx = dstwidth -ox -rs.w;
                     dy = dstheight-oy -rs.h;
		     dfbdst->SetBlittingFlags(dfbdst,DSBLIT_ROTATE180);
		     break;
     case ROTATE_270:dx = dstwidth -oy -rs.h ;
		     dy = ox;
		     dfbdst->SetBlittingFlags(dfbdst,DSBLIT_ROTATE270);
		     break;
     default: return E_ERROR;
     }
     dfbdst->Blit(dfbdst,dfbsrc,&rs,dx,dy);
     LOGV("dstsurface=%p srcsurface=%p (%d,%d,%d,%d) to pos(%d,%d)/(%d,%d)",dstsurface,srcsurface,rs.x,rs.y,rs.w,rs.h,ox,oy,dx,dy);
     return ret;
}

INT GFXDestroySurface(HANDLE surface)
{
     LOGV("surface=%p",surface);
     IDirectFBSurface*dfbsurface=(IDirectFBSurface*)surface;
     return dfbsurface->Release(dfbsurface);
}
