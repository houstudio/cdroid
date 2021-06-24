#include <ngl_timer.h>
#include <aui_rtc.h>
#include <time.h>
#include <sys/time.h>
#include <cdlog.h>
#include <errno.h>
NGL_MODULE(TIMER);
#define USE_SYSTIME 1
static aui_hdl hdl_rtc=0;
static NGL_TIME last_setted_time,last_time_time;
void nglTimerInit(){
   if(0==hdl_rtc){
       aui_rtc_init();
       aui_rtc_open(&hdl_rtc);
   }
}

void nglGetTime(NGL_TIME *pTime)
{
    if(pTime){
#ifndef USE_SYSTIME
       *pTime=last_setted_time+time(NULL)-last_time_time;
#else
       *pTime=time(NULL);
#endif
    }
    
    return;
}

DWORD nglSetTime(NGL_TIME*ptime){
    struct timeval tv;
    int rc;
    NGL_TM tm;
    aui_clock clock;
    if(NULL==ptime)
        return E_INVALID_PARA; 
#ifndef  USE_SYSTIME
    nglTimerInit();
    last_setted_time=*ptime;
    last_time_time=time(NULL);
#else
    rc=stime(ptime);
    tv.tv_sec=*ptime;
    tv.tv_usec=0;
    rc=settimeofday(&tv,NULL);
    LOGV("stime=%d %s tz=%s timezone=%d",rc,ctime(ptime),getenv("TZ"),timezone);
    LOGV("settime=%ld gettime=%ld",*ptime,time(NULL));
#endif;
    return E_OK;
}

void nglTimeToTm(const NGL_TIME  *pTime, NGL_TM  *pTm )
{
    struct tm *t=gmtime((time_t*)pTime);
    if(pTm){
        pTm->uiSec=t->tm_sec;
        pTm->uiMin=t->tm_min;
        pTm->uiHour=t->tm_hour;
        pTm->uiMonthDay=t->tm_mday;
        pTm->uiMonth=t->tm_mon;
        pTm->uiYear=t->tm_year;
        pTm->uiWeekDay=t->tm_wday;
        pTm->uiYearDay=t->tm_yday;
    }
    return;
}

void nglTmToTime(const NGL_TM *pTm, NGL_TIME *pTime)
{
  if(pTime){
    struct tm t;
    t.tm_sec=pTm->uiSec;
    t.tm_min=pTm->uiMin;
    t.tm_hour=pTm->uiHour;
    t.tm_mday=pTm->uiMonthDay;
    t.tm_mon=pTm->uiMonth;
    t.tm_year=pTm->uiYear;
    t.tm_wday=pTm->uiWeekDay;
    t.tm_yday=pTm->uiYearDay;
    *pTime=mktime(&t);
  }
  return;
}
void nglGetRunTime(NGL_RunTime *pRunTime)
{
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC,&ts);
  if(pRunTime){
     pRunTime->uiMilliSec=ts.tv_sec*1000;
     pRunTime->uiMicroSec=ts.tv_nsec/1000;
  }
  return;
}

