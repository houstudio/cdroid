#include<ngl_nvm.h>
#include <aui_flash.h>

DWORD NGLOpenFlash(DWORD *handle)
{
    aui_flash_open_param open_param;
    aui_hdl flash_handle = 0;
    open_param.flash_id =0;//?
    open_param.flash_type =AUI_FLASH_TYPE_NAND;
    AUI_RTN_CODE err = aui_flash_open(&open_param, &flash_handle);
    *handle=(DWORD)flash_handle;
}
DWORD NGLCloseFlash(DWORD handle)
{
    aui_flash_close((aui_hdl)handle);
}
DWORD NGLWriteFlash(DWORD handle,DWORD dwStartAddr, BYTE *pData, DWORD lDataLen)
{
    INT32 writed;
    aui_flash_write((aui_hdl)handle,dwStartAddr,lDataLen,&writed,pData);
    return writed;
}

DWORD NGLReadFlash(DWORD handle,DWORD dwStartAddr, BYTE *pData, DWORD lDataLen)
{
    INT32 readed;
    aui_flash_read((aui_hdl)handle,dwStartAddr,lDataLen,&readed,pData);
    return readed;
}
