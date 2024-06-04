#ifndef __HANDLER_ACTION_QUEUE_H__
#define __HANDLER_ACTION_QUEUE_H__
#include <view/view.h>
namespace  cdroid{

class HandlerActionQueue {
private:
    class HandlerAction {
    public:
        Runnable action;
        long delay;
    public:
        HandlerAction(const Runnable& action, long delay);
        bool matches(const Runnable& otherAction)const;
    };
private:
    std::vector<HandlerAction*> mActions;
public:
    void post(const Runnable& action);
    void postDelayed(const Runnable& action, long delayMillis);
    void removeCallbacks(const Runnable& action);
    void executeActions(UIEventSource& handler);
    int size()const;
    Runnable& getRunnable(int index);
    long getDelay(int index)const;
};
}/*endof namespace*/
#endif /*__HANDLER_ACTION_QUEUE_H__*/
