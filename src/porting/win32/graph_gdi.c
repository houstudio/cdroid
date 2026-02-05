#include <Windows.h>
#include <cdgraph.h>
#include <cdlog.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <cdinput.h>
#include <core/eventcodes.h>
#include <pixman.h>

typedef struct {
    HWND hwnd;
    HDC hdc;
    int width;
    int height;
    int pitch;
} FBDEVICE;

typedef struct {
    int dispid;
    HDC hDC;
    HBITMAP hBitmap;
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
    int format;
    int ishw;
    char*buffer;
} FBSURFACE;

static FBDEVICE devs[2]= {0};
static GFXRect screenMargin= {0};
#define SENDKEY(k,down) {InjectKey(EV_KEY,k,down);}
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
static void InjectABS(unsigned long etime, int type, int axis, int value) {
    INPUTEVENT i = { 0 };
    i.tv_sec = etime/1000;
    i.tv_usec = (etime%1000)*1000;
    i.type = type;
    i.code = axis;
    i.value = value;
    i.device = INJECTDEV_TOUCH;
    InputInjectEvents(&i, 1, 1);
}
static LRESULT CALLBACK Window_PROC(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    DWORD msgTime = GetMessageTime();
    switch (uMsg) {
    case WM_TOUCH: {
        UINT cInputs = LOWORD(wParam);
        if (cInputs == 0)break;
        PTOUCHINPUT pInputs = (PTOUCHINPUT)malloc(cInputs * sizeof(TOUCHINPUT));
        if (GetTouchInputInfo((HTOUCHINPUT)lParam, cInputs, pInputs, sizeof(TOUCHINPUT))) {
            for (UINT i = 0; i < cInputs; i++) {
                TOUCHINPUT ti = pInputs[i];
                LOGI("Touch point %u: x = %ld, y = %ld", i, ti.x / 100, ti.y / 100);
            }
            CloseTouchInputHandle((HTOUCHINPUT)lParam);
            free(pInputs);
        }break;
    }
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
        InjectABS(msgTime, EV_KEY, BTN_TOUCH, (uMsg == WM_LBUTTONDOWN) ? 1 : 0);
        SENDMOUSE(msgTime, (LOWORD(lParam) - screenMargin.x), (HIWORD(lParam) - screenMargin.y));
        break;
    case WM_MOUSEMOVE:
        if (wParam & MK_LBUTTON) {
            SENDMOUSE(msgTime, (LOWORD(lParam) - screenMargin.x), (HIWORD(lParam) - screenMargin.y));
        }
        break;
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd,&ps);
        BitBlt(hdc, 0, 0, devs[0].width,devs[0].height, devs[0].hdc, 0, 0, SRCCOPY);
        EndPaint(hWnd, &ps);
        }break;
    default:return DefWindowProcW(hWnd, uMsg, wParam, lParam);
    }
    return 0;
}
#define CDROID_WINDOW_CLASSNAME L"CDROID.Window"
static unsigned int __stdcall display_thread(void * param)
{
    DWORD window_style= WS_OVERLAPPEDWINDOW;
    window_style &= ~(WS_SIZEBOX | WS_MAXIMIZEBOX | WS_THICKFRAME);
    char*env = getenv("SCREEN_SIZE");
    int width=1280,height=720;
    WNDCLASSEXW window_class;
    MSG message;
    if(env){
        width= atoi(env);
        env = strpbrk(env,"xX*,");
        if(env)height=atoi(env+1);
    }
    memset(&window_class,0, sizeof(WNDCLASSEXW));
    window_class.cbSize = sizeof(WNDCLASSEXW);
    window_class.lpfnWndProc = Window_PROC;
    window_class.lpszClassName = CDROID_WINDOW_CLASSNAME;
    ATOM atrc = RegisterClassExW(&window_class);
    LOGI("RegisterCLassExW=%d", atrc);
    devs[0].hwnd = CreateWindowExW(WS_EX_CLIENTEDGE,CDROID_WINDOW_CLASSNAME,L"CDROID",
        window_style,CW_USEDEFAULT,0,width,height,NULL,NULL,NULL,NULL);
    devs[0].width = width;
    devs[0].height= height;
    devs[0].pitch = width * 4;
    LOGI("Win32 hwnd=%p solution=%dx%d \r\n", devs[0].hwnd, width, height);
    ShowWindow(devs[0].hwnd, SW_SHOW);
    UpdateWindow(devs[0].hwnd);
    SetEvent((HANDLE)param);
    //SetTimer(devs[0].hwnd, 1, 50, NULL);
    while(GetMessageW(&message, NULL, 0, 0)) {
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }
    return 0;
}

int32_t GFXInit() {
    if(devs[0].hwnd>0)return E_OK;
    memset(devs,0,sizeof(devs));
    FBDEVICE*dev=&devs[0];
    const char*strMargins=getenv("SCREEN_MARGINS");
    const char* DELIM=",;";
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
    HANDLE mutex = CreateEventExW(NULL, CDROID_WINDOW_CLASSNAME, 0, EVENT_ALL_ACCESS);
    HANDLE thread = (HANDLE)_beginthreadex(NULL,0,display_thread,mutex,0,NULL);
    WaitForSingleObjectEx(mutex, INFINITE, FALSE);
    CloseHandle(mutex);
    return E_OK;
}

int32_t GFXGetDisplayCount() {
    return 1;
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
    RECT winrec = { rec.x,rec.y,rec.x + rec.w,rec.y + rec.h };
    HBRUSH hb = CreateSolidBrush(color);
    FillRect(ngs->hDC, &winrec, (HBRUSH)hb);
    DeleteObject(hb);
    return E_OK;
}

int32_t GFXFlip(GFXHANDLE surface) {
    FBSURFACE*surf=(FBSURFACE*)surface;
    FBDEVICE*dev=devs+surf->dispid;
    if(surf->ishw) {
        GFXRect rect= {0,0,surf->width,surf->height};
        InvalidateRect(dev->hwnd,NULL,FALSE);
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
    {
        BITMAPINFO bitmap_info;
        HDC wdc = GetDC(dev->hwnd);
        surf->hDC = CreateCompatibleDC(wdc);
        ReleaseDC(dev->hwnd, wdc);
        bitmap_info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bitmap_info.bmiHeader.biWidth = surf->width;
        bitmap_info.bmiHeader.biHeight = -surf->height;
        bitmap_info.bmiHeader.biPlanes = 1;
        bitmap_info.bmiHeader.biBitCount = 32;
        bitmap_info.bmiHeader.biCompression = BI_RGB;
        surf->hBitmap = CreateDIBSection(surf->hDC,(PBITMAPINFO)(&bitmap_info),
            DIB_RGB_COLORS,(void**)&surf->buffer, NULL,0);
        SelectObject(surf->hDC, surf->hBitmap);
        devs[0].hdc = surf->hDC;
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
    pbs+=rs.y*nsrc->pitch+rs.x*4;
    if(ndst->ishw==0)pbd+=dy*ndst->pitch+dx*4;
    else pbd+=(dy+screenMargin.y)*ndst->pitch+(dx+screenMargin.x)*4;
    const int cpw=rs.w*4;
    BitBlt(ndst->hDC, dx, dy, rs.w, rs.h, nsrc->hDC, rs.x, rs.y, SRCCOPY);
    return 0;
}

int32_t GFXDestroySurface(GFXHANDLE surface) {
    FBSURFACE* surf = (FBSURFACE*)surface;
    FBDEVICE* dev = devs + surf->dispid;
    if (surf->hBitmap) {
        DeleteObject((HBITMAP)surf->hBitmap);
        DeleteDC(surf->hDC);
    }
    free(surf);
    return 0;
}
