#include <aui_dmx.h>
#include <aui_decv.h>
#include <aui_deca.h>
#include <aui_snd.h>
#include <ngl_types.h>
#include <ngl_log.h>

NGL_MODULE(AUDIO);

static aui_hdl snd_hdl=NULL;
INT nglSndInit(){
    if(snd_hdl!=NULL)return E_OK;
    aui_attr_snd attr_snd;
    MEMSET(&attr_snd,0,sizeof(aui_attr_snd));
    aui_snd_init(NULL,NULL);
    if(aui_find_dev_by_idx(AUI_MODULE_SND, 0, &snd_hdl)) {
        if (aui_snd_open(&attr_snd,&snd_hdl)) {
            NGLOGD("\n aui_snd_open fail\n");
            return -1;
        }
    }
    return E_OK;
}

INT nglSndSetVolume(int idx,int vol){
    int rc=aui_snd_vol_set(snd_hdl,vol);
    NGLOGV("snd_hdl=%p vol=%d rc=%d",snd_hdl,vol,rc);
    return E_OK;
}

INT nglSndGetColume(int idx){
    BYTE vol;
    int rc=aui_snd_vol_get(snd_hdl,&vol);
    NGLOGV("snd_hdl=%p vol=%d rc=%d",snd_hdl,vol,rc);
    return vol;
} 
INT nglSndSetMute(int idx,BOOL mute){
    return aui_snd_mute_set(snd_hdl,mute);
}

INT nglSndSetOutput(int ifc,int type){
    int ret;
    aui_attr_snd attr_snd;
    aui_snd_out_mode SndMode;
    aui_snd_out_type_status snd_out_type;
    MEMSET(&attr_snd,0,sizeof(aui_attr_snd));
    MEMSET(&SndMode,0,sizeof(aui_snd_out_mode));
#if 1 
    switch(ifc){
    case 0:snd_out_type.snd_out_type=AUI_SND_OUT_I2SO;
           break;
    case 1:snd_out_type.snd_out_type=AUI_SND_OUT_SPDIF;
           SndMode.snd_out_type =  AUI_SND_OUT_SPDIF;
           break;
    case 2:snd_out_type.snd_out_type=AUI_SND_OUT_HDMI;
           SndMode.snd_out_type =  AUI_SND_OUT_HDMI;
           break;
    default:break;
    }
    snd_out_type.uc_enabel = 1;
    switch(type){
    case 0://OFF:
         //snd_out_type.snd_out_type=AUI_SND_OUT_I2SO;
         snd_out_type.uc_enabel=0;
         aui_snd_out_interface_type_set(snd_hdl,snd_out_type);
         break;
    case 1://PCM
         SndMode.snd_data_type = AUI_SND_OUT_MODE_DECODED;
         //snd_out_type.snd_out_type = AUI_SND_OUT_HDMI;
         ret = aui_snd_out_interface_type_set(snd_hdl,snd_out_type);
         ret = aui_snd_out_data_type_set(snd_hdl,SndMode);
         break;
    case 2:
         SndMode.snd_data_type = AUI_SND_OUT_MODE_ENCODED;
         //snd_out_type.snd_out_type = AUI_SND_OUT_HDMI;
         ret = aui_snd_out_interface_type_set(snd_hdl,snd_out_type);
         ret = aui_snd_out_data_type_set(snd_hdl,SndMode);
         break;
    }
    NGLOGD("interface=%d outtype=%d ret=%d",ifc,type,ret);
#else
    snd_out_type.uc_enabel = 1;
    switch(type){
    case 0://OFF:
         snd_out_type.snd_out_type=AUI_SND_OUT_I2SO;
         ret=aui_snd_out_interface_type_set(snd_hdl,snd_out_type);
         NGLOGD("snd_hdl=%p ret=%d",snd_hdl,ret); 
         break;
    case 1://PCM:
         SndMode.snd_data_type = AUI_SND_OUT_MODE_DECODED;
	 SndMode.snd_out_type =  AUI_SND_OUT_HDMI;
	 snd_out_type.snd_out_type = AUI_SND_OUT_HDMI;
         ret = aui_snd_out_interface_type_set(snd_hdl,snd_out_type);
 	 ret = aui_snd_out_data_type_set(snd_hdl,SndMode);
         NGLOGD("snd_hdl=%p ret=%d",snd_hdl,ret); 
  
  	 SndMode.snd_data_type = AUI_SND_OUT_MODE_DECODED;
         SndMode.snd_out_type =  AUI_SND_OUT_SPDIF;
	 snd_out_type.snd_out_type = AUI_SND_OUT_SPDIF;
	 ret = aui_snd_out_interface_type_set(snd_hdl,snd_out_type);
	 ret = aui_snd_out_data_type_set(snd_hdl,SndMode);
         NGLOGD("snd_hdl=%p ret=%d",snd_hdl,ret); 
         break;
    case 2://PASS:
 	 SndMode.snd_data_type = AUI_SND_OUT_MODE_ENCODED;
	 SndMode.snd_out_type =  AUI_SND_OUT_HDMI;
	 snd_out_type.snd_out_type = AUI_SND_OUT_HDMI;
	 ret = aui_snd_out_interface_type_set(snd_hdl,snd_out_type);
	 ret = aui_snd_out_data_type_set(snd_hdl,SndMode);
         NGLOGD("snd_hdl=%p ret=%d",snd_hdl,ret); 
	 
         SndMode.snd_data_type = AUI_SND_OUT_MODE_ENCODED;
	 SndMode.snd_out_type =  AUI_SND_OUT_SPDIF;
	 snd_out_type.snd_out_type = AUI_SND_OUT_SPDIF;
	 ret = aui_snd_out_interface_type_set(snd_hdl,snd_out_type);
	 ret = aui_snd_out_data_type_set(snd_hdl,SndMode);
         NGLOGD("snd_hdl=%p ret=%d",snd_hdl,ret); 
         break;
    }
#endif
    return ret;
}

