
#include <cdtypes.h>
#include <cdplayer.h>
#include <cdlog.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    void*hplayer;
    MP_CALLBACK cbk;
    void*userdata;
} NGL_PLAYER;

HANDLE MPOpen(const char*fname) {
    NGL_PLAYER *mp=(NGL_PLAYER*)malloc(sizeof(NGL_PLAYER));
    memset(mp,0,sizeof(NGL_PLAYER));
    char*p=strchr(fname,':');
    return mp;
}

int MPPlay(HANDLE handle) {
    NGL_PLAYER*mp=(NGL_PLAYER*)handle;
    return E_OK;
}

int MPStop(HANDLE handle) {
    NGL_PLAYER*mp=(NGL_PLAYER*)handle;
    return E_OK;
}

int MPResume(HANDLE handle) {
    NGL_PLAYER*mp=(NGL_PLAYER*)handle;
    return E_OK;
}

int MPPause(HANDLE handle) {
    NGL_PLAYER*mp=(NGL_PLAYER*)handle;
    return E_OK;
}

int MPClose(HANDLE handle) {
    NGL_PLAYER*mp=(NGL_PLAYER*)handle;
    return E_OK;
}

int MPGetTime(HANDLE handle,uint32_t*curtime,uint32_t*timems) {
    return E_OK;
}

int MPSeek(HANDLE handle,double timems) {
    NGL_PLAYER*mp=(NGL_PLAYER*)handle;
    //mpg_cmd_set_speed
}

int SetCallback(HANDLE handle,MP_CALLBACK cbk,void*userdata) {
    NGL_PLAYER*mp=(NGL_PLAYER*)handle;
    mp->cbk=cbk;
    mp->userdata=userdata;
}

int MPSetWindow(HANDLE handle,int x,int y,int w,int h) {
    return 0;
}
int MPGetDuration(HANDLE hanle,double*mediatime){
}
int MPGetPosition(HANDLE handle,double*mediatime){
}
int MPGetStatus(HANDLE handle){
}
int MPRotate(HANDLE handle,int rot) {
    return 0;
}
