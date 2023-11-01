#if 1//ndef HAVE_INPUT_H
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
#include <poll.h>
#endif
#ifdef HAVE_EPOLL_H
#include <sys/epoll.h>
#endif
#include <cdtypes.h>
#include <cdinput.h>
#include <map>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <cdlog.h>
#include <ngl_msgq.h>
#include <ngl_timer.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/inotify.h>
typedef struct{
    int fd;
    std::string name;
}DEVICENODE;
typedef struct {
    int maxfd;
    int inotify;
    int pipe[2];
    std::vector<DEVICENODE> fds;
    std::vector<DEVICENODE>::iterator findByFd(int fd){
	for(auto it=fds.begin();it!=fds.end();it++)
	   if(it->fd==fd)return it;
	return fds.end();
    }

    std::vector<DEVICENODE>::iterator findByPath(const std::string&path){
	for(auto it=fds.begin();it!=fds.end();it++)
	   if(it->name==path)return it;
	return fds.end();
    }
} INPUTDEVICE;

static INPUTDEVICE dev= {0,0};
#define WATCHED_PATH "/dev/input"
INT InputInit() {
    if(dev.pipe[0]>0)
        return 0;
    pipe(dev.pipe);
    dev.fds.push_back({dev.pipe[0],"pipe"});
    dev.maxfd = dev.pipe[0];
    int rc = fcntl(dev.pipe[0],F_SETFL,O_NONBLOCK);
    struct dirent **namelist=nullptr;
    LOGD("cplusplus=%di fcntl=%d fd[0]=%d input_event.size=%d %d",__cplusplus,rc,dev.fds[0],sizeof(struct input_event),sizeof(struct timeval));
    int nf=scandir(WATCHED_PATH,&namelist,[&dev](const struct dirent * ent)->int{
        char fname[256];
        int fd = -1;
        snprintf(fname,sizeof(fname),WATCHED_PATH"/%s",ent->d_name);
        if(ent->d_type!=DT_DIR) {
            fd = open(fname,O_RDWR);
            LOGD("%s fd=%d",fname,fd);
            if(fd>0) {
                dev.maxfd=std::max(dev.maxfd,fd);
                dev.fds.push_back({fd,fname});
            }
        }
        return fd>0;
    },nullptr);
    free(namelist);
    dev.inotify = inotify_init();
    dev.maxfd = std::max(dev.maxfd,dev.inotify);
    inotify_add_watch (dev.inotify,WATCHED_PATH,IN_CREATE | IN_DELETE);
    LOGD("maxfd=%d numfd=%d\r\n",dev.maxfd,nf+1);
    return 0;
}

#define SET_BIT(array,bit)    ((array)[(bit)/8] |= (1<<((bit)%8)))

INT InputGetDeviceInfo(int device,INPUTDEVICEINFO*devinfo) {
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
    devinfo->product= id.product;
    devinfo->vendor = id.vendor;
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
        devinfo->vendor = INJECTDEV_TOUCH>>16;
        devinfo->product= INJECTDEV_TOUCH&0xFF;
        SET_BIT(devinfo->absBitMask,ABS_X);
        SET_BIT(devinfo->absBitMask,ABS_Y);
        SET_BIT(devinfo->keyBitMask,BTN_TOUCH);
        break;
    case INJECTDEV_MOUSE:
        strcpy(devinfo->name,"Touch-Inject");
        devinfo->vendor = INJECTDEV_MOUSE>>16;
        devinfo->product= INJECTDEV_MOUSE&0xFF;
        SET_BIT(devinfo->relBitMask,REL_X);
        SET_BIT(devinfo->relBitMask,REL_Y);
        break;
    case INJECTDEV_KEY:
        strcpy(devinfo->name,"qwerty");
        devinfo->vendor = INJECTDEV_KEY>>16;
        devinfo->product= INJECTDEV_KEY&0xFF;
        SET_BIT(devinfo->keyBitMask,BTN_MISC);
        SET_BIT(devinfo->keyBitMask,KEY_OK);
        break;
    default:
        break;
    }
    return 0;
}

INT InputInjectEvents(const INPUTEVENT*es,UINT count,DWORD timeout) {
    const char*evtnames[] = {"SYN","KEY","REL","ABS","MSC","SW"};
    struct timespec tv;
    INPUTEVENT*events = (INPUTEVENT*)malloc(count*sizeof(INPUTEVENT));
    memcpy(events,es,count*sizeof(INPUTEVENT));

    if(dev.pipe[1]>0) {
        int rc = write(dev.pipe[1],events,count*sizeof(INPUTEVENT));
        LOGV_IF(count&&(es->type<=EV_SW),"pipe=%d %s,%x,%x write=%d",dev.pipe[1],evtnames[es->type],es->code,es->value,rc);
    }
    free(events);
    return count;
}

INT InputGetEvents(INPUTEVENT*outevents,UINT max,DWORD timeout) {
    int rc,count = 0,event_pos = 0;
    struct timeval tv;
    struct input_event events[64];
    char inotifyBuffer[512];
    INPUTEVENT*e = outevents;
    fd_set rfds;
    static const char*type2name[]= {"SYN","KEY","REL","ABS","MSC","SW"};
    tv.tv_usec= (timeout%1000)*1000;//1000L*timeout;
    tv.tv_sec = timeout/1000;
    FD_ZERO(&rfds);
    for(int i = 0; i < dev.fds.size(); i++) {
        FD_SET(dev.fds[i].fd,&rfds);
    }
    if((rc = select(dev.maxfd+1,&rfds,NULL,NULL,&tv))<0) {
        LOGD("select error");
        return E_ERROR;
    }
    std::vector<DEVICENODE> fds = dev.fds;
    for(int i=0; i < fds.size(); i++) {
        if(!FD_ISSET(dev.fds[i].fd,&rfds))continue;
         if(dev.fds[i].fd == dev.pipe[0]){ //for pipe
            rc = read(dev.fds[i].fd,e, (max-count)*sizeof(INPUTEVENT));
            e += rc/sizeof(INPUTEVENT);
            count += rc/sizeof(INPUTEVENT);
        }else if(dev.fds[i].fd == dev.inotify){
            struct inotify_event*ievent=(struct inotify_event*)inotifyBuffer;
            rc = read(dev.inotify,inotifyBuffer,sizeof(inotifyBuffer));
            if(rc<sizeof(struct inotify_event))
                continue;
             while(rc>=sizeof(struct inotify_event)){
                ievent = (struct inotify_event*)(inotifyBuffer+event_pos);
                std::string path = WATCHED_PATH;
                path.append("/").append(ievent->name);
                LOGI("device %s:%d %s",path.c_str(),ievent->wd,((ievent->mask&IN_CREATE)?"added":"removed"));
                const int eventsize = sizeof(struct inotify_event)+ievent->len;
                rc -= eventsize;
                event_pos += eventsize;
                e->type = (ievent->mask & IN_CREATE) ? EV_ADD : EV_REMOVE;
                if(ievent->mask & IN_DELETE){
                    auto it = dev.findByPath(path);
                    if(it!=dev.fds.end())dev.fds.erase(it);
                    e->device = it->fd;
                }else{
                    e->device = open(path.c_str(),O_RDWR);
                    dev.fds.push_back({e->device,path});
                    LOGE_IF(e->device<0,"%s open failed",path.c_str());
                }
                e++;
            }
        }else {/*for input devices*/
            rc = read(dev.fds[i].fd,events, sizeof(events)/sizeof(struct input_event));
            for(int j=0; j<rc/sizeof(struct input_event)&&(count<max); j++,e++,count++) {
                e->tv_sec = events[j].time.tv_sec;
                e->tv_usec= events[j].time.tv_usec;
                e->type = events[j].type;
                e->code = events[j].code;
                e->value= events[j].value;
                e->device = dev.fds[i].fd;
                LOGV_IF(e->type<EV_SW,"fd:%d [%s]%02x,%02x,%02x time=%ld.%06ld time2=%ld.%ld",dev.fds[i].fd,
                     type2name[e->type],e->type,e->code,e->value,e->tv_sec,e->tv_usec,events[j].time.tv_sec,events[j].time.tv_usec);
            }
        }
        LOGV_IF(rc,"fd %d read %d bytes ispipe=%d",dev.fds[i],rc,dev.fds[i].fd==dev.pipe[0]);
    }
    return e-outevents;
}

