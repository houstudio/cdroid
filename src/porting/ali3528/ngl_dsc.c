#include<stdlib.h>
#include<stdio.h>
#include<ngl_dsc.h>
#include<aui_dsc.h>
#include<aui_dmx.h>
#include<aui_kl.h>
#include <ngl_types.h>
#include <ngl_log.h>

NGL_MODULE(DSCR);

#define INVALID_PID 0x1FFFF
typedef struct{
   aui_hdl hdl;
   aui_hdl hdl_kl;
   aui_attr_dsc attr;
   aui_cfg_kl klcfg;
   BYTE key[64];
   BYTE iv[32];
   BYTE pk[32];
   int algo;
   int schip_flag;
   USHORT pid;
   USHORT pids[16];
   UINT pid_cnt;
   NGLCipherMode cipherMode;
   int key_len;
}NGLDSC;
typedef struct{
   aui_hdl hdl;
   unsigned short pids[16];
   unsigned int pid_count;
}DSCHDL;

NGLSCHIP_ContentKey*pContentKey=NULL;
#define NUM_DSCS 8
static NGLDSC nglDSCS[NUM_DSCS];
static DSCHDL dscHandles[eDSC_ALGO_ENUM_LAST];
static NGLDSC*GetNGDSC(UINT16 pid){
	int i;
	for(i=0;i<sizeof(nglDSCS)/sizeof(NGLDSC);i++)
		if(nglDSCS[i].pid==pid)
			return nglDSCS+i;
	return NULL;
}
#define CHECK(p) {if(p<nglDSCS||p>=&nglDSCS[NUM_DSCS])return E_INVALID_PARA;}
DWORD nglDscInit(){
    aui_attr_dsc attr;
    NGLOGD("");
    aui_dsc_init(NULL,NULL);
    aui_kl_init(NULL,NULL);
    NGLOGD("aui_dsc_init");
    aui_log_priority_set(AUI_MODULE_DSC,AUI_LOG_PRIO_DEBUG);
    aui_log_priority_set(AUI_MODULE_KL,AUI_LOG_PRIO_DEBUG);
    bzero(nglDSCS,sizeof(nglDSCS));
    bzero(dscHandles,sizeof(dscHandles));
	//bzero(nglDscChPids,sizeof(nglDscChPids));

    attr.uc_dev_idx = 0;
    attr.dsc_data_type = AUI_DSC_DATA_TS;
    attr.uc_algo = AUI_DSC_ALGO_CSA;
    aui_dsc_open(&attr,&dscHandles[eDSC_ALGO_DVB_CSA].hdl);

    attr.uc_dev_idx=1;
    attr.uc_algo=AUI_DSC_ALGO_AES;
    aui_dsc_open(&attr,&dscHandles[eDSC_ALGO_AES_128_CBC].hdl);
    NGLOGD("dsc handles[0]=%p handles[1]=%p",dscHandles[eDSC_ALGO_DVB_CSA].hdl,dscHandles[eDSC_ALGO_AES_128_CBC].hdl);
}

DWORD nglDscOpen(USHORT*pids,UINT cnt)
{
    int i;
	NGLDSC*dsc=GetNGDSC(pids[0]);
	if(cnt==0||NULL!=dsc||pids[0]>=0x1FFF){
		NGLOGE("pid %d exists or invalid pid",(pids?pids[0]:-1));
		return  NULL;
	}
    for(i=0;i<sizeof(nglDSCS)/sizeof(NGLDSC);i++){
        if(nglDSCS[i].pid==0){
           dsc=nglDSCS+i;
           break;
        }
    }
    bzero(&dsc->attr,sizeof(dsc->attr));
    bzero(&dsc->klcfg,sizeof(aui_cfg_kl));
    dsc->attr.uc_dev_idx = (dsc-nglDSCS);
    dsc->attr.dsc_data_type =AUI_DSC_DATA_TS;
    dsc->attr.en_residue = AUI_DSC_RESIDUE_BLOCK_IS_AS_ATSC;//AUI_DSC_RESIDUE_BLOCK_IS_RESERVED;
    dsc->attr.en_en_de_crypt=AUI_DSC_DECRYPT;//AUI_DSC_ENCRYPT;CSA must be DECRYPT(othrewise aui_dsc_attach_key_info2dsc will return error)

    dsc->attr.uc_algo = AUI_DSC_ALGO_CSA;
    dsc->algo = eDSC_ALGO_DVB_CSA;
    dsc->attr.csa_version=AUI_DSC_CSA2;
    dsc->attr.uc_mode=AUI_DSC_WORK_MODE_IS_CBC;
    dsc->attr.puc_iv_ctr=NULL;

    dsc->hdl=NULL;
    memcpy(dsc->pids,pids,cnt*sizeof(USHORT));
    dsc->schip_flag=0;
    dsc->pid=dsc->pids[0];
    dsc->attr.pus_pids=&dsc->pids;
    dsc->attr.ul_pid_cnt=dsc->pid_cnt=cnt;
    NGLOGD("\t %s dsc=%p pid=%d cnt=%d",__FUNCTION__,dsc,pids[0],cnt);
    return (DWORD)dsc;
}

DWORD nglDscClose(DWORD dwDescrambleID )
{
    NGLDSC*dsc=(NGLDSC*)dwDescrambleID;
    CHECK(dsc);
    NGLOGD("%s dsc=%p hdl=%p hdl_kl=%p pid=%d",__FUNCTION__,dsc,dsc->hdl,dsc->hdl_kl,(dsc?dsc->pid:0));
    if((dsc->hdl!=NULL)||(0!=dsc->pid)){
        aui_dsc_deattach_key_by_pid(dsc->hdl,dsc->pid);
        int cnt=dscHandles[dsc->algo].pid_count;
        for(int i=0;i<cnt;i++){
            unsigned short *ps=dscHandles[dsc->algo].pids; 
            if(dscHandles[dsc->algo].pids[i]==dsc->pid){
                memcpy(ps+i,ps+i+1,(cnt-i-1)*sizeof(short));
                dscHandles[dsc->algo].pid_count--;
            }
        }
		if(dsc->attr.pus_pids)// clean pids
			bzero(dsc->attr.pus_pids,dsc->attr.ul_pid_cnt);
         dsc->pid_cnt=0;
		 dsc->pid=0;
         //aui_dsc_close(dsc->hdl);
        if(dsc->hdl_kl)
           aui_kl_close(dsc->hdl_kl);
         dsc->hdl_kl=NULL;
         dsc->hdl=NULL;
    }
    return E_OK;
}
static const char*AuiAlgo(int algo)
{
   const char *names[]={"DES","AES","SHA","TDES","CSA"};
   if(algo<sizeof(names)/sizeof(char*)&&algo>=0)
      return names[algo];
   return "??";
}
static char*AuiKeyType(int tp){
   const char*names[]={"REG","SRAM","KL","OTP"};
   if(tp<sizeof(names)/sizeof(char*)&&tp>=0)
      return names[tp];
   return "??";
}
static INT OpenHDL(NGLDSC*dsc){
    int rc=0;
    if(NULL==dsc->hdl){
        aui_hdl hdl_dmx;
        aui_dmx_data_path dmx_path;
        dsc->attr.dsc_key_type =(0==dsc->schip_flag/*||AUI_DSC_ALGO_CSA==dsc->attr.uc_algo*/)?AUI_DSC_HOST_KEY_SRAM:AUI_DSC_CONTENT_KEY_KL;
        switch(dsc->attr.uc_algo){
        case AUI_DSC_ALGO_CSA:dsc->attr.en_en_de_crypt=AUI_DSC_DECRYPT;break;//CSA must be DECRYPT
        case AUI_DSC_ALGO_AES:dsc->attr.en_en_de_crypt=AUI_DSC_DECRYPT;break;
        }
        dsc->hdl=dscHandles[dsc->algo].hdl;  
	    dsc->attr.uc_dev_idx =(int)dsc->algo;
        
        if(AUI_RTN_SUCCESS!=aui_find_dev_by_idx(AUI_MODULE_DMX,0,&hdl_dmx))
            NGLOGD("OpenHDL find_dev of DMX failed");

        if(dsc->hdl){
          bzero(&dmx_path,sizeof(dmx_path));
          dmx_path.data_path_type = AUI_DMX_DATA_PATH_DE_PLAY;
          dmx_path.dsc_type = AUI_DMX_DATA_PATH_DSC_TYPE_HANDLE;
          dmx_path.p_hdl_de_dev =dsc->hdl;
          rc=aui_dmx_data_path_set(hdl_dmx,&dmx_path);
        }
        NGLOGD_IF(rc,"OpenHDL aui_dmx_data_path_set=%d hdl_dmx=%p  dschdl=%p algo=%s",rc,hdl_dmx,dsc->hdl,AuiAlgo(dsc->attr.uc_algo));
	}
	if(NULL==dsc->hdl_kl){
        int idx=dsc-nglDSCS;
        if(aui_find_dev_by_idx(AUI_MODULE_KL,idx,&dsc->hdl_kl)){
            struct aui_attr_kl attr;
            memset(&attr,0,sizeof(aui_attr_kl));
            attr.uc_dev_idx = idx;
            attr.en_key_pattern = (8==dsc->key_len) ? AUI_KL_OUTPUT_KEY_PATTERN_64_ODD_EVEN:AUI_KL_OUTPUT_KEY_PATTERN_128_ODD_EVEN;
            attr.en_level = AUI_KL_KEY_TWO_LEVEL;//AUI_KL_KEY_THREE_LEVEL AUI_KL_KEY_TWO_LEVEL AUI_KL_KEY_ONE_LEVEL
            attr.en_root_key_idx = AUI_KL_ROOT_KEY_0_0;
            attr.en_key_ladder_type=AUI_KL_TYPE_ALI;
            rc=aui_kl_open(&attr,&dsc->hdl_kl);
        }
        NGLOGD("dsc=%p dsc->hdl_kl=%p algo=%s",dsc,dsc->hdl_kl,AuiAlgo(dsc->attr.uc_algo));
    }
    switch(dsc->attr.dsc_key_type){
    case AUI_DSC_CONTENT_KEY_KL:{
            UINT sessionKeyLen=16;
            dsc->attr.puc_key=NULL;
            dsc->klcfg.run_level_mode =AUI_KL_RUN_LEVEL_MODE_LEVEL_ALL;
            dsc->klcfg.en_kl_algo = AUI_KL_ALGO_TDES;//AUI_KL_ALGO_TDES; AUI_KL_ALGO_AES
            dsc->klcfg.en_crypt_mode = AUI_KL_DECRYPT;
            dsc->klcfg.en_cw_key_attr = AUI_KL_CW_KEY_ODD_EVEN;
            memcpy(dsc->klcfg.ac_key_val,dsc->pk,16);///*sSessionKey*/,sizeof(sSessionKey));
            switch(dsc->attr.ul_key_pattern){
            case AUI_DSC_KEY_PATTERN_ODD_EVEN:dsc->klcfg.en_cw_key_attr=AUI_KL_CW_KEY_ODD_EVEN;break;
            case AUI_DSC_KEY_PATTERN_EVEN:dsc->klcfg.en_cw_key_attr=AUI_KL_CW_KEY_EVEN;break;
            case AUI_DSC_KEY_PATTERN_ODD:dsc->klcfg.en_cw_key_attr=AUI_KL_CW_KEY_ODD;break;
            }
            memcpy(dsc->klcfg.ac_key_val+sessionKeyLen,dsc->key,sizeof(dsc->key));//uiOddKeyLength+uiEvenKeyLength);
            rc=aui_kl_get(dsc->hdl_kl,AUI_KL_GET_KEY_POS,(void *)&dsc->attr.ul_key_pos);
            NGLOGV("aui_kl_get=%d ul_key_pos=%d",rc,dsc->attr.ul_key_pos);
            rc=aui_kl_get(dsc->hdl_kl,AUI_KL_GET_KEY_SIZE,(void*)&dsc->attr.ul_key_len);
            NGLOGV("aui_kl_get=%d ul_key_len=%d",rc,dsc->attr.ul_key_len);
            rc=aui_kl_gen_key_by_cfg(dsc->hdl_kl,&dsc->klcfg,&dsc->attr.ul_key_pos);
            NGLOGD("aui_kl_gen_key_by_cfg=%d  hdl_kl=%p  ul_key_pos=%d",rc,dsc->hdl_kl,dsc->attr.ul_key_pos);
        }break;
    case AUI_DSC_HOST_KEY_SRAM:
        dsc->attr.puc_key=dsc->key;
        dsc->attr.en_en_de_crypt=AUI_DSC_DECRYPT;//AUI_DSC_ENCRYPT;CSA must be DECRYPT(othrewise aui_dsc_attach_key_info2dsc will return error)

        dsc->attr.uc_mode=AUI_DSC_WORK_MODE_IS_CBC;
        break;
    case AUI_DSC_HOST_KEY_REG:
    case AUI_DSC_CONTENT_KEY_OTP:
        break;
   }
}

static const char*PRINTALGO(int a){
  switch(a){
  case eDSC_ALGO_DVB_CSA:return "DSC_ALGO_DVB_CSA";
  case eDSC_ALGO_AES_128_CBC:return "DSC_ALGO_AES_128_CBC";
  case eDSC_ALGO_DVB_CSA3_STANDARD_MODE:return "DSC_ALGO_DVB_CSA3_STANDARD_MODE";
  case eDSC_ALGO_DVB_CSA3_MINIMALLY_ENHANCED_MODE:return "DSC_ALGO_DVB_CSA3_MINIMALLY_ENHANCED_MODE";
  case eDSC_ALGO_DVB_CSA3_FULLY_ENHANCED_MODE:return "DSC_ALGO_DVB_CSA3_FULLY_ENHANCED_MODE";
  default:return "Unknown Algo";
  }
}

DWORD nglDscSetParameters(DWORD dwStbStreamHandle,const NGLDSC_Param *param )
{
    NGLDSC*dsc=(NGLDSC*)dwStbStreamHandle;
    CHECK(dsc);
    NGLOGD("dsc=%p  algo=%s ivLength=%d",dsc,PRINTALGO(param->algo),param->uiIVLength);
    switch(param->algo){
    case eDSC_ALGO_DVB_CSA:
           dsc->attr.uc_algo = AUI_DSC_ALGO_CSA;
           dsc->attr.csa_version=AUI_DSC_CSA2;
           dsc->attr.uc_mode=AUI_DSC_WORK_MODE_IS_CBC;
           if(NULL!=param->pIV||param->uiIVLength)return E_INVALID_PARA;
           dsc->attr.puc_iv_ctr=NULL;
           break;
    case eDSC_ALGO_AES_128_CBC:
           dsc->attr.uc_algo = AUI_DSC_ALGO_AES;
           dsc->attr.uc_mode=AUI_DSC_WORK_MODE_IS_CBC;//uv_mode only use for AES/DES/TDES
           if(NULL==param->pIV||0==param->uiIVLength)return E_INVALID_PARA;
           dsc->attr.puc_iv_ctr=dsc->iv;

           memcpy(dsc->iv,param->pIV,param->uiIVLength);
           break;
    case eDSC_ALGO_DVB_CSA3_STANDARD_MODE:           dsc->attr.uc_algo = AUI_DSC_ALGO_DES;break;
    case eDSC_ALGO_DVB_CSA3_MINIMALLY_ENHANCED_MODE: dsc->attr.uc_algo = AUI_DSC_ALGO_TDES;break;
    case eDSC_ALGO_DVB_CSA3_FULLY_ENHANCED_MODE:     dsc->attr.uc_algo = AUI_DSC_ALGO_AES;break;
    default:break;
    }
	dsc->hdl=NULL;
	dsc->algo=param->algo;
    OpenHDL(dsc);
    return E_OK;
}

#define MAX_KEY_SIZE 16
static INT VA_DSCR_SCHIP_SetHostKeys(DWORD dwStbDescrHandle,
	UINT16 uiOddKeyLength, const BYTE  *pOddKey,
	UINT16 uiEvenKeyLength, const BYTE  *pEvenKey )
{
	aui_attr_dsc dsc_attr;
	unsigned char key_buffer[MAX_KEY_SIZE * 2] = {0};
        NGLDSC*dsc=(NGLDSC*)dwStbDescrHandle;	
	
	memcpy(key_buffer, pOddKey, uiOddKeyLength);	
	memcpy(key_buffer + uiOddKeyLength, pEvenKey, uiEvenKeyLength);	

	dsc_attr.puc_key = key_buffer;
	dsc_attr.en_residue = AUI_DSC_RESIDUE_BLOCK_IS_AS_ATSC;
	if (dsc->algo == eDSC_ALGO_AES_128_CBC) {
		dsc_attr.uc_mode = AUI_DSC_WORK_MODE_IS_CBC;
		dsc_attr.puc_iv_ctr = dsc->iv;
	}else{
		dsc_attr.csa_version = AUI_DSC_CSA2;
		if(uiEvenKeyLength > MAX_KEY_SIZE/2 || uiOddKeyLength > MAX_KEY_SIZE/2)
			return E_INVALID_PARA;
	}
	
	if(uiEvenKeyLength)
		dsc_attr.ul_key_len = uiEvenKeyLength * 8;
	if(uiOddKeyLength)
		dsc_attr.ul_key_len = uiOddKeyLength * 8;

	dsc_attr.en_en_de_crypt = AUI_DSC_DECRYPT;
	dsc_attr.ul_key_pattern = AUI_DSC_KEY_PATTERN_ODD_EVEN; /*Odd & Even Keys are provided*/
	dsc_attr.dsc_key_type = AUI_DSC_HOST_KEY_SRAM;
	dsc_attr.en_parity = AUI_DSC_PARITY_MODE_AUTO_PARITY_MODE0;	/*The parity is detected from TS packet header*/

	dsc_attr.ul_pid_cnt = dsc->pid_cnt;
	dsc_attr.pus_pids = dsc->pids;


	if(dsc->hdl){
		if(aui_dsc_attach_key_info2dsc(dsc->hdl, &dsc_attr)){
			NGLOGD("va_dscr set key fail\n");
			goto err1;
		}
	}else{
		/*default using CSA algo, for TEST_CASE_3*/
		aui_hdl hdl;

		dsc_attr.uc_dev_idx = 0;
		dsc_attr.dsc_data_type = AUI_DSC_DATA_TS;
		dsc_attr.uc_algo = AUI_DSC_ALGO_CSA;
		dsc_attr.csa_version = AUI_DSC_CSA2;
        dsc->hdl=dscHandles[dsc->algo].hdl;
		if(aui_dsc_attach_key_info2dsc(dsc->hdl, &dsc_attr)){
			NGLOGE("va_dscr set key fail");
			goto err1;
		}
	}
	return E_OK;

err1:
	/*if(aui_dsc_close(dsc->hdl)){
		NGLOGE("dsc close fail");
		return E_ERROR;
	}*/
	return E_ERROR;
}

DWORD nglSchipSetKeys(DWORD dwStbDescrHandle,const BYTE  *pOddKey,UINT32 uiOddKeyLength,
		const BYTE  *pEvenKey,UINT32 uiEvenKeyLength){
	NGLDSC*dsc=(NGLDSC*)dwStbDescrHandle;
	aui_attr_dsc dsc_attr;
	aui_attr_kl kl_attr;
	aui_hdl kl_hdl;
	struct aui_cfg_kl cfg;
    int rc;
	unsigned char key_buffer[MAX_KEY_SIZE * 3] = {0};
    const char*mdname[]={"INACTIVE","SESSION","LOCKED"};
	aui_cfg_kl cfg_kl;
	aui_kl_key_source_attr key_source;
	aui_kl_key_source_attr data_source;

	memcpy(key_buffer, dsc->pk, MAX_KEY_SIZE);
	memcpy(key_buffer + MAX_KEY_SIZE, pOddKey, uiOddKeyLength);
	memcpy(key_buffer + MAX_KEY_SIZE + uiOddKeyLength, pEvenKey, uiEvenKeyLength);
	NGLOG_DUMP("PK & CW:", key_buffer, MAX_KEY_SIZE + 2 * uiOddKeyLength);
	NGLOGD("[%d]Setup 1 contentkey.eChipsetMode: %s dsc.ciphermode=%s",dsc->pid,mdname[pContentKey->eChipsetMode],mdname[dsc->cipherMode]);

	if(pContentKey->eChipsetMode != dsc->cipherMode){
		rc=VA_DSCR_SCHIP_SetHostKeys(dwStbDescrHandle, uiOddKeyLength, pOddKey, uiEvenKeyLength, pEvenKey);
		NGLOGD("va_dscr_schip_sethostkeys rc=%d",rc);
		return rc;
	}

	kl_attr.uc_dev_idx = (dsc-nglDSCS);
	kl_attr.en_level = AUI_KL_KEY_TWO_LEVEL;
	kl_attr.en_root_key_idx = AUI_KL_ROOT_KEY_0_0;	/*0x4d*/
	kl_attr.en_key_ladder_type = AUI_KL_TYPE_ALI;
	
	if(uiOddKeyLength == MAX_KEY_SIZE){
		kl_attr.en_key_pattern = AUI_KL_OUTPUT_KEY_PATTERN_128_ODD_EVEN;
	}else{
		kl_attr.en_key_pattern = AUI_KL_OUTPUT_KEY_PATTERN_64_ODD_EVEN;
	}

	if(!dsc->hdl_kl){
	    if(aui_kl_open(&kl_attr, &kl_hdl)){
	 	NGLOGE("aui_kl_open fail");
		return E_ERROR;
	    }
	    dsc->hdl_kl = kl_hdl;
	}
	kl_hdl = dsc->hdl_kl;

	unsigned long key_dst_pos;
	cfg.run_level_mode = AUI_KL_RUN_LEVEL_MODE_LEVEL_ALL;
	cfg.en_kl_algo = AUI_KL_ALGO_TDES;
	cfg.en_crypt_mode = AUI_KL_DECRYPT;
	cfg.en_cw_key_attr = AUI_KL_CW_KEY_ODD_EVEN;	/*It is used for TS mode*/
	memcpy(cfg.ac_key_val, key_buffer, MAX_KEY_SIZE + 2 * uiOddKeyLength);
	if(aui_kl_gen_key_by_cfg(kl_hdl, &cfg, &key_dst_pos)){
		NGLOGE("aui_kl_gen_key_by_cfg fail");
		goto err2;
	}

	NGLOGD("2 key_dst_pos: %d", key_dst_pos);

	// set KL position to dsc
	dsc_attr.ul_key_pos = key_dst_pos;
	NGLOGD("3 key_dst_pos: %d dsc->algo=%s\n", key_dst_pos,PRINTALGO(dsc->algo));
	dsc_attr.en_residue = AUI_DSC_RESIDUE_BLOCK_IS_AS_ATSC;

	if (dsc->algo == eDSC_ALGO_AES_128_CBC){//eSCRAMBLING_ALGO_AES_128_CBC) {
		NGLOGD("eSCRAMBLING_ALGO_AES_128_CBC");
		dsc_attr.uc_mode = AUI_DSC_WORK_MODE_IS_CBC;
		dsc_attr.puc_iv_ctr = dsc->iv;
	} else{
		NGLOGD("eSCRAMBLING_ALGO_DVB_CSA");
		dsc_attr.csa_version = AUI_DSC_CSA2;
		if(uiEvenKeyLength > MAX_KEY_SIZE/2 || uiOddKeyLength > MAX_KEY_SIZE/2)
			return E_INVALID_PARA;
	}

	dsc_attr.ul_key_len = uiEvenKeyLength * 8;
	dsc_attr.en_en_de_crypt = AUI_DSC_DECRYPT;
	dsc_attr.ul_key_pattern = AUI_DSC_KEY_PATTERN_ODD_EVEN; /*Odd & Even Keys are provided*/
	dsc_attr.dsc_key_type = AUI_DSC_CONTENT_KEY_KL;
	dsc_attr.en_parity = AUI_DSC_PARITY_MODE_AUTO_PARITY_MODE0;	/*The parity is detected from TS packet header*/
	dsc_attr.ul_pid_cnt = dsc->pid_cnt;
	dsc_attr.pus_pids = dsc->pids;

	if(dsc->hdl) {
		if(aui_dsc_attach_key_info2dsc(dsc->hdl, &dsc_attr)){
			NGLOGD("va_dscr set key fail");
			goto err1;
		}
	}else{
		/*default using CSA algo, for TEST_CASE_3*/
		aui_hdl hdl;
		dsc_attr.uc_dev_idx = 0;
		dsc_attr.dsc_data_type = AUI_DSC_DATA_TS;
		dsc_attr.uc_algo = AUI_DSC_ALGO_CSA;
		dsc_attr.csa_version = AUI_DSC_CSA2;
        dsc->hdl=dscHandles[dsc->algo].hdl;
		if(aui_dsc_attach_key_info2dsc(dsc->hdl, &dsc_attr)){
			NGLOGE("va_dscr set key fail");
			goto err1;
		}
	}
	return E_OK;
err2:
	if(aui_kl_close(kl_hdl)){
		NGLOGE("kl close fail");
		return E_ERROR;
	}
err1:
	/*if(aui_dsc_close(dsc->hdl)){
		NGLOGE("dsc close fail");
		return E_ERROR;
	}*/
	return E_ERROR; 
}

DWORD nglDscSetKeys(DWORD dwStbDescrHandle,const BYTE  *pOddKey,UINT32 uiOddKeyLength,
         const BYTE  *pEvenKey,UINT32 uiEvenKeyLength)
{
    int ret;
    BYTE oddkey[16],evenkey[16];
    NGLDSC*dsc=(NGLDSC*)dwStbDescrHandle;
    const char*mdname[]={"INACTIVE","SESSION","LOCKED"};
    CHECK(dsc);
	NGLOGD("dsc[%d]=%p dsc->hdl=%p OddKey=%p/%d EvenKey=%p/%d schip=%d chipsetmode=%s/%s",dsc->pid,dsc,dsc->hdl,pOddKey,uiOddKeyLength,
              pEvenKey,uiEvenKeyLength,dsc->schip_flag,(pContentKey?mdname[pContentKey->eChipsetMode]:"NULL"),mdname[dsc->cipherMode]);
	if(dsc->schip_flag){
		return nglSchipSetKeys(dwStbDescrHandle,pOddKey,uiOddKeyLength,pEvenKey,uiEvenKeyLength);
	}
    if(NULL==pOddKey)uiOddKeyLength=0;
    if(NULL==pEvenKey)uiEvenKeyLength=0;

    if ( (0==uiOddKeyLength+uiEvenKeyLength)||(uiOddKeyLength+uiEvenKeyLength)%8 )
        return E_INVALID_PARA;

    if(8==uiOddKeyLength||8==uiEvenKeyLength) dsc->attr.uc_algo=AUI_DSC_ALGO_CSA;
    if(16==uiOddKeyLength||16==uiEvenKeyLength) dsc->attr.uc_algo=AUI_DSC_ALGO_AES;
    
    switch(dsc->attr.uc_algo){
	case AUI_DSC_ALGO_CSA://eDSC_ALGO_DVB_CSA
			dsc->attr.ul_key_len=8*8;
			dsc->attr.en_en_de_crypt=AUI_DSC_DECRYPT;
            if(uiOddKeyLength>8||uiEvenKeyLength>8){ NGLOGE("CSA.E_INVALID_PARA");return E_INVALID_PARA;}
            dsc->attr.puc_iv_ctr=NULL;
			if(pOddKey){
				memcpy(oddkey, pOddKey, uiOddKeyLength);	
				oddkey[3] = (oddkey[2] + oddkey[1] + oddkey[0])&0xff;
				oddkey[7] = (oddkey[6] + oddkey[5] + oddkey[4])&0xff;
			}
			if(pEvenKey){
				memcpy(evenkey, pEvenKey, uiEvenKeyLength);	
				evenkey[3] = (evenkey[2] + evenkey[1] + evenkey[0])&0xff;
				evenkey[7] = (evenkey[6] + evenkey[5] + evenkey[4])&0xff;
			}
            break;
	case AUI_DSC_ALGO_AES://eDSC_ALGO_AES_128_CBC
			dsc->attr.ul_key_len=16*8;
			dsc->attr.en_en_de_crypt=AUI_DSC_DECRYPT;
            if(uiOddKeyLength>16||uiEvenKeyLength>16){NGLOGE("AES.E_INVALID_PARA");return E_INVALID_PARA;}
            dsc->attr.puc_iv_ctr=dsc->iv;
			if(pOddKey&&uiOddKeyLength)
				memcpy(oddkey,pOddKey,uiOddKeyLength);
			if(pEvenKey&&uiEvenKeyLength)
				memcpy(evenkey , pEvenKey,uiEvenKeyLength );
            break;
    }
    if(NULL!=pOddKey && NULL!=pEvenKey)
         dsc->attr.ul_key_pattern= AUI_DSC_KEY_PATTERN_ODD_EVEN;
    else if(NULL==pOddKey)
         dsc->attr.ul_key_pattern= AUI_DSC_KEY_PATTERN_EVEN;
    else
         dsc->attr.ul_key_pattern= AUI_DSC_KEY_PATTERN_ODD;

    if(pOddKey&&uiOddKeyLength)
		memcpy(dsc->key,oddkey,uiOddKeyLength);

    if(pEvenKey&&uiEvenKeyLength)
		memcpy(dsc->key+uiOddKeyLength,evenkey ,uiEvenKeyLength );
	NGLOG_DUMP("ODDEVEN",dsc->key,uiOddKeyLength+uiEvenKeyLength);
    dsc->attr.en_parity = AUI_DSC_PARITY_MODE_AUTO_PARITY_MODE0;

    OpenHDL(dsc);
	if(dsc->hdl)
		ret=aui_dsc_attach_key_info2dsc(dsc->hdl,&dsc->attr);
	NGLOGD("attach_key_info2dsc=%d dsc=%p KeyLen=%d/%d hdl=%p iv=%p algo=%s",ret,dsc,uiOddKeyLength,uiEvenKeyLength,
			dsc->hdl,dsc->attr.puc_iv_ctr,AuiAlgo(dsc->attr.uc_algo));
	return ret==0?E_OK:E_ERROR;
}

#define CHIPSET_OTP_ADDR (0)
#define KEY_OTP_ADDR	(0x03 * 4)
#define KL_KEY_OTP_SET	(1 << 23)

DWORD nglGetCipherMode(NGLCipherMode*md){
    DWORD data;
    AUI_RTN_CODE ret;
    ret= aui_otp_read(KEY_OTP_ADDR, (unsigned char *)&data, sizeof(data));
	if(AUI_RTN_SUCCESS!=ret)return E_INVALID_PARA;
	if( (data & KL_KEY_OTP_SET) && (pContentKey->eChipsetMode==eCM_LOCKED) )
       *md=eCM_LOCKED;
	else if(pContentKey->eChipsetMode==eCM_INACTIVE)
	      *md=eCM_INACTIVE;
	else *md=eCM_SESSION;
    return E_OK;
}

DWORD nglSetCipherMode(NGLCipherMode md){
    DWORD data;
    AUI_RTN_CODE ret;
	if(NULL==pContentKey)pContentKey = (NGLSCHIP_ContentKey *)malloc(sizeof(NGLSCHIP_ContentKey));

	switch(md){//
	case eCM_INACTIVE:
		ret = aui_otp_read(KEY_OTP_ADDR, (unsigned char *)&data, sizeof(data));
		if (AUI_RTN_SUCCESS!=ret){
		    NGLOGD("otp_read error");
			goto Error;
		}
		if((data&KL_KEY_OTP_SET)==1){
		    NGLOGD("Can't use INACTIVE mode");
		}
		pContentKey->eChipsetMode=eCM_INACTIVE;
		break;
	case eCM_SESSION:
		ret = aui_otp_read(KEY_OTP_ADDR, (unsigned char *)&data, sizeof(data));
		if( (AUI_RTN_SUCCESS!=ret) ){
			NGLOGD("otp_read error");goto Error;
		}
		if( (data&KL_KEY_OTP_SET)==1){
		    NGLOGD("Can't use eSESSION mode");goto Error;
		}
		pContentKey->eChipsetMode=eCM_SESSION;
		break;
	case eCM_LOCKED:
		ret = aui_otp_read(KEY_OTP_ADDR, (unsigned char *)&data, sizeof(data));
		if(AUI_RTN_SUCCESS!=ret){
		    NGLOGD("aui_otp_read error.");
			goto Error;
		}
		data|=KL_KEY_OTP_SET;
		// because once OTP has been written, this chip can't be reversible. 
		//ret = aui_otp_write(KEY_OTP_ADDR, (unsigned char *)&data, sizeof(data));
		pContentKey->eChipsetMode=eCM_LOCKED;
		break;
	default:return E_INVALID_PARA;
	}
	return E_OK;
Error:
	free(pContentKey);
	pContentKey=NULL;
	return E_ERROR;
}

DWORD nglSetCipherSessionKey(const BYTE*pSessionKey,UINT uiSessionKeyLength){
    DWORD index, index2;
    char buff[32];
    NGLDSC *pStbSession,*pStbSession2;
    //NGLDSC*dsc;	
    if(!pSessionKey || uiSessionKeyLength == 0 ||(uiSessionKeyLength % 8)){
	return E_INVALID_PARA;
    }
	if(NULL==pContentKey)pContentKey = (NGLSCHIP_ContentKey *)malloc(sizeof(NGLSCHIP_ContentKey));
	pContentKey->cwFlag = 1;
	pContentKey->pSessionKey = pSessionKey;
	pContentKey->uiSessionKeyLength = uiSessionKeyLength;

    for(index = 0; index < NUM_DSCS; index++ ) {
 	if(nglDSCS[index].pid) {
	    pStbSession =&nglDSCS[index];// (tVA_DSCR_StbSession *)vaDscrHandle[index];
	    for(index2 = 0; index2 < NUM_DSCS; index2++){
		if(index2 == index)
			continue;
		if(nglDSCS[index2].pid){
 		    pStbSession2 =&nglDSCS[index2];// (tVA_DSCR_StbSession *)vaDscrHandle[index2];
		    break;
		}

 		if(index2 == NUM_DSCS - 1)
                    NGLOGE("");return E_ERROR;
	    }
	    break;
        }

        if(index == NUM_DSCS - 1)
  	   return E_OK;	/*For SCHIP_CASE_2*/
    }

    pStbSession->schip_flag = 1;
    pStbSession2->schip_flag = 1;
   	pStbSession->cipherMode = pContentKey->eChipsetMode;
    memcpy(pStbSession->pk, pSessionKey, uiSessionKeyLength);
	pStbSession2->cipherMode= pContentKey->eChipsetMode;
    memcpy(pStbSession2->pk, pSessionKey, uiSessionKeyLength);
    NGLOGD("=index=%d index2=%d sCipherMode=%d",index,index2,pContentKey->eChipsetMode);
	NGLOGD("pSessionKey=%p index=%d/%d index2=%d/%d CiphersetMode=%d",pSessionKey,index,pStbSession->pid,index2,pStbSession2->pid,pContentKey->eChipsetMode);
    NGLOG_DUMP("pSessionKey",pSessionKey,uiSessionKeyLength);
    sprintf(buff,"SessionKey[%d]",pStbSession->pid);
	NGLOG_DUMP(buff,pStbSession->pk,uiSessionKeyLength);
    sprintf(buff,"SessionKey[%d]",pStbSession2->pid);
	NGLOG_DUMP(buff,pStbSession2->pk,uiSessionKeyLength);
    return E_OK;
}

#define CHIPSET_OTP_ADDR (0)
DWORD nglGetChipID(){
     DWORD chipid;
     BYTE ids[8];
     int rc=aui_otp_read(CHIPSET_OTP_ADDR,&chipid,sizeof(DWORD));//LSB
     NGLOGD("aui_otp_read=%d chipid=%08X\r\n",rc,chipid);
     return (rc==AUI_RTN_SUCCESS)?chipid:-1;//0x002013BA;//chipid;
}

