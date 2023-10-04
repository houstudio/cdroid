#include <cdtypes.h>
#include <cdgraph.h>
#include <cdlog.h>
#include <linux/fb.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <string.h>
#include <core/eventcodes.h>
#include <cdinput.h>
#include <memory.h>
#include <stdio.h>
#include "mi_common.h"
#include "mi_sys.h"
#include "mi_gfx.h"

typedef struct {
    int fb;
    struct fb_fix_screeninfo fix;
    struct fb_var_screeninfo var;
} FBDEVICE;

typedef struct {
    int dispid;
    UINT width;
    UINT height;
    UINT pitch;
    int format;
    int ishw;
    int current;
    int alpha;
    size_t msize;
    size_t alloced;/*pre allocated memory size*/
    char*buffer;//drawbuffer
    char*orig_buffer;/*used only in double buffer*/
    MI_PHY kbuffer;/*kernel buffer address,MI_PHY unsigned long long*/
} FBSURFACE;

#define DOUBLE_BUFFER 0
#define USE_PREALLOC_SURFACE 1

static FBDEVICE devs[2]= {-1};
static FBSURFACE *primarySurface;
static FBSURFACE devSurfaces[32];
/*if screensize it not macthed the driver size,margins is required*/
static GFXRect screenMargin={0,0,0,0};
static void gfxexit() {
    LOGI("gfxexit");
    MI_GFX_Close();
    MI_SYS_Exit();
}

int GFXInit() {
    int ret;
    FBDEVICE*dev=&devs[0];
    if(dev->fb>=0)return E_OK;
    memset(devs,0,sizeof(devs));
    ret=MI_SYS_Init();
    LOGI("SYS_Init=%d",ret);
    ret=MI_GFX_Open();
    LOGI("MI_GFX_Open=%d",ret);
    devs[0].fb=open("/dev/fb0", O_RDWR);
    // Get fixed screen information
    if(ioctl(dev->fb, FBIOGET_FSCREENINFO, &dev->fix) == -1) {
        LOGE("Error reading fixed information fd=%d",dev->fb);
        return E_ERROR;
    }
    atexit(gfxexit);
    LOGI("fb %d mem.addr=%x fbmem.size=%d pitch=%d",dev->fb,devs[0].fix.smem_start,dev->fix.smem_len,dev->fix.line_length);
    // Get variable screen information
    if(ioctl(dev->fb, FBIOGET_VSCREENINFO, &dev->var) == -1) {
        LOGE("Error reading variable information");
        return E_ERROR;
    }
    devs[0].var.yoffset=0;//set first screen memory for display
    ret=ioctl(dev->fb,FBIOPUT_VSCREENINFO,&dev->var);
    LOGI("FBIOPUT_VSCREENINFO=%d",ret);
    LOGI("fb solution=%dx%d accel_flags=0x%x ScreenMargin=(%d,%d,%d,%d)",dev->var.xres,dev->var.yres,dev->var.accel_flags,
		    screenMargin.x,screenMargin.y,screenMargin.w,screenMargin.h);
    memset(devSurfaces,0,sizeof(devSurfaces));
    MI_PHY memstart = devs[0].fix.smem_start;
    size_t i , offset =0;
    for(i=0;offset<dev->fix.smem_len;i++){
        size_t alloced;
        devSurfaces[i].kbuffer = memstart +offset;
        if(i==0)
            alloced = (dev->var.yres*dev->fix.line_length);
        else
            alloced = (dev->var.yres-screenMargin.y-screenMargin.h)*dev->fix.line_length;
        if(devSurfaces[i].kbuffer+alloced>memstart+dev->fix.smem_len)
            alloced = memstart+dev->fix.smem_len-devSurfaces[i].kbuffer;
        devSurfaces[i].alloced = alloced;
        offset += alloced;
        LOGD("surface[%d] addr=%llx size=%d",i,devSurfaces[i].kbuffer,alloced);
    }
    LOGI("%d surfaces is configured for app usage",i);
    return E_OK;
}

INT GFXGetDisplayCount() {
    return 1;
}

INT GFXGetDisplaySize(int dispid,UINT*width,UINT*height) {
    if(dispid<0||dispid>=GFXGetDisplayCount())
        return E_ERROR;
    LOGI_IF(width==NULL||height==NULL,"Params Error");
    FBDEVICE*dev=&devs[dispid];
    *width =dev->var.xres - screenMargin.x - screenMargin.w;
    *height=dev->var.yres - screenMargin.y - screenMargin.h;
    return E_OK;
}

INT GFXLockSurface(HANDLE surface,void**buffer,UINT*pitch) {
    FBSURFACE*ngs=(FBSURFACE*)surface;
    *buffer=ngs->buffer;
    *pitch=ngs->pitch;
    return 0;
}

INT GFXGetSurfaceInfo(HANDLE surface,UINT*width,UINT*height,INT *format) {
    FBSURFACE*ngs=(FBSURFACE*)surface;
    *width = ngs->width;
    *height= ngs->height;
    *format= ngs->format;
    return E_OK;
}

INT GFXUnlockSurface(HANDLE surface) {
    return E_OK;
}

INT GFXSurfaceSetOpacity(HANDLE surface,BYTE alpha) {
    FBSURFACE*ngs=(FBSURFACE*)surface;
    if(ngs)ngs->alpha=alpha;
    return E_OK;//dispLayer->SetOpacity(dispLayer,alpha);
}

static void toMIGFX(const FBSURFACE*fb,MI_GFX_Surface_t*gfx) {
    gfx->eColorFmt= E_MI_GFX_FMT_ARGB8888;
    gfx->u32Width = fb->width;
    gfx->u32Height= fb->height;
    gfx->u32Stride= fb->pitch;
    gfx->phyAddr  = fb->kbuffer;
}

INT GFXFillRect(HANDLE surface,const GFXRect*rect,UINT color) {
    FBSURFACE*ngs=(FBSURFACE*)surface;
    UINT x,y;
    MI_S32 ret;
    MI_SYS_FrameData_t dt;
    GFXRect rec= {0,0,0,0};
    rec.w=ngs->width;
    rec.h=ngs->height;
    if(rect)rec=*rect;
    if(ngs->kbuffer) {
        MI_U16 fence;
        MI_GFX_Surface_t gfxsurf;
        MI_GFX_Rect_t mirec;
        toMIGFX(ngs,&gfxsurf);
        mirec.s32Xpos = rec.x;
        mirec.s32Ypos = rec.y;
        mirec.u32Width= rec.w;
        mirec.u32Height=rec.h;
        ret=MI_GFX_QuickFill(&gfxsurf,&mirec,color,&fence);
        MI_GFX_WaitAllDone(FALSE,fence);
        LOGV("Fill(%d,%d,%d,%d) with %x",rec.x,rec.y,rec.w,rec.h,color);
    } else {
        UINT*fb=(UINT*)(ngs->buffer+ngs->pitch*rec.y+rec.x*4);
        UINT*fbtop=fb;
        for(x=0; x<rec.w; x++)fb[x]=color;
        const int cpw=rec.w*4;
        long copied=0;
        for(y=1; y<rec.h; y++) {
            fb+=(ngs->pitch>>2);
            memcpy(fb,fbtop,cpw);
            copied+=ngs->pitch;
        }
    }
    LOGV("FillRect %p %d,%d-%d,%d color=0x%x pitch=%d ret=%d",ngs,rec.x,rec.y,rec.w,rec.h,color,ngs->pitch,ret);
    return E_OK;
}

INT GFXFlip(HANDLE surface) {
    FBSURFACE*surf=(FBSURFACE*)surface;
    const size_t screen_size=surf->height*surf->pitch;
    if(surf->ishw && (surf->msize>screen_size) ) {
        FBDEVICE*dev=&devs[surf->dispid];
        LOGI_IF(screen_size!=dev->var.xres * dev->var.yres * dev->var.bits_per_pixel / 8,
                "screensize=%dx%dx%dbpp",dev->var.xres,dev->var.yres,dev->var.bits_per_pixel);
        if(surf->current==0) {
            LOGI_IF(dev->fix.smem_start!=surf->kbuffer,"kbuffer error1");
            MI_SYS_MemcpyPa(surf->kbuffer+screen_size,surf->kbuffer,screen_size);//drawbuffer->screenbuffer
            surf->buffer=surf->orig_buffer+screen_size;
            surf->kbuffer+=screen_size;
            dev->var.yoffset=0;//dev->var.yres;
        } else {
            LOGI_IF(dev->fix.smem_start!=surf->kbuffer-screen_size,"kbuffer error2");
            MI_SYS_MemcpyPa(surf->kbuffer-screen_size,surf->kbuffer,screen_size);//drawbuffer->screenbuffer
            surf->buffer=surf->orig_buffer;
            dev->var.yoffset=dev->var.yres;
            surf->kbuffer-=screen_size;
        }
        surf->current=(surf->current+1)%2;
        int ret=ioctl(dev->fb, FBIOPAN_DISPLAY, &dev->var);
        LOGI_IF(ret<0,"FBIOPAN_DISPLAY=%d yoffset=%d res=%dx%d dev=%p fb=%d",ret,dev->var.yoffset,dev->var.xres,dev->var.yres,dev,dev->fb);
    }
    return 0;
}

static int setfbinfo(FBSURFACE*surf) {
    int rc=-1;
    FBDEVICE*dev=&devs[surf->dispid];
    struct fb_var_screeninfo*v=&dev->var;
    v->xres=surf->width;
    v->yres=surf->height;
    v->xres_virtual=surf->width;
    v->yres_virtual=surf->height;
    v->bits_per_pixel=32;
    switch(surf->format) {
    case GPF_ARGB:
        v->transp.offset=24;
        v->transp.length=8;
        v->red.offset=16;
        v->red.length=8;
        v->green.offset=8;
        v->green.length=8;
        v->blue.offset=0;
        v->blue.length=8;
        break;
    case GPF_ABGR:
        v->transp.offset=24;
        v->transp.length=8;
        v->blue.offset=16;
        v->blue.length=8;
        v->green.offset=8;
        v->green.length=8;
        v->red.offset=0;
        v->red.length=8;
        break;
    default:
        break;
    }
    rc=ioctl(dev->fb,FBIOPUT_VSCREENINFO,v);
    LOGD("FBIOPUT_VSCREENINFO=%d",rc);
    return rc;
}

static FBSURFACE*getFreeFace(){
    for(int i=0;i<sizeof(devSurfaces)/sizeof(FBSURFACE);i++){
        if(devSurfaces[i].kbuffer&&devSurfaces[i].buffer==NULL)
	    return devSurfaces+i; 
    }
    return NULL;
}

INT GFXCreateSurface(int dispid,HANDLE*surface,UINT width,UINT height,INT format,BOOL hwsurface) {
#ifdef USE_PREALLOC_SURFACE
    FBSURFACE*surf=getFreeFace();
#else
    FBSURFACE*surf=(FBSURFACE*)malloc(sizeof(FBSURFACE));
#endif
    FBDEVICE*dev=&devs[dispid];
    surf->dispid=dispid;
    surf->width =hwsurface?dev->var.xres:width;
    surf->height=hwsurface?dev->var.yres:height;
    surf->format=format;
    surf->ishw=hwsurface;
    surf->pitch=surf->width*4;
    surf->kbuffer=NULL;
    surf->orig_buffer=NULL;
    surf->current = 0;
    surf->msize= surf->pitch*surf->height;

    MI_S32 ret=0;
    if(hwsurface) {
#if DOUBLE_BUFFER
        surf->msize*=2;
#endif
#ifndef USE_PREALLOC_SURFACE
        surf->kbuffer = dev->fix.smem_start;
#endif
        ret=MI_SYS_Mmap(surf->kbuffer,surf->msize,(void**)&surf->buffer,FALSE);
        dev->var.yoffset=0;
        LOGI("ioctl offset(0)=%d dev=%p ret=%d",ioctl(dev->fb,FBIOPAN_DISPLAY,&dev->var),dev,ret);
#if DOUBLE_BUFFER
        dev->var.yoffset=1280;
        LOGI("ioctl offset(0)=%d dev=%p",ioctl(dev->fb,FBIOPAN_DISPLAY,&dev->var),dev);
#endif
        primarySurface = surf;
    } else {
#ifdef USE_PREALLOC_SURFACE
        ret=MI_SYS_Mmap(surf->kbuffer,surf->msize,(void**)&surf->buffer,FALSE);
#else	
        ret=MI_SYS_MMA_Alloc("mma_heap_name0",surf->msize,&surf->kbuffer);
        LOGI("surface %p phyaddr=%x ret=%d",surf,surf->kbuffer,ret);
        MI_SYS_Mmap(surf->kbuffer, surf->msize, (void**)&surf->buffer, FALSE);
#endif
    }
    MI_SYS_MemsetPa(surf->kbuffer,0x000000,surf->msize);
    surf->orig_buffer=surf->buffer;
    if(hwsurface)  setfbinfo(surf);
    surf->ishw=hwsurface;
    surf->alpha=255;
    LOGI("Surface=%p buf=%p/%llx size=%dx%d/%d hw=%d\r\n",surf,surf->buffer,surf->kbuffer,surf->width,surf->height,surf->msize,hwsurface);
    *surface=surf;
    return E_OK;
}

#define MIN(x,y) ((x)>(y)?(y):(x))
INT GFXBlit(HANDLE dstsurface,int dx,int dy,HANDLE srcsurface,const GFXRect*srcrect) {
    unsigned int x,y,sw,sh;
    FBSURFACE*ndst=(FBSURFACE*)dstsurface;
    FBSURFACE*nsrc=(FBSURFACE*)srcsurface;
    GFXRect rs= {0,0};
    BYTE*pbs=(BYTE*)nsrc->buffer;
    BYTE*pbd=(BYTE*)ndst->buffer;
    int ret;
    MI_U16 fence;
    MI_GFX_Opt_t opt;
    MI_GFX_Surface_t gfxsrc,gfxdst;
    MI_GFX_Rect_t stSrcRect, stDstRect;

    rs.w = nsrc->width;
    rs.h = nsrc->height;
    if(srcrect)rs=*srcrect;

    LOGD_IF(ndst!=primarySurface&&ndst->ishw,"dst is not primarySurface");
    if(((int)rs.w+dx<=0)||((int)rs.h+dy<=0)||(dx>=(int)ndst->width)||(dy>=(int)ndst->height)||(rs.x<0)||(rs.y<0)){
        LOGD("dx=%d,dy=%d rs=(%d,%d-%d,%d)",dx,dy,rs.x,rs.y,rs.w,rs.h);
        return E_INVALID_PARA;
    }

    LOGV("Blit %p(%d,%d,%d,%d)->%p(%d,%d,%d,%d)",nsrc,rs.x,rs.y,rs.w,rs.h,ndst,dx,dy,rs.w,rs.h);
    if(dx<0){ rs.x -= dx; rs.w = (int)rs.w + dx; dx = 0;}
    if(dy<0){ rs.y -= dy; rs.h = (int)rs.h + dy; dy = 0;}
    if(dx + rs.w > ndst->width - screenMargin.x - screenMargin.w)
	 rs.w = ndst->width - screenMargin.x - screenMargin.w - dx;
    if(dy + rs.h > ndst->height - screenMargin.y- screenMargin.h)
	 rs.h = ndst->height - screenMargin.y - screenMargin.h - dy;

    toMIGFX(nsrc,&gfxsrc);
    toMIGFX(ndst,&gfxdst);
    bzero(&opt,sizeof(opt));

    opt.u32GlobalSrcConstColor = 0xFF000000;//nsrc->alpha<<24;
    opt.u32GlobalDstConstColor = 0xFF000000;
    opt.eSrcDfbBldOp = E_MI_GFX_DFB_BLD_ONE;
    opt.eDstDfbBldOp = E_MI_GFX_DFB_BLD_ZERO;
    opt.eDFBBlendFlag= E_MI_GFX_DFB_BLEND_NOFX;
    opt.stClipRect.s32Xpos = dx + screenMargin.x*(ndst->ishw?1:0);
    opt.stClipRect.s32Ypos = dy + screenMargin.y*(ndst->ishw?1:0);
    opt.stClipRect.u32Width= rs.w;//rs.w;//ndst->width;
    opt.stClipRect.u32Height=rs.h;//rs.h;//ndst->height;
    //if(nsrc->alpha!=255)
    //    opt.eDFBBlendFlag = E_MI_GFX_DFB_BLEND_SRC_PREMULTCOLOR;

    opt.eMirror = E_MI_GFX_MIRROR_NONE;
    opt.eRotate = E_MI_GFX_ROTATE_0;

    stSrcRect.s32Xpos = rs.x;
    stSrcRect.s32Ypos = rs.y;
    stSrcRect.u32Width = rs.w;
    stSrcRect.u32Height= rs.h;

    LOGV("Blit %p(%d,%d-%d,%d)-> (%d,%d)copied",nsrc,rs.x,rs.y,rs.w,rs.h,dx,dy);
    
    stDstRect.s32Xpos = dx + screenMargin.x*(ndst->ishw?1:0);//opt.stClipRect.s32Xpos;
    stDstRect.s32Ypos = dy + screenMargin.y*(ndst->ishw?1:0);//opt.stClipRect.s32Ypos;
    stDstRect.u32Width = rs.w;
    stDstRect.u32Height= rs.h;
    ret = MI_GFX_BitBlit(&gfxsrc,&stSrcRect,&gfxdst, &stDstRect,&opt,&fence);
    LOGV("*Blit %p(%d,%d,%d,%d)->%p(%d,%d,%d,%d) gfx.src=%dx%dx%d@%llx gfx.dst=%dx%dx%d@%llx ret=%d",
	nsrc,stSrcRect.s32Xpos,stSrcRect.s32Ypos, stSrcRect.u32Width, stSrcRect.u32Height,
	ndst,stDstRect.s32Xpos,stDstRect.s32Ypos, stDstRect.u32Width,stDstRect.u32Height,
	nsrc->width,nsrc->height,nsrc->pitch,nsrc->kbuffer, ndst->width,ndst->height,ndst->pitch,ndst->kbuffer,ret);
    MI_GFX_WaitAllDone(FALSE,fence);
    return 0;
}

INT GFXBatchBlit(HANDLE dstsurface,const GFXPoint*dest_point,HANDLE srcsurface,const GFXRect*srcrects) {
#if 0
#endif
    return 0;
}

INT GFXDestroySurface(HANDLE surface) {
    FBSURFACE*surf=(FBSURFACE*)surface;
    LOGI("GFXDestroySurface %p/%p",surf,surf->buffer);
    if(surf->kbuffer) {
        MI_SYS_Munmap(surf->buffer,surf->msize);
#ifndef USE_PREALLOC_SURFACE
        MI_SYS_MMA_Free((MI_PHY)surf->kbuffer);
#endif
        surf->buffer = NULL;
    } else if(surf->buffer) {
#ifndef USE_PREALLOC_SURFACE
        free(surf->buffer);
#endif
    }
    free(surf);
    return 0;
}
