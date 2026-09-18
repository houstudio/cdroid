#ifndef CDROID_ESPRESSO_UICONTROLLERIMPL_H
#define CDROID_ESPRESSO_UICONTROLLERIMPL_H

/*
 * android.support.test.espresso.base.UiControllerImpl — implementation of
 * UiController. Ported line-by-line with three CDROID-threading adaptations
 * (single process, single UI thread — AOSP's instrumentation thread does not
 * exist here; Espresso runs ON the main thread):
 *
 * 1. keyEventExecutor (single-thread executor running the blocking
 *    instrumentation RPC) is gone: EventInjector's enqueue-only seam runs
 *    inline. The SignalingTask→KEY/MOTION_INJECTION_HAS_COMPLETED→loopUntil
 *    handshake survives verbatim — the completion signal is posted right
 *    after the enqueue and loopUntil's pump drains the input queue, so the
 *    call still returns only after delivery.
 * 2. AOSP loopUntil manually dispatches main-queue messages with
 *    QueueInterrogator.getNextMessage(); CDROID does the same via
 *    MessageQueue::nextDue() and, when nothing is due, additionally ticks
 *    Looper::pollOnce (bounded) — the tick is what runs the EventHandler
 *    drains (injected input delivery, UIEventSource frames, Choreographer),
 *    the in-process stand-in for AOSP's system-server-side dispatch.
 * 3. The AsyncTask/compat-pool monitors are dropped (no such ecosystem in
 *    CDROID); the IdlingResourceRegistry (dynamic tasks) stays, poll-based
 *    (see idlingresource.h).
 */

#include <bitset>

#include <core/handler.h>
#include <core/looper.h>

#include <app/espresso/eventinjector.h>
#include <app/espresso/idlingresource.h>
#include <app/espresso/queueinterrogator.h>
#include <app/espresso/uicontroller.h>

namespace cdroid {
namespace espresso {

class UiControllerImpl : public UiController, public Handler {
public:
    /** AOSP IdleCondition enum — ordinal is the signal Message.what. */
    enum IdleCondition {
        DELAY_HAS_PAST,
        ASYNC_TASKS_HAVE_IDLED,      // parity only: no AsyncTask ecosystem in CDROID
        COMPAT_TASKS_HAVE_IDLED,     // parity only: dropped monitor
        KEY_INJECT_HAS_COMPLETED,
        MOTION_INJECTION_HAS_COMPLETED,
        DYNAMIC_TASKS_HAVE_IDLED
    };
    static constexpr int IDLE_CONDITION_COUNT = 6;

    explicit UiControllerImpl(Looper* mainLooper,
            EventInjector* eventInjector = nullptr);

    ~UiControllerImpl() override = default;

    bool injectKeyEvent(KeyEvent& event) override;
    bool injectMotionEvent(MotionEvent& event) override;
    bool injectString(const std::string& str) override;
    void loopMainThreadUntilIdle() override;
    void loopMainThreadForAtLeast(int64_t millisDelay) override;

    /** AOSP Handler.Callback.handleMessage — raises idle-condition signals. */
    void handleMessage(Message& msg) override;

private:
    /** IdleCondition.isSignaled / reset / signal on the condition bitset. */
    bool isSignaled(IdleCondition condition) const { return mConditionSet.test((size_t)condition); }
    void resetCondition(IdleCondition condition) { mConditionSet.reset((size_t)condition); }

    /** IdleCondition.createSignal + SignalingTask.done()'s sendMessage. */
    void sendIdleSignal(IdleCondition condition, int myGeneration);

    void loopUntil(IdleCondition condition);
    /**
     * Loops the main thread until all IdleConditions have been signaled.
     * Once they've been signaled, the conditions are reset and the generation
     * value is incremented (AOSP loopUntil contract).
     */
    void loopUntil(const std::vector<IdleCondition>& conditions);

    void initialize();

    /** AOSP UiControllerImpl.getKeyCharacterMap (VIRTUAL_KEYBOARD). */
    static KeyCharacterMap* getKeyCharacterMap();

    /** AOSP KeyEvent.changeTimeRepeat — kept local: KeyEvent has no such static. */
    static KeyEvent* changeTimeRepeat(KeyEvent* event, int64_t eventTime, int newRepeat);

    static constexpr int64_t MASTER_IDLE_TIMEOUT_MS = 60000;  // IdlingPolicies master (60s)

    EventInjector* mEventInjector;            // borrowed (default: owned singleton below)
    std::unique_ptr<EventInjector> mOwnedInjector;
    IdlingResourceRegistry& mIdlingResourceRegistry;
    QueueInterrogator mQueueInterrogator;
    Looper* mMainLooper;

    std::bitset<IDLE_CONDITION_COUNT> mConditionSet;
    bool mLooping = false;   // only updated on main thread
    int mGeneration = 0;
};

} /*endof namespace espresso*/
} /*endof namespace cdroid*/

#endif /*CDROID_ESPRESSO_UICONTROLLERIMPL_H*/
