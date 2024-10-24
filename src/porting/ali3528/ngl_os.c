/*
  FILE : stub_os.c
  PURPOSE: This file is a stub for linking tests.
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <dtvos.h>
#include <aui_os.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/time.h>
#include <unistd.h>
#include <cdlog.h>

NGL_MODULE(OS);

#define LINUX 1
//typedef pthread_mutex_t NGLMutex;
typedef struct{
   pthread_mutex_t mtx;
   pthread_cond_t cond;
   UINT nsem;
}SEM;

INT nglSysInit(){
   return E_OK;
}

INT nglCreateSemaphore(NGLSemaphore * const pSemaphore, UINT uiValue )
{
#ifdef LINUX
  //sem_init(pSemaphore,0,uiValue);
  if(pSemaphore){
      SEM*s=(SEM*)nglMalloc(sizeof(SEM));
      pthread_mutex_init(&s->mtx,NULL);
      pthread_condattr_t attr; 
      pthread_condattr_init(&attr);
      pthread_condattr_setclock(&attr,CLOCK_MONOTONIC);
      pthread_cond_init(&s->cond,&attr);
      pthread_condattr_destroy(&attr);
      s->nsem=uiValue;
      *pSemaphore=s;
  }
  return NULL==pSemaphore?E_ERROR:E_OK;
#else
  aui_hdl hdl;
  aui_attr_sem attr;
  strncpy(attr.sz_name,name,AUI_DEV_NAME_LEN);
  attr.ul_init_val=initcount;
  attr.ul_cnt=initcount;
  attr.ul_max_val=maxcount;
  aui_os_sem_create(&attr,&hdl);
  return (DWORD)hdl;
#endif
}

INT nglDeleteSemaphore(NGLSemaphore pSemaphore)
{
#ifdef LINUX
  //sem_desrtoy((sem_t*)pSemaphore);
  SEM*s=(SEM*)pSemaphore;
  if(NULL==s)
     return E_ERROR;
  pthread_mutex_destroy(&s->mtx);
  pthread_cond_destroy(&s->cond);
  nglFree(s);
  return E_OK;
#else
  aui_hdl hdl=(aui_hdl)pSemaphore;
  aui_os_sem_delete(&hdl);
#endif
  return 0;
}

INT nglAcquireSemaphore(NGLSemaphore pSemaphore, UINT uiDuration )
{
#ifdef LINUX
  int rc=E_OK;
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC,&ts);
  ts.tv_sec+=uiDuration/1000;
  ts.tv_nsec+=(uiDuration%1000)*1000000l;

  if(ts.tv_nsec>=1000000000l){
      ts.tv_sec+=ts.tv_nsec/1000000000l;
      ts.tv_nsec%=1000000000l;
  }
  SEM*s=(SEM*)pSemaphore;
  if(NULL==s)
      return E_ERROR;
  if(0==uiDuration){ts.tv_sec=ts.tv_nsec=0;}
  pthread_mutex_lock(&s->mtx);
  if(s->nsem==0)
     rc=pthread_cond_timedwait(&s->cond,&s->mtx,&ts);
  s->nsem--;
  pthread_mutex_unlock(&s->mtx);
  return rc;
#else
  return aui_os_sem_wait((aui_hdl)pSemaphore,uiDuration)==AUI_RTN_SUCCESS?E_OK:E_ERROR;
#endif
}


INT nglReleaseSemaphore(NGLSemaphore pSemaphore )
{
#ifdef LINUX
    SEM*s=(SEM*)pSemaphore;
    if(NULL==s)
       return E_ERROR;
    pthread_mutex_lock(&s->mtx);
    if(s->nsem==0)
        pthread_cond_broadcast(&s->cond);
    s->nsem++;
    pthread_mutex_unlock(&s->mtx);
    return E_OK;
#else
  aui_os_sem_release((aui_hdl)pSemaphore);
  return 0;
#endif
}


INT nglCreateMutex(NGLMutex * const pMutex)
{
#ifdef LINUX
    if(NULL==pMutex)return E_INVALID_PARA;
    pthread_mutexattr_t attr;
    pthread_mutex_t *mtx=(pthread_mutex_t*)nglMalloc(sizeof(pthread_mutex_t));;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_setpshared(&attr,PTHREAD_PROCESS_SHARED);//PTHREAD_PROCESS_PRIVATE PTHREAD_PROCESS_SHARED
    pthread_mutexattr_settype(&attr,PTHREAD_MUTEX_RECURSIVE);//PTHREAD_MUTEX_RECURSIVE);//PTHREAD_MUTEX_ERRORCHECK
    pthread_mutex_init(mtx,&attr);
    pthread_mutexattr_destroy(&attr);
    *pMutex=(NGLMutex*)mtx;
    return E_OK;
#else
  
#endif
  return 0;
}


INT nglDeleteMutex(NGLMutex pMutex)
{
    if(NULL==pMutex)
       return E_INVALID_PARA;
    int rc=pthread_mutex_destroy((pthread_mutex_t*)pMutex);
    nglFree((void*)pMutex);
    return E_OK;
}


INT nglLockMutex(NGLMutex pMutex)
{
   if(NULL==pMutex)
       return E_INVALID_PARA;
   return 0==pthread_mutex_lock((pthread_mutex_t*)pMutex)?E_OK:E_ERROR;
}


INT nglUnlockMutex(NGLMutex pMutex)
{
    if(NULL==pMutex)
       return E_INVALID_PARA;
    return 0==pthread_mutex_unlock((pthread_mutex_t*)pMutex)?E_OK:E_ERROR;
}

typedef struct{
    pthread_mutex_t mtx;
    pthread_cond_t cond;
    pthread_condattr_t attr;
    BOOL triggered;
    BOOL autoreset;
}EVENT;

OSHANDLE nglCreateEvent(BOOL state,BOOL autoreset )
{
    EVENT*e=(EVENT*)nglMalloc(sizeof(EVENT));
    pthread_mutex_init(&e->mtx,NULL);
    pthread_condattr_init(&e->attr);
    pthread_condattr_setclock(&e->attr,CLOCK_MONOTONIC);
    pthread_cond_init(&e->cond,&e->attr);
    e->triggered=state;
    e->autoreset=autoreset;
    return (OSHANDLE)e;
#ifdef LINUX

#else
     aui_hdl hdl;
     aui_attr_event attr;
     strncpy(attr.sz_name,name,AUI_DEV_NAME_LEN);
     attr.b_auto_reset=0;
     attr.b_initial_state=0;
     aui_os_event_create(&attr,&hdl);
     return (OSHANDLE)hdl;
#endif
}


int32_t nglDestroyEvent(OSHANDLE eventid)
{
#ifdef LINUX
    EVENT*e=(EVENT*)eventid;
    if(NULL==e)return E_INVALID_PARA;
    pthread_cond_destroy(&e->cond);
    pthread_mutex_destroy(&e->mtx);
    pthread_condattr_destroy(&e->attr);
    nglFree(e);
    return E_OK;
#else
     aui_hdl hdl=(aui_hdl)eventid;
     aui_os_event_delete(&hdl);
     return 0;
#endif
}

int32_t nglResetEvent(OSHANDLE eventid)
{
#ifdef LINUX
     EVENT*e=(EVENT*)eventid;
     if(NULL==e)return E_INVALID_PARA;
     pthread_mutex_lock(&e->mtx);
     e->triggered=false;
     pthread_mutex_unlock(&e->mtx);
     return 0;
#else
     aui_os_event_clear((aui_hdl)eventid);
     return 0;
#endif
}

int32_t nglSetEvent(OSHANDLE eventid)
{
#ifdef LINUX
    EVENT*e=(EVENT*)eventid;
    pthread_mutex_lock(&e->mtx);
    pthread_cond_broadcast(&e->cond);
    e->triggered=true;
    pthread_mutex_unlock(&e->mtx);
#else
    aui_os_event_set((aui_hdl)eventid,0);
    return 0;
#endif
}

int32_t nglWaitEvent(OSHANDLE eventid, uint32_t timeout)
{
#ifdef LINUX
    EVENT*e=(EVENT*)eventid;
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC,&ts);
    
    ts.tv_sec+=timeout/1000;
    ts.tv_nsec+=(timeout%1000)*1000000l;

    if(ts.tv_nsec>=1000000000l){
         ts.tv_sec+=ts.tv_nsec/1000000000l;
         ts.tv_nsec%=1000000000l;
    }
    else if(timeout==0)ts.tv_sec=ts.tv_nsec=0;
    
    pthread_mutex_lock(&e->mtx); 
    while(!e->triggered){
         if(pthread_cond_timedwait(&e->cond,&e->mtx,&ts)){
               pthread_mutex_unlock(&e->mtx);
               return E_ERROR;
         }
    }
    if(e->autoreset)e->triggered=FALSE;
    pthread_mutex_unlock(&e->mtx);
    return E_OK;
#else
     int rc=aui_os_event_wait((aui_hdl)eventid,timeout,0);
     return rc!=AUI_RTN_SUCCESS;
#endif
}

void nglCreateThread(OSHANDLE *threadid,int p,int stacksize,THREAD_PROC proc,void*param)
{
    pthread_t thid;
    pthread_create(&thid,NULL,(void * (*)(void *))proc,param);
    *threadid=thid;
}

void nglDeleteThread(OSHANDLE threadid){
    
}

void nglJoinThread(OSHANDLE threadid){
    pthread_join((pthread_t)threadid,NULL);
}

void *nglMalloc(UINT uiSize){
    return uiSize==0?NULL:malloc(uiSize);
}

void* nglAlloc ( UINT uiSize){
    return (void*)calloc(uiSize,1);
}

void* nglRealloc(void*p,UINT size){
    return (void*)realloc(p,size);
}

void nglFree(void * ptr){
    if(ptr!=NULL)
       free(ptr);
    return;
}


INT nglPrintf(const char *pFormat, ...)
{
    va_list args;
    va_start(args, pFormat);
    vprintf(pFormat,args);
    va_end(args);
    return 0;
}

void nglSleep( UINT uiDuration ){
    usleep(uiDuration*1000);
    return;
}

/* End of File */
