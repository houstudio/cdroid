#include <cdtypes.h>
#include <ngl_os.h>
#include <cdgraph.h>
#include <cdinput.h>
#include <cdlog.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>
#include <pthread.h>
#include <string.h>
#include <eventcodes.h>
#ifdef ENABLE_RFB
#include <rfb/rfb.h>
#include <rfb/keysym.h>
#include <rfb/rfbproto.h>
#endif

static Display*x11Display=NULL;
static Window x11Window=0;
static Visual *x11Visual=NULL;
static Atom WM_DELETE_WINDOW;
static GC mainGC=0;
static XImage*mainSurface=NULL;
static void* X11EventProc(void*p);

#define SENDKEY(k,down) {InjectKey(EV_KEY,k,down);}
#define SENDMOUSE(x,y)  {InjectABS(EV_ABS,0,x);InjectABS(EV_ABS,1,y);InjectABS(EV_SYN,SYN_REPORT,0);}

#ifdef ENABLE_RFB
static rfbScreenInfoPtr rfbScreen=NULL;
static enum rfbNewClientAction onNewClient(rfbClientPtr cl);
static void onChatText(struct _rfbClientRec* cl, int length, char *string);
static void onVNCClientKey(rfbBool down,rfbKeySym key,rfbClientPtr cl);
static void onMousePtr(int buttonMask,int x,int y,rfbClientPtr cl);
typedef struct ClientData {
    rfbBool oldButton;
    int oldx,oldy;
} RFBClientData;
#endif

static void InjectKey(int type,int code,int value){
    INPUTEVENT i={0};
    i.type=type;
    i.code=code;
    i.value=value;
    i.device=INJECTDEV_KEY;
    InputInjectEvents(&i,1,1);
}
static void InjectABS(int type,int axis,int value){
    INPUTEVENT i={0};
    i.type=type;
    i.code=axis;
    i.value=value;
    i.device=INJECTDEV_PTR;
    InputInjectEvents(&i,1,1);
}
static void onExit(){
    LOGD("X11 Graph shutdown!");
    if(x11Display){
        XDestroyWindow(x11Display,x11Window);
        XCloseDisplay(x11Display);
        x11Display=NULL;
    }
    #ifdef ENABLE_RFB
    rfbShutdownServer(rfbScreen,1);
    rfbScreenCleanup(rfbScreen);
    #endif
}

DWORD GFXInit(){
    if(x11Display)return E_OK;
    #ifdef ENABLE_RFB
    if(rfbScreen) return E_OK;
    rfbScreen = rfbGetScreen(NULL,NULL,1280,720,8,3,3);
    rfbScreen->desktopName = "CDROID-RFB";
    rfbScreen->frameBuffer = NULL;
    rfbScreen->alwaysShared = (1==1);
    rfbScreen->kbdAddEvent=onVNCClientKey;
    rfbScreen->ptrAddEvent=onMousePtr;
    rfbScreen->newClientHook=onNewClient;
    rfbScreen->setTextChat=onChatText;
    rfbScreen->permitFileTransfer=-1;
    rfbScreen->bitsPerPixel=24;
    rfbInitServer(rfbScreen);
    rfbRunEventLoop(rfbScreen,5,TRUE);//non block
    LOGD("VNC Server Inited rfbScreen=%p port=%d framebuffer=%p",rfbScreen,rfbScreen->port,rfbScreen->frameBuffer);
    #endif 
    XInitThreads(); 
    x11Display=XOpenDisplay(NULL);
    LOGD("x11Display=%p ",x11Display);
    if(x11Display){
        pthread_t tid;
        XSetWindowAttributes winattrs;
	    XGCValues values;
        int screen=DefaultScreen(x11Display);
        x11Visual = DefaultVisual(x11Display, screen);
        x11Window=XCreateSimpleWindow(x11Display, RootWindow(x11Display, screen), 0, 0,1280,720, 1,
                BlackPixel(x11Display, screen), WhitePixel(x11Display, screen));

        WM_DELETE_WINDOW = XInternAtom(x11Display, "WM_DELETE_WINDOW", False);
        XSetWMProtocols(x11Display,x11Window, &WM_DELETE_WINDOW, 1);
	    mainGC = XCreateGC(x11Display,x11Window,0, &values);
        XSelectInput(x11Display, x11Window, ExposureMask | KeyPressMask|KeyReleaseMask |ResizeRedirectMask|
                  ButtonPressMask | ButtonReleaseMask | PointerMotionMask | Button1MotionMask | Button2MotionMask );
        XMapWindow(x11Display,x11Window);
        pthread_create(&tid,NULL,X11EventProc,NULL);
    }
    atexit(onExit);
    return E_OK;
}

DWORD GFXGetScreenSize(UINT*width,UINT*height){
    *width=1280;//dispCfg.width;
    *height=720;//dispCfg.height;
    return E_OK;
}

DWORD GFXLockSurface(HANDLE surface,void**buffer,UINT*pitch){
    XImage*img=(XImage*)surface;
    *buffer=img->data;
    *pitch=img->bytes_per_line;
    return 0;
}

DWORD GFXGetSurfaceInfo(HANDLE surface,UINT*width,UINT*height,INT *format)
{
    XImage*img=(XImage*)surface;
    *width=img->width;
    *height=img->height;
    if(format)*format=GPF_ARGB;
    return E_OK;
}

DWORD GFXUnlockSurface(HANDLE surface){
    return 0;
}

DWORD GFXSurfaceSetOpacity(HANDLE surface,BYTE alpha){
    return 0;//dispLayer->SetOpacity(dispLayer,alpha);
}

static void  X11Expose(int x,int y,int w,int h){
     XExposeEvent e;
     memset(&e,0,sizeof(e));
     e.type = Expose;
     e.send_event=True;
     e.display= x11Display;
     e.window = x11Window;
     e.x = x ;       e.y = y ;
     e.width = w;  e.height=h;
     e.count=1;
     if(x11Display&&mainSurface){
         XPutImage(x11Display,x11Window,mainGC,mainSurface,x,y,x,y,w,h);
         //XSendEvent(x11Display,x11Window,False,0,(XEvent*)&e);
         //XPutBackEvent(x11Display,(XEvent*)&e);
     }
}
DWORD GFXFillRect(HANDLE surface,const GFXRect*rect,UINT color){
    XImage*img=(XImage*)surface;
    UINT x,y;
    GFXRect rec={0,0,0,0};
    rec.w=img->width;
    rec.h=img->height;
    if(rect)rec=*rect;
    LOGV("FillRect %p %d,%d-%d,%d color=0x%x pitch=%d",img,rec.x,rec.y,rec.w,rec.h,color,img->bytes_per_line);
    UINT*fb=(UINT*)(img->data+img->bytes_per_line*rec.y+rec.x*4);
    for(y=0;y<rec.h;y++){
        for(x=0;x<rec.w;x++)
           fb[x]=color;
        fb+=(img->bytes_per_line>>2);
    }
   if(surface==mainSurface){
       X11Expose(rec.x,rec.y,rec.w,rec.h);
   }
   return E_OK;
}

DWORD GFXFlip(HANDLE surface){
    //(XFlush(x11Display);
    return 0;
}
#ifdef ENABLE_RFB
static void ResetScreenFormat(XImage*fb,int width,int height,int format){
    rfbPixelFormat*fmt=&rfbScreen->serverFormat;
    fmt->trueColour=TRUE;
    switch(format){
    case GPF_ARGB:
         fmt->bitsPerPixel=24;
         fmt->redShift=16;//bitsPerSample*2
         fmt->greenShift=8;
         fmt->blueShift=0;
         break;
    case GPF_ABGR:
         fmt->bitsPerPixel=32;
         fmt->redShift=0;
         fmt->greenShift=8;
         fmt->blueShift=16;
         break;
    default:return;
    }
    LOGD("format=%d",format);
    rfbNewFramebuffer(rfbScreen,fb->data,width,height,8,3,4);
}
#endif

DWORD GFXCreateSurface(HANDLE*surface,UINT width,UINT height,INT format,BOOL hwsurface)
{
    XImage*img=NULL;
    if(x11Display){
        int imagedepth = DefaultDepth(x11Display,DefaultScreen(x11Display));
        img=XCreateImage(x11Display,x11Visual, imagedepth,ZPixmap,0,NULL,width,height,32,0);
    }else{
        img=(XImage*)malloc(sizeof(XImage));
        img->width=width;
        img->height=height;
        img->bits_per_pixel=24;
        img->bytes_per_line=width*4;
    }
    img->data=(char*)malloc(width*height*img->bytes_per_line);
    *surface=img;
    LOGD("%p  size=%dx%dx%d %db",img,width,height,img->bytes_per_line,img->bits_per_pixel);
    if(hwsurface){
        mainSurface=img;
        #ifdef ENABLE_RFB
        rfbScreen->frameBuffer = img->data;
        ResetScreenFormat(img,width,height,format);
        #endif
    }
    return E_OK;
}


DWORD GFXBlit(HANDLE dstsurface,int dx,int dy,HANDLE srcsurface,const GFXRect* srcrect)
{
    unsigned int x,y,sw,sh;
    XImage *ndst=(XImage*)dstsurface;
    XImage*nsrc=(XImage*)srcsurface;
    GFXRect rs={0,0};
    BYTE*pbs=(BYTE*)nsrc->data;
    BYTE*pbd=(BYTE*)ndst->data;
    rs.w=nsrc->width;rs.h=nsrc->height;
    if(srcrect)rs=*srcrect;
    if(((int)rs.w+dx<=0)||((int)rs.h+dy<=0)||(dx>=(int)ndst->width)||(dy>=(int)ndst->height)||(rs.x<0)||(rs.y<0)){
        LOGV("dx=%d,dy=%d rs=(%d,%d-%d,%d)",dx,dy,rs.x,rs.y,rs.w,rs.h);
        return E_INVALID_PARA;
    }

    if(dx<0){ rs.x-=dx; rs.w=(int)rs.w+dx; dx=0;}
    if(dy<0){ rs.y-=dy; rs.h=(int)rs.h+dy; dy=0;}
    if(dx+rs.w > ndst->width ) rs.w=ndst->width-dx;
    if(dy+rs.h > ndst->height) rs.h=ndst->height-dy;

    //LOGV_IF(ndst==mainSurface,"Blit %p %d,%d-%d,%d -> %p %d,%d buffer=%p->%p",nsrc,rs.x,rs.y,rs.w,rs.h,ndst,dx,dy,pbs,pbd);
    pbs += rs.y*nsrc->bytes_per_line+rs.x*4;
    pbd += dy*ndst->bytes_per_line+dx*4;
    for(y=0;y<rs.h;y++){
        memcpy(pbd,pbs,rs.w*4);
        pbs+=nsrc->bytes_per_line;
        pbd+=ndst->bytes_per_line;
    }
    if(ndst==mainSurface){
        if(x11Display) X11Expose(dx,dy,rs.w,rs.h);
        #ifdef ENABLE_RFB
        rfbMarkRectAsModified(rfbScreen,dx,dy,dx+rs.w,dy+rs.h);
        #endif
    }
    return 0;
}

DWORD GFXDestroySurface(HANDLE surface)
{
    XDestroyImage((XImage*)surface);
    return 0;
}

static void* X11EventProc(void*p){
    const static int btns[]={BTN_LEFT,BTN_MIDDLE,BTN_RIGHT,BTN_WHEEL,0,0};
    XEvent event;
    int down;
    while(x11Display){
        int rc=XNextEvent(x11Display, &event);
        switch(event.type){
        case Expose:if(mainSurface){
            XExposeEvent e=event.xexpose;
            LOGV("event.xexpose=%d,%d-%d,%d",e.x,e.y,e.width,e.height);
            XPutImage(x11Display,x11Window,mainGC,mainSurface,e.x,e.y,e.x,e.y,e.width,e.height);
            }break;
        case ConfigureNotify:
            XPutImage(x11Display,x11Window,mainGC,mainSurface,0,0,0,0,mainSurface->width,mainSurface->height);
            break;
        case KeyPress:/* 当检测到键盘按键,退出消息循环 */
        case KeyRelease:
            down=event.type==KeyPress;
            switch(event.xkey.keycode){
            case 0x13:   SENDKEY(KEY_0,down); break;
            case 0x0a ... 0x12:   SENDKEY(KEY_1+event.xkey.keycode-0x0a,down);  break;
            case 0x43 ... 0x4d:   SENDKEY(KEY_F1+event.xkey.keycode-0x43,down); break;
            case 0x5F ... 0x60:   SENDKEY(KEY_F11+event.xkey.keycode-0x5F,down); break;
            case 0x64 :  SENDKEY(KEY_LEFT,down) ;  break;
            case 0x66 :  SENDKEY(KEY_RIGHT,down);  break;
            case 0x61 :  SENDKEY(KEY_HOME,down) ;  break;
            case 0x62 :  SENDKEY(KEY_UP,down)   ;  break;
            case 0x63 :  SENDKEY(KEY_PAGEUP,down); break;
            case 0x67 :  SENDKEY(KEY_MOVE_END,down);break;
            case 0x68 :  SENDKEY(KEY_DOWN,down) ;  break;
            case 0x69 :  SENDKEY(KEY_PAGEDOWN,down);break;
            case 0x6A :  SENDKEY(KEY_INSERT,down); break;
            case 0x6B :  SENDKEY(KEY_DELETE,down); break;
            case 0x6C :  SENDKEY(KEY_DPAD_CENTER,down);break;
            case 0x09 :  SENDKEY(KEY_ESCAPE,down); break;
            case 0x24 :  SENDKEY(KEY_ENTER,down);  break;
            case 0x41 :  SENDKEY(KEY_SPACE,down);  break;
            case 0x16 :  SENDKEY(KEY_BACKSPACE,down);break;
            case 0x75 :  SENDKEY(KEY_MENU,down) ;  break;

            case 0x18 :  SENDKEY(KEY_Q,down);break;
            case 0x19 :  SENDKEY(KEY_W,down);break;
            case 0x1A :  SENDKEY(KEY_E,down);break;
            case 0x1B :  SENDKEY(KEY_R,down);break;
            case 0x1C :  SENDKEY(KEY_T,down);break;
            case 0x1D :  SENDKEY(KEY_Y,down);break;
            case 0x1E :  SENDKEY(KEY_U,down);break;
            case 0x1F :  SENDKEY(KEY_I,down);break;
            case 0x20 :  SENDKEY(KEY_O,down);break;
            case 0x21 :  SENDKEY(KEY_P,down);break;

            case 0x26 :  SENDKEY(KEY_A,down);break;
            case 0x27 :  SENDKEY(KEY_S,down);break;
            case 0x28 :  SENDKEY(KEY_D,down);break;
            case 0x29 :  SENDKEY(KEY_F,down);break;
            case 0x2A :  SENDKEY(KEY_G,down);break;
            case 0x2B :  SENDKEY(KEY_H,down);break;
            case 0x2C :  SENDKEY(KEY_J,down);break;
            case 0x2D :  SENDKEY(KEY_K,down);break;
            case 0x2E :  SENDKEY(KEY_L,down);break;

            case 0x34 :  SENDKEY(KEY_Z,down);break;
            case 0x35 :  SENDKEY(KEY_X,down);break;
            case 0x36 :  SENDKEY(KEY_C,down);break;
            case 0x37 :  SENDKEY(KEY_V,down);break;
            case 0x38 :  SENDKEY(KEY_B,down);break;
            case 0x39 :  SENDKEY(KEY_N,down);break;
            case 0x3A :  SENDKEY(KEY_M,down);break;
            }
            LOGV("%d",event.xkey.keycode);
            break;
        case ButtonPress:
        case ButtonRelease:{
            int x=event.xbutton.x;
            int y=event.xbutton.y;
            int btnidx=event.xbutton.button;
            LOGV("Button%s pos=%d,%d btn=%d",(event.type==ButtonPress?"Press":"Release"),x,y,btnidx);
            InjectABS(EV_KEY,(event.type==ButtonPress)?1:0,btns[btnidx]);
            }break;
        case MotionNotify:{
            int x=event.xmotion.x;
            int y=event.xmotion.y;
            int state=event.xmotion.state;
            SENDMOUSE(x,y);
            LOGV_IF(state,"Motion (%d,%d) state=%d/%x",x,y,state,state);
            }
            break;
        case DestroyNotify:
            LOGD("====X11Window::Destroied");
            return 0;
        case ClientMessage:
	        if ( (Atom) event.xclient.data.l[0] == WM_DELETE_WINDOW){
                LOGD("====ClientMessage WM_DELETE_WINDOW");
                LOGD("GraphX11.Terminated");
                exit(0);
            }break;
        case UnmapNotify:
            LOGD("===UnmapNotify ");break;
        default:
            LOGD("event.type=%d",event.type);break;
        };
    }
}

///////////////////////////////////////RFB/////////////////////////////////////////////

#ifdef ENABLE_RFB
static void onClientDone(rfbClientPtr cl){
   free(cl->clientData);
   cl->clientData = NULL;
}

static enum rfbNewClientAction onNewClient(rfbClientPtr cl){
    cl->clientData = (void*)calloc(sizeof(RFBClientData),1);
    cl->clientGoneHook = onClientDone;
    return RFB_CLIENT_ACCEPT;
}
static int bitcount(int bits){
    int i,rc=0;
    for(i=0;i<32;i++)
       if(bits&(1<<i))rc++;
    return rc;
}
static int btnidx(int btn){
    int idx=-1;
    if(btn==0)return 0;
    while(btn){
        btn>>=1;
        idx++;
    }
    return idx;
}
static void onChatText(struct _rfbClientRec* cl, int length, char *string){
    LOGD("RCV ChatText:%s",string);
}

static void onMousePtr(int buttonMask,int x,int y,rfbClientPtr cl){
   RFBClientData* cd=cl->clientData;
   if(x>=0 && y>=0 && x<rfbScreen->width && y<rfbScreen->height) {
      int bc0=bitcount(cd->oldButton);
      int bc1=bitcount(buttonMask);
      int btns[]={BTN_LEFT,BTN_MIDDLE,BTN_RIGHT,BTN_WHEEL};
      int btn=cd->oldButton^buttonMask;
      SENDMOUSE(x,y);
      if(buttonMask) {//buttonmask 1->left 2-middle 4-right
         int i,j,x1,x2,y1,y2;
         if(cd->oldButton==buttonMask) { /* draw a line */
            rfbDrawLine(cl->screen,x,y,cd->oldx,cd->oldy,0x00FF);
            x1=x; y1=y;
            if(x1>cd->oldx) x1++; else cd->oldx++;
            if(y1>cd->oldy) y1++; else cd->oldy++;
            rfbMarkRectAsModified(cl->screen,x,y,cd->oldx,cd->oldy);
         } else { /* draw a point (diameter depends on button) */
            int w=cl->screen->paddedWidthInBytes;
            x1=x-buttonMask; if(x1<0) x1=0;
            x2=x+buttonMask; if(x2>rfbScreen->width) x2=rfbScreen->width;
            y1=y-buttonMask; if(y1<0) y1=0;
            y2=y+buttonMask; if(y2>rfbScreen->height) y2=rfbScreen->height;

            for(i=x1*4/*BPP*/;i<x2*4;i++)
              for(j=y1;j<y2;j++)
                cl->screen->frameBuffer[j*w+i]=(char)0xff;
            rfbMarkRectAsModified(cl->screen,x1,y1,x2,y2);
         }
      }else  cd->oldButton=0;
      if(btn>0&&btn<0x10){
          LOGV("ButtonMask %x->%x btn=%x/%d",cd->oldButton,buttonMask,btn,btnidx(btn),btns[btnidx(btn)]);
          InjectABS(EV_KEY,(bc1>bc0?1:0),btns[btnidx(btn)]);
      }
      cd->oldx=x; cd->oldy=y; cd->oldButton=buttonMask;
   }
   rfbDefaultPtrAddEvent(buttonMask,x,y,cl);
}
static void onVNCClientKey(rfbBool down,rfbKeySym key,rfbClientPtr cl)
{
     LOGV("rcv KEY %d down=%d   XK_B=%d",key,down,XK_B);
     switch(key){
     case XK_F1:SENDKEY(KEY_F1,down);break;
     case XK_F2:SENDKEY(KEY_F2,down);break;
     case XK_F3:SENDKEY(KEY_F3,down);break;
     case XK_F4:SENDKEY(KEY_F4,down);break;
     case XK_F5:SENDKEY(KEY_VOLUMEUP,down);break;
     case XK_F6:SENDKEY(KEY_VOLUMEDOWN,down);break;
     case XK_F10:SENDKEY(KEY_POWER,down);break;
     case XK_F11:SENDKEY(KEY_EPG,down);break;
     case XK_F12:SENDKEY(KEY_MENU,down);break;
     case XK_Page_Up:SENDKEY(KEY_PAGEUP,down);break;
     case XK_Page_Down:SENDKEY(KEY_PAGEDOWN,down);break;
     case XK_Up   : SENDKEY(KEY_UP,down)  ;break;
     case XK_Down : SENDKEY(KEY_DOWN,down);break;
     case XK_Left : SENDKEY(KEY_LEFT,down);break;
     case XK_Right: SENDKEY(KEY_RIGHT,down);break;
     case XK_BackSpace:SENDKEY(KEY_BACKSPACE,down);break;
     case XK_Escape:SENDKEY(KEY_ESCAPE,down);break;
     case XK_Menu : SENDKEY(KEY_MENU,down);break;
     case XK_space: SENDKEY(KEY_SPACE,down);break;
     case XK_Return:SENDKEY(KEY_ENTER,down);break;
     case XK_Delete:SENDKEY(KEY_DELETE,down);break;
     case XK_0     :SENDKEY(KEY_0,down);break;
     case XK_1     :SENDKEY(KEY_1,down);break;
     case XK_2     :SENDKEY(KEY_2,down);break;
     case XK_3     :SENDKEY(KEY_3,down);break;
     case XK_4     :SENDKEY(KEY_4,down);break;
     case XK_5     :SENDKEY(KEY_5,down);break;
     case XK_6     :SENDKEY(KEY_6,down);break;
     case XK_7     :SENDKEY(KEY_7,down);break;
     case XK_8     :SENDKEY(KEY_8,down);break;
     case XK_9     :SENDKEY(KEY_9,down);break;
     case XK_a:    SENDKEY(KEY_A,down);break;
     case XK_b:    SENDKEY(KEY_B,down);break;
     case XK_c:    SENDKEY(KEY_C,down);break;
     case XK_d:    SENDKEY(KEY_D,down);break;
     case XK_e:    SENDKEY(KEY_E,down);break;
     case XK_f:    SENDKEY(KEY_F,down);break;
     case XK_g:    SENDKEY(KEY_G,down);break;
     case XK_h:    SENDKEY(KEY_H,down);break;
     case XK_i:    SENDKEY(KEY_I,down);break;
     case XK_j:    SENDKEY(KEY_J,down);break;
     case XK_k:    SENDKEY(KEY_K,down);break;
     case XK_l:    SENDKEY(KEY_L,down);break;
     case XK_m:    SENDKEY(KEY_M,down);break;
     case XK_n:    SENDKEY(KEY_N,down);break;
     case XK_o:    SENDKEY(KEY_O,down);break;
     case XK_p:    SENDKEY(KEY_P,down);break;
     case XK_q:    SENDKEY(KEY_Q,down);break;
     case XK_r:    SENDKEY(KEY_R,down);break;
     case XK_s:    SENDKEY(KEY_S,down);break;
     case XK_t:    SENDKEY(KEY_T,down);break;
     case XK_u:    SENDKEY(KEY_U,down);break;
     case XK_v:    SENDKEY(KEY_V,down);break;
     case XK_w:    SENDKEY(KEY_W,down);break;
     case XK_x:    SENDKEY(KEY_X,down);break;
     case XK_y:    SENDKEY(KEY_Y,down);break;
     case XK_z:    SENDKEY(KEY_Z,down);break;
     case XK_semicolon:SENDKEY(KEY_SEMICOLON,down);break;
     case XK_slash:SENDKEY(KEY_SLASH,down);break;
     case XK_backslash:SENDKEY(KEY_BACKSLASH,down);break;
     case XK_comma :SENDKEY(KEY_COMMA,down);break;
     //case XK_dot :SENDKEY(KEY_DOT,down);break;
     case XK_braceleft :SENDKEY(KEY_LEFTBRACE,down);break;
     case XK_braceright:SENDKEY(KEY_RIGHTBRACE,down);break;
     case XK_minus     :SENDKEY(KEY_MINUS,down);break;
     //case XK_plus      :SENDKEY(KEY_PLUS,down);break;
     /*case XK_W     :SENDKEY(KEY_W,down);break;
     case XK_S     :SENDKEY(KEY_S,down);break;
     case XK_A     :SENDKEY(KEY_A,down);break;
     case XK_D     :SENDKEY(KEY_D,down);break;*/
     }
}
#endif //ENABLE_RFB


