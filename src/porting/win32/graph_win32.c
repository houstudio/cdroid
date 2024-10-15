#include <cdgraph.h>
#include <cdlog.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

#include <Windows.h>
typedef struct {
    HWND hwnd;
    int width;
    int height;
} FBDEVICE;

typedef struct {
    int dispid;
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
    int format;
    int ishw;
    char*buffer;
    char*kbuffer;/*kernel buffer address*/
} FBSURFACE;

static FBDEVICE devs[2]= {-1};
static GFXRect screenMargin= {0};

static unsigned int __stdcall display_thread(void * param)
{
    DWORD window_style= WS_OVERLAPPEDWINDOW;
    window_style &= ~(WS_SIZEBOX | WS_MAXIMIZEBOX | WS_THICKFRAME);
    char*env = getenv("SCREEN_SIZE");
    int width=1280,height=720;
    MSG message;
    if(env){
        width= atoi(env);
        env = strpbrk(env,"xX*,");
        if(env)height=atoi(env+1);
    }
    devs[0].hwnd = CreateWindowExW(WS_EX_CLIENTEDGE,
           L"CDROID.Window",L"CDROID",window_style,CW_USEDEFAULT,
           0,width,height,NULL,NULL,NULL,NULL);
    
    ShowWindow(devs[0].hwnd, SW_SHOW);
    UpdateWindow(devs[0].hwnd);
    LOGI("Win32 solution=%dx%d \r\n", width, height);
    SetEvent((HANDLE)param);
    while(GetMessageW(&message, NULL, 0, 0)) {
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }
}

int32_t GFXInit() {
    if(devs[0].hwnd>=0)return E_OK;
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
    HANDLE mutex = CreateEventExW(NULL, NULL, 0, EVENT_ALL_ACCESS);
    HANDLE thread = (HANDLE)_beginthreadex(NULL,0,display_thread,mutex,0,NULL);
    WaitForSingleObjectEx(mutex, INFINITE, FALSE);
    CloseHandle(mutex);
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


int32_t GFXLockSurface(HANDLE surface,void**buffer,uint32_t*pitch) {
    FBSURFACE*ngs=(FBSURFACE*)surface;
    *buffer=ngs->buffer;
    *pitch=ngs->pitch;
    return 0;
}

int32_t GFXGetSurfaceInfo(HANDLE surface,uint32_t*width,uint32_t*height,int32_t *format) {
    FBSURFACE*ngs=(FBSURFACE*)surface;
    *width = ngs->width;
    *height= ngs->height;
    *format= ngs->format;
    return E_OK;
}

int32_t GFXUnlockSurface(HANDLE surface) {
    return 0;
}

int32_t GFXSurfaceSetOpacity(HANDLE surface,uint8_t alpha) {
    return 0;//dispLayer->SetOpacity(dispLayer,alpha);
}

int32_t GFXFillRect(HANDLE surface,const GFXRect*rect,uint32_t color) {
    FBSURFACE*ngs=(FBSURFACE*)surface;
    uint32_t x,y;
    GFXRect rec= {0,0,0,0};
    rec.w=ngs->width;
    rec.h=ngs->height;
    if(rect)rec=*rect;
    LOGV("FillRect %p %d,%d-%d,%d color=0x%x pitch=%d",ngs,rec.x,rec.y,rec.w,rec.h,color,ngs->pitch);
    uint32_t*fb=(uint32_t*)(ngs->buffer+ngs->pitch*rec.y+rec.x*4);
    uint32_t*fbtop=fb;
    for(x=0; x<rec.w; x++)fb[x]=color;
    const int cpw=rec.w*4;
    long copied=0;
    for(y=1; y<rec.h; y++) {
        fb+=(ngs->pitch>>2);
        memcpy(fb,fbtop,cpw);
        copied+=ngs->pitch;
    }
    return E_OK;
}

int32_t GFXFlip(HANDLE surface) {
    FBSURFACE*surf=(FBSURFACE*)surface;
    FBDEVICE*dev=devs+surf->dispid;
    if(surf->ishw) {
        GFXRect rect= {0,0,surf->width,surf->height};
        //if(rc)rect=*rc;
        dev->var.yoffset=0;
        int ret=ioctl(dev->fb, FBIOPAN_DISPLAY, &dev->var);
        LOGD_IF(ret<0,"FBIOPAN_DISPLAY=%d yoffset=%d",ret,dev->var.yoffset);
    }
    return 0;
}

static int setfbinfo(FBSURFACE*surf) {
    int rc=-1;
    FBDEVICE*dev=devs+surf->dispid;
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


int32_t GFXCreateSurface(int dispid,HANDLE*surface,uint32_t width,uint32_t height,int32_t format,BOOL hwsurface) {
    FBSURFACE*surf=(FBSURFACE*)malloc(sizeof(FBSURFACE));
    FBDEVICE*dev = &devs[dispid];
    surf->dispid=dispid;
    surf->width= hwsurface?dev->var.xres:width;
    surf->height=hwsurface?dev->var.yres:height;
    surf->format=format;
    surf->ishw=hwsurface;
    surf->pitch=width*4;
    size_t buffer_size=surf->height*surf->pitch;
    if(hwsurface && devs[dispid].fix.smem_len) {
        size_t mem_len=((dev->fix.smem_start) -((dev->fix.smem_start) & ~(getpagesize() - 1)));
        buffer_size=surf->height*dev->fix.line_length;
        setfbinfo(surf);
        surf->kbuffer=(dev->fix.smem_start);
        surf->buffer=(char*)mmap( NULL,buffer_size,PROT_READ | PROT_WRITE, MAP_SHARED,dev->fb, 0 );
        surf->pitch=dev->fix.line_length;
    } else {
        surf->kbuffer=0;
        surf->buffer=(char*)malloc(buffer_size);
    }
    surf->ishw=hwsurface;
    LOGV("surface=%x buf=%p/%p size=%dx%d hw=%d",surf,surf->kbuffer,surf->buffer,width,height,hwsurface);
    *surface=surf;
    return E_OK;
}


int32_t GFXBlit(HANDLE dstsurface,int dx,int dy,HANDLE srcsurface,const GFXRect*srcrect) {
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
    pbs+=rs.y*nsrc->pitch+rs.x*4;
    if(ndst->ishw==0)pbd+=dy*ndst->pitch+dx*4;
    else pbd+=(dy+screenMargin.y)*ndst->pitch+(dx+screenMargin.x)*4;
    const int cpw=rs.w*4;
    for(y=0; y<rs.h; y++) {
        memcpy(pbd,pbs,cpw);
        pbs+=nsrc->pitch;
        pbd+=ndst->pitch;
    }
    return 0;
}

int32_t GFXDestroySurface(HANDLE surface) {
    FBSURFACE*surf=(FBSURFACE*)surface;
    FBDEVICE*dev=devs+surf->dispid;
    if(surf->ishw)
        munmap(surf->buffer,surf->pitch*surf->height);
    else if(surf->buffer)free(surf->buffer);
    free(surf);
    return 0;
}
