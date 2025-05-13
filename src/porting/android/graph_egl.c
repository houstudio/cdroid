#include <stdio.h>
#include <cdgraph.h>
#include <cdlog.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <cdinput.h>
#include <core/eventcodes.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include "android_native_app_glue.h"
typedef struct {
    EGLDisplay display;
    int width;
    int height;
    int pitch;
    GLuint texture;
    char* buffer;
} FBDEVICE;

typedef struct {
    int dispid;
    GLuint texture;
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
    int format;
    int ishw;
    char*buffer;
} FBSURFACE;

static FBDEVICE devs[2]= {0};
static GFXRect screenMargin= {0};
#define SENDKEY(time,k,down) {InjectKey(time,EV_KEY,k,down);}
#if 1
#define SENDMOUSE(time,x,y)  {\
      InjectABS(time,EV_ABS,ABS_MT_TRACKING_ID,12);\
      InjectABS(time,EV_ABS,ABS_MT_POSITION_X,x);\
      InjectABS(time,EV_ABS,ABS_MT_POSITION_Y,y);\
      InjectABS(time,EV_ABS,SYN_MT_REPORT,0);\
      InjectABS(time,EV_SYN,SYN_REPORT,0);}
#else
#define SENDMOUSE(time,x,y)  {\
      InjectABS(time,EV_ABS,ABS_X,x);\
      InjectABS(time,EV_ABS,ABS_Y,y);\
      InjectABS(time,EV_SYN,SYN_REPORT,0);}
#endif
static void InjectKey(unsigned long etime,int type, int code, int value) {
    INPUTEVENT i = { 0 };
    i.tv_sec = etime/1000;
    i.tv_usec = (etime%1000)*1000;
    i.type = type;
    i.code = code;
    i.value = value;
    i.device = INJECTDEV_KEY;
    InputInjectEvents(&i, 1, 1);
}

static void InjectABS(unsigned long etime, int type, int axis, int value);
static void DrawScene(int width, int height);
static unsigned int xglThreadProc(void * param);

int32_t GFXInit() {
    if(devs[0].display)return E_OK;
    memset(devs,0,sizeof(devs));
    FBDEVICE*dev=&devs[0];
    const char*strMargins=getenv("SCREEN_MARGINS");
    const char* DELIM=",;";
    if(strMargins){
        char *sm  = strdup(strMargins);
        char*token= strtok(sm,DELIM);
        screenMargin.x = atoi(token);
        token = strtok(NULL,DELIM);
        screenMargin.y = atoi(token);
        token = strtok(NULL,DELIM);
        screenMargin.w = atoi(token);
        token = strtok(NULL,DELIM);
        screenMargin.h = atoi(token);
        free(sm);
    }
    devs[0].display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    return E_OK;
}

int32_t GFXGetDisplayCount() {
    return 1;
}

int32_t GFXGetDisplaySize(int dispid,uint32_t*width,uint32_t*height) {
    if(dispid<0||dispid>=GFXGetDisplayCount())return E_ERROR;
    FBDEVICE*dev=devs+dispid;
    *width =dev->width -(screenMargin.x + screenMargin.w);
    *height=dev->height-(screenMargin.y + screenMargin.h);
    LOGV("screen[%d]size=%dx%d/%dx%d",dispid,*width,*height,dev->width,dev->height);
    return E_OK;
}


int32_t GFXLockSurface(GFXHANDLE surface,void**buffer,uint32_t*pitch) {
    FBSURFACE*ngs=(FBSURFACE*)surface;
    *buffer=ngs->buffer;
    *pitch=ngs->pitch;
    return 0;
}

int32_t GFXGetSurfaceInfo(GFXHANDLE surface,uint32_t*width,uint32_t*height,int32_t *format) {
    FBSURFACE*ngs=(FBSURFACE*)surface;
    *width = ngs->width;
    *height= ngs->height;
    *format= ngs->format;
    return E_OK;
}

int32_t GFXUnlockSurface(GFXHANDLE surface) {
    return 0;
}

int32_t GFXSurfaceSetOpacity(GFXHANDLE surface,uint8_t alpha) {
    return 0;//dispLayer->SetOpacity(dispLayer,alpha);
}

int32_t GFXFillRect(GFXHANDLE surface,const GFXRect*rect,uint32_t color) {
    FBSURFACE*ngs=(FBSURFACE*)surface;
    uint32_t x,y;
    GFXRect rec= {0,0,0,0};
    rec.w=ngs->width;
    rec.h=ngs->height;
    if(rect)rec=*rect;
    LOGV("FillRect %p %d,%d-%d,%d color=0x%x pitch=%d",ngs,rec.x,rec.y,rec.w,rec.h,color,ngs->pitch);
    uint32_t* fb = (uint32_t*)(ngs->buffer + ngs->pitch * rec.y + rec.x * 4);
    uint32_t* fbtop = fb;
    for (x = 0; x < rec.w; x++)fb[x] = color;
    const int cpw = rec.w * 4;
    for (y = 1; y < rec.h; y++) {
        fb += (ngs->pitch >> 2);
        memcpy(fb, fbtop, cpw);
    }
    return E_OK;
}

int32_t GFXFlip(GFXHANDLE surface) {
    FBSURFACE*surf=(FBSURFACE*)surface;
    FBDEVICE*dev=devs+surf->dispid;
    if(surf->ishw) {
        //XFlush(devs[0].display);
    }
    return 0;
}

int32_t GFXCreateSurface(int dispid,GFXHANDLE*surface,uint32_t width,uint32_t height,int32_t format,bool hwsurface) {
    FBSURFACE*surf=(FBSURFACE*)malloc(sizeof(FBSURFACE));
    FBDEVICE*dev = &devs[dispid];
    surf->dispid=dispid;
    surf->width= hwsurface?dev->width:width;
    surf->height=hwsurface?dev->height:height;
    surf->format=format;
    surf->ishw=hwsurface;
    surf->pitch=width*4;

    if(hwsurface){
        surf->texture=devs[0].texture;
        surf->buffer=devs[0].buffer;
    } else {
        surf->buffer = malloc(surf->pitch * surf->height);
    }
    surf->ishw=hwsurface;
    LOGI("surface=%x buf=%p size=%dx%dx%d hw=%d",surf,surf->buffer,width,height,surf->pitch,hwsurface);
    *surface=surf;
    return E_OK;
}


int32_t GFXBlit(GFXHANDLE dstsurface,int dx,int dy,GFXHANDLE srcsurface,const GFXRect*srcrect) {
    unsigned int x,y,sw,sh;
    FBSURFACE*ndst=(FBSURFACE*)dstsurface;
    FBSURFACE*nsrc=(FBSURFACE*)srcsurface;
    GFXRect rs= {0,0};
    uint8_t*pbs=(uint8_t*)nsrc->buffer;
    uint8_t*pbd=(uint8_t*)ndst->buffer;
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
    if(dx+rs.w>ndst->width - screenMargin.x - screenMargin.w)
        rs.w = ndst->width - screenMargin.x - screenMargin.w-dx;
    if(dy+rs.h>ndst->height- screenMargin.y- screenMargin.h)
        rs.h = ndst->height- screenMargin.y- screenMargin.h -dy;

    LOGV("Blit %p %d,%d-%d,%d -> %p %d,%d buffer=%p->%p",nsrc,rs.x,rs.y,rs.w,rs.h,ndst,dx,dy,pbs,pbd);
#if 0
    pbs += rs.y * nsrc->pitch + rs.x * 4;
    if (ndst->ishw == 0)pbd += dy * ndst->pitch + dx * 4;
    else pbd += (dy + screenMargin.y) * ndst->pitch + (dx + screenMargin.x) * 4;
    const int cpw = rs.w * 4;
    for (y = 0; y < rs.h; y++) {
        memcpy(pbd, pbs, cpw);
        pbs += nsrc->pitch;
        pbd += ndst->pitch;
    }
#else
#endif
    return 0;
}

int32_t GFXDestroySurface(GFXHANDLE surface) {
    FBSURFACE* surf = (FBSURFACE*)surface;
    FBDEVICE* dev = devs + surf->dispid;
    if (!surf->ishw) {
        free(surf->buffer);
        surf->buffer = NULL;
    }
    free(surf);
    return 0;
}

static void InjectABS(unsigned long etime, int type, int axis, int value) {
    INPUTEVENT i = { 0 };
    i.tv_sec = etime / 1000;
    i.tv_usec = (etime % 1000) * 1000;
    i.type = type;
    i.code = axis;
    i.value = value;
    i.device = INJECTDEV_TOUCH;
    InputInjectEvents(&i, 1, 1);
}

static unsigned int xglThreadProc(void * param)
{
    return 0;
}

