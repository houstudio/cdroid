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
#include <stdio.h>
#include <core/eventcodes.h>
#include <cdinput.h>
#include <lib2d/ingenic2d.h>
#include <libhardware2/fb.h>
#include <signal.h>
typedef struct {
    int fb;
    struct fb_fix_screeninfo fix;
    struct fb_var_screeninfo var;
    struct fb_device_info fb_info;
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
    struct ingenic_2d_frame*frame;
} FBSURFACE;

static FBDEVICE devs[2]= {-1};
static struct ingenic_2d *g2d;
static GFXRect screenMargin= {0};

static void OnExit(){
    FBDEVICE*dev=&devs[0];
    printf("%s Graph Exit g2d=%p\r\n",__FUNCTION__,g2d);
    fb_close(dev->fb,&dev->fb_info);
    if(g2d){
        ingenic_2d_close(g2d);
        g2d=NULL;
    }
}

static void signal_handler(int sig) {
    // 如果在这里调用 _exit(0)，atexit 不会被调用
    FBDEVICE*dev=&devs[0];
    printf("%s Graph Exit g2d=%p\r\n",__FUNCTION__,g2d);
    fb_close(dev->fb,&dev->fb_info);
    if(g2d){
        ingenic_2d_close(g2d);
        g2d=NULL;
    }
    _exit(0);
}

int32_t GFXInit() {
    if(devs[0].fb>=0)return E_OK;
    memset(devs,0,sizeof(devs));
    FBDEVICE*dev=&devs[0];
#if 0
    dev->fb=open("/dev/fb0", O_RDWR);
    // Get fixed screen information
    if(ioctl(dev->fb, FBIOGET_FSCREENINFO, &dev->fix) == -1) {
        LOGE("Error reading fixed information fd=%d",dev->fb);
        return E_ERROR;
    }
    LOGI("fbmem.addr=%x fbmem.size=%d pitch=%d",dev->fix.smem_start,dev->fix.smem_len,dev->fix.line_length);

    // Get variable screen information
    if(ioctl(dev->fb, FBIOGET_VSCREENINFO, &dev->var) == -1) {
        LOGE("Error reading variable information");
        return E_ERROR;
    }
#else
    struct fb_device_info fb_info;
    dev->fb=fb_open("/dev/fb0",&dev->fb_info);
    dev->fix=dev->fb_info.fix;
    dev->var=dev->fb_info.var;
#endif
    const char*strMargin=getenv("SCREEN_MARGINS");
    const char* DELIM=",;";
    if(strMargin){
        char *sm=strdup(strMargin);
        char*token=strtok(sm,DELIM);
        screenMargin.x=atoi(token);
        token=strtok(NULL,DELIM);
        screenMargin.y=atoi(token);
        token=strtok(NULL,DELIM);
        screenMargin.w=atoi(token);
        token=strtok(NULL,DELIM);
        screenMargin.h=atoi(token);
        free(sm);
    }

    dev->var.yoffset=0;//set first screen memory for display
    LOGI("fb_open fd =%d fb_enabled=%d",dev->fb,fb_enable(dev->fb));
    g2d = ingenic_2d_open();
    LOGI("FBIOPUT_VSCREENINFO=%d g2d=%p",ioctl(dev->fb,FBIOPUT_VSCREENINFO,&dev->var),g2d);
    LOGI("fb solution=%dx%d accel_flags=0x%x ScreenMargin=(%d,%d,%d,%d)\r\n",dev->var.xres,dev->var.yres,dev->var.accel_flags,
            screenMargin.x,screenMargin.y,screenMargin.w,screenMargin.h);
    atexit(OnExit);
    signal(SIGINT, signal_handler);
    return E_OK;
}

int32_t GFXTerm(){
    FBDEVICE*dev=&devs[0];
    fb_close(dev->fb,&dev->fb_info);
    ingenic_2d_close(g2d);
    LOGI("Graph Term");
}

int32_t GFXGetDisplayCount() {
    return 1;
}

int32_t GFXGetDisplaySize(int dispid,uint32_t*width,uint32_t*height) {
    if(dispid<0||dispid>=GFXGetDisplayCount())return E_ERROR;
    FBDEVICE*dev=devs+dispid;
    *width =dev->var.xres-(screenMargin.x + screenMargin.w);
    *height=dev->var.yres-(screenMargin.y + screenMargin.h);
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

int32_t GFXFlip(GFXHANDLE surface) {
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

int32_t GFXCreateSurface(int dispid,GFXHANDLE*surface,uint32_t width,uint32_t height,int32_t format,bool hwsurface) {
    FBSURFACE*surf=(FBSURFACE*)malloc(sizeof(FBSURFACE));
    FBDEVICE*dev=devs+dispid;
    surf->dispid=dispid;
    if(hwsurface){
        width = dev->var.xres;
        height= dev->var.yres;
    }
    surf->width=width;
    surf->height=height;
    surf->format=format;
    surf->ishw=hwsurface;
    surf->pitch=width*4;
    size_t buffer_size=surf->height*surf->pitch;
    if(hwsurface && devs[dispid].fix.smem_len) {
        size_t mem_len=((dev->fix.smem_start) -((dev->fix.smem_start) & ~(getpagesize() - 1)));
        buffer_size=surf->height*dev->fix.line_length;
        setfbinfo(surf);
        surf->kbuffer=dev->fix.smem_start;
        surf->buffer=(char*)mmap( NULL,buffer_size,PROT_READ | PROT_WRITE, MAP_SHARED,dev->fb, 0 );
        surf->pitch=dev->fix.line_length;
        surf->frame=ingenic_2d_alloc_frame_by_user(g2d,width,height,INGENIC_2D_ARGB8888,surf->kbuffer,surf->buffer,buffer_size);
    } else {
        surf->frame = ingenic_2d_alloc_frame(g2d,width,height,INGENIC_2D_ARGB8888);
        surf->buffer= surf->frame->addr[0];
        surf->kbuffer=surf->frame->phyaddr[0];
    }
    LOGI("surface=%x buf=%p/%p size=%dx%d hw=%d",surf,surf->kbuffer,surf->buffer,width,height,hwsurface);
    *surface=surf;
    return E_OK;
}


int32_t GFXBlit(GFXHANDLE dstsurface,int dx,int dy,GFXHANDLE srcsurface,const GFXRect*srcrect) {
    unsigned int x,y,sw,sh;
    FBSURFACE*ndst=(FBSURFACE*)dstsurface;
    FBSURFACE*nsrc=(FBSURFACE*)srcsurface;
    struct ingenic_2d_rect ingenic_src,ingenic_dst;
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
    ingenic_src.x=rs.x;
    ingenic_src.y=rs.y;
    ingenic_src.w=rs.w;
    ingenic_src.h=rs.h;
    ingenic_src.frame=nsrc->frame;
    ingenic_dst.x=dx;
    ingenic_dst.y=dy;
    ingenic_dst.w=rs.w;
    ingenic_dst.h=rs.h;
    if(ndst->ishw){
        ingenic_dst.x+=screenMargin.x;
        ingenic_dst.y+=screenMargin.y;
    }
    ingenic_dst.frame=ndst->frame;
    ingenic_2d_blend(g2d,&ingenic_src,&ingenic_dst,255);
    return 0;
}

int32_t GFXDestroySurface(GFXHANDLE surface) {
    FBSURFACE*surf=(FBSURFACE*)surface;
    FBDEVICE*dev=devs+surf->dispid;
    if(surf->ishw){
        munmap(surf->buffer,surf->pitch*surf->height);
    }else {
        if(surf->frame)ingenic_2d_free_frame(g2d,surf->frame);
    }
    free(surf);
    return 0;
}
