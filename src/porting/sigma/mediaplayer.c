
#include <cdtypes.h>
#include <ngl_mediaplayer.h>
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

HANDLE nglMPOpen(const char*fname){
    NGL_PLAYER *mp=(NGL_PLAYER*)malloc(sizeof(NGL_PLAYER));
    memset(mp,0,sizeof(NGL_PLAYER));
    char*p=strchr(fname,':');
    return mp;
}

DWORD nglMPPlay(HANDLE handle){
    NGL_PLAYER*mp=(NGL_PLAYER*)handle;
    return E_OK;
}

DWORD nglMPStop(HANDLE handle){
    NGL_PLAYER*mp=(NGL_PLAYER*)handle;
    return E_OK;
}

DWORD nglMPResume(HANDLE handle){
    NGL_PLAYER*mp=(NGL_PLAYER*)handle;
    return E_OK;
}

DWORD nglMPPause(HANDLE handle){
    NGL_PLAYER*mp=(NGL_PLAYER*)handle;
    return E_OK;
}

DWORD nglMPClose(HANDLE handle){
    NGL_PLAYER*mp=(NGL_PLAYER*)handle;
    return E_OK;
}

DWORD nglMPGetTime(HANDLE handle,UINT*curtime,UINT*timems){
    return E_OK;
}
DWORD nglMPSeek(HANDLE handle,UINT timems){
    NGL_PLAYER*mp=(NGL_PLAYER*)handle;
    //mpg_cmd_set_speed
}

DWORD nglSetCallback(HANDLE handle,MP_CALLBACK cbk,void*userdata){
    NGL_PLAYER*mp=(NGL_PLAYER*)handle;
    mp->cbk=cbk;
    mp->userdata=userdata;
}

