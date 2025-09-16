
#include <cdtypes.h>
#include <cdplayer.h>
#include <cdlog.h>
#include <stdlib.h>
#include <string.h>
//#include <interface.h>
#include <cdgraph.h>
#include <signal.h>
#include <time.h>

typedef struct{
   void*hplayer;
   char*mediaURL;
   MP_CALLBACK cbk;
   void*userdata;
   GFXRect viewPort;
}MP_PLAYER;
#if 0
static timer_t timerid=0;

static void timer_handler(int signum/*,siginfo_t*, void**/) {
    LOGV("TODO:add mediaplayer callback here");
}

static timer_t initTimer(){
    struct sigaction sa;
    struct sigevent sev;
    timer_t timerid;
    struct itimerspec its;
    long long freq_nanosecs = 500*1e6;//500ms
    clockid_t clockid = CLOCK_REALTIME;

    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = timer_handler;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGRTMIN, &sa, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_signo = SIGRTMIN;
    sev.sigev_value.sival_ptr = &timerid;
    if (timer_create(clockid, &sev, &timerid) == -1) {
        perror("timer_create");
        exit(EXIT_FAILURE);
    }

    its.it_value.tv_sec = 1;
    its.it_value.tv_nsec = 0;
    its.it_interval.tv_sec = freq_nanosecs / 1000000000;
    its.it_interval.tv_nsec = freq_nanosecs % 1000000000;
    if (timer_settime(timerid, 0, &its, NULL) == -1) {
        perror("timer_settime");
        return -1;//exit(EXIT_FAILURE);
    }
    return timerid;
}
#endif
HANDLE MPOpen(const char*fname){
    MP_PLAYER *mp=(MP_PLAYER*)malloc(sizeof(MP_PLAYER));
    memset(mp,0,sizeof(MP_PLAYER));
    mp->mediaURL=strdup(fname);//
    //if(timerid==0)timerid=initTimer();
    return mp;
}

int MPPlay(HANDLE handle){
    MP_PLAYER*mp=(MP_PLAYER*)handle;
    GFXRect r = mp->viewPort;
    //mm_player_open(mp->mediaURL,r.x,r.y,r.w,r.h);
    return E_OK;
}

int MPStop(HANDLE handle){
    MP_PLAYER*mp=(MP_PLAYER*)handle;
    //mm_player_stop();no stop api
    return E_OK;
}

int MPResume(HANDLE handle){
    MP_PLAYER*mp=(MP_PLAYER*)handle;
    //mm_player_resume();
    return E_OK;
}

int MPPause(HANDLE handle){
    MP_PLAYER*mp=(MP_PLAYER*)handle;
    //mm_player_pause();
    return E_OK;
}

int MPClose(HANDLE handle){
    MP_PLAYER*mp=(MP_PLAYER*)handle;
    free(mp->mediaURL);
    free(mp);
    //mm_player_close();
    return E_OK;
}

int MPGetPosition(HANDLE handle,double*pos){
    //mm_player_getposition(pos);
    return E_OK;
}

int MPGetDuration(HANDLE handle,double*dur){
    //mm_player_getduration(dur);
}

int MPSeek(HANDLE handle,double pos){
    MP_PLAYER*mp=(MP_PLAYER*)handle;
    //mpg_cmd_set_speed
    //mm_player_seek2time(pos);
}

int MPSetCallback(HANDLE handle,MP_CALLBACK cbk,void*userdata){
    MP_PLAYER*mp=(MP_PLAYER*)handle;
    mp->cbk=cbk;
    mp->userdata=userdata;
}

int MPSetWindow(HANDLE handle,int x,int y,int width,int height){
    MP_PLAYER*mp=(MP_PLAYER*)handle;
    GFXRect *r=&mp->viewPort;
    r->x = x;
    r->y = y;
    r->w = width;
    r->h = height;
    return 0;
}

int MPRotate(HANDLE handle, int type) {
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

    //mm_player_set_opts("video_rotate", "", type);

    return 0;
}

int MPGetStatus(HANDLE handle) {
    return 0;//mm_player_get_status();
}

int MPFlushScreen(bool enable) {
    //mm_player_flush_screen(enable);
    return 0;
}
