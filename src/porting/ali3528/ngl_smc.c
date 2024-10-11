/*
  FILE : stub_sc.c
  PURPOSE: This file is a stub for linking tests.
*/
#include "dtvsmc.h"
#include "aui_smc.h"
#include <stdio.h>
#include <string.h>
#include <cdlog.h>

typedef struct{
  aui_hdl hdl;
  aui_smc_attr attr;
  BYTE pps;
  INT locked;
  INT state;
  INT busy;//for reset read write
}SMCCARD;
typedef struct{
   INT slot;
   NGL_SMCSTATE_NOTIFY fn;
   void*userdata;
}SMCNOTIFY;

#define NB_NOTIFIERS 16
static SMCCARD CARDS[1]={NULL,0,0};
static SMCNOTIFY sNotifiers[NB_NOTIFIERS]={0,NULL};
static DWORD smc_msgq=0;
#define CHECKSLOT(s) {if(s<0||s>=sizeof(CARDS)/sizeof(SMCCARD))return E_INVALID_PARA;}

#define NGL_INSERT_CARD_FIRST -2
#define NGL_RESET_CARD_FIRST -3
#define NGL_RESOURCE_BUSY -4
typedef struct{
   DWORD slot;
   char*cmd;
   UINT size; 
   void* CBK;
   void*userdata;
}SMCMSG;

static void CardStateProc(int slot,unsigned int state)//state :plugged in or plugged out
{
    int i;
    CARDS[slot].state=(state?eSMCCARD_INSERTED:eSMCCARD_EXTRACTED);
    for(i=0;i<NB_NOTIFIERS;i++){
        if(NULL==sNotifiers[i].fn)continue;
        if((sNotifiers[i].slot==slot)||(sNotifiers[i].slot<0))
            sNotifiers[i].fn(slot, CARDS[slot].state,sNotifiers[i].userdata);
    }
    NGLOGD("Card %d state=%d",slot,state); 
}

static int m_conversion_factor[] = {  372, 372, 558, 744, 1116, 1488, 1860, -1, -1, 512, 768, 1024, 1536, 2048, -1, -1};
// Di=64 is valid in ISO/IEC 7816-3:2006
static int m_adjustment_factor[] = {-1, 1, 2, 4, 8, 16, 32, 64, 12, 20, -1, -1, -1, -1, -1, -1};
static int va_sc_get_smaller_DN(int DR)
{
    int i;
    int length;
    int factor_array[32] = {0};
    int factor_cnt = 0;
    int DN = -1;
    int max = 0;
    
    //step 1: copy the values that smaller than DR to factor_array
    length = sizeof(m_adjustment_factor)/sizeof(int);
    for (i = 0; i < length; i ++)  {
        if (  (m_adjustment_factor[DR] > m_adjustment_factor[i]) &&   (m_adjustment_factor[i] != -1)   )
        {
            factor_array[factor_cnt] = m_adjustment_factor[i];
            factor_cnt ++;
        }
    }

    // can not find the smaller array
    if (0 == factor_cnt) {
        for (i = 0; i < length; i ++) {
            if (1 == m_adjustment_factor[i])  {
                DN = i;
                break;
            }
        }
    } else  {
        max = 0;
        //step 2: find the biggest value of factor_array[]
        for (i = 0; i < factor_cnt; i ++){
            if (factor_array[i] > max) {
                max = factor_array[i];
            }
        }

        //step 3: return the DN
        for (i = 0; i < length; i ++) {
            if (max == m_adjustment_factor[i]) {
                DN = i;
                break;
            }
        }
    }
    NGLOGD("max = %d, DN = %d\n", max, DN);
    
    return DN;
    
}
#define MAX_ATR_LN 33
static void ResetSMC(DWORD slot,BYTE*atr,UINT atr_len,NGL_SMCRESET_NOTIFY cbk){
    unsigned char TS0, TD1, index = 1;
    int T = 0;
    int set_pps = 0;
    int write_len, read_len;
    int i;
    int va_ret;
    unsigned char FI, DI;
    int Fi, Di, etu;
    unsigned char FR, DR;
    int Fr, Dr;
    unsigned char FN, DN;
    aui_smc_param_t smc_param; 
    unsigned char tx_buffer[4], rx_buffer[280];
    BYTE pps1_value=0;
    AUI_RTN_CODE ret = AUI_RTN_FAIL;
    int ret_val =E_OK;
    pps1_value=CARDS[slot].pps;
    FR = (pps1_value >> 4) & 0x0F;
    DR = pps1_value & 0x0F;
    Fr = m_conversion_factor[FR];
    Dr = m_adjustment_factor[DR];
    NGLOGD("FR:%d, DR:%d, Fr:%d, Dr:%d, Fd: 372, Dd: 1\n", (int)FR, (int)DR, (int)Fr, (int)Dr);

    if (pps1_value){
        if ((atr[1] & 0x10) && (atr_len >= 2)){
            set_pps = 1;
  	    FI = (atr[2] >> 4) & 0x0F;
	    DI = atr[2] & 0x0F;
	    Fi = m_conversion_factor[FI];
	    Di = m_adjustment_factor[DI];

	    if (Fr > Fi)FN = FI;
	    else	FN = FR;
			
	    if (Dr > Di)	DN = DI;
	    else	DN = DR;
			
	    pps1_value =  (FN << 4)| (DN & 0x0f);

	}else{
	    //do not set pps if no TA[0] in ATR;
	    set_pps = 0; 
	    pps1_value = 0x11;
   	}
	NGLOGD("pps1_value = 0x%x\n", pps1_value);
		
	index = 1;//atr[0] is initial charater
	TS0 = atr[index++];
	if (TS0 & 0x10) index++;
	if (TS0 & 0x20) index++;
	if (TS0 & 0x40) index++;
	if (TS0 & 0x80) { // if T0 bit8=1, TD1 exist.
	    TD1 = atr[index];
	    NGLOGD("TD1=0x%02X\n", TD1);
	    if(!(TD1 & 0x10)) { 
	        // if TD1 bit5=0, TA2 is not exist, card in negotiable mode, can set PPS
	       T = TD1 & 0x0F;
	       NGLOGD("Smart card is negotiable mode\n");
	    } else {
		// if TD1 bit5=1, TA2 exist, card in specific mode. can not set PPS
		NGLOGD("Smart card is specific mode\n");
		set_pps = 0;
	    }
        } else {
	    TD1 = 0;
	    T = 0;
        }
	NGLOGD("set_pps=%d, TS0=0x%x\n", set_pps, (unsigned int)TS0);
  }else{
	//do not set pps if pps1_value = 0;
	set_pps = 0; 
  }	
  NGLOGD("final pps1_value = 0x%x\n", pps1_value);
  retry_pps:
       if (set_pps){
           memset(&smc_param, 0, sizeof(smc_param));
           memset(rx_buffer, 0x00, sizeof(rx_buffer));
        
           //send PPS request
    	   write_len = 4;
    	   read_len = 4;
    	   tx_buffer[0] = 0xFF;
    	   tx_buffer[1] = 0x10 | T;
    	   tx_buffer[2] = pps1_value;
    	   tx_buffer[3] = 0x00;
    	   for (i = 0; i < 3; i++) {
    		tx_buffer[3] ^= tx_buffer[i];
    	   }
           //NGLOG_DUMP("Send PPS: ", tx_buffer, 4);

    	   FI = (tx_buffer[2] >> 4) & 0x0F;
    	   DI = tx_buffer[2] & 0x0F;
    	   Fi = m_conversion_factor[FI];
    	   Di = m_adjustment_factor[DI];

           NGLOGD("F:%d, D:%d, Fi:%d, Di:%d, Fd: 372, Dd: 1\n", (int)FI, (int)DI, (int)Fi, (int)Di);

           //send PPS command to smart card
           ret = aui_smc_setpps(CARDS[slot].hdl, tx_buffer, write_len, rx_buffer, &read_len);
           if (AUI_RTN_SUCCESS != ret){
               NGLOGE("aui_smc_setpps error, ret = %d\n", (int)ret);
               ret_val =E_ERROR;
               goto reset_thread_exit;
           }
           NGLOG_DUMP("Get from PPS: ", rx_buffer, 4);
           NGLOGD("Fn:%d, Dn:%d\n",(int)((rx_buffer[2] >> 4) & 0x0F), (int)(rx_buffer[2] & 0x0F));

           if (memcmp(tx_buffer, rx_buffer, 4) == 0){
    		etu = Fi / Di;
    		smc_param.m_nETU = etu;
    		if (T == 0) {
    			smc_param.m_e_protocol = EM_AUISMC_PROTOCOL_T0;
    		} else if (T == 1) {
    			smc_param.m_e_protocol = EM_AUISMC_PROTOCOL_T1;
    		}
                ret = aui_smc_param_set(CARDS[slot].hdl, &smc_param);
               if (AUI_RTN_SUCCESS != ret){
                    NGLOGE("aui_smc_param_set error, ret = %d\n", (int)ret);
                    ret_val = E_ERROR;
                    goto reset_thread_exit;
               }
           }else{ // not support the pps, retry with smaller DN
               FN = (pps1_value >> 4) & 0x0F;
               DR = pps1_value & 0x0F;
               if (1 == DR){//DR = 1, can not set pps with the smallest value, do not need to try again
                   pps1_value = CARDS[slot].pps;//m_va_sc_slot[slot_num].pps1_value;
                   NGLOGD("can not set pps with the smallest value, do not need to try");
                
               }else{
                   DN = va_sc_get_smaller_DN(DR);
                   pps1_value =  (FN << 4)| (DN & 0x0f);
                   NGLOGD("retry pps1_value=0x%x\n", (unsigned int)pps1_value);
                   goto retry_pps;
               }
           }
       }//if set(pps)
   reset_thread_exit:
       if( E_OK == ret_val)
           cbk(slot,atr, atr_len, pps1_value);
       else
           cbk(slot,NULL,0,0);
}
#define CMD_MAX_LEN 512
static void SendCMD(DWORD slot_num,BYTE*cmd_buf,UINT cmd_size,NGL_SMCCOMMAND_NOTIFY cbk,void*userdata){
    AUI_RTN_CODE ret = AUI_RTN_FAIL;
    int read_cmd = 0;
    short actlen = 0;
    unsigned char *cmd_head = NULL;
    unsigned char cmd_head_tmp[5];
    unsigned char *cmd_data = NULL;
    short cmd_head_size;
    short cmd_data_size;
    unsigned char recv_buffer[CMD_MAX_LEN];   
    INT va_ret;
    unsigned char *cmd_response = NULL;
    UINT32 cmd_response_size;
    int i;
    int apdu_case_1 = 0;
    
    NGLOGD("slot num: %d, cmd_size=%d\n", (unsigned int)slot_num, cmd_size);
    
    cmd_head_size = 5;

    if (cmd_head_size == cmd_size){
        NGLOGD("It is smart card read cmmmand!\n");    
        read_cmd = 1;
    }
    
    if (cmd_size == 4) {  
        memcpy(cmd_head_tmp,cmd_buf, 4);
        cmd_head_tmp[4] = 0;
        cmd_head = cmd_head_tmp;
        read_cmd = 1;
        apdu_case_1 = 1;
    }else {
        cmd_head = (unsigned char*)cmd_buf;
    }
   
    cmd_data = (unsigned char*)(cmd_buf + cmd_head_size);
    if (cmd_size > cmd_head_size){
        cmd_data_size = cmd_size - cmd_head_size;
    }else{
        //cmd_head_size = m_va_sc_slot[slot_num].cmd_size;
        cmd_data_size = 0;
    }
    
    //step1: send command head(5 bytes)
    NGLOG_DUMP("cmd_head:", cmd_head, cmd_head_size);
    ret = aui_smc_raw_write(CARDS[slot_num].hdl, cmd_head, cmd_head_size, &actlen);
    if (AUI_RTN_SUCCESS != ret){
        NGLOGE("aui_smc_raw_write error, ret = %d\n", (int)ret);
        goto send_thread_exit;
    }
    NGLOGD("actlen: %d, cmd_data_size=%d\n", actlen, (int)cmd_data_size);

    //step2: get command response, if no command data, no need to get command head response,
    // get SW1 and SW2 is enough.
    if (cmd_data_size || read_cmd) {
        for (i = 0; i < 10; i++){
            ret = aui_smc_raw_read(CARDS[slot_num].hdl, recv_buffer, 1, &actlen);
            if (AUI_RTN_SUCCESS != ret){
                NGLOGE("aui_smc_raw_read error, ret = %d\n", (int)ret);
                goto send_thread_exit;
            }
             
            if (recv_buffer[0] == 0x60) {
                NGLOGD("NULL BYTE detected\n");
                continue;        
            } else if ((apdu_case_1 == 1) && (recv_buffer[0] == cmd_head[1])) {
                NGLOGD("ACK received\n");
                continue;
            } else if ((apdu_case_1 == 1) && (recv_buffer[0] == (cmd_head[1] ^ 0xFF))) {
                NGLOGD("ACK (INS ^ FF) received\n");
                continue;
            } else {
                NGLOGD("SW1 detected\n");
                break;
            }
        }
        if (i >= 10) {
            NGLOGE("smart card read data response sw1 timeout, recv_buffer[0]=0x%x\n",(unsigned int)(recv_buffer[0]));
            ret = AUI_RTN_FAIL;
            goto send_thread_exit;
        }
        NGLOG_DUMP("SC send head ACK: ", recv_buffer, actlen);
        if (((recv_buffer[0] >> 4) == 0x06) || ((recv_buffer[0] >> 4) == 0x09)){
            // if the response is 0x6X or 0x9X(!= 0x60), it means SW1, now read SW2
            ret = aui_smc_raw_read(CARDS[slot_num].hdl, recv_buffer+1, 1, &actlen);
            if (AUI_RTN_SUCCESS != ret){
                NGLOGE("aui_smc_raw_read SW2 error, ret = %d\n", (int)ret);
                goto send_thread_exit;
            }
            NGLOG_DUMP("SC SW1, SW2: ", recv_buffer, 2);
            //m_va_sc_slot[slot_num].card_status = CARD_STATUS_SENT_CMD;
            //va_ret = VA_SC_CommandDone(slot_num, 2, recv_buffer);
            cbk(slot_num,recv_buffer,2,userdata);
            goto send_thread_exit;
        }
    }

    if (!read_cmd){ // send data

        //step3: send command data
        if (cmd_data_size)  {
            ret = aui_smc_raw_write(CARDS[slot_num].hdl, cmd_data, cmd_data_size, &actlen);
            if (AUI_RTN_SUCCESS != ret){
                NGLOGE("aui_smc_raw_read error, ret = %d\n", (int)ret);
                goto send_thread_exit;
            }
        }

        //step4.1:  get command response(SW1)
        cmd_response = recv_buffer;
        cmd_response_size = 2;
        for (i = 0; i < 10; i++){
            ret = aui_smc_raw_read(CARDS[slot_num].hdl, cmd_response, 1, &actlen);
            if (AUI_RTN_SUCCESS != ret){
                NGLOGE("aui_smc_raw_read SW1 error, ret = %d\n", (int)ret);
                goto send_thread_exit;
            }
            NGLOG_DUMP("SC send data response SW1: ", cmd_response, actlen);
            if (recv_buffer[0] != 0x60)
                break;
        }
        if (i >= 10) {
            NGLOGE("smart card read data response sw1 timeout, recv_buffer[0]=0x%x\n",
                (unsigned int)(recv_buffer[0]));
            ret = AUI_RTN_FAIL;
            goto send_thread_exit;
        }

        //step4.2:  get command response(SW2)        
        ret = aui_smc_raw_read(CARDS[slot_num].hdl, cmd_response+1, 1, &actlen);
        if (AUI_RTN_SUCCESS != ret){
            NGLOGE("aui_smc_raw_read SW2 error, ret = %d\n", (int)ret);
            goto send_thread_exit;
        }
        NGLOG_DUMP("SC send data response SW2: ", cmd_response+1, actlen);

        //m_va_sc_slot[slot_num].card_status = CARD_STATUS_SENT_CMD;
        cbk(slot_num,cmd_response,cmd_response_size,userdata);
        /*va_ret = VA_SC_CommandDone(slot_num, cmd_response_size, cmd_response);
        if (va_ret){
            NGLOGE("VA_SC_CommandDone error, ret = %d\n", (int)va_ret);
            goto send_thread_exit;
        }*/
    }else{// read data
        //step3: read data
        short read_length = cmd_buf[4];
        NGLOGD("smart card read length=%d\n", (int)read_length);
        if (0 == read_length)
            read_length = 256;
        
        ret = aui_smc_raw_read(CARDS[slot_num].hdl, recv_buffer, read_length, &actlen);
        if (AUI_RTN_SUCCESS != ret){
            NGLOGE("aui_smc_raw_read error, ret = %d\n", (int)ret);
            goto send_thread_exit;
        }
        NGLOG_DUMP("SC read data: ", recv_buffer, actlen);

        //step4.1:  get command response(SW1)
        int i;
        cmd_response = recv_buffer+read_length;
        cmd_response_size = read_length+2;
        for (i = 0; i < 10; i++){
            ret = aui_smc_raw_read(CARDS[slot_num].hdl, cmd_response, 1, &actlen);
            if (AUI_RTN_SUCCESS != ret){
                NGLOGE("aui_smc_raw_read SW1 error, ret = %d\n", (int)ret);
                goto send_thread_exit;
            }
            NGLOG_DUMP("SC send data response SW1: ", cmd_response, actlen);
            if (recv_buffer[0] != 0x60)
                break;
        }
        if (i >= 10) {
            NGLOGE("smart card read data response sw1 timeout, recv_buffer[0]=0x%x\n", (unsigned int)(recv_buffer[0]));
            ret = AUI_RTN_FAIL;
            goto send_thread_exit;
        }

        //step4.2:  get command response(SW2)        
        ret = aui_smc_raw_read(CARDS[slot_num].hdl, cmd_response+1, 1, &actlen);
        if (AUI_RTN_SUCCESS != ret){
            NGLOGE("aui_smc_raw_read SW2 error, ret = %d\n", (int)ret);
            goto send_thread_exit;
        }
        NGLOG_DUMP("SC send data response SW2: ", cmd_response+1, actlen);

        NGLOG_DUMP("SC send reding data: ", recv_buffer, cmd_response_size);
        //m_va_sc_slot[slot_num].card_status = CARD_STATUS_SENT_CMD;
        cbk(slot_num,recv_buffer,cmd_response_size,userdata);
    }
send_thread_exit:
    //m_va_sc_slot[slot_num].card_status = CARD_STATUS_SENT_CMD;
    if (ret)
        cbk(slot_num,NULL,-1,userdata);//VA_SC_CommandFailed(slot_num);
    NGLOGD("send command Done!\n");
}
static void ResetProc(void*p){
    while(1){
        SMCMSG msg;
        unsigned char atr[MAX_ATR_LN];
        unsigned short atr_len=MAX_ATR_LN;
        int rc=nglMsgQReceive(smc_msgq,&msg,sizeof(SMCMSG),1000);
        if(rc!=E_OK)continue;
        NGLOGD("msg.slot=%d cmd=%p size=%d",msg.slot,msg.cmd,msg.size);
        if(msg.cmd==NULL){
            int rc=aui_smc_reset(msg.slot,atr,&atr_len,1);
            NGLOGD("aui_smc_reset(%d)=%d",msg.slot,rc);
            ResetSMC(msg.slot,atr,atr_len,(NGL_SMCRESET_NOTIFY)msg.CBK);
            CARDS[msg.slot].busy--;
        }else{
            SendCMD(msg.slot,msg.cmd,msg.size,(NGL_SMCCOMMAND_NOTIFY)msg.CBK,msg.userdata);
            nglFree(msg.cmd);
            CARDS[msg.slot].busy--;
        }
   }//endof while(1)
}

static AUI_RTN_CODE *smc_init_cb(void *para)
{
    aui_smc_device_cfg_t *pconfig = (aui_smc_device_cfg_t *)para;
    NGLOGD("M3528 viaccess. running smc_init_cb");
    //pconfig++; // 3733 use slot 1
    pconfig->init_clk_trigger = 1;
    pconfig->init_clk_number = 1;
    pconfig->apd_disable_trigger = 1;
    pconfig->def_etu_trigger = 1;
    pconfig->default_etu = 372;
    pconfig->warm_reset_trigger = 1;
    pconfig->init_clk_array[0] = 3600000;
    
    pconfig->invert_detect = 1;

    pconfig->force_tx_rx_trigger = 1;
    pconfig->force_tx_rx_cmd = 0xdd;
    pconfig->force_tx_rx_cmd_len = 5;

    pconfig->disable_pps = 1;
    NGLOGD("disable_pps: %d", pconfig->disable_pps);
    return AUI_RTN_SUCCESS;
}

INT nglSmcInit(){
    int rc,i;
    aui_hdl hdl;
    DWORD thid;
    if(0!=smc_msgq)
        return E_OK;
    smc_msgq=nglMsgQCreate(8,sizeof(SMCMSG));
    nglCreateThread(&thid,0,0,ResetProc,NULL);
    rc=aui_find_dev_by_idx(AUI_MODULE_SMC,0,&hdl);
    rc=aui_smc_init(smc_init_cb);
    memset(sNotifiers,0,sizeof(sNotifiers));
    for(i=0;i<sizeof(CARDS)/sizeof(SMCCARD);i++){
        SMCCARD *s=CARDS+i;
        aui_smc_attr attr;
        memset(&s->attr,0,sizeof(aui_smc_attr));
        s->attr.ul_smc_id=i;
        s->attr.p_fn_smc_cb=CardStateProc;
        rc=aui_smc_open(&s->attr,&s->hdl);
    }
    return E_OK;
}

INT nglSmcRegisterNotify(DWORD dwScSlot,NGL_SMCSTATE_NOTIFY fn,void*userdata){
    int i;
    NGLOGD("fn=%p",fn);
    for(i=0;i<NB_NOTIFIERS;i++){
         if(sNotifiers[i].fn==fn)
            return E_ERROR;
    }
    for(i=0;i<NB_NOTIFIERS;i++){
        if(NULL==sNotifiers[i].fn){
            sNotifiers[i].fn=fn;
            sNotifiers[i].slot=dwScSlot;
            sNotifiers[i].userdata=userdata;
            break;
        }
    }
    return E_OK;
}

INT nglSmcUnRegister(NGL_SMCSTATE_NOTIFY fn){
    int i;
    for(i=0;i<NB_NOTIFIERS;i++){
        if(fn==sNotifiers[i].fn){
            sNotifiers[i].fn=NULL;
            sNotifiers[i].slot=-1;
            return E_OK;
        }
    }
    return E_ERROR;
}
#define MAX_ATR_LN 33

INT nglSmcReset( DWORD dwScSlot, UINT8 uiPPSNegotiationValue,NGL_SMCRESET_NOTIFY fn )
{ 
   aui_smc_param_t smc_param;
   int rc=E_OK;
   unsigned char atr[MAX_ATR_LN];
   unsigned short i,atr_len = MAX_ATR_LN;
   NGLOGD("slot=%d pps=0x%x",dwScSlot,uiPPSNegotiationValue);
   CHECKSLOT(dwScSlot);
   bzero(&smc_param,sizeof(smc_param));
   if(eSMCCARD_EXTRACTED==CARDS[dwScSlot].state){
        fn(dwScSlot,NULL,-1,0);
         NGLOGD("NGL_INSERT_CARD_FIRST");
        return NGL_INSERT_CARD_FIRST;
   }
   if(CARDS[dwScSlot].busy){
        fn(dwScSlot,NULL,-1,0);
        NGLOGD("NGL_RESOURCE_BUSY");
        return NGL_RESOURCE_BUSY;
   }
   CARDS[dwScSlot].busy++;
   CARDS[dwScSlot].pps=uiPPSNegotiationValue;
   //rc=aui_smc_reset(CARDS[dwScSlot].hdl,atr,&atr_len,1);
   SMCMSG msg;
   msg.slot=dwScSlot;
   msg.cmd=NULL;
   msg.size=uiPPSNegotiationValue;
   msg.CBK=fn;
   nglMsgQSend(smc_msgq,&msg,sizeof(SMCMSG),100);
   NGLOGD("atr=%p atr_len=%d",atr,atr_len);
   /*if(rc==0){
       CARDS[dwScSlot].state=eSMCCARD_READY;
       fn(dwScSlot,atr,atr_len,uiPPSNegotiationValue);
   }else
       fn(dwScSlot,NULL,-1,0);
   */
   return E_OK;
}

INT nglSmcSendCommand(DWORD dwScSlot,BYTE*command,UINT cmdsize,NGL_SMCCOMMAND_NOTIFY fn,void*userdata){
    CHECKSLOT(dwScSlot);
    if(NULL==command||0==cmdsize)
        return E_INVALID_PARA;
    switch(CARDS[dwScSlot].state){
    case eSMCCARD_EXTRACTED:return NGL_INSERT_CARD_FIRST;
    case eSMCCARD_INSERTED:return NGL_RESET_CARD_FIRST;
    case eSMCCARD_READY://path through to default
    default:break;
    }
    if(CARDS[dwScSlot].busy)
        return NGL_RESOURCE_BUSY;
    SMCMSG msg;
    CARDS[dwScSlot].busy++;
    msg.slot=dwScSlot;
    msg.cmd=(char*)nglMalloc(cmdsize);
    memcpy(msg.cmd,command,cmdsize);
    msg.size=cmdsize;
    msg.CBK=fn;
    msg.userdata=userdata;
    nglMsgQSend(smc_msgq,&msg,sizeof(SMCMSG),100);
}

INT nglSmcTransfer( DWORD dwScSlot,BYTE*command,UINT cmdsize,BYTE*response,UINT *responseSize)
{
    CHECKSLOT(dwScSlot);
    if(NULL==command||0==cmdsize)
        return E_INVALID_PARA;
    //NGLOGD("%s  state=%s\r\n",__FUNCTION__,STATENAME(CARDS[dwScSlot].state));
    switch(CARDS[dwScSlot].state){
    case eSMCCARD_EXTRACTED:return NGL_INSERT_CARD_FIRST;
    case eSMCCARD_INSERTED:return NGL_RESET_CARD_FIRST;
    case eSMCCARD_READY://path through to default
    default:break;
    }
    if(CARDS[dwScSlot].busy)
        return NGL_RESOURCE_BUSY;
    CARDS[dwScSlot].busy++;
    int rc=aui_smc_transfer(CARDS[dwScSlot].hdl,command,cmdsize,response,&responseSize);
    return E_OK;
}

INT nglSmcGetState(DWORD dwScSlot, tNGL_SMC_State *pScState)
{ 
   NGLOGD("slot=%d pScState=%p state=%d",dwScSlot,pScState,((dwScSlot==0)?CARDS[dwScSlot].state:-1));
   CHECKSLOT(dwScSlot);
   if(NULL==pScState)
       return E_INVALID_PARA;

   *pScState=CARDS[dwScSlot].state;
   return E_OK;
}




INT nglSmcRead(DWORD dwScSlot, BYTE *pCommand,UINT uiCommandSize)
{ 
   INT rc,responseSize,writed;
   NGLOGD("slot=",dwScSlot);
   CHECKSLOT(dwScSlot);
   if(NULL==pCommand||uiCommandSize==0)
       return E_INVALID_PARA;
   switch(CARDS[dwScSlot].state){
   case eSMCCARD_EXTRACTED:return E_ERROR;//NGL_INSERT_CARD_FIRST;
   case eSMCCARD_INSERTED:return E_ERROR;//NGL_RESET_CARD_FIRST;
   case eSMCCARD_READY://path through to default
   default:break;
   }
   INT readed;
   BYTE response[8];
   rc=aui_smc_receive(CARDS[dwScSlot].hdl,pCommand,uiCommandSize,&readed,response,&responseSize,-1/*timeout*/);
   return readed;
}
     
INT nglSmcActivate(DWORD dwScSlot)
{ 
   NGLOGD("slot=",dwScSlot);
   CHECKSLOT(dwScSlot);
   if(eSMCCARD_EXTRACTED==CARDS[dwScSlot].state)
        return NGL_INSERT_CARD_FIRST; 
   aui_smc_active(CARDS[dwScSlot].hdl);
   return E_OK;
}


INT nglSmcDeactivate(DWORD dwScSlot)
{ 
   CHECKSLOT(dwScSlot);
   if(eSMCCARD_EXTRACTED==CARDS[dwScSlot].state)
        return NGL_INSERT_CARD_FIRST; 
   aui_smc_deactive(CARDS[dwScSlot].hdl);
   return E_OK;
}


INT nglSmcLock(DWORD dwScSlot)
{ 
    NGLOGD("slot=",dwScSlot);
    CHECKSLOT(dwScSlot);
    if(CARDS[dwScSlot].locked>0)
        return E_ERROR;//RESOURCE_ALREADY_LOCKED;
    if(eSMCCARD_EXTRACTED==CARDS[dwScSlot].state)
        return NGL_INSERT_CARD_FIRST; 
    CARDS[dwScSlot].locked++;
    return E_OK;
}


INT nglSmcUnlock(DWORD dwScSlot)
{ 
   NGLOGD("slot=",dwScSlot);
   CHECKSLOT(dwScSlot);
   if(CARDS[dwScSlot].locked==0)
       return E_ERROR;//NGL_RESOURCE_NOT_LOCKED;
   CARDS[dwScSlot].locked--;
   if(eSMCCARD_EXTRACTED==CARDS[dwScSlot].state)
        return NGL_INSERT_CARD_FIRST; 
   return E_OK;
}

/* End of File */
