#include <cdtypes.h>
#include <ngl_os.h>
#include <ngl_graph.h>
#include <cdlog.h>
#include <window.h>

NGL_MODULE(GRAPH);

HINSTANCE hInstance;
HWND hwndOSD=NULL;
DWORD nglGraphInit(){
    MSG msg;
    hInstance=GetModuleHandle(NULL);
    hwndOSD=CreateWindowEx(WS_EX_TOOLWINDOW,_T("#32770"),(LPTSTR)NULL,WS_VISIBLE|WS_POPUP |WS_BORDER|WS_EX_CLIENTEDGE,
        100,100,400,200,NULL,(HMENU)0,hInstance,NULL);

    while(GetMessage(&msg,NULL,0,0)){
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

    return E_OK;
}

DWORD nglGetScreenSize(UINT*width,UINT*height){
    *width=1280;//dispCfg.width;
    *height=720;//dispCfg.height;
    return E_OK;
}

DWORD nglLockSurface(DWORD surface,void**buffer,UINT*pitch){
    return 0;
}

DWORD nglGetSurfaceInfo(DWORD surface,UINT*width,UINT*height,INT *format)
{
    return E_OK;
}

DWORD nglUnlockSurface(DWORD surface){
    return 0;
}

DWORD nglSurfaceSetOpacity(DWORD surface,BYTE alpha){
    return 0;//dispLayer->SetOpacity(dispLayer,alpha);
}

DWORD nglFillRect(DWORD surface,const NGLRect*rec,UINT color){
    return E_OK;
}

DWORD nglFlip(DWORD surface){
    return 0;
}

DWORD nglCreateSurface(DWORD*surface,INT width,INT height,INT format,BOOL hwsurface)
{
     return E_OK;
}


DWORD nglBlit(DWORD dstsurface,DWORD srcsurface,const NGLRect*srcrect,const NGLRect* dstrect)
{
     return 0;
}

DWORD nglDestroySurface(DWORD surface)
{
     return 0;
}
