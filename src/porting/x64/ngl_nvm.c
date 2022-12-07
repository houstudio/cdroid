#include<ngl_nvm.h>
//#include <aui_flash.h>

DWORD NGLOpenFlash(DWORD *handle)
{
    *handle=0;
}
DWORD NGLCloseFlash(DWORD handle)
{
}
DWORD NGLWriteFlash(DWORD handle,DWORD dwStartAddr, BYTE *pData, DWORD lDataLen)
{
    INT32 writed=lDataLen;
    return writed;
}

DWORD NGLReadFlash(DWORD handle,DWORD dwStartAddr, BYTE *pData, DWORD lDataLen)
{
    INT32 readed=lDataLen;
    return readed;
}
