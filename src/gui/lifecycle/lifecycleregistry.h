#ifndef __LIFECYCLEREGISTRY_H__
#define __LIFECYCLEREGISTRY_H__
/*********************************************************************************
 * Port of androidx.lifecycle.LifecycleRegistry. Manages multiple observers and
 * drives them through the lifecycle state machine with the upstream sync
 * algorithm (forward/backward passes converging on the registry's state).
 * Coroutine/Flow/StateFlow and LifecycleTracer are omitted.
 *********************************************************************************/
#include <memory>
#include <vector>
#include <lifecycle/lifecycle.h>
#include <lifecycle/lifecycleowner.h>
#include <lifecycle/lifecycleobserver.h>
#include <lifecycle/lifecycleeventobserver.h>
#include <lifecycle/defaultlifecycleobserver.h>
#include <lifecycle/lifecycling.h>
#include <lifecycle/safeiterablemap.h>

namespace cdroid{
namespace lifecycle{

class LifecycleRegistry : public Lifecycle{
public:
    explicit LifecycleRegistry(LifecycleOwner* provider);
    // Creates a registry without main-thread enforcement (for tests).
    static LifecycleRegistry* createUnsafe(LifecycleOwner* owner);

    // Deprecated; equivalent to setCurrentState.
    void markState(State state);
    void setCurrentState(State state);
    void handleLifecycleEvent(Event event);

    void addObserver(LifecycleObserver* observer) override;
    void removeObserver(LifecycleObserver* observer) override;
    State getCurrentState() const override;
    int getObserverCount();

private:
    // Couples an observer with its current state and the resolved event dispatcher.
    struct ObserverWithState{
        LifecycleObserver* observer;
        State state;
        LifecycleEventObserver* lifecycleObserver; // resolved (owned if adapter)
        bool ownsLifecycleObserver;
        explicit ObserverWithState(LifecycleObserver* o, State initial);
        ~ObserverWithState();
        void dispatchEvent(LifecycleOwner* owner, Event event);
    };

    using ObserverMap = SafeIterableMap<LifecycleObserver*, std::shared_ptr<ObserverWithState>>;

    LifecycleRegistry(LifecycleOwner* provider, bool enforceMainThread);

    ObserverMap mObserverMap;
    LifecycleOwner* mLifecycleOwner; // owner outlives the registry (no GC in C++)
    int mAddingObserverCounter = 0;
    bool mHandlingEvent = false;
    bool mNewEventOccurred = false;
    std::vector<State> mParentStates;
    State mInternalState = State::INITIALIZED;
    bool mEnforceMainThread;

    void moveToState(State next);
    bool isSynced() const;
    State calculateTargetState(LifecycleObserver* observer) const;
    void forwardPass(LifecycleOwner* owner);
    void backwardPass(LifecycleOwner* owner);
    void sync();
    void enforceMainThreadIfNeeded(const char* method);
    void checkLifecycleStateTransition(State current, State next);
};

}//namespace lifecycle
}//namespace cdroid
#endif
