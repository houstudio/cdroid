
#include <cdtypes.h>
#include <cdplayer.h>
#include <cdlog.h>
#include <ngl_os.h>
#include <stdlib.h>
#include <string.h>
#include <interface.h>


typedef struct{
   void*hplayer;
   MP_CALLBACK cbk;
   void*userdata;
}NGL_PLAYER;

HANDLE MPOpen(const char*fname){
    NGL_PLAYER *mp=(NGL_PLAYER*)malloc(sizeof(NGL_PLAYER));
    memset(mp,0,sizeof(NGL_PLAYER));
    char*p=strchr(fname,':');
    mm_player_open(fname,0,0,0,0);// uint16_t x, uint16_t y, uint16_t width, uint16_t height)
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

DWORD nglMPResume(HANDLE handle){
    NGL_PLAYER*mp=(NGL_PLAYER*)handle;
    mm_player_resume();
    return E_OK;
}

DWORD MPPause(HANDLE handle){
    NGL_PLAYER*mp=(NGL_PLAYER*)handle;
    mm_player_pause();
    return E_OK;
}

DWORD MPClose(HANDLE handle){
    NGL_PLAYER*mp=(NGL_PLAYER*)handle;
    mm_player_close();
    return E_OK;
}

DWORD MPGetPosition(HANDLE handle,double*pos){
    mm_player_getposition(pos);
    return E_OK;
}

DWORD MPGetDuration(HANDLE handle,double*dur){
    mm_player_getduration(dur);
}

DWORD MPSeek(HANDLE handle,double pos){
    NGL_PLAYER*mp=(NGL_PLAYER*)handle;
    //mpg_cmd_set_speed
    mm_player_seek2time(pos);
}

DWORD SetCallback(HANDLE handle,MP_CALLBACK cbk,void*userdata){
    NGL_PLAYER*mp=(NGL_PLAYER*)handle;
    mp->cbk=cbk;
    mp->userdata=userdata;
}

