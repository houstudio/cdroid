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

#include <xf86drm.h>
#include <xf86drmMode.h>

//#define USE_PIXMAN 1
#ifdef USE_PIXMAN
#include <pixman.h>
#endif

typedef struct {
    int fb;
    int drm;/*sor cursor layer*/
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

drmModeConnector *conn;	     //connetor相关的结构体
drmModeRes *res;		     //资源
drmModePlaneRes *plane_res;  //plane资源

uint32_t conn_id;            //connector的ID
uint32_t crtc_id;            //crtc的ID
uint32_t plane_id[3];        //plane的ID

static int drm_create_fb(struct drm_device *bo)
{
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

void drm_init(FBDEVICE* dev) {
#ifdef __USE_XOPEN2K8
	dev->drm = open("/dev/dri/card0", O_RDWR | O_CLOEXEC);
#else
    dev->drm = open("/dev/dri/card0", O_RDWR);
#endif
	drmModeObjectProperties *props;
	drmModeAtomicReq *req;

	res = drmModeGetResources(dev->drm);
	crtc_id = res->crtcs[0];
	conn_id = res->connectors[0];
    drmSetClientCap(dev->drm, DRM_CLIENT_CAP_UNIVERSAL_PLANES, 1);
	plane_res = drmModeGetPlaneResources(dev->drm);
	for(int i = 0;i < 3;i++){
		plane_id[i] = plane_res->planes[i];
		printf("planes[%d]= %d\n",i,plane_id[i]);
	}

	conn = drmModeGetConnector(dev->drm, conn_id);
	devSurfaces[0].width = conn->modes[0].hdisplay;
	devSurfaces[0].height = conn->modes[0].vdisplay;
	drm_create_fb(&buf);

	drmSetClientCap(dev->drm, DRM_CLIENT_CAP_ATOMIC, 1);

	/* get connector properties */
	props = drmModeObjectGetProperties(dev->drm, conn_id,	DRM_MODE_OBJECT_CONNECTOR);
	printf("/-----conn_Property-----/\n");
	get_property(dev->drm, props);
	printf("\n");
	pc.property_crtc_id = get_property_id(dev->drm, props, "CRTC_ID");
	drmModeFreeObjectProperties(props);

	/* get crtc properties */
	props = drmModeObjectGetProperties(dev->drm, crtc_id, DRM_MODE_OBJECT_CRTC);
	printf("/-----CRTC_Property-----/\n");
	get_property(dev->drm, props);
	printf("\n");
	pc.property_active = get_property_id(dev->drm, props, "ACTIVE");
	pc.property_mode_id = get_property_id(dev->drm, props, "MODE_ID");
	drmModeFreeObjectProperties(props);

	/* create blob to store current mode, and retun the blob id */
	drmModeCreatePropertyBlob(dev->drm, &conn->modes[0],
				sizeof(conn->modes[0]), &pc.blob_id);

	/* start modeseting */
	req = drmModeAtomicAlloc();
	drmModeAtomicAddProperty(req, crtc_id, pc.property_active, 1);
	drmModeAtomicAddProperty(req, crtc_id, pc.property_mode_id, pc.blob_id);
	drmModeAtomicAddProperty(req, conn_id, pc.property_crtc_id, crtc_id);
	drmModeAtomicCommit(dev->drm, req, DRM_MODE_ATOMIC_ALLOW_MODESET, NULL);
	drmModeAtomicFree(req);
}


int32_t GFXInit() {
    if(devs[0].fb>=0)return E_OK;
    bzero(devs,sizeof(devs));
    FBDEVICE*dev = &devs[0];

    drm_init(dev);




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

int32_t GFXCreateSurface(int dispid,GFXHANDLE*surface,uint32_t width,uint32_t height,int32_t format,bool hwsurface) {
    FBSURFACE*surf=(FBSURFACE*)malloc(sizeof(FBSURFACE));
    surf->dispid=dispid;
    surf->width=width;
    surf->height=height;
    surf->format=format;
    surf->ishw=hwsurface;
    surf->pitch=width*4;
    size_t buffer_size=surf->height*surf->pitch;
    FBDEVICE*dev=devs+dispid;
    if(hwsurface && devs[dispid].fix.smem_len) {
        size_t mem_len=((dev->fix.smem_start) -((dev->fix.smem_start) & ~(getpagesize() - 1)));
        buffer_size=surf->height*dev->fix.line_length;
        surf->buffer=(char*)mmap( NULL,buffer_size,PROT_READ | PROT_WRITE, MAP_SHARED,dev->fb, 0 );
        surf->pitch=dev->fix.line_length;
    } else {
        surf->buffer=(char*)malloc(buffer_size);
    }
    surf->ishw=hwsurface;
    LOGV("surface=%x buf=%p size=%dx%d hw=%d",surf,surf->buffer,width,height,hwsurface);
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
