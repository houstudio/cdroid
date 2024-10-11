#include <dtvmsgq.h>
#include <dtvos.h>
#include <aui_os.h>
#include <pthread.h>
#include <cdlog.h>

NGL_MODULE(MSGQ);

typedef struct{
   pthread_mutex_t mutex;
   pthread_cond_t cget;
   pthread_cond_t cput;
   int msgSize;//max size of message item
   int queSize;
   int rdidx;
   int wridx;
   int msgCount;
   unsigned char*msgs;
}MSGQUEUE;
HANDLE nglMsgQCreate(int howmany, int sizepermag)
{
#ifndef LINUX
        aui_hdl hdl;
        aui_attr_msgq attr;
        attr.ul_buf_size=howmany*sizepermag;
        attr.ul_msg_size_max=sizepermag;
        aui_os_msgq_create(&attr,&hdl);
        return (DWORD)hdl;
#else
     if(howmany*sizepermag==0)return 0;
     MSGQUEUE*q=(MSGQUEUE*)nglMalloc(sizeof(MSGQUEUE));
     pthread_mutex_init(&q->mutex,NULL);
     pthread_condattr_t attr;
     pthread_condattr_init(&attr);
     pthread_condattr_setclock(&attr,CLOCK_MONOTONIC);
     pthread_cond_init(&q->cget,&attr);
     pthread_cond_init(&q->cput,&attr);
     pthread_condattr_destroy(&attr);
     q->queSize=howmany;
     q->msgSize=sizepermag;
     q->rdidx=q->wridx=q->msgCount=0;
     q->msgs=(unsigned char*)nglMalloc(sizepermag*howmany);
     LOGV("msgq=%p",q);
     return (HANDLE)q;
#endif
}

DWORD nglMsgQDestroy(HANDLE msgq)
{
#ifndef LINUX
     aui_hdl hdl=(aui_hdl)msgq;
     aui_os_msgq_delete(&hdl);
     return E_OK;
#else
     MSGQUEUE*q=(MSGQUEUE*)msgq;
     pthread_cond_destroy(&q->cput);
     pthread_cond_destroy(&q->cget);
     pthread_mutex_destroy(&q->mutex);
     nglFree(q->msgs);
     nglFree(q);
     return E_OK;
#endif
}

DWORD nglMsgQSend(HANDLE msgq, const void* pvmsg, int msgsize, DWORD timeout)
{
#ifndef LINUX
     AUI_RTN_CODE rc=aui_os_msgq_snd((aui_hdl)msgq,(void*)pvmsg,msgsize,timeout);
     return rc!=AUI_RTN_SUCCESS;
#else
    MSGQUEUE*q=(MSGQUEUE*)msgq;
    struct timespec ts;
    int rc=0;
    clock_gettime(CLOCK_MONOTONIC,&ts);
    ts.tv_sec += timeout/1000;
    ts.tv_nsec+= (timeout%1000)*1000000l;
    if(ts.tv_nsec>=1000000000l){
         ts.tv_sec+=ts.tv_nsec/1000000000l;
         ts.tv_nsec%=1000000000l;
    }
    if(0==timeout){ts.tv_sec=ts.tv_nsec=0;}
    pthread_mutex_lock(&q->mutex);
    LOGV("rdidx=%d wridx=%d msgsize=%d/%d/%d",q->rdidx,q->wridx,q->msgCount,
        (q->wridx+q->queSize-q->rdidx)%q->queSize,q->queSize);
    if((q->wridx==q->rdidx)&&(q->queSize==q->msgCount)){
        LOGV("queue is full");
        rc=pthread_cond_timedwait(&q->cput,&q->mutex,&ts);
    }
    if(0==rc){
        memcpy(q->msgs+q->wridx*q->msgSize,pvmsg,((msgsize<=0)?q->msgSize:msgsize));
        q->wridx=(q->wridx+1)%q->queSize;
        q->msgCount++;
        pthread_cond_signal(&q->cget);
    }
    pthread_mutex_unlock(&q->mutex);
    return rc==0?E_OK:E_ERROR;
#endif
}

DWORD nglMsgQReceive(HANDLE msgq,void* pvmsg, DWORD msgsize, DWORD timeout)
{
#ifndef LINUX
    unsigned long asize;
    aui_os_msgq_rcv((aui_hdl)msgq,(void*)pvmsg,msgsize,&asize,timeout);
    CAASSERT(asize==msgsize);
    return 0;
#else
    MSGQUEUE*q=(MSGQUEUE*)msgq;
    struct timespec ts;
    int rc=0;
    clock_gettime(CLOCK_MONOTONIC,&ts);
    ts.tv_sec += timeout/1000;
    ts.tv_nsec+= (timeout%1000)*1000000l;
    if(ts.tv_nsec>=1000000000l){
         ts.tv_sec+=ts.tv_nsec/1000000000l;
         ts.tv_nsec%=1000000000l;
    }
    if(0==timeout){ts.tv_sec=ts.tv_nsec=0;}
    pthread_mutex_lock(&q->mutex);
    LOGV("rdidx=%d wridx=%d msgsize=%d/%d/%d",q->rdidx,q->wridx,q->msgCount,
        (q->wridx+q->queSize-q->rdidx)%q->queSize,   q->queSize);
    if((q->wridx==q->rdidx)&&(q->msgCount==0)){
        LOGV("queue is empty waiting...");
        rc=pthread_cond_timedwait(&q->cget,&q->mutex,&ts);
    }
    if(0==rc){
        memcpy(pvmsg,q->msgs+q->rdidx*q->msgSize,((msgsize<=0)?q->msgSize:msgsize));
        q->rdidx=(q->rdidx+1)%q->queSize;
        q->msgCount--;
        pthread_cond_signal(&q->cput);
    }
    pthread_mutex_unlock(&q->mutex);
    return rc==0?E_OK:E_ERROR;
#endif
}

DWORD nglMsgQGetCount(HANDLE msgq,UINT*count){
    MSGQUEUE*q=(MSGQUEUE*)msgq;
    pthread_mutex_lock(&q->mutex);
    *count=(q->wridx+q->queSize-q->rdidx)%q->queSize;//it is the same to q->msgCount;
    pthread_mutex_unlock(&q->mutex);
    return 0;
}

