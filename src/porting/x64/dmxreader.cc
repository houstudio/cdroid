#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cdtypes.h>
#include <ngl_os.h>
#include <cdlog.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <dmxreader.h>
#include <iostream>

#define RBUFF_SIZE (188*100)
#define TSPACK_SIZE (188*7)
#define ABS(a) ((a)>0?(a):-(a))

static void UDPRcvProc(void*p) {
    struct sockaddr_in  addr;
    int sock;
    int optval=1;
    UINT pksize=0;
    unsigned char buffer[188*7];
    void**pp=(void**)p;
    ON_TS_RECEIVED tscbk=(ON_TS_RECEIVED)pp[0];
    void*userdata=pp[1];
    pp[0]=NULL;
    sock=socket(AF_INET,SOCK_DGRAM,0);
    setsockopt(sock,SOL_SOCKET,SO_BROADCAST,(char*)&optval,sizeof(int));
    //ioctlsocket(sock,FIONBIO,(u_long*)&optval);
    addr.sin_family=AF_INET;
    addr.sin_port=htons(8315);
    addr.sin_addr.s_addr=INADDR_ANY;
    LOGI("socket=%d",sock);
    if(bind(sock,(struct sockaddr*)&addr,sizeof(addr))<0) {
        LOGE("socket %d bind error",sock);
        return ;
    }
    while(1) {
        int len=recvfrom(sock,buffer,188*7,0,NULL,NULL);
        if(len<0)continue;
        tscbk(buffer,len,userdata);
    }
}
void StartTSReceiver(ON_TS_RECEIVED cbk,void*data) {
    HANDLE tid;
    void*params[2];
    params[0]=(void*)cbk;
    params[1]=data;
    //nglCreateThread(&tid,0,0,TSRcvProc,params);
    nglCreateThread(&tid,0,0,UDPRcvProc,params);
    while(params[0]!=NULL) {
        usleep(10);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////

int get_ts_pid(BYTE*ts) {
    return ((ts[1]&0x1F)<<8)|ts[2];
}

int get_ts_unit_start(BYTE*ts) {
    return (ts[1]&0x40);
}

int get_ts_continue_count(BYTE*ts) {
    return ts[3]&0x0F;
}

int get_ts_payload(BYTE*ts,BYTE*data,int issection) {
    BYTE*pd=ts+4;
    int len=0;
    if(ts[3]&0x20)//adapt_field
        pd+=(*pd)+1;//(issection?1:0);
    if(get_ts_unit_start(ts)&&issection)
        pd+=(*pd)+1;
    if(ts[3]&0x10) {
        len=188-(pd-ts);
        memcpy(data,pd,len);
    }
    return len;
}
int get_section_length(BYTE*sec) {
    return (sec[1]&0x0F)<<8|sec[2];
}
