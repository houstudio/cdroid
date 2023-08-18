#include <cdtypes.h>
#include <ngl_os.h>
#include <cdgraph.h>
#include <cdlog.h>
#include <stdio.h>
#include <stdlib.h> /* getenv(), etc. */
#include <unistd.h>
#include <sys/time.h>
#include <rfb/rfb.h>
#include <rfb/keysym.h>
#include <rfb/rfbproto.h>
#include <cdinput.h>

typedef struct {
    unsigned int width;
    unsigned int height;
    unsigned int pitch;
    int format;
    void*data;
    unsigned int ishw;
} NGLSURFACE;
extern void setupRFB(rfbScreenInfoPtr rfbScreen,const char*name,int port);
static rfbScreenInfoPtr rfbScreen;
#define PRINTFMT(f) LOGV("truecolor=%d ,depth=%d bitsPerPixel=%d rgbmax=%x/%x/%x rgbshift=%x/%x/%x",\
	       	!!(f)->serverFormat.trueColour,(f)->serverFormat.depth,(f)->serverFormat.bitsPerPixel,\
		(f)->serverFormat.redMax,(f)->serverFormat.greenMax,(f)->serverFormat.blueMax,\
  		(f)->serverFormat.redShift,(f)->serverFormat.greenShift,(f)->serverFormat.blueShift)
static void onExit() {
    LOGD("rfbServer shutdown!");
    rfbShutdownServer(rfbScreen,1);
    rfbScreenCleanup(rfbScreen);
}
DWORD GFXInit() {
    int x=0,y=0,*fb;
    HANDLE tid;
    int width=1280,height=720;
    if(rfbScreen)return E_OK;

    rfbScreen = rfbGetScreen(NULL,NULL,800,600,8,3,3);
    setupRFB(rfbScreen,"X5RFB",0);
    rfbInitServer(rfbScreen);
    PRINTFMT(rfbScreen);
    LOGD("VNC Server Inited rfbScreen=%p port=%d framebuffer=%p",rfbScreen,rfbScreen->port,rfbScreen->frameBuffer);
    atexit(onExit);
    return E_OK;
}

DWORD GFXGetDisplaySize(UINT*width,UINT*height) {
    *width = rfbScreen->width;
    *height = rfbScreen->height;
    LOGD("size=%dx%d",*width,*height);
    return E_OK;
}

DWORD GFXLockSurface(HANDLE surface,void**buffer,UINT*pitch) {
    NGLSURFACE*ngs=(NGLSURFACE*)surface;
    *buffer=ngs->data;
    *pitch=ngs->pitch;
    //LOGD("%p buffer=%p pitch=%d",ngs,*buffer,*pitch);
    return 0;
}

DWORD GFXGetSurfaceInfo(HANDLE surface,UINT*width,UINT*height,INT *format) {
    NGLSURFACE*ngs=(NGLSURFACE*)surface;
    if(width)*width=ngs->width;
    if(height)*height=ngs->height;
    if(format)*format=ngs->format;//GPF_ABGR;
    //LOGV("%p size=(%dx%d) format=%d",ngs,ngs->width,ngs->height,ngs->format);
    //PRINTFMT(rfbScreen);
    return E_OK;
}

DWORD GFXUnlockSurface(HANDLE surface) {
    return 0;
}

DWORD GFXSurfaceSetOpacity(HANDLE surface,BYTE alpha) {
    return 0;//dispLayer->SetOpacity(dispLayer,alpha);
}

DWORD GFXFillRect(HANDLE surface,const GFXRect*rect,UINT color) {
    NGLSURFACE*ngs=(NGLSURFACE*)surface;
    UINT x,y;
    GFXRect rec= {0,0,0,0};
    rec.w=ngs->width;
    rec.h=ngs->height;
    if(rect)rec=*rect;
    LOGV("FillRect %p %d,%d-%d,%d color=0x%x pitch=%d",ngs,rec.x,rec.y,rec.w,rec.h,color,ngs->pitch);
    UINT*fb=(UINT*)(ngs->data+ngs->pitch*rec.y+rec.x*4);
    for(y=0; y<rec.h; y++) {
        for(x=0; x<rec.w; x++)
            fb[x]=color;
        fb+=(ngs->pitch>>2);
    }
    return E_OK;
}

DWORD GFXFlip(HANDLE surface) {
    NGLSURFACE*ngs=(NGLSURFACE*)surface;
    if(ngs->ishw)rfbMarkRectAsModified(rfbScreen,0,0,rfbScreen->width,rfbScreen->height);
    LOGV("flip %p ishw=%d",ngs,ngs->ishw);
    return 0;
}

static void ResetScreenFormat(NGLSURFACE*fb,int width,int height,int format) {
    rfbPixelFormat*fmt=&rfbScreen->serverFormat;
    fmt->trueColour=TRUE;
    switch(format) {
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
    default:
        return;
    }
    LOGD("format=%d",format);
    rfbNewFramebuffer(rfbScreen,fb->data,width,height,8,3,4);
}

DWORD GFXCreateSurface(HANDLE*surface,UINT width,UINT height,INT format,BOOL ishwsurface) {
    //XShmCreateImage XShmCreatePixmap
    NGLSURFACE*nglsurface=(NGLSURFACE*)malloc(sizeof(NGLSURFACE));
    if((height*width==0)||(surface==NULL))
        return E_INVALID_PARA;
    if(format!=GPF_ABGR&&format!=GPF_ARGB) {
        LOGD("Format %d is not supported %d",format,GPF_ARGB);
        return E_NOT_SUPPORT;
    }
    nglsurface->data=malloc(width*height*4);
    if(ishwsurface) {
        rfbScreen->frameBuffer=nglsurface->data;
        ResetScreenFormat(nglsurface,width,height,format);
    }
    nglsurface->ishw=ishwsurface;
    nglsurface->width=width;
    nglsurface->height=height;
    nglsurface->pitch=width*4;
    nglsurface->format=format;
    *surface=(HANDLE)nglsurface;
    LOGV("surface=%p framebuffer=%p size=%dx%d format=%d hw=%d",nglsurface,
         nglsurface->data,width,height,format,ishwsurface);
    return E_OK;
}

DWORD GFXBlit(HANDLE dstsurface,int dx,int dy,HANDLE srcsurface,const GFXRect*srcrect) {
    unsigned int x,y,sw,sh;
    NGLSURFACE*ndst=(NGLSURFACE*)dstsurface;
    NGLSURFACE*nsrc=(NGLSURFACE*)srcsurface;
    GFXRect rs= {0,0};
    BYTE*pbs=(BYTE*)nsrc->data;
    BYTE*pbd=(BYTE*)ndst->data;
    rs.w=nsrc->width;
    rs.h=nsrc->height;
    if(srcrect)rs=*srcrect;
    if(((int)rs.w+dx<=0)||((int)rs.h+dy<=0)||(dx>=(int)ndst->width)||(dy>=(int)ndst->height)||(rs.x<0)||(rs.y<0)) {
        LOGV("dx=%d,dy=%d rs=(%d,%d-%d,%d)",dx,dy,rs.x,rs.y,rs.w,rs.h);
        return E_INVALID_PARA;
    }

    LOGV("Blit %p[%dx%d] %d,%d-%d,%d -> %p[%dx%d] %d,%d",nsrc,nsrc->width,nsrc->height,
         rs.x,rs.y,rs.w,rs.h,ndst,ndst->width,ndst->height,dx,dy);
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
    if(dx+rs.w>ndst->width)rs.w=ndst->width-dx;
    if(dy+rs.h>ndst->height)rs.h=ndst->height-dy;

    LOGV("Blit %p %d,%d-%d,%d -> %p %d,%d buffer=%p->%p",nsrc,rs.x,rs.y,rs.w,rs.h,ndst,dx,dy,pbs,pbd);
    pbs+=rs.y*nsrc->pitch+rs.x*4;
    pbd+=dy*ndst->pitch+dx*4;
    for(y=0; y<rs.h; y++) {
        memcpy(pbd,pbs,rs.w*4);
        pbs+=nsrc->pitch;
        pbd+=ndst->pitch;
    }
    if(ndst->ishw)rfbMarkRectAsModified(rfbScreen,dx,dy,dx+rs.w,dy+rs.h);
    return 0;
}

DWORD GFXDestroySurface(HANDLE surface) {
    NGLSURFACE*ngs=(NGLSURFACE*)surface;
    if(ngs->data) {
        free(ngs->data);
        ngs->data=NULL;
    }
    if(ngs->ishw)
        ResetScreenFormat(ngs,ngs->width,ngs->height,ngs->format);
    free(ngs);
    return 0;
}
