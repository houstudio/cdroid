/*
  FILE : stub_dmx.c
  PURPOSE: This file is a stub for linking tests.
*/
#include <stdio.h>
#include <ngl_dmx.h>
#include <string.h>
#include <stdlib.h>
#define MASK_LEN 16
#define MAX_CHANNEL 64
#define MAX_FILTER  128
#define UNUSED_PID -1
#include <cdlog.h>
#include <ngl_os.h>
#include <dmxreader.h>
#include <mutex>

#define CHECKDMX(x) {if((x)!=AUI_RTN_SUCCESS)LOGV("%s:%d %s=%d\n",__FUNCTION__,__LINE__,#x,(x));}
typedef struct {
    int pid;
    int type;
    int num_filt;
    int num_started;
    int started;
    int used;
    int cc;
    int pos;
    BYTE *section;
} DMXCHANNEL;

#define TSBUF_SIZE 188*7

typedef struct {
    DMXCHANNEL*ch;
    FILTER_NOTIFY CallBack;
    void*userdata;
    int started;
    BYTE mask[MASK_LEN];
    BYTE value[MASK_LEN];
    BYTE reverse[MASK_LEN];
    BYTE masklen;
    BYTE*tsBuffer;
} NGLDMXFILTER;

static std::recursive_mutex mtx_dmx;
static DMXCHANNEL Channels[MAX_CHANNEL];
static NGLDMXFILTER  Filters[MAX_FILTER];
static DMXCHANNEL*GetChannel(int pid) {
    int i;
    for(i=0; i<MAX_CHANNEL; i++) {
        if(Channels[i].pid==pid)
            return Channels+i;
    }
    return NULL;
}
static DMXCHANNEL*GetFreeChannel() {
    int i;
    for(i=0; i<MAX_CHANNEL; i++)
        if(0==Channels[i].used) {
            Channels[i].used=1;
            return Channels+i;
        }
    return NULL;
}
static NGLDMXFILTER*GetIdleFilter() {
    int i;
    for(i=0; i<MAX_FILTER; i++) {
        if(Filters[i].ch==NULL)
            return Filters+i;
    }
    return NULL;
}

#define CHECKFILTER(flt) {if((flt<Filters)||(flt>=&Filters[MAX_FILTER]))return E_INVALID_PARA;}
typedef void(*ON_TS_RECEIVED)(BYTE*tsdata,int len,void*userdata);
void StartTSReceiver(ON_TS_RECEIVED cbk,void*data);

static int PatternMatch(NGLDMXFILTER*f,BYTE*data) {
    int i;
    for(i=0; i<f->masklen; i++) {
        if((data[i]&f->mask[i])!=(f->value[i]&f->mask[i]))
            return 0;
    }
    return 1;
}
static void OnTSArrived(BYTE*tsdata,int len,void*p) {
    BYTE*pts=tsdata;
    for(pts=tsdata; pts-tsdata<len; pts+=188) {
        int pid=get_ts_pid(pts);
        int i,cc=get_ts_continue_count(pts);
        LOGD_IF(*pts!=0x47,"invalid data sync=0x%x",*pts);
        //nglLockMutex(mtx_dmx);//dead lock???
        DMXCHANNEL*ch=GetChannel(pid);
        if(ch==NULL||ch->started==0)goto unlock;
        if( (ch->type==DMX_TS) && (ch->pid==pid) ) {
            for(i=0; i<MAX_FILTER; i++) {
                NGLDMXFILTER*f=Filters+i;
                if((f->ch!=ch)||(f->started==0)||(f->CallBack==NULL))continue;
                f->CallBack(f,pts,188,f->userdata);
            }
            goto unlock;
        }
        if(get_ts_unit_start(pts)) {
            if(ch->type==DMX_SECTION) {
                memcpy(ch->section+ch->pos,pts+5,pts[4]);
                ch->pos+=pts[4];
            } else if(ch->type==DMX_PES) {
                //memcpy(ch->section+ch->pos,pts+4,184);
                //ch->pos+=184;
            }
            for(i=0; i<MAX_FILTER&&ch->pos>4; i++) {
                NGLDMXFILTER*f=Filters+i;
                if((f->ch!=ch)||(f->started==0)||(f->CallBack==NULL))continue;
                if(PatternMatch(f,ch->section))
                    f->CallBack(f,ch->section,ch->pos,f->userdata);
            }
            if(ch->type==DMX_SECTION) {
                ch->pos=188-5-pts[4];
                memcpy(ch->section,pts+5+pts[4],ch->pos);
            } else if(ch->type==DMX_PES) {
                ch->pos=184;
                memcpy(ch->section,pts+4,184);
            }
        } else {
            if((ch->cc+1)%16==cc ) {
                ch->pos+=get_ts_payload(pts,ch->section+ch->pos,(ch->type==DMX_SECTION));
            } else if(cc==ch->cc) {
                LOGV("duplicate CC for pid %d",pid);
                continue;
            } else {
                LOGV("continue counter interrupt pid=%d CC:%d-->%d",pid,ch->cc,cc);
                ch->pos=0;
            }
        }
        ch->cc=cc;
unlock:;//nglUnlockMutex(mtx_dmx);
    }
}
DWORD nglDmxInit() {
    int i;
    DWORD thid;
    static int sDMX_INITED=0;
    if(sDMX_INITED)return 0;
    sDMX_INITED++;
    LOGV("nglDmxInit tscbk=%p",OnTSArrived);

    for(i=0; i<MAX_CHANNEL; i++) {
        //Channels[i].dmx=NULL;
        //Channels[i].channel=NULL;
        Channels[i].pid=UNUSED_PID;
        Channels[i].num_filt=0;
        Channels[i].pos=-1;
        Channels[i].used=0;
        Channels[i].section=(BYTE*)malloc(64*1024);
    }
    StartTSReceiver(OnTSArrived,NULL);
    return 0;
}

DWORD nglDmxUninit() {
    int i;
    for(i=0; i<MAX_CHANNEL; i++) {
        //Channels[i].dmx=NULL;
        //Channels[i].channel=NULL;
        Channels[i].pid=UNUSED_PID;
        Channels[i].num_filt=0;
        Channels[i].pos=-1;
        Channels[i].used=0;
        free(Channels[i].section);
    }
    return 0;
}
HANDLE nglAllocateSectionFilter(INT dmx_id,WORD  wPid,FILTER_NOTIFY cbk,void*userdata,DMX_TYPE dmxtype) {
    int rc;
    std::unique_lock<std::recursive_mutex>lck(mtx_dmx);
    DMXCHANNEL*ch=GetChannel(wPid);
    if(cbk==NULL||wPid>=0x1FFF||(ch&&ch->type!=dmxtype))return NULL;
    if(ch==NULL) {
        ch=GetFreeChannel();
        ch->pid=wPid;
        ch->type=dmxtype;
        ch->num_filt=0;
        ch->pos=0;
        memset(ch->section,0xFF,8);
    }
    NGLDMXFILTER*flt=GetIdleFilter();
    flt->ch=ch;
    flt->started=0;
    ch->num_filt++;
    flt->CallBack=cbk;
    flt->userdata=userdata;
    LOGV("flt %p pid=%d ch=%p",flt,wPid,ch);
    return (HANDLE)flt;
}

INT nglGetFilterPid( HANDLE dwStbFilterHandle) {
    NGLDMXFILTER*flt=(NGLDMXFILTER*)dwStbFilterHandle;
    return (flt&&flt->ch)?flt->ch->pid:-1;
}

INT nglFreeSectionFilter( HANDLE dwStbFilterHandle ) {
    std::unique_lock<std::recursive_mutex>lck(mtx_dmx);
    NGLDMXFILTER*flt=(NGLDMXFILTER*)dwStbFilterHandle;
    CHECKFILTER(flt);
    nglStopSectionFilter(dwStbFilterHandle);
    if(flt->ch) {
        flt->ch->num_filt--;
        if(flt->ch->num_filt==0)
            flt->ch->used=0;
        flt->ch=NULL;
    } else {
        return E_ERROR;
    }
    return E_OK;
}

INT nglSetSectionFilterParameters( HANDLE dwStbFilterHandle, BYTE *pMask, BYTE *pValue,UINT uiLength) {
    BYTE reverse[16];
    std::unique_lock<std::recursive_mutex>lck(mtx_dmx);
    NGLDMXFILTER*flt=(NGLDMXFILTER*)dwStbFilterHandle;
    CHECKFILTER(flt);
    memset(flt->mask,0,sizeof(flt->mask));
    memset(flt->value,0,sizeof(flt->value));
    if((uiLength>0&&(pMask==NULL||pValue==NULL))||(uiLength>MAX_FILTER_DEPTH)) {
        return E_INVALID_PARA;
    }
    if(uiLength&&pMask&&pValue) {
        memcpy(flt->mask,pMask,uiLength);
        memcpy(flt->value,pValue,uiLength);
    }
    flt->masklen=uiLength;
    LOGV("filter pid=%d masklen=%d",flt->ch->pid,uiLength);
    return E_OK;
}

INT nglFilterSetCRC(HANDLE filter,BOOL crc) {
    return E_OK;
}

INT nglStartSectionFilter(HANDLE  dwStbFilterHandle) {
    std::unique_lock<std::recursive_mutex>lck(mtx_dmx);
    NGLDMXFILTER*flt=(NGLDMXFILTER*)dwStbFilterHandle;
    CHECKFILTER(flt);
    if(flt->started==0) {
        flt->started++;
        flt->ch->num_started++;
        if(flt->ch->started==0)
            flt->ch->started++;
    }
    return E_OK;
}

/**AllocSectionFilter without FreeSectionFilter ?*/
INT nglStopSectionFilter(HANDLE dwStbFilterHandle) {
    std::unique_lock<std::recursive_mutex>lck(mtx_dmx);
    LOGV("NGLStopSectionFilter filter=0x%x",dwStbFilterHandle);
    NGLDMXFILTER*flt=(NGLDMXFILTER*)dwStbFilterHandle;
    CHECKFILTER(flt);
    if(flt->started) {
        flt->started--;
        flt->ch->num_started--;
        if(flt->ch->num_started==0) {
            flt->ch->pos=0;
            flt->ch->started=0;
        }
    }
    return E_OK;
}

/* End of File */
