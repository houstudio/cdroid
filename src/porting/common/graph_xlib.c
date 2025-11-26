#include <cdtypes.h>
#include <cdgraph.h>
#include <cdinput.h>
#include <cdlog.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <sys/ipc.h>
#include <pthread.h>
#include <string.h>
#include <linux/input.h>
#include <time.h>
#include <pixman.h>
static pthread_t xThreadId;
static Display*x11Display=NULL;
static Window x11Window=0;
static Visual *x11Visual=NULL;
static Atom WM_DELETE_WINDOW;
static GC mainGC=0;
static XImage*mainSurface=NULL;
static void* X11EventProc(void*p);
static GFXRect screenMargin= {0}; //{60,0,60,0};
#define USE_PIXMAN 1
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
    if(x11Display) {
        XSelectInput(x11Display,x11Window,0);
        XDestroyWindow(x11Display,x11Window);
        XCloseDisplay(x11Display);
        x11Display=NULL;
    }
    pthread_destroy(xThreadId);
}

int32_t GFXInit() {
    if(x11Display)return E_OK;
    XInitThreads();
    x11Display=XOpenDisplay(NULL);
    if(x11Display) {
        XSetWindowAttributes winattrs;
        XGCValues values;
        XSizeHints sizehints;
        int width,height;
        int screen=DefaultScreen(x11Display);
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
        x11Visual = DefaultVisual(x11Display, screen);
        GFXGetDisplaySize(0,&width,&height);
        width += screenMargin.x + screenMargin.w;
        height+= screenMargin.y + screenMargin.h;
        x11Window=XCreateSimpleWindow(x11Display, RootWindow(x11Display, screen), 0, 0,width,height,
                    1, BlackPixel(x11Display, screen), WhitePixel(x11Display, screen));
        LOGI("screenMargin=(%d,%d,%d,%d)[%s]",screenMargin.x,screenMargin.y,screenMargin.w,screenMargin.h,strMargin);
#if 0
        sizehints.flags = PMinSize | PMaxSize;
        sizehints.min_width = width;
        sizehints.max_width = width;
        sizehints.min_height = height;
        sizehints.max_height = height;
        XSetWMNormalHints(x11Display,x11Window,&sizehints);

        XRenderPictFormat* pictFormat = XRenderFindVisualFormat(x11Display, x11Visual);
        Picture picture = XRenderCreatePicture(x11Display, x11Window, pictFormat, 0, NULL);
        shminfo.shmid = shmget(IPC_PRIVATE, width*height*4, IPC_CREAT | 0666);
        shminfo.shmaddr = shmat(shminfo.shmid, 0, 0);
        shminfo.readOnly = False;
        XShmAttach(x11Display, &shminfo);
#endif
        WM_DELETE_WINDOW = XInternAtom(x11Display, "WM_DELETE_WINDOW", False);
        XSetWMProtocols(x11Display,x11Window, &WM_DELETE_WINDOW, 1);
        mainGC = XCreateGC(x11Display,x11Window,0, &values);
        XSelectInput(x11Display, x11Window, ExposureMask | KeyPressMask|KeyReleaseMask |ResizeRedirectMask|
                     ButtonPressMask | ButtonReleaseMask | PointerMotionMask | Button1MotionMask | Button2MotionMask );
        XMapWindow(x11Display,x11Window);
        pthread_create(&xThreadId,NULL,X11EventProc,NULL);
    }
    atexit(onExit);
    return E_OK;
}

int32_t GFXGetDisplaySize(int dispid,uint32_t*width,uint32_t*height) {
    const char*env= getenv("SCREEN_SIZE");
    if(env==NULL) {
        *width=1280;//dispCfg.width;
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
    XImage*img=(XImage*)surface;
    *buffer=img->data;
    *pitch=img->bytes_per_line;
    return 0;
}

int32_t GFXGetSurfaceInfo(GFXHANDLE surface,uint32_t*width,uint32_t*height,int32_t *format) {
    XImage*img=(XImage*)surface;
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

static void  X11Expose(int x,int y,int w,int h) {
    XExposeEvent e;
    memset(&e,0,sizeof(e));
    e.type = Expose;
    e.send_event=True;
    e.display= x11Display;
    e.window = x11Window;
    e.x = x ;
    e.y = y ;
    e.width = w;
    e.height=h;
    e.count=1;
    if(x11Display&&mainSurface) {
        XPutImage(x11Display,x11Window,mainGC,mainSurface,x,y,x,y,w,h);
        //XSendEvent(x11Display,x11Window,False,0,(XEvent*)&e);
        //XPutBackEvent(x11Display,(XEvent*)&e);
    }
}

static pixman_color_t argb32_to_pixman_color(uint32_t argb) {
    pixman_color_t color;
    color.alpha = (argb >> 24) & 0xFF * 0xFFFF / 0xFF; // Alpha: 0-255 -> 0-65535
    color.red   = (argb >> 16) & 0xFF * 0xFFFF / 0xFF; // Red: 0-255 -> 0-65535
    color.green = (argb >> 8)  & 0xFF * 0xFFFF / 0xFF; // Green: 0-255 -> 0-65535
    color.blue  = (argb >> 0)  & 0xFF * 0xFFFF / 0xFF; // Blue: 0-255 -> 0-65535
    return color;
}
int32_t GFXFillRect(GFXHANDLE surface,const GFXRect*rect,uint32_t color) {
    XImage*img=(XImage*)surface;
    uint32_t x,y;
    GFXRect rec= {0,0,0,0};
    rec.w=img->width;
    rec.h=img->height;
    if(rect)rec=*rect;
    LOGV("FillRect %p %d,%d-%d,%d color=0x%x pitch=%d",img,rec.x,rec.y,rec.w,rec.h,color,img->bytes_per_line);
#ifndef USE_PIXMAN
    uint32_t*fb=(uint32_t*)(img->data+img->bytes_per_line*rec.y+rec.x*4);
    uint32_t*fbtop=fb;
    for(x=0; x<rec.w; x++)fb[x]=color;
    for(y=1; y<rec.h; y++) {
        fb+=(img->bytes_per_line>>2);
        memcpy(fb,fbtop,rec.w*4);
    }
#else
    pixman_color_t pmcolor=argb32_to_pixman_color(color);
    pixman_rectangle16_t rec16={rec.x,rec.y,rec.w,rec.h};
    pixman_image_t *src_image = pixman_image_create_bits(PIXMAN_a8r8g8b8, img->width, img->height, img->data, img->bytes_per_line);
    pixman_image_fill_rectangles(PIXMAN_OP_SRC, src_image, &pmcolor, 1, &rec16);
    pixman_image_unref(src_image);
#endif
    if(surface==mainSurface) {
        X11Expose(rec.x,rec.y,rec.w,rec.h);
    }
    return E_OK;
}

int32_t GFXFlip(GFXHANDLE surface) {
    XImage *img=(XImage*)surface;
    if(mainSurface==surface) {
        GFXRect rect= {0,0,img->width,img->height};
        //X11Expose(0,0,img->width,img->height);//very slowly
    }
    return 0;
}

int32_t GFXCreateSurface(int dispid,GFXHANDLE*surface,uint32_t width,uint32_t height,int32_t format,bool hwsurface) {
    XImage*img=NULL;
    if(x11Display) {
        int imagedepth = DefaultDepth(x11Display,DefaultScreen(x11Display));
        if(hwsurface) {
            width += screenMargin.x + screenMargin.w;
            height+= screenMargin.y + screenMargin.h;
        }
        img=XCreateImage(x11Display,x11Visual, imagedepth,ZPixmap,0,NULL,width,height,32,width*4);
    } else {
        img=(XImage*)malloc(sizeof(XImage));
        img->width=width;
        img->height=height;
        img->bits_per_pixel=32;
        img->bytes_per_line=width*4;
    }
    img->data=(char*)malloc(height*img->bytes_per_line);
    *surface=img;
    LOGD("%p  size=%dx%dx%d %db",img,width,height,img->bytes_per_line,img->bits_per_pixel);
    if(hwsurface) {
        mainSurface=img;
    }
    return E_OK;
}


int32_t GFXBlit(GFXHANDLE dstsurface,int dx,int dy,GFXHANDLE srcsurface,const GFXRect* srcrect) {
    XImage *ndst=(XImage*)dstsurface;
    XImage *nsrc=(XImage*)srcsurface;
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
#ifndef USE_PIXMAN
    pbs += rs.y*nsrc->bytes_per_line+rs.x*4;
    pbd += dy*ndst->bytes_per_line+dx*4;
    if(ndst==mainSurface) {
        pbd += (screenMargin.x*4 + screenMargin.h*ndst->bytes_per_line);
    }
    for(unsigned int y=0; y<rs.h; y++) {
        memcpy(pbd,pbs,rs.w*4);
        pbs+=nsrc->bytes_per_line;
        pbd+=ndst->bytes_per_line;
    }
#else
    pixman_image_t *src_image = pixman_image_create_bits(PIXMAN_a8r8g8b8, nsrc->width, nsrc->height, nsrc->data, nsrc->bytes_per_line);
    pixman_image_t *dst_image = pixman_image_create_bits(PIXMAN_a8r8g8b8, ndst->width, ndst->height, ndst->data, ndst->bytes_per_line);
    pixman_image_composite(PIXMAN_OP_SRC, src_image,NULL/*mask*/, dst_image,
                       rs.x, rs.y, 0, 0, dx, dy, rs.w, rs.h);
    pixman_image_unref(src_image);
    pixman_image_unref(dst_image);    
#endif
    LOGV("src (%d,%d,%d,%d) dst (%d,%d,%d,%d)",rs.x,rs.y,rs.w,rs.h,dx,dy,rs.w,rs.h);
    if((ndst==mainSurface)&&x11Display) {
        X11Expose(dx+screenMargin.x,dy+screenMargin.h,rs.w,rs.h);
    }
    return 0;
}

int32_t GFXDestroySurface(GFXHANDLE surface) {
    XDestroyImage((XImage*)surface);
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

static void* X11EventProc(void*p) {
    XEvent event;
    int i,keysym,key=0,down;
    while(x11Display) {
        int rc=XNextEvent(x11Display, &event);
        switch(event.type) {
        case Expose:
            if(mainSurface) {
                XExposeEvent e=event.xexpose;
                XPutImage(x11Display,x11Window,mainGC,mainSurface,e.x,e.y,e.x,e.y,e.width,e.height);
            }
            break;
        case ConfigureNotify:
            XPutImage(x11Display,x11Window,mainGC,mainSurface,0,0,0,0,mainSurface->width,mainSurface->height);
            break;
        case KeyPress:/* 当检测到键盘按键,退出消息循环 */
        case KeyRelease:
            down=event.type==KeyPress;
            keysym=XLookupKeysym((XKeyEvent*)&event,0);
            for(i=0;i<sizeof(X11KEY2CD)/sizeof(X11KEY2CD[0]);i++){
                if(keysym==X11KEY2CD[i].xkey){
                    SENDKEY((key=X11KEY2CD[i].key),down);
                    break;
                }
            }
            LOGV("keycode:%d sym:%d %d",event.xkey.keycode,keysym,key);
            break;
        case ButtonPress:
        case ButtonRelease:
            if(1==event.xbutton.button) {
                InjectABS(event.xbutton.time,EV_KEY,BTN_TOUCH,(event.type==ButtonPress)?1:0);
                SENDMOUSE(event.xbutton.time,event.xbutton.x - screenMargin.x, event.xbutton.y - screenMargin.y);
            }
            break;
        case MotionNotify:
            if(event.xmotion.state&Button1MotionMask) {
                SENDMOUSE(event.xmotion.time,event.xmotion.x - screenMargin.x, event.xmotion.y - screenMargin.y);
            }
            break;
        case DestroyNotify:
            LOGD("====X11Window::Destroied");
            return 0;
        case ClientMessage:
            if ( (Atom) event.xclient.data.l[0] == WM_DELETE_WINDOW) {
                LOGD("====ClientMessage WM_DELETE_WINDOW");
                LOGD("GraphX11.Terminated");
                exit(0);
            }
            break;
        case UnmapNotify:
            LOGD("===UnmapNotify ");
            break;
        default:
            LOGD("event.type=%d",event.type);
            break;
        };
        if(x11Display==0)break;
    }
}

