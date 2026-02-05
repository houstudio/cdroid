#include <cdtypes.h>
#include <cdgraph.h>
#include <cdlog.h>
#include <linux/fb.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <string.h>
#include <cdinput.h>
#include <memory.h>
#include <stdio.h>
#include "mi_common.h"
#include "mi_sys.h"
#include "mi_gfx.h"
#include "sstarFb.h"
typedef struct {
    int fb;
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
    int current;
    int alpha;
    size_t msize;
    int used;
    int hasLogo;
    char*buffer;//drawbuffer
    char*orig_buffer;/*used only in double buffer*/
    MI_PHY kbuffer;/*kernel buffer address,MI_PHY unsigned long long,nullptr for usermemory surface()*/
} FBSURFACE;

#define DOUBLE_BUFFER 0
#define MAX_HWSURFACE 4
static FBDEVICE devs[2]= {-1};
static FBSURFACE *primarySurface;
static FBSURFACE devSurfaces[16];
/*if screensize it not macthed the driver size,margins is required*/
static GFXRect screenMargin={0,0,0,0};

static void gfxexit() {
    LOGI("gfxexit");
    MI_GFX_Close();
    MI_SYS_Exit();
}

void GFXSuspend(){
    int ret1,ret2;
    ret1=MI_GFX_Close();
    ret2=MI_SYS_Exit();
    printf("prepared to suspend gfxclose=%d sysexit=%d\r\n",ret1,ret2);
    system("/customer/suspend.sh");
    ret1=MI_SYS_Init();
    ret2=MI_GFX_Open();
    printf("suspend end,we are wake up now. sysinit=%d,gfxopen=%d\r\nd",ret1,ret2);
}

static void* loadLogo(const char*fileName){
    #define __tmin__(a,b) ((a)>(b)?(b):(a))
    struct stat st;
    if (lstat(fileName,&st) == 0) {            
        size_t rlen, tlen = 0;
        FILE *fo  = fopen(fileName, "rb");
        void*buffer=malloc(st.st_size);
        while (tlen < st.st_size && (rlen = fread(buffer + tlen, 1, __tmin__(st.st_size - tlen, 4096), fo)) > 0) {
            tlen += rlen;
        }
        fclose(fo);
        LOGI("Logo buf=%p blen=%u tlen=%u", buffer,st.st_size, tlen);
        return buffer;
    }
    return NULL;
}
static void showLogo(FBSURFACE*dst,void*buffer){
    memcpy(dst->buffer,buffer,dst->height*dst->pitch);
}

int GFXInit() {
    int ret;
    char*logoBuffer,*strMargin;
    FBDEVICE*dev=&devs[0];
    if(dev->fb>=0)return E_OK;
    memset(devs,0,sizeof(devs));
    ret = MI_SYS_Init();
    logoBuffer = loadLogo("logo.dat");
    LOGI("SYS_Init=%d",ret);
    ret = MI_GFX_Open();
    LOGI("MI_GFX_Open=%d",ret);
    devs[0].fb = open("/dev/fb0", O_RDWR);
    // Get fixed screen information
    if(ioctl(dev->fb, FBIOGET_FSCREENINFO, &dev->fix) == -1) {
        LOGE("Error reading fixed information fd=%d",dev->fb);
        return E_ERROR;
    }
    atexit(gfxexit);
    LOGI("fb %d mem.addr=%x fbmem.size=%d pitch=%d",dev->fb,devs[0].fix.smem_start,dev->fix.smem_len,dev->fix.line_length);
    // Get variable screen information
    if(ioctl(dev->fb, FBIOGET_VSCREENINFO, &dev->var) == -1) {
        LOGE("Error reading variable information");
        return E_ERROR;
    }
    strMargin = getenv("SCREEN_MARGINS");
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
    devs[0].var.yoffset=0;//set first screen memory for display
    ret=ioctl(dev->fb,FBIOPUT_VSCREENINFO,&dev->var);
    LOGI("FBIOPUT_VSCREENINFO=%d",ret);
    LOGI("fb solution=%dx%d accel_flags=0x%x ScreenMargin=(%d,%d,%d,%d)[%s]",dev->var.xres,dev->var.yres,
        dev->var.accel_flags, screenMargin.x,screenMargin.y,screenMargin.w,screenMargin.h,strMargin);
    memset(devSurfaces,0,sizeof(devSurfaces));

    const size_t displayScreenSize=(dev->var.yres * dev->fix.line_length);
    const size_t screenSize = (dev->var.yres - screenMargin.y - screenMargin.h) * (dev->fix.line_length - (screenMargin.x + screenMargin.w)*4);
    const size_t allocedSize = displayScreenSize + screenSize * MAX_HWSURFACE;

    MI_PHY preallocedMem;
    void* preallocedVMem;
    ret = MI_SYS_MMA_Alloc("mma_heap_name0",allocedSize,&preallocedMem);

    MI_SYS_Mmap(preallocedMem,allocedSize, (void**)&preallocedVMem, FALSE);
    devSurfaces[0].kbuffer= devs[0].fix.smem_start;
    devSurfaces[0].width = dev->var.xres;
    devSurfaces[0].height = dev->var.yres;
    devSurfaces[0].pitch = dev->fix.line_length;
#if defined(DOUBLE_BUFFER)&&DOUBLE_BUFFER
    devSurfaces[0].msize = dev->fix.smem_len;
#else
    devSurfaces[0].msize = displayScreenSize;
#endif
    const int isFBmemNotInAllocedMemRange =(devs[0].fix.smem_start+displayScreenSize<preallocedMem)
         ||(devs[0].fix.smem_start>preallocedMem+allocedSize);
    LOGI("fbmem %x is %s in prealocted memory's range(%llx,%llx)",devs[0].fix.smem_start,
	     (isFBmemNotInAllocedMemRange?"not":""),preallocedMem,preallocedMem+allocedSize);
    if(isFBmemNotInAllocedMemRange){
        for(int i=0;i<MAX_HWSURFACE+1;i++){
            devSurfaces[i+1].kbuffer= preallocedMem+screenSize*i;
            devSurfaces[i+1].buffer= preallocedVMem+screenSize*i;
            devSurfaces[i+1].msize = screenSize;
        }
    }else{
        LOGI("fbmem %x is in preallocted memory range(%llx,%llx)",devs[0].fix.smem_start,preallocedMem,preallocedMem+allocedSize);
        MI_PHY pmem= preallocedMem;
        void*vmem  = preallocedVMem;
        for(int i=0; pmem < preallocedMem +allocedSize;i++){
            devSurfaces[i].kbuffer = pmem;
            devSurfaces[i].msize = screenSize;
            devSurfaces[i].buffer= vmem;
            const int isfbmem=(pmem!=devs[0].fix.smem_start);
            vmem+=isfbmem?displayScreenSize:screenSize;
            pmem+=isfbmem?displayScreenSize:screenSize;
            LOGI("[%d]mem=%llx,%p",i,pmem,vmem);
        }
    }
    //devSurfaces[0].buffer=(char*)mmap( dev->fix.smem_start,dev->fix.smem_len,PROT_READ | PROT_WRITE, MAP_SHARED,dev->fb, 0 );
    MI_SYS_Mmap(dev->fix.smem_start,dev->fix.smem_len, (void**)&devSurfaces[0].buffer, FALSE);
    if(logoBuffer){
        showLogo(&devSurfaces[0],logoBuffer);
        devSurfaces[0].hasLogo=1;
        free(logoBuffer);
    }

    for(int i =0;devSurfaces[i].kbuffer;i++){
        LOGI("Surface[%d]buffer=%llx/%p %d",i,devSurfaces[i].kbuffer,devSurfaces[i].buffer,devSurfaces[i].msize);
    }
#if defined(DOUBLE_BUFFER)&&DOUBLE_BUFFER
    MI_SYS_MemcpyPa(devSurfaces[0].kbuffer+screenSize,devSurfaces[0].kbuffer,screenSize);
#endif
    return E_OK;
}

int32_t GFXGetDisplayCount() {
    return 1;
}

int32_t GFXGetDisplaySize(int dispid,uint32_t*width,uint32_t*height) {
    if(dispid<0||dispid>=GFXGetDisplayCount())
        return E_ERROR;
    LOGI_IF(width==NULL||height==NULL,"Params Error");
    FBDEVICE*dev=&devs[dispid];
    *width =dev->var.xres - screenMargin.x - screenMargin.w;
    *height=dev->var.yres - screenMargin.y - screenMargin.h;
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
    return E_OK;
}

int32_t GFXSurfaceSetOpacity(GFXHANDLE surface,uint8_t alpha) {
    FBSURFACE*ngs=(FBSURFACE*)surface;
    MI_FB_GlobalAlpha_t fba;//MI_FB_API.pdf
    fba.bAlphaEnable = 1;
    fba.bAlphaChannel= 0;
    fba.u8Alpha0 = 255;
    fba.u8Alpha1 = 255;
    fba.u8GlobalAlpha = alpha;
    if(ngs)ngs->alpha=alpha;
    else ioctl (devs[0].fb,FBIOSET_GLOBAL_ALPHA,&fba);
    return E_OK;//dispLayer->SetOpacity(dispLayer,alpha);
}

static void toMIGFX(const FBSURFACE*fb,MI_GFX_Surface_t*gfx) {
    gfx->eColorFmt= E_MI_GFX_FMT_ARGB8888;
    gfx->u32Width = fb->width;
    gfx->u32Height= fb->height;
    gfx->u32Stride= fb->pitch;
    gfx->phyAddr  = fb->kbuffer;
}

int32_t GFXFillRect(GFXHANDLE surface,const GFXRect*rect,uint32_t color) {
    FBSURFACE*ngs=(FBSURFACE*)surface;
    uint32_t x,y;
    MI_S32 ret;
    MI_SYS_FrameData_t dt;
    GFXRect rec= {0,0,0,0};
    rec.w=ngs->width;
    rec.h=ngs->height;
    if(rect)rec=*rect;
    if(ngs->kbuffer) {
        MI_U16 fence;
        MI_GFX_Surface_t gfxsurf;
        MI_GFX_Rect_t mirec;
        toMIGFX(ngs,&gfxsurf);
        mirec.s32Xpos = rec.x;
        mirec.s32Ypos = rec.y;
        mirec.u32Width= rec.w;
        mirec.u32Height=rec.h;
        ret=MI_GFX_QuickFill(&gfxsurf,&mirec,color,&fence);
        MI_GFX_WaitAllDone(FALSE,fence);
        LOGV("Fill(%d,%d,%d,%d) with %x",rec.x,rec.y,rec.w,rec.h,color);
    } else {
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
    LOGV("FillRect %p %d,%d-%d,%d color=0x%x pitch=%d ret=%d",ngs,rec.x,rec.y,rec.w,rec.h,color,ngs->pitch,ret);
    return E_OK;
}

#define DMAFLIP 1//uncomment this line to use GFXFLIP

int32_t GFXFlip(GFXHANDLE surface) {
    FBSURFACE*surf=(FBSURFACE*)surface;
    const size_t screen_size=surf->height*surf->pitch;
    if(surf->ishw && (surf->msize>screen_size) ) {
        FBDEVICE*dev=&devs[surf->dispid];
        FBSURFACE dstSurf=*surf;
        int sync = ioctl(dev->fb, FBIO_WAITFORVSYNC, NULL);
        if(surf->current==0) {
            LOGI_IF(dev->fix.smem_start!=surf->kbuffer,"kbuffer error1");
            dstSurf.kbuffer=surf->kbuffer+screen_size;
#if defined(DMAFLIP)&&DMAFLIP
            MI_SYS_MemcpyPa(surf->kbuffer+screen_size,surf->kbuffer,screen_size);
#else
            GFXBlit(&dstSurf,0,0,surf,NULL);
#endif
            surf->buffer = surf->orig_buffer+screen_size;
            surf->kbuffer+=screen_size;
            dev->var.yoffset = 0;
        } else {
            LOGI_IF(dev->fix.smem_start!=surf->kbuffer-screen_size,"kbuffer error2");
            dstSurf.kbuffer=surf->kbuffer-screen_size;
#if defined(DMAFLIP)&&DMAFLIP
            MI_SYS_MemcpyPa(surf->kbuffer-screen_size,surf->kbuffer,screen_size);
#else
            GFXBlit(&dstSurf,0,0,surf,NULL);
#endif
            surf->buffer = surf->orig_buffer;
            dev->var.yoffset = dev->var.yres;
            surf->kbuffer-=screen_size;
        }
        surf->current=(surf->current+1)%2;
        int ret = ioctl(dev->fb, FBIOPAN_DISPLAY, &dev->var);
        LOGI_IF(ret<0||sync<0,"FBIOPAN_DISPLAY=%d yoffset=%d res=%dx%d dev=%p fb=%d,sync=%d",ret,dev->var.yoffset,dev->var.xres,dev->var.yres,dev,dev->fb,sync);
    }
    return 0;
}

static int setfbinfo(FBSURFACE*surf) {
    int rc=-1;
    FBDEVICE*dev=&devs[surf->dispid];
    struct fb_var_screeninfo*v = &dev->var;
    v->bits_per_pixel = 32;
    switch(surf->format) {
    case GPF_ARGB:
        v->transp.offset=24;  v->transp.length=8;
        v->red.offset=16;     v->red.length=8;
        v->green.offset=8;    v->green.length=8;
        v->blue.offset=0;     v->blue.length=8;
        break;
    case GPF_ABGR:
        v->transp.offset=24;  v->transp.length=8;
        v->blue.offset=16;    v->blue.length=8;
        v->green.offset=8;    v->green.length=8;
        v->red.offset=0;      v->red.length=8;
        break;
    default:
        break;
    }
    rc=ioctl(dev->fb,FBIOPUT_VSCREENINFO,v);
    LOGD("FBIOPUT_VSCREENINFO=%d",rc);
    return rc;
}

static FBSURFACE*getFreeFace(){
    for(int i=0;i<sizeof(devSurfaces)/sizeof(FBSURFACE);i++){
        if(!devSurfaces[i].used){
            devSurfaces[i].used++;
            return devSurfaces+i;
        }
    }
    return NULL;
}

int32_t GFXCreateSurface(int dispid,GFXHANDLE*surface,uint32_t width,uint32_t height,int32_t format,bool hwsurface) {
    FBSURFACE*surf=getFreeFace();
    FBDEVICE*dev = &devs[dispid];
    surf->dispid = dispid;
    surf->width  = hwsurface?dev->var.xres:width;
    surf->height = hwsurface?dev->var.yres:height;
    surf->format = format;
    surf->ishw = hwsurface;
    surf->pitch= surf->width*4;
    surf->current = 0;
#if !defined(DOUBLE_BUFFER)||(DOUBLE_BUFFER==0)    
    surf->msize= surf->pitch*surf->height;
#endif
    MI_S32 ret=0;
    if(hwsurface) {
#if !defined(DOUBLE_BUFFER)||(DOUBLE_BUFFER==0)
        dev->var.yoffset=0;
        LOGI("ioctl offset(0)=%d dev=%p ret=%d",ioctl(dev->fb,FBIOPAN_DISPLAY,&dev->var),dev,ret);
#else// DOUBLE_BUFFER
        dev->var.yoffset = dev->var.yres;
        LOGI("ioctl offset(0)=%d dev=%p",ioctl(dev->fb,FBIOPAN_DISPLAY,&dev->var),dev);
#endif
        primarySurface = surf;
    } else {
        if(surf->kbuffer==0){
            surf->buffer = (char*)malloc(surf->msize);
            memset(surf->buffer,0,surf->msize);
        }
    }
    if(surf->kbuffer&&(surf->hasLogo==0)) MI_SYS_MemsetPa(surf->kbuffer,0xFF000000,surf->msize);
    surf->orig_buffer = surf->buffer;
    if(hwsurface) setfbinfo(surf);
    surf->ishw = hwsurface;
    surf->alpha= 255;
    LOGI("Surface=%p buf=%llx/%p size=%dx%d/%d hw=%d",surf,surf->kbuffer,surf->buffer,surf->width,surf->height,surf->msize,hwsurface);
    *surface=surf;
    return E_OK;
}

#define MIN(x,y) ((x)>(y)?(y):(x))
int32_t GFXBlit(GFXHANDLE dstsurface,int dx,int dy,GFXHANDLE srcsurface,const GFXRect*srcrect) {
    unsigned int x,y,sw,sh;
    FBSURFACE*ndst=(FBSURFACE*)dstsurface;
    FBSURFACE*nsrc=(FBSURFACE*)srcsurface;
    GFXRect rs= {0,0};
    uint8_t*pbs=(uint8_t*)nsrc->buffer;
    uint8_t*pbd=(uint8_t*)ndst->buffer;
    int ret;
    MI_U16 fence;
    MI_GFX_Opt_t opt;
    MI_GFX_Surface_t gfxsrc,gfxdst;
    MI_GFX_Rect_t stSrcRect, stDstRect;

    rs.w = nsrc->width;
    rs.h = nsrc->height;
    if(srcrect)rs=*srcrect;

    LOGD_IF(ndst!=primarySurface&&ndst->ishw,"dst is not primarySurface");
    if(((int)rs.w+dx<=0)||((int)rs.h+dy<=0)||(dx>=(int)ndst->width)||(dy>=(int)ndst->height)||(rs.x<0)||(rs.y<0)){
        LOGD("dx=%d,dy=%d rs=(%d,%d-%d,%d)",dx,dy,rs.x,rs.y,rs.w,rs.h);
        return E_INVALID_PARA;
    }

    LOGV("Blit %p(%d,%d,%d,%d)->%p(%d,%d,%d,%d)",nsrc,rs.x,rs.y,rs.w,rs.h,ndst,dx,dy,rs.w,rs.h);
    if(dx<0){ rs.x -= dx; rs.w = (int)rs.w + dx; dx = 0;}
    if(dy<0){ rs.y -= dy; rs.h = (int)rs.h + dy; dy = 0;}

    //dx += screenMargin.x;
    //dy += screenMargin.y;
    if(dx + rs.w > ndst->width - screenMargin.x - screenMargin.w)
        rs.w = ndst->width - screenMargin.x - screenMargin.w - dx;
    if(dy + rs.h > ndst->height - screenMargin.y- screenMargin.h)
        rs.h = ndst->height- screenMargin.y - screenMargin.h - dy;
	
    toMIGFX(nsrc,&gfxsrc);
    toMIGFX(ndst,&gfxdst);
    bzero(&opt,sizeof(opt));

    opt.u32GlobalSrcConstColor = nsrc->alpha<<24;
    opt.u32GlobalDstConstColor = 0xFF000000;
    opt.eSrcDfbBldOp = E_MI_GFX_DFB_BLD_ONE;
    opt.eDstDfbBldOp = E_MI_GFX_DFB_BLD_ZERO;
    opt.eDFBBlendFlag= E_MI_GFX_DFB_BLEND_NOFX;
    opt.stClipRect.s32Xpos = dx+screenMargin.x*(ndst->ishw?1:0);
    opt.stClipRect.s32Ypos = dy+screenMargin.y*(ndst->ishw?1:0);
    opt.stClipRect.u32Width= rs.w;
    opt.stClipRect.u32Height=rs.h;
    /*if(nsrc->alpha!=255){
        opt.u32GlobalSrcConstColor = 0x00FFFFFF|(0xa0<<24);//(nsrc->alpha<<24);
        opt.u32GlobalDstConstColor = 0xFFFFFFFF;
        opt.eSrcDfbBldOp  = E_MI_GFX_DFB_BLD_ONE;// 透叠
        opt.eDstDfbBldOp  = E_MI_GFX_DFB_BLD_INVSRCALPHA;
        opt.eDFBBlendFlag = E_MI_GFX_DFB_BLEND_COLORALPHA
	          | E_MI_GFX_DFB_BLEND_ALPHACHANNEL
              | E_MI_GFX_DFB_BLEND_SRC_PREMULTIPLY;
    }else{
        opt.u32GlobalSrcConstColor = 0xFFFFFFFF;
        opt.u32GlobalDstConstColor = 0xFFFFFFFF;
        opt.eSrcDfbBldOp  = E_MI_GFX_DFB_BLD_ONE;  // 透叠
        //opt.eDFBBlendFlag = E_MI_GFX_DFB_BLEND_COLORALPHA;E_MI_GFX_DFB_BLEND_COLORIZE
        opt.eDFBBlendFlag = E_MI_GFX_DFB_BLEND_SRC_PREMULTIPLY;
    }*/
    opt.eMirror = E_MI_GFX_MIRROR_NONE;
    opt.eRotate = E_MI_GFX_ROTATE_0;

    stSrcRect.s32Xpos = rs.x;
    stSrcRect.s32Ypos = rs.y;
    stSrcRect.u32Width = rs.w;
    stSrcRect.u32Height= rs.h;

    stDstRect.s32Xpos = dx+screenMargin.x*(ndst->ishw?1:0);
    stDstRect.s32Ypos = dy+screenMargin.y*(ndst->ishw?1:0);
    stDstRect.u32Width = rs.w;
    stDstRect.u32Height= rs.h;
    LOGV("*Blit %p(%d,%d,%d,%d)->%p(%d,%d,%d,%d) gfx.src=%dx%dx%d/%d@%llx gfx.dst=%dx%dx%d/%d@%llx",
        nsrc,stSrcRect.s32Xpos,stSrcRect.s32Ypos, stSrcRect.u32Width, stSrcRect.u32Height,
        ndst,stDstRect.s32Xpos,stDstRect.s32Ypos, stDstRect.u32Width,stDstRect.u32Height,
        nsrc->width,nsrc->height,nsrc->pitch,gfxsrc.u32Stride,nsrc->kbuffer,
        ndst->width,ndst->height,ndst->pitch,gfxdst.u32Stride,ndst->kbuffer);
    if(nsrc->kbuffer && ndst->kbuffer){
        ret = MI_GFX_BitBlit(&gfxsrc,&stSrcRect,&gfxdst, &stDstRect,&opt,&fence);
        MI_GFX_WaitAllDone(TRUE,fence);
    }else{
        pbs+=rs.y*nsrc->pitch+rs.x*4;
        pbd+=dy*ndst->pitch+dx*4;
        const int cpw=rs.w*4;
        for(y=0; y<rs.h; y++) {
            memcpy(pbd,pbs,cpw);
            pbs += nsrc->pitch;
            pbd += ndst->pitch;
        }
    }
    return 0;
}

int32_t GFXBatchBlit(GFXHANDLE dstsurface,const GFXPoint*dest_point,GFXHANDLE srcsurface,const GFXRect*srcrects) {
    return 0;
}

int32_t GFXDestroySurface(GFXHANDLE surface) {
    FBSURFACE*surf=(FBSURFACE*)surface;
    LOGI("GFXDestroySurface %p:%llx/%p",surf,surf->kbuffer,surf->buffer);
    if( surf->used && (surf->kbuffer==NULL) ) { //user space memory surface
        free(surf->buffer);
        surf->buffer = NULL;
    }
    surf->used=0;
    return 0;
}
