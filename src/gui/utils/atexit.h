#ifndef __AT_EXIT_H__
#define __AT_EXIT_H__
#include <functional>
#include <vector>
#include <mutex>

namespace cdroid{
class AtExit {
private:
    AtExit()=default;
public:
    class AtExitInitializer;
    friend AtExitInitializer;
    friend void AtExitProc();
    // Register a callback to be executed on exit.
    static void registerCallback(std::function<void()> callback);
private:
    static void executeCallbacks();
    static std::vector<std::function<void()>>& callbacks();
    static std::mutex& callbacksMutex();
};

}
#endif/*__AT_EXIT_H__*/
