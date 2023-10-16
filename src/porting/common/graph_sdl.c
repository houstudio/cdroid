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
#include <core/eventcodes.h>
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
    int keycode=event->key.keysym.sym;
    switch(event->key.keysym.sym) {
    case SDLK_0 :
        keycode=KEY_0 ;
        break;
    case SDLK_1 :
        keycode=KEY_1 ;
        break;
    case SDLK_2 :
        keycode=KEY_2 ;
        break;
    case SDLK_3 :
        keycode=KEY_3 ;
        break;
    case SDLK_4 :
        keycode=KEY_4 ;
        break;
    case SDLK_5 :
        keycode=KEY_5 ;
        break;
    case SDLK_6 :
        keycode=KEY_6 ;
        break;
    case SDLK_7 :
        keycode=KEY_7 ;
        break;
    case SDLK_8 :
        keycode=KEY_8 ;
        break;
    case SDLK_9 :
        keycode=KEY_9 ;
        break;

    case SDLK_F1 :
        keycode=KEY_F1 ;
        break;
    case SDLK_F2 :
        keycode=KEY_F2 ;
        break;
    case SDLK_F3 :
        keycode=KEY_F3 ;
        break;
    case SDLK_F4 :
        keycode=KEY_F4 ;
        break;
    case SDLK_F5 :
        keycode=KEY_F5 ;
        break;
    case SDLK_F6 :
        keycode=KEY_F6 ;
        break;
    case SDLK_F7 :
        keycode=KEY_F7 ;
        break;
    case SDLK_F8 :
        keycode=KEY_F8 ;
        break;
    case SDLK_F9 :
        keycode=KEY_F9 ;
        break;
    case SDLK_F10:
        keycode=KEY_F10;
        break;
    case SDLK_F11:
        keycode=KEY_F11;
        break;
    case SDLK_F12:
        keycode=KEY_F12;
        break;

    case SDLK_TAB:
        keycode=KEY_TAB;
        break;
    case SDLK_ESCAPE:
        keycode=KEY_ESCAPE;
        break;
    case SDLK_RETURN:
        keycode=KEY_ENTER;
        break;
    case SDLK_PAGEDOWN:
        keycode=KEY_PAGEDOWN;
        break;
    case SDLK_PAGEUP:
        keycode=KEY_PAGEUP;
        break;
    case SDLK_LEFT:
        keycode=KEY_LEFT;
        break;
    case SDLK_RIGHT:
        keycode=KEY_RIGHT;
        break;
    case SDLK_UP:
        keycode=KEY_UP;
        break;
    case SDLK_DOWN:
        keycode=KEY_DOWN;
        break;
    case SDLK_MENU:
        keycode=KEY_MENU;
        break;
    case SDLK_POWER:
        keycode=KEY_POWER;
        break;
    case SDLK_DELETE:
        keycode=KEY_DELETE;
        break;
    case SDLK_BACKSPACE:
        keycode=KEY_BACKSPACE;
        break;
    case SDLK_VOLUMEDOWN:
        keycode=KEY_VOLUMEDOWN;
        break;
    case SDLK_VOLUMEUP:
        keycode=KEY_VOLUMEUP;
        break;
    case SDLK_MUTE:
        keycode=KEY_MUTE;
        break;

    case SDLK_a:
        keycode=KEY_A;
        break;
    case SDLK_b:
        keycode=KEY_B;
        break;
    case SDLK_c:
        keycode=KEY_C;
        break;
    case SDLK_d:
        keycode=KEY_D;
        break;
    case SDLK_e:
        keycode=KEY_E;
        break;
    case SDLK_f:
        keycode=KEY_F;
        break;
    case SDLK_g:
        keycode=KEY_G;
        break;
    case SDLK_h:
        keycode=KEY_H;
        break;
    case SDLK_i:
        keycode=KEY_I;
        break;
    case SDLK_j:
        keycode=KEY_J;
        break;
    case SDLK_k:
        keycode=KEY_K;
        break;
    case SDLK_l:
        keycode=KEY_L;
        break;
    case SDLK_m:
        keycode=KEY_M;
        break;
    case SDLK_n:
        keycode=KEY_N;
        break;
    case SDLK_o:
        keycode=KEY_O;
        break;
    case SDLK_p:
        keycode=KEY_P;
        break;
    case SDLK_q:
        keycode=KEY_Q;
        break;
    case SDLK_r:
        keycode=KEY_R;
        break;
    case SDLK_s:
        keycode=KEY_S;
        break;
    case SDLK_t:
        keycode=KEY_T;
        break;
    case SDLK_u:
        keycode=KEY_U;
        break;
    case SDLK_v:
        keycode=KEY_V;
        break;
    case SDLK_w:
        keycode=KEY_W;
        break;
    case SDLK_x:
        keycode=KEY_X;
        break;
    case SDLK_y:
        keycode=KEY_Y;
        break;
    case SDLK_z:
        keycode=KEY_Z;
        break;
    default:
        return;
    }
    InjectEvent(EV_KEY,keycode,(event->type==SDL_KEYDOWN),INJECTDEV_KEY);
}

