#include <cdtypes.h>
#include <cdgraph.h>
#include <cdlog.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <string.h>
#include <linux/input.h>
#include <cdinput.h>

#include <xf86drm.h>
#include <xf86drmMode.h>

typedef struct drm_device {
	uint32_t width;			                // 显示器的宽的像素点数量
	uint32_t height;		                // 显示器的高的像素点数量
	uint32_t pitch;			                // 每行占据的字节数
	uint32_t handle;		                // drm_mode_create_dumb的返回句柄
	uint32_t size;			                // 显示器占据的总字节数
	uint32_t *vaddr;		                // mmap的首地址
	uint32_t fb_id;			                // 创建的framebuffer的id号
	struct drm_mode_create_dumb create ;	// 创建的dumb
 	struct drm_mode_map_dumb map;			// 内存映射结构体
} DRMDEVICE;

typedef struct property_crtc {
	uint32_t blob_id;                       // blob（大对象） 的 ID | 显示模式的详细信息
	uint32_t property_crtc_id;              // CRTC 相关的属性的 ID
	uint32_t property_mode_id;              // 模式的属性 ID | 输出的分辨率和刷新率等参数
	uint32_t property_active;               // 是否激活  | 1表示激活，0表示未激活
} CRTCINFO;

drmModeConnector *conn;	     //connetor相关的结构体
drmModeRes *res;		     //资源
drmModePlaneRes *plane_res;  //plane资源

int fd;					     //文件描述符
uint32_t conn_id;            //connector的ID
uint32_t crtc_id;            //crtc的ID
uint32_t plane_id[3];        //plane的ID

DRMDEVICE buf;
CRTCINFO pc;

static GFXRect screenMargin = {0};

static int drm_create_fb(struct drm_device *bo) {
	/* create a dumb-buffer, the pixel format is XRGB888 */
	bo->create.width = bo->width;
	bo->create.height = bo->height;
	bo->create.bpp = 32;

	/* handle, pitch, size will be returned */
	drmIoctl(fd, DRM_IOCTL_MODE_CREATE_DUMB, &bo->create);

	/* bind the dumb-buffer to an FB object */
	bo->pitch = bo->create.pitch;
	bo->size = bo->create.size;
	bo->handle = bo->create.handle;
	drmModeAddFB(fd, bo->width, bo->height, 24, 32, bo->pitch,
			   bo->handle, &bo->fb_id);
	
	//每行占用字节数，共占用字节数，MAP_DUMB的句柄
	printf("pitch = %d ,size = %d, handle = %d \n",bo->pitch,bo->size,bo->handle);

	/* map the dumb-buffer to userspace */
	bo->map.handle = bo->create.handle;
	drmIoctl(fd, DRM_IOCTL_MODE_MAP_DUMB, &bo->map);

	bo->vaddr = mmap(0, bo->create.size, PROT_READ | PROT_WRITE,
			MAP_SHARED, fd, bo->map.offset);

	/* initialize the dumb-buffer with white-color */
	memset(bo->vaddr, 0xff,bo->size);

	return 0;
}

static void drm_destroy_fb(struct drm_device *bo) {
	struct drm_mode_destroy_dumb destroy = {};
	drmModeRmFB(fd, bo->fb_id);
	munmap(bo->vaddr, bo->size);
	destroy.handle = bo->handle;
	drmIoctl(fd, DRM_IOCTL_MODE_DESTROY_DUMB, &destroy);
}

static uint32_t get_property(int fd, drmModeObjectProperties *props)  {
	drmModePropertyPtr property;
	uint32_t i, id = 0;

	for (i = 0; i < props->count_props; i++) {
		property = drmModeGetProperty(fd, props->props[i]);
		printf("\"%s\"\t\t---",property->name);
		printf("id = %d , value=%ld\n",props->props[i],props->prop_values[i]);
	}
    return 0;
}

static uint32_t get_property_id(int fd, drmModeObjectProperties *props, const char *name) {
	drmModePropertyPtr property;
	uint32_t i, id = 0;

	/* find property according to the name */
	for (i = 0; i < props->count_props; i++) {
		property = drmModeGetProperty(fd, props->props[i]);
		if (!strcmp(property->name, name))
			id = property->prop_id;
		drmModeFreeProperty(property);

		if (id)
			break;
	}

	return id;
}

int32_t GFXInit() {
	drmModeObjectProperties *props;
	drmModeAtomicReq *req;

#ifdef __USE_XOPEN2K8
	fd = open("/dev/dri/card0", O_RDWR | O_CLOEXEC);
#else
    fd = open("/dev/dri/card0", O_RDWR);
#endif

	res = drmModeGetResources(fd);
	crtc_id = res->crtcs[0];
	conn_id = res->connectors[0];

	drmSetClientCap(fd, DRM_CLIENT_CAP_UNIVERSAL_PLANES, 1);
	plane_res = drmModeGetPlaneResources(fd);
	for(int i=0;i<3;i++){
		plane_id[i] = plane_res->planes[i];
		printf("planes[%d]= %d\n",i,plane_id[i]);
	}

	conn = drmModeGetConnector(fd, conn_id);
	buf.width = conn->modes[0].hdisplay;
	buf.height = conn->modes[0].vdisplay;
	drm_create_fb(&buf);

	drmSetClientCap(fd, DRM_CLIENT_CAP_ATOMIC, 1);

	/* get connector properties */
	props = drmModeObjectGetProperties(fd, conn_id,	DRM_MODE_OBJECT_CONNECTOR);
	printf("/-----conn_Property-----/\n");
	get_property(fd, props);
	printf("\n");
	pc.property_crtc_id = get_property_id(fd, props, "CRTC_ID");
	drmModeFreeObjectProperties(props);

	/* get crtc properties */
	props = drmModeObjectGetProperties(fd, crtc_id, DRM_MODE_OBJECT_CRTC);
	printf("/-----CRTC_Property-----/\n");
	get_property(fd, props);
	printf("\n");
	pc.property_active = get_property_id(fd, props, "ACTIVE");
	pc.property_mode_id = get_property_id(fd, props, "MODE_ID");
	drmModeFreeObjectProperties(props);

	/* create blob to store current mode, and retun the blob id */
	drmModeCreatePropertyBlob(fd, &conn->modes[0],
				sizeof(conn->modes[0]), &pc.blob_id);

	/* start modeseting */
	req = drmModeAtomicAlloc();
	drmModeAtomicAddProperty(req, crtc_id, pc.property_active, 1);
	drmModeAtomicAddProperty(req, crtc_id, pc.property_mode_id, pc.blob_id);
	drmModeAtomicAddProperty(req, conn_id, pc.property_crtc_id, crtc_id);
	drmModeAtomicCommit(fd, req, DRM_MODE_ATOMIC_ALLOW_MODESET, NULL);
	drmModeAtomicFree(req);

    //1：1设置屏幕，没有该函数不会显示画面
	drmModeSetPlane(fd, plane_id[0], crtc_id, buf.fb_id, 0,
			0, 0, buf.width, buf.height,
			0 << 16, 0 << 16, buf.width << 16, buf.height << 16);

    LOGI("fb=%d smem.start=%p size=%d pitch=%d",fd,(char*)buf.vaddr,buf.size,buf.pitch);

    return E_OK;
}

int32_t GFXGetDisplayCount() {
    return 1;
}

int32_t GFXGetDisplaySize(int dispid,uint32_t*width,uint32_t*height) {
    if(dispid<0||dispid>=GFXGetDisplayCount())return E_ERROR;
    *width =buf.width;
    *height=buf.height;
    LOGV("screen[%d]size=%dx%d/%dx%d",dispid,*width,*height,dev->var.xres,dev->var.yres);
    return E_OK;
}


int32_t GFXLockSurface(GFXHANDLE surface,void**buffer,uint32_t*pitch) {
    DRMDEVICE*ngs=(DRMDEVICE*)surface;
    *buffer=ngs->vaddr;
    *pitch=ngs->pitch;
    return 0;
}

int32_t GFXGetSurfaceInfo(GFXHANDLE surface,uint32_t*width,uint32_t*height,int32_t *format) {
    DRMDEVICE*ngs=(DRMDEVICE*)surface;
    *width = ngs->width;
    *height= ngs->height;
    // *format= ngs->format;
    return E_OK;
}

int32_t GFXUnlockSurface(GFXHANDLE surface) {
    return 0;
}

int32_t GFXSurfaceSetOpacity(GFXHANDLE surface,uint8_t alpha) {
    return 0;//dispLayer->SetOpacity(dispLayer,alpha);
}

int32_t GFXFillRect(GFXHANDLE surface,const GFXRect*rect,uint32_t color) {
    DRMDEVICE*ngs=(DRMDEVICE*)surface;

    return E_OK;
}

int32_t GFXFlip(GFXHANDLE surface) {
    DRMDEVICE*ngs=(DRMDEVICE*)surface;

    return 0;
}

static DRMDEVICE*getFreeSurface(){
    // for(int i=0;i<sizeof(devSurfaces)/sizeof(DRMDEVICE);i++){
    //     if(!devSurfaces[i].used){
    //         devSurfaces[i].used++;
    //         return devSurfaces+i;
    //     }
    // }
    return NULL;
}

int32_t GFXCreateSurface(int dispid,GFXHANDLE*surface,uint32_t width,uint32_t height,int32_t format,bool hwsurface) {
    // FBSURFACE*surf = getFreeSurface();
    // FBDEVICE*dev = &devs[dispid];
    // surf->dispid= dispid;
    // surf->width = hwsurface?dev->var.xres:width;
    // surf->height= hwsurface?dev->var.yres:height;
    // surf->format= format;
    // surf->ishw = hwsurface;
    // surf->pitch = width*4;
    // size_t buffer_size=surf->height*surf->pitch;
    // if(hwsurface) {
    //     setfbinfo(surf);
    //     dev->var.yoffset = 0;
    //     ioctl(dev->fb,FBIOPAN_DISPLAY,&dev->var);
    // } else {
    //     if(surf->kbuffer==0){
    //         surf->buffer = (char*)malloc(buffer_size);
	// }
    // }
    // surf->ishw=hwsurface;
    // LOGD("surface=%x buf=%p/%p size=%dx%d hw=%d",surf,surf->kbuffer,surf->buffer,width,height,hwsurface);
    // *surface = surf;
    return E_OK;
}


int32_t GFXBlit(GFXHANDLE dstsurface,int dx,int dy,GFXHANDLE srcsurface,const GFXRect*srcrect) {
    unsigned int x, y;
    DRMDEVICE *ndst = (DRMDEVICE *)dstsurface;
    DRMDEVICE *nsrc = (DRMDEVICE *)srcsurface;
    GFXRect rs = {0, 0};
    uint8_t *pbs = (uint8_t *)nsrc->vaddr;
    uint8_t *pbd = (uint8_t *)ndst->vaddr;
    rs.w = nsrc->width;
    rs.h = nsrc->height;
    if (srcrect) rs = *srcrect;
    if (((int)rs.w + dx <= 0) || ((int)rs.h + dy <= 0) || (dx >= (int)ndst->width) || (dy >= (int)ndst->height) || (rs.x < 0) || (rs.y < 0)) {
        LOGV("dx=%d,dy=%d rs=(%d,%d-%d,%d)", dx, dy, rs.x, rs.y, rs.w, rs.h);
        return E_INVALID_PARA;
    }

    LOGV("Blit %p[%dx%d] %d,%d-%d,%d -> %p[%dx%d] %d,%d", nsrc, nsrc->width, nsrc->height,
         rs.x, rs.y, rs.w, rs.h, ndst, ndst->width, ndst->height, dx, dy);
    if (dx < 0) {
        rs.x -= dx;
        rs.w = (int)rs.w + dx;
        dx = 0;
    }
    if (dy < 0) {
        rs.y -= dy;
        rs.h = (int)rs.h + dy;
        dy = 0;
    }
    if (dx + rs.w > ndst->width - screenMargin.x - screenMargin.w)
        rs.w = ndst->width - screenMargin.x - screenMargin.w - dx;
    if (dy + rs.h > ndst->height - screenMargin.y - screenMargin.h)
        rs.h = ndst->height - screenMargin.y - screenMargin.h - dy;

    LOGV("Blit %p %d,%d-%d,%d -> %p %d,%d buffer=%p->%p", nsrc, rs.x, rs.y, rs.w, rs.h, ndst, dx, dy, pbs, pbd);
    pbs += rs.y * nsrc->pitch + rs.x * 4;
    pbd += dy * ndst->pitch + dx * 4;

    // DRM specific blit operation
    for (y = 0; y < rs.h; y++) {
        memcpy(pbd, pbs, rs.w * 4);
        pbs += nsrc->pitch;
        pbd += ndst->pitch;
    }

    drmModePageFlip(fd, crtc_id, buf.fb_id, DRM_MODE_PAGE_FLIP_EVENT, NULL);
    return E_OK;
}

int32_t GFXDestroySurface(GFXHANDLE surface) {
    // drm_destroy_fb();
    return E_OK;
}
