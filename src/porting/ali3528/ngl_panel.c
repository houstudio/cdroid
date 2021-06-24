#include <ngl_panel.h>
#include <cdlog.h>
#include <aui_panel.h>

static aui_hdl hdl_panel;

DWORD nglFPInit(){
    if(0!=aui_find_dev_by_idx(AUI_MODULE_PANEL,0,&hdl_panel))
        aui_panel_open(0, NULL, &hdl_panel);
    return E_OK;
}

DWORD nglFPShowText(const char*txt,int len){
    nglFPInit();
    int rc=aui_panel_display(hdl_panel, AUI_PANEL_DATA_ANSI,(unsigned char*)txt,len); 
    
    return E_OK;
}

DWORD nglFPSetBrightness(int value){
    int rc=0;//aui_panel_set_led_brightness_level(hdl_panel,value);
    return E_OK;
}
