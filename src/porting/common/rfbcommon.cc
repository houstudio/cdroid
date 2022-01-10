#include <cdtypes.h>
#include <cdlog.h>
#include <linux/fb.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <eventcodes.h>
#include <cdinput.h>
#include <rfb/rfb.h>
#include <rfb/keysym.h>
#include <rfb/rfbproto.h>

typedef struct ClientData {
    rfbBool oldButton;
    int oldx,oldy;
} RFBClientData;
#define SENDKEY(k,down) {InjectKey(EV_KEY,k,down);}
#define SENDMOUSE(x,y)  {InjectABS(EV_ABS,0,x);InjectABS(EV_ABS,1,y);InjectABS(EV_SYN,SYN_REPORT,0);}

static enum rfbNewClientAction onNewClient(rfbClientPtr cl);
static void onVNCClientKey(rfbBool down,rfbKeySym key,rfbClientPtr cl);
static void onMousePtr(int buttonMask,int x,int y,rfbClientPtr cl);
static int FileTransferPermitted(rfbClientPtr cl);
extern "C" int rfbTightProcessArg(int argc,char *argv[]);
extern "C" void setupRFB(rfbScreenInfoPtr rfbScreen,const char*name,int port){
    char*ftpargs[]={"-ftproot","/"};
    rfbScreen->desktopName = name;
    rfbScreen->frameBuffer = NULL;
    rfbScreen->alwaysShared = (1==1);
    rfbScreen->kbdAddEvent=onVNCClientKey;
    rfbScreen->ptrAddEvent=onMousePtr;
    rfbScreen->newClientHook=onNewClient;
    rfbScreen->setTextChat=NULL;//onChatText;
    rfbScreen->permitFileTransfer=TRUE;
    rfbScreen->bitsPerPixel=24;
    rfbScreen->autoPort=(port<=0);
    if(port>0)rfbScreen->port=port;
    rfbScreen->getFileTransferPermission=FileTransferPermitted;
    rfbRegisterTightVNCFileTransferExtension();

    rfbInitServer(rfbScreen);
    rfbRunEventLoop(rfbScreen,5,TRUE);//non block
    
    rfbTightProcessArg(2,ftpargs);
    return;
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

static void onClientDone(rfbClientPtr cl){
   free(cl->clientData);
   cl->clientData = NULL;
}

static int FileTransferPermitted(rfbClientPtr cl){
    LOGD("FileTransferPermitted from %d",cl->sock);
    return TRUE;
}

static enum rfbNewClientAction onNewClient(rfbClientPtr cl){
    cl->clientData = (void*)calloc(sizeof(RFBClientData),1);
    cl->clientGoneHook = onClientDone;
    LOGD("new rfbclient %d",cl->sock);
    return RFB_CLIENT_ACCEPT;
}

static void onMousePtr(int buttonMask,int x,int y,rfbClientPtr cl){
   RFBClientData* cd=(RFBClientData*)cl->clientData;
   rfbScreenInfoPtr rfbScreen=cl->screen;
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

