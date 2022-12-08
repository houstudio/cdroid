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
#include <core/eventcodes.h>
#include <cdinput.h>
#include <memory.h>
#include "mi_common.h"
#include "mi_sys.h"
#include "mi_gfx.h"

typedef struct{
    int fb;
    struct fb_fix_screeninfo fix;
    struct fb_var_screeninfo var;
}FBDEVICE;

typedef struct{
   UINT width;
   UINT height;
   UINT pitch;
   int format;
   int ishw;
   size_t msize;
   char*buffer;
   char*kbuffer;/*kernel buffer address*/
}FBSURFACE;

static FBDEVICE dev={-1};

static void gfxexit(){
   LOGI("gfxexit");
   MI_GFX_Close();
   MI_SYS_Exit();
}
DWORD GFXInit(){
    int ret;
    if(dev.fb>=0)return E_OK;
    ret=MI_SYS_Init();   LOGI("SYS_Init=%d",ret);
    ret=MI_GFX_Open();   LOGI("MI_GFX_Open=%d",ret);
    dev.fb=open("/dev/fb0", O_RDWR);
     // Get fixed screen information
    if(ioctl(dev.fb, FBIOGET_FSCREENINFO, &dev.fix) == -1) {
        LOGE("Error reading fixed information fd=%d",dev.fb);
        return E_ERROR;
    }
    atexit(gfxexit);
    LOGI("fb %d mem.addr=%x fbmem.size=%d pitch=%d",dev.fb,dev.fix.smem_start,dev.fix.smem_len,dev.fix.line_length);
    // Get variable screen information
    if(ioctl(dev.fb, FBIOGET_VSCREENINFO, &dev.var) == -1) {
        LOGE("Error reading variable information");
        return E_ERROR;
    }
    dev.var.yoffset=0;//set first screen memory for display
    ret=ioctl(dev.fb,FBIOPUT_VSCREENINFO,&dev.var);
    LOGI("FBIOPUT_VSCREENINFO=%d",ret);
    LOGI("fb solution=%dx%d accel_flags=0x%x",dev.var.xres,dev.var.yres,dev.var.accel_flags);
    return E_OK;
}

DWORD GFXGetScreenSize(UINT*width,UINT*height){
    LOGI_IF(width==NULL||height==NULL,"Params Error");
    *width=dev.var.xres;
    *height=dev.var.yres;
    LOGI("screensize=%dx%d",*width,*height);
    return E_OK;
}

DWORD GFXLockSurface(HANDLE surface,void**buffer,UINT*pitch){
    FBSURFACE*ngs=(FBSURFACE*)surface;
    *buffer=ngs->buffer;
    *pitch=ngs->pitch;
    return 0;
}

DWORD GFXGetSurfaceInfo(HANDLE surface,UINT*width,UINT*height,INT *format){
    FBSURFACE*ngs=(FBSURFACE*)surface;
    *width = ngs->width;
    *height= ngs->height;
    *format= ngs->format;
    return E_OK;
}

DWORD GFXUnlockSurface(HANDLE surface){
    return 0;
}

DWORD GFXSurfaceSetOpacity(HANDLE surface,BYTE alpha){
    return 0;//dispLayer->SetOpacity(dispLayer,alpha);
}

static void toMIGFX(const FBSURFACE*fb,MI_GFX_Surface_t*gfx){
    gfx->eColorFmt= E_MI_GFX_FMT_ARGB8888;
    gfx->u32Width = fb->width;
    gfx->u32Height= fb->height;
    gfx->u32Stride= fb->pitch;
    gfx->phyAddr  = (MI_PHY)fb->kbuffer;
}

DWORD GFXFillRect(HANDLE surface,const GFXRect*rect,UINT color){
    FBSURFACE*ngs=(FBSURFACE*)surface;
    UINT x,y;
    MI_S32 ret;
    MI_SYS_FrameData_t dt;
    GFXRect rec={0,0,0,0};
    rec.w=ngs->width;
    rec.h=ngs->height;
    if(rect)rec=*rect;
    if(ngs->kbuffer){
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
    }else{
       UINT*fb=(UINT*)(ngs->buffer+ngs->pitch*rec.y+rec.x*4);
       UINT*fbtop=fb;
       for(x=0;x<rec.w;x++)fb[x]=color;
       const int cpw=rec.w*4;
       long copied=0;
       for(y=1;y<rec.h;y++){
           fb+=(ngs->pitch>>2);
           memcpy(fb,fbtop,cpw);
           copied+=ngs->pitch;
       }	    
    }
    LOGV("FillRect %p %d,%d-%d,%d color=0x%x pitch=%d ret=%d",ngs,rec.x,rec.y,rec.w,rec.h,color,ngs->pitch,ret);
    return E_OK;
}

DWORD GFXFlip(HANDLE surface){
    FBSURFACE*surf=(FBSURFACE*)surface;
    if(surf->ishw){
        GFXRect rect={0,0,surf->width,surf->height};
        //if(rc)rect=*rc;
        dev.var.yoffset=0;
        int ret=ioctl(dev.fb, FBIOPAN_DISPLAY, &dev.var);
        LOGD_IF(ret<0,"FBIOPAN_DISPLAY=%d yoffset=%d",ret,dev.var.yoffset);
    }
    return 0;
}

static int setfbinfo(FBSURFACE*surf){
    int rc=-1;
    struct fb_var_screeninfo*v=&dev.var;
    v->xres=surf->width;
    v->yres=surf->height;
    v->xres_virtual=surf->width;
    v->yres_virtual=surf->height;
    v->bits_per_pixel=32;
    switch(surf->format){
    case GPF_ARGB:
	 v->transp.offset=24; v->transp.length=8;
	 v->red.offset=16; v->red.length=8;
	 v->green.offset=8; v->green.length=8;
	 v->blue.offset=0; v->blue.length=8;
	 break;
    case GPF_ABGR:
	 v->transp.offset=24; v->transp.length=8;
	 v->blue.offset=16; v->blue.length=8;
	 v->green.offset=8; v->green.length=8;
	 v->red.offset=0; v->red.length=8;
	 break;
    default:break; 
    }
    rc=ioctl(dev.fb,FBIOPUT_VSCREENINFO,v);
    LOGD("FBIOPUT_VSCREENINFO=%d",rc);
    return rc;
}

#define ALIGN(x,y) ((x&~(y))|y)
DWORD GFXCreateSurface(HANDLE*surface,UINT width,UINT height,INT format,BOOL hwsurface){
    FBSURFACE*surf=(FBSURFACE*)malloc(sizeof(FBSURFACE));
    static int created=0;
    char name[128];
    surf->width=width;
    surf->height=height;
    surf->format=format;
    surf->ishw=hwsurface;
    surf->pitch=width*4;
    surf->kbuffer=NULL;
    surf->msize=(surf->pitch*height);//sizeof dword
    sprintf(name,"#mmap_name%d",created++);

    MI_PHY phaddr;
    MI_S32 ret=MI_SYS_MMA_Alloc(name,surf->msize,&phaddr);
    if(ret==0){
        surf->kbuffer=(char*)phaddr;
        MI_SYS_Mmap(phaddr, surf->msize, (void**)&surf->buffer, FALSE);
        MI_SYS_MemsetPa(phaddr,0xFFFFFFFF,surf->msize);
    }
    if(hwsurface)  setfbinfo(surf);
    surf->ishw=hwsurface;
    LOGI("surface=%x buf=%p/%p size=%dx%d hw=%d\r\n",surf,surf->buffer,surf->kbuffer,width,height,hwsurface);
    *surface=surf;
    return E_OK;
}

DWORD GFXBlit(HANDLE dstsurface,int dx,int dy,HANDLE srcsurface,const GFXRect*srcrect){
    unsigned int x,y,sw,sh;
    FBSURFACE*ndst=(FBSURFACE*)dstsurface;
    FBSURFACE*nsrc=(FBSURFACE*)srcsurface;
    GFXRect rs={0,0};
    BYTE*pbs=(BYTE*)nsrc->buffer;
    BYTE*pbd=(BYTE*)ndst->buffer;
    rs.w=nsrc->width;rs.h=nsrc->height;
    if(srcrect)rs=*srcrect;
    if((nsrc->kbuffer==NULL)||(ndst->kbuffer==NULL)){
        if(((int)rs.w+dx<=0)||((int)rs.h+dy<=0)||(dx>=(int)ndst->width)||(dy>=(int)ndst->height)||(rs.x<0)||(rs.y<0)){
            LOGV("dx=%d,dy=%d rs=(%d,%d-%d,%d)",dx,dy,rs.x,rs.y,rs.w,rs.h);
            return E_INVALID_PARA;
        }

        LOGV("Blit %p[%dx%d] %d,%d-%d,%d -> %p[%dx%d] %d,%d",nsrc,nsrc->width,nsrc->height,
             rs.x,rs.y,rs.w,rs.h,ndst,ndst->width,ndst->height,dx,dy);
        if(dx<0){rs.x-=dx;rs.w=(int)rs.w+dx; dx=0;}
        if(dy<0){rs.y-=dy;rs.h=(int)rs.h+dy;dy=0;}
        if(dx+rs.w>ndst->width)rs.w=ndst->width-dx;
        if(dy+rs.h>ndst->height)rs.h=ndst->height-dy;

        LOGV("Blit %p %d,%d-%d,%d -> %p %d,%d buffer=%p->%p",nsrc,rs.x,rs.y,rs.w,rs.h,ndst,dx,dy,pbs,pbd);
        pbs+=rs.y*nsrc->pitch+rs.x*4;
        pbd+=dy*ndst->pitch+dx*4;
        const int cpw=rs.w*4;
        for(y=0;y<rs.h;y++){
            memcpy(pbd,pbs,cpw);
            pbs+=nsrc->pitch;
            pbd+=ndst->pitch;
        }
    }else{
	int ret;
	MI_U16 fence;
	MI_GFX_Opt_t opt;
	MI_GFX_Surface_t gfxsrc,gfxdst;
        MI_GFX_Rect_t stSrcRect, stDstRect;
	toMIGFX(nsrc,&gfxsrc);
	toMIGFX(ndst,&gfxdst);
	bzero(&opt,sizeof(opt));

        opt.u32GlobalSrcConstColor = 0xFF000000;
        opt.u32GlobalDstConstColor = 0xFF000000;
        opt.eSrcDfbBldOp = E_MI_GFX_DFB_BLD_ONE;
        opt.eDstDfbBldOp = E_MI_GFX_DFB_BLD_ZERO;
        opt.eMirror = E_MI_GFX_MIRROR_NONE;
        opt.eRotate = E_MI_GFX_ROTATE_0;

        stSrcRect.s32Xpos = rs.x;
        stSrcRect.s32Ypos = rs.y;
        stSrcRect.u32Width = rs.w;
        stSrcRect.u32Height= rs.h;

        stDstRect.s32Xpos = dx;
        stDstRect.s32Ypos = dy;
        stDstRect.u32Width = rs.w;
        stDstRect.u32Height= rs.h;
        ret = MI_GFX_BitBlit(&gfxsrc,&stSrcRect,&gfxdst, &stDstRect,&opt,&fence);
	MI_GFX_WaitAllDone(FALSE,fence);
    }    
    return 0;
}

DWORD GFXDestroySurface(HANDLE surface){
    FBSURFACE*surf=(FBSURFACE*)surface;
    LOGI("GFXDestroySurface %p/%p",surf,surf->buffer);
    if(surf->kbuffer){
        MI_SYS_Munmap(surf->buffer,surf->msize);
        MI_SYS_MMA_Free((MI_PHY)surf->kbuffer);
    }else if(surf->buffer){
        free(surf->buffer);
    }
    free(surf);
    return 0;
}
