#include<aui_dis.h>
#include<aui_osd.h>
#include<cdtypes.h>
#include<cdlog.h>
#include<ngl_disp.h>

static aui_hdl dis_hd=NULL;
static aui_hdl dis_sd=NULL;

int32_t DispInit(){
    int rc;
    aui_attr_dis attr_dis;
    MEMSET(&attr_dis, 0 ,sizeof(aui_attr_dis));

    if (aui_find_dev_by_idx(AUI_MODULE_DIS, AUI_DIS_HD, &dis_hd)){
        attr_dis.uc_dev_idx = AUI_DIS_HD;
	    rc=aui_dis_open(&attr_dis, &dis_hd);
	    LOGD("aui_dis_open HD =%d",rc);
    }

    LOGD("init_dis_handle(),222");	
	
    if (aui_find_dev_by_idx(AUI_MODULE_DIS, AUI_DIS_SD, &dis_sd)){
	    attr_dis.uc_dev_idx = AUI_DIS_SD;
	    rc=aui_dis_open(&attr_dis, &dis_sd);
   	    LOGD(" aui_dis_open SD =%d",rc);
    }
    LOGD("dis_hd=%p dis_sd=%p",dis_hd,dis_sd);
    return E_OK;
}

int32_t DispSetResolution(int dev,int res){
    int rc;
    unsigned int tve_src=0;
    unsigned long output_width = 1280;
    unsigned long output_height = 720;
    aui_scale_param osd_param;
    BOOL progressive=FALSE;
    aui_hdl layer_hd= NULL;
    enum aui_dis_tvsys  tvsys = AUI_DIS_TVSYS_LINE_720_25;
    switch(res){
    case DISP_RES_1080I:
    case DISP_RES_1080P:
      	tvsys = (res==DISP_RES_1080I)?AUI_DIS_TVSYS_LINE_1080_25:AUI_DIS_TVSYS_LINE_1080_50;
	    output_width = 1920;
	    output_height = 1080;
	    progressive =res==DISP_RES_1080P;
        break;
    case DISP_RES_720I:
    case DISP_RES_720P:
        tvsys = AUI_DIS_TVSYS_LINE_720_50;
	    output_width = 1280;
	    output_height = 720;
	    progressive = res==DISP_RES_720P;
        break;
    case DISP_RES_576I:
    case DISP_RES_576P:
     	tvsys = AUI_DIS_TVSYS_PAL;
	    output_width = 720;
	    output_height = 576;
	    progressive = true;
    case DISP_RES_480I:
        tvsys = AUI_DIS_TVSYS_NTSC;
        output_width=720;
        output_height=480;
        progressive =FALSE;
    default:break;
    }
    
    unsigned int attrs[4];
    memset(&osd_param,0,sizeof(aui_scale_param));
    aui_dis_dac_unreg(dis_sd, AUI_DIS_TYPE_UNREG_CVBS);

    aui_dis_dac_unreg(dis_hd, AUI_DIS_TYPE_UNREG_YUV);
    aui_dis_dac_unreg(dis_hd, AUI_DIS_TYPE_UNREG_RGB);
    aui_dis_dac_unreg(dis_hd, AUI_DIS_TYPE_UNREG_SVIDEO);
    aui_dis_dac_unreg(dis_hd, AUI_DIS_TYPE_UNREG_CVBS);
    aui_dis_dac_unreg(dis_sd, AUI_DIS_TYPE_UNREG_CVBS);

    rc = aui_dis_tv_system_set(dis_hd, tvsys, progressive);
    LOGD("aui_dis_tv_system_set=%d ",rc);

    osd_param.input_width = 1280;
    osd_param.output_width = output_width;
    osd_param.input_height = 720;
    osd_param.output_height = output_height;
    if(0!=aui_find_dev_by_idx(AUI_MODULE_GFX, AUI_OSD_LAYER_GMA0, &layer_hd))
        aui_gfx_layer_open(AUI_OSD_LAYER_GMA0,&layer_hd);
    rc= aui_gfx_layer_scale(layer_hd,&osd_param);
    LOGD("aui_dis_tv_system_set ret = %d",rc); 
	
    /** step 3, set HD TV encoder signal source**/
    /** HD TV encoder will be attached to DEN**/
    tve_src = AUI_DIS_HD; 
    rc =aui_dis_set(dis_hd, AUI_DIS_SET_TVESDHD_SOURCE, &tve_src);
		
    /** step 4 */
    /** in our sd products,there is only one tv encoder,cvbs should only register on this tv encoder **/
    /** register cvbs to the hd tv encoder **/

    /** register YUV to the hd tv encoder **/
    attrs[0] = AUI_DIS_TYPE_V;
    attrs[1] = AUI_DIS_TYPE_U;
    attrs[2] = AUI_DIS_TYPE_Y;
    attrs[3] = AUI_DIS_TYPE_NONE;
    aui_dis_dac_reg(dis_hd, attrs, 4);
			
    /** register cvbs to the sd tv encoder **/
    attrs[0] = AUI_DIS_TYPE_NONE;
    attrs[1] = AUI_DIS_TYPE_NONE;
    attrs[2] = AUI_DIS_TYPE_NONE;
    attrs[3] = AUI_DIS_TYPE_CVBS;
    rc=aui_dis_dac_reg(dis_sd, attrs, 4);
    LOGD("aui_dis_dac_reg=%d",rc);
    return E_OK;
}

int32_t DispSetAspectRatio(int dev,int ratio){
    int rc;
    switch(ratio){
    case DISP_APR_AUTO:   ratio=AUI_DIS_AP_AUTO;break;
    case DISP_APR_4_3:    ratio=AUI_DIS_AP_4_3;break;
    case DISP_APR_16_9:   ratio=AUI_DIS_AP_16_9;break;
    default:LOGD("Invalid value %d",ratio);return E_ERROR;
    }    
    rc=aui_dis_aspect_ratio_set(dis_hd,ratio);
    LOGD("ratio=%d rc=%d",ratio,rc);
    return rc;
}

int32_t DispGetAspectRatio(int dev,int*ratio){
    struct aui_dis_info dis_info;
    memset(&dis_info,0,sizeof(aui_dis_info));
    aui_dis_get((void*)dis_hd,AUI_DIS_GET_INFO,(void*)&dis_info);
    switch(dis_info.dis_aspect_ratio){
    case AUI_DIS_AP_AUTO: *ratio=DISP_APR_AUTO; break;
    case AUI_DIS_AP_4_3:  *ratio=DISP_APR_4_3;  break;
    case AUI_DIS_AP_16_9: *ratio= DISP_APR_16_9;break;
    default:return E_ERROR;
    }
    return E_OK;
}

int32_t DispSetMatchMode(int dev,int md){
    int rc;
    aui_dis_match_mode mm;
    switch(md){
    case DISP_MM_PANSCAN : mm=AUI_DIS_MM_PANSCAN;break;
    case DISP_MM_LETTERBOX:mm=AUI_DIS_MM_LETTERBOX;break;
    case DISP_MM_PILLBOX : mm=AUI_DIS_MM_PILLBOX;break;
    case DISP_MM_NORMAL_SCALE:mm=AUI_DIS_MM_NORMAL_SCALE;break;
    case DISP_COMBINED_SCALE:mm=AUI_DIS_MM_COMBINED_SCALE;break;
    return E_ERROR;
    }
    rc=aui_dis_match_mode_set(dis_hd,mm);
    LOGD("md=%d rc=%d",md,rc);
    return rc;
}

int32_t DispSetBrightNess(int dev,int value){
    int rc;
    aui_dis_enhance_set(dis_sd,AUI_DIS_ENHANCE_BRIGHTNESS,value);
    rc=aui_dis_enhance_set(dis_hd,AUI_DIS_ENHANCE_BRIGHTNESS,value);
    LOGD("rc=%d value=%d",rc,value);
}

int32_t DispSetContrast(int dev,int value){
    int rc;
    aui_dis_enhance_set(dis_sd,AUI_DIS_ENHANCE_CONTRAST,value);
    rc = aui_dis_enhance_set(dis_hd,AUI_DIS_ENHANCE_CONTRAST,value);
    LOGD("rc=%d value=%d",rc,value);
}

int32_t DispSetSaturation(int dev,int value){
    int rc;
    aui_dis_enhance_set(dis_sd,AUI_DIS_ENHANCE_SATURATION,value);
    rc = aui_dis_enhance_set(dis_hd,AUI_DIS_ENHANCE_SATURATION,value);
    LOGD("rc=%d value=%d ",value,rc);
}

int32_t DispSetColorTemp(int dev,const DISP_ColorTemperature*){
    return 0;
}

int32_t DispGetColorTemp(int dev,DISP_ColorTemperature*){
    return 0;
}

/*DispSetGamma
 *
 * entryNumber Gamma Entry number,-1(0xFFFFFFFF):disable gamma
 * */
int32_t DispSetGamma(int dev,uint8_t*r,uint8_t*g,uint8_t*b,uint32_t entryNumber){
    return 0;
}

int32_t DispGetGamma(int dev,uint8_t*r,uint8_t*g,uint8_t*b,uint32_t *entryNumber){
    return 0;
}
