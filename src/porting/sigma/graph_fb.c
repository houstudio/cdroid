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
typedef struct{
    int fb;
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
   char*buffer;
   char*bkbuffer;/*kernel buffer address*/
}FBSURFACE;

static FBDEVICE devs[2]={-1};


INT GFXInit(){
    if(devs[0].fb>=0)return E_OK;
    memset(devs,0,sizeof(devs));
    FBDEVICE*dev=&devs[0];
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

    dev->var.yoffset=0;//set first screen memory for display
    LOGI("FBIOPUT_VSCREENINFO=%d",ioctl(dev->fb,FBIOPUT_VSCREENINFO,&dev->var));
    LOGI("fb solution=%dx%d accel_flags=0x%x\r\n",dev->var.xres,dev->var.yres,dev->var.accel_flags);
    return E_OK;
}

INT GFXGetDisplayCount(){
    return 1;
}

INT GFXGetDisplaySize(int dispid,UINT*width,UINT*height){
    if(dispid<0||dispid>=GFXGetDisplayCount())return E_ERROR;
    FBDEVICE*dev=devs+dispid;
    *width=dev->var.xres;
    *height=dev->var.yres;
    LOGD("screensize=%dx%d",*width,*height);
    return E_OK;
}

GFX_ROTATION GFXGetRotation(int dispid){
    return ROTATE_0;
}

INT GFXSetRotation(int dispid,GFX_ROTATION r){
    return 0;
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
    return 0;
}

INT GFXSurfaceSetOpacity(HANDLE surface,BYTE alpha){
    return 0;//dispLayer->SetOpacity(dispLayer,alpha);
}

INT GFXFillRect(HANDLE surface,const GFXRect*rect,UINT color){
    FBSURFACE*ngs=(FBSURFACE*)surface;
    UINT x,y;
    GFXRect rec={0,0,0,0};
    rec.w=ngs->width;
    rec.h=ngs->height;
    if(rect)rec=*rect;
    LOGV("FillRect %p %d,%d-%d,%d color=0x%x pitch=%d",ngs,rec.x,rec.y,rec.w,rec.h,color,ngs->pitch);
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
    return E_OK;
}

INT GFXFlip(HANDLE surface){
    FBSURFACE*surf=(FBSURFACE*)surface;
    FBDEVICE*dev=devs+surf->dispid;
    if(surf->ishw){
        GFXRect rect={0,0,surf->width,surf->height};
        //if(rc)rect=*rc;
        dev->var.yoffset=0;
        int ret=ioctl(dev->fb, FBIOPAN_DISPLAY, &dev->var);
        LOGD_IF(ret<0,"FBIOPAN_DISPLAY=%d yoffset=%d",ret,dev->var.yoffset);
    }
    return 0;
}

static int setfbinfo(FBSURFACE*surf){
    int rc=-1;
    FBDEVICE*dev=devs+surf->dispid;
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
    surf->dispid=dispid;
    surf->width=width;
    surf->height=height;
    surf->format=format;
    surf->ishw=hwsurface;
    surf->pitch=width*4;
    size_t buffer_size=surf->height*surf->pitch;
    FBDEVICE*dev=devs+dispid;
    if(hwsurface && devs[dispid].fix.smem_len){
        size_t mem_len=((dev->fix.smem_start) -((dev->fix.smem_start) & ~(getpagesize() - 1)));
	    buffer_size=surf->height*dev->fix.line_length;
        setfbinfo(surf);
        surf->buffer=(char*)mmap(dev->fix.smem_start,buffer_size,PROT_READ | PROT_WRITE, MAP_SHARED,dev->fb, 0 );
        surf->pitch=dev->fix.line_length;
    }else{
        surf->buffer=(char*)mmap( dev->fix.smem_start+buffer_size,buffer_size,PROT_READ | PROT_WRITE, MAP_SHARED,dev->fb, 0 );
    }
    surf->ishw=hwsurface;
    LOGI("surface=%x buf=%p size=%dx%d hw=%d",surf,surf->buffer,width,height,hwsurface);
    *surface=surf;
    return E_OK;
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
    return 0;
}

INT GFXDestroySurface(HANDLE surface){
    FBSURFACE*surf=(FBSURFACE*)surface;
    FBDEVICE*dev=devs+surf->dispid;
    if(surf->ishw)
        munmap(surf->buffer,surf->pitch*surf->height);
    else if(surf->buffer)free(surf->buffer);
    free(surf);
    return 0;
}
