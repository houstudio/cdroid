#include <cdtypes.h>
#include <dtvpvr.h>
#include <aui_pvr.h>
#include <aui_ca_pvr.h>
#include <aui_dsc.h>
#include <cdlog.h>
#include <ngl_timer.h>
#include <ngl_video.h>

NGL_MODULE(PVR);

static unsigned char*pvr_buffer=NULL;
static opened_pvr=0;
typedef struct{
    PVR_CALLBACK cbk;
    void*data;
}CBKDATA;

#define MAX_NOTIFIER 4 
typedef struct{
   aui_hdl hdl;
   char path[256];
   CBKDATA Notifiers[MAX_NOTIFIER];
}NGLPVR;
#define MAX_PVR 2
#define INVALID_PID 0x1FFF
static NGLPVR PVRS[MAX_PVR];
static NGLPVR PLAYERS[MAX_PVR];
static void NOTIFY(NGLPVR*pvr,UINT evt){
   int i;
   for(i=0;i<MAX_NOTIFIER;i++){
       if(pvr->Notifiers[i].cbk){
             pvr->Notifiers[i].cbk(pvr,evt,pvr->Notifiers[i].data);
       }
   }
}
DWORD nglPvrInit(){
    unsigned int pvr_buffer_len = 20*1024*1024;
    if(NULL!=pvr_buffer){//already inited
       return 0;
    }
    pvr_buffer = (unsigned char *)malloc(pvr_buffer_len);
    if(pvr_buffer == NULL){
 	LOGE("pvr_buffer is %p",pvr_buffer);
	return E_ERROR;
    }
    memset(PVRS,0,sizeof(PVRS));
    memset(PLAYERS,0,sizeof(PLAYERS));
    aui_pvr_init_param param;
    memset(&param,0,sizeof(param));
    param.max_rec_number = 2;//>2 aui_pvr_init willfailed
    param.max_play_number =1 ;
    param.ac3_decode_support = 1;
    param.continuous_tms_en = 0;
    param.debug_level   = AUI_PVR_DEBUG_ALL;
    STRCPY(param.dvr_path_prefix,"ALIDVRS2/");
    STRCPY(param.info_file_name,"info.dvr");	
    STRCPY(param.info_file_name_new,"info3.dvr");
    STRCPY(param.ts_file_format,"dvr");	
    STRCPY(param.ts_file_format_new, "ts");	
    STRCPY(param.ps_file_format,"mpg");	
    STRCPY(param.test_file1,"test_write1.dvr");
    STRCPY(param.test_file2,"test_write2.dvr");
    STRCPY(param.storeinfo_file_name,"storeinfo.dvr");
    param.record_min_len = 15;		// in seconds, recomment to 15s, the record will be deleted if shorter that this limit
    param.tms_time_max_len = 7200;	// in seconds, recomment to 2h(7200);
    param.tms_file_min_size= 2;	// in MBytes,  recomment to 10M
    param.prj_mode  = AUI_PVR_DVBS2; 
    param.cache_addr = (unsigned int)pvr_buffer;
    param.cache_size = pvr_buffer_len;

    int rc=aui_pvr_init(&param);
    LOGD("aui_pvr_init=%d",rc);
    aui_log_priority_set(AUI_MODULE_PVR,AUI_LOG_PRIO_DEBUG);

    aui_pvr_disk_attach_info dap;
    memset(&dap, 0, sizeof(aui_pvr_disk_attach_info));
    strcpy(dap.mount_name,"/mnt/usb/sda1");
    dap.disk_mode=0;//USB MODE 1-IDE MODE
    dap.disk_usage = AUI_PVR_REC_AND_TMS_DISK;
    dap.sync = 1;
    dap.init_list = 1;
    dap.check_speed = 1;
    rc=aui_pvr_disk_attach(&dap); 
    LOGD("aui_pvr_disk_attach(%s)=%d",dap.mount_name,rc);
    return rc;
}

static void ali_aui_pvr_callback(aui_hdl handle, unsigned int msg_type, unsigned int msg_code, void* user_data){
    unsigned int index;
    NGLPVR*pvr=(NGLPVR*)user_data;
    aui_pvr_pid_info aui_pid_info;
    aui_pvr_rec_item_info rec_info;
    LOGV("handle=%p msg_type=%d msg_code=%x %s",handle,msg_type,msg_code,pvr->path);
    switch(msg_type){
    case AUI_EVNT_PVR_END_DATAEND: break;
    case AUI_EVNT_PVR_END_DISKFULL:NOTIFY(pvr,NGL_PVR_DISKFULL);break;
    case AUI_EVNT_PVR_END_TMS:     
    case AUI_EVNT_PVR_END_REVS:     break;
    case AUI_EVNT_PVR_END_WRITEFAIL:NOTIFY(pvr,NGL_PVR_WRITE_FAIL);break;
    case AUI_EVNT_PVR_END_READFAIL :NOTIFY(pvr,NGL_PVR_READ_FAIL);break;
    case AUI_EVNT_PVR_TMS_OVERLAP:  break;
    case AUI_EVNT_PVR_STATUS_UPDATE:/*13*/NOTIFY(pvr,NGL_PVR_UPDATE); break;
    case AUI_EVNT_PVR_STATUS_FILE_CHG:
    case AUI_EVNT_PVR_STATUS_PID_CHG:break;
    case AUI_EVNT_PVR_SPEED_LOW:  NOTIFY(pvr,NGL_PVR_SPEED_LOW);break;
    case AUI_EVNT_PVR_STATUS_CHN_CHG:
    case AUI_EVNT_PVR_MSG_REC_START:/*18*/
    case AUI_EVNT_PVR_MSG_REC_STOP:

    case AUI_EVNT_PVR_MSG_PLAY_START:
    case AUI_EVNT_PVR_MSG_PLAY_STOP:
    case AUI_EVNT_PVR_MSG_UPDATE_KEY:
    case AUI_EVNT_PVR_MSG_UPDATE_CW:
    case AUI_EVNT_PVR_MSG_TMS_CAP_UPDATE:
    
    break;		
    }
}

static unsigned short ali_pvr_get_ts_dsc_handle_callback(unsigned short program_id,aui_hdl *p_dsc_handler){
    aui_hdl hdl;
    if (aui_find_dev_by_idx(AUI_MODULE_DSC,0, &hdl)) {
        LOGD("aui_find_dev_by_idx fault");
        return -1;
    }
    *p_dsc_handler = hdl;
    return 0;
}
static int ali_pvr_open_csa_sub_device(){
    aui_hdl hdl;
    if (aui_find_dev_by_idx(AUI_MODULE_DSC,0, &hdl)) { //Don't need open a unused DSC.
        LOGD("cannot find dsc device, ready to open it");
        return 1;
    }
    LOGD("find the used CSA device");
    return 0;
}


DWORD nglPvrRecordOpen(const char*record_path,const NGLPVR_RECORD_PARAM*param){
    unsigned int rc, i = 0;
    NGLPVR*pvr=NULL;
    for(i=0;i<MAX_PVR;i++){
       if(PVRS[i].hdl==NULL){
           pvr=PVRS+i;
       }
    }
    aui_hdl aui_pvr_handler=NULL;
    AUI_RTN_CODE ret= AUI_RTN_SUCCESS;
    aui_record_prog_param st_arp;
    aui_ca_pvr_callback ca_pvr_callback;
    aui_ca_pvr_config config;
    MEMSET(&config,0,sizeof(aui_ca_pvr_config));

    nglPvrInit();
    #ifdef AUI_LINUX
    aui_ca_pvr_config config;
    memset(&config,0,sizeof(aui_ca_pvr_config));
    #endif
    memset(&st_arp,0,sizeof(st_arp));
    
    st_arp.dmx_id=0;
    st_arp.is_tms_record=param->recordMode;
    st_arp.av_flag=param->video_pid!=INVALID_PID;
    
    st_arp.h264_flag = param->video_type;
    st_arp.user_data=pvr;
    st_arp.fn_callback = ali_aui_pvr_callback;
    st_arp.pid_info.video_pid =param->video_pid;
    st_arp.pid_info.pcr_pid =param->pcr_pid;

    LOGD("videopid=%d vtype=%d pcr_pid=%d recmode=%d",param->video_pid,param->video_type,param->pcr_pid,param->recordMode);
    for(i = 0; i < PVR_MAX_AUDIO; i++) {
        int idx=st_arp.pid_info.audio_count;
        st_arp.pid_info.audio_pid[idx] = param->audio_pids[i];
	st_arp.pid_info.audio_type[idx] =param->audio_types[i];
        if((param->audio_pids[idx]!=INVALID_PID)&&(param->audio_pids[idx]!=0)){
           LOGD("audiopid[%d]=%d type=%d",idx,param->audio_pids[i],param->audio_types[i]);
           st_arp.pid_info.audio_count++;
        }
        idx=st_arp.pid_info.ecm_pid_count;
        if((param->ecm_pids[idx]!=INVALID_PID)&&(param->ecm_pids[i]!=0)){
            st_arp.pid_info.ecm_pids[idx]=param->ecm_pids[i];
            st_arp.pid_info.ecm_pid_count++;
        }
    }
    st_arp.pid_info.pmt_pid=param->pmt_pid;
    st_arp.ca_mode=st_arp.pid_info.ecm_pid_count?1:0;//0--free 1 ca scrambled
    st_arp.rec_type = AUI_PVR_REC_TYPE_TS; 
    switch(param->encrypt){
    case 0:
         st_arp.is_reencrypt = 0;
         st_arp.rec_special_mode = AUI_PVR_NONE;
         st_arp.is_scrambled = 0;
         break;
    case 4:
         st_arp.is_reencrypt = 0;
	 st_arp.rec_special_mode = AUI_PVR_NONE;//AUI_RSM_GEN_CA_MULTI_RE_ENCRYPTION;
	 st_arp.is_scrambled = 1;
         break;
    case 6:
         st_arp.is_reencrypt = 1;
	 st_arp.rec_special_mode = AUI_PVR_VMX_MULTI_RE_ENCRYPTION;;
	 st_arp.is_scrambled = 0;
         break;
    }
    if(param->encrypt==4){
         ali_pvr_open_csa_sub_device();
         //init_pvr_encrypt_mode(AUI_PVR_VMX_MULTI_RE_ENCRYPTION);
         config.special_mode = st_arp.rec_special_mode;
   	 aui_ca_pvr_init_ext(&config);

         ca_pvr_callback.fp_pure_data_callback = NULL;
 	 ca_pvr_callback.fp_ts_callback = ali_pvr_get_ts_dsc_handle_callback;
	 aui_ca_register_callback(&ca_pvr_callback);
    }

    if(NULL==record_path){
        char fname[64];
        NGL_TIME tnow;
        NGL_TM tmn;
        nglGetTime(&tnow);
        nglTimeToTm(&tnow,&tmn);
        sprintf(fname,"PVR_%d-%d-%d_%02d%02d%02d",1900+tmn.uiYear,tmn.uiMonth,tmn.uiMonthDay,tmn.uiHour,tmn.uiMin,tmn.uiSec);
        strcpy(st_arp.folder_name,fname);
        record_path=st_arp.folder_name;      
    }else{
        LOGI("record_path=%s",record_path);
        strcpy(st_arp.folder_name,record_path);
        LOGI("st_arp.folder_name=%s",st_arp.folder_name);
    }
    rc=aui_pvr_rec_open(&st_arp,&aui_pvr_handler);
    if( (0==rc) && NULL!=pvr ){ 
        pvr->hdl=aui_pvr_handler;
        strcpy(pvr->path,st_arp.folder_name);
        opened_pvr++;
        LOGI("aui_pvr_rec_open=%d path=%s  handle=%p/%p opened_pvr=%d",rc,st_arp.folder_name,pvr,aui_pvr_handler,opened_pvr);
        return pvr;
    }
    LOGE("aui_pvr_rec_open failed %d opened_pvr=%d",rc,opened_pvr);
    return 0;
}

DWORD nglPvrRecordPause(DWORD handler){
    UINT duration;
    NGLPVR*pvr=(NGLPVR*)handler;
    if(pvr<PVRS||pvr>=&PVRS[MAX_PVR]||NULL==pvr->hdl)return E_INVALID_PARA;
    INT ret = aui_pvr_rec_state_change(pvr->hdl,AUI_PVR_REC_STATE_PAUSE);
    aui_pvr_get(handler,AUI_PVR_REC_TIME_S,&duration,0,0);
    LOGD("******pvr reocrd pause at [%d]",duration);
    return E_OK;
}

DWORD nglPvrRecordRegisterCallBack(DWORD handler,PVR_CALLBACK cbk,void*data){
    int i;
    NGLPVR*pvr=(NGLPVR*)handler;
    if(pvr<PVRS||pvr>=&PVRS[MAX_PVR]||NULL==pvr->hdl)return E_INVALID_PARA;
    for(i=0;i<MAX_NOTIFIER;i++){
         if(pvr->Notifiers[i].cbk==NULL){
              pvr->Notifiers[i].cbk=cbk;
              pvr->Notifiers[i].data=data;
              break;
         }
    }
}

DWORD nglPvrRecordResume(DWORD handler){
    UINT duration;
    NGLPVR*pvr=(NGLPVR*)handler;
    if(pvr<PVRS||pvr>=&PVRS[MAX_PVR]||NULL==pvr->hdl)return E_INVALID_PARA;
    INT ret = aui_pvr_rec_state_change(pvr->hdl,AUI_PVR_REC_STATE_RECORDING);
    aui_pvr_get(pvr->hdl,AUI_PVR_REC_TIME_S,&duration,0,0);
    LOGD("************pvr reocrd resume at [%d]",duration);
    return E_OK;
}

DWORD nglPvrRecordClose(DWORD handler){
    int rc;
    NGLPVR*pvr=(NGLPVR*)handler;
    if(pvr<PVRS||pvr>=&PVRS[MAX_PVR]||NULL==pvr->hdl)return E_INVALID_PARA;

    rc= aui_pvr_rec_close(pvr->hdl,1);
    --opened_pvr;
    LOGD("aui_pvr_rec_close=%d hdl=%p/%p opened_pvr=%d",rc,pvr,pvr->hdl,opened_pvr);
    pvr->hdl=NULL;
    memset(pvr->Notifiers,0,sizeof(pvr->Notifiers));
    LOGD("aui_pvr_rec_close %p=%d opened_pvr=%d",handler,rc,opened_pvr);
    return (AUI_RTN_SUCCESS==rc)?E_OK:E_ERROR;
}

void nglGetPvrPath(DWORD handler,char*path){
    NGLPVR*pvr=(NGLPVR*)handler;
    if(pvr<PVRS||pvr>=&PVRS[MAX_PVR])return E_ERROR;
    LOGD("handle=%p",handler);
    strcpy(path,"/mnt/usb/sda1/ALIDVRS2/");
    strcat(path,pvr->path);
}

///////////////////////////////PVR PLAYER////////////////////////////

DWORD nglPvrPlayerOpen(const char*pvrpath){
    int i,ret;
    NGLPVR*pvr=NULL;
    for(i=0;i<MAX_PVR;i++){
        if(PLAYERS[i].hdl==NULL){
             pvr=PLAYERS+i;break;
        }
    }
    aui_hdl hdl=NULL;
    aui_ply_param st_app;
    MEMSET(&st_app,0,sizeof(st_app));
    st_app.dmx_id =2;
    st_app.index =0;
    st_app.live_dmx_id =0;
    nglPvrInit();
    nglAvStop(0);
    if(NULL==pvrpath){
        strcpy(st_app.path,"/mnt/usb/sda1/ALIDVRS2/");
    }else
        strcpy(st_app.path,pvrpath);//"/mnt/uda1/ALIDVRS2/[TS]2013-11-01-13-32-30");
    st_app.preview_mode =0;
    st_app.speed  = AUI_PVR_PLAY_SPEED_1X;
    st_app.start_mode = AUI_P_OPEN_FROM_HEAD;
    st_app.start_pos =0 ;
    st_app.start_time =0;
    st_app.state = AUI_PVR_PLAY_STATE_PLAY;
    st_app.user_data=pvr;
    st_app.fn_callback=ali_aui_pvr_callback;
    ret = aui_pvr_ply_open(&st_app,&hdl);
    LOGD("aui_pvr_ply_open=%d handle=%p media=%s",ret,hdl,st_app.path);
    if(ret==0&&pvr){
        pvr->hdl=hdl;
        strcpy(pvr->path,st_app.path);
        return pvr;
    }
    return 0;
}

DWORD nglPvrPlayerPlay(DWORD handle){
    NGLPVR*pvr=(NGLPVR*)handle;
    if(pvr<PLAYERS||pvr>=&PLAYERS[MAX_PVR]||NULL==pvr->hdl)return E_INVALID_PARA;
    int ret=aui_pvr_ply_state_change(pvr->hdl,AUI_PVR_PLAY_STATE_PLAY,0); 
    LOGD("aui_pvr_ply_state_change=%d handle=%p",ret,handle);
    return ret==AUI_RTN_SUCCESS?E_OK:E_ERROR;
}

DWORD nglPvrPlayerGetTime(DWORD handle,DWORD*duration){
    int ret;
    NGLPVR*pvr=(NGLPVR*)handle;
    if(pvr<PLAYERS||pvr>=&PLAYERS[MAX_PVR]||NULL==pvr->hdl)return E_INVALID_PARA;
    ret=aui_pvr_get(pvr->hdl,AUI_PVR_ITEM_DURATION,duration,0,0);
    return ret==AUI_RTN_SUCCESS?E_OK:E_ERROR;
}

DWORD nglPvrPlayerStop(DWORD handle){
    NGLPVR*pvr=(NGLPVR*)handle;
    if(pvr<PLAYERS||pvr>=&PLAYERS[MAX_PVR]||NULL==pvr->hdl)return E_INVALID_PARA;
    int ret=aui_pvr_ply_state_change(pvr->hdl,AUI_PVR_PLAY_STATE_STOP,0);
    LOGD("aui_pvr_ply_state_change=%d handle=%p",ret,handle);
    return ret==AUI_RTN_SUCCESS?E_OK:E_ERROR;
}

DWORD nglPvrPlayerPause(DWORD handle){
    NGLPVR*pvr=(NGLPVR*)handle;
    if(pvr<PLAYERS||pvr>=&PLAYERS[MAX_PVR]||NULL==pvr->hdl)return E_INVALID_PARA;
    int ret=aui_pvr_ply_state_change(pvr->hdl,AUI_PVR_PLAY_STATE_PAUSE,0);
    LOGD("aui_pvr_play_state_chage=%d handle=%p",ret,handle);
    return ret==AUI_RTN_SUCCESS?E_OK:E_ERROR;
}

DWORD nglPvrPlayerResume(DWORD handle){
    NGLPVR*pvr=(NGLPVR*)handle;
    if(pvr<PLAYERS||pvr>=&PLAYERS[MAX_PVR]||NULL==pvr->hdl)return E_INVALID_PARA;
    int ret=aui_pvr_ply_state_change(pvr->hdl,AUI_PVR_PLAY_STATE_PLAY,0);
    return ret==AUI_RTN_SUCCESS?E_OK:E_ERROR;
}

DWORD nglPvrPlayerClose(DWORD handle){
    int ret;
    NGLPVR*pvr=(NGLPVR*)handle;
    if(pvr<PLAYERS||pvr>=&PLAYERS[MAX_PVR])return E_INVALID_PARA;
    aui_pvr_stop_ply_param st_apsp;
    st_apsp.stop_mode = AUI_PVR_STOPPED_ONLY;
    st_apsp.sync =TRUE;
    st_apsp.vpo_mode=0;
    
    ret = aui_pvr_ply_close(pvr->hdl,&st_apsp);
    pvr->hdl=NULL;
    LOGD("aui_pvr_ply_close=%d handle=%p",ret,handle);
    return ret==AUI_RTN_SUCCESS?E_OK:E_ERROR;
}

