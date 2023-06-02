
#include <cdtypes.h>
#include <cdplayer.h>
#include <cdlog.h>
#include <ngl_os.h>
#include <stdlib.h>
#include <string.h>

NGL_MODULE(NGLMP);

typedef struct{
   void*hplayer;
   MP_CALLBACK cbk;
   void*userdata;
}NGL_PLAYER;

HANDLE MPOpen(const char*fname){
    NGL_PLAYER *mp=(NGL_PLAYER*)malloc(sizeof(NGL_PLAYER));
    memset(mp,0,sizeof(NGL_PLAYER));
    char*p=strchr(fname,':');
    return mp;
}

DWORD MPPlay(HANDLE handle){
    NGL_PLAYER*mp=(NGL_PLAYER*)handle;
    return E_OK;
}

DWORD MPStop(HANDLE handle){
    NGL_PLAYER*mp=(NGL_PLAYER*)handle;
    return E_OK;
}

DWORD MPResume(HANDLE handle){
    NGL_PLAYER*mp=(NGL_PLAYER*)handle;
    return E_OK;
}

DWORD MPPause(HANDLE handle){
    NGL_PLAYER*mp=(NGL_PLAYER*)handle;
    return E_OK;
}

DWORD MPClose(HANDLE handle){
    NGL_PLAYER*mp=(NGL_PLAYER*)handle;
    return E_OK;
}

DWORD MPGetTime(HANDLE handle,UINT*curtime,UINT*timems){
    return E_OK;
}
DWORD MPSeek(HANDLE handle,double timems){
    NGL_PLAYER*mp=(NGL_PLAYER*)handle;
    //mpg_cmd_set_speed
}

DWORD SetCallback(HANDLE handle,MP_CALLBACK cbk,void*userdata){
    NGL_PLAYER*mp=(NGL_PLAYER*)handle;
    mp->cbk=cbk;
    mp->userdata=userdata;
}

DWORD MPSetWindow(HANDLE handle,int x,int y,int w,int h){
    return 0;
}

DWORD MPRotate(HANDLE handle,int rot){
    return 0;
}
