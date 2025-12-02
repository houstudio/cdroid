#include <cdtypes.h>
#include <cdgraph.h>
#include <cdinput.h>
#include <cdlog.h>
#include <stdlib.h>
#include <xcb/xcb.h>
#include <xcb/xcb_image.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/keysymdef.h>
#include <pthread.h>
#include <string.h>
#include <linux/input.h>
#include <time.h>
static Display *xcbDisplay;
static xcb_connection_t *xcbConnection = NULL;
static const xcb_setup_t *xcbSetup=NULL;
static xcb_window_t xcbWindow;
static xcb_gcontext_t xcbGC;
static xcb_pixmap_t xcbPixmap;
static xcb_image_t*mainSurface=NULL;
static void* XCBEventProc(void*p);
static GFXRect screenMargin= {0}; //{60,0,60,0};
#define SENDKEY(k,down) {InjectKey(EV_KEY,k,down);}
#if 1
   #define SENDMOUSE(time,x,y)  {\
      InjectABS(time,EV_ABS,ABS_MT_TRACKING_ID,12);\
      InjectABS(time,EV_ABS,ABS_MT_POSITION_X,x);\
      InjectABS(time,EV_ABS,ABS_MT_POSITION_Y,y);\
      InjectABS(time,EV_ABS,SYN_MT_REPORT,0);\
      InjectABS(time,EV_SYN,SYN_REPORT,0);}

   #define SENDMOUSE(time,x,y)  {\
      InjectABS(time,EV_ABS,ABS_X,x);\
      InjectABS(time,EV_ABS,ABS_Y,y);\
      InjectABS(time,EV_SYN,SYN_REPORT,0);}
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
    xcbDisplay = XOpenDisplay(NULL);
    xcbConnection = xcb_connect(NULL, NULL);
    if (xcb_connection_has_error(xcbConnection)) {
        LOGE("Cannot open display %p",xcbConnection);
        return E_ERROR;
    }
    xcbSetup = xcb_get_setup(xcbConnection);
    xcb_screen_iterator_t iter = xcb_setup_roots_iterator(xcbSetup);
    xcb_screen_t *screen = iter.data;
    
    xcb_format_iterator_t iterator = xcb_setup_pixmap_formats_iterator(xcbSetup);

    while (iterator.rem) {
        xcb_format_t *format = iterator.data;
        LOGD("depth=%d bits_per_pixel=%d scanline_pad=%d",format->depth,format->bits_per_pixel,format->scanline_pad);
        xcb_format_next(&iterator);
    } 
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
    uint32_t value_list[4] = { xcbPixmap, screen->white_pixel,
        XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE
        |XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE | XCB_EVENT_MASK_POINTER_MOTION};

    xcb_create_window(xcbConnection, XCB_COPY_FROM_PARENT,
        xcbWindow, screen->root, 0, 0, width, height, 4,
        XCB_WINDOW_CLASS_INPUT_OUTPUT,
        screen->root_visual, value_mask, value_list);
    
    xcb_change_property(xcbConnection, XCB_PROP_MODE_REPLACE, xcbWindow,
            XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8, 6,"CDROID"); 

    const uint32_t gc_mask = XCB_GC_FOREGROUND | XCB_GC_GRAPHICS_EXPOSURES;
    const uint32_t gc_values[] = {screen->black_pixel, 0};
    xcbGC = xcb_generate_id(xcbConnection);
    xcb_create_gc(xcbConnection, xcbGC, xcbPixmap, gc_mask, gc_values);
    
    xcb_map_window(xcbConnection, xcbWindow);
    xcb_flush(xcbConnection);

    LOGD("xcbConnection=%p xcbWindow=%p(%dx%dx%d) maxlen=%d",xcbConnection,xcbWindow,width,height,screen->root_depth,xcb_get_maximum_request_length(xcbConnection));
    pthread_t xThreadId;
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

static void XCBExpose(int x,int y,int width,int height){
    xcb_expose_event_t ev;
    ev.response_type = XCB_EXPOSE;
    ev.window = xcbWindow;
    ev.x=x;  ev.y=y;
    ev.width = width;
    ev.height= height;
    ev.count=0;
    xcb_send_event(xcbConnection, 0, xcbWindow, XCB_EVENT_MASK_EXPOSURE, (char *)&ev);
    //xcb_clear_area(xcbConnection,0,xcbWindow,x,y,width,height);
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
        XCBExpose(rec.x,rec.y,rec.w,rec.h);
    }
    return E_OK;
}

int32_t GFXFlip(GFXHANDLE surface) {
    xcb_image_t *img=(xcb_image_t*)surface;
    if(mainSurface==surface) {
        GFXRect rect= {0,0,img->width,img->height};
        xcb_flush(xcbConnection);
    }
    return 0;
}

int32_t GFXCreateSurface(int dispid,GFXHANDLE*surface,uint32_t width,uint32_t height,int32_t format,bool hwsurface) {
    xcb_image_t*img=NULL;
    if(hwsurface) {
        width += screenMargin.x + screenMargin.w;
        height+= screenMargin.y + screenMargin.h;
    }
    img = xcb_image_create(width,height,XCB_IMAGE_FORMAT_Z_PIXMAP,32/*xpad*/,24,32,0,
                XCB_IMAGE_ORDER_LSB_FIRST, XCB_IMAGE_ORDER_LSB_FIRST,NULL, width*height*4, NULL);
    *surface=img;
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
    uint8_t*pbs = (uint8_t*)nsrc->data;
    uint8_t*pbd = (uint8_t*)ndst->data;
    rs.w = nsrc->width;
    rs.h = nsrc->height;
    if(srcrect)rs = *srcrect;
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
        LOGV("src (%d,%d,%d,%d) dst (%d,%d,%d,%d)",rs.x,rs.y,rs.w,rs.h,dx,dy,rs.w,rs.h);
        xcb_image_t*subimage=xcb_image_subimage(mainSurface,rs.x,rs.y,rs.w,rs.h,NULL,mainSurface->size,NULL);
        xcb_image_put(xcbConnection,xcbPixmap,xcbGC,subimage,dx,dy,0);
        xcb_image_destroy(subimage);
        XCBExpose(dx,dy,rs.w,rs.h);
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
    while ((event = xcb_wait_for_event(xcbConnection))) {
        const int eventType =event->response_type & ~0x80;
        switch (eventType) {
        case XCB_EXPOSE: {
            xcb_expose_event_t *expose = (xcb_expose_event_t *) event;
            if(mainSurface){
                xcb_copy_area(xcbConnection, xcbPixmap, xcbWindow,xcbGC,expose->x,expose->y,expose->x,expose->y, expose->width,expose->height);
                xcb_flush(xcbConnection);
                LOGV("Expose event on window 0x%08x (%d,%d,%d,%d)count=%d", expose->window,expose->x,expose->y,expose->width,expose->height,expose->count);
            }
            break;
        }
        case XCB_KEY_PRESS:
        case XCB_KEY_RELEASE:{
            xcb_key_press_event_t *key_press = (xcb_key_press_event_t *) event;
            KeySym keysym = XkbKeycodeToKeysym(xcbDisplay, key_press->detail, 0, 0);
            for(int i=0;i<sizeof(X11KEY2CD)/sizeof(X11KEY2CD[0]);i++){
                if(keysym==X11KEY2CD[i].xkey){
                    SENDKEY(X11KEY2CD[i].key,eventType==XCB_KEY_PRESS);
                    break;
                }
            }
            LOGD("Key press event: key %d, state %d keyname=%s", key_press->detail, key_press->state,XKeysymToString(keysym));
            break;
        }
        case XCB_BUTTON_PRESS:
        case XCB_BUTTON_RELEASE:{
            xcb_button_press_event_t *bp = (xcb_button_press_event_t *) event;
            InjectABS(bp->time,EV_KEY,BTN_TOUCH,eventType==XCB_BUTTON_PRESS?1:0);
            SENDMOUSE(bp->time,bp->event_x - screenMargin.x, bp->event_y - screenMargin.y);
            break;
        }
        case XCB_MOTION_NOTIFY:{
            xcb_motion_notify_event_t *mv = (xcb_motion_notify_event_t *)event;
            SENDMOUSE(mv->time,mv->event_x - screenMargin.x, mv->event_y - screenMargin.y);
            LOGV("Motion Notify: x=%d, y=%d", mv->event_x, mv->event_y);
        }
        default:
            LOGV("Unknown event type: %d", eventType);
            break;
        }
        free(event);
    }
    xcb_destroy_window(xcbConnection, xcbWindow);
    xcb_disconnect(xcbConnection);
    LOGD("XCBEventProc End.");
    return NULL;
}

