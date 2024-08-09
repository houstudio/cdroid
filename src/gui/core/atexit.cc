#include "atexit.h"
#include <algorithm>
#include <iostream>

namespace cdroid {

std::vector<std::function<void()>>& AtExit::callbacks() {
    static std::vector<std::function<void()>> callbacks;
    return callbacks;
}

std::mutex& AtExit::callbacksMutex() {
    static std::mutex mutex;
    return mutex;
}

void AtExit::registerCallback(std::function<void()> callback) {
    std::lock_guard<std::mutex> lock(callbacksMutex());
    callbacks().push_back(std::move(callback));
}

void AtExit::executeCallbacks() {
    std::lock_guard<std::mutex> lock(callbacksMutex());
    std::for_each(callbacks().rbegin(), callbacks().rend(), [](const std::function<void()>& callback) {
        callback();
    });
}
class AtExit::AtExitInitializer{
public:
    AtExitInitializer();
};

// Ensure callbacks are executed at exit
AtExit::AtExitInitializer::AtExitInitializer(){
    std::atexit([] {
        AtExit::executeCallbacks();
    });
}

AtExit::AtExitInitializer atExitInitializer;

} /*namespace cdroid*/

