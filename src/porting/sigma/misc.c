#include <cdmisc.h>
#include "mi_common.h"
#include "mi_sys.h"
#include "mi_gfx.h"
#include <stdlib.h>

int SYSInit(){
    return 0;
}

int SYSSuspend(){
    int ret1,ret2;
    ret1=MI_GFX_Close();
    ret2=MI_SYS_Exit();
    printf("prepared to suspend gfxclose=%d sysexit=%d\r\n",ret1,ret2);
    system("/customer/suspend.sh");
    ret1=MI_SYS_Init();
    ret2=MI_GFX_Open();
    printf("suspend end,we are wake up now. sysinit=%d,gfxopen=%d\r\nd",ret1,ret2);
    return ret1+ret1;
}

int SYSGetSerialNo(char*sn,int max_size){
    MI_U64 u64Uuid;
    MI_S32 s32Ret = MI_ERR_SYS_FAILED;
    s32Ret = MI_SYS_ReadUuid (&u64Uuid);
    sprintf(sn,"%llx",u64Uuid);
    return s32Ret;
}
