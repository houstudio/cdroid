
#include <cdtypes.h>
#include <cdplayer.h>
#include <cdlog.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    void*hplayer;
    MP_CALLBACK cbk;
    void*userdata;
}DUMMY_PLAYER;

HANDLE MPOpen(const char*fname) {
    DUMMY_PLAYER *mp=(DUMMY_PLAYER*)malloc(sizeof(DUMMY_PLAYER));
    memset(mp,0,sizeof(DUMMY_PLAYER));
    char*p=strchr(fname,':');
    return mp;
}

int MPPlay(HANDLE handle) {
    DUMMY_PLAYER*mp=(DUMMY_PLAYER*)handle;
    return E_OK;
}

int MPStop(HANDLE handle) {
    DUMMY_PLAYER*mp=(DUMMY_PLAYER*)handle;
    return E_OK;
}

int MPResume(HANDLE handle) {
    DUMMY_PLAYER*mp=(DUMMY_PLAYER*)handle;
    return E_OK;
}

int MPPause(HANDLE handle) {
    DUMMY_PLAYER*mp=(DUMMY_PLAYER*)handle;
    return E_OK;
}

int MPClose(HANDLE handle) {
    DUMMY_PLAYER*mp=(DUMMY_PLAYER*)handle;
    return E_OK;
}

int MPSeek(HANDLE handle,double timems) {
    DUMMY_PLAYER*mp=(DUMMY_PLAYER*)handle;
    //mpg_cmd_set_speed
}

int SetCallback(HANDLE handle,MP_CALLBACK cbk,void*userdata) {
    DUMMY_PLAYER*mp=(DUMMY_PLAYER*)handle;
    mp->cbk=cbk;
    mp->userdata=userdata;
}

int MPSetWindow(HANDLE handle,int x,int y,int w,int h) {
    return 0;
}
int MPGetDuration(HANDLE hanle,double*mediatime){
    if(mediatime)*mediatime=0;
    return E_OK;
}
int MPGetPosition(HANDLE handle,double*mediatime){
   if(mediatime)*mediatime=0;
   return E_OK;
}

int MPGetStatus(HANDLE handle){
}
int MPRotate(HANDLE handle,int rot) {
    return 0;
}
