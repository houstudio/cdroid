#include <cdtypes.h>
#include <cdgraph.h>
#include <cdinput.h>
#include <cdlog.h>
#include <stdlib.h>
#include <xcb/xcb.h>
#include <xcb/xcb_image.h>
#include <xcb/xcb_keysyms.h>
#include <X11/keysym.h>
#include <pthread.h>
#include <string.h>
#include <linux/input.h>
#include <time.h>
static xcb_connection_t *xcbConnection = NULL;
static xcb_window_t xcbWindow;
static xcb_gcontext_t xcbGC;
static xcb_pixmap_t xcbPixmap;
static xcb_image_t*mainSurface=NULL;
static void* XCBEventProc(void*p);
const char *xcb_error_to_string(int error_code);
static GFXRect screenMargin= {0}; //{60,0,60,0};
//#define USE_PIXMAN 1
#define SENDKEY(k,down) {InjectKey(EV_KEY,k,down);}
#if 1
   #define SENDMOUSE(time,x,y)  {\
      InjectABS(time,EV_ABS,ABS_MT_TRACKING_ID,12);\
      InjectABS(time,EV_ABS,ABS_MT_POSITION_X,x);\
      InjectABS(time,EV_ABS,ABS_MT_POSITION_Y,y);\
      InjectABS(time,EV_ABS,SYN_MT_REPORT,0);\
      InjectABS(time,EV_SYN,SYN_REPORT,0);}

/*   #define SENDMOUSE(time,x,y)  {\
      InjectABS(time,EV_ABS,ABS_X,x);\
      InjectABS(time,EV_ABS,ABS_Y,y);\
      InjectABS(time,EV_SYN,SYN_REPORT,0);}*/
#else
   #define SENDMOUSE(time,x,y)  {\
      InjectREL(time,EV_REL,REL_X,x);\
      InjectREL(time,EV_REL,REL_Y,y);\
      InjectREL(time,EV_SYN,SYN_REPORT,0);}
#endif
static void InjectKey(int type,int code,int value) {
    INPUTEVENT i= {0};
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC,&ts);
    i.tv_sec=ts.tv_sec;
    i.tv_usec=ts.tv_nsec/1000;
    i.type=type;
    i.code=code;
    i.value=value;
    i.device=INJECTDEV_KEY;
    InputInjectEvents(&i,1,1);
}
static void InjectABS(unsigned long time,int type,int axis,int value) {
    INPUTEVENT i= {0};
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC,&ts);
    i.tv_sec=ts.tv_sec;
    i.tv_usec=ts.tv_nsec/1000;
    i.type=type;
    i.code=axis;
    i.value=value;
    i.device=INJECTDEV_TOUCH;
    InputInjectEvents(&i,1,1);
}
static void InjectREL(unsigned long time,int type,int axis,int value) {
    INPUTEVENT i= {0};
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC,&ts);
    i.tv_sec=ts.tv_sec;
    i.tv_usec=ts.tv_nsec/1000;
    i.type=type;
    i.code=axis;
    i.value=value;
    i.device=INJECTDEV_MOUSE;
    InputInjectEvents(&i,1,1);
}
static void onExit() {
    LOGD("X11 Graph shutdown!");
}

int32_t GFXInit() {
    if(xcbConnection)return E_OK;
    //xcw.Open(800,600);
    xcbConnection = xcb_connect(NULL, NULL);
    if (xcb_connection_has_error(xcbConnection)) {
        LOGE("Cannot open display %p",xcbConnection);
        return E_ERROR;
    }
    const xcb_setup_t *setup = xcb_get_setup(xcbConnection);
    xcb_screen_iterator_t iter = xcb_setup_roots_iterator(setup);
    xcb_screen_t *screen = iter.data;

    pthread_t xThreadId;
    uint32_t width,height;
    const char*strMargin=getenv("SCREEN_MARGINS");
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
    GFXGetDisplaySize(0,&width,&height);
    width += screenMargin.x + screenMargin.w;
    height+= screenMargin.y + screenMargin.h;

    xcbPixmap = xcb_generate_id(xcbConnection);
    xcb_create_pixmap(xcbConnection,screen->root_depth,xcbPixmap,screen->root,width,height);

    xcbWindow = xcb_generate_id(xcbConnection);
    uint32_t value_mask = XCB_CW_BACK_PIXMAP |XCB_CW_BORDER_PIXEL | XCB_CW_EVENT_MASK;
    uint32_t value_list[4] = {
        xcbPixmap,
        screen->white_pixel,
        XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_KEY_PRESS};

    xcb_create_window(xcbConnection, XCB_COPY_FROM_PARENT,
        xcbWindow, screen->root, 0, 0, width, height, 4,
        XCB_WINDOW_CLASS_INPUT_OUTPUT,
        screen->root_visual, value_mask, value_list);
    
    xcb_change_property(xcbConnection, XCB_PROP_MODE_REPLACE, xcbWindow,
            XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8, 6,"CDROID"); 

    
    const uint32_t gc_values[] = {0};
    xcbGC = xcb_generate_id(xcbConnection);
    xcb_create_gc(xcbConnection, xcbGC, xcbPixmap, 0, gc_values);
    
    xcb_map_window(xcbConnection, xcbWindow);
    xcb_flush(xcbConnection);

    LOGD("xcbConnection=%p xcbWindow=%p(%dx%dx%d) maxlen=%d",xcbConnection,xcbWindow,width,height,screen->root_depth,xcb_get_maximum_request_length(xcbConnection));
    pthread_create(&xThreadId,NULL,XCBEventProc,NULL);
    pthread_detach(xThreadId);
    atexit(onExit);
    return E_OK;
}

int32_t GFXGetDisplaySize(int dispid,uint32_t*width,uint32_t*height) {
    const char*env= getenv("SCREEN_SIZE");
    if(env==NULL) {
        *width =1280;//dispCfg.width;
        *height=720;//dispCfg.height;
    } else {
        *width=atoi(env)- screenMargin.x - screenMargin.w;
        env=strpbrk(env,"x*,");
        if((*width<=0)||(env==NULL))exit(-1);
        *height=atoi(env+1)- screenMargin.y - screenMargin.h;
        if(*height<=0)exit(-1);
    }
    return E_OK;
}

int32_t GFXGetDisplayCount() {
    return 1;
}

int32_t GFXLockSurface(GFXHANDLE surface,void**buffer,uint32_t*pitch) {
    xcb_image_t*img=(xcb_image_t*)surface;
    *buffer=img->data;
    *pitch=img->stride;
    return 0;
}

int32_t GFXGetSurfaceInfo(GFXHANDLE surface,uint32_t*width,uint32_t*height,int32_t *format) {
    xcb_image_t*img=(xcb_image_t*)surface;
    *width=img->width;
    *height=img->height;
    if(format)*format=GPF_ARGB;
    return E_OK;
}

int32_t GFXUnlockSurface(GFXHANDLE surface) {
    return 0;
}

int32_t GFXSurfaceSetOpacity(GFXHANDLE surface,uint8_t alpha) {
    return 0;//dispLayer->SetOpacity(dispLayer,alpha);
}

int32_t GFXFillRect(GFXHANDLE surface,const GFXRect*rect,uint32_t color) {
    xcb_image_t*img=(xcb_image_t*)surface;
    uint32_t x,y;
    GFXRect rec= {0,0,0,0};
    rec.w=img->width;
    rec.h=img->height;
    if(rect)rec=*rect;
    LOGV("FillRect %p %d,%d-%d,%d color=0x%x pitch=%d",img,rec.x,rec.y,rec.w,rec.h,color,img->stride);
    uint32_t*fb=(uint32_t*)(img->data+img->stride*rec.y+rec.x*4);
    uint32_t*fbtop=fb;
    for(x=0; x<rec.w; x++)fb[x]=color;
    for(y=1; y<rec.h; y++) {
        fb+=(img->stride>>2);
        memcpy(fb,fbtop,rec.w*4);
    }
    if(surface==mainSurface) {
        xcb_clear_area(xcbConnection, 0, xcbWindow, 0,0,img->width,img->height);
    }
    return E_OK;
}

static void XCBExpose(int x,int y,int width,int height){
    xcb_expose_event_t ev;
    ev.response_type = XCB_EXPOSE;
    ev.window = xcbWindow;
    ev.x=x;
    ev.y=y;
    ev.width=width;
    ev.height=height;
    ev.count=0;
    xcb_send_event(xcbConnection, 0, xcbWindow, XCB_EVENT_MASK_EXPOSURE, (char *)&ev);
}
int32_t GFXFlip(GFXHANDLE surface) {
    xcb_image_t *img=(xcb_image_t*)surface;
    if(mainSurface==surface) {
        GFXRect rect= {0,0,img->width,img->height};
        //XCBExpose(0,0,img->width,img->height);//very slowly
    }
    return 0;
}

int32_t GFXCreateSurface(int dispid,GFXHANDLE*surface,uint32_t width,uint32_t height,int32_t format,bool hwsurface) {
    xcb_image_t*img=NULL;
    if(hwsurface) {
        width += screenMargin.x + screenMargin.w;
        height+= screenMargin.y + screenMargin.h;
    }
    void*base =malloc(width*height*4);
    img=xcb_image_create(width,height,XCB_IMAGE_FORMAT_Z_PIXMAP,0/*xpad*/,32,32,0,
                XCB_IMAGE_ORDER_LSB_FIRST, XCB_IMAGE_ORDER_LSB_FIRST,base, width*height*4, (uint8_t*)base);
    *surface=img;
    img->stride=width*4;
    LOGD("%p:%p  size=%dx%dx%d %d hwsurface=%d",img,img->data,width,height,img->stride,img->bpp,hwsurface);
    if(hwsurface) {
        mainSurface=img;
    }
    return E_OK;
}

int32_t GFXBlit(GFXHANDLE dstsurface,int dx,int dy,GFXHANDLE srcsurface,const GFXRect* srcrect) {
    xcb_image_t *ndst=(xcb_image_t*)dstsurface;
    xcb_image_t *nsrc=(xcb_image_t*)srcsurface;
    GFXRect rs= {0,0};
    uint8_t*pbs=(uint8_t*)nsrc->data;
    uint8_t*pbd=(uint8_t*)ndst->data;
    rs.w=nsrc->width;
    rs.h=nsrc->height;
    if(srcrect)rs=*srcrect;
    if(((int)rs.w+dx<=0)||((int)rs.h+dy<=0)||(dx>=(int)ndst->width)||(dy>=(int)ndst->height)||(rs.x<0)||(rs.y<0)) {
        LOGD("dx=%d,dy=%d rs=(%d,%d-%d,%d)",dx,dy,rs.x,rs.y,rs.w,rs.h);
        return E_INVALID_PARA;
    }

    if(dx<0) {
        rs.x -= dx;
        rs.w = (int)rs.w+dx;
        dx=0;
    }
    if(dy<0) {
        rs.y -= dy;
        rs.h = (int)rs.h+dy;
        dy=0;
    }
    if(dx + rs.w > ndst->width -screenMargin.x - screenMargin.w) rs.w = ndst->width -screenMargin.x - screenMargin.w -dx;
    if(dy + rs.h > ndst->height-screenMargin.y - screenMargin.h) rs.h = ndst->height-screenMargin.y - screenMargin.h -dy;

    //LOGV_IF(ndst==mainSurface,"Blit %p %d,%d-%d,%d -> %p %d,%d buffer=%p->%p",nsrc,rs.x,rs.y,rs.w,rs.h,ndst,dx,dy,pbs,pbd);
    pbs += rs.y*nsrc->stride+rs.x*4;
    pbd += dy*ndst->stride+dx*4;
    if(ndst==mainSurface) {
        pbd += (screenMargin.x*4 + screenMargin.h*ndst->stride);
    }
    for(unsigned int y=0; y<rs.h; y++) {
        memcpy(pbd,pbs,rs.w*4);
        pbs+=nsrc->stride;
        pbd+=ndst->stride;
    }
    if((ndst==mainSurface)&&xcbConnection) {
    LOGD("src (%d,%d,%d,%d) dst (%d,%d,%d,%d)",rs.x,rs.y,rs.w,rs.h,dx,dy,rs.w,rs.h);
        for(int x=100;x<200;x++)for(int y=100;y<200;y++)xcb_image_put_pixel(mainSurface,x,y,0xFFFF0000|x<<12|y);
        xcb_image_put(xcbConnection,xcbPixmap,xcbGC,mainSurface,dx,dy,0);
        XCBExpose(dx,dy,rs.w,rs.h);
        xcb_flush(xcbConnection);
    }
    return 0;
}

int32_t GFXDestroySurface(GFXHANDLE surface) {
    xcb_image_destroy((xcb_image_t*)surface);
    return 0;
}

static struct{int xkey;int key;}X11KEY2CD[]={
   {XK_0,7},{XK_1,8},{XK_2,9},{XK_3,10},{XK_4,11},{XK_5,12},{XK_6,13},{XK_7,14},{XK_8,15},{XK_9,16},
   {XK_F1,131},{XK_F2,132},{XK_F3,133},{XK_F4,134},{XK_F5,135},{XK_F6,136},{XK_F7,137},{XK_F8,138}, {XK_F9,139},{XK_F10,140},{XK_F11,141},{XK_F12,142},
   {XK_KP_Left,21},{XK_KP_Right,22},{XK_KP_Up,19},{XK_KP_Down,20},{XK_Home,122},{XK_End,123},{XK_Page_Up,92},{XK_Page_Down,93},
   {XK_Insert,124},{XK_Delete,67},{XK_KP_Enter,23/*DPAD_CENTER*/},{XK_Escape,111},{XK_Return,66},{XK_KP_Space,62},{XK_BackSpace,112},{XK_Menu,82},
   {XK_q,45},{XK_w,51},{XK_e,33},{XK_r,46},{XK_t,48},{XK_y,53/*Y*/},{XK_u,49},{XK_i,37},{XK_o,43},{XK_p,44},
   {XK_a,29},{XK_s,47},{XK_d,32},{XK_f,34},{XK_g,35},{XK_h,36},{XK_j,38},{XK_k,39},{XK_l,40},
   {XK_z,54},{XK_x,52},{XK_c,31},{XK_v,50},{XK_b,30},{XK_n,42},{XK_m,41}
};

static void* XCBEventProc(void*p) {
    xcb_generic_event_t *event;
    LOGD("XCBEventProc");
    while ((event = xcb_wait_for_event(xcbConnection))) {
        switch (event->response_type & ~0x80) {
        case XCB_EXPOSE: {
            xcb_expose_event_t *expose = (xcb_expose_event_t *) event;
            if(mainSurface){
                xcb_copy_area(xcbConnection, xcbPixmap, xcbWindow,xcbGC,expose->x,expose->y,expose->x,expose->y, expose->width,expose->height);
                xcb_flush(xcbConnection);
                LOGD("Expose event on window 0x%08x (%d,%d,%d,%d)mainSurface=%p", expose->window,expose->x,expose->y,expose->width,expose->height,mainSurface);
            }
            break;
        }
        case XCB_KEY_PRESS: {
            xcb_key_press_event_t *key_press = (xcb_key_press_event_t *) event;
            LOGD("Key press event: key %d, state 0x%08x", key_press->detail, key_press->state);
            break;
        }
        case XCB_BUTTON_PRESS: {
            xcb_button_press_event_t *button_press = (xcb_button_press_event_t *) event;
            LOGD("Button press event: button %d, state 0x%08x", button_press->detail, button_press->state);
            break;
        }
        default:
            LOGD("Unknown event type: %d", event->response_type & ~0x80);
            break;
        }
        free(event);
    }
    xcb_destroy_window(xcbConnection, xcbWindow);
    xcb_disconnect(xcbConnection);
    LOGD("XCBEventProc End.");
    return NULL;
}

const char *xcb_error_to_string(int error_code) {
    switch (error_code) {
        case XCB_REQUEST: return "XCB_REQUEST";
        case XCB_VALUE: return "XCB_VALUE";
        case XCB_WINDOW: return "XCB_WINDOW";
        case XCB_PIXMAP: return "XCB_PIXMAP";
        case XCB_ATOM: return "XCB_ATOM";
        case XCB_CURSOR: return "XCB_CURSOR";
        case XCB_FONT: return "XCB_FONT";
        case XCB_MATCH: return "XCB_MATCH";
        case XCB_DRAWABLE: return "XCB_DRAWABLE";
        case XCB_ACCESS: return "XCB_ACCESS";
        case XCB_ALLOC: return "XCB_ALLOC";
        case XCB_COLORMAP: return "XCB_COLORMAP";
        case XCB_G_CONTEXT: return "XCB_G_CONTEXT";
        case XCB_ID_CHOICE: return "XCB_ID_CHOICE";
        case XCB_NAME: return "XCB_NAME";
        case XCB_LENGTH: return "XCB_LENGTH";
        case XCB_IMPLEMENTATION: return "XCB_IMPLEMENTATION";
        default: return "Unknown XCB error";
    }
}
