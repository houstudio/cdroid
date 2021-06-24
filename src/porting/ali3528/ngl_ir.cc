#include <ngl_input.h>
#include <aui_input.h>
#include <map>
#include <iostream>
#include <fstream>
#include <cdlog.h>
#include <ngl_msgq.h>
#include <ngl_timer.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <signal.h>
#include <linux/input.h>

typedef struct{
   aui_hdl hdl;
   int epollfd;
   int pipe[2];
}IRDEV;

#define NB_DEV 8

static IRDEV dev={nullptr,0};
static void key_callback(aui_key_info*ki,void*d);

INT nglInputInit(){
    struct epoll_event event;
    if(dev.epollfd>0)
       return E_OK;
    aui_key_init(NULL,NULL);
    aui_log_priority_set(AUI_MODULE_INPUT,AUI_LOG_PRIO_DEBUG);    
    aui_log_priority_set(AUI_MODULE_PANEL,AUI_LOG_PRIO_DEBUG);    
    
    dev.epollfd=epoll_create(8);
    pipe(dev.pipe);
    event.data.fd=dev.pipe[0];
    event.events = EPOLLET | EPOLLIN;
    epoll_ctl(dev.epollfd,EPOLL_CTL_ADD,dev.pipe[0],&event);

    int rc=aui_key_open(0,NULL,&dev.hdl);
    rc= aui_key_set_ir_rep_interval(dev.hdl,100,100);//default is 600 350
    rc=aui_key_callback_register(dev.hdl,key_callback);
}

#define DEVICE_IR 0x1001

static void key_callback(aui_key_info*ki,void*d){
    INPUTEVENT event;
    gettimeofday((struct timeval*)&event,NULL);
    event.type=EV_KEY;
    event.code=ki->n_key_code;
    if(ki->e_status<=2){
       event.device=DEVICE_IR;
       event.value=ki->e_status==1?1:0;//s_status=1:Pressed,2:Released,4:repeated
       write(dev.pipe[1],&event,sizeof(event));
    }
}


INT nglInputGetDeviceInfo(int device,INPUTDEVICEINFO*devinfo){
    switch(device){
    case DEVICE_IR:
         strcpy(devinfo->name,"3528ir");
         devinfo->vendor=INJECTDEV_PTR>>16;
         devinfo->product=INJECTDEV_PTR&0xFF;
         devinfo->source=(1<<EV_KEY)|(1<<EV_SYN);
         break; 
    case INJECTDEV_PTR:
         strcpy(devinfo->name,"Mouse-Inject");
         devinfo->vendor=INJECTDEV_PTR>>16;
         devinfo->product=INJECTDEV_PTR&0xFF;
         devinfo->source=(1<<EV_ABS)|(1<<EV_KEY)|(1<<EV_SYN);
         break;
    case INJECTDEV_KEY:
         strcpy(devinfo->name,"Keyboard-Inject");
         devinfo->vendor=INJECTDEV_KEY>>16;
         devinfo->product=INJECTDEV_KEY&0xFF;
         devinfo->source=(1<<EV_KEY)|(1<<EV_SYN);
         break;
    default:break;
    }
    return 0;
}

INT nglInputGetEvents(INPUTEVENT*outevents,UINT count,DWORD timeout){
    int ret=0;
    struct epoll_event events[32];
    int poll_count=epoll_wait(dev.epollfd, events,count, timeout);
    for(int i=0;i<poll_count;i++){
        if(events[i].events & EPOLLIN){
           INPUTEVENT*e= outevents+ret;
           int rc=read(events[i].data.fd,e,sizeof(INPUTEVENT)*(32-ret));
           LOGV_IF(e->type==EV_KEY,"epollfd=%d fd:%d event:%x,%x,%x rc=%d",dev.epollfd,
                events[i].data.fd, e->type,e->code,e->value,rc);
           ret+=rc/sizeof(INPUTEVENT);
        }
    }
    return ret;
}

INT  nglInputInjectEvents(const INPUTEVENT*events,UINT count,DWORD timeout){
    write(dev.pipe[1],events,count*sizeof(INPUTEVENT));
    return count;
}


