#include <cdtypes.h>
#include <ngl_os.h>
#include <ngl_graph.h>
#include <cdlog.h>
#include <unistd.h>  
#include <stdio.h>  
#include <fcntl.h>  
#include <linux/fb.h>  
#include <sys/mman.h>  
#include <stdlib.h>  
#include <string.h>

NGL_MODULE(GRAPH);


static int fp;

DWORD nglGraphInit(){
    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;
    fp = open("/dev/graphics/fb0", O_RDWR);
    if(ioctl(fp, FBIOGET_FSCREENINFO, &finfo)){  
        LOGE("Error reading fixed information");  
        return E_ERROR;;
    }  
    if(ioctl(fp, FBIOGET_VSCREENINFO, &vinfo)){  
        LOGE("Error reading variable information");  
        return E_ERROR;
    }  
    return E_OK;
}

DWORD nglGetScreenSize(UINT*width,UINT*height){
    *width=1280;//dispCfg.width;
    *height=720;//dispCfg.height;
    return E_OK;
}

DWORD nglLockSurface(DWORD surface,void**buffer,UINT*pitch){
    GdkPixbuf*pb=(GdkPixbuf*)surface;
    *buffer= gdk_pixbuf_get_pixels(pb);
    *pitch = gdk_pixbuf_get_rowstride(pb);
    return 0;
}

DWORD nglGetSurfaceInfo(DWORD surface,UINT*width,UINT*height,INT *format)
{
    GdkPixbuf*pb=(GdkPixbuf*)surface;
    *width = gdk_pixbuf_get_width(pb);;
    *height= gdk_pixbuf_get_height(pb);
    *format=GPF_ARGB;
    return E_OK;
}

DWORD nglUnlockSurface(DWORD surface){
    return 0;
}

DWORD nglSurfaceSetOpacity(DWORD surface,BYTE alpha){
    return 0;//dispLayer->SetOpacity(dispLayer,alpha);
}

DWORD nglFillRect(DWORD surface,const NGLRect*rec,UINT color){
    return E_OK;
}

DWORD nglFlip(DWORD surface){
    return 0;
}

DWORD nglCreateSurface(DWORD*surface,INT width,INT height,INT format,BOOL hwsurface)
{
     GdkPixbuf*pixbuf=ng_pixbuf;
     if(!hwsurface) {
         GdkPixbuf*pixbuf= gdk_pixbuf_new(GDK_COLORSPACE_RGB, 0, 8,width,height);    
	 surface=pixbuf;
     }
     surface=pixbuf;
     LOGD("surface=%x pixbuf=%p",surface,pixbuf);
     return E_OK;
}


DWORD nglBlit(DWORD dstsurface,DWORD srcsurface,const NGLRect*srcrect,const NGLRect* dstrect)
{
     GdkPixbuf*src=(GdkPixbuf*)srcsurface;
     GdkPixbuf*dst=(GdkPixbuf*)dstsurface;
     gdk_pixbuf_copy_area(dst,(dstrect?dstrect->x:0),(dstrect?dstrect->y:0),
	  (dstrect?dstrect->w: gdk_pixbuf_get_width(src)),(dstrect?dstrect->h: gdk_pixbuf_get_width(src)),
          src,(srcrect?srcrect->x:0),(srcrect?srcrect->y:0));
//	  (srcrect?srcrect->w:src->w),
//	  (srcrect?srcrect->h:src->h));
     return 0;
}

DWORD nglDestroySurface(DWORD surface)
{
     gdk_pixbuf_destroy((GdkPixbuf*)surface);
     return 0;
}
