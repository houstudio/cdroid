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
#include <linux/input.h>
#include <cdinput.h>
#include <g2d_driver_enh.h>
#include <sunximem.h>

//#define USE_PIXMAN 1
#ifdef USE_PIXMAN
#include <pixman.h>
#endif

typedef struct {
    int fb;
    int g2d;
    struct fb_fix_screeninfo fix;
    struct fb_var_screeninfo var;
} FBDEVICE;

typedef struct {
    int dispid;
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
    int format;
    int ishw;
    int used;
    char*buffer;
    char*kbuffer;/*kernel buffer address*/
} FBSURFACE;

static FBDEVICE devs[2] = {-1};
static GFXRect screenMargin = {0};
static FBSURFACE devSurfaces[16];

int32_t GFXInit() {
    if(devs[0].fb>=0)return E_OK;
    bzero(devs,sizeof(devs));
    FBDEVICE*dev = &devs[0];
    dev->fb = open("/dev/fb0", O_RDWR);
    // Get fixed screen information
    if(ioctl(dev->fb, FBIOGET_FSCREENINFO, &dev->fix) == -1) {
        LOGE("Error reading fixed information fd=%d",dev->fb);
        return E_ERROR;
    }
    LOGI("fb=%d smem.start=%p size=%d pitch=%d",dev->fb,(char*)dev->fix.smem_start,dev->fix.smem_len,dev->fix.line_length);

    // Get variable screen information
    if(ioctl(dev->fb, FBIOGET_VSCREENINFO, &dev->var) == -1) {
        LOGE("Error reading variable information");
        return E_ERROR;
    }
    dev->g2d = open("/dev/g2d",O_RDWR);
    const char*strMargin = getenv("SCREEN_MARGINS");
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
    const size_t displayScreenSize=(dev->var.yres * dev->fix.line_length);
    const size_t screenSize = (dev->var.yres - screenMargin.y - screenMargin.h) * (dev->fix.line_length - (screenMargin.x + screenMargin.w)*4);
    size_t numSurfaces= (dev->fix.smem_len - displayScreenSize)/screenSize+1;
    char*buffStart = (char*)mmap(0,dev->fix.smem_len, PROT_READ | PROT_WRITE, MAP_SHARED, dev->fb, 0);
    char*kbuffStart= (char*)devs[0].fix.smem_start;
    devSurfaces[0].kbuffer= kbuffStart;
    devSurfaces[0].buffer = buffStart;
    devSurfaces[0].width  = dev->var.xres;
    devSurfaces[0].height = dev->var.yres;
    devSurfaces[0].pitch  = dev->fix.line_length;
    kbuffStart += displayScreenSize;
    buffStart  += displayScreenSize;
    sunxifb_mem_init();
    LOGI("g2dfd=%d numSurfaces=%d dissze=%d,screenSize=%d",dev->g2d,numSurfaces,displayScreenSize,screenSize);
    for(int i = 1; (i < numSurfaces)&&(i<sizeof(devSurfaces)/sizeof(devSurfaces[0])) ; i++){
        devSurfaces[i].kbuffer= kbuffStart;
        devSurfaces[i].buffer = buffStart;
        devSurfaces[i].width  = dev->var.xres;
        devSurfaces[i].height = dev->var.yres;
        devSurfaces[i].used = 0;
        kbuffStart += screenSize;
        buffStart += screenSize;
        LOGI("suf[%d]buffer=%p/%p",i,kbuffStart,buffStart);
    }
    for(int i=numSurfaces;i<4;i++){
        devSurfaces[i].buffer = sunxifb_mem_alloc(screenSize,"ion.surface");
        devSurfaces[i].kbuffer= sunxifb_mem_get_phyaddr(devSurfaces[i].buffer);
        devSurfaces[i].width  = dev->var.xres;
        devSurfaces[i].height = dev->var.yres;
        devSurfaces[i].used = 0;
        if(devSurfaces[i].buffer) numSurfaces++;
        else break;
        LOGI("suf[%d]buffer=%p/%p",i,devSurfaces[i].kbuffer,devSurfaces[i].buffer);
    }
    dev->var.yoffset = 0;//set first screen memory for display
    int rc = ioctl(dev->fb,FBIOPUT_VSCREENINFO,&dev->var);
    LOGI("FBIOPUT_VSCREENINFO=%d g2dfd=%d numSurfaces=%d",rc,dev->g2d,numSurfaces);
    LOGI("fb solution=%dx%d accel_flags=0x%x\r\n",dev->var.xres,dev->var.yres,dev->var.accel_flags);
    return E_OK;
}

int32_t GFXGetDisplayCount() {
    return 1;
}

int32_t GFXGetDisplaySize(int dispid,uint32_t*width,uint32_t*height) {
    if(dispid<0||dispid>=GFXGetDisplayCount())return E_ERROR;
    FBDEVICE*dev=devs+dispid;
    *width =dev->var.xres-(screenMargin.x + screenMargin.w);
    *height=dev->var.yres-(screenMargin.y + screenMargin.h);
    LOGV("screen[%d]size=%dx%d/%dx%d",dispid,*width,*height,dev->var.xres,dev->var.yres);
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
    FBDEVICE*dev =&devs[ngs->dispid];
    uint32_t x,y;
    GFXRect rec= {0,0,0,0};
    rec.w=ngs->width;
    rec.h=ngs->height;
    if(rect)rec=*rect;
    LOGV("FillRect %p %d,%d-%d,%d color=0x%x pitch=%d",ngs,rec.x,rec.y,rec.w,rec.h,color,ngs->pitch);
    if((ngs->kbuffer==0)&&(ngs->ishw==0)){
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
    }else{
        g2d_fillrect_h fill;
        bzero(&fill,sizeof(fill));
        fill.dst_image_h.mode = G2D_PIXEL_ALPHA;
        fill.dst_image_h.color = color;
        fill.dst_image_h.format = G2D_FORMAT_ARGB8888;
        fill.dst_image_h.clip_rect.x = rec.x;
        fill.dst_image_h.clip_rect.y = rec.y;
        fill.dst_image_h.clip_rect.w = rec.w;
        fill.dst_image_h.clip_rect.h = rec.h;
        fill.dst_image_h.width = ngs->pitch;
        fill.dst_image_h.height = ngs->height;
        fill.dst_image_h.align[0] = 0;
        fill.dst_image_h.align[1] = fill.dst_image_h.align[0];
        fill.dst_image_h.align[2] = fill.dst_image_h.align[0];
        fill.dst_image_h.laddr[0] = (uintptr_t) ngs->kbuffer;//sunxifb_mem_get_phyaddr(dest_buf);
        fill.dst_image_h.laddr[1] = (uintptr_t) 0;
        fill.dst_image_h.laddr[2] = (uintptr_t) 0;
        fill.dst_image_h.use_phy_addr = 1;
        ioctl(dev->g2d, G2D_CMD_FILLRECT_H, (uintptr_t)(&fill));
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

static FBSURFACE*getFreeSurface(){
    for(int i=0;i<sizeof(devSurfaces)/sizeof(FBSURFACE);i++){
        if(!devSurfaces[i].used){
            devSurfaces[i].used++;
            return devSurfaces+i;
        }
    }
    return NULL;
}

int32_t GFXCreateSurface(int dispid,GFXHANDLE*surface,uint32_t width,uint32_t height,int32_t format,bool hwsurface) {
    FBSURFACE*surf = getFreeSurface();
    FBDEVICE*dev = &devs[dispid];
    surf->dispid= dispid;
    surf->width = hwsurface?dev->var.xres:width;
    surf->height= hwsurface?dev->var.yres:height;
    surf->format= format;
    surf->ishw = hwsurface;
    surf->pitch = width*4;
    size_t buffer_size=surf->height*surf->pitch;
    if(hwsurface) {
        setfbinfo(surf);
        dev->var.yoffset = 0;
        ioctl(dev->fb,FBIOPAN_DISPLAY,&dev->var);
    } else {
        if(surf->kbuffer==0){
            surf->buffer = (char*)malloc(buffer_size);
        }
    }
    surf->ishw=hwsurface;
    LOGD("surface=%x buf=%p/%p size=%dx%d hw=%d",surf,surf->kbuffer,surf->buffer,width,height,hwsurface);
    *surface = surf;
    return E_OK;
}


int32_t GFXBlit(GFXHANDLE dstsurface,int dx,int dy,GFXHANDLE srcsurface,const GFXRect*srcrect) {
    unsigned int x,y,sw,sh;
    FBSURFACE*ndst = (FBSURFACE*)dstsurface;
    FBSURFACE*nsrc = (FBSURFACE*)srcsurface;
    FBDEVICE*dev = devs+ndst->dispid;
    GFXRect rs= {0,0};
    uint8_t*pbs = (uint8_t*)nsrc->buffer;
    uint8_t*pbd = (uint8_t*)ndst->buffer;
    rs.w = nsrc->width;
    rs.h = nsrc->height;
    if(srcrect)rs =*srcrect;
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
    if((dev->g2d<0)||((nsrc->ishw==0)&&(nsrc->kbuffer==NULL))){
#ifndef USE_PIXMAN
        for(y=0; y<rs.h; y++) {
            memcpy(pbd,pbs,cpw);
            pbs+=nsrc->pitch;
            pbd+=ndst->pitch;
        }
#else
        pixman_image_t *src_image = pixman_image_create_bits(PIXMAN_a8r8g8b8, nsrc->width, nsrc->height, (uint32_t*)nsrc->buffer, nsrc->pitch);
        pixman_image_t *dst_image = pixman_image_create_bits(PIXMAN_a8r8g8b8, ndst->width, ndst->height, (uint32_t*)ndst->buffer, ndst->pitch);
        pixman_image_composite(PIXMAN_OP_SRC, src_image,NULL/*mask*/, dst_image,rs.x, rs.y, 0, 0, dx, dy, rs.w, rs.h);
        pixman_image_unref(src_image);
        pixman_image_unref(dst_image);	
#endif
    }else{
        g2d_blt_h blt;
        bzero(&blt,sizeof(blt));
        blt.flag_h = G2D_BLT_NONE|G2D_ROT_0;
        blt.src_image_h.width = nsrc->width;
        blt.src_image_h.height = nsrc->height;
        blt.src_image_h.laddr[0]=(uintptr_t)((nsrc->kbuffer||nsrc->ishw)?nsrc->kbuffer:nsrc->buffer);
        blt.src_image_h.clip_rect.x = rs.x;
        blt.src_image_h.clip_rect.y = rs.y;
        blt.src_image_h.clip_rect.w= rs.w;
        blt.src_image_h.clip_rect.h = rs.h;
        blt.src_image_h.format =G2D_FORMAT_ARGB8888;
        blt.src_image_h.align[0] = 0;
        blt.src_image_h.align[1] = blt.src_image_h.align[0];
        blt.src_image_h.align[2] = blt.src_image_h.align[0];
        blt.src_image_h.alpha = 255;
        blt.src_image_h.use_phy_addr = (nsrc->kbuffer!=NULL);
        //blit dest info
        blt.dst_image_h.clip_rect.x = dx;
        blt.dst_image_h.clip_rect.y = dy;
        blt.dst_image_h.clip_rect.w= rs.w;
        blt.dst_image_h.clip_rect.h = rs.h;
        blt.dst_image_h.width = ndst->width;
        blt.dst_image_h.height = ndst->height;
        blt.dst_image_h.mode = G2D_GLOBAL_ALPHA;
        blt.dst_image_h.laddr[0]=(uintptr_t)((ndst->kbuffer||ndst->ishw)?ndst->kbuffer:ndst->buffer);
        blt.dst_image_h.format =G2D_FORMAT_ARGB8888;
        blt.dst_image_h.align[0] = 0;
        blt.dst_image_h.align[1] = blt.dst_image_h.align[0];
        blt.dst_image_h.align[2] = blt.dst_image_h.align[0];        
        blt.dst_image_h.alpha = 255;
        blt.dst_image_h.use_phy_addr = (ndst->kbuffer!=NULL)||ndst->ishw;
        blt.dst_image_h.color = 0xee8899;

        if (ioctl(dev->g2d, G2D_CMD_BITBLT_H, &blt)< 0){
            LOGE("Error: G2D CMD BITBLT H failed");
        }
    }
    return 0;
}

int32_t GFXDestroySurface(GFXHANDLE surface) {
    FBSURFACE*surf=(FBSURFACE*)surface;
    FBDEVICE*dev=devs+surf->dispid;
    if(surf->used && (surf->kbuffer==NULL) && (surf->ishw==0)){
        free(surf->buffer);
        surf->buffer = NULL;
    }
    surf->used = 0;
    return 0;
}
