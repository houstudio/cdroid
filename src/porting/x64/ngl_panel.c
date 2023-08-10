#include<ngl_panel.h>
#include<cdlog.h>

DWORD nglFPInit() {
    return E_OK;
}

DWORD nglFPShowText(const char*txt,int len) {

    return E_OK;
}

DWORD nglFPSetBrightness(int value) {
    int rc=0;//aui_panel_set_led_brightness_level(hdl_panel,value);
    return E_OK;
}
