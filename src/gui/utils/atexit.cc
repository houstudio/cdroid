#include <algorithm>
#include <iostream>
#include <stdlib.h>
#include <memory>
#include <porting/cdlog.h>
#include <utils/atexit.h>
namespace cdroid {

static std::vector<std::function<void()>> *mCallbacks = nullptr;
std::vector<std::function<void()>>& AtExit::callbacks() {
    if(mCallbacks==nullptr)
        mCallbacks = new std::vector<std::function<void()>>;
    return *mCallbacks;
}

std::mutex& AtExit::callbacksMutex() {
    static std::mutex mutex;
    return mutex;
}

void AtExit::registerCallback(std::function<void()> callback) {
    std::lock_guard<std::mutex> lock(callbacksMutex());
    callbacks().push_back(callback);
    LOGD("callbacks.size=%d %p",callbacks().size(),&callback);
}

void AtExit::executeCallbacks() {
    std::lock_guard<std::mutex> lock(callbacksMutex());
    std::vector<std::function<void()>>&cbks = callbacks();
    LOGD_IF(cbks.size(),"AtExit::executeCallbacks %d",int(cbks.size()));
    for(auto& callback:cbks) {
        if(callback)callback();
    }
    delete mCallbacks;
}

class AtExit::AtExitInitializer{
public:
    AtExitInitializer();
};

// Ensure callbacks are executed at exit
AtExit::AtExitInitializer::AtExitInitializer(){
    LOGD("**AtExitInitializer::AtExitInitializer");
    atexit(AtExit::executeCallbacks);
}

AtExit::AtExitInitializer atExitInitializer;

} /*namespace cdroid*/

