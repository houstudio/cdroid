
#include <cdtypes.h>
#include <cdplayer.h>
#include <cdlog.h>
#include <ngl_os.h>
#include <stdlib.h>
#include <string.h>
#include <interface.h>
#include <cdgraph.h>

typedef struct{
   void*hplayer;
   char*mediaURL;
   MP_CALLBACK cbk;
   void*userdata;
   GFXRect viewPort;
}MP_PLAYER;

HANDLE MPOpen(const char*fname){
    MP_PLAYER *mp=(MP_PLAYER*)malloc(sizeof(MP_PLAYER));
    memset(mp,0,sizeof(MP_PLAYER));
    mp->mediaURL=strdup(fname);//
    return mp;
}

DWORD MPPlay(HANDLE handle){
    MP_PLAYER*mp=(MP_PLAYER*)handle;
    GFXRect r = mp->viewPort;
    mm_player_open(mp->mediaURL,r.x,r.y,r.w,r.h);// uint16_t x, uint16_t y, uint16_t width, uint16_t height)
    return E_OK;
}

DWORD MPStop(HANDLE handle){
    MP_PLAYER*mp=(MP_PLAYER*)handle;
    //mm_player_stop();no stop api
    return E_OK;
}

DWORD MPResume(HANDLE handle){
    MP_PLAYER*mp=(MP_PLAYER*)handle;
    mm_player_resume();
    return E_OK;
}

DWORD MPPause(HANDLE handle){
    MP_PLAYER*mp=(MP_PLAYER*)handle;
    mm_player_pause();
    return E_OK;
}

DWORD MPClose(HANDLE handle){
    MP_PLAYER*mp=(MP_PLAYER*)handle;
    free(mp->mediaURL);
    free(mp);
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
    MP_PLAYER*mp=(MP_PLAYER*)handle;
    //mpg_cmd_set_speed
    mm_player_seek2time(pos);
}

DWORD MPSetCallback(HANDLE handle,MP_CALLBACK cbk,void*userdata){
    MP_PLAYER*mp=(MP_PLAYER*)handle;
    mp->cbk=cbk;
    mp->userdata=userdata;
}

DWORD MPSetWindow(HANDLE handle,int x,int y,int width,int height){
    MP_PLAYER*mp=(MP_PLAYER*)handle;
    GFXRect *r=&mp->viewPort;
    r->x = x;
    r->y = y;
    r->w = width;
    r->h = height;
    return 0;
}

DWORD MPRotate(HANDLE handle, int type) {
    MP_PLAYER *mp =(MP_PLAYER*)handle;
    GFXRect   *r  = &mp->viewPort;

    switch (type)
    {
    case 1/*顺时针旋转90:AV_ROTATE_90*/:{
        int t;
        t = r->x;
        r->x = r->y;
        r->y = t;
        t = r->w;
        r->w = r->h;
        r->h = t;
    }break;

    default:
        return -1;
    }

    mm_player_set_opts("video_rotate", "", type);

    return 0;
}

int MPGetStatus(HANDLE handle) {
    return mm_player_get_status();
}

int MPFlushScreen(bool enable) {
    mm_player_flush_screen(enable);
    return 0;
}
