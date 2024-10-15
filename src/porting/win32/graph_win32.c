#include <cdgraph.h>
#include <cdlog.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

#include <Windows.h>
typedef struct {
    HWND hwnd;
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

static FBDEVICE devs[2]= {-1};
static GFXRect screenMargin= {0};
static LRESULT CALLBACK cdroid_window_message_proc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}
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
    window_class.lpfnWndProc = cdroid_window_message_proc;
    window_class.lpszClassName = L"CDROID.Window";
    RegisterClassExW(&window_class);
    devs[0].hwnd = CreateWindowExW(WS_EX_CLIENTEDGE,
           L"CDROID.Window",L"CDROID",window_style,CW_USEDEFAULT,
           0,width,height,NULL,NULL,NULL,NULL);
    
    ShowWindow(devs[0].hwnd, SW_SHOW);
    UpdateWindow(devs[0].hwnd);
    LOGI("Win32 solution=%dx%d \r\n", width, height);
    SetEvent((HANDLE)param);
    while(GetMessageW(&message, NULL, 0, 0)) {
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }
}

int32_t GFXInit() {
    if(devs[0].hwnd>=0)return E_OK;
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
    HANDLE mutex = CreateEventExW(NULL, NULL, 0, EVENT_ALL_ACCESS);
    HANDLE thread = (HANDLE)_beginthreadex(NULL,0,display_thread,mutex,0,NULL);
    WaitForSingleObjectEx(mutex, INFINITE, FALSE);
    CloseHandle(mutex);
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


int32_t GFXLockSurface(HANDLE surface,void**buffer,uint32_t*pitch) {
    FBSURFACE*ngs=(FBSURFACE*)surface;
    *buffer=ngs->buffer;
    *pitch=ngs->pitch;
    return 0;
}

int32_t GFXGetSurfaceInfo(HANDLE surface,uint32_t*width,uint32_t*height,int32_t *format) {
    FBSURFACE*ngs=(FBSURFACE*)surface;
    *width = ngs->width;
    *height= ngs->height;
    *format= ngs->format;
    return E_OK;
}

int32_t GFXUnlockSurface(HANDLE surface) {
    return 0;
}

int32_t GFXSurfaceSetOpacity(HANDLE surface,uint8_t alpha) {
    return 0;//dispLayer->SetOpacity(dispLayer,alpha);
}

int32_t GFXFillRect(HANDLE surface,const GFXRect*rect,uint32_t color) {
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

int32_t GFXFlip(HANDLE surface) {
    FBSURFACE*surf=(FBSURFACE*)surface;
    FBDEVICE*dev=devs+surf->dispid;
    if(surf->ishw) {
        GFXRect rect= {0,0,surf->width,surf->height};
    }
    return 0;
}

int32_t GFXCreateSurface(int dispid,HANDLE*surface,uint32_t width,uint32_t height,int32_t format,BOOL hwsurface) {
    FBSURFACE*surf=(FBSURFACE*)malloc(sizeof(FBSURFACE));
    FBDEVICE*dev = &devs[dispid];
    surf->dispid=dispid;
    surf->width= hwsurface?dev->width:width;
    surf->height=hwsurface?dev->height:height;
    surf->format=format;
    surf->ishw=hwsurface;
    surf->pitch=width*4;
    size_t buffer_size=surf->height*surf->pitch;

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
        surf->hBitmap = CreateDIBSection(surf->hDC,(PBITMAPINFO)(&bitmap_info),
            DIB_RGB_COLORS,(void**)&surf->buffer, NULL,0);
        buffer_size=surf->pitch*surf->height;
        setfbinfo(surf);
        surf->pitch=dev->pitch;
    }
    surf->ishw=hwsurface;
    LOGV("surface=%x buf=%p size=%dx%d hw=%d",surf,surf->buffer,width,height,hwsurface);
    *surface=surf;
    return E_OK;
}


int32_t GFXBlit(HANDLE dstsurface,int dx,int dy,HANDLE srcsurface,const GFXRect*srcrect) {
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
    BitBlt(ndst->hDC, dx, dy, rs.w, rs.y, nsrc->hDC, rs.x, rs.y, SRCCOPY);
    return 0;
}

int32_t GFXDestroySurface(HANDLE surface) {
    FBSURFACE* surf = (FBSURFACE*)surface;
    FBDEVICE* dev = devs + surf->dispid;
    if (surf->hBitmap) {
        DeleteObject((HBITMAP)surf->hBitmap);
        DeleteDC(surf->hDC);
    }
    free(surf);
    return 0;
}
