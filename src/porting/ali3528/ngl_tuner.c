#include "dtvtuner.h"
#include "aui_nim.h"
#include <aui_gpio.h>
#include <inttypes.h>
#include <dtvos.h>
#include <cdlog.h>
#include <dtvmsgq.h>
#include <aui_common.h>
#include <aui_gpio.h>

#define NB_TUNERS 1
#define MAX_LISTENERS 32

typedef struct{
    int tuneridx;
    NGLTunerCallBack Callback;
    INT locked;
    void *param;
}TUNERNOTIFY;

static TUNERNOTIFY sCallBacks[MAX_LISTENERS];
static NGLMutex nim_mutex=0;
static HANDLE tunerMSGQ;
typedef struct{
   int nim_id;
   aui_hdl hdl;
   aui_demod_type type;
   struct aui_nim_connect_param param;
   struct aui_nim_attr attr;
   INT locked;
   INT autoScan; 
   void*data;//used by autoscan
}NGLTUNER;

static NGLTUNER sTuners[]={
  {0,NULL,AUI_NIM_QPSK},
  {1,NULL,AUI_NIM_QPSK},
  {2,NULL,AUI_NIM_QPSK},
  {3,NULL,AUI_NIM_QPSK}
};

typedef struct{
   int idx;
   NGLTunerParam param;
}TUNERMSG;
static DWORD  nglTunerLockInner(int tuneridx,NGLTunerParam*tp);

static void NotifyLockState(int tuneridx,int newstate){
    for(int j=0;j<MAX_LISTENERS;j++){
       if(sCallBacks[j].tuneridx!=tuneridx)continue;
       if(NULL==sCallBacks[j].Callback)continue;
       if(sTuners[tuneridx].locked!=newstate){
           sCallBacks[j].Callback(tuneridx,AUI_NIM_STATUS_LOCKED==newstate,sCallBacks[j].param);
           sCallBacks[j].locked=newstate;
       }
    }
    sTuners[tuneridx].locked=newstate;
}
static void TunerStateProc(void*p){
   while(true){
      int rc,i,j;
      TUNERMSG msg;
      rc=nglMsgQReceive(tunerMSGQ,&msg,sizeof(msg),500);
      if(rc==E_OK){
          INT locked=-2;
          INT timeout=1800;
          nglTunerLockInner(msg.idx,&msg.param);
          sTuners[i].locked=-1;
          while( timeout>0 && locked!=1 ){
              rc=aui_nim_is_lock(sTuners[msg.idx].hdl,&locked);
              nglSleep(50);timeout-=50;
          }
          NotifyLockState(msg.idx,locked);
          continue;
      } 
   } 
}

static AUI_RTN_CODE nim_init_cb(void *pv_param);
DWORD nglTunerInit(){
    int i,ret ;
    DWORD threadId;
    if(0!=nim_mutex)return E_OK;
    nglCreateMutex(&nim_mutex);
    tunerMSGQ=nglMsgQCreate(2,sizeof(TUNERMSG));
    ret=aui_nim_init(nim_init_cb);
    aui_log_priority_set(AUI_MODULE_NIM,AUI_LOG_PRIO_DEBUG);
    for(i=0;i<NB_TUNERS;i++){
        memset(&sTuners[i].attr,0x00,sizeof(aui_nim_attr));
        memset(&sTuners[i].param,0,sizeof(aui_nim_connect_param));
        sTuners[i].locked=-1;
        sTuners[i].nim_id=sTuners[i].attr.ul_nim_id = i;
        sTuners[i].attr.en_dmod_type = sTuners[i].type;
        LOGD("init tuner %d",i);
        if(AUI_RTN_SUCCESS!=aui_find_dev_by_idx(AUI_MODULE_NIM,i,&sTuners[i].hdl)){
            LOGD("aui_find_dev_by_idx reopen");
            ret = aui_nim_open(&sTuners[i].attr, &sTuners[i].hdl);
        }
        LOGD("aui_nim_open=%d nim_id=%d type=%d hdl=%p",ret,i, sTuners[i].type,sTuners[i].hdl);
        if(sTuners[i].attr.en_dmod_type == AUI_NIM_QPSK && ret==0 ){
            ret=aui_diseqc_oper(sTuners[i].hdl,AUI_DISEQC_MODE_BURST0,NULL,0,NULL,NULL);
            LOGD("aui_diseqc_oper =%d",ret);
        }
    }
    nglCreateThread(&threadId,0,1024,TunerStateProc,NULL);
    memset(sCallBacks,0,sizeof(sCallBacks));
    return 0;
}

enum { BAND_KU, BAND_C };
static int dvbs_freq2inter(int freq,int polar,int *band,UINT LO)
{
    int lnb_mode=freq>5150?BAND_KU : BAND_C;
    //LOGD("*freq=%d lnb_mode=%d polar=%d",freq,lnb_mode,polar);
    if(LO > 0){
	if (lnb_mode == BAND_KU) {
            *band = (freq>11700)?AUI_NIM_HIGH_BAND:AUI_NIM_LOW_BAND;
            LOGD("freq=%d LO=%d freq-LO=%d",freq,LO,freq-LO);
   	    return freq - LO;//x-10600=13100;
       } else {// C band Cause:
               // If the band is not AUI_NIM_LOW_BAND/AUI_NIM_HIGH_BAND,
               // polar/band setting will not be do in NIM connect and auto scan.
            *band = AUI_NIM_LOW_BAND;
            LOGD("freq=%d LO=%d LO-freq=%d",freq,LO,LO-freq);
            return LO - freq;
    	}
    }
    if (lnb_mode == BAND_KU) {
        *band=(freq > 11700)?AUI_NIM_HIGH_BAND:AUI_NIM_LOW_BAND;
        freq-=((freq > 11700)?10600:9750);
        return freq;
    } else {// C band Cause:
            // If the band is not AUI_NIM_LOW_BAND/AUI_NIM_HIGH_BAND,
            // polar/band setting will not be do in NIM connect and auto scan.
        *band = AUI_NIM_LOW_BAND;
        return (polar == AUI_NIM_POLAR_VERTICAL)?(5750 - freq):(5150 - freq);
    }
}

static unsigned int  Polarity_Control(UINT TUNER_Polarity)
{
    aui_hdl hdl = 0;
    AUI_RTN_CODE ret = AUI_RTN_FAIL;	
    aui_gpio_attr gpio_power;
	
    memset(&gpio_power, 0, sizeof(aui_gpio_attr));
    gpio_power.io = AUI_GPIO_O_DIR;
    gpio_power.uc_dev_idx = 96;//control  H/V
    ret= aui_gpio_open(&gpio_power, &hdl);
    if(!ret){
   	if(TUNER_Polarity)// 1: V
 	    ret = aui_gpio_set_value(hdl, 0);
	else//H
	    ret = aui_gpio_set_value(hdl, 1);
    }
    return ret;
}

static DWORD  nglTunerLockInner(int tuneridx,NGLTunerParam*tp){
    int rc,TUNER_Polarity=2;
    int high_band = 0;
    struct aui_nim_connect_param*pc=&sTuners[tuneridx].param;
    switch(sTuners[tuneridx].type){
    case AUI_NIM_QAM:  /* DVB-C */
	pc->connect_param.cab.ul_symrate = tp->u.c.symbol_rate;
	pc->connect_param.cab.en_qam_mode =tp->u.c.modulation;
        break;
    case AUI_NIM_QPSK:	/* DVB-S */
        pc->ul_freq = dvbs_freq2inter(tp->frequency/1000,tp->u.s.polar, &high_band,tp->u.s.lnbfreq);
	pc->connect_param.sat.ul_freq_band = AUI_NIM_BAND_INVALID;//high_band; /* chinasat6b */
	pc->connect_param.sat.ul_symrate =tp->u.s.symbol_rate;//TUNER_SymbolRate;
	pc->connect_param.sat.fec = AUI_NIM_FEC_AUTO;//TUNER_Fec;
	pc->connect_param.sat.ul_polar = tp->u.s.polar;//requierment
	pc->connect_param.sat.ul_src = 0;//LNBStatus;
        //pc->connect_param.sat.ul_freq_band=0;
	break;
    case AUI_NIM_OFDM: /* DVB-T */
	switch (tp->u.t.bandwidth) {
	case NGL_NIM_BANDWIDTH_6_MHZ: pc->connect_param.ter.bandwidth = AUI_NIM_BANDWIDTH_6_MHZ; break;
	case NGL_NIM_BANDWIDTH_7_MHZ: pc->connect_param.ter.bandwidth = AUI_NIM_BANDWIDTH_7_MHZ; break;
	case NGL_NIM_BANDWIDTH_8_MHZ: pc->connect_param.ter.bandwidth = AUI_NIM_BANDWIDTH_8_MHZ; break;
	default :return -1;
	}
	pc->connect_param.ter.std = AUI_NIM_STD_ISDBT;//AUI_STD_DVBT2_COMBO(DVBT/DVBT2); /* automatic detection between DVBT and DVBT2, preferred DVBT2 */
	pc->connect_param.ter.fec = AUI_NIM_FEC_AUTO;
	pc->connect_param.ter.spectrum = AUI_NIM_SPEC_AUTO;
    default:
	LOGD("%s %d -> nim type %d is wrong!", __FUNCTION__, __LINE__,tp->delivery_type);
	return E_ERROR;
    }
    Polarity_Control((sTuners[tuneridx].type==AUI_NIM_QPSK)?tp->u.s.polar:0);
    nglLockMutex(nim_mutex);
    sTuners[tuneridx].locked=-1;//set to -1 ,make sure atleast 1 state callback will be called
    rc=aui_nim_connect(sTuners[tuneridx].hdl, &sTuners[tuneridx].param);
    nglUnlockMutex(nim_mutex);
    LOGD("aui_nim_connect(hdl=%p,ul_freq=%d/%d)=%d",sTuners[tuneridx].hdl,pc->ul_freq,tp->frequency,rc);
    return rc?E_ERROR:E_OK;
}
DWORD  nglTunerLock(int tuneridx,NGLTunerParam*tp){
    TUNERMSG msg;
    msg.idx=tuneridx;
    msg.param=*tp;
    return nglMsgQSend(tunerMSGQ,&msg,sizeof(msg),100);
}

#define TP_MAX 128
typedef struct{//AUTOSCANDATA{
    aui_hdl hdl;
    int tuneridx;
    int tpidx;
    aui_autoscan_sat sp;
    NGLTunerAutoScan CBK;
    void*userdata;
    NGLTunerParam results[TP_MAX];
}AUTOSCANDATA;
static int aui_autoscanCBK(unsigned char uc_status, unsigned char uc_polar,unsigned int u_freq,unsigned int u_sym, 
    unsigned char uc_fec,void *pv_user_data,unsigned int u_stream_id){
    //uc_status 0 unlocked,1 locked
    AUTOSCANDATA*sp=(AUTOSCANDATA*)pv_user_data;
    LOGV("LOCKED sp=%p freq %d sym=%d uc_status:%d sp->tpidx:%d",sp,u_freq,u_sym,uc_status,sp->tpidx);
    if(uc_status){
        NGLTunerParam*tp=sp->results+sp->tpidx;
        tp->frequency=u_freq;
        tp->u.s.polar=uc_polar;//todo convert to ngl polar define
        tp->u.s.symbol_rate=u_sym;
        tp->u.s.fec=uc_fec;//todo converto ngl define
        LOGV("frequency:%d polar:%d symbol_rate:%d fec:%d",tp->frequency,tp->u.s.polar,tp->u.s.symbol_rate,tp->u.s.fec);
        sp->tpidx++;
    }
    return 0;
}

static ThreadProc(void*param){
    AUTOSCANDATA*sd=(AUTOSCANDATA*)param;
    int ret=0;
    for(int i=0;i<2;i++){
        sd->sp.ul_polar=i;
        ret=aui_nim_auto_scan_start(sd->hdl,&sd->sp);   
        ret=aui_nim_auto_scan_stop(sd->hdl);
    }//add multi sat search later MX
    sd->CBK(sd->tuneridx,sd->results,sd->tpidx,sd->userdata);        
    LOGI("AutoScan finished recv %d TP",sd->tpidx);
    sTuners[sd->tuneridx].autoScan--;
}

DWORD nglTunerAutoScan(INT tuneridx,const NGLTunerParam*start,INT endfreq,NGLTunerAutoScan cbk,void*userdata){
    
    HANDLE threadid;
    AUTOSCANDATA*sd=(AUTOSCANDATA*)sTuners[tuneridx].data;
    if(sTuners[tuneridx].autoScan){
        LOGI("Tuner[%d]'s  autoscan is running",tuneridx);
        return -1;
    }
    if(sd==NULL){
       sd=(AUTOSCANDATA*)malloc(sizeof(AUTOSCANDATA));
       sTuners[tuneridx].data=sd;
    }
    sTuners[tuneridx].autoScan++;
    bzero(sd,sizeof(AUTOSCANDATA));
    sd->hdl=sTuners[tuneridx].hdl;
    sd->sp.ul_start_freq=start->frequency;
    sd->sp.ul_stop_freq=endfreq;
    //sd->sp.ul_polar=NGL_NIM_POLAR_HORIZONTAL;
    sd->sp.aui_as_cb=aui_autoscanCBK;
    sd->sp.pv_user_data=sd;
    sd->CBK=cbk;
    sd->userdata=userdata;
    sd->tpidx=0;
    sd->tuneridx=tuneridx;

    nglCreateThread(&threadid,0,0,ThreadProc,sd);
    return 0;
}

DWORD nglTunerRegisteCBK(INT tuneridx,NGLTunerCallBack cbk,void*param){
    int i;
    nglLockMutex(nim_mutex); 
    for(i=0;i<MAX_LISTENERS;i++){
        if(sCallBacks[i].Callback==NULL){
            sCallBacks[i].tuneridx=tuneridx;
            sCallBacks[i].Callback=cbk;
            sCallBacks[i].param=param;
            sCallBacks[i].locked=-1;
            nglUnlockMutex(nim_mutex);
            return E_OK;
        }
    }
    nglUnlockMutex(nim_mutex);
    return E_ERROR;
}

DWORD nglTunerUnRegisteCBK(INT tuneridx,NGLTunerCallBack cbk){
    int i;
    for(i=0;i<MAX_LISTENERS;i++){
        if(sCallBacks[i].Callback==cbk){
            sCallBacks[i].Callback=NULL;
            sCallBacks[i].param=NULL;
            sCallBacks[i].tuneridx=-1;
            return E_OK;
        }
    }
    return E_ERROR;
}

DWORD nglTunerGetState(INT tuneridx,NGLTunerState*state){
    int rc;
    aui_signal_status info;
    char buf[256];//aui_nim_signal_info_get will caused crash ,add  var to avoid crash
    memset(&info, 0, sizeof(aui_signal_status));
    memcpy(buf,&info,sizeof(aui_signal_status));
    aui_nim_is_lock(sTuners[tuneridx].hdl,&state->locked);
    LOGV("w==>state->locked=%d",state->locked);
	if(AUI_NIM_STATUS_LOCKED!=state->locked){
	    memset(state,0,sizeof(*state));
	return -1;
	}
    rc=aui_nim_signal_info_get(sTuners[tuneridx].hdl,&info);
    //LOGD_IF(rc,"rc=%d tuner %d hdl=%p,freq=%d",rc,tuneridx,sTuners[tuneridx].hdl,info.ul_freq);
    state->frequency=info.ul_freq;
    state->ber=info.ul_bit_error_rate;
    state->cnr=info.ul_signal_cn;
    state->mer=info.ul_mer;
    state->strength=info.ul_signal_strength;
    state->quality=info.ul_signal_quality;
    return rc;
}

//TUNER_Polarity 水平时为1，垂直时为2 off 时为0

DWORD nglTunerSetLNB(int tuneridx,int polarity){
	aui_hdl hdl = 0;
	aui_hdl lnbhdl =NULL;
	AUI_RTN_CODE ret = AUI_RTN_FAIL;
	
	if (aui_nim_handle_get_byid(0, &hdl)){
		LOGD("[GL_TUNER_SetLNBPower] error\n");
		return (1);
	}
	ret = aui_nim_lnb_power_set(hdl,polarity);
	LOGV("lnbpower=%d, ret=%d",polarity,(int)ret);

	return ret;	
}

DWORD nglTunerSet22K(int tuneridx,INT k22){
    int ret=aui_nim_set22k_onoff(sTuners[tuneridx].hdl,k22);
    LOGV("ret=%d",ret);
    return ret;
}

DWORD nglSendDiseqcCommand(int tuneridx,BYTE*Command,BYTE cmdlen,BYTE diseqc_version){
    int rc=E_ERROR;
    aui_hdl hdl=sTuners[tuneridx].hdl;
    if(diseqc_version==1)
        rc=aui_diseqc_oper(hdl,AUI_DISEQC_MODE_BYTES,Command,cmdlen,NULL,NULL);
    LOGV("hdl=%p aui_diseqc_oper=%d  cmdlen=%d",hdl,rc,cmdlen);
    return rc==0?E_OK:E_ERROR;
}

////////////////////////////////////////////////////////////////
#define BOARD_CFG_M3528 1
#define SUPPORT_RDA5815M_TUNER 1
#define INC_INDEX(index) { index++; if (index == AUI_NIM_HANDLER_MAX) return AUI_RTN_FAIL; }

AUI_RTN_CODE nim_init_cb(void *pv_param)
{
	struct aui_nim_config *nim, *nim_config = (struct aui_nim_config *)pv_param;
	int index = 0;
	if (!nim_config)
		return AUI_RTN_FAIL;

#if (defined BOARD_CFG_M3515) || (defined BOARD_CFG_M3715B) // 3715B need modify

	struct aui_tuner_dvbs_config *tuner;
	struct aui_demod_dvbs_config *demod;

#if (defined AUI_BOARD_VERSION_01V04)
	/* first NIM : DVB-S with internal demod and Sharp VZ7306 tuner */
	nim = nim_config + index;
	tuner = &nim->config.dvbs.tuner;
	demod = &nim->config.dvbs.demod;

	tuner->freq_low = 900;
	tuner->freq_high = 2200;
	tuner->i2c_type_id = I2C_TYPE_SCB0;
	tuner->i2c_base_addr = 0xc4;
	tuner->id = AUI_AV_2012;

	nim->id = AUI_NIM_ID_M3503_0;

	nim->nim_reset_gpio.position = AUI_GPIO_NONE;
	nim->nim_reset_gpio.gpio_val = AUI_GPIO_VALUE_LOW;

	nim->lnb_power_gpio.position = 1;
	nim->lnb_power_gpio.io = AUI_GPIO_O_DIR;
	nim->lnb_power_gpio.gpio_val = AUI_GPIO_VALUE_HIGH;

	demod->i2c_type_id =  I2C_TYPE_SCB0;
	demod->i2c_base_addr = 0xe6;
	//QPSK_Config: 0xfd (1111 1101)
	demod->QPSK_Config = QPSK_CONFIG_MODE_8BIT
			| QPSK_CONFIG_NEW_AGC1
			| QPSK_CONFIG_POLAR_REVERT
			| QPSK_CONFIG_I2C_THROUGH
			| QPSK_CONFIG_IQ_SWAP
			| QPSK_CONFIG_FREQ_OFFSET;

	/* second NIM : with external demod M3501 and Sharp VZ7306 tuner */
	INC_INDEX(index);
	nim = nim_config + index;
	tuner = &nim->config.dvbs.tuner;
	demod = &nim->config.dvbs.demod;

	tuner->freq_low = 900;
	tuner->freq_high = 2200;
	tuner->i2c_type_id = I2C_TYPE_GPIO0;
	tuner->i2c_base_addr = 0xc4;
	tuner->id = AUI_AV_2012;

	nim->id = AUI_NIM_ID_M3501_1;

	nim->nim_reset_gpio.position = 5;
	nim->nim_reset_gpio.io = AUI_GPIO_O_DIR;
	nim->nim_reset_gpio.gpio_val = AUI_GPIO_VALUE_LOW;

	nim->lnb_power_gpio.position = 1;
	nim->lnb_power_gpio.io = AUI_GPIO_O_DIR;
	nim->lnb_power_gpio.gpio_val = AUI_GPIO_VALUE_HIGH;

	demod->i2c_type_id =  I2C_TYPE_GPIO0;
	demod->i2c_base_addr = 0x66;
	//QPSK_Config: 0x7d (0111 1101)
	demod->QPSK_Config = QPSK_CONFIG_MODE_2BIT
			| QPSK_CONFIG_NEW_AGC1
			| QPSK_CONFIG_POLAR_REVERT
			| QPSK_CONFIG_I2C_THROUGH
			| QPSK_CONFIG_IQ_SWAP
			| QPSK_CONFIG_FREQ_OFFSET;

#else
	/* first NIM : DVB-S with internal demod and Sharp VZ7306 tuner */
	nim = nim_config + index;
	tuner = &nim->config.dvbs.tuner;
	demod = &nim->config.dvbs.demod;

	tuner->freq_low = 900;
	tuner->freq_high = 2200;
	tuner->i2c_type_id = I2C_TYPE_SCB0;
	tuner->i2c_base_addr = 0xc0;
	tuner->id = AUI_SHARP_VZ7306;

	nim->id = AUI_NIM_ID_M3503_0;

	nim->nim_reset_gpio.position = AUI_GPIO_NONE;
	nim->nim_reset_gpio.gpio_val = AUI_GPIO_VALUE_LOW;

	nim->lnb_power_gpio.position = 1;
	nim->lnb_power_gpio.io = AUI_GPIO_O_DIR;
	nim->lnb_power_gpio.gpio_val = AUI_GPIO_VALUE_HIGH;

	demod->i2c_type_id =  I2C_TYPE_SCB0;
	demod->i2c_base_addr = 0xe6;
	//QPSK_Config: 0xe9 (1110 1001)
	demod->QPSK_Config = QPSK_CONFIG_MODE_8BIT
			| QPSK_CONFIG_NEW_AGC1
			| QPSK_CONFIG_I2C_THROUGH
			| QPSK_CONFIG_FREQ_OFFSET;

	/* second NIM : with external demod M3501 and Sharp VZ7306 tuner */
	INC_INDEX(index);
	nim = nim_config + index;
	tuner = &nim->config.dvbs.tuner;
	demod = &nim->config.dvbs.demod;

	tuner->freq_low = 900;
	tuner->freq_high = 2200;
	tuner->i2c_type_id = I2C_TYPE_GPIO0;
	tuner->i2c_base_addr = 0xc0;
	tuner->id = AUI_SHARP_VZ7306;

	nim->id = AUI_NIM_ID_M3501_1;

	nim->nim_reset_gpio.position = 5;
	nim->nim_reset_gpio.io = AUI_GPIO_O_DIR;
	nim->nim_reset_gpio.gpio_val = AUI_GPIO_VALUE_LOW;

	nim->lnb_power_gpio.position = 1;
	nim->lnb_power_gpio.io = AUI_GPIO_O_DIR;
	nim->lnb_power_gpio.gpio_val = AUI_GPIO_VALUE_HIGH;

	demod->i2c_type_id =  I2C_TYPE_GPIO0;
	demod->i2c_base_addr = 0x66;
	//QPSK_Config: 0x69 (0110 1001)
	demod->QPSK_Config = QPSK_CONFIG_MODE_2BIT
			| QPSK_CONFIG_NEW_AGC1
			| QPSK_CONFIG_I2C_THROUGH
			| QPSK_CONFIG_FREQ_OFFSET;
#endif

#elif (defined BOARD_CFG_M3515B)

	struct aui_tuner_dvbs_config *dvbs_tuner;
	struct aui_demod_dvbs_config *dvbs_demod;

#if (defined AUI_BOARD_VERSION_01V01)

	/* first NIM : internal demod and AV2012 tuner */
	nim = nim_config + index;
	dvbs_tuner = &nim->config.dvbs.tuner;
	dvbs_demod = &nim->config.dvbs.demod;

	dvbs_tuner->freq_low = 900;
	dvbs_tuner->freq_high = 2200;
	dvbs_tuner->i2c_type_id = I2C_TYPE_SCB0;
	dvbs_tuner->i2c_base_addr = 0xc2;
	dvbs_tuner->id = AUI_AV_2012;

	nim->id = AUI_NIM_ID_M3503_0;

	nim->nim_reset_gpio.position = AUI_GPIO_NONE;
	nim->nim_reset_gpio.io = AUI_GPIO_O_DIR;
	nim->nim_reset_gpio.gpio_val = AUI_GPIO_VALUE_LOW;

	nim->lnb_power_gpio.position = 1; /* CUT_LNB signal */
	nim->lnb_power_gpio.io = AUI_GPIO_O_DIR;
	nim->lnb_power_gpio.gpio_val = AUI_GPIO_VALUE_HIGH;

	dvbs_demod->i2c_type_id =  I2C_TYPE_SCB0;
	dvbs_demod->i2c_base_addr = 0xe6;

	//QPSK_Config: 0xfd (1111 1101)
	dvbs_demod->QPSK_Config = QPSK_CONFIG_MODE_8BIT
			| QPSK_CONFIG_NEW_AGC1
			| QPSK_CONFIG_POLAR_REVERT
			| QPSK_CONFIG_I2C_THROUGH
			| QPSK_CONFIG_IQ_SWAP
			| QPSK_CONFIG_FREQ_OFFSET;

	/* Second NIM : DVB-S with external demod M3501 and AV2012 tuner */
	INC_INDEX(index);
	nim = nim_config + index;
	dvbs_tuner = &nim->config.dvbs.tuner;
	dvbs_demod = &nim->config.dvbs.demod;

	dvbs_tuner->freq_low = 900;
	dvbs_tuner->freq_high = 2200;
	dvbs_tuner->i2c_type_id = I2C_TYPE_GPIO0;
	dvbs_tuner->i2c_base_addr = 0xc2;
	dvbs_tuner->id = AUI_AV_2012;

	nim->id = AUI_NIM_ID_M3501_1;

	nim->nim_reset_gpio.position = 5; /* DEMO_RST signal */
	nim->nim_reset_gpio.io = AUI_GPIO_O_DIR;
	nim->nim_reset_gpio.gpio_val = AUI_GPIO_VALUE_LOW;

	nim->lnb_power_gpio.position = 1; /* CUT_LNB signal */
	nim->lnb_power_gpio.io = AUI_GPIO_O_DIR;
	nim->lnb_power_gpio.gpio_val = AUI_GPIO_VALUE_HIGH;

	dvbs_demod->i2c_type_id =  I2C_TYPE_GPIO0;
	dvbs_demod->i2c_base_addr = 0x66;
	//QPSK_Config: 0x7d (0111 1101)
	dvbs_demod->QPSK_Config = QPSK_CONFIG_MODE_2BIT
			| QPSK_CONFIG_NEW_AGC1
			| QPSK_CONFIG_POLAR_REVERT
			| QPSK_CONFIG_I2C_THROUGH
			| QPSK_CONFIG_IQ_SWAP
			| QPSK_CONFIG_FREQ_OFFSET;

#elif (defined AUI_BOARD_VERSION_01V02)
	/* first NIM : internal demod and AV2012 tuner */
	nim = nim_config + index;
	dvbs_tuner = &nim->config.dvbs.tuner;
	dvbs_demod = &nim->config.dvbs.demod;

	dvbs_tuner->freq_low = 900;
	dvbs_tuner->freq_high = 2200;
	dvbs_tuner->i2c_type_id = I2C_TYPE_SCB0;
	dvbs_tuner->i2c_base_addr = 0xc4;
	dvbs_tuner->id = AUI_AV_2012;

	nim->id = AUI_NIM_ID_M3503_0;

	nim->nim_reset_gpio.position = AUI_GPIO_NONE;
	nim->nim_reset_gpio.io = AUI_GPIO_O_DIR;
	nim->nim_reset_gpio.gpio_val = AUI_GPIO_VALUE_LOW;

	nim->lnb_power_gpio.position = 1; /* CUT_LNB signal */
	nim->lnb_power_gpio.io = AUI_GPIO_O_DIR;
	nim->lnb_power_gpio.gpio_val = AUI_GPIO_VALUE_HIGH;

	dvbs_demod->i2c_type_id =  I2C_TYPE_SCB0;
	dvbs_demod->i2c_base_addr = 0xe6;

	//QPSK_Config: 0xfd (1111 1101)
	dvbs_demod->QPSK_Config = QPSK_CONFIG_MODE_8BIT
			| QPSK_CONFIG_NEW_AGC1
			| QPSK_CONFIG_POLAR_REVERT
			| QPSK_CONFIG_I2C_THROUGH
			| QPSK_CONFIG_IQ_SWAP
			| QPSK_CONFIG_FREQ_OFFSET;

	/* Second NIM : DVB-S with external demod M3501 and AV2012 tuner */
	INC_INDEX(index);
	nim = nim_config + index;
	dvbs_tuner = &nim->config.dvbs.tuner;
	dvbs_demod = &nim->config.dvbs.demod;

	dvbs_tuner->freq_low = 900;
	dvbs_tuner->freq_high = 2200;
	dvbs_tuner->i2c_type_id = I2C_TYPE_GPIO0;
	dvbs_tuner->i2c_base_addr = 0xc4;
	dvbs_tuner->id = AUI_AV_2012;

	nim->id = AUI_NIM_ID_M3501_1;

	nim->nim_reset_gpio.position = 5; /* DEMO_RST signal */
	nim->nim_reset_gpio.io = AUI_GPIO_O_DIR;
	nim->nim_reset_gpio.gpio_val = AUI_GPIO_VALUE_LOW;

	nim->lnb_power_gpio.position = 1; /* CUT_LNB signal */
	nim->lnb_power_gpio.io = AUI_GPIO_O_DIR;
	nim->lnb_power_gpio.gpio_val = AUI_GPIO_VALUE_HIGH;

	dvbs_demod->i2c_type_id =  I2C_TYPE_GPIO0;
	dvbs_demod->i2c_base_addr = 0x66;
	//QPSK_Config: 0x7d (0111 1101)
	dvbs_demod->QPSK_Config = QPSK_CONFIG_MODE_2BIT
			| QPSK_CONFIG_NEW_AGC1
			| QPSK_CONFIG_POLAR_REVERT
			| QPSK_CONFIG_I2C_THROUGH
			| QPSK_CONFIG_IQ_SWAP
			| QPSK_CONFIG_FREQ_OFFSET;
#else
//#error "Selected board is not this version by AUI"
#endif

#elif (defined BOARD_CFG_M3733)

	/* first NIM : DVB-C with internal demod and MxL603 tuner */
	nim = nim_config + 0;
	struct aui_tuner_dvbc_config *dvbc_tuner = &nim->config.dvbc.tuner;
	struct aui_demod_dvbc_config *dvbc_demod = &nim->config.dvbc.demod;

	dvbc_tuner->id = AUI_MXL603;
	dvbc_tuner->agc_ref = 0x80;
	dvbc_tuner->rf_agc_min = 0x2A;
	dvbc_tuner->rf_agc_max = 0xBA;
	dvbc_tuner->if_agc_max = 0xFE;
	dvbc_tuner->if_agc_min = 0x01;
	dvbc_tuner->tuner_crystal = 16;
	dvbc_tuner->i2c_base_addr = 0xc2;
	dvbc_tuner->tuner_special_config = 0x01;
	dvbc_tuner->wtuner_if_freq = 5000;
	dvbc_tuner->tuner_ref_divratio = 64;
	dvbc_tuner->tuner_agc_top = 1;
	dvbc_tuner->tuner_step_freq = 62.5;
	dvbc_tuner->i2c_type_id = I2C_TYPE_SCB0; //I2C_TYPE_SCB1 for old 3733
	dvbc_tuner->tuner_if_freq_J83A = 5000;
	dvbc_tuner->tuner_if_freq_J83B = 5380;
	dvbc_tuner->tuner_if_freq_J83C = 5000;

	nim->id = AUI_NIM_ID_M3281_0;

	nim->nim_reset_gpio.position = AUI_GPIO_NONE;
	nim->nim_reset_gpio.gpio_val = AUI_GPIO_VALUE_LOW;
	nim->lnb_power_gpio.position = AUI_GPIO_NONE;
	nim->lnb_power_gpio.gpio_val = AUI_GPIO_VALUE_LOW;

	dvbc_demod->i2c_type_id =  I2C_TYPE_SCB0;
	dvbc_demod->i2c_base_addr = 0x66;
	dvbc_demod->qam_mode = AUI_NIM_DVBC_MODE_J83AC | AUI_NIM_DEMO_SAMPLE_CLK_27M;

	/* Second NIM : DVB-S with external demod M3501 and AV2012 tuner */
	INC_INDEX(index);
	nim = nim_config + index;
	struct aui_tuner_dvbs_config *dvbs_tuner = &nim->config.dvbs.tuner;
	struct aui_demod_dvbs_config *dvbs_demod = &nim->config.dvbs.demod;

	dvbs_tuner->freq_low = 900;
	dvbs_tuner->freq_high = 2200;
	dvbs_tuner->i2c_type_id = I2C_TYPE_SCB1;
	dvbs_tuner->i2c_base_addr = 0xc2;
	dvbs_tuner->id = AUI_AV_2012;

	nim->id = AUI_NIM_ID_M3501_0;

	nim->nim_reset_gpio.position = 77;
	nim->nim_reset_gpio.io = AUI_GPIO_O_DIR;
	nim->nim_reset_gpio.gpio_val = AUI_GPIO_VALUE_LOW;

	nim->lnb_power_gpio.position = 76;
	nim->lnb_power_gpio.io = AUI_GPIO_O_DIR;
	nim->lnb_power_gpio.gpio_val = AUI_GPIO_VALUE_HIGH;

	dvbs_demod->i2c_type_id =  I2C_TYPE_SCB1;
	dvbs_demod->i2c_base_addr = 0x66;
	//QPSK_Config: 0x79 (0111 1001)
	dvbs_demod->QPSK_Config = QPSK_CONFIG_MODE_2BIT
			| QPSK_CONFIG_NEW_AGC1
			| QPSK_CONFIG_POLAR_REVERT
			| QPSK_CONFIG_I2C_THROUGH
			| QPSK_CONFIG_FREQ_OFFSET;

#define DVB_S_NIM_M3501_AV2012
//#define DVB_C_NIM_TDA10028_TDA18250

#if defined (DVB_S_NIM_M3501_AV2012)
	/* Full NIM 1 : DVB-S with external demod M3501 and AV2012 tuner */
	INC_INDEX(index);
	nim = nim_config + index;
	dvbs_tuner = &nim->config.dvbs.tuner;
	dvbs_demod = &nim->config.dvbs.demod;

	dvbs_tuner->freq_low = 900;
	dvbs_tuner->freq_high = 2200;
	dvbs_tuner->i2c_type_id = I2C_TYPE_SCB1;
	dvbs_tuner->i2c_base_addr = 0xc0;
	dvbs_tuner->id = AUI_AV_2012;

	nim->id = AUI_NIM_ID_M3501_1;

	nim->nim_reset_gpio.position = 77;
	nim->nim_reset_gpio.io = AUI_GPIO_O_DIR;
	nim->nim_reset_gpio.gpio_val = AUI_GPIO_VALUE_LOW;

	nim->lnb_power_gpio.position = 76;
	nim->lnb_power_gpio.io = AUI_GPIO_O_DIR;
	nim->lnb_power_gpio.gpio_val = AUI_GPIO_VALUE_HIGH;

	dvbs_demod->i2c_type_id =  I2C_TYPE_SCB1;
	dvbs_demod->i2c_base_addr = 0x06;

	//QPSK_Config: 0x39 (0011 1001)
	dvbs_demod->QPSK_Config = QPSK_CONFIG_MODE_1BIT
			| QPSK_CONFIG_NEW_AGC1
			| QPSK_CONFIG_POLAR_REVERT
			| QPSK_CONFIG_I2C_THROUGH
			| QPSK_CONFIG_FREQ_OFFSET;

	/* Full NIM 2 : DVB-S with external demod M3501 and AV2012 tuner */
	INC_INDEX(index);
	nim = nim_config + index;
	dvbs_tuner = &nim->config.dvbs.tuner;
	dvbs_demod = &nim->config.dvbs.demod;

	dvbs_tuner->freq_low = 900;
	dvbs_tuner->freq_high = 2200;
	dvbs_tuner->i2c_type_id = I2C_TYPE_SCB1;
	dvbs_tuner->i2c_base_addr = 0xc0;
	dvbs_tuner->id = AUI_AV_2012;

	nim->id = AUI_NIM_ID_M3501_2;

	nim->nim_reset_gpio.position = 77;
	nim->nim_reset_gpio.io = AUI_GPIO_O_DIR;
	nim->nim_reset_gpio.gpio_val = AUI_GPIO_VALUE_LOW;

	nim->lnb_power_gpio.position = AUI_GPIO_NONE;
	nim->lnb_power_gpio.gpio_val = AUI_GPIO_VALUE_LOW;

	dvbs_demod->i2c_type_id =  I2C_TYPE_SCB1;
	dvbs_demod->i2c_base_addr = 0x46;
	//QPSK_Config: 0x39 (0011 1001)
	dvbs_demod->QPSK_Config = QPSK_CONFIG_MODE_1BIT
			| QPSK_CONFIG_NEW_AGC1
			| QPSK_CONFIG_POLAR_REVERT
			| QPSK_CONFIG_I2C_THROUGH
			| QPSK_CONFIG_FREQ_OFFSET;

	/* Full NIM 3 : DVB-S with external demod M3501 and AV2012 tuner */
	INC_INDEX(index);
	nim = nim_config + index;
	dvbs_tuner = &nim->config.dvbs.tuner;
	dvbs_demod = &nim->config.dvbs.demod;

	dvbs_tuner->freq_low = 900;
	dvbs_tuner->freq_high = 2200;
	dvbs_tuner->i2c_type_id = I2C_TYPE_SCB1;
	dvbs_tuner->i2c_base_addr = 0xc0;
	dvbs_tuner->id = AUI_AV_2012;

	nim->id = AUI_NIM_ID_M3501_3;

	nim->nim_reset_gpio.position = 77;
	nim->nim_reset_gpio.io = AUI_GPIO_O_DIR;
	nim->nim_reset_gpio.gpio_val = AUI_GPIO_VALUE_LOW;

	nim->lnb_power_gpio.position = AUI_GPIO_NONE;
	nim->lnb_power_gpio.gpio_val = AUI_GPIO_VALUE_LOW;

	dvbs_demod->i2c_type_id =  I2C_TYPE_SCB1;
	dvbs_demod->i2c_base_addr = 0x26;
	//QPSK_Config: 0x39 (0011 1001)
	dvbs_demod->QPSK_Config = QPSK_CONFIG_MODE_1BIT
			| QPSK_CONFIG_NEW_AGC1
			| QPSK_CONFIG_POLAR_REVERT
			| QPSK_CONFIG_I2C_THROUGH
			| QPSK_CONFIG_FREQ_OFFSET;

#elif defined (DVB_C_NIM_TDA10028_TDA18250)
	/*                    DVB-C NIM TDA18250 + TDA10028                   */
	/***********************            TU1             *******************/
	INC_INDEX(index);
	nim = nim_config + index;
	dvbc_tuner = &nim->config.dvbc.tuner;
	dvbc_demod = &nim->config.dvbc.demod;

	dvbc_tuner->id = AUI_TDA18250;
	dvbc_tuner->agc_ref = 0x80;
	dvbc_tuner->rf_agc_min = 0x2A;
	dvbc_tuner->rf_agc_max = 0xBB;
	dvbc_tuner->if_agc_max = 0xFF;
	dvbc_tuner->if_agc_min = 0xB6;
	dvbc_tuner->tuner_crystal = 16;
	dvbc_tuner->i2c_base_addr = 0xc0;
	dvbc_tuner->tuner_special_config = 0x01;
	dvbc_tuner->wtuner_if_freq = 5000;
	dvbc_tuner->tuner_ref_divratio = 64;
	dvbc_tuner->tuner_agc_top = 1;
	dvbc_tuner->tuner_step_freq = 63;
	dvbc_tuner->i2c_type_id = I2C_TYPE_SCB1; //I2C_TYPE_SCB1 for old 3733
	dvbc_tuner->tuner_if_freq_J83A = 5000;
	dvbc_tuner->tuner_if_freq_J83B = 5380;
	dvbc_tuner->tuner_if_freq_J83C = 5000;

	nim->id = AUI_NIM_ID_10025_0;

	nim->nim_reset_gpio.position = 77;
	nim->nim_reset_gpio.io = AUI_GPIO_O_DIR;
	nim->nim_reset_gpio.gpio_val = AUI_GPIO_VALUE_LOW;
	nim->lnb_power_gpio.position = AUI_GPIO_NONE;
	nim->lnb_power_gpio.gpio_val = AUI_GPIO_VALUE_LOW;

	dvbc_demod->i2c_type_id =  I2C_TYPE_SCB1;
	dvbc_demod->i2c_base_addr = 0x18;
	dvbc_demod->qam_mode = AUI_NIM_DVBC_MODE_J83AC | AUI_NIM_DEMO_SAMPLE_CLK_27M;

	/*                    DVB-C NIM TDA18250 + TDA10028                   */
	/***********************            TU2             *******************/
	INC_INDEX(index);
	nim = nim_config + index;
	dvbc_tuner = &nim->config.dvbc.tuner;
	dvbc_demod = &nim->config.dvbc.demod;

	dvbc_tuner->id = AUI_TDA18250;
	dvbc_tuner->agc_ref = 0x80;
	dvbc_tuner->rf_agc_min = 0x2A;
	dvbc_tuner->rf_agc_max = 0xBB;
	dvbc_tuner->if_agc_max = 0xFF;
	dvbc_tuner->if_agc_min = 0xB6;
	dvbc_tuner->tuner_crystal = 16;
	dvbc_tuner->i2c_base_addr = 0xc0;
	dvbc_tuner->tuner_special_config = 0x01;
	dvbc_tuner->wtuner_if_freq = 5000;
	dvbc_tuner->tuner_ref_divratio = 64;
	dvbc_tuner->tuner_agc_top = 1;
	dvbc_tuner->tuner_step_freq = 63;
	dvbc_tuner->i2c_type_id = I2C_TYPE_SCB1; //I2C_TYPE_SCB1 for old 3733
	dvbc_tuner->tuner_if_freq_J83A = 5000;
	dvbc_tuner->tuner_if_freq_J83B = 5380;
	dvbc_tuner->tuner_if_freq_J83C = 5000;

	nim->id = AUI_NIM_ID_10025_0;

	nim->nim_reset_gpio.position = 77;
	nim->nim_reset_gpio.io = AUI_GPIO_O_DIR;
	nim->nim_reset_gpio.gpio_val = AUI_GPIO_VALUE_LOW;
	nim->lnb_power_gpio.position = AUI_GPIO_NONE;
	nim->lnb_power_gpio.gpio_val = AUI_GPIO_VALUE_LOW;

	dvbc_demod->i2c_type_id =  I2C_TYPE_SCB1;
	dvbc_demod->i2c_base_addr = 0x18;
	dvbc_demod->qam_mode = AUI_NIM_DVBC_MODE_J83AC | AUI_NIM_DEMO_SAMPLE_CLK_27M;

	/*                    DVB-C NIM TDA18250 + TDA10028                   */
	/***********************            TU3             *******************/
	INC_INDEX(index);
	nim = nim_config + index;
	dvbc_tuner = &nim->config.dvbc.tuner;
	dvbc_demod = &nim->config.dvbc.demod;

	dvbc_tuner->id = AUI_TDA18250;
	dvbc_tuner->agc_ref = 0x80;
	dvbc_tuner->rf_agc_min = 0x2A;
	dvbc_tuner->rf_agc_max = 0xBB;
	dvbc_tuner->if_agc_max = 0xFF;
	dvbc_tuner->if_agc_min = 0xB6;
	dvbc_tuner->tuner_crystal = 16;
	dvbc_tuner->i2c_base_addr = 0xc0;
	dvbc_tuner->tuner_special_config = 0x01;
	dvbc_tuner->wtuner_if_freq = 5000;
	dvbc_tuner->tuner_ref_divratio = 64;
	dvbc_tuner->tuner_agc_top = 1;
	dvbc_tuner->tuner_step_freq = 63;
	dvbc_tuner->i2c_type_id = I2C_TYPE_SCB1; //I2C_TYPE_SCB1 for old 3733
	dvbc_tuner->tuner_if_freq_J83A = 5000;
	dvbc_tuner->tuner_if_freq_J83B = 5380;
	dvbc_tuner->tuner_if_freq_J83C = 5000;

	nim->id = AUI_NIM_ID_10025_0;

	nim->nim_reset_gpio.position = 77;
	nim->nim_reset_gpio.io = AUI_GPIO_O_DIR;
	nim->nim_reset_gpio.gpio_val = AUI_GPIO_VALUE_LOW;
	nim->lnb_power_gpio.position = AUI_GPIO_NONE;
	nim->lnb_power_gpio.gpio_val = AUI_GPIO_VALUE_LOW;

	dvbc_demod->i2c_type_id =  I2C_TYPE_SCB1;
	dvbc_demod->i2c_base_addr = 0x18;
	dvbc_demod->qam_mode = AUI_NIM_DVBC_MODE_J83AC | AUI_NIM_DEMO_SAMPLE_CLK_27M;
#endif

#elif (defined BOARD_CFG_M3755)

    // Untested: Linux AUI NIM settints
    /* first NIM : DVB-C with internal demod and MxL603 tuner */
    nim = nim_config + 0;
    struct aui_tuner_dvbc_config *dvbc_tuner = &nim->config.dvbc.tuner;
    //struct aui_demod_dvbc_config *dvbc_demod = &nim->config.dvbc.demod;

    dvbc_tuner->id = AUI_MXL603;
    dvbc_tuner->rf_agc_max = 0xBA;
    dvbc_tuner->rf_agc_min = 0x2A;
    dvbc_tuner->if_agc_max = 0xFE;
    dvbc_tuner->if_agc_min = 0x01;
    dvbc_tuner->agc_ref = 0x80;

    dvbc_tuner->tuner_crystal = 16;
    dvbc_tuner->i2c_base_addr = 0xc0;
    dvbc_tuner->i2c_type_id = I2C_TYPE_SCB1;
    dvbc_tuner->tuner_special_config = 0x01;
    dvbc_tuner->wtuner_if_freq = 5000;
    dvbc_tuner->tuner_ref_divratio = 64;
    dvbc_tuner->tuner_agc_top = 1;
    dvbc_tuner->tuner_step_freq = 62.5;
    dvbc_tuner->tuner_if_freq_J83A = 5000;
    dvbc_tuner->tuner_if_freq_J83B = 5380;
    dvbc_tuner->tuner_if_freq_J83C = 5380;

    nim->id = AUI_NIM_ID_M3281_0;

    nim->nim_reset_gpio.position = AUI_GPIO_NONE;
    nim->nim_reset_gpio.gpio_val = AUI_GPIO_VALUE_LOW;
    nim->lnb_power_gpio.position = AUI_GPIO_NONE;
    nim->lnb_power_gpio.gpio_val = AUI_GPIO_VALUE_LOW;

    //dvbc_demod->i2c_type_id =  I2C_TYPE_SCB0;
    //dvbc_demod->i2c_base_addr = 0x66;
    //dvbc_demod->qam_mode = AUI_NIM_DVBC_MODE_J83AC | AUI_NIM_DEMO_SAMPLE_CLK_27M;

    /* second NIM : DVB-C with internal demod and MxL603 tuner */
    INC_INDEX(index);
    nim = nim_config + index;
    dvbc_tuner = &nim->config.dvbc.tuner;
    //dvbc_demod = &nim->config.dvbc.demod;

    dvbc_tuner->id = AUI_MXL603;
    dvbc_tuner->rf_agc_max = 0xBA;
    dvbc_tuner->rf_agc_min = 0x2A;
    dvbc_tuner->if_agc_max = 0xFE;
    dvbc_tuner->if_agc_min = 0x01;
    dvbc_tuner->agc_ref = 0x80;

    dvbc_tuner->tuner_crystal = 16;
    dvbc_tuner->i2c_base_addr = 0xc0;
    dvbc_tuner->i2c_type_id = I2C_TYPE_SCB2;
    dvbc_tuner->tuner_special_config = 0x01;
    dvbc_tuner->wtuner_if_freq = 5000;
    dvbc_tuner->tuner_ref_divratio = 64;
    dvbc_tuner->tuner_agc_top = 1;
    dvbc_tuner->tuner_step_freq = 62.5;
    dvbc_tuner->tuner_if_freq_J83A = 5000;
    dvbc_tuner->tuner_if_freq_J83B = 5380;
    dvbc_tuner->tuner_if_freq_J83C = 5380;

    nim->id = AUI_NIM_ID_M3281_1;

    nim->nim_reset_gpio.position = AUI_GPIO_NONE;
    nim->nim_reset_gpio.gpio_val = AUI_GPIO_VALUE_LOW;
    nim->lnb_power_gpio.position = AUI_GPIO_NONE;
    nim->lnb_power_gpio.gpio_val = AUI_GPIO_VALUE_LOW;

    //dvbc_demod->i2c_type_id =  I2C_TYPE_SCB0;
    //dvbc_demod->i2c_base_addr = 0x66;
    //dvbc_demod->qam_mode = AUI_NIM_DVBC_MODE_J83AC | AUI_NIM_DEMO_SAMPLE_CLK_27M;

#ifdef CONFIG_ALI_EMULATOR        
//========================================================================//
	INC_INDEX(index);
	nim = nim_config + index;
	struct aui_tuner_dvbs_config *dvbs_tuner;
	struct aui_demod_dvbs_config *dvbs_demod;
	dvbs_tuner = &nim->config.dvbs.tuner;
	dvbs_demod = &nim->config.dvbs.demod;

	dvbs_tuner->freq_low = 900;
	dvbs_tuner->freq_high = 2200;
	dvbs_tuner->i2c_type_id = I2C_TYPE_SCB3;
	dvbs_tuner->i2c_base_addr = 0x40;
	dvbs_tuner->id = AUI_M3031;

	nim->id = AUI_NIM_ID_M3501_0;

	nim->nim_reset_gpio.position = 92;
	nim->nim_reset_gpio.io = AUI_GPIO_O_DIR;
	nim->nim_reset_gpio.gpio_val = AUI_GPIO_VALUE_LOW;

	nim->lnb_power_gpio.position = 6;
	nim->lnb_power_gpio.io = AUI_GPIO_O_DIR;
	nim->lnb_power_gpio.gpio_val = AUI_GPIO_VALUE_HIGH;

	dvbs_demod->i2c_type_id =  I2C_TYPE_SCB3;
	dvbs_demod->i2c_base_addr = 0x66;
	//QPSK_Config: 0x79 (0111 1001)
	dvbs_demod->QPSK_Config = QPSK_CONFIG_MODE_2BIT
			| QPSK_CONFIG_NEW_AGC1
			| QPSK_CONFIG_POLAR_REVERT
			| QPSK_CONFIG_I2C_THROUGH
			| QPSK_CONFIG_FREQ_OFFSET;
#endif    
#elif (defined BOARD_CFG_M3823)

#if (defined AUI_BOARD_VERSION_04V01)
	/* first NIM : ISDBT with internal demod and MxL603 tuner */
	nim = nim_config + 0;
	struct aui_tuner_dvbt_config *dvbt_tuner = &nim->config.dvbt.tuner;
	//struct aui_demod_dvbt_config *dvbt_demod = &nim->config.dvbt.demod;

	dvbt_tuner->freq_low = 40; // KHz
	dvbt_tuner->freq_high = 900; // KHz
	dvbt_tuner->id = AUI_MXL603;
	dvbt_tuner->agc_ref = 0x63;
	//dvbt_tuner->rf_agc_min = 0x2A; // ?
	//dvbt_tuner->rf_agc_max = 0xBA; // ?
	dvbt_tuner->if_agc_max = 0xC3;
	dvbt_tuner->if_agc_min = 0x00;
	dvbt_tuner->tuner_crystal = 16;
	//dvbt_tuner->tuner_special_config = 0x01; // ?
	dvbt_tuner->wtuner_if_freq = 5000;
	//dvbt_tuner->tuner_ref_divratio = 64; // ?
	dvbt_tuner->tuner_agc_top = 252;
	//dvbt_tuner->tuner_step_freq = 62.5; // ?
	dvbt_tuner->i2c_type_id = I2C_TYPE_SCB2;
	dvbt_tuner->i2c_base_addr = 0xC0;
	dvbt_tuner->chip = tuner_chip_maxlinear;

	nim->id = AUI_NIM_ID_S3821_0;

	nim->nim_reset_gpio.position = AUI_GPIO_NONE;
	nim->nim_reset_gpio.gpio_val = AUI_GPIO_VALUE_LOW;
	nim->lnb_power_gpio.position = AUI_GPIO_NONE;
	nim->lnb_power_gpio.gpio_val = AUI_GPIO_VALUE_LOW;

	/* Second NIM : DVB-S with external demod M3501 and M3031 tuner */
	INC_INDEX(index);
	nim = nim_config + index;
	struct aui_tuner_dvbs_config *dvbs_tuner = &nim->config.dvbs.tuner;
	struct aui_demod_dvbs_config *dvbs_demod = &nim->config.dvbs.demod;

	dvbs_tuner->freq_low = 900;
	dvbs_tuner->freq_high = 2200;
	dvbs_tuner->i2c_type_id = I2C_TYPE_GPIO1;
	//dvbs_tuner->i2c_base_addr = 0xc2;
	dvbs_tuner->i2c_base_addr = 0x40;
	dvbs_tuner->id = AUI_M3031;

	nim->id = AUI_NIM_ID_M3501_1;

	nim->nim_reset_gpio.position = 6;
	nim->nim_reset_gpio.io = AUI_GPIO_O_DIR;
	nim->nim_reset_gpio.gpio_val = AUI_GPIO_VALUE_LOW;

	nim->lnb_power_gpio.position = 104;
	nim->lnb_power_gpio.io = AUI_GPIO_O_DIR;
	nim->lnb_power_gpio.gpio_val = AUI_GPIO_VALUE_HIGH;

	dvbs_demod->i2c_type_id =  I2C_TYPE_GPIO1;
	dvbs_demod->i2c_base_addr = 0x66;
	//QPSK_Config: 0x79 (0111 1001)
	dvbs_demod->QPSK_Config = QPSK_CONFIG_MODE_2BIT
			| QPSK_CONFIG_NEW_AGC1
			| QPSK_CONFIG_POLAR_REVERT
			| QPSK_CONFIG_I2C_THROUGH
			| QPSK_CONFIG_FREQ_OFFSET;
#else
	/* first NIM : ISDBT with internal demod and MxL603 tuner */
	nim = nim_config + 0;
	struct aui_tuner_dvbt_config *dvbt_tuner = &nim->config.dvbt.tuner;
	//struct aui_demod_dvbt_config *dvbt_demod = &nim->config.dvbt.demod;

	dvbt_tuner->freq_low = 40; // KHz
	dvbt_tuner->freq_high = 900; // KHz
	dvbt_tuner->id = AUI_MXL603;
	dvbt_tuner->agc_ref = 0x63;
	//dvbt_tuner->rf_agc_min = 0x2A; // ?
	//dvbt_tuner->rf_agc_max = 0xBA; // ?
	dvbt_tuner->if_agc_max = 0xC3;
	dvbt_tuner->if_agc_min = 0x00;
	dvbt_tuner->tuner_crystal = 16;
	//dvbt_tuner->tuner_special_config = 0x01; // ?
	dvbt_tuner->wtuner_if_freq = 5000;
	//dvbt_tuner->tuner_ref_divratio = 64; // ?
	dvbt_tuner->tuner_agc_top = 252;
	//dvbt_tuner->tuner_step_freq = 62.5; // ?
	dvbt_tuner->i2c_type_id = I2C_TYPE_SCB2;
	dvbt_tuner->i2c_base_addr = 0xC0;
	dvbt_tuner->chip = tuner_chip_maxlinear;

	nim->id = AUI_NIM_ID_S3821_0;

	nim->nim_reset_gpio.position = AUI_GPIO_NONE;
	nim->nim_reset_gpio.gpio_val = AUI_GPIO_VALUE_LOW;
	nim->lnb_power_gpio.position = AUI_GPIO_NONE;
	nim->lnb_power_gpio.gpio_val = AUI_GPIO_VALUE_LOW;

	/* Second NIM : DVB-S with external demod M3501 and AV2012 tuner */
	INC_INDEX(index);
	nim = nim_config + index;
	struct aui_tuner_dvbs_config *dvbs_tuner = &nim->config.dvbs.tuner;
	struct aui_demod_dvbs_config *dvbs_demod = &nim->config.dvbs.demod;

	dvbs_tuner->freq_low = 900;
	dvbs_tuner->freq_high = 2200;
	dvbs_tuner->i2c_type_id = I2C_TYPE_GPIO1;
	dvbs_tuner->i2c_base_addr = 0xc2;
	dvbs_tuner->id = AUI_AV_2012;

	nim->id = AUI_NIM_ID_M3501_1;

	nim->nim_reset_gpio.position = 6;
	nim->nim_reset_gpio.io = AUI_GPIO_O_DIR;
	nim->nim_reset_gpio.gpio_val = AUI_GPIO_VALUE_LOW;

	nim->lnb_power_gpio.position = 104;
	nim->lnb_power_gpio.io = AUI_GPIO_O_DIR;
	nim->lnb_power_gpio.gpio_val = AUI_GPIO_VALUE_HIGH;

	dvbs_demod->i2c_type_id =  I2C_TYPE_GPIO1;
	dvbs_demod->i2c_base_addr = 0x66;
	//QPSK_Config: 0x6d (0110 1101)
	dvbs_demod->QPSK_Config = QPSK_CONFIG_MODE_2BIT
			| QPSK_CONFIG_NEW_AGC1
			| QPSK_CONFIG_I2C_THROUGH
			| QPSK_CONFIG_IQ_SWAP
			| QPSK_CONFIG_FREQ_OFFSET;
#endif

#elif (defined BOARD_CFG_M3735)
	/* first NIM : DVB-C with internal demod and MxL603 tuner */
	nim = nim_config + 0;
	struct aui_tuner_dvbc_config *dvbc_tuner = &nim->config.dvbc.tuner;
	struct aui_demod_dvbc_config *dvbc_demod = &nim->config.dvbc.demod;

	dvbc_tuner->id = AUI_MXL214C ;
	dvbc_tuner->agc_ref = 0x80;
	dvbc_tuner->rf_agc_min = 0x2A;
	dvbc_tuner->rf_agc_max = 0xBA;
	dvbc_tuner->if_agc_max = 0xFE;
	dvbc_tuner->if_agc_min = 0x01;
	dvbc_tuner->tuner_crystal = 16;
	dvbc_tuner->i2c_base_addr = 0xa0; //0xc2;
	dvbc_tuner->tuner_special_config = 0x01;
	dvbc_tuner->wtuner_if_freq = 5000;
	dvbc_tuner->tuner_ref_divratio = 64;
	dvbc_tuner->tuner_agc_top = 1;
	dvbc_tuner->tuner_step_freq = 62.5;
	dvbc_tuner->i2c_type_id = I2C_TYPE_SCB1;//I2C_TYPE_SCB0;
	dvbc_tuner->tuner_if_freq_J83A = 5000;
	dvbc_tuner->tuner_if_freq_J83B = 5380;
	dvbc_tuner->tuner_if_freq_J83C = 5380;

	nim->id = AUI_NIM_ID_MXL214C_0;

	nim->nim_reset_gpio.position = AUI_GPIO_NONE;
	nim->lnb_power_gpio.position = AUI_GPIO_NONE;

	dvbc_demod->i2c_type_id =  I2C_TYPE_SCB1; //I2C_TYPE_SCB0;
	dvbc_demod->i2c_base_addr = 0xa0;//0x66;
	dvbc_demod->qam_mode = AUI_NIM_DVBC_MODE_J83AC | AUI_NIM_DEMO_SAMPLE_CLK_27M;

	/* second NIM : DVB-C with internal demod and MxL214 tuner */
	INC_INDEX(index);
	nim = nim_config + index;
	dvbc_tuner = &nim->config.dvbc.tuner;
	dvbc_demod = &nim->config.dvbc.demod;

	dvbc_tuner->id = AUI_MXL214C ;
	dvbc_tuner->agc_ref = 0x80;
	dvbc_tuner->rf_agc_min = 0x2A;
	dvbc_tuner->rf_agc_max = 0xBA;
	dvbc_tuner->if_agc_max = 0xFE;
	dvbc_tuner->if_agc_min = 0x01;
	dvbc_tuner->tuner_crystal = 16;
	dvbc_tuner->i2c_base_addr = 0xa0;
	dvbc_tuner->tuner_special_config = 0x01;
	dvbc_tuner->wtuner_if_freq = 5000;
	dvbc_tuner->tuner_ref_divratio = 64;
	dvbc_tuner->tuner_agc_top = 1;
	dvbc_tuner->tuner_step_freq = 62.5;
	dvbc_tuner->i2c_type_id = I2C_TYPE_SCB1;
	dvbc_tuner->tuner_if_freq_J83A = 5000;
	dvbc_tuner->tuner_if_freq_J83B = 5380;
	dvbc_tuner->tuner_if_freq_J83C = 5380;

	nim->id = AUI_NIM_ID_MXL214C_1;

	nim->nim_reset_gpio.position = AUI_GPIO_NONE;
	nim->lnb_power_gpio.position = AUI_GPIO_NONE;

	dvbc_demod->i2c_type_id =  I2C_TYPE_SCB1;
	dvbc_demod->i2c_base_addr = 0xa0;
	dvbc_demod->qam_mode = AUI_NIM_DVBC_MODE_J83AC | AUI_NIM_DEMO_SAMPLE_CLK_27M;

	/* thirdth NIM : DVB-C with internal demod and MxL214 tuner */
	INC_INDEX(index);
	nim = nim_config + index;
	dvbc_tuner = &nim->config.dvbc.tuner;
	dvbc_demod = &nim->config.dvbc.demod;

	dvbc_tuner->id = AUI_MXL214C ;
	dvbc_tuner->agc_ref = 0x80;
	dvbc_tuner->rf_agc_min = 0x2A;
	dvbc_tuner->rf_agc_max = 0xBA;
	dvbc_tuner->if_agc_max = 0xFE;
	dvbc_tuner->if_agc_min = 0x01;
	dvbc_tuner->tuner_crystal = 16;
	dvbc_tuner->i2c_base_addr = 0xa0;
	dvbc_tuner->tuner_special_config = 0x01;
	dvbc_tuner->wtuner_if_freq = 5000;
	dvbc_tuner->tuner_ref_divratio = 64;
	dvbc_tuner->tuner_agc_top = 1;
	dvbc_tuner->tuner_step_freq = 62.5;
	dvbc_tuner->i2c_type_id = I2C_TYPE_SCB1;
	dvbc_tuner->tuner_if_freq_J83A = 5000;
	dvbc_tuner->tuner_if_freq_J83B = 5380;
	dvbc_tuner->tuner_if_freq_J83C = 5380;

	nim->id = AUI_NIM_ID_MXL214C_2;

	nim->nim_reset_gpio.position = AUI_GPIO_NONE;
	nim->lnb_power_gpio.position = AUI_GPIO_NONE;

	dvbc_demod->i2c_type_id =  I2C_TYPE_SCB1;
	dvbc_demod->i2c_base_addr = 0xa0;
	dvbc_demod->qam_mode = AUI_NIM_DVBC_MODE_J83AC | AUI_NIM_DEMO_SAMPLE_CLK_27M;

	/* fourth NIM : DVB-C with internal demod and MxL214 tuner */
	INC_INDEX(index);
	nim = nim_config + index;
	dvbc_tuner = &nim->config.dvbc.tuner;
	dvbc_demod = &nim->config.dvbc.demod;

	dvbc_tuner->id = AUI_MXL214C ;
	dvbc_tuner->agc_ref = 0x80;
	dvbc_tuner->rf_agc_min = 0x2A;
	dvbc_tuner->rf_agc_max = 0xBA;
	dvbc_tuner->if_agc_max = 0xFE;
	dvbc_tuner->if_agc_min = 0x01;
	dvbc_tuner->tuner_crystal = 16;
	dvbc_tuner->i2c_base_addr = 0xa0;
	dvbc_tuner->tuner_special_config = 0x01;
	dvbc_tuner->wtuner_if_freq = 5000;
	dvbc_tuner->tuner_ref_divratio = 64;
	dvbc_tuner->tuner_agc_top = 1;
	dvbc_tuner->tuner_step_freq = 62.5;
	dvbc_tuner->i2c_type_id = I2C_TYPE_SCB1;
	dvbc_tuner->tuner_if_freq_J83A = 5000;
	dvbc_tuner->tuner_if_freq_J83B = 5380;
	dvbc_tuner->tuner_if_freq_J83C = 5380;

	nim->id = AUI_NIM_ID_MXL214C_3;

	nim->nim_reset_gpio.position = AUI_GPIO_NONE;
	nim->lnb_power_gpio.position = AUI_GPIO_NONE;

	dvbc_demod->i2c_type_id =  I2C_TYPE_SCB1;
	dvbc_demod->i2c_base_addr = 0xa0;
	dvbc_demod->qam_mode = AUI_NIM_DVBC_MODE_J83AC | AUI_NIM_DEMO_SAMPLE_CLK_27M;

#elif (defined BOARD_CFG_M3527)

	struct aui_tuner_dvbs_config *tuner;
	struct aui_demod_dvbs_config *demod;
#if 0
	    /* first NIM : DVB-S with internal demod and ALi RDA5815M tuner */
	    nim = nim_config + index;
	    tuner = &nim->config.dvbs.tuner;
	    demod = &nim->config.dvbs.demod;

	    tuner->freq_low = 900;
	    tuner->freq_high = 2200;
	    tuner->i2c_type_id = I2C_TYPE_SCB1;
	    tuner->i2c_base_addr = 0x18;
	    tuner->id = AUI_RDA5815M;

	    nim->id = AUI_NIM_ID_C3505_0;

	    // Internal demodulator does not need reset GPIO
	    nim->nim_reset_gpio.position = AUI_GPIO_NONE;
	//  nim->nim_reset_gpio.io = AUI_GPIO_O_DIR;
	//  nim->nim_reset_gpio.gpio_val = AUI_GPIO_VALUE_LOW;

	    nim->lnb_power_gpio.position = 71;
	    nim->lnb_power_gpio.io = AUI_GPIO_O_DIR;
	    nim->lnb_power_gpio.gpio_val = AUI_GPIO_VALUE_HIGH;

	    //demod->i2c_type_id =  I2C_TYPE_SCB0;
	    //demod->i2c_base_addr = 0xe6;
	    //QPSK_Config: 0xfd (1111 1101)
	    demod->QPSK_Config = QPSK_CONFIG_MODE_8BIT
	            | QPSK_CONFIG_NEW_AGC1
	            | QPSK_CONFIG_POLAR_REVERT
	            | QPSK_CONFIG_I2C_THROUGH
	            | QPSK_CONFIG_IQ_SWAP
	            | QPSK_CONFIG_FREQ_OFFSET;
#endif

	/* first NIM : DVB-S with internal demod and ALi M3031 tuner */
	nim = nim_config + index;
	tuner = &nim->config.dvbs.tuner;
	demod = &nim->config.dvbs.demod;

	tuner->freq_low = 900;
	tuner->freq_high = 2200;
	tuner->i2c_type_id = I2C_TYPE_SCB1;
	tuner->i2c_base_addr = 0x40;
	tuner->id = AUI_M3031;

	nim->id = AUI_NIM_ID_C3505_0;

	// Internal demodulator does not need reset GPIO
	nim->nim_reset_gpio.position = AUI_GPIO_NONE;
//	nim->nim_reset_gpio.io = AUI_GPIO_O_DIR;
//	nim->nim_reset_gpio.gpio_val = AUI_GPIO_VALUE_LOW;

	nim->lnb_power_gpio.position = 71;
	nim->lnb_power_gpio.io = AUI_GPIO_O_DIR;
	nim->lnb_power_gpio.gpio_val = AUI_GPIO_VALUE_HIGH;

	//demod->i2c_type_id =  I2C_TYPE_SCB0;
	//demod->i2c_base_addr = 0xe6;
	//QPSK_Config: 0xf9 (1111 1001)
	demod->QPSK_Config = QPSK_CONFIG_MODE_8BIT
			| QPSK_CONFIG_NEW_AGC1
			| QPSK_CONFIG_POLAR_REVERT
			| QPSK_CONFIG_I2C_THROUGH
			| QPSK_CONFIG_FREQ_OFFSET;

	/* second NIM : with external demod M3501 and ALi M3031 tuner */
	INC_INDEX(index);
	nim = nim_config + index;
	tuner = &nim->config.dvbs.tuner;
	demod = &nim->config.dvbs.demod;

	tuner->freq_low = 900;
	tuner->freq_high = 2200;
	tuner->i2c_type_id = I2C_TYPE_SCB3;
	tuner->i2c_base_addr = 0x42;
	tuner->id = AUI_M3031;

	nim->id = AUI_NIM_ID_M3501_1;

	nim->nim_reset_gpio.position = 95;
	nim->nim_reset_gpio.io = AUI_GPIO_O_DIR;
	nim->nim_reset_gpio.gpio_val = AUI_GPIO_VALUE_LOW;

	nim->lnb_power_gpio.position = 71;
	nim->lnb_power_gpio.io = AUI_GPIO_O_DIR;
	nim->lnb_power_gpio.gpio_val = AUI_GPIO_VALUE_HIGH;

	demod->i2c_type_id =  I2C_TYPE_SCB3;
	demod->i2c_base_addr = 0x66;
	//QPSK_Config: 0x79 (0111 1001)
	demod->QPSK_Config = QPSK_CONFIG_MODE_2BIT
			| QPSK_CONFIG_NEW_AGC1
			| QPSK_CONFIG_POLAR_REVERT
			| QPSK_CONFIG_I2C_THROUGH
			| QPSK_CONFIG_FREQ_OFFSET;
	
#elif (defined BOARD_CFG_M3528)
        struct aui_tuner_dvbs_config *tuner;
        struct aui_demod_dvbs_config *demod;
#ifdef SUPPORT_RDA5815M_TUNER
        /* first NIM : DVB-S with internal demod and ALi RDA5815M tuner */
        nim = nim_config + index;
        tuner = &nim->config.dvbs.tuner;
        demod = &nim->config.dvbs.demod;

        tuner->freq_low = 900;
        tuner->freq_high = 2200;
        tuner->i2c_type_id = I2C_TYPE_SCB1;
        tuner->i2c_base_addr = 0x18;
        tuner->id = AUI_RDA5815M;

        nim->id = AUI_NIM_ID_C3505_0;
        // Internal demodulator does not need reset GPIO
        nim->nim_reset_gpio.position = AUI_GPIO_NONE;
        //      nim->nim_reset_gpio.io = AUI_GPIO_O_DIR;
        //      nim->nim_reset_gpio.gpio_val = AUI_GPIO_VALUE_LOW;

        nim->lnb_power_gpio.position = 92;
        nim->lnb_power_gpio.io = AUI_GPIO_O_DIR;
        nim->lnb_power_gpio.gpio_val = AUI_GPIO_VALUE_HIGH;

        //demod->i2c_type_id =  I2C_TYPE_SCB0;
        //demod->i2c_base_addr = 0xe6;
        //QPSK_Config: 0xfd (1111 1101)
        demod->QPSK_Config = QPSK_CONFIG_MODE_8BIT
                             | QPSK_CONFIG_NEW_AGC1
                             | QPSK_CONFIG_POLAR_REVERT
                             | QPSK_CONFIG_I2C_THROUGH
                             | QPSK_CONFIG_IQ_SWAP
                             | QPSK_CONFIG_FREQ_OFFSET;
#else

       /* first NIM : DVB-S with internal demod and ALi M3031 tuner */
       nim = nim_config + index;
       tuner = &nim->config.dvbs.tuner;
       demod = &nim->config.dvbs.demod;

       tuner->freq_low = 900;
       tuner->freq_high = 2200;
       tuner->i2c_type_id = I2C_TYPE_SCB1;

       #if 0//def SUPPORT_RDA5815M_TUNER
       tuner->i2c_base_addr = 0x18;
       tuner->id = AUI_RDA5815M;
       #else
       tuner->i2c_base_addr = 0x40;
       tuner->id = AUI_M3031;
       #endif

       nim->id = AUI_NIM_ID_C3505_0;
       // Internal demodulator does not need reset GPIO
       nim->nim_reset_gpio.position = AUI_GPIO_NONE;
       //      nim->nim_reset_gpio.io = AUI_GPIO_O_DIR;
       //      nim->nim_reset_gpio.gpio_val = AUI_GPIO_VALUE_LOW;
       nim->lnb_power_gpio.position = 92;
       nim->lnb_power_gpio.io = AUI_GPIO_O_DIR;
       nim->lnb_power_gpio.gpio_val = AUI_GPIO_VALUE_HIGH;

       demod->i2c_base_addr = 0xe6;
       //QPSK_Config: 0xf9 (1111 1001)
       demod->QPSK_Config = QPSK_CONFIG_MODE_8BIT
                       | QPSK_CONFIG_NEW_AGC1
                   #if 0//def SUPPORT_RDA5815M_TUNER
                       | QPSK_CONFIG_IQ_SWAP
                   #endif
                       | QPSK_CONFIG_POLAR_REVERT
                       | QPSK_CONFIG_I2C_THROUGH
                       | QPSK_CONFIG_FREQ_OFFSET;
#endif

       /* second NIM : with external demod M3501 and ALi M3031 tuner */
       INC_INDEX(index);
       nim = nim_config + index;
       tuner = &nim->config.dvbs.tuner;
       demod = &nim->config.dvbs.demod;

       tuner->freq_low = 900;
       tuner->freq_high = 2200;
       tuner->i2c_type_id = I2C_TYPE_SCB1;
       tuner->i2c_base_addr = 0x42;
       tuner->id = AUI_M3031;

       nim->id = AUI_NIM_ID_M3501_1;

       nim->nim_reset_gpio.position = 86;
       nim->nim_reset_gpio.io = AUI_GPIO_O_DIR;
       nim->nim_reset_gpio.gpio_val = AUI_GPIO_VALUE_LOW;

       nim->lnb_power_gpio.position = 71;
       nim->lnb_power_gpio.io = AUI_GPIO_O_DIR;
       nim->lnb_power_gpio.gpio_val = AUI_GPIO_VALUE_HIGH;

       demod->i2c_type_id =  I2C_TYPE_SCB1;
       demod->i2c_base_addr = 0xA6;
       //QPSK_Config: 0x79 (0111 1001)
       demod->QPSK_Config = QPSK_CONFIG_MODE_2BIT
                            | QPSK_CONFIG_NEW_AGC1
                            | QPSK_CONFIG_POLAR_REVERT
                            | QPSK_CONFIG_I2C_THROUGH
                            | QPSK_CONFIG_FREQ_OFFSET;
	

#elif (defined BOARD_CFG_M3529)

      struct aui_tuner_dvbs_config *tuner;
      struct aui_demod_dvbs_config *demod;

      /* first NIM : DVB-S with internal demod and ALi M3031 tuner */
      nim = nim_config + index;
      tuner = &nim->config.dvbs.tuner;
      demod = &nim->config.dvbs.demod;

      tuner->freq_low = 900;
      tuner->freq_high = 2200;
      tuner->i2c_type_id = I2C_TYPE_SCB1;
      tuner->i2c_base_addr = 0x40;
      tuner->id = AUI_M3031;

      nim->id = AUI_NIM_ID_C3505_0;

      // Internal demodulator does not need reset GPIO
      nim->nim_reset_gpio.position = AUI_GPIO_NONE;
      // nim->nim_reset_gpio.io = AUI_GPIO_O_DIR;
      // nim->nim_reset_gpio.gpio_val = AUI_GPIO_VALUE_LOW;

      nim->lnb_power_gpio.position = 6;
      nim->lnb_power_gpio.io = AUI_GPIO_O_DIR;
      nim->lnb_power_gpio.gpio_val = AUI_GPIO_VALUE_HIGH;

      //demod->i2c_type_id =  I2C_TYPE_SCB0;
      //demod->i2c_base_addr = 0xe6;
      //QPSK_Config: 0xf9 (1111 1001)
      demod->QPSK_Config = QPSK_CONFIG_MODE_8BIT
		| QPSK_CONFIG_NEW_AGC1
		| QPSK_CONFIG_POLAR_REVERT
		| QPSK_CONFIG_I2C_THROUGH
		| QPSK_CONFIG_FREQ_OFFSET;

     /* second NIM : with external demod M3501 and ALi M3031 tuner */
     INC_INDEX(index);
     nim = nim_config + index;
     tuner = &nim->config.dvbs.tuner;
     demod = &nim->config.dvbs.demod;

     tuner->freq_low = 900;
     tuner->freq_high = 2200;
     tuner->i2c_type_id = I2C_TYPE_SCB3;
     tuner->i2c_base_addr = 0x40;
     tuner->id = AUI_M3031;

     nim->id = AUI_NIM_ID_M3501_1;

     nim->nim_reset_gpio.position = 92;
     nim->nim_reset_gpio.io = AUI_GPIO_O_DIR;
     nim->nim_reset_gpio.gpio_val = AUI_GPIO_VALUE_LOW;

     nim->lnb_power_gpio.position = 6;
     nim->lnb_power_gpio.io = AUI_GPIO_O_DIR;
     nim->lnb_power_gpio.gpio_val = AUI_GPIO_VALUE_HIGH;

     demod->i2c_type_id =  I2C_TYPE_SCB3;
     demod->i2c_base_addr = 0x66;
     //QPSK_Config: 0x79 (0111 1001)
     demod->QPSK_Config = QPSK_CONFIG_MODE_2BIT
			| QPSK_CONFIG_NEW_AGC1
			| QPSK_CONFIG_POLAR_REVERT
			| QPSK_CONFIG_I2C_THROUGH
			| QPSK_CONFIG_FREQ_OFFSET;

#elif (defined BOARD_CFG_M3627)
#ifdef SUPPORT_TWO_TUNER
     struct aui_tuner_dvbs_config *tuner;
     struct aui_demod_dvbs_config *demod;

     /* first NIM : DVB-S with internal demod and ALi M3031 tuner */
     nim = nim_config + index;
     tuner = &nim->config.dvbs.tuner;
     demod = &nim->config.dvbs.demod;
     tuner->freq_low = 900;
     tuner->freq_high = 2200;
     tuner->i2c_type_id = I2C_TYPE_SCB1;
     tuner->i2c_base_addr = 0x40;
     tuner->id = AUI_M3031;

     nim->id = AUI_NIM_ID_C3505_0;

     // Internal demodulator does not need reset GPIO
     nim->nim_reset_gpio.position = AUI_GPIO_NONE;
     //nim->nim_reset_gpio.io = AUI_GPIO_O_DIR;
     //nim->nim_reset_gpio.gpio_val = AUI_GPIO_VALUE_LOW;

     nim->lnb_power_gpio.position = 71;
     nim->lnb_power_gpio.io = AUI_GPIO_O_DIR;
     nim->lnb_power_gpio.gpio_val = AUI_GPIO_VALUE_HIGH;

     //demod->i2c_type_id =  I2C_TYPE_SCB0;
     //demod->i2c_base_addr = 0xe6;
     //QPSK_Config: 0xf9 (1111 1001)
     demod->QPSK_Config = QPSK_CONFIG_MODE_8BIT
			| QPSK_CONFIG_NEW_AGC1
			| QPSK_CONFIG_POLAR_REVERT
			| QPSK_CONFIG_I2C_THROUGH
			| QPSK_CONFIG_FREQ_OFFSET;

     /* second NIM : DVB-T2 with external demod and CXD2872 tuner */
     INC_INDEX(index);
#endif	
     nim = nim_config + index;

     struct aui_tuner_dvbt_config *dvbt_tuner = &nim->config.dvbt.tuner;
     struct aui_demod_dvbt_config *dvbt_demod = &nim->config.dvbt.demod;

     dvbt_demod->i2c_base_addr = 0xD8;
     dvbt_demod->i2c_type_id = I2C_TYPE_SCB3;

     dvbt_tuner->freq_low = 40; // KHz
     dvbt_tuner->freq_high = 900; // KHz
     dvbt_tuner->id = AUI_CXD2872;
     dvbt_tuner->agc_ref = 0x63;
     //dvbt_tuner->rf_agc_min = 0x2A; // ?
     //dvbt_tuner->rf_agc_max = 0xBA; // ?
     dvbt_tuner->if_agc_max = 0xC3;
     dvbt_tuner->if_agc_min = 0x00;
     dvbt_tuner->tuner_crystal = 16;
     //dvbt_tuner->tuner_special_config = 0x01; // ?
     dvbt_tuner->wtuner_if_freq = 5000;
     //dvbt_tuner->tuner_ref_divratio = 64; // ?
     dvbt_tuner->tuner_agc_top = 252;
     //dvbt_tuner->tuner_step_freq = 62.5; // ?
     dvbt_tuner->i2c_type_id = I2C_TYPE_SCB3;
     dvbt_tuner->i2c_base_addr = 0xC0;
     dvbt_tuner->chip = tuner_chip_maxlinear;

     nim->id = AUI_NIM_ID_CXD2837_0;

     nim->nim_reset_gpio.position = 95;
     nim->nim_reset_gpio.io = AUI_GPIO_O_DIR;
     nim->nim_reset_gpio.gpio_val = AUI_GPIO_VALUE_LOW;
     nim->lnb_power_gpio.position = 96;
     nim->lnb_power_gpio.io = AUI_GPIO_O_DIR;
     nim->lnb_power_gpio.gpio_val = AUI_GPIO_VALUE_LOW;

#elif (defined BOARD_CFG_M3626)

#elif (defined BOARD_CFG_M3727) || (defined BOARD_CFG_M3712)|| (defined BOARD_CFG_M3712L)
     // Untested: Linux AUI NIM settints
     /* first NIM : DVB-C with internal demod and MxL603 tuner */
     nim = nim_config + 0;
     struct aui_tuner_dvbc_config *dvbc_tuner = &nim->config.dvbc.tuner;
     //struct aui_demod_dvbc_config *dvbc_demod = &nim->config.dvbc.demod;

	dvbc_tuner->id = AUI_MXL603;
	dvbc_tuner->rf_agc_max = 0xBA;
	dvbc_tuner->rf_agc_min = 0x2A;
	dvbc_tuner->if_agc_max = 0xFE;
	dvbc_tuner->if_agc_min = 0x01;
	dvbc_tuner->agc_ref = 0x80;

	dvbc_tuner->tuner_crystal = 16;
	dvbc_tuner->i2c_base_addr = 0xc0;
	dvbc_tuner->i2c_type_id = I2C_TYPE_SCB1;
	dvbc_tuner->tuner_special_config = 0x01;
	dvbc_tuner->wtuner_if_freq = 5000;
	dvbc_tuner->tuner_ref_divratio = 64;
	dvbc_tuner->tuner_agc_top = 1;
	dvbc_tuner->tuner_step_freq = 62.5;
	dvbc_tuner->tuner_if_freq_J83A = 5000;
	dvbc_tuner->tuner_if_freq_J83B = 5380;
	dvbc_tuner->tuner_if_freq_J83C = 5380;

	nim->id = AUI_NIM_ID_M3281_0;

	nim->nim_reset_gpio.position = AUI_GPIO_NONE;
	nim->nim_reset_gpio.gpio_val = AUI_GPIO_VALUE_LOW;
	nim->lnb_power_gpio.position = AUI_GPIO_NONE;
	nim->lnb_power_gpio.gpio_val = AUI_GPIO_VALUE_LOW;

	//dvbc_demod->i2c_type_id =  I2C_TYPE_SCB0;
	//dvbc_demod->i2c_base_addr = 0x66;
	//dvbc_demod->qam_mode = AUI_NIM_DVBC_MODE_J83AC | AUI_NIM_DEMO_SAMPLE_CLK_27M;

//#define DVB_C_NIM_R836
	/* second NIM : DVB-C with internal demod and MxL603 tuner */
	INC_INDEX(index);
	nim = nim_config + index;
	dvbc_tuner = &nim->config.dvbc.tuner;
	//dvbc_demod = &nim->config.dvbc.demod;

#if defined(DVB_C_NIM_R836)
    dvbc_tuner->id = AUI_R836;
#else
    dvbc_tuner->id = AUI_MXL603;
#endif
	dvbc_tuner->rf_agc_max = 0xBA;
	dvbc_tuner->rf_agc_min = 0x2A;
	dvbc_tuner->if_agc_max = 0xFE;
	dvbc_tuner->if_agc_min = 0x01;
	dvbc_tuner->agc_ref = 0x80;

	dvbc_tuner->tuner_crystal = 16;
#if defined(DVB_C_NIM_R836)
    dvbc_tuner->i2c_base_addr = 0x34;
#else
    dvbc_tuner->i2c_base_addr = 0xc0;
#endif
	dvbc_tuner->i2c_type_id = I2C_TYPE_SCB3;    
	dvbc_tuner->tuner_special_config = 0x01;
	dvbc_tuner->wtuner_if_freq = 5000;
	dvbc_tuner->tuner_ref_divratio = 64;
	dvbc_tuner->tuner_agc_top = 1;
	dvbc_tuner->tuner_step_freq = 62.5;
	dvbc_tuner->tuner_if_freq_J83A = 5000;
	dvbc_tuner->tuner_if_freq_J83B = 5380;
	dvbc_tuner->tuner_if_freq_J83C = 5380;

	nim->id = AUI_NIM_ID_M3281_1;

	nim->nim_reset_gpio.position = AUI_GPIO_NONE;
	nim->nim_reset_gpio.gpio_val = AUI_GPIO_VALUE_LOW;
	nim->lnb_power_gpio.position = AUI_GPIO_NONE;
	nim->lnb_power_gpio.gpio_val = AUI_GPIO_VALUE_LOW;

	//dvbc_demod->i2c_type_id =  I2C_TYPE_SCB0;
	//dvbc_demod->i2c_base_addr = 0x66;
	//dvbc_demod->qam_mode = AUI_NIM_DVBC_MODE_J83AC | AUI_NIM_DEMO_SAMPLE_CLK_27M;

#elif (defined BOARD_CFG_M3716L)
	nim = nim_config + 0;
	struct aui_tuner_dvbc_config *dvbc_tuner = &nim->config.dvbc.tuner;
	dvbc_tuner->id = AUI_R858;
	dvbc_tuner->rf_agc_max = 0xBA;
	dvbc_tuner->rf_agc_min = 0x2A;
	dvbc_tuner->if_agc_max = 0xFE;
	dvbc_tuner->if_agc_min = 0x01;
	dvbc_tuner->agc_ref = 0x80;
	dvbc_tuner->tuner_crystal = 16;
	dvbc_tuner->i2c_base_addr = 0xD4;
	dvbc_tuner->i2c_type_id = I2C_TYPE_SCB1;
	dvbc_tuner->tuner_special_config = 0x01;
	dvbc_tuner->wtuner_if_freq = 5000;
	dvbc_tuner->tuner_ref_divratio = 64;
	dvbc_tuner->tuner_agc_top = 1;
	dvbc_tuner->tuner_step_freq = 62.5;
	dvbc_tuner->tuner_if_freq_J83A = 5000;
	dvbc_tuner->tuner_if_freq_J83B = 5380;
	dvbc_tuner->tuner_if_freq_J83C = 5380;
	nim->id = AUI_NIM_ID_M3281_0;
	nim->nim_reset_gpio.position = AUI_GPIO_NONE;
	nim->nim_reset_gpio.gpio_val = AUI_GPIO_VALUE_LOW;
	nim->lnb_power_gpio.position = AUI_GPIO_NONE;
	nim->lnb_power_gpio.gpio_val = AUI_GPIO_VALUE_LOW;
	/* second NIM : DVB-C with internal demod and R858 tuner */
	INC_INDEX(index);
	nim = nim_config + index;
	dvbc_tuner = &nim->config.dvbc.tuner;
	//dvbc_demod = &nim->config.dvbc.demod;

	dvbc_tuner->id = AUI_R858;
	dvbc_tuner->rf_agc_max = 0xBA;
	dvbc_tuner->rf_agc_min = 0x2A;
	dvbc_tuner->if_agc_max = 0xFE;
	dvbc_tuner->if_agc_min = 0x01;
	dvbc_tuner->agc_ref = 0x80;

	dvbc_tuner->tuner_crystal = 16;
	dvbc_tuner->i2c_base_addr = 0xf4;
	dvbc_tuner->i2c_type_id = I2C_TYPE_SCB3;
	dvbc_tuner->tuner_special_config = 0x01;
	dvbc_tuner->wtuner_if_freq = 5000;
	dvbc_tuner->tuner_ref_divratio = 64;
	dvbc_tuner->tuner_agc_top = 1;
	dvbc_tuner->tuner_step_freq = 62.5;
	dvbc_tuner->tuner_if_freq_J83A = 5000;
	dvbc_tuner->tuner_if_freq_J83B = 5380;
	dvbc_tuner->tuner_if_freq_J83C = 5380;

	nim->id = AUI_NIM_ID_M3281_1;

	nim->nim_reset_gpio.position = AUI_GPIO_NONE;
	nim->nim_reset_gpio.gpio_val = AUI_GPIO_VALUE_LOW;
	nim->lnb_power_gpio.position = AUI_GPIO_NONE;
	nim->lnb_power_gpio.gpio_val = AUI_GPIO_VALUE_LOW;

#else
#error "Selected board is not supported by AUI"
#endif
	return 0;
}


