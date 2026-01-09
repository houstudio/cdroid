#include "cdgraph.h"
#include "cdtypes.h"
#include "cdlog.h"
#include <signal.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <libdrm/drm_fourcc.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

static int drmFD=-1;
static drmModeConnector *drmConn;
static drmModeRes *drmModeres;
static uint32_t conn_id;
static uint32_t crtc_id;
static int terminate=0;

typedef struct buffer_object {
    int32_t width;
    uint32_t height;
    uint32_t pitch;
    uint32_t handle;
    uint32_t size;
    uint32_t *vaddr;
    uint32_t fb_id;
}SURFACE;
static SURFACE*primary;

static int modeset_create_fb(int fd, SURFACE *bo, uint32_t color){
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
    uint32_t handlers[4]={create.handle};
    uint32_t strides[4] ={bo->pitch};
    uint32_t offsets[4] ={0};
    int32_t added = drmModeAddFB2(fd, bo->width, bo->height, DRM_FORMAT_XRGB8888, handlers, strides, offsets, &bo->fb_id,0);
    LOGD("drmModeAddFB2=%d fbid=%d",added,bo->fb_id);
    map.handle = create.handle;
    drmIoctl(fd, DRM_IOCTL_MODE_MAP_DUMB, &map);
    bo->vaddr = (uint32_t*)mmap(0, create.size, PROT_READ | PROT_WRITE,MAP_SHARED, fd, map.offset);
    return 0;
}

static void modeset_destroy_fb(int fd, struct buffer_object *bo){
    struct drm_mode_destroy_dumb destroy = {};
    drmModeRmFB(fd, bo->fb_id);
    munmap(bo->vaddr, bo->size);
    destroy.handle = bo->handle;
    drmIoctl(fd, DRM_IOCTL_MODE_DESTROY_DUMB, &destroy);
}

static void sigint_handler(int arg){
    terminate = 1;
}

int32_t GFXInit() {
    if(drmFD>0)return 0;
    drmModePlaneResPtr plane_resources = nullptr;
    drmFD = open("/dev/dri/card0", O_RDWR | O_CLOEXEC);
    if(drmFD < 0){
        FATAL("drm open failed! ");
        exit(0);
    }
    drmModeres = drmModeGetResources(drmFD);
    crtc_id = drmModeres->crtcs[0];
    conn_id = drmModeres->connectors[0];
    drmConn = drmModeGetConnector(drmFD, conn_id);
    plane_resources = drmModeGetPlaneResources(drmFD);
    if(plane_resources){
        LOGI("Total number of planes found: %u\n", plane_resources->count_planes);
        for (uint32_t i = 0; i < plane_resources->count_planes; i++) {
            uint32_t plane_id = plane_resources->planes[i];
            LOGI("Plane ID [%u]: %u\n", i, plane_id);

            // 可选：获取更详细的 Plane 信息
            drmModePlanePtr plane_info = drmModeGetPlane(drmFD, plane_id);
            if (plane_info) {
                // ... 在这里可以查询 plane_info->possible_crtcs, plane_info->formats 等 ...
                LOGI("  - Possible CRTCs mask: 0x%x", plane_info->possible_crtcs);
                LOGI("  - Number of formats: %u", plane_info->count_formats);
                // 注意：plane_info->formats 是一个指向格式数组的指针，可能需要进一步查询
                drmModeFreePlane(plane_info); // 释放 plane_info
            } else {
                LOGE("  - drmModeGetPlane failed for this ID");
            }
        }
        drmModeFreePlaneResources(plane_resources);
    }
    //signal(SIGint32_t, sigint_handler);
    return 0;
}

int32_t GFXGetDisplaySize(int dispid,uint32_t*width,uint32_t*height) {
    if(width)*width = drmConn->modes[0].hdisplay;
    if(height)*height = drmConn->modes[0].vdisplay;
    //LOGD("screensize=%dx%d",*width,*height);
    return 0;
}

int32_t GFXGetDisplayCount(){
    return 1;
}

int32_t GFXLockSurface(GFXHANDLE surface,void**buffer,uint32_t*pitch) {
    SURFACE*gfx = (SURFACE*)surface;
    *buffer = gfx->vaddr;
    *pitch  = gfx->pitch;
    return 0;
}

int32_t GFXGetSurfaceInfo(GFXHANDLE surface,uint32_t*width,uint32_t*height,int32_t *format) {
     SURFACE*gfx = (SURFACE*)surface;
     *width = gfx->width;
     *height= gfx->height;
    return 0;
}

int32_t GFXUnlockSurface(GFXHANDLE surface) {
    return 0;
}

int32_t GFXSurfaceSetOpacity(GFXHANDLE surface,uint8_t alpha) {
    return 0;
}

int32_t GFXFillRect(GFXHANDLE surface,const GFXRect*rect,uint32_t color) {
    SURFACE*ngs = (SURFACE*)surface;
    uint32_t x,y;
    GFXRect rec= {0,0,0,0};
    rec.w = ngs->width;
    rec.h = ngs->height;
    if(rect)rec=*rect;
    LOGV("FillRect %p %d,%d-%d,%d color=0x%x pitch=%d",ngs,rec.x,rec.y,rec.w,rec.h,color,ngs->pitch);
    uint32_t*fb=(uint32_t*)(ngs->vaddr+ngs->pitch*rec.y+rec.x*4);
    uint32_t*fbtop=fb;
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

static void modeset_flip_handler(int fd, uint32_t frame,
              uint32_t sec, uint32_t usec, void *data){
    static int i = 0;
    uint32_t crtc_id = *(uint32_t *)data;
    i += 1;

    int ret = drmModePageFlip(drmFD, crtc_id, primary->fb_id,
	DRM_MODE_PAGE_FLIP_EVENT, data);
    LOGD_IF(i%200==0,"crtcid=%d time %d.%d flip=%d",crtc_id,sec,usec,ret);
}

int32_t GFXFlip(GFXHANDLE surface) {
    SURFACE*gfx = (SURFACE*)surface;
    /*drmEventContext ev = {};
    ev.version = DRM_EVENT_CONTEXT_VERSION;
    ev.page_flip_handler = modeset_flip_handler;
    while (!terminate) {
        drmHandleEvent(drmFD, &ev);
    }
    terminate=0;*/
    const int ret= drmModePageFlip(drmFD,crtc_id,gfx->fb_id,DRM_MODE_PAGE_FLIP_EVENT, &crtc_id);
    //LOGD("drmModePageFlip=%d crtc_id=%d",ret,crtc_id);
    return 0;
}

int32_t GFXCreateSurface(int,GFXHANDLE*surface,uint32_t width,uint32_t height,int32_t format,bool hwsurface) {
    SURFACE*gfx = (SURFACE*)malloc(sizeof(SURFACE));
    gfx->width = width;
    gfx->height= height;
    modeset_create_fb(drmFD,gfx,0x0000FF);
    *surface=gfx;
    LOGD("surface %p size=%dx%dx%d buffer=%p fb_id=%d hw=%d",gfx,width,height,gfx->pitch,gfx->vaddr,gfx->fb_id,hwsurface);
    if(hwsurface){
        int ret = drmModeSetCrtc(drmFD,crtc_id,gfx->fb_id,0,0,&conn_id,1,&drmConn->modes[0]);
        int ret1= drmModePageFlip(drmFD,crtc_id,gfx->fb_id,DRM_MODE_PAGE_FLIP_EVENT, &crtc_id);
        primary = gfx;
        LOGD("drmModeSetCrtc=%d %d crtc_id=%d",ret,ret1,crtc_id);
    }
    return 0;
}

int32_t GFXBlit(GFXHANDLE dstsurface,int dx,int dy,GFXHANDLE srcsurface,const GFXRect*srcrect) {
    SURFACE*ndst = (SURFACE*)dstsurface;
    SURFACE*nsrc = (SURFACE*)srcsurface;
    GFXRect rs= {0,0};
    uint8_t*pbs = (uint8_t*)nsrc->vaddr;
    uint8_t*pbd = (uint8_t*)ndst->vaddr;
    rs.w = nsrc->width;
    rs.h = nsrc->height;
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
    pbs += rs.y*nsrc->pitch+rs.x*4;
    pbd += dy*ndst->pitch+dx*4;
    const int cpw = rs.w*4;
    for(int y=0; y<rs.h; y++) {
        memcpy(pbd,pbs,cpw);
        pbs += nsrc->pitch;
        pbd += ndst->pitch;
    } 
    return 0;
}

int32_t GFXDestroySurface(GFXHANDLE surface) {
    SURFACE*gfx = (SURFACE*)surface;
    modeset_destroy_fb(drmFD,gfx);
    free(gfx);
    return 0;
}

