#include <cdtypes.h>
#include <cdgraph.h>
#include <cdlog.h>
#include <linux/fb.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/mman.h>
#ifdef ENABLE_RFB
#include <rfb/rfb.h>
#include <rfb/keysym.h>
#include <rfb/rfbproto.h>
#endif
#define USE_DOUBLE_BUFFER 0/*do not open doublebuffer,it is not necessary for the gui core */

#ifdef HAVE_FY_TDE2 /*for small screen ,it is no necessary ,memcpy is faster for smallscreen*/
#include <mpi_tde.h>
#include <vd/VDecApi.h>
#include <vd/VoApi.h>
#endif

typedef struct{
    int fb;
    struct fb_fix_screeninfo fix;
    struct fb_var_screeninfo var;
#if ENABLE_RFB
    rfbScreenInfoPtr rfbScreen;
#endif
}FBDEVICE;

typedef struct{
   uint32_t width;
   uint32_t height;
   uint32_t pitch;
   int format;
   int ishw;
   void*current;
   void*buffer;
   void*bkbuffer;/*kernel buffer address*/
   void*phybuffer;
   void*phybkbuffer;
}FBSURFACE;

static FBDEVICE dev={-1};

int32_t GFXInit(){
    if(dev.fb>=0)return E_OK;
#ifdef HAVE_FY_TDE2
    VO_Enable();
    LOGI("FY_TDE2_Open=%d",FY_TDE2_Open());
#endif
    dev.fb=open("/dev/fb0", O_RDWR);
    // Get fixed screen information
    if(ioctl(dev.fb, FBIOGET_FSCREENINFO, &dev.fix) == -1) {
        LOGE("Error reading fixed information fd=%d",dev.fb);
        return E_ERROR;
    }
    LOGI("NX5.fbmem.addr=%x fbmem.size=%d pitch=%d",dev.fix.smem_start,dev.fix.smem_len,dev.fix.line_length);

    // Get variable screen information
    if(ioctl(dev.fb, FBIOGET_VSCREENINFO, &dev.var) == -1) {
        LOGE("Error reading variable information");
        return E_ERROR;
    }

    /************************************/
    unsigned int fb_mem_offset = (((long)dev.fix.smem_start) - (((long)dev.fix.smem_start) & ~(getpagesize() - 1)));
    int fb_mem_len = dev.fix.smem_len + fb_mem_offset;
    unsigned char*fb_vir_addr = (unsigned char *)mmap(0,fb_mem_len,PROT_READ|PROT_WRITE,MAP_SHARED,dev.fb, 0);
    if ((long)fb_vir_addr == -1){
        LOGE("Error: failed to map framebuffer device to memory");
        return E_ERROR;
    }
    /* 初始化，清空framebuffer */
    memset(fb_vir_addr, 0x00,fb_mem_len);
/************************************/
	//set first screen memory for display
    dev.var.yoffset=0;
    LOGI("FBIOPUT_VSCREENINFO=%d",ioctl(dev.fb,FBIOPUT_VSCREENINFO,&dev.var));
    LOGI("fb solution=%dx%d accel_flags=0x%x\r\n",dev.var.xres,dev.var.yres,dev.var.accel_flags);

#if ENABLE_RFB
    rfbScreenInfoPtr rfbScreen = rfbGetScreen(NULL,NULL,800,600,8,3,3);
    setupRFB(rfbScreen,"X5-RFB",0);
    dev.rfbScreen=rfbScreen;
#endif
    return E_OK;
}

#ifdef HAVE_FY_TDE2
void copySurface(void*phyfrom,void*phyto){
    TDE2_RECT_S src_rect; 
    memset(&src_rect, 0, sizeof(TDE2_RECT_S));
    src_rect.s32Xpos    = 0;
    src_rect.s32Ypos    = 0;
    src_rect.u32Width   = dev.var.xres;
    src_rect.u32Height  = dev.var.yres;

    TDE2_SURFACE_S src_surface; 
    memset(&src_surface, 0, sizeof(TDE2_SURFACE_S));
    src_surface.enColorFmt      = TDE2_COLOR_FMT_ARGB8888;
    src_surface.u32Width        = dev.var.xres;
    src_surface.u32Height       = dev.var.yres;
    src_surface.u32Stride       = dev.fix.line_length;
    src_surface.bAlphaMax255    = FY_TRUE;

    TDE2_SURFACE_S dst_surface; 
    memset(&dst_surface, 0, sizeof(TDE2_SURFACE_S));
    dst_surface.enColorFmt      = TDE2_COLOR_FMT_ARGB8888;
    dst_surface.u32Width        = dev.var.xres;
    dst_surface.u32Height       = dev.var.yres;
    dst_surface.u32Stride       = dev.fix.line_length;
    dst_surface.bAlphaMax255    = FY_TRUE;

    TDE2_RECT_S dst_rect; 
    memset(&dst_rect, 0, sizeof(TDE2_RECT_S));
    dst_rect.s32Xpos    = 0;
    dst_rect.s32Ypos    = 0;
    dst_rect.u32Width   = dev.var.xres;
    dst_rect.u32Height  = dev.var.yres;

    // 获取虚拟地址对应的物理地址
    src_surface.u32PhyAddr = (uint32_t)phyfrom;//surf->current==surf->buffer?surf->phybkbuffer:surf->phybuffer;//src_phy;
    dst_surface.u32PhyAddr = (uint32_t)phyto;//surf->current==surf->buffer?surf->phybuffer:surf->phybkbuffer;//dst_phy;

    TDE_HANDLE tde_handle= FY_TDE2_BeginJob();
    if (FY_TDE2_QuickCopy(tde_handle, &src_surface, &src_rect, &dst_surface, &dst_rect)){
        LOGE("gpu copy error\n");
    }
    FY_TDE2_EndJob(tde_handle, FY_TRUE, FY_TRUE, 0);
}

static void swapBuffer(FBSURFACE*surf){
    if(surf->bkbuffer==NULL)
        return;
    if(surf->current==surf->buffer){
	    copySurface(surf->phybuffer,surf->phybkbuffer);
	}else{
	    copySurface(surf->phybkbuffer,surf->phybuffer);
	}
}

void FastFillRect(FBSURFACE*surf,const GFXRect*rect,uint32_t color){
    TDE2_SURFACE_S dst_surface; 
    memset(&dst_surface, 0, sizeof(TDE2_SURFACE_S));
    dst_surface.enColorFmt      = TDE2_COLOR_FMT_ARGB8888;
    dst_surface.u32Width        = surf->width;
    dst_surface.u32Height       = surf->height;
    dst_surface.u32Stride       = surf->pitch;
    dst_surface.bAlphaMax255    = FY_TRUE;
    if(surf->bkbuffer==NULL){
        dst_surface.u32PhyAddr = (uint32_t)((surf->current==surf->buffer)?surf->phybuffer:surf->phybkbuffer);
    }
    TDE_HANDLE tde_handle=FY_TDE2_BeginJob();

    if (FY_TDE2_QuickFill(tde_handle, &dst_surface, (TDE2_RECT_S*)&rect, color)){
        LOGD("gpu fill error\n");
    }
    FY_TDE2_EndJob(tde_handle, FY_TRUE, FY_TRUE, 0);
}
#endif

int32_t GFXGetScreenSize(uint32_t*width,uint32_t*height){
    *width=dev.var.xres;
    *height=dev.var.yres;
    LOGD("screensize=%dx%d",*width,*height);
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

int32_t GFXLockSurface(HANDLE surface,void**buffer,uint32_t*pitch){
    FBSURFACE*ngs=(FBSURFACE*)surface;
    *buffer=ngs->buffer;
    *pitch=ngs->pitch;
    return 0;
}

int32_t GFXGetSurfaceInfo(HANDLE surface,uint32_t*width,uint32_t*height,INT *format){
    FBSURFACE*ngs=(FBSURFACE*)surface;
    *width = ngs->width;
    *height= ngs->height;
    *format= ngs->format;
    return E_OK;
}

int32_t GFXUnlockSurface(HANDLE surface){
    return 0;
}

int32_t GFXSurfaceSetOpacity(HANDLE surface,uint8_t alpha){
    return 0;//dispLayer->SetOpacity(dispLayer,alpha);
}

int32_t GFXFillRect(HANDLE surface,const GFXRect*rect,uint32_t color){
    FBSURFACE*ngs=(FBSURFACE*)surface;
    uint32_t x,y;
    GFXRect rec={0,0,0,0};
    rec.w=ngs->width;
    rec.h=ngs->height;
    if(rect)rec=*rect;
    LOGV("FillRect %p %d,%d-%d,%d color=0x%x pitch=%d",ngs,rec.x,rec.y,rec.w,rec.h,color,ngs->pitch);
    uint32_t*fb=(uint32_t*)(ngs->buffer+ngs->pitch*rec.y+rec.x*4);
    uint32_t *fbtop=fb; 
#ifdef HAVE_FY_TDE2
    if(ngs->ishw){
        FastFillRect(ngs,rect,color);
        return 0;
    }
#endif
    for(x=0;x<rec.w;x++)
        fb[x]=color;
    for(y=1;y<rec.h;y++){
        fb+=(ngs->pitch>>2);
        memcpy(fb,fbtop,rec.w*4);
    }
    return E_OK;
}

int32_t GFXFlip(HANDLE surface){
    FBSURFACE*surf=(FBSURFACE*)surface;
    if(surf->ishw){
#ifdef HAVE_FY_TDE2
       //swapBuffer(surf);
#endif
       dev.var.yoffset=0;
       int ret=ioctl(dev.fb,FBIOPAN_DISPLAY,&dev.var);
       LOGD_IF(ret,"FBIOPAN_DISPLAY error %d",ret);
#if ENABLE_RFB
       rfbMarkRectAsModified(dev.rfbScreen,0,0,surf->width,surf->height);
#endif
    }
    return 0;
}

static int setfbinfo(FBSURFACE*surf){
    int rc=-1;
    struct fb_var_screeninfo*v=&dev.var;
    v->xres=surf->width;
    v->yres=surf->height;
    v->xres_virtual=surf->width;
    v->yres_virtual=surf->height;
    v->bits_per_pixel=32;
    switch(surf->format){
    case GPF_ARGB:
	 v->transp.offset=24; v->transp.length=8;
	 v->red.offset=16; v->red.length=8;
	 v->green.offset=8; v->green.length=8;
	 v->blue.offset=0; v->blue.length=8;
	 break;
    case GPF_ABGR:
	 v->transp.offset=24; v->transp.length=8;
	 v->blue.offset=16; v->blue.length=8;
	 v->green.offset=8; v->green.length=8;
	 v->red.offset=0; v->red.length=8;
	 break;
    default:break; 
    }
    rc=ioctl(dev.fb,FBIOPUT_VSCREENINFO,v);
    LOGD("FBIOPUT_VSCREENINFO=%d",rc);
    return rc;
}

#ifdef ENABLE_RFB
static void ResetScreenFormat(FBSURFACE*fb,int width,int height,int format){
    rfbPixelFormat*fmt=&dev.rfbScreen->serverFormat;
    fmt->trueColour=TRUE;
    switch(format){
    case GPF_ARGB:
         fmt->bitsPerPixel=24;
         fmt->redShift=16;//bitsPerSample*2
         fmt->greenShift=8;
         fmt->blueShift=0;
         break;
    case GPF_ABGR:
         fmt->bitsPerPixel=32;
         fmt->redShift=0;
         fmt->greenShift=8;
         fmt->blueShift=16;
         break;
    default:return;
    }
    rfbNewFramebuffer(dev.rfbScreen,fb->buffer,width,height,8,3,4);
    dev.rfbScreen->paddedWidthInBytes=fb->pitch;
    LOGV("format=%d %dx%d:%d",format,width,height,fb->pitch);
}
#endif

int32_t GFXCreateSurface(HANDLE*surface,uint32_t width,uint32_t height,INT format,BOOL hwsurface){
    FBSURFACE*surf=(FBSURFACE*)malloc(sizeof(FBSURFACE));
    bzero(surf,sizeof(FBSURFACE));
    surf->width=width;
    surf->height=height;
    surf->format=format;
    surf->ishw=hwsurface;
    surf->pitch=width*4;
    if(hwsurface){
        const size_t fb_size=dev.fix.line_length*height;
        size_t mem_len=((dev.fix.smem_start) -((dev.fix.smem_start) & ~(getpagesize() - 1)));
        setfbinfo(surf);
        surf->buffer=mmap( NULL,dev.fix.smem_len,PROT_READ | PROT_WRITE, MAP_SHARED,dev.fb, 0 );
        surf->pitch=dev.fix.line_length;
        surf->phybuffer=dev.fix.smem_start;
        memset(surf->buffer,0,dev.fix.smem_len);
#ifdef USE_DOUBLE_BUFFER
        if(dev.fix.smem_len>=fb_size*2){
            surf->bkbuffer =surf->buffer+fb_size;
            surf->phybkbuffer=surf->phybuffer+fb_size;
        }
#endif
#ifdef ENABLE_RFB
        dev.rfbScreen->frameBuffer = surf->buffer;
        ResetScreenFormat(surf,width,height,format);
#endif
    }else{
        surf->buffer=malloc(width*surf->pitch);
    }
    surf->ishw=hwsurface;
    LOGV("surface=%x buf=%p size=%dx%d hw=%d",surf,surf->buffer,width,height,hwsurface);
    *surface=surf;
    return E_OK;
}

int32_t GFXBlit(HANDLE dstsurface,int dx,int dy,HANDLE srcsurface,const GFXRect*srcrect){
    unsigned int x,y,sw,sh;
    FBSURFACE*ndst=(FBSURFACE*)dstsurface;
    FBSURFACE*nsrc=(FBSURFACE*)srcsurface;
    GFXRect rs={0,0};
    uint8_t*pbs=(uint8_t*)nsrc->buffer;
    uint8_t*pbd=(uint8_t*)ndst->buffer;
    rs.w=nsrc->width;rs.h=nsrc->height;
    if(srcrect)rs=*srcrect;
    if(((int)rs.w+dx<=0)||((int)rs.h+dy<=0)||(dx>=(int)ndst->width)||(dy>=(int)ndst->height)||(rs.x<0)||(rs.y<0)){
        LOGV("dx=%d,dy=%d rs=(%d,%d-%d,%d)",dx,dy,rs.x,rs.y,rs.w,rs.h);
        return E_INVALID_PARA;
    }

    LOGV("Blit %p[%dx%d] %d,%d-%d,%d -> %p[%dx%d] %d,%d",nsrc,nsrc->width,nsrc->height,
         rs.x,rs.y,rs.w,rs.h,ndst,ndst->width,ndst->height,dx,dy);
    if(dx<0){rs.x-=dx;rs.w=(int)rs.w+dx; dx=0;}
    if(dy<0){rs.y-=dy;rs.h=(int)rs.h+dy;dy=0;}
    if(dx+rs.w>ndst->width)rs.w=ndst->width-dx;
    if(dy+rs.h>ndst->height)rs.h=ndst->height-dy;

    LOGV("Blit %p %d,%d-%d,%d -> %p %d,%d buffer=%p->%p",nsrc,rs.x,rs.y,rs.w,rs.h,ndst,dx,dy,pbs,pbd);
    pbs+=rs.y*nsrc->pitch+rs.x*4;
    pbd+=dy*ndst->pitch+dx*4;
    const int cpw=rs.w<<2;
    for(y=0;y<rs.h;y++){
        memcpy(pbd,pbs,cpw);
        pbs+=nsrc->pitch;
        pbd+=ndst->pitch;
    }
    if(ndst->ishw)GFXFlip(dstsurface);
    return 0;
}

int32_t GFXDestroySurface(HANDLE surface){
    FBSURFACE*surf=(FBSURFACE*)surface;
    if(surf->ishw)
        munmap(surf->buffer,dev.fix.smem_len);
    else if(surf->buffer)free(surf->buffer);
    free(surf);
    return 0;
}
