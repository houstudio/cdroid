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
    UINT width;
    UINT height;
    UINT pitch;
    int format;
    int ishw;
    SDL_Surface*surface;
    SDL_Texture*texture;
} FBSURFACE;

#define REFRESH_EVENT 0x4321
static int SDLProc(void*params);
static void SendKeyEvent(SDL_Event*event);
static void SendMouseEvent(SDL_Event*event);
static FBSURFACE*PrimarySurface;

INT GFXInit() {
    UINT width,height;
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

INT GFXGetDisplayCount() {
    return 1;
}

INT GFXGetDisplaySize(int dispid,UINT*width,UINT*height) {
    const char*env= getenv("SCREENSIZE");
    if(dispid<0||dispid>=GFXGetDisplayCount())return E_ERROR;
    if(env==NULL) {
        SDL_DisplayMode displayMode;
        SDL_GetCurrentDisplayMode(0, &displayMode);
        *width = displayMode.w;
        *height= displayMode.h;
    } else {
        *width=atoi(env);//- screenMargin.x - screenMargin.w;
        env=strpbrk(env,"x*,");
        if((*width<=0)||(env==NULL))exit(-1);
        *height=atoi(env+1);//- screenMargin.y - screenMargin.h;
        if(*height<=0)exit(-1);
    }
    LOGV("screensize=%dx%d",*width,*height);
    return E_OK;
}

INT GFXLockSurface(HANDLE surface,void**buffer,UINT*pitch) {
    FBSURFACE*ngs=(FBSURFACE*)surface;
    *buffer=ngs->surface->pixels;
    *pitch=ngs->pitch;
    return 0;
}

INT GFXGetSurfaceInfo(HANDLE surface,UINT*width,UINT*height,INT *format) {
    FBSURFACE*ngs=(FBSURFACE*)surface;
    *width = ngs->width;
    *height= ngs->height;
    *format= ngs->format;
    return E_OK;
}

INT GFXUnlockSurface(HANDLE surface) {
    return 0;
}

INT GFXSurfaceSetOpacity(HANDLE surface,BYTE alpha) {
    return 0;//dispLayer->SetOpacity(dispLayer,alpha);
}

INT GFXFillRect(HANDLE surface,const GFXRect*rect,UINT color) {
    FBSURFACE*ngs=(FBSURFACE*)surface;
    GFXRect rec= {0,0,0,0};
    rec.w=ngs->width;
    rec.h=ngs->height;
    if(rect)rec=*rect;
    LOGV("FillRect %p %d,%d-%d,%d color=0x%x pitch=%d",ngs,rec.x,rec.y,rec.w,rec.h,color,ngs->pitch);
    SDL_FillRect(ngs->surface,(SDL_Rect*)&rec,color);
    return E_OK;
}

INT GFXFlip(HANDLE surface) {
    return 0;
}

INT GFXCreateSurface(int dispid,HANDLE*surface,UINT width,UINT height,INT format,BOOL hwsurface) {
    FBSURFACE*surf=(FBSURFACE*)malloc(sizeof(FBSURFACE));
    surf->dispid=dispid;
    surf->width=width;
    surf->height=height;
    surf->format=format;
    surf->ishw=hwsurface;
    surf->pitch=width*4;
    size_t buffer_size=surf->height*surf->pitch;
    UINT rmask, gmask, bmask, amask;
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

INT GFXBlit(HANDLE dstsurface,int dx,int dy,HANDLE srcsurface,const GFXRect*srcrect) {
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

INT GFXDestroySurface(HANDLE surface) {
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
	{SDLK_0,7},{SDLK_1,8},{SDLK_2,9},{SDLK_3,10},{SDLK_4,11},{SDLK_5,12},{SDLK_6,13},{SDLK_7,14},{SDLK_8,15},{SDLK_9,16},
	{SDLK_F1,131},{SDLK_F2,132},{SDLK_F3,133},{SDLK_F4,134},{SDLK_F5,135},{SDLK_F6,136},{SDLK_F7,137},{SDLK_F8,138}, {SDLK_F9,139},{SDLK_F10,140},{SDLK_F11,141},{SDLK_F12,142},
        {SDLK_LEFT,21},{SDLK_RIGHT,22},{SDLK_UP,19},{SDLK_DOWN,20},{SDLK_HOME,122},{SDLK_END,123},{SDLK_PAGEUP,92},{SDLK_PAGEDOWN,93},
        {SDLK_INSERT,124},{SDLK_DELETE,67},{SDLK_KP_ENTER,23/*DPAD_CENTER*/},{SDLK_ESCAPE,111},{SDLK_RETURN,66},{SDLK_SPACE,62},{SDLK_BACKSPACE,112},{SDLK_MENU,82},
        {SDLK_q,45},{SDLK_w,51},{SDLK_e,33},{SDLK_r,46},{SDLK_t,48},{SDLK_y,53/*Y*/},{SDLK_u,49},{SDLK_i,37},{SDLK_o,43},{SDLK_p,44},
        {SDLK_a,29},{SDLK_s,47},{SDLK_d,32},{SDLK_f,34},{SDLK_g,35},{SDLK_h,36},{SDLK_j,38},{SDLK_k,39},{SDLK_l,40},
        {SDLK_z,54},{SDLK_x,52},{SDLK_c,31},{SDLK_v,50},{SDLK_b,30},{SDLK_n,42},{SDLK_m,41}
    };
    for(int i=0;i<sizeof(KEYS)/sizeof(KEYS[0]);i++){
	if(event->key.keysym.sym == KEYS[i].sdlk){
            LOGV("sdlkey=%d,%x => %d",event->key.keysym.sym,event->key.keysym.sym,KEYS[i].key);
            InjectEvent(EV_KEY,KEYS[i].key,(event->type==SDL_KEYDOWN),INJECTDEV_KEY);
	    break;
	}
    }
}

