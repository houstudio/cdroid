#include <Windows.h>
#include <cdlog.h>
#include <cdinput.h>
#include <map>
#include <iostream>
#include <fstream>
#include <vector>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#undef SW_MAX
#include <core/eventcodes.h>
#include <io.h>
#include <fcntl.h>
#define write _write
#define open _open


#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>

typedef struct {
    int maxfd;
    int nfd;
    int inotify;
    HANDLE pipe[2];
    int fds[128];
    std::map<int,int>keymap;
} INPUTDEVICE;

static INPUTDEVICE dev= {0,0};

int32_t InputInit() {
    if(dev.pipe[0]!=0) return 0;
    DWORD flags = PIPE_READMODE_BYTE | PIPE_NOWAIT;
    BOOL rc = CreatePipe(&dev.pipe[0],&dev.pipe[1], NULL, 0);
    LOGI("CreatePipe %p,%p", dev.pipe[0], dev.pipe[1]);
    SetNamedPipeHandleState(dev.pipe[0], &flags, NULL, NULL);
    SetNamedPipeHandleState(dev.pipe[1], &flags, NULL, NULL);
    return 0;
}

#define SET_BIT(array,bit)    ((array)[(bit)/8] |= (1<<((bit)%8)))

int32_t InputGetDeviceInfo(int device,INPUTDEVICEINFO*devinfo) {
    int rc1,rc2;
    memset(devinfo,0,sizeof(INPUTDEVICEINFO));
#if 0
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
  #endif
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
    return 0;
}

int32_t InputInjectEvents(const INPUTEVENT*es,uint32_t count, uint32_t timeout) {
    const char*evtnames[]= {"SYN","KEY","REL","ABS","MSC","SW"};
    INPUTEVENT*events=(INPUTEVENT*)malloc(count*sizeof(INPUTEVENT));
    if(events)memcpy(events,es,count*sizeof(INPUTEVENT));

    if(events && (dev.pipe[1]>0)) {
        int rc=WriteFile(dev.pipe[1],events,count*sizeof(INPUTEVENT),NULL,NULL);
        LOGV_IF(count&&(es->type<=EV_SW),"pipe=%d %s,%x,%x write=%d",dev.pipe[1],evtnames[es->type],es->code,es->value,rc);
    }
    if(events)free(events);
    return count;
}

int32_t InputGetEvents(INPUTEVENT*outevents, uint32_t max, uint32_t timeout) {
    int rc,count=0;
    INPUTEVENT*e=outevents;
    DWORD readedBytes=0;
    ReadFile(dev.pipe[0], outevents, sizeof(INPUTEVENT) * max, &readedBytes, NULL);
    /*for (int i = 0; i<dev.nfd; i++) {
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
    }*/
    return readedBytes/sizeof(INPUTEVENT);
}

