#include <cdtypes.h>
#include <ngl_os.h>
#include <cdgraph.h>
#include <cdlog.h>
#include <stdio.h>
#include <stdlib.h> /* getenv(), etc. */
#include <unistd.h> 
#include <sys/time.h>
#include <rfb/rfb.h>
#include <rfb/keysym.h>
#include <rfb/rfbproto.h>
#include <cdinput.h>

typedef struct{
    unsigned int width;
    unsigned int height;
    unsigned int pitch;
    int format;
    void*data;
    unsigned int ishw;
}NGLSURFACE;
typedef struct ClientData {
    rfbBool oldButton;
    int oldx,oldy;
} RFBClientData;

static rfbScreenInfoPtr rfbScreen=NULL;
static int surface_count=0;

static void InjectKey(int type,int code,int value){
   INPUTEVENT i={0};
   i.type=type;
   i.code=code;
   i.value=value;
   i.device=INJECTDEV_KEY;
   nglInputInjectEvents(&i,1,1);
}
static void InjectABS(int type,int axis,int value){
   INPUTEVENT i={0};
   i.type=type;
   i.code=axis;
   i.value=value;
   i.device=INJECTDEV_PTR;
   nglInputInjectEvents(&i,1,1);
}
#define SENDKEY(k,down) {InjectKey(EV_KEY,k,down);}
#define SENDMOUSE(x,y) {InjectABS(EV_ABS,0,x);InjectABS(EV_ABS,1,y);InjectABS(EV_SYN,SYN_REPORT,0);}
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
     case XK_Escape:SENDKEY(KEY_ESC,down);break;
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

static int  FileTransferPermitted(struct _rfbClientRec* cl){
    LOGV("___");
    return -1;//-1 allow file transfer
}
static void onChatText(struct _rfbClientRec* cl, int length, char *string){
    LOGD("RCV ChatText:%s",string);
}
#define PRINTFMT(f) LOGV("truecolor=%d ,depth=%d bitsPerPixel=%d rgbmax=%x/%x/%x rgbshift=%x/%x/%x",\
	       	!!(f)->serverFormat.trueColour,(f)->serverFormat.depth,(f)->serverFormat.bitsPerPixel,\
		(f)->serverFormat.redMax,(f)->serverFormat.greenMax,(f)->serverFormat.blueMax,\
  		(f)->serverFormat.redShift,(f)->serverFormat.greenShift,(f)->serverFormat.blueShift)
static void onExit(){
    LOGD("rfbServer shutdown!");
    rfbShutdownServer(rfbScreen,1);
    rfbScreenCleanup(rfbScreen);
}
DWORD nglGraphInit(){
    int x=0,y=0,*fb;
    HANDLE tid;
    int width=1280,height=720;
    if(rfbScreen)return E_OK;
    rfbLogEnable(0);
    InitFileTransfer();    SetFtpRoot("/");
    rfbScreen = rfbGetScreen(NULL,NULL,width,height,8,3,3);
    rfbScreen->desktopName = "NGL-RFB";
    rfbScreen->frameBuffer = NULL;//(char*)malloc(width*height*4);
    rfbScreen->alwaysShared = (1==1);
    rfbScreen->kbdAddEvent=onVNCClientKey;
    rfbScreen->ptrAddEvent=onMousePtr;
    rfbScreen->newClientHook=onNewClient;
    rfbScreen->setTextChat=onChatText;
    rfbScreen->permitFileTransfer=-1;
    rfbScreen->bitsPerPixel=24;
    rfbScreen->getFileTransferPermission=FileTransferPermitted;
    rfbRegisterTightVNCFileTransferExtension();

    rfbInitServer(rfbScreen);
    PRINTFMT(rfbScreen);
    rfbRunEventLoop(rfbScreen,5,TRUE);//non block 
    LOGD("VNC Server Inited rfbScreen=%p port=%d framebuffer=%p",rfbScreen,rfbScreen->port,rfbScreen->frameBuffer); 
    atexit(onExit);
    return E_OK;
}

DWORD nglGetScreenSize(UINT*width,UINT*height){
    *width = rfbScreen->width;
    *height = rfbScreen->height;
    LOGD("size=%dx%d",*width,*height);
    return E_OK;
}

DWORD nglLockSurface(HANDLE surface,void**buffer,UINT*pitch){
    NGLSURFACE*ngs=(NGLSURFACE*)surface;
    *buffer=ngs->data;
    *pitch=ngs->pitch;
    //LOGD("%p buffer=%p pitch=%d",ngs,*buffer,*pitch);
    return 0;
}

DWORD nglGetSurfaceInfo(HANDLE surface,UINT*width,UINT*height,INT *format)
{
    NGLSURFACE*ngs=(NGLSURFACE*)surface;
    if(width)*width=ngs->width;
    if(height)*height=ngs->height;
    if(format)*format=ngs->format;//GPF_ABGR;
    //LOGV("%p size=(%dx%d) format=%d",ngs,ngs->width,ngs->height,ngs->format);
    //PRINTFMT(rfbScreen);
    return E_OK;
}

DWORD nglUnlockSurface(HANDLE surface){
    return 0;
}

DWORD nglSurfaceSetOpacity(HANDLE surface,BYTE alpha){
    return 0;//dispLayer->SetOpacity(dispLayer,alpha);
}

DWORD nglFillRect(HANDLE surface,const NGLRect*rect,UINT color){
    NGLSURFACE*ngs=(NGLSURFACE*)surface;
    UINT x,y;
    NGLRect rec={0,0,0,0};
    rec.w=ngs->width;
    rec.h=ngs->height;
    if(rect)rec=*rect;
    LOGV("FillRect %p %d,%d-%d,%d color=0x%x pitch=%d",ngs,rec.x,rec.y,rec.w,rec.h,color,ngs->pitch);
    UINT*fb=(UINT*)(ngs->data+ngs->pitch*rec.y+rec.x*4);
    for(y=0;y<rec.h;y++){
        for(x=0;x<rec.w;x++)
           fb[x]=color;
        fb+=(ngs->pitch>>2);
    }
    return E_OK;
}

DWORD nglFlip(HANDLE surface){
    NGLSURFACE*ngs=(NGLSURFACE*)surface;
    if(ngs->ishw)rfbMarkRectAsModified(rfbScreen,0,0,rfbScreen->width,rfbScreen->height);
    LOGV("flip %p ishw=%d",ngs,ngs->ishw);
    return 0;
}

static void ResetScreenFormat(NGLSURFACE*fb,int width,int height,int format){
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

DWORD nglCreateSurface(HANDLE*surface,UINT width,UINT height,INT format,BOOL ishwsurface)
{//XShmCreateImage XShmCreatePixmap
     NGLSURFACE*nglsurface=(NGLSURFACE*)malloc(sizeof(NGLSURFACE));
     if((height*width==0)||(surface==NULL))
         return E_INVALID_PARA;
     if(format!=GPF_ABGR&&format!=GPF_ARGB){
         LOGD("Format %d is not supported %d",format,GPF_ARGB);
         return NGL_NOT_SUPPORT;
     }
     nglsurface->data=malloc(width*height*4);
     if(ishwsurface){
         rfbScreen->frameBuffer=nglsurface->data;
         ResetScreenFormat(nglsurface,width,height,format);
     }
     nglsurface->ishw=ishwsurface;
     nglsurface->width=width;
     nglsurface->height=height;
     nglsurface->pitch=width*4;
     nglsurface->format=format;
     *surface=(HANDLE)nglsurface;
     surface_count++;
     LOGV("surface=%p framebuffer=%p size=%dx%d format=%d hw=%d  surfacecount=%d",nglsurface,
         nglsurface->data,width,height,format,ishwsurface,surface_count);
     return E_OK;
}

DWORD nglBlit(HANDLE dstsurface,int dx,int dy,HANDLE srcsurface,const NGLRect*srcrect)
{
     unsigned int x,y,sw,sh;
     NGLSURFACE*ndst=(NGLSURFACE*)dstsurface;
     NGLSURFACE*nsrc=(NGLSURFACE*)srcsurface;
     NGLRect rs={0,0};
     BYTE*pbs=(BYTE*)nsrc->data;
     BYTE*pbd=(BYTE*)ndst->data;
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
     for(y=0;y<rs.h;y++){
         memcpy(pbd,pbs,rs.w*4);
         pbs+=nsrc->pitch;
         pbd+=ndst->pitch;
     }
     if(ndst->ishw)rfbMarkRectAsModified(rfbScreen,dx,dy,dx+rs.w,dy+rs.h);
     return 0;
}

DWORD nglDestroySurface(HANDLE surface)
{
     NGLSURFACE*ngs=(NGLSURFACE*)surface;
     if(ngs->data){
        free(ngs->data);
        ngs->data=NULL;
     }
     if(ngs->ishw)
        ResetScreenFormat(ngs,ngs->width,ngs->height,ngs->format);
     surface_count--;
     LOGV("Surface %p Destroied hw=%d surfacecount=%d",ngs,ngs->ishw,surface_count);
     free(ngs);
     return 0;
}
