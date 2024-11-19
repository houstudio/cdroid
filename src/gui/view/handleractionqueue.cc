#include <view/handleractionqueue.h>
#include <core/uieventsource.h>
namespace cdroid{

void HandlerActionQueue::post(const Runnable& action) {
    postDelayed(action, 0);
}

void HandlerActionQueue::postDelayed(const Runnable& action, long delayMillis) {
    HandlerAction* handlerAction = new HandlerAction(action, delayMillis);
    mActions.push_back(handlerAction);
}

void HandlerActionQueue::removeCallbacks(const Runnable& action) {
    const size_t count = mActions.size();
    size_t j = 0;
    for (size_t i = 0; i < count; i++) {
        if (mActions[i]->matches(action)) {
            // Remove this action by overwriting it within
            // this loop or nulling it out later.
            continue;
        }
        if (j != i) {
            // At least one previous entry was removed, so
            // this one needs to move to the "new" list.
            mActions[j] = mActions[i];
        }
        j++;
    }

    // The "new" list only has j entries.
    // Null out any remaining entries.
    const int finalCount = j;
    for (; j < count; j++) {
        delete mActions[j];
        mActions[j] = nullptr;
    }
    mActions.resize(finalCount);
}

void HandlerActionQueue::executeActions(UIEventSource& handler) {
    for (size_t i = 0, count = mActions.size(); i < count; i++) {
        HandlerAction* handlerAction = mActions[i];
        handler.postDelayed(handlerAction->action, handlerAction->delay);
    }
    mActions.clear();
}

int HandlerActionQueue::size() const{
    return mActions.size();
}

Runnable& HandlerActionQueue::getRunnable(int index) {
    FATAL_IF(index >= mActions.size(),"IndexOutOfBoundsException");
    return mActions[index]->action;
}

long HandlerActionQueue::getDelay(int index) const{
    FATAL_IF(index >= mActions.size(),"IndexOutOfBoundsException");
    return mActions[index]->delay;
}

HandlerActionQueue::HandlerAction::HandlerAction(const Runnable& action, long delay) {
    this->action = action;
    this->delay = delay;
}

bool HandlerActionQueue::HandlerAction::matches(const Runnable& otherAction)const {
    return ((otherAction == nullptr) && (action == nullptr))
            || ((action != nullptr) && (action==otherAction));
}

}/*endof namespace*/
