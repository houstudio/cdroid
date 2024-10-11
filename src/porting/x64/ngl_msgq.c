#include <ngl_msgq.h>
#include <ngl_os.h>
#include <pthread.h>
#include <cdlog.h>
#include <string.h>

typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t cget;
    pthread_cond_t cput;
    int msgSize;//max size of message item
    int queSize;
    int rdidx;
    int wridx;
    int msgCount;
    unsigned char*msgs;
} MSGQUEUE;

HANDLE nglMsgQCreate(int howmany, int sizepermag) {
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
    return q;
}

int32_t nglMsgQDestroy(HANDLE msgid) {
    MSGQUEUE*q=(MSGQUEUE*)msgid;
    pthread_cond_destroy(&q->cput);
    pthread_cond_destroy(&q->cget);
    pthread_mutex_destroy(&q->mutex);
    nglFree(q->msgs);
    nglFree(q);
    return E_OK;
}

int32_t nglMsgQSend(HANDLE msgid, const void* pvmsg, uint msgsize, uint32_t timeout) {
    MSGQUEUE*q=(MSGQUEUE*)msgid;
    struct timespec ts;
    int rc=0;
    clock_gettime(CLOCK_MONOTONIC,&ts);
    ts.tv_sec += timeout/1000;
    ts.tv_nsec+= (timeout%1000)*1000000l;
    if(ts.tv_nsec>=1000000000l) {
        ts.tv_sec+=ts.tv_nsec/1000000000l;
        ts.tv_nsec%=1000000000l;
    }
    if(0==timeout) {
        ts.tv_sec=ts.tv_nsec=0;
    }
    pthread_mutex_lock(&q->mutex);
    //LOGV("rdidx=%d wridx=%d msgsize=%d/%d",q->rdidx,q->wridx,q->msgCount,q->queSize);
    if((q->wridx==q->rdidx)&&(q->queSize==q->msgCount)) {
        LOGV("queue is full");
        rc=pthread_cond_timedwait(&q->cput,&q->mutex,&ts);
    }
    if(0==rc) {
        memcpy(q->msgs+q->wridx*q->msgSize,pvmsg,((msgsize<=0)?q->msgSize:msgsize));
        q->wridx=(q->wridx+1)%q->queSize;
        q->msgCount++;
        pthread_cond_signal(&q->cget);
    }
    pthread_mutex_unlock(&q->mutex);
    return rc==0?E_OK:E_ERROR;
}

int32_t nglMsgQReceive(HANDLE msgid,void* pvmsg, uint32_t msgsize, uint32_t timeout) {
    MSGQUEUE*q=(MSGQUEUE*)msgid;
    struct timespec ts;
    int rc=0;
    clock_gettime(CLOCK_MONOTONIC,&ts);
    ts.tv_sec += timeout/1000;
    ts.tv_nsec+= (timeout%1000)*1000000l;
    if(ts.tv_nsec>=1000000000l) {
        ts.tv_sec+=ts.tv_nsec/1000000000l;
        ts.tv_nsec%=1000000000l;
    }
    if(0==timeout) {
        ts.tv_sec=ts.tv_nsec=0;
    }
    pthread_mutex_lock(&q->mutex);
    //LOGV("rdidx=%d wridx=%d msgsize=%d/%d",q->rdidx,q->wridx,q->msgCount,q->queSize);
    if((q->wridx==q->rdidx)&&(q->msgCount==0)) {
        //LOGV("queue is empty waiting...");
        rc=pthread_cond_timedwait(&q->cget,&q->mutex,&ts);
    }
    if(0==rc) {
        memcpy(pvmsg,q->msgs+q->rdidx*q->msgSize,((msgsize<=0)?q->msgSize:msgsize));
        q->rdidx=(q->rdidx+1)%q->queSize;
        q->msgCount--;
        pthread_cond_signal(&q->cput);
    }
    pthread_mutex_unlock(&q->mutex);
    return rc==0?E_OK:E_ERROR;
}

int32_t nglMsgQGetCount(HANDLE msgid,uint32_t*count) {
    MSGQUEUE*q=(MSGQUEUE*)msgid;
    pthread_mutex_lock(&q->mutex);
    *count=q->msgCount;
    pthread_mutex_unlock(&q->mutex);
    return 0;
}

