#include <stdio.h>
#include <cdgraph.h>
#include <cdlog.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <cdinput.h>
#include <core/eventcodes.h>
#include <X11/Xlib.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <pixman.h>
typedef struct {
    Display*display;
    Window*window;
    GLXContext glc;
    int width;
    int height;
    int pitch;
    GLuint texture;
    char* buffer;
} FBDEVICE;

typedef struct {
    int dispid;
    GLuint texture;
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
    int format;
    int ishw;
    char*buffer;
} FBSURFACE;

static FBDEVICE devs[2]= {0};
static GFXRect screenMargin= {0};
#define SENDKEY(time,k,down) {InjectKey(time,EV_KEY,k,down);}
#if 1
#define SENDMOUSE(time,x,y)  {\
      InjectABS(time,EV_ABS,ABS_MT_TRACKING_ID,12);\
      InjectABS(time,EV_ABS,ABS_MT_POSITION_X,x);\
      InjectABS(time,EV_ABS,ABS_MT_POSITION_Y,y);\
      InjectABS(time,EV_ABS,SYN_MT_REPORT,0);\
      InjectABS(time,EV_SYN,SYN_REPORT,0);}
#else
#define SENDMOUSE(time,x,y)  {\
      InjectABS(time,EV_ABS,ABS_X,x);\
      InjectABS(time,EV_ABS,ABS_Y,y);\
      InjectABS(time,EV_SYN,SYN_REPORT,0);}
#endif
static void InjectKey(unsigned long etime,int type, int code, int value) {
    INPUTEVENT i = { 0 };
    i.tv_sec = etime/1000;
    i.tv_usec = (etime%1000)*1000;
    i.type = type;
    i.code = code;
    i.value = value;
    i.device = INJECTDEV_KEY;
    InputInjectEvents(&i, 1, 1);
}

static void InjectABS(unsigned long etime, int type, int axis, int value);
static void DrawScene(int width, int height);
static unsigned int xglThreadProc(void * param);

int32_t GFXInit() {
    if(devs[0].display)return E_OK;
    memset(devs,0,sizeof(devs));
    FBDEVICE*dev=&devs[0];
    const char*strMargins=getenv("SCREEN_MARGINS");
    const char* DELIM=",;";
    pthread_t thread1;
    if(strMargins){
        char *sm  = strdup(strMargins);
        char*token= strtok(sm,DELIM);
        screenMargin.x = atoi(token);
        token = strtok(NULL,DELIM);
        screenMargin.y = atoi(token);
        token = strtok(NULL,DELIM);
        screenMargin.w = atoi(token);
        token = strtok(NULL,DELIM);
        screenMargin.h = atoi(token);
        free(sm);
    }
    pthread_create(&thread1, NULL,xglThreadProc, NULL);
    sleep(2);
    return E_OK;
}

int32_t GFXGetDisplayCount() {
    return 1;
}

int32_t GFXGetDisplaySize(int dispid,uint32_t*width,uint32_t*height) {
    if(dispid<0||dispid>=GFXGetDisplayCount())return E_ERROR;
    FBDEVICE*dev=devs+dispid;
    *width =dev->width -(screenMargin.x + screenMargin.w);
    *height=dev->height-(screenMargin.y + screenMargin.h);
    LOGV("screen[%d]size=%dx%d/%dx%d",dispid,*width,*height,dev->width,dev->height);
    return E_OK;
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
    uint32_t x,y;
    GFXRect rec= {0,0,0,0};
    rec.w=ngs->width;
    rec.h=ngs->height;
    if(rect)rec=*rect;
    LOGV("FillRect %p %d,%d-%d,%d color=0x%x pitch=%d",ngs,rec.x,rec.y,rec.w,rec.h,color,ngs->pitch);
    uint32_t* fb = (uint32_t*)(ngs->buffer + ngs->pitch * rec.y + rec.x * 4);
    uint32_t* fbtop = fb;
    for (x = 0; x < rec.w; x++)fb[x] = color;
    const int cpw = rec.w * 4;
    for (y = 1; y < rec.h; y++) {
        fb += (ngs->pitch >> 2);
        memcpy(fb, fbtop, cpw);
    }
    return E_OK;
}

int32_t GFXFlip(GFXHANDLE surface) {
    FBSURFACE*surf=(FBSURFACE*)surface;
    FBDEVICE*dev=devs+surf->dispid;
    if(surf->ishw) {
	XEvent event;
	event.type = Expose;
        event.xexpose.window = devs[0].window;
        event.xexpose.x = 0;
        event.xexpose.y = 0;
        event.xexpose.width = surf->width;
        event.xexpose.height = surf->height;
        event.xexpose.count = 0;

        XSendEvent(devs[0].display, devs[0].window, False, ExposureMask, &event);
        //XFlush(devs[0].display);
    }
    return 0;
}

int32_t GFXCreateSurface(int dispid,GFXHANDLE*surface,uint32_t width,uint32_t height,int32_t format,bool hwsurface) {
    FBSURFACE*surf=(FBSURFACE*)malloc(sizeof(FBSURFACE));
    FBDEVICE*dev = &devs[dispid];
    surf->dispid=dispid;
    surf->width= hwsurface?dev->width:width;
    surf->height=hwsurface?dev->height:height;
    surf->format=format;
    surf->ishw=hwsurface;
    surf->pitch=width*4;

    if(hwsurface){
        surf->texture=devs[0].texture;
        surf->buffer=devs[0].buffer;
    } else {
        surf->buffer = malloc(surf->pitch * surf->height);
    }
    surf->ishw=hwsurface;
    LOGI("surface=%x buf=%p size=%dx%dx%d hw=%d",surf,surf->buffer,width,height,surf->pitch,hwsurface);
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
#if 0
    pbs += rs.y * nsrc->pitch + rs.x * 4;
    if (ndst->ishw == 0)pbd += dy * ndst->pitch + dx * 4;
    else pbd += (dy + screenMargin.y) * ndst->pitch + (dx + screenMargin.x) * 4;
    const int cpw = rs.w * 4;
    for (y = 0; y < rs.h; y++) {
        memcpy(pbd, pbs, cpw);
        pbs += nsrc->pitch;
        pbd += ndst->pitch;
    }
#else
    pixman_image_t *src_image = pixman_image_create_bits(PIXMAN_a8r8g8b8, nsrc->width, nsrc->height, (uint32_t*)nsrc->buffer, nsrc->pitch);
    pixman_image_t *dst_image = pixman_image_create_bits(PIXMAN_a8r8g8b8, ndst->width, ndst->height, (uint32_t*)ndst->buffer, ndst->pitch);
    pixman_image_composite(PIXMAN_OP_SRC, src_image,NULL/*mask*/, dst_image,
                       rs.x, rs.y, 0, 0, dx, dy, rs.w, rs.h);
    pixman_image_unref(src_image);
    pixman_image_unref(dst_image);
#endif
    //if(ndst->ishw) X11Expose(dx,dy,rs.w,rs.h);
    return 0;
}

int32_t GFXDestroySurface(GFXHANDLE surface) {
    FBSURFACE* surf = (FBSURFACE*)surface;
    FBDEVICE* dev = devs + surf->dispid;
    if (!surf->ishw) {
        free(surf->buffer);
        surf->buffer = NULL;
    }
    free(surf);
    return 0;
}

static void InjectABS(unsigned long etime, int type, int axis, int value) {
    INPUTEVENT i = { 0 };
    i.tv_sec = etime / 1000;
    i.tv_usec = (etime % 1000) * 1000;
    i.type = type;
    i.code = axis;
    i.value = value;
    i.device = INJECTDEV_TOUCH;
    InputInjectEvents(&i, 1, 1);
}

static void DrawScene(int width,int height) {
    if (devs[0].buffer == NULL)return;

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, devs[0].texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, devs[0].buffer);

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, 0.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0f, -1.0f, 0.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(1.0f, 1.0f, 0.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f, 1.0f, 0.0f);
    glEnd();

    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);
    glFlush();
    glXSwapBuffers(devs[0].display, devs[0].window);
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

static unsigned int xglThreadProc(void * param)
{
    int width = 1280, height = 720;
    Colormap cmap;
    XSetWindowAttributes swa;
    Atom wm_delete_window;
    char* env = getenv("SCREEN_SIZE");
    if (env) {
        width = atoi(env);
        env = strpbrk(env, "xX*,");
        if (env)height = atoi(env + 1);
    }
    LOGI("====GL_EXT_bgra=%d", isExtensionSupported("GL_EXT_bgra"));
    devs[0].display = XOpenDisplay(NULL);
    devs[0].width = width;
    devs[0].height = height;
    devs[0].pitch = width * 4;
    devs[0].buffer = (char*)malloc(width * height * 4);

    int screen = DefaultScreen(devs[0].display);
    int glp[] = {GLX_RGBA, GLX_DEPTH_SIZE, 16, GLX_DOUBLEBUFFER, None};
    XVisualInfo *vi = glXChooseVisual(devs[0].display, screen, glp);

    cmap = XCreateColormap(devs[0].display, RootWindow(devs[0].display, vi->screen), vi->visual, AllocNone);
    swa.colormap = cmap;
    swa.event_mask = ExposureMask | KeyPressMask;

    devs[0].window = XCreateWindow(devs[0].display, RootWindow(devs[0].display, vi->screen), 0, 0, width, height, 0, vi->depth,
                           InputOutput, vi->visual, CWColormap | CWEventMask, &swa);
    XStoreName(devs[0].display, devs[0].window, "CDROID");

    XSelectInput(devs[0].display, devs[0].window, ExposureMask | KeyPressMask|KeyReleaseMask |ResizeRedirectMask|
                     ButtonPressMask | ButtonReleaseMask | PointerMotionMask | Button1MotionMask | Button2MotionMask );

    devs[0].glc = glXCreateContext(devs[0].display, vi, NULL, GL_TRUE);

    glXMakeCurrent(devs[0].display, devs[0].window, devs[0].glc);
    XMapWindow(devs[0].display, devs[0].window);

    
    glGenTextures(1, &devs[0].texture);
    glBindTexture(GL_TEXTURE_2D, devs[0].texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, devs[0].buffer);

    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1.0, 1.0, 1.0, -1.0, -1.0, 1.0);
    //gluOrtho2D(-1.0, 1.0, 1.0, -1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    //SetEvent((HANDLE)param);
    wm_delete_window = XInternAtom(devs[0].display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(devs[0].display, devs[0].window, &wm_delete_window, 1);
    while(1){
        XEvent event;
	int i,keysym,key=0,down;
        XNextEvent(devs[0].display,&event);
        switch(event.type){
        case Expose:
            DrawScene(width,height);break;
	case ClientMessage:
	    //DrawScene(width,height);
            if ((Atom)event.xclient.data.l[0] == wm_delete_window) {
                LOGD("GraphXGL.Terminated");
                goto end;
            }break;
        case KeyPress:/* 真真真真,真真真 */
        case KeyRelease:
            down=event.type==KeyPress;
            keysym=XLookupKeysym((XKeyEvent*)&event,0);
            for(i=0;i<sizeof(X11KEY2CD)/sizeof(X11KEY2CD[0]);i++){
                if(keysym==X11KEY2CD[i].xkey){
                    SENDKEY(event.xmotion.time,(key=X11KEY2CD[i].key),down);
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
        case UnmapNotify:
            LOGD("===UnmapNotify ");
            break;
        default:
            LOGD("event.type=%d",event.type);
            break;

        }
    }
end:
    glXMakeCurrent(devs[0].display, None, NULL);
    glXDestroyContext(devs[0].display, devs[0].glc);
    XDestroyWindow(devs[0].display, devs[0].window);
    XCloseDisplay(devs[0].display);
    LOGD("XGL Thread Exited");
    return 0;
}

