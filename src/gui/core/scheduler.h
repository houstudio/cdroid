#ifndef __SHCEDULE_H__
#define __SHCEDULE_H__
#include <chrono>
#include <functional>
#include <map>
#include <core/looper.h>

using namespace std::chrono;
namespace cdroid{

class Scheduler:public MessageHandler{
public:
typedef std::function<void(void)> Function;
private:
protected:
    std::map<system_clock::time_point,Function>taskQueue;
public:
    void schedule(Function f,system_clock::time_point t);
    void scheduleFromNow(Function f, int64_t deltaSeconds);
    void scheduleEvery(Function f, int64_t deltaSeconds);
    void scheduleHourly(Function f,system_clock::time_point t);
    void scheduleDaily(Function f,system_clock::time_point t);
    void scheduleWeekly(Function f,system_clock::time_point t);
    void scheduleMonthly(Function f,system_clock::time_point t);/*TODO*/
    virtual void remove(system_clock::time_point t);
    void handleMessage(const Message&)override;
};

}
#endif

