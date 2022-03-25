#include <core/scheduler.h>
#include <cdtypes.h>
#include <cdlog.h>

namespace cdroid{

static void Repeat(Scheduler* s, Scheduler::Function f, int64_t deltaSeconds){
    f();
    s->scheduleFromNow(std::bind(&Repeat, s, f, deltaSeconds), deltaSeconds);
}

static void Repeat2(Scheduler* s, Scheduler::Function f,system_clock::time_point t,int deltaSeconds){
    f();
    t+=std::chrono::seconds(deltaSeconds);
    s->schedule(std::bind(&Repeat2,s,f,t,deltaSeconds),t);
}


void Scheduler::schedule(Function f,system_clock::time_point t){
    if(t>system_clock::now())
       taskQueue.insert(std::make_pair(t, f));
}

void Scheduler::scheduleFromNow(Function f, int64_t deltaSeconds){
    schedule(f,system_clock::now() + std::chrono::seconds(deltaSeconds));
}

void Scheduler::scheduleEvery(Scheduler::Function f, int64_t deltaSeconds){
    scheduleFromNow(std::bind(&Repeat, this, f, deltaSeconds), deltaSeconds);
}

void Scheduler::scheduleHourly(Function f,system_clock::time_point t){
    while(t<system_clock::now())
        t+=std::chrono::seconds(3600);
    schedule(std::bind(&Repeat2,this,f,t,3600),t);
}

void Scheduler::scheduleDaily(Function f,system_clock::time_point t){
    while(t<system_clock::now()){
       t+=std::chrono::seconds(24L*3600);
    }
    schedule(std::bind(&Repeat2,this,f,t,24L*3600),t);
}

void Scheduler::scheduleWeekly(Function f,system_clock::time_point t){
    while(t<system_clock::now()){
       t+=std::chrono::seconds(7L*24*3600);
    }
    schedule(std::bind(&Repeat2,this,f,t,7L*24*3600),t);
}

void Scheduler::scheduleMonthly(Function f,system_clock::time_point t){
    LOGW("TODO");        
}

void Scheduler::remove(system_clock::time_point t){
    auto it=taskQueue.find(t);
    if(it!=taskQueue.end())
        taskQueue.erase(it);
}

void Scheduler::handleMessage(Message&){
}
#if 0
bool Scheduler::check(){
    auto task=taskQueue.begin();
    if(taskQueue.size()==0)return 0;
    LOGV("time.now=%lld",system_clock::now().time_since_epoch().count());
    for(auto it=taskQueue.begin();it!=taskQueue.end();it++)
        LOGV("it->time=%lld",it->first.time_since_epoch().count());
    if(task->first<=system_clock::now()){
        task->second();/*Call Function*/
        taskQueue.erase(taskQueue.begin());
        return 1;
    }
    return 0;
}
#endif

}/*namespace cdroid*/
