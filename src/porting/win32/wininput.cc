#ifdef HAVE_INPUT_H
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
#include<cdlog.h>
#include<cdinput.h>
#include<map>
#include<iostream>
#include<fstream>
#include<vector>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifndef _WIN32
#include <dirent.h>
#include <unistd.h>
#include <poll.h>
#else
#include <io.h>
#include <fcntl.h>
#define write _write
#define open _open
#endif

#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>

typedef struct {
    int maxfd;
    int nfd;
    int inotify;
    int pipe[2];
    int fds[128];
    std::map<int,int>keymap;
} INPUTDEVICE;

static INPUTDEVICE dev= {0,0};

INT InputInit() {
    if(dev.pipe[0]>0)
        return 0;
#if 0
    pipe(dev.pipe);
    dev.nfd=0;
    dev.fds[dev.nfd++]=dev.pipe[0];
    dev.maxfd=dev.pipe[0];
    int rc=fcntl(dev.pipe[0],F_SETFL,O_NONBLOCK);
    struct dirent **namelist=nullptr;
    LOGD("cplusplus=%di nfd=%d fcntl=%d fd[0]=%d input_event.size=%d %d",__cplusplus,dev.nfd,rc,dev.fds[0],sizeof(struct input_event),sizeof(struct timeval));
    int nf=scandir("/dev/input",&namelist,[&dev](const struct dirent * ent)->int{
        char fname[256];
        int fd=-1;
        snprintf(fname,sizeof(fname),"/dev/input/%s",ent->d_name);
        if(ent->d_type!=DT_DIR) {
            fd=open(fname,O_RDWR);
            LOGD("%s fd=%d",fname,fd);
            if(fd>0) {
                dev.maxfd=std::max(dev.maxfd,fd);
                dev.fds[dev.nfd++]=fd;
            }
        }
        return fd>0;
    },nullptr);
    free(namelist);
    LOGD("maxfd=%d numfd=%d\r\n",dev.maxfd,nf+1);
#endif
    return 0;
}

#define SET_BIT(array,bit)    ((array)[(bit)/8] |= (1<<((bit)%8)))

INT InputGetDeviceInfo(int device,INPUTDEVICEINFO*devinfo) {
#if 0
    int rc1,rc2;
    memset(devinfo,0,sizeof(INPUTDEVICEINFO));

    struct input_id id;
    rc1=ioctl(device, EVIOCGNAME(sizeof(devinfo->name) - 1),devinfo->name);
    rc2=ioctl(device, EVIOCGID, &id);

    for(int i=0,j=0; i<ABS_CNT; i++) {
        struct input_absinfo info;
        if((0==ioctl(device, EVIOCGABS(i),&info))&&(info.minimum!=info.maximum)) {
            INPUTAXISINFO*a = devinfo->axis+j++;
            a->axis   = i;
            a->fuzz   = info.fuzz;
            a->flat   = info.flat;
            a->minimum= info.minimum;
            a->maximum= info.maximum;
            a->resolution=info.resolution;
            LOGI_IF(a->maximum-a->minimum||1,"dev %d axis[%d]=[%d,%d,%d]",device,a->axis, a->minimum,a->maximum,a->resolution);
        }
    }
    devinfo->product=id.product;
    devinfo->vendor=id.vendor;
    ioctl(device, EVIOCGBIT(EV_KEY, sizeof(devinfo->keyBitMask)), devinfo->keyBitMask);
    ioctl(device, EVIOCGBIT(EV_ABS, sizeof(devinfo->absBitMask)), devinfo->absBitMask);
    ioctl(device, EVIOCGBIT(EV_REL, sizeof(devinfo->relBitMask)), devinfo->relBitMask);
    ioctl(device, EVIOCGBIT(EV_SW, sizeof(devinfo->swBitMask)), devinfo->swBitMask);
    ioctl(device, EVIOCGBIT(EV_LED, sizeof(devinfo->ledBitMask)), devinfo->ledBitMask);
    ioctl(device, EVIOCGBIT(EV_FF, sizeof(devinfo->ffBitMask)), devinfo->ffBitMask);
    ioctl(device, EVIOCGPROP(sizeof(devinfo->propBitMask)), devinfo->propBitMask);
    switch(device) {
    case INJECTDEV_TOUCH:
        strcpy(devinfo->name,"Touch-Inject");
        devinfo->vendor=INJECTDEV_TOUCH>>16;
        devinfo->product=INJECTDEV_TOUCH&0xFF;
        SET_BIT(devinfo->absBitMask,ABS_X);
        SET_BIT(devinfo->absBitMask,ABS_Y);
        SET_BIT(devinfo->keyBitMask,BTN_TOUCH);
        break;
    case INJECTDEV_MOUSE:
        strcpy(devinfo->name,"Touch-Inject");
        devinfo->vendor=INJECTDEV_MOUSE>>16;
        devinfo->product=INJECTDEV_MOUSE&0xFF;
        SET_BIT(devinfo->relBitMask,REL_X);
        SET_BIT(devinfo->relBitMask,REL_Y);
        break;
    case INJECTDEV_KEY:
        strcpy(devinfo->name,"qwerty");
        devinfo->vendor=INJECTDEV_KEY>>16;
        devinfo->product=INJECTDEV_KEY&0xFF;
        SET_BIT(devinfo->keyBitMask,BTN_MISC);
        SET_BIT(devinfo->keyBitMask,KEY_OK);
        break;
    default:
        break;
    }
#endif
    return 0;
}

INT InputInjectEvents(const INPUTEVENT*es,UINT count,DWORD timeout) {
    const char*evtnames[]= {"SYN","KEY","REL","ABS","MSC","SW"};
    INPUTEVENT*events=(INPUTEVENT*)malloc(count*sizeof(INPUTEVENT));
    if(events)memcpy(events,es,count*sizeof(INPUTEVENT));

    if(events && (dev.pipe[1]>0)) {
        int rc=write(dev.pipe[1],events,count*sizeof(INPUTEVENT));
        LOGV_IF(count&&(es->type<=EV_SW),"pipe=%d %s,%x,%x write=%d",dev.pipe[1],evtnames[es->type],es->code,es->value,rc);
    }
    if(events)free(events);
    return count;
}

INT InputGetEvents(INPUTEVENT*outevents,UINT max,DWORD timeout) {
#if 0
    int rc,count=0;
    struct timeval tv;

    struct input_event events[64];
    INPUTEVENT*e=outevents;
    fd_set rfds;
    static const char*type2name[]= {"SYN","KEY","REL","ABS","MSC","SW"};
    tv.tv_usec=(timeout%1000)*1000;//1000L*timeout;
    tv.tv_sec=timeout/1000;
    FD_ZERO(&rfds);
    for(int i=0; i<dev.nfd; i++) {
        FD_SET(dev.fds[i],&rfds);
    }
    rc=select(dev.maxfd+1,&rfds,NULL,NULL,&tv);
    if(rc<0) {
        LOGD("select error");
        return E_ERROR;
    }
    for(int i=0; i<dev.nfd; i++) {
        if(!FD_ISSET(dev.fds[i],&rfds))continue;
        if(dev.fds[i]!=dev.pipe[0]) {
            rc=read(dev.fds[i],events, sizeof(events)/sizeof(struct input_event));
            for(int j=0; j<rc/sizeof(struct input_event)&&(count<max); j++,e++,count++) {
                e->tv_sec = events[j].time.tv_sec;
                e->tv_usec= events[j].time.tv_usec;
                e->type = events[j].type;
                e->code = events[j].code;
                e->value= events[j].value;
                e->device=dev.fds[i];
                LOGV_IF(e->type<EV_SW,"fd:%d [%s]%02x,%02x,%02x time=%ld.%ld time2=%ld.%ld",dev.fds[i],
                        type2name[e->type],e->type,e->code,e->value,e->tv_sec,e->tv_usec,events[j].time.tv_sec,events[j].time.tv_usec);
            }
        } else { //for pipe
            rc=read(dev.fds[i],e, (max-count)*sizeof(INPUTEVENT));
            e+=rc/sizeof(INPUTEVENT);
            count+=rc/sizeof(INPUTEVENT);
        }
        LOGV_IF(rc,"fd %d read %d bytes ispipe=%d",dev.fds[i],rc,dev.fds[i]==dev.pipe[0]);
    }
    return e-outevents;
#endif
    return 0;
}

