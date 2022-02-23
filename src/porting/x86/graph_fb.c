#include <cdtypes.h>
#include <cdgraph.h>
#include <cdlog.h>
#include <linux/fb.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <core/eventcodes.h>
#include <cdinput.h>
#ifdef ENABLE_RFB
#include <rfb/rfb.h>
#include <rfb/keysym.h>
#include <rfb/rfbproto.h>
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
   UINT width;
   UINT height;
   UINT pitch;
   int format;
   int ishw;
   void*buffer;
   void*bkbuffer;/*kernel buffer address*/
}FBSURFACE;

static FBDEVICE dev={-1};


DWORD GFXInit(){
    if(dev.fb>=0)return E_OK;
    dev.fb=open("/dev/fb0", O_RDWR);
     // Get fixed screen information
    if(ioctl(dev.fb, FBIOGET_FSCREENINFO, &dev.fix) == -1) {
        LOGE("Error reading fixed information fd=%d",dev.fb);
        return E_ERROR;
    }
    LOGI("fbmem.addr=%x fbmem.size=%d pitch=%d",dev.fix.smem_start,dev.fix.smem_len,dev.fix.line_length);

    // Get variable screen information
    if(ioctl(dev.fb, FBIOGET_VSCREENINFO, &dev.var) == -1) {
        LOGE("Error reading variable information");
        return E_ERROR;
    }

    dev.var.yoffset=0;//set first screen memory for display
    LOGI("FBIOPUT_VSCREENINFO=%d",ioctl(dev.fb,FBIOPUT_VSCREENINFO,&dev.var));
    LOGI("fb solution=%dx%d accel_flags=0x%x\r\n",dev.var.xres,dev.var.yres,dev.var.accel_flags);
#if ENABLE_RFB
    rfbScreenInfoPtr rfbScreen = rfbGetScreen(NULL,NULL,800,600,8,3,3);
    setupRFB(rfbScreen,"X5RFB",0);
    dev.rfbScreen=rfbScreen;
#endif
    return E_OK;
}

DWORD GFXGetScreenSize(UINT*width,UINT*height){
    *width=dev.var.xres;
    *height=dev.var.yres;
    LOGD("screensize=%dx%d",*width,*height);
    return E_OK;
}

DWORD GFXLockSurface(HANDLE surface,void**buffer,UINT*pitch){
    FBSURFACE*ngs=(FBSURFACE*)surface;
    *buffer=ngs->buffer;
    *pitch=ngs->pitch;
    return 0;
}

DWORD GFXGetSurfaceInfo(HANDLE surface,UINT*width,UINT*height,INT *format){
    FBSURFACE*ngs=(FBSURFACE*)surface;
    *width = ngs->width;
    *height= ngs->height;
    *format= ngs->format;
    return E_OK;
}

DWORD GFXUnlockSurface(HANDLE surface){
    return 0;
}

DWORD GFXSurfaceSetOpacity(HANDLE surface,BYTE alpha){
    return 0;//dispLayer->SetOpacity(dispLayer,alpha);
}

DWORD GFXFillRect(HANDLE surface,const GFXRect*rect,UINT color){
    FBSURFACE*ngs=(FBSURFACE*)surface;
    UINT x,y;
    GFXRect rec={0,0,0,0};
    rec.w=ngs->width;
    rec.h=ngs->height;
    if(rect)rec=*rect;
    LOGV("FillRect %p %d,%d-%d,%d color=0x%x pitch=%d",ngs,rec.x,rec.y,rec.w,rec.h,color,ngs->pitch);
    UINT*fb=(UINT*)(ngs->buffer+ngs->pitch*rec.y+rec.x*4);
    UINT*fbtop=fb;
    for(x=0;x<rec.w;x++)fb[x]=color;
    for(y=1;y<rec.h;y++){
        fb+=(ngs->pitch>>2);
        memcpy(fb,fbtop,rec.w*4);
    }
#ifdef ENABLE_RFB
    rfbMarkRectAsModified(dev.rfbScreen,rec.x,rec.y,rec.w,rec.h);
#endif
    return E_OK;
}

DWORD GFXFlip(HANDLE surface){
    FBSURFACE*surf=(FBSURFACE*)surface;
    if(surf->ishw){
       dev.var.yoffset=0;
       int ret=ioctl(dev.fb, FBIOPAN_DISPLAY, &dev.var);
       LOGD_IF(ret<0,"FBIOPAN_DISPLAY=%d yoffset=%d",ret,dev.var.yoffset);
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
    dev.rfbScreen->paddedWidthInBytes=surf->pitch;
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
    LOGV("format=%d %dx%d:%d",format,width,height,fb->pitch);
    rfbNewFramebuffer(dev.rfbScreen,fb->buffer,width,height,8,3,4);
    dev.rfbScreen->paddedWidthInBytes=fb->pitch;
}
#endif

DWORD GFXCreateSurface(HANDLE*surface,UINT width,UINT height,INT format,BOOL hwsurface){
    FBSURFACE*surf=(FBSURFACE*)malloc(sizeof(FBSURFACE));
    surf->width=width;
    surf->height=height;
    surf->format=format;
    surf->ishw=hwsurface;
    surf->pitch=width*4;
    if(hwsurface){
        size_t mem_len=((dev.fix.smem_start) -((dev.fix.smem_start) & ~(getpagesize() - 1)));
        setfbinfo(surf);
        surf->buffer=mmap( NULL,dev.fix.smem_len,PROT_READ | PROT_WRITE, MAP_SHARED,dev.fb, 0 );
        dev.rfbScreen->frameBuffer = surf->buffer;
        surf->pitch=dev.fix.line_length;
        ResetScreenFormat(surf,width,height,format);
    }else{
        surf->buffer=malloc(width*surf->pitch);
    }
    surf->ishw=hwsurface;
    LOGV("surface=%x buf=%p size=%dx%d hw=%d",surf,surf->buffer,width,height,hwsurface);
    *surface=surf;
    return E_OK;
}


DWORD GFXBlit(HANDLE dstsurface,int dx,int dy,HANDLE srcsurface,const GFXRect*srcrect){
    unsigned int x,y,sw,sh;
    FBSURFACE*ndst=(FBSURFACE*)dstsurface;
    FBSURFACE*nsrc=(FBSURFACE*)srcsurface;
    GFXRect rs={0,0};
    BYTE*pbs=(BYTE*)nsrc->buffer;
    BYTE*pbd=(BYTE*)ndst->buffer;
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
    for(y=0;y<rs.h;y++){
        memcpy(pbd,pbs,rs.w*4);
        pbs+=nsrc->pitch;
        pbd+=ndst->pitch;
    }
#ifdef ENABLE_RFB
    if(ndst->ishw)rfbMarkRectAsModified(dev.rfbScreen,dx,dy,dx+rs.w,dy+rs.h);
#endif
    return 0;
}

DWORD GFXDestroySurface(HANDLE surface){
    FBSURFACE*surf=(FBSURFACE*)surface;
    if(surf->ishw)
        munmap(surf->buffer,dev.fix.smem_len);
    else if(surf->buffer)free(surf->buffer);
    free(surf);
    return 0;
}
