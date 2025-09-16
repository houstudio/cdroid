#include <cdmisc.h>
#include <stdlib.h>
#include <stdio.h>

int SYSInit(){
    return 0;
}

int SYSSuspend(){
    return 0;
}

int SYSGetSerialNo(char*sn,int max_size){
    sprintf(sn,"%llx",0x12345678);
    return 0;
}
