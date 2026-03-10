
#include <cdtypes.h>
#include <cdplayer.h>
#include <cdlog.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include <libmedia/media_demuxing.h>
#include <libmedia/media_player.h>
#include <libmedia/decode/ffmpeg_video_decoder.h>
#include <libmedia/decode/ffmpeg_audio_decoder.h>
#include <libmedia/resample/ffmpeg_audio_resampler.h>
#include <libmedia/play/fb_video_player.h>
#include <libmedia/play/alsa_audio_player.h>
#include <libmedia/play/async_video_player.h>
#include <libmedia/play/async_audio_player.h>
#include <libmedia/rotate/hw_video_rotater.h>
typedef struct {
    struct media_player *player;
    struct media_demuxing_param demuxing_param;
    struct ffmpeg_video_decoder_param video_decoder_param;
    struct fb_video_player_param fb_param;
    struct async_video_player_param async_video_param;

    struct alsa_audio_player_param alsa_player_param;
    struct async_audio_player_param async_audio_param;
    struct audio_resampler_param resampler_param;
    struct ffmpeg_audio_decoder_param audio_decoder_param;

    struct media_player_param m_p_param;
    bool running;
    pthread_t threadId;
    MP_CALLBACK cbk;
    void*userdata;
} INGENIC_PLAYER;

HANDLE MPOpen(const char*fname) {
    INGENIC_PLAYER *mp=(INGENIC_PLAYER*)malloc(sizeof(INGENIC_PLAYER));
    memset(mp,0,sizeof(INGENIC_PLAYER));

    char *audio_device = "/dev/snd/pcmC0D0p";
    if (!access("/dev/snd/pcmC1D0p", F_OK))
        audio_device = "/dev/snd/pcmC1D0p";

    ffmpeg_demuxing_init_param(&mp->demuxing_param);
    ffmpeg_video_decoder_init_param(&mp->video_decoder_param);
    fb_video_player_init_default_param(&mp->fb_param);
    async_video_player_init_param(&mp->async_video_param, mp->fb_param.fb_device, &mp->fb_param.param);

    ffmpeg_audio_decoder_init_param(&mp->audio_decoder_param);
    alsa_audio_player_init_default_param(&mp->alsa_player_param, 2, 48000);
    async_audio_player_init_param(&mp->async_audio_param, audio_device, &mp->alsa_player_param.param);
    ffmpeg_audio_resample_init_default_param(&mp->resampler_param, 2, 48000, AUDIO_fltp, AUDIO_s16le);

    mp->m_p_param.demuxing_param = &mp->demuxing_param;
    mp->m_p_param.video_player_param = &mp->async_video_param.param;
    mp->m_p_param.video_decoder_param = &mp->video_decoder_param.param;

    mp->m_p_param.audio_player_param = &mp->async_audio_param.param;
    mp->m_p_param.audio_decoder_param = &mp->audio_decoder_param.param;
    mp->m_p_param.audio_resampler_param = &mp->resampler_param;

    mp->m_p_param.demuxing_param->input_file = fname;
    mp->m_p_param.decode_buf_size = 1;

    mp->running =false;
    mp->player=media_player_open(&mp->m_p_param);
    return mp;
}

static void* PlayProc(void*p){
    INGENIC_PLAYER*mp=(INGENIC_PLAYER*)p;
    mp->running=true;
    while(mp&&mp->running){
        media_player_play_one_frame(mp->player);
    }
}

int MPPlay(HANDLE handle) {
    INGENIC_PLAYER*mp=(INGENIC_PLAYER*)handle;
    pthread_create(&mp->threadId,NULL,PlayProc,mp);
    return E_OK;
}

int MPStop(HANDLE handle) {
    INGENIC_PLAYER*mp=(INGENIC_PLAYER*)handle;
    return E_OK;
}

int MPResume(HANDLE handle) {
    INGENIC_PLAYER*mp=(INGENIC_PLAYER*)handle;
    media_player_resume(mp->player);
    return E_OK;
}

int MPPause(HANDLE handle) {
    INGENIC_PLAYER*mp=(INGENIC_PLAYER*)handle;
    media_player_pause(mp->player);
    return E_OK;
}

int MPClose(HANDLE handle) {
    INGENIC_PLAYER*mp=(INGENIC_PLAYER*)handle;
    void* thread_return_value;
    media_player_close(mp->player);

    pthread_join(mp->threadId,&thread_return_value);
    free(mp);
    return E_OK;
}

int MPSetVolume(HANDLE handle,int colume){
    INGENIC_PLAYER*mp=(INGENIC_PLAYER*)handle;
    LOGD("todo:");
    return E_OK;
}

int MPSeek(HANDLE handle,double timems) {
    INGENIC_PLAYER*mp=(INGENIC_PLAYER*)handle;
    if(timems<0){
        const int64_t us=(int64_t)(timems*1000LL);
        media_player_seek_backward(mp->player,us);
    }else{
        const int64_t us=(int64_t)(timems*1000LL,us);
        media_player_seek_forward(mp->player,us);
    }
}

int MPSetCallback(HANDLE handle,MP_CALLBACK cbk,void*userdata) {
    INGENIC_PLAYER*mp=(INGENIC_PLAYER*)handle;
    mp->cbk=cbk;
    mp->userdata=userdata;
}

int MPSetWindow(HANDLE handle,int x,int y,int w,int h) {
    INGENIC_PLAYER*mp=(INGENIC_PLAYER*)handle;
    struct video_display_config fb_display_config = {
        .xpos = x,
        .ypos = y,
        .xres = w,
        .yres = h,
    };
    media_player_set_video_display_config(mp->player,&fb_display_config);
    return 0;
}

int MPGetDuration(HANDLE handle,double*mediatims){
    INGENIC_PLAYER*mp=(INGENIC_PLAYER*)handle;
    uint64_t dur;
    media_player_get_duration(mp->player,&dur);
    if(mediatims){
        *mediatims=dur/1000.0;
    }
    return E_OK;
}

int MPGetPosition(HANDLE handle,double*mediatims){
    INGENIC_PLAYER*mp=(INGENIC_PLAYER*)handle;
    int64_t cur=media_player_current_time(mp->player);
    if(mediatims){
        *mediatims=cur/1000.0;
    }
    return E_OK;
}

int MPGetStatus(HANDLE handle){
}
int MPRotate(HANDLE handle,int rot) {
    return 0;
}
