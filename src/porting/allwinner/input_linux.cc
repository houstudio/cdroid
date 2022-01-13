#ifndef HAVE_INPUT_H
#include<linux/input.h>
#else
#define EV_SYN                  0x00
#define EV_KEY                  0x01
#define EV_REL                  0x02
#define EV_ABS                  0x03
#define EV_MSC                  0x04
#define EV_SW                   0x05
#define EV_LED                  0x11
#define EV_SND                  0x12
#define EV_REP                  0x14
#define EV_FF                   0x15
#endif
#ifdef HAVE_POLL_H
#include<poll.h>
#endif
#ifdef HAVE_EPOLL_H
#include<sys/epoll.h>
#endif
#include<cdtypes.h>
#include<cdinput.h>
#include<map>
#include<iostream>
#include<fstream>
#include<vector>
#include<cdlog.h>
#include<ngl_msgq.h>
#include<ngl_timer.h>
#include<string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>

typedef struct{
   int maxfd;
   int nfd;
   int inotify;
   int pipe[2]; 
   int fds[128];
   int source[128];//source type
   std::map<int,int>keymap;
}INPUTDEVICE;

static INPUTDEVICE dev={0,0};
#ifdef HAVE_INPUT_H
static int test_bit(int nr, uint32_t * addr){
    int mask;
    addr += nr >> 5;
    mask = 1 << (nr & 0x1f);
    return ((mask & *addr) != 0);
}

static int getfeatures(int fd){
   BYTE bitmask[EV_CNT];
   static char features[256];
   const char*evs[]={"SYN ","KEY ","REL ","ABS ","MSC ","SW ","LED ","SND ","REP ","FF ","PWR ","FFS ","MAX"};
   int rc;
   int source=0;
   memset(bitmask,0,EV_CNT);
   rc=ioctl(fd, EVIOCGBIT(0, EV_MAX),bitmask);
   features[0]=0;
   LOGD_IF(rc<=0,"EVIOCGBIT %d",rc);
   for(int i=0;i<EV_CNT;i++){
       if(test_bit(i,(unsigned int*)bitmask)&&(i<=5||i>0x10)){
           strcat(features,evs[(i<=5?i:i-10)]);
           source|=(1<<i);
       }
   }
   if(test_bit(ABS_MT_TOUCH_MAJOR,(unsigned int*)bitmask)
       && test_bit(ABS_MT_POSITION_X,(unsigned int*)bitmask)
       && test_bit(ABS_MT_POSITION_Y,(unsigned int*)bitmask)){
      LOGD("fd %d is multitouchdevice!");
   }
   LOGD("fd:%d feature:%s source=%x",fd,features,source);
   return source;
}
#endif
INT InputInit(){
    if(dev.pipe[0]>0)
        return 0;
    pipe(dev.pipe);
    dev.nfd=0;
    dev.fds[dev.nfd++]=dev.pipe[0];
    dev.maxfd=dev.pipe[0];
    int rc=fcntl(dev.pipe[0],F_SETFL,O_NONBLOCK);
    LOGD("cplusplus=%di nfd=%d fcntl=%d fd[0]=%d",__cplusplus,dev.nfd,rc,dev.fds[0]);
#ifdef HAVE_INPUT_H
    struct dirent **namelist=nullptr;
    int nf=scandir("/dev/input",&namelist,[&dev](const struct dirent * ent)->int{
        char fname[256];
        int rc=ent->d_type!=DT_DIR; 
        snprintf(fname,sizeof(fname),"/dev/input/%s",ent->d_name);
        if(rc){
            int fd=open(fname,O_RDWR);//|O_NONBLOCK;
            LOGD("%s fd=%d",fname,fd);
            if(fd>0){
                dev.maxfd=std::max(dev.maxfd,fd);
                dev.fds[dev.nfd]=fd;
                dev.source[dev.nfd++]=getfeatures(fd);
            }
        }
        return rc; 
    },nullptr);
    free(namelist);
    LOGD(".....end nglInputInit maxfd=%d numfd=%d\r\n",dev.maxfd,nf);
#endif
    return 0;
}

INT InputGetDeviceInfo(int device,INPUTDEVICEINFO*devinfo){
    int rc1,rc2;
    memset(devinfo->name,0,sizeof(devinfo->name));
#ifdef HAVE_INPUT_H
    struct input_id id;
    rc1=ioctl(device, EVIOCGNAME(sizeof(devinfo->name) - 1),devinfo->name);
    rc2=ioctl(device, EVIOCGID, &id);
    LOGD_IF(rc2,"fd=%d[%s] rc1=%d,rc2=%d",device,devinfo->name,rc1,rc2);
    devinfo->source=getfeatures(device);
    devinfo->product=id.product;
    devinfo->vendor=id.vendor;
#endif
    switch(device){
    case INJECTDEV_PTR:
         strcpy(devinfo->name,"Mouse-Inject");
         devinfo->vendor=INJECTDEV_PTR>>16;
         devinfo->product=INJECTDEV_PTR&0xFF;
         devinfo->source=(1<<EV_ABS)|(1<<EV_KEY)|(1<<EV_SYN); 
         break;
    case INJECTDEV_KEY:
         strcpy(devinfo->name,"qwerty");
         devinfo->vendor=INJECTDEV_KEY>>16;
         devinfo->product=INJECTDEV_KEY&0xFF;
         devinfo->source=(1<<EV_KEY)|(1<<EV_SYN); 
         break;
    default:break;
    }
    return 0;
}

INT InputInjectEvents(const INPUTEVENT*es,UINT count,DWORD timeout){
    const char*evtnames[]={"SYN","KEY","REL","ABS","MSC","SW"};
    struct timespec tv;
    INPUTEVENT*events=(INPUTEVENT*)malloc(count*sizeof(INPUTEVENT));
    memcpy(events,es,count*sizeof(INPUTEVENT));
    //gettimeofday(&tv,NULL);
    clock_gettime(CLOCK_MONOTONIC,&tv);
    for(int i=0;i<count;i++){
        events[i].tv_sec=tv.tv_sec;
        events[i].tv_usec=tv.tv_nsec/1000;
    }
    if(dev.pipe[1]>0){
       int rc=write(dev.pipe[1],events,count*sizeof(INPUTEVENT));
       LOGV_IF(count&&(es->type<=EV_SW),"pipe=%d %s,%x,%x write=%d",dev.pipe[1],evtnames[es->type],es->code,es->value,rc);
    }
    free(events);
    return count;
}

INT InputGetEvents(INPUTEVENT*outevents,UINT max,DWORD timeout){
    int rc,ret=0;
    struct timeval tv;
#ifdef HAVE_INPUT_H
    struct input_event events[64];
#endif
    INPUTEVENT*e=outevents;
    fd_set rfds;
    static const char*type2name[]={"SYN","KEY","REL","ABS","MSC","SW"};
    tv.tv_usec=(timeout%1000)*1000;//1000L*timeout;
    tv.tv_sec=timeout/1000;
    FD_ZERO(&rfds);
    for(int i=0;i<dev.nfd;i++){
        FD_SET(dev.fds[i],&rfds);
    }
    rc=select(dev.maxfd+1,&rfds,NULL,NULL,&tv);
    if(rc<0){
        LOGD("select error");
        return E_ERROR;
    }
    for(int i=0;i<dev.nfd;i++){
        if(!FD_ISSET(dev.fds[i],&rfds))continue;
#ifdef HAVE_INPUT_H
        if(dev.fds[i]!=dev.pipe[0]){
           rc=read(dev.fds[i],events, (max-ret)*sizeof(struct input_event));
           for(int j=0;j<rc/sizeof(struct input_event);j++,e++){
               *(struct input_event*)e=events[j];
               e->device=dev.fds[i];
               LOGV_IF(e->type<EV_SW,"fd:%d [%s]%x,%x,%x ",dev.fds[i],
                  type2name[e->type],e->type,e->code,e->value);
           }
        }else
#endif
	{//for pipe
           rc=read(dev.fds[i],e, (max-ret)*sizeof(INPUTEVENT));
           e+=rc/sizeof(INPUTEVENT);
        }
        LOGV_IF(rc,"fd %d read %d bytes ispipe=%d",dev.fds[i],rc,dev.fds[i]==dev.pipe[0]);
    }
    return e-outevents;
}

