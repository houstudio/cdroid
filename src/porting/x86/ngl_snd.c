#include <cdtypes.h>
#include <cdlog.h>
#include <ngl_snd.h>

INT nglSndInit(){
    return E_OK;
}

static int volume=50;
INT nglSndSetVolume(int idx,int vol){
    int rc=0;
    volume=vol;
    LOGV("vol=%d rc=%d",vol,rc);
    return E_OK;
}

INT nglSndGetColume(int idx){
    LOGV(" vol=%d",volume);
    return volume;
} 
INT nglSndSetMute(int idx,BOOL mute){
    return E_OK;
}

INT nglSndSetOutput(int ifc,int type){
    return 0;
}

