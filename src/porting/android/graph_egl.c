#include <stdio.h>
#include <cdgraph.h>
#include <cdlog.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <cdinput.h>
#include <core/eventcodes.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>
#include <android/hardware_buffer.h>
#include "android_native_app_glue.h"

typedef struct {
    EGLDisplay display;
    int width;
    int height;
    int pitch;
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

GLuint primarySurfaceTexture;
static FBDEVICE devs[2]= {0};
static GFXRect screenMargin= {0};

static void InjectABS(unsigned long etime, int type, int axis, int value);
static void DrawScene(int width, int height);

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
    const char*env = getenv("SCREEN_SIZE");
    if(env==NULL) {
        *width = 1280;//dispCfg.width;
        *height= 720;//dispCfg.height;
    } else {
        *width=atoi(env)- screenMargin.x - screenMargin.w;
        env =strpbrk(env,"x*,");
        if((*width<=0)||(env==NULL)) exit(-1);
        *height = atoi(env+1)- screenMargin.y - screenMargin.h;
        if(*height<=0) exit(-1);
    }
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
    FBSURFACE*ngs=(FBSURFACE*)surface;
    *buffer= ngs->buffer;
    *pitch = ngs->pitch;
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
    rec.w = ngs->width;
    rec.h = ngs->height;
    if(rect) rec=*rect;
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
    FBSURFACE*surf= (FBSURFACE*)surface;
    FBDEVICE *dev = devs+surf->dispid;
    if(surf->ishw) {
        //XFlush(devs[0].display);
    }
    return 0;
}

int32_t GFXCreateSurface(int dispid,GFXHANDLE*surface,uint32_t width,uint32_t height,int32_t format,bool hwsurface) {
    FBSURFACE*surf = (FBSURFACE*)malloc(sizeof(FBSURFACE));
    FBDEVICE*dev= &devs[dispid];
    surf->dispid= dispid;
    surf->width = hwsurface ? dev->width:width;
    surf->height= hwsurface ? dev->height:height;
    surf->format= format;
    surf->ishw  = hwsurface;
    surf->pitch = width*4;
    glGenTextures(1,&surf->texture);
    if(hwsurface) {
        primarySurfaceTexture =surf->texture;
    }
    surf->buffer = malloc(surf->pitch*surf->height);
    surf->ishw = hwsurface;
    LOGI("surface=%x buf=%p size=%dx%dx%d hw=%d",surf,surf->buffer,width,height,surf->pitch,hwsurface);
    *surface = surf;
    return E_OK;
}


int32_t GFXBlit(GFXHANDLE dstsurface,int dx,int dy,GFXHANDLE srcsurface,const GFXRect*srcrect) {
    unsigned int x,y,sw,sh;
    FBSURFACE*ndst = (FBSURFACE*)dstsurface;
    FBSURFACE*nsrc = (FBSURFACE*)srcsurface;
    GFXRect rs = {0,0};
    uint8_t*pbs= (uint8_t*)nsrc->buffer;
    uint8_t*pbd= (uint8_t*)ndst->buffer;
    rs.w = nsrc->width;
    rs.h = nsrc->height;
    if(srcrect)rs = *srcrect;
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
    pbs += rs.y * nsrc->pitch + rs.x * 4;
    if (ndst->ishw == 0)pbd += dy * ndst->pitch + dx * 4;
    else pbd += (dy + screenMargin.y) * ndst->pitch + (dx + screenMargin.x) * 4;
    const int cpw = rs.w * 4;
    for (y = 0; y < rs.h; y++) {
        memcpy(pbd, pbs, cpw);
        pbs += nsrc->pitch;
        pbd += ndst->pitch;
    }
    if(ndst->ishw){//GL_UNPACK_ROW_LENGTH is defined in GLES3/gl3.h but not defined in GLES2/gl2.h
        glBindTexture(GL_TEXTURE_2D, ndst->texture);
        glPixelStorei(GL_UNPACK_ROW_LENGTH,ndst->pitch);
        glTexSubImage2D(GL_TEXTURE_2D, 0, rs.x, rs.y, rs.w, rs.h, GL_RGB, GL_UNSIGNED_BYTE, ndst->buffer);
        glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    return 0;
}

int32_t GFXDestroySurface(GFXHANDLE surface) {
    FBSURFACE* surf = (FBSURFACE*)surface;
    FBDEVICE* dev = devs + surf->dispid;
    glBindTexture(GL_TEXTURE_2D, surf->texture);
    free(surf->buffer);
    surf->buffer = NULL;
    free(surf);
    return 0;
}


