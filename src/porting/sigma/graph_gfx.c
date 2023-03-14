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
#include "pixman.h"

typedef struct{
    int fb;
    int rotation;
    struct fb_fix_screeninfo fix;
    struct fb_var_screeninfo var;
}FBDEVICE;

typedef struct{
   int dispid;
   UINT width;
   UINT height;
   UINT pitch;
   int format;
   int ishw;
   int current;
   size_t msize;
   char*buffer;//drawbuffer
   char*orig_buffer;//used only in double buffer*/
   char*kbuffer;/*kernel buffer address*/
   pixman_image_t*image;
}FBSURFACE;

static FBDEVICE devs[2]={-1};

static void gfxexit(){
   LOGI("gfxexit");
   MI_GFX_Close();
   MI_SYS_Exit();
}

int GFXInit(){
    int ret;
    FBDEVICE*dev=&devs[0];
    if(dev->fb>=0)return E_OK;
    memset(devs,0,sizeof(devs));
    ret=MI_SYS_Init();   LOGI("SYS_Init=%d",ret);
    ret=MI_GFX_Open();   LOGI("MI_GFX_Open=%d",ret);
    devs[0].fb=open("/dev/fb0", O_RDWR);
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
    devs[0].var.yoffset=0;//set first screen memory for display
    ret=ioctl(dev->fb,FBIOPUT_VSCREENINFO,&dev->var);
    LOGI("FBIOPUT_VSCREENINFO=%d",ret);
    LOGI("fb solution=%dx%d accel_flags=0x%x",dev->var.xres,dev->var.yres,dev->var.accel_flags);
    return E_OK;
}

INT GFXGetDisplayCount(){
    return 1;
}

INT GFXGetDisplaySize(int dispid,UINT*width,UINT*height){
    if(dispid<0||dispid>=GFXGetDisplayCount())
	 return E_ERROR;
    LOGI_IF(width==NULL||height==NULL,"Params Error");
    FBDEVICE*dev=&devs[dispid];
    *width=dev->var.xres;
    *height=dev->var.yres;
    LOGV("screensize=%dx%d",*width,*height);
    return E_OK;
}

INT GFXSetRotation(int dispid, GFX_ROTATION roatation){
    if(dispid<0||dispid>=GFXGetDisplayCount())return E_ERROR;
    devs[dispid].rotation=roatation; 
}

GFX_ROTATION GFXGetRotation(int dispid){
    if(dispid<0||dispid>=sizeof(devs)/sizeof(FBDEVICE))return E_ERROR;
    return devs[dispid].rotation;
}

INT GFXLockSurface(HANDLE surface,void**buffer,UINT*pitch){
    FBSURFACE*ngs=(FBSURFACE*)surface;
    *buffer=ngs->buffer;
    *pitch=ngs->pitch;
    return 0;
}

INT GFXGetSurfaceInfo(HANDLE surface,UINT*width,UINT*height,INT *format){
    FBSURFACE*ngs=(FBSURFACE*)surface;
    *width = ngs->width;
    *height= ngs->height;
    *format= ngs->format;
    return E_OK;
}

INT GFXUnlockSurface(HANDLE surface){
    return E_OK;
}

INT GFXSurfaceSetOpacity(HANDLE surface,BYTE alpha){
    return E_OK;//dispLayer->SetOpacity(dispLayer,alpha);
}

static void toMIGFX(const FBSURFACE*fb,MI_GFX_Surface_t*gfx){
    gfx->eColorFmt= E_MI_GFX_FMT_ARGB8888;
    gfx->u32Width = fb->width;
    gfx->u32Height= fb->height;
    gfx->u32Stride= fb->pitch;
    gfx->phyAddr  = (MI_PHY)fb->kbuffer;
}

INT GFXFillRect(HANDLE surface,const GFXRect*rect,UINT color){
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
       //pixman_fill(ngs->buffer,ngs->pitch/sizeof(uint32_t),PIXMAN_FORMAT_BPP(PIXMAN_a8r8g8b8),rec.x,rec.y,rec.w,rec.h,color);
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
       //pixman_fill(ngs->buffer,ngs->pitch/sizeof (uint32_t),PIXMAN_FORMAT_BPP(PIXMAN_a8r8g8b8),rec.x,rec.y,rec.w,rec.h,color);       
    }
    LOGV("FillRect %p %d,%d-%d,%d color=0x%x pitch=%d ret=%d",ngs,rec.x,rec.y,rec.w,rec.h,color,ngs->pitch,ret);
    return E_OK;
}

INT GFXFlip(HANDLE surface){
    FBSURFACE*surf=(FBSURFACE*)surface;
    const size_t screen_size=surf->msize/2;
    if( surf->ishw && (surf->msize>screen_size) ){
	FBDEVICE*dev=&devs[surf->dispid];
	LOGI_IF(screen_size!=dev->var.xres * dev->var.yres * dev->var.bits_per_pixel / 8,
	   "screensize=%dx%dx%dbpp",dev->var.xres,dev->var.yres,dev->var.bits_per_pixel);
	if(surf->current==0){
	    LOGI_IF(dev->fix.smem_start!=surf->kbuffer,"kbuffer error1");
    	    MI_SYS_MemcpyPa(surf->kbuffer+screen_size,surf->kbuffer,screen_size);//drawbuffer->screenbuffer
	    surf->buffer=surf->orig_buffer+screen_size;
	    surf->kbuffer+=screen_size;
	    dev->var.yoffset=0;//dev->var.yres;
	}else{
	    LOGI_IF(dev->fix.smem_start!=surf->kbuffer-screen_size,"kbuffer error2");
	    MI_SYS_MemcpyPa(surf->kbuffer-screen_size,surf->kbuffer,screen_size);//drawbuffer->screenbuffer
	    surf->buffer=surf->orig_buffer;
	    dev->var.yoffset=dev->var.yres;
	    surf->kbuffer-=screen_size;
	}
	surf->current=(surf->current+1)%2;
        int ret=ioctl(dev->fb, FBIOPAN_DISPLAY, &dev->var);
        LOGI_IF(ret<0,"FBIOPAN_DISPLAY=%d yoffset=%d res=%dx%d dev=%p fb=%d",ret,dev->var.yoffset,dev->var.xres,dev->var.yres,dev,dev->fb);
    }
    return 0;
}

static int setfbinfo(FBSURFACE*surf){
    int rc=-1;
    FBDEVICE*dev=&devs[surf->dispid];
    struct fb_var_screeninfo*v=&dev->var;
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
    rc=ioctl(dev->fb,FBIOPUT_VSCREENINFO,v);
    LOGD("FBIOPUT_VSCREENINFO=%d",rc);
    return rc;
}

INT GFXCreateSurface(int dispid,HANDLE*surface,UINT width,UINT height,INT format,BOOL hwsurface){
    FBSURFACE*surf=(FBSURFACE*)malloc(sizeof(FBSURFACE));
    FBDEVICE*dev=&devs[dispid];
    surf->dispid=dispid;
    surf->width=width;
    surf->height=height;
    surf->format=format;
    surf->ishw=hwsurface;
    surf->pitch=width*4;
    surf->kbuffer=NULL;
    surf->orig_buffer=NULL;
    surf->current = 0;
    surf->msize= surf->pitch*height;

    MI_PHY phaddr=dev->fix.smem_start;
    MI_S32 ret=0;
    if(hwsurface){
	surf->msize*=2;
        surf->buffer=mmap(dev->fix.smem_start,surf->msize,PROT_READ | PROT_WRITE, MAP_SHARED,dev->fb, 0);
        dev->var.yoffset=0;
        LOGI("ioctl offset(0)=%d",ioctl(dev->fb, FBIOPAN_DISPLAY, &dev->var));
        dev->var.yoffset=1280;
        LOGI("ioctl offset(0)=%d dev=%p",ioctl(dev->fb,FBIOPAN_DISPLAY,&dev->var),dev);	
    }else{
	int i=0;
	ret=MI_SYS_MMA_Alloc("mma_heap_name0",surf->msize,&phaddr);
	while((i++<3)&&(phaddr==dev->fix.smem_start)){
	    ret=MI_SYS_MMA_Alloc("mma_heap_name0",surf->msize,&phaddr);
	    LOGI("[%d]=%x ret=%d",i,phaddr,ret);
	}
        MI_SYS_Mmap(phaddr, surf->msize, (void**)&surf->buffer, FALSE);
    }
    surf->kbuffer=(char*)phaddr;
    if(hwsurface&&((GFXGetRotation(0)==ROTATE_90)||(GFXGetRotation(0)==ROTATE_270))){
	surf->width=height;
        surf->height=width;
        surf->pitch=height*4;
    }
    MI_SYS_MemsetPa(phaddr,0xFFFFFFFF,surf->msize);
    surf->orig_buffer=surf->buffer;
    if(hwsurface)  setfbinfo(surf);
    surf->ishw=hwsurface;
    //surf->image = pixman_image_create_bits_no_clear(PIXMAN_a8r8g8b8,surf->width,surf->height,surf->buffer,surf->pitch);
    LOGI("Surface=%x buf=%p/%p size=%dx%d/%d hw=%d\r\n",surf,surf->buffer,surf->kbuffer,width,height,surf->msize,hwsurface);
    *surface=surf;
    return E_OK;
}

static int DMABlit(FBSURFACE*ndst,MI_GFX_Rect_t*recDst,FBSURFACE*nsrc,MI_GFX_Rect_t*recSrc){
    MI_SYS_FrameData_t stSrcFrame, stDstFrame;
    memset(&stSrcFrame,0,sizeof(stSrcFrame));
    memset(&stDstFrame,0,sizeof(stDstFrame));
    stSrcFrame.ePixelFormat = E_MI_SYS_PIXEL_FRAME_ARGB8888;
    stSrcFrame.phyAddr[0]= nsrc->kbuffer;
    stSrcFrame.u16Width  = nsrc->width;
    stSrcFrame.u16Height = nsrc->height;
    stSrcFrame.u32Stride[0] = nsrc->pitch/4;
    stDstFrame.ePixelFormat = E_MI_SYS_PIXEL_FRAME_ARGB8888;//E_MI_SYS_PIXEL_FRAME_I8;

    stDstFrame.phyAddr[0]= ndst->kbuffer;
    stDstFrame.u16Width  = ndst->width;
    stDstFrame.u16Height = ndst->height;
    stDstFrame.u32Stride[0] = ndst->pitch/4;

    return MI_SYS_BufBlitPa(&stDstFrame,(MI_SYS_WindowRect_t*)recDst , &stSrcFrame, (MI_SYS_WindowRect_t*)recSrc);    
}

INT GFXBlit(HANDLE dstsurface,int dx,int dy,HANDLE srcsurface,const GFXRect*srcrect){
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
	//pixman_blt(nsrc->buffer,ndst->buffer,nsrc->pitch/sizeof (uint32_t),ndst->pitch/sizeof (uint32_t),
	//	PIXMAN_FORMAT_BPP(PIXMAN_a8r8g8b8),PIXMAN_FORMAT_BPP(PIXMAN_a8r8g8b8),rs.x,rs.y,dx,dy,rs.w,rs.h);
    }else{
	int ret;
	MI_U16 fence;
	MI_GFX_Opt_t opt;
	MI_GFX_Surface_t gfxsrc,gfxdst;
        MI_GFX_Rect_t stSrcRect, stDstRect;
	toMIGFX(nsrc,&gfxsrc);
	toMIGFX(ndst,&gfxdst);
	bzero(&opt,sizeof(opt));

        LOGV("Blit %p %d,%d-%d,%d -> %p %d,%d buffer=%p->%p",nsrc,rs.x,rs.y,rs.w,rs.h,ndst,dx,dy,pbs,pbd);
        opt.u32GlobalSrcConstColor = 0xFF000000;
        opt.u32GlobalDstConstColor = 0xFF000000;
        opt.eSrcDfbBldOp = E_MI_GFX_DFB_BLD_ONE;
        opt.eDstDfbBldOp = E_MI_GFX_DFB_BLD_ZERO;
        opt.eMirror = E_MI_GFX_MIRROR_NONE;
	opt.eRotate=ndst->ishw?GFXGetRotation(nsrc->dispid):E_MI_GFX_ROTATE_0;
        stSrcRect.s32Xpos = rs.x;
        stSrcRect.s32Ypos = rs.y;
        stSrcRect.u32Width = rs.w;
        stSrcRect.u32Height= rs.h;
	if(ndst->ishw){
	    int tmp;
      	    switch(GFXGetRotation(nsrc->dispid)){
	    case ROTATE_0 :break;
	    case ROTATE_90:
		 tmp=dx;
	         dx=ndst->width-dy-rs.h;
		 dy=tmp;
		 break;
	    case ROTATE_180:
		 dx=ndst->width-dx-rs.w;
		 dy=ndst->height-dy-rs.h;
		 if(dx<0){
		     rs.w+=dx;
		     stSrcRect.u32Width+=dx;
		     dx=0;
		 }
		 break;
	    case ROTATE_270:
		 tmp=dx;
		 dx=dy;
		 dy=ndst->height-tmp-rs.w;
		 break;
	    }
	}
        stDstRect.s32Xpos = dx;
        stDstRect.s32Ypos = dy;
        stDstRect.u32Width = rs.w;
        stDstRect.u32Height= rs.h;
	/*if(opt.eRotate==0){
	    ret= DMABlit(ndst,&stDstRect,nsrc,&stSrcRect);
	}else*/{ 
	    ret = MI_GFX_BitBlit(&gfxsrc,&stSrcRect,&gfxdst, &stDstRect,&opt,&fence);
	    MI_GFX_WaitAllDone(FALSE,fence);
	}
    }    
    return 0;
}

INT GFXBatchBlit(HANDLE dstsurface,const GFXPoint*dest_point,HANDLE srcsurface,const GFXRect*srcrects){
#if 0	
#endif    
    return 0;
}

INT GFXDestroySurface(HANDLE surface){
    FBSURFACE*surf=(FBSURFACE*)surface;
    LOGI("GFXDestroySurface %p/%p",surf,surf->buffer);
    if(surf->kbuffer){
        MI_SYS_Munmap(surf->buffer,surf->msize);
        if(surf->ishw==0)
	    MI_SYS_MMA_Free((MI_PHY)surf->kbuffer);
    }else if(surf->buffer){
        free(surf->buffer);
    }
    //pixman_image_unref(surf->image);
    free(surf);
    return 0;
}
