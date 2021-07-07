#include <cdtypes.h>
#include <ngl_os.h>
#include <cdgraph.h>
#include <cdlog.h>
#include <directfb.h>
#include <directfb_util.h>
#include <direct/messages.h>
#include <aui_input.h>
#include <aui_osd.h>
#define MAX_SURFACE_COUNT 16

NGL_MODULE(GRAPH);

static IDirectFB *directfb=NULL;
static IDirectFBSurface*primarySurface;
//AUI porting has some crash roblem,but work's fine  
#define USE_DIRECTFB 1 /*directfb is 10x faster than aui_gfx*/

static int created_surface=0;
static int destroyed_surface=0;
static aui_hdl g_hw_layer;

static void DFBDisplayLayerCBK(DFBDisplayLayerID layer_id, DFBDisplayLayerDescription  desc, void *callbackdata){
    LOGD("Layer %d[%s] type:%x surface.caps=%x accessor=%x",layer_id,desc.name,desc.type,desc.surface_caps,desc.surface_accessor);
}

DWORD GFXInit()
{
#ifdef USE_DIRECTFB
    if(directfb!=NULL)return E_OK;
    DirectFBInit (NULL,NULL);
    DirectFBCreate (&directfb);
    directfb->EnumDisplayLayers(directfb,DFBDisplayLayerCBK,NULL);
    directfb->SetCooperativeLevel (directfb, DFSCL_FULLSCREEN);
    
    DFBSurfaceDescription   desc;
    memset(&desc,0,sizeof(DFBSurfaceDescription));
    desc.flags=DSDESC_CAPS;
    desc.caps=DSCAPS_PRIMARY;
    directfb->CreateSurface( directfb, &desc,&primarySurface);

#endif
    static int inited=0;
    if(0==inited){
        aui_gfx_init(NULL, NULL);
        inited++;
    } 
    return E_OK;
}

static void Rect2Aui(GFXRect*r,struct aui_osd_rect*ar){
    ar->uLeft=r->x;
    ar->uTop=r->y;
    ar->uWidth=r->w;
    ar->uHeight=r->h;
}

DWORD GFXGetScreenSize(UINT*width,UINT*height){
#ifdef USE_DIRECTFB
    IDirectFBDisplayLayer *dispLayer;
    DFBDisplayLayerConfig dispCfg;
    GFXInit();
    directfb->GetDisplayLayer( directfb, DLID_PRIMARY, &dispLayer );
    dispLayer->GetConfiguration( dispLayer, &dispCfg );
    *width=dispCfg.width;
    *height=dispCfg.height;
#else
    *width=1280;
    *height=720;
#endif
    return E_OK;
}

DWORD GFXLockSurface(HANDLE surface,void**buffer,UINT*pitch){
#ifdef USE_DIRECTFB
    IDirectFBSurface*surf=(IDirectFBSurface*)surface;
    int ret=surf->Lock(surf,DSLF_READ | DSLF_WRITE,buffer,pitch);
    LOGV_IF(ret,"surface=%p buffer=%p pitch=%d",surf,buffer,*pitch);
#else
    aui_surface_info info;
    int ret=aui_gfx_surface_lock((aui_hdl)surface);
    aui_gfx_surface_info_get((aui_hdl)surface,&info);
    *buffer=info.p_surface_buf;
    *pitch=info.pitch;
    LOGV_IF(ret,"surface=%p buffer=%p width=%d pitch=%d",surface,buffer,info.width,*pitch); 
#endif
    return ret;
}

DWORD GFXGetSurfaceInfo(HANDLE surface,UINT*width,UINT*height,INT *format)
{
#ifdef USE_DIRECTFB
    IDirectFBSurface*surf=(IDirectFBSurface*)surface;
    if(NULL==width||NULL==height)
       return E_INVALID_PARA;
    int ret=surf->GetSize(surf,width,height);
    if(format)*format=GPF_ARGB;
#else
    aui_surface_info info;
    int ret=aui_gfx_surface_info_get((aui_hdl)surface,&info);
    *width=info.width;//p_surface_buf;
    *height=info.height;
    if(format)*format=GPF_ARGB;
#endif
    LOGV("surface=%p,format=%d ret=%d",(surface?surface:0),(format?*format:0),ret);
    return ret==0?E_OK:E_ERROR;
}

DWORD GFXUnlockSurface(HANDLE surface){
#ifdef USE_DIRECTFB
    IDirectFBSurface*surf=(IDirectFBSurface*)surface;
    int ret=surf->Unlock(surf);
#else
    int ret=aui_gfx_surface_unlock((aui_hdl)surface);
#endif
    LOGV_IF(ret,"surface=%p ret=%d",surface,ret);
    return ret;
}

DWORD GFXSurfaceSetOpacity(HANDLE surface,BYTE alpha){
    LOGD("==setopacity=%x",alpha);
#ifdef USE_DIRECTFB
    IDirectFBSurface*surf=(IDirectFBSurface*)surface;
    IDirectFBDisplayLayer *dispLayer;
    directfb->GetDisplayLayer( directfb, DLID_PRIMARY, &dispLayer );
    if ((g_hw_layer==NULL) && aui_find_dev_by_idx(AUI_MODULE_GFX, AUI_OSD_LAYER_GMA0, &g_hw_layer))
        aui_gfx_layer_open(AUI_OSD_LAYER_GMA0, (aui_hdl*)(&g_hw_layer));
    if(g_hw_layer!=NULL)
       aui_gfx_layer_alpha_set(g_hw_layer,alpha);
    LOGE_IF(NULL==g_hw_layer,"g_hw_layer is null,global alpha setting failed");
    return dispLayer->SetOpacity(dispLayer,alpha);
#else
    return aui_gfx_layer_alpha_set(g_hw_layer,alpha);
#endif
}

DWORD GFXFillRect(HANDLE surface,const GFXRect*rec,UINT color){
#ifdef USE_DIRECTFB
    IDirectFBSurface*surf=(IDirectFBSurface*)surface;
    GFXRect r={0,0,0,0};
    if(NULL==rec)
        surf->GetSize(surf,&r.w,&r.h);
    else
        r=*rec;
    surf->SetColor(surf,(color>>24),(color>>16),(color>>8),color);
    surf->FillRectangle(surf,r.x,r.y,r.w,r.h);
#else
    aui_osd_rect rc={0,0,0,0};
    UINT w,h,f;
    nglGetSurfaceInfo(surface,&w,&h,&f);
    if(rec==NULL){
        rc.uWidth=w;rc.uHeight=h;
    }else Rect2Aui(rec,&rc);
    aui_gfx_surface_fill(surface,color,&rc);
    LOGD_IF((rc.uLeft+rc.uWidth>w)||(rc.uTop+rc.uHeight>h),"Filrect range error");
#endif
    return E_OK;
}

DWORD GFXFlip(HANDLE surface){
#ifdef USE_DIRECTFB
    IDirectFBSurface*surf=(IDirectFBSurface*)surface;
    int ret=surf->Flip( surf, NULL, DSFLIP_NONE);//ONSYNC);
#else
    aui_osd_rect rc={0,0,0,0};
    UINT w,h,f;
    aui_surface_info info;
    int ret=aui_gfx_surface_info_get((aui_hdl)surface,&info);
    rc.uWidth=info.width;rc.uHeight=info.height;

    if(info.is_hw_surface)
    ret=aui_gfx_surface_flush(surface,&rc); 
    LOGV_IF(ret,"flip %x=%d ishw=%d size=%dx%d ret=%d",surface,ret,info.is_hw_surface,rc.uWidth,rc.uHeight,ret);
#endif
    return ret;
}

DWORD GFXCreateSurface(HANDLE*surface,UINT width,UINT height,INT format,BOOL hwsurface)
{
#ifdef USE_DIRECTFB
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
     LOGD_IF(ret,"surface=%x  ishw=%d surfaces=%d/%d",dfbsurface,hwsurface,created_surface,destroyed_surface);
     if(!hwsurface)dfbsurface->MakeClient(dfbsurface);
     created_surface++;
     dfbsurface->Clear(dfbsurface,0,0,0,0);
     *surface=(HANDLE)dfbsurface;
     return E_OK;
#else
     int ret;
     aui_hdl surf_handle;
     aui_osd_pixel_format pixel_format = AUI_OSD_HD_ARGB8888;
     struct aui_osd_rect region_rect={0,0,1280,720};
     if(hwsurface){
         unsigned long layer_id = AUI_OSD_LAYER_GMA0;
         if (aui_find_dev_by_idx(AUI_MODULE_GFX, layer_id, &g_hw_layer)) 
	     ret = aui_gfx_layer_open(layer_id, (aui_hdl*)(&g_hw_layer));
         aui_gfx_layer_show_on_off(g_hw_layer, 1);
         ret=aui_gfx_hw_surface_create(g_hw_layer, pixel_format, &region_rect,&surf_handle, 1);
         aui_gfx_surface_fill(surf_handle,0, &region_rect);
     }else{
         ret=aui_gfx_sw_surface_create(pixel_format,width,height,&surf_handle);
     }
     region_rect.uWidth=width; region_rect.uHeight=height;
     aui_gfx_surface_clip_rect_set(surf_handle,&region_rect,AUI_GE_CLIP_INSIDE);
     aui_gfx_surface_fill(surf_handle,0x00,&region_rect);
     //aui_gfx_surface_fill(surf_handle,0xFF000000,&region_rect);
     *surface=(HANDLE)surf_handle;
     aui_gfx_layer_antifliker_on_off(surf_handle,0);
     LOGE_IF(ret,"surface=%x  ishw=%d ret=%d",surf_handle,hwsurface,ret);
     return E_OK;
#endif
}

DWORD GFXSetSurfaceColorKey(HANDLE surface,UINT color){
#ifdef USE_DIRECTFB
      IDirectFBSurface*dfbsrc=(IDirectFBSurface*)surface;
#else
     
#endif
}

#define MIN(x,y) ((x)>(y)?(y):(x))
DWORD GFXBlit(HANDLE dstsurface,int dx,int dy,HANDLE srcsurface,const GFXRect*srcrect)
{
#ifdef USE_DIRECTFB
     int ret,dw,dh;
     GFXRect rs={0,0};
     IDirectFBSurface*dfbsrc=(IDirectFBSurface*)srcsurface;
     IDirectFBSurface*dfbdst=(IDirectFBSurface*)dstsurface;

     dfbdst=primarySurface;
     dfbdst->GetSize(dfbdst,&dw,&dh);
     dfbsrc->GetSize(dfbsrc,(int*)&rs.w,(int*)&rs.h);

     if(srcrect)rs=*srcrect;

     if(((int)rs.w+dx<=0)||((int)rs.h+dy<=0)||(dx>=dw)||(dy>=dh)||(rs.x<0)||(rs.y<0)){
         return E_INVALID_PARA;
     }
     if(dx<0){rs.x-=dx;rs.w=(int)rs.w+dx; dx=0;}
     if(dy<0){rs.y-=dy;rs.h=(int)rs.h+dy;dy=0;}
     if(dx+rs.w>dw)rs.w=dw-dx;
     if(dy+rs.h>dh)rs.h=dh-dy;
     dfbdst->SetBlittingFlags(dfbdst,DSBLIT_NOFX);
     dfbdst->SetPorterDuff(dfbdst,DSPD_SRC_OVER);
     ret=dfbdst->Blit(dfbdst,dfbsrc,&rs,dx,dy);
     LOGD_IF(ret,"dstsurface=%p srcsurface=%p ret=%d",dstsurface,srcsurface,ret);
     return ret;
#else
     aui_blit_operation blit_op;
     aui_blit_rect blit_rect;
     int dw,dh,fmt;
     GFXRect rs={0,0};
     nglGetSurfaceInfo(srcsurface,&rs.w,&rs.h,&fmt);
     nglGetSurfaceInfo(dstsurface,&dw,&dh,&fmt);
     if(srcrect)rs=*srcrect;
     if(((int)rs.w+dx<=0)||((int)rs.h+dy<=0)||(dx>=dw)||(dy>=dh)||(rs.x<0)||(rs.y<0)){
         return E_INVALID_PARA;
     }
     if(dx<0){rs.x-=dx;rs.w=(int)rs.w+dx; dx=0;}
     if(dy<0){rs.y-=dy;rs.h=(int)rs.h+dy;dy=0;}
     if(dx+rs.w>dw)rs.w=dw-dx;
     if(dy+rs.h>dh)rs.h=dh-dy;

     memset(&blit_rect, 0, sizeof(aui_blit_rect));
     memset(&blit_op, 0, sizeof(aui_blit_operation));

     struct aui_osd_rect*dr=&blit_rect.dst_rect;
     struct aui_osd_rect*sr=&blit_rect.fg_rect;

     sr->uLeft=rs.x;
     sr->uTop=rs.y;
     sr->uWidth= rs.w;
     sr->uHeight=rs.h;

     dr->uLeft=dx;      
     dr->uTop=dy;
     dr->uWidth=sr->uWidth;
     dr->uHeight=sr->uHeight;
    
     blit_op.rop_operation = AUI_GFX_ROP_DERECT_COVER;
     int ret=aui_gfx_surface_blit(dstsurface,srcsurface,NULL,&blit_op,&blit_rect);
     LOGV("blit=%d dstsurface=%x  srcsurface=%x dst=%d,%d-%d,%d src=%d,%d,%d,%d",
             ret,dstsurface,srcsurface,dr->uLeft,dr->uTop,dr->uWidth,dr->uHeight,
             sr->uLeft,sr->uTop,sr->uWidth,sr->uHeight);
#endif
     return ret;
}

DWORD GFXDestroySurface(HANDLE surface)
{
#ifdef USE_DIRECTFB
     destroyed_surface++;
     LOGV("surface=%p  created=%d destroyed=%d",surface,created_surface,destroyed_surface);
     IDirectFBSurface*dfbsurface=(IDirectFBSurface*)surface;
     return dfbsurface->Release(dfbsurface);
#else
     aui_gfx_surface_delete((aui_hdl)surface);
     return E_OK;     
#endif
}
