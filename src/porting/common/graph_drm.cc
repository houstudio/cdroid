#include "gfxdrm.h"
#include "cdgraph.h"
#include "cdtypes.h"
#include "cdlog.h"
#include <xf86drm.h>
#include <xf86drmMode.h>

static GFXDrm *drm=nullptr;//("/dev/dri/card0");
static int drmFD=-1;
static drmModeConnector *drmConn;
static drmModeRes *drmModeres;
static uint32_t conn_id;
static uint32_t crtc_id;

typedef struct buffer_object {
    int32_t width;
    uint32_t height;
    uint32_t pitch;
    uint32_t handle;
    uint32_t size;
    uint32_t *vaddr;
    uint32_t fb_id;
}SURFACE;

static int modeset_create_fb(int fd, struct buffer_object *bo, uint32_t color){
    struct drm_mode_create_dumb create = {};
    struct drm_mode_map_dumb map = {};
    uint32_t i;

    create.width = bo->width;
    create.height = bo->height;
    create.bpp = 32;
    drmIoctl(fd, DRM_IOCTL_MODE_CREATE_DUMB, &create);

    bo->pitch = create.pitch;
    bo->size = create.size;
    bo->handle = create.handle;
    drmModeAddFB(fd, bo->width, bo->height, 24, 32, bo->pitch, bo->handle, &bo->fb_id);
    map.handle = create.handle;
    drmIoctl(fd, DRM_IOCTL_MODE_MAP_DUMB, &map);
    bo->vaddr = (uint32_t*)mmap(0, create.size, PROT_READ | PROT_WRITE,MAP_SHARED, fd, map.offset);
    //printf("createfb %dx%dx%d=%p\r\n",create.width,create.height,create.pitch,bo->vaddr);
    for (i = 0; i < (bo->size / 4); i++)
	bo->vaddr[i] = color;
    return 0;
}
static void modeset_destroy_fb(int fd, struct buffer_object *bo){
    struct drm_mode_destroy_dumb destroy = {};
    drmModeRmFB(fd, bo->fb_id);
    munmap(bo->vaddr, bo->size);
    destroy.handle = bo->handle;
    drmIoctl(fd, DRM_IOCTL_MODE_DESTROY_DUMB, &destroy);
}

INT GFXInit() {
   if(drmFD>0)return 0;
    drmFD = open("/dev/dri/card0", O_RDWR | O_CLOEXEC);
    drmModeres = drmModeGetResources(drmFD);
    crtc_id = drmModeres->crtcs[0];
    conn_id = drmModeres->connectors[0];
    drmConn = drmModeGetConnector(drmFD, conn_id);
#if 0
    buffer_object buf;
    buf.width = drmConn->modes[0].hdisplay;
    buf.height= drmConn->modes[0].vdisplay;
    modeset_create_fb(drmFD, &buf, 0x0000ff);
    int ret1=drmModeSetCrtc(drmFD,crtc_id,buf.fb_id,0,0,&conn_id,1,&drmConn->modes[0]);
    int ret2=drmModePageFlip(drmFD,crtc_id, buf.fb_id,DRM_MODE_PAGE_FLIP_EVENT, &crtc_id);
    printf("buff.fb_id=%d drmModeSetCrtc=%d,%d\n",buf.fb_id,ret1,ret2);
    sleep(1);
#endif
    return 0;
}

INT GFXGetDisplaySize(int dispid,UINT*width,UINT*height) {
    if(width)*width=drmConn->modes[0].hdisplay;
    if(height)*height=drmConn->modes[0].vdisplay;
    //LOGD("screensize=%dx%d",*width,*height);
    return 0;
}

INT GFXGetDisplayCount(){
    return 1;
}

INT GFXLockSurface(HANDLE surface,void**buffer,UINT*pitch) {
    SURFACE*gfx=(SURFACE*)surface;
    *buffer=gfx->vaddr;
    *pitch=gfx->pitch;
    return 0;
}

INT GFXGetSurfaceInfo(HANDLE surface,UINT*width,UINT*height,INT *format) {
     SURFACE*gfx=(SURFACE*)surface;
     *width=gfx->width;
     *height=gfx->height;
    return 0;
}

INT GFXUnlockSurface(HANDLE surface) {
    return 0;
}

INT GFXSurfaceSetOpacity(HANDLE surface,BYTE alpha) {
    return 0;
}

INT GFXFillRect(HANDLE surface,const GFXRect*rect,UINT color) {
    SURFACE*ngs=(SURFACE*)surface;
    UINT x,y;
    GFXRect rec= {0,0,0,0};
    rec.w=ngs->width;
    rec.h=ngs->height;
    if(rect)rec=*rect;
    LOGV("FillRect %p %d,%d-%d,%d color=0x%x pitch=%d",ngs,rec.x,rec.y,rec.w,rec.h,color,ngs->pitch);
    UINT*fb=(UINT*)(ngs->vaddr+ngs->pitch*rec.y+rec.x*4);
    UINT*fbtop=fb;
    for(x=0; x<rec.w; x++)fb[x]=color;
    const int cpw=rec.w*4;
    long copied=0;
    for(y=1; y<rec.h; y++) {
        fb+=(ngs->pitch>>2);
        memcpy(fb,fbtop,cpw);
        copied+=ngs->pitch;
    }
    return 0;
}

INT GFXFlip(HANDLE surface) {
    SURFACE*gfx=(SURFACE*)surface;
    int ret= drmModePageFlip(drmFD,crtc_id,gfx->fb_id,DRM_MODE_PAGE_FLIP_EVENT, &crtc_id);
    LOGV("drmModePageFlip=%d",ret);
    return 0;
}

INT GFXCreateSurface(int,HANDLE*surface,UINT width,UINT height,INT format,BOOL hwsurface) {
    SURFACE*gfx=(SURFACE*)malloc(sizeof(SURFACE));
    gfx->width = width;
    gfx->height= height;
    modeset_create_fb(drmFD,gfx,0x0000FF);
    *surface=gfx;
    LOGD("surface %p size=%dx%dx%d buffer=%p fb_id=%d hw=%d",gfx,width,height,gfx->pitch,gfx->vaddr,gfx->fb_id,hwsurface);
    if(hwsurface){
        int ret=drmModeSetCrtc(drmFD,crtc_id,gfx->fb_id,0,0,&conn_id,1,&drmConn->modes[0]);
        int ret1=drmModePageFlip(drmFD,crtc_id,gfx->fb_id,DRM_MODE_PAGE_FLIP_EVENT, &crtc_id);
        LOGD("drmModeSetCrtc=%d %d",ret,ret1);
    }
    return 0;
}

INT GFXBlit(HANDLE dstsurface,int dx,int dy,HANDLE srcsurface,const GFXRect*srcrect) {
    SURFACE*ndst=(SURFACE*)dstsurface;
    SURFACE*nsrc=(SURFACE*)srcsurface;
    GFXRect rs= {0,0};
    BYTE*pbs=(BYTE*)nsrc->vaddr;
    BYTE*pbd=(BYTE*)ndst->vaddr;
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
    const int cpw=rs.w*4;
    for(int y=0; y<rs.h; y++) {
        memcpy(pbd,pbs,cpw);
        pbs+=nsrc->pitch;
        pbd+=ndst->pitch;
    } 
    return 0;
}

INT GFXDestroySurface(HANDLE surface) {
    SURFACE*gfx=(SURFACE*)surface;
    modeset_destroy_fb(drmFD,gfx);
    free(gfx);
    return 0;
}

