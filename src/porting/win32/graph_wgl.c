#include <Windows.h>
#include <cdgraph.h>
#include <cdlog.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <cdinput.h>
#include <core/eventcodes.h>
#include <gl/gl.h>
#include <gl/glu.h>
#include <pixman.h>
typedef struct {
    HWND hwnd;
    HDC hdc;
    HGLRC hgrc;
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

static void InjectABS(unsigned long etime, int type, int axis, int value);
static void InitOpenGL(HDC hdc, int width, int height);
static void SetupPixelFormat(HDC hdc);
static void DrawScene(int width, int height);
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
    default:return DefWindowProcW(hWnd, uMsg, wParam, lParam);
    }
    return 0;
}
#define CDROID_WINDOW_CLASSNAME L"CDROID.Window"
static unsigned int __stdcall msgLooperProc(void * param)
{
    MSG message;
    int width = 1280, height = 720;
    DWORD window_style = WS_OVERLAPPEDWINDOW;
    window_style &= ~(WS_SIZEBOX | WS_MAXIMIZEBOX | WS_THICKFRAME);
    char* env = getenv("SCREEN_SIZE");
    WNDCLASSEXW window_class;
    if (env) {
        width = atoi(env);
        env = strpbrk(env, "xX*,");
        if (env)height = atoi(env + 1);
    }
    LOGI("====GL_EXTENSIONS=%s", (const char*)glGetString(GL_EXTENSIONS));
    memset(&window_class, 0, sizeof(WNDCLASSEXW));
    window_class.cbSize = sizeof(WNDCLASSEXW);
    window_class.lpfnWndProc = Window_PROC;
    window_class.lpszClassName = CDROID_WINDOW_CLASSNAME;
    ATOM atrc = RegisterClassExW(&window_class);

    devs[0].hwnd = CreateWindowExW(WS_EX_CLIENTEDGE, CDROID_WINDOW_CLASSNAME, L"CDROID",
        window_style, CW_USEDEFAULT, 0, width, height, NULL, NULL, NULL, NULL);
    devs[0].width = width;
    devs[0].height = height;
    devs[0].pitch = width * 4;
    devs[0].buffer = (char*)malloc(width * height * 4);

    devs[0].hdc = GetDC(devs[0].hwnd);
    SetupPixelFormat(devs[0].hdc);

    // 创建 OpenGL 上下文
    devs[0].hgrc = wglCreateContext(devs[0].hdc);
    glGenTextures(1, &devs[0].texture);

    glBindTexture(GL_TEXTURE_2D, devs[0].texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, devs[0].buffer);

    wglMakeCurrent(devs[0].hdc, devs[0].hgrc);
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-1.0, 1.0, 1.0, -1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    ShowWindow(devs[0].hwnd, SW_SHOW);
    UpdateWindow(devs[0].hwnd);
    SetEvent((HANDLE)param);
    while (GetMessageW(&message, devs[0].hwnd, 0, 0)) {
        TranslateMessage(&message);
        DispatchMessageW(&message);
        if (devs[0].buffer) {
           wglMakeCurrent(devs[0].hdc, devs[0].hgrc);
           DrawScene(devs[0].width, devs[0].height);
           SwapBuffers(devs[0].hdc);
        }
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
    DWORD window_style = WS_OVERLAPPEDWINDOW;
    window_style &= ~(WS_SIZEBOX | WS_MAXIMIZEBOX | WS_THICKFRAME);
    
    HANDLE mutex = CreateEventExW(NULL, CDROID_WINDOW_CLASSNAME, 0, EVENT_ALL_ACCESS);
    HANDLE thread = (HANDLE)_beginthreadex(NULL,0,msgLooperProc,mutex,0,NULL);
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

static void InitOpenGL(HDC hdc, int width, int height) {
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, ((GLdouble)width) / ((GLdouble)height), 1.0f, 100.0f);
    glMatrixMode(GL_MODELVIEW);
}
static void SetupPixelFormat(HDC hdc) {
    PIXELFORMATDESCRIPTOR pfd;
    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.iLayerType = PFD_MAIN_PLANE;

    int pixelFormat = ChoosePixelFormat(hdc, &pfd);
    SetPixelFormat(hdc, pixelFormat, &pfd);
}

static void DrawScene(int width,int height) {
    if (devs[0].buffer == NULL)return;

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, devs[0].texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, devs[0].buffer);

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, 0.0f);  // 左下角
    glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0f, -1.0f, 0.0f);   // 右下角
    glTexCoord2f(1.0f, 1.0f); glVertex3f(1.0f, 1.0f, 0.0f);    // 右上角
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f, 1.0f, 0.0f);   // 左上角
    glEnd();

    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);
    glFlush();
}
