#include <cdtypes.h>
#include <cdgraph.h>
#include <cdlog.h>
#include <SDL.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <string.h>
#include <linux/input.h>
#include <cdinput.h>

static SDL_Window *sdlWindow = NULL;
static SDL_Renderer *sdlRenderer = NULL;
typedef struct {
    int dispid;
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
    int format;
    int ishw;
    SDL_Surface*surface;
    SDL_Texture*texture;
} FBSURFACE;
static GFXRect screenMargin={0,0,0,0};
#define REFRESH_EVENT 0x4321
static int SDLProc(void*params);
static void SendKeyEvent(SDL_Event*event);
static void SendMouseEvent(SDL_Event*event);
static FBSURFACE*PrimarySurface;

int32_t GFXInit() {
    uint32_t width,height;
    if(sdlWindow)return E_OK;
    SDL_Init(SDL_INIT_VIDEO);
    GFXGetDisplaySize(0,&width,&height);
    sdlWindow = SDL_CreateWindow("CDROID Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                 width,height, SDL_WINDOW_OPENGL /*| SDL_WINDOW_RESIZABLE*/);
    LOGI("SDL ScreenSize =%dx%d sdlWindow=%p driver=%s",width,height,sdlWindow,SDL_GetCurrentVideoDriver());
    if (!sdlWindow) return E_ERROR;

    sdlRenderer = SDL_CreateRenderer(sdlWindow, -1, 0);//基于窗口创建渲染器
    SDL_SetRenderDrawColor(sdlRenderer, 255, 0, 0, 255);
    SDL_RenderClear(sdlRenderer);
    SDL_RenderPresent(sdlRenderer);
    LOGI("sdlWindow=%p sdlRenderer=%p",sdlWindow,sdlRenderer);
    SDL_Delay(100);
    SDL_CreateThread(SDLProc,NULL,NULL);
    return E_OK;
}

int32_t GFXGetDisplayCount() {
    return 1;
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

    LOGV("screensize=%dx%d",*width,*height);
    return E_OK;
}

GFXHANDLE GFXCreateCursor(const GFXCursorImage*cursorImage){
    return (GFXHANDLE)0;
}

void GFXAttachCursor(GFXHANDLE cursorHandle){
}
void GFXMoveCursor(int32_t xPos,int32_t yPos){
}

void GFXDestroyCursor(GFXHANDLE cursorHandle){
}

int32_t GFXLockSurface(GFXHANDLE surface,void**buffer,uint32_t*pitch) {
    FBSURFACE*ngs=(FBSURFACE*)surface;
    *buffer=ngs->surface->pixels;
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
    GFXRect rec= {0,0,0,0};
    rec.w=ngs->width;
    rec.h=ngs->height;
    if(rect)rec=*rect;
    LOGV("FillRect %p %d,%d-%d,%d color=0x%x pitch=%d",ngs,rec.x,rec.y,rec.w,rec.h,color,ngs->pitch);
    SDL_FillRect(ngs->surface,(SDL_Rect*)&rec,color);
    return E_OK;
}

int32_t GFXFlip(GFXHANDLE surface) {
    return 0;
}

int32_t GFXCreateSurface(int dispid,GFXHANDLE*surface,uint32_t width,uint32_t height,int32_t format,bool hwsurface) {
    FBSURFACE*surf=(FBSURFACE*)malloc(sizeof(FBSURFACE));
    surf->dispid=dispid;
    surf->width=width;
    surf->height=height;
    surf->format=format;
    surf->ishw=hwsurface;
    surf->pitch=width*4;
    size_t buffer_size=surf->height*surf->pitch;
    uint32_t rmask, gmask, bmask, amask;
#if 0//SDL_BYTEORDER == SDL_BIG_ENDIAN
    rmask = 0xff000000;
    gmask = 0x00ff0000;
    bmask = 0x0000ff00;
    amask = 0x000000ff;
#else
    amask = 0xff000000;
    rmask = 0x00ff0000;
    gmask = 0x0000ff00;
    bmask = 0x000000ff;
#endif
    if(hwsurface) {
        surf->surface=SDL_GetWindowSurface(sdlWindow);
        surf->texture=SDL_CreateTextureFromSurface(sdlRenderer,surf->surface);
    } else {
        surf->surface=SDL_CreateRGBSurface(SDL_SWSURFACE,width, height, 32,rmask, gmask, bmask, amask);
        surf->texture=SDL_CreateTextureFromSurface(sdlRenderer,surf->surface);
    }
    LOGV("surface=%x buf=%p size=%dx%d hw=%d",surf,surf->surface->pixels,width,height,hwsurface);
    *surface=surf;
    if(hwsurface)PrimarySurface=surf;
    return E_OK;
}

int32_t GFXBlit(GFXHANDLE dstsurface,int dx,int dy,GFXHANDLE srcsurface,const GFXRect*srcrect) {
    FBSURFACE*ndst=(FBSURFACE*)dstsurface;
    FBSURFACE*nsrc=(FBSURFACE*)srcsurface;
    GFXRect rs= {0,0};
    rs.w = nsrc->width;
    rs.h = nsrc->height;
    if(srcrect)rs = *srcrect;
    if(((int)rs.w+dx<=0)||((int)rs.h+dy<=0)||(dx>=(int)ndst->width)||(dy>=(int)ndst->height)||(rs.x<0)||(rs.y<0)) {
        LOGV("dx=%d,dy=%d rs=(%d,%d-%d,%d)",dx,dy,rs.x,rs.y,rs.w,rs.h);
        return E_INVALID_PARA;
    }

    LOGV("Blit %p[%dx%d] %d,%d-%d,%d -> %p[%dx%d] %d,%d",nsrc,nsrc->width,nsrc->height,
         rs.x,rs.y,rs.w,rs.h,ndst,ndst->width,ndst->height,dx,dy);
    if(dx<0) {
        rs.x -= dx;
        rs.w = (int)rs.w + dx;
        dx = 0;
    }
    if(dy<0) {
        rs.y -= dy;
        rs.h = (int)rs.h + dy;
        dy = 0;
    }
    if(dx + rs.w > ndst->width )rs.w = ndst->width - dx;
    if(dy + rs.h > ndst->height)rs.h = ndst->height- dy;

    LOGV("Blit %p %d,%d-%d,%d -> %p %d,%d",nsrc,rs.x,rs.y,rs.w,rs.h,ndst,dx,dy);
    GFXRect rd=rs;
    rd.x = dx;
    rd.y = dy;
    SDL_BlitSurface(nsrc->surface,(const SDL_Rect *)&rs,ndst->surface,(const SDL_Rect *)&rd);
    if(ndst->ishw)
        SDL_UpdateWindowSurface(sdlWindow);
    return 0;
}

int32_t GFXDestroySurface(GFXHANDLE surface) {
    FBSURFACE*surf=(FBSURFACE*)surface;
    SDL_FreeSurface(surf->surface);
    SDL_DestroyTexture(surf->texture);
    free(surf);
    return 0;
}

static int SDLProc(void*params) {
    int running=1;
    while(running) {
        SDL_Event event;
        if(SDL_WaitEventTimeout(&event,0)==0)continue;
        switch(event.type) {
        case SDL_QUIT:
            running=0;
            break;
        case SDL_KEYDOWN:
        case SDL_KEYUP  :
            SendKeyEvent(&event);
            break;
        case SDL_MOUSEMOTION:
            SendMouseEvent(&event);
            break;
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP  :
            SendMouseEvent(&event);
            break;
        case SDL_MOUSEWHEEL:
            event.wheel;
            break;
        case SDL_FINGERDOWN:
            event.tfinger;
            break;
        case SDL_FINGERUP  :
            event.tfinger;
            break;
        case SDL_FINGERMOTION:
            event.tfinger;
            break;
        case SDL_DISPLAYEVENT:
            LOGD("SDL_DISPLAYEVENT");
            break;
        default:
            break;
        }
    }
}

static void InjectEvent(int type,int code,int value,int dev) {
    INPUTEVENT i= {0};
    struct timespec ts;
#ifdef _WIN32
    (void)timespec_get(&ts, 0);
#else
    clock_gettime(CLOCK_MONOTONIC, &ts);
#endif
    i.tv_sec=ts.tv_sec;
    i.tv_usec=ts.tv_nsec/1000;
    i.type=type;
    i.code=code;
    i.value=value;
    i.device=dev;//INJECTDEV_TOUCH;
    InputInjectEvents(&i,1,1);
}

static void SendMouseEvent(SDL_Event*event) {
    switch(event->type) {
    case SDL_MOUSEBUTTONUP:
    case SDL_MOUSEBUTTONDOWN:
        InjectEvent(EV_ABS,0,event->button.x,INJECTDEV_TOUCH);
        InjectEvent(EV_ABS,1,event->button.y,INJECTDEV_TOUCH);
        InjectEvent(EV_KEY,BTN_TOUCH,event->button.state,INJECTDEV_TOUCH);
        break;
    case SDL_MOUSEMOTION:
        InjectEvent(EV_ABS,0,event->motion.x,INJECTDEV_TOUCH);
        InjectEvent(EV_ABS,1,event->motion.y,INJECTDEV_TOUCH);
        break;
    default:
        return;
    }
    InjectEvent(EV_SYN,SYN_REPORT,0,INJECTDEV_TOUCH);
}

static void SendKeyEvent(SDL_Event*event) {
    int keycode = event->key.keysym.sym;
    struct {int sdlk;int key}KEYS[]={
        {SDL_SCANCODE_0,7},{SDL_SCANCODE_1,8},{SDL_SCANCODE_2,9},{SDL_SCANCODE_3,10},{SDL_SCANCODE_4,11},
        {SDL_SCANCODE_5,12},{SDL_SCANCODE_6,13},{SDL_SCANCODE_7,14},{SDL_SCANCODE_8,15},{SDL_SCANCODE_9,16},
        {SDL_SCANCODE_F1,131},{SDL_SCANCODE_F2,132},{SDL_SCANCODE_F3,133},{SDL_SCANCODE_F4,134},{SDL_SCANCODE_F5,135},
        {SDL_SCANCODE_F6,136},{SDL_SCANCODE_F7,137},{SDL_SCANCODE_F8,138}, {SDL_SCANCODE_F9,139},{SDL_SCANCODE_F10,140},
        {SDL_SCANCODE_F11,141},{SDL_SCANCODE_F12,142},{SDL_SCANCODE_LEFT,21},{SDL_SCANCODE_RIGHT,22},{SDL_SCANCODE_UP,19},
        {SDL_SCANCODE_DOWN,20},{SDL_SCANCODE_HOME,122},{SDL_SCANCODE_END,123},{SDL_SCANCODE_PAGEUP,92},{SDL_SCANCODE_PAGEDOWN,93},
        {SDL_SCANCODE_INSERT,124},{SDL_SCANCODE_DELETE,67},{SDL_SCANCODE_KP_ENTER,23/*DPAD_CENTER*/},{SDL_SCANCODE_ESCAPE,111},
        {SDL_SCANCODE_RETURN,66},{SDL_SCANCODE_SPACE,62},{SDL_SCANCODE_BACKSPACE,112},{SDL_SCANCODE_MENU,82},
        {SDL_SCANCODE_Q,45},{SDL_SCANCODE_W,51},{SDL_SCANCODE_E,33},{SDL_SCANCODE_R,46},{SDL_SCANCODE_T,48},
        {SDL_SCANCODE_Y,53/*Y*/},{SDL_SCANCODE_U,49},{SDL_SCANCODE_I,37},{SDL_SCANCODE_O,43},{SDL_SCANCODE_P,44},
        {SDL_SCANCODE_A,29},{SDL_SCANCODE_S,47},{SDL_SCANCODE_D,32},{SDL_SCANCODE_F,34},{SDL_SCANCODE_G,35},
        {SDL_SCANCODE_H,36},{SDL_SCANCODE_J,38},{SDL_SCANCODE_K,39},{SDL_SCANCODE_L,40},
        {SDL_SCANCODE_Z,54},{SDL_SCANCODE_X,52},{SDL_SCANCODE_C,31},{SDL_SCANCODE_V,50},
        {SDL_SCANCODE_B,30},{SDL_SCANCODE_N,42},{SDL_SCANCODE_M,41}
    };
    for(int i=0;i<sizeof(KEYS)/sizeof(KEYS[0]);i++){
        if(event->key.keysym.scancode == KEYS[i].sdlk){
            LOGD("key %d,%d==>%x",event->key.keysym.scancode,event->key.keysym.scancode,KEYS[i].sdlk);
            InjectEvent(EV_KEY,KEYS[i].key,(event->type==SDL_KEYDOWN),INJECTDEV_KEY);
            break;
        }
    }
}

