#include <cdtypes.h>
#include <ngl_os.h>
#include <cdgraph.h>
#include <cdlog.h>
#include <X11/Xlib.h>
#include <gtk/gtk.h>
#include <pthread.h>
NGL_MODULE(GRAPH);

GtkWidget *gtk_window=NULL;
GtkWidget *gtk_area = NULL;//gtk_gl_area_new();
GdkPixbuf *gtk_pixbuf;
static gboolean on_expose_event (GtkWidget * widget, GdkEventExpose *event, gpointer data){
    gdk_draw_rectangle();
}
static void GTKProc(void*param){
    gtk_init(0,NULL);
    gtk_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_position(GTK_WINDOW(gtk_window), GTK_WIN_POS_CENTER);   // 中央位置显示
    gtk_widget_set_size_request(gtk_window, 1280, 720);            // 窗口最小大小
    gtk_window_set_resizable(GTK_WINDOW(gtk_window), FALSE);
    gtk_area = gtk_gl_area_new(); 
    gtk_container_add(GTK_CONTAINER(gtk_window), GTK_WIDGET(gtk_area));

    gtk_pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, 0, 8,1280, 720);
    //gdk_window_set_back_pixmap(gtk_window,gtk_pixbuf,FALSE);
    gtk_widget_show_all(gtk_window);
    gtk_main();
}

DWORD nglGraphInit(){
    pthread_t tid;
    if(gtk_window)return E_OK;
    pthread_create(&tid,NULL,GTKProc,NULL);
    return E_OK;
}

DWORD nglGetScreenSize(UINT*width,UINT*height){
    *width=1280;//dispCfg.width;
    *height=720;//dispCfg.height;
    return E_OK;
}

DWORD nglLockSurface(HANDLE surface,void**buffer,UINT*pitch){
    GdkPixbuf*pb=(GdkPixbuf*)surface;
    *buffer= gdk_pixbuf_get_pixels(pb);
    *pitch = gdk_pixbuf_get_rowstride(pb);
    return 0;
}

DWORD nglGetSurfaceInfo(HANDLE surface,UINT*width,UINT*height,INT *format)
{
    GdkPixbuf*pb=(GdkPixbuf*)surface;
    *width = gdk_pixbuf_get_width(pb);;
    *height= gdk_pixbuf_get_height(pb);
    if(format)*format=GPF_ARGB;
    return E_OK;
}

DWORD nglUnlockSurface(HANDLE surface){
    return 0;
}

DWORD nglSurfaceSetOpacity(HANDLE surface,BYTE alpha){
    return 0;//dispLayer->SetOpacity(dispLayer,alpha);
}

DWORD nglFillRect(HANDLE surface,const NGLRect*rect,UINT color){
    GdkPixbuf*nsrc=(GdkPixbuf*)surface;
    NGLRect rec={0,0,0,0};
    rec.w = gdk_pixbuf_get_width(nsrc);
    rec.h = gdk_pixbuf_get_height(nsrc);
    if(rect)rec=*rect;
    UINT*fb=(UINT*)(gdk_pixbuf_get_pixels(nsrc)+gdk_pixbuf_get_rowstride(nsrc)*rec.y+rec.x*4);
     for(int y=0;y<rec.h;y++){
        for(int x=0;x<rec.w;x++)
           fb[x]=color;
        fb += (gdk_pixbuf_get_rowstride(nsrc)>>2);
     }
    if(surface==gtk_pixbuf)
        gtk_widget_queue_draw(GTK_WINDOW(gtk_area));
    return E_OK;
}

DWORD nglFlip(HANDLE surface){
    if(surface==gtk_pixbuf){
       gtk_widget_queue_draw(GTK_WIDGET(gtk_window));
    }
    return 0;
}

DWORD nglCreateSurface(HANDLE*surface,UINT width,UINT height,INT format,BOOL hwsurface)
{
     GdkPixbuf*pixbuf= gdk_pixbuf_new(GDK_COLORSPACE_RGB, 0, 8,width,height);    
     if(hwsurface)
         gtk_pixbuf=pixbuf;
     *surface=pixbuf;
     LOGD("surface=%x pixbuf=%p",surface,pixbuf);
     return E_OK;
}


DWORD nglBlit(HANDLE dstsurface,int dx,int dy,HANDLE srcsurface,const NGLRect*srcrect)
{
     unsigned int x,y,sw,sh,dw,dh;
     GdkPixbuf*ndst=(GdkPixbuf*)dstsurface;
     GdkPixbuf*nsrc=(GdkPixbuf*)srcsurface;
     NGLRect rs={0,0};
     rs.w=dw=gdk_pixbuf_get_width(nsrc);rs.h=dh=gdk_pixbuf_get_height(nsrc);
     if(srcrect)rs=*srcrect;
     if(((int)rs.w+dx<=0)||((int)rs.h+dy<=0)||(dx>=dw)||(dy>=dh)||(rs.x<0)||(rs.y<0)){
         LOGV("dx=%d,dy=%d rs=(%d,%d-%d,%d)",dx,dy,rs.x,rs.y,rs.w,rs.h);
         return E_INVALID_PARA;
     }

     LOGV("Blit %p[%dx%d] %d,%d-%d,%d -> %p[%dx%d] %d,%d",nsrc,gdk_pixbuf_get_width(nsrc),gdk_pixbuf_get_height(nsrc),
          rs.x,rs.y,rs.w,rs.h,ndst,dw,dh,dx,dy);
     if(dx<0){rs.x-=dx;rs.w=(int)rs.w+dx; dx=0;}
     if(dy<0){rs.y-=dy;rs.h=(int)rs.h+dy;dy=0;}
     if(dx+rs.w>dw) rs.w = dw - dx;
     if(dy+rs.h>dh) rs.h = dh - dy;

     LOGV("Blit %p %d,%d-%d,%d -> %p %d,%d ",nsrc,rs.x,rs.y,rs.w,rs.h,ndst,dx,dy);
     gdk_pixbuf_copy_area(ndst,dx,dy,rs.w,rs.h,nsrc,rs.x,rs.y);
     if(gtk_pixbuf==ndst){
         gtk_widget_queue_draw(GTK_WIDGET(gtk_area));
     }
     return 0;
}

DWORD nglDestroySurface(HANDLE surface)
{
     g_object_unref((GdkPixbuf*)surface);
     return 0;
}
