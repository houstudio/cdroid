#include <app/espresso/uicontrollerimpl.h>

#include <climits>
#include <fstream>
#include <mutex>

#include <stdexcept>
#include <string>
#include <vector>

#include <app/espresso/espressoexception.h>

#include <core/message.h>
#include <core/systemclock.h>
#include <porting/cdlog.h>
#include <private/keycharactermap.h>
#include <text/textutils.h>

namespace cdroid {
namespace espresso {

static const char* TAG = "UiControllerImpl";

UiControllerImpl::UiControllerImpl(Looper* mainLooper, EventInjector* eventInjector)
    : Handler(mainLooper),
      mEventInjector(eventInjector ? eventInjector : nullptr),
      mIdlingResourceRegistry(IdlingResourceRegistry::getInstance()),
      mQueueInterrogator(mainLooper),
      mMainLooper(mainLooper) {
    if (!mEventInjector) {
        mOwnedInjector.reset(new UiAutomationEventInjector());
        mEventInjector = mOwnedInjector.get();
    }
}

void UiControllerImpl::initialize() {
    // AOSP lazily creates controllerHandler(this); the Handler base class is
    // already bound to the main looper here, so initialization is a no-op.
}

void UiControllerImpl::sendIdleSignal(IdleCondition condition, int myGeneration) {
    // IdleCondition.createSignal: Message.obtain(handler, ordinal, myGeneration, 0, null)
    Message* message = Message::obtain(this, (int)condition, myGeneration, 0);
    sendMessage(message);
}

bool UiControllerImpl::injectKeyEvent(KeyEvent& event) {
    if (Looper::myLooper() != mMainLooper) {
        throw std::runtime_error("Expecting to be on main thread!");
    }
    initialize();
    loopMainThreadUntilIdle();

    // AOSP submits a SignalingTask to the keyEventExecutor and loopUntil()s
    // until KEY_INJECT_HAS_COMPLETED; the task body is sendKeySync — a
    // BLOCKING call that returns only after the event is fully dispatched,
    // so the done() signal follows the dispatch. The in-process seam is
    // enqueue-only, so run the looper pass that drains the parked event
    // (doEventHandlers plays the InputDispatcher) before signaling —
    // otherwise loopUntil() can exit with the event still parked and it
    // lands in whatever test pumps the looper next.
    bool injected = mEventInjector->injectKeyEvent(event);
    if (injected) {
        mMainLooper->pollOnce(0);
    }
    sendIdleSignal(KEY_INJECT_HAS_COMPLETED, mGeneration);

    loopUntil(KEY_INJECT_HAS_COMPLETED);
    // AOSP checks injectTask.isDone() then get() — the signal only fires after
    // the injector returned, so the result is already final here.
    return injected;
}

bool UiControllerImpl::injectMotionEvent(MotionEvent& event) {
    if (Looper::myLooper() != mMainLooper) {
        throw std::runtime_error("Expecting to be on main thread!");
    }
    initialize();

    bool injected = mEventInjector->injectMotionEvent(event);
    // Same seam as injectKeyEvent: sendPointerSync also blocks until the
    // event is dispatched, so deliver the parked event before signaling
    // completion. (Motion only ever drained "by accident" before — the tap
    // path's loopMainThreadForAtLeast happens to fall into pollOnce; paths
    // without such a delay would strand the event exactly like typeText.)
    if (injected) {
        mMainLooper->pollOnce(0);
    }
    sendIdleSignal(MOTION_INJECTION_HAS_COMPLETED, mGeneration);
    try {
        loopUntil(MOTION_INJECTION_HAS_COMPLETED);
    } catch (...) {
        loopMainThreadUntilIdle();
        throw;
    }
    loopMainThreadUntilIdle();  // AOSP finally-block order: idle-out after every motion
    return injected;
}

bool UiControllerImpl::injectString(const std::string& str) {
    if (Looper::myLooper() != mMainLooper) {
        throw std::runtime_error("Expecting to be on main thread!");
    }
    initialize();

    // No-op if string is empty.
    if (str.empty()) {
        LOGW("Supplied string is empty resulting in no-op (nothing is typed).");
        return true;
    }

    bool eventInjected = false;
    KeyCharacterMap* keyCharacterMap = getKeyCharacterMap();

    // AOSP: keyCharacterMap.getEvents(str.toCharArray())
    const std::u16string chars = TextUtils::utf8_utf16(str);
    std::vector<KeyEvent> events;
    if (!keyCharacterMap->getEvents(-1 /* KeyCharacterMap.VIRTUAL_KEYBOARD */,
                chars.data(), chars.size(), events)) {
        throw std::runtime_error(
                "Failed to get key events for string " + str
                + " (i.e. current IME does not understand how to translate the string "
                  "into key events). As a workaround, you can use replaceText action "
                  "to set the text directly in the EditText field.");
    }

    LOGD("Injecting string: \"%s\"", str.c_str());

    for (KeyEvent& keyEvent : events) {
        eventInjected = false;
        // NOTE: the extra attempts++ inside the body is verbatim AOSP — the
        // loop runs at most 2 iterations, not 4 (upstream quirk kept).
        for (int attempts = 0; !eventInjected && attempts < 4; attempts++) {
            attempts++;

            // All KeyEvents returned by getEvents share a time stamp; the
            // system rejects too-old events, so re-stamp before injecting.
            KeyEvent* event = changeTimeRepeat(&keyEvent, SystemClock::uptimeMillis(), 0);
            eventInjected = injectKeyEvent(*event);
            // The injection seam copies the event before parking it, so the
            // caller-owned original must go back to the pool here (same
            // convention as the pressKey paths in viewactions.cc).
            event->recycle();
        }

        if (!eventInjected) {
            LOGE("Failed to inject event for character with key code (%d)",
                 keyEvent.getKeyCode());
            break;
        }
    }

    return eventInjected;
}

// static
KeyCharacterMap* UiControllerImpl::getKeyCharacterMap() {
    // AOSP loads KeyCharacterMap.VIRTUAL_KEYBOARD; CDROID's KeyCharacterMap
    // port exposes the combined default map instead (which now also probes
    // upward from the executable — see KeyCharacterMap::getDefault).
    return KeyCharacterMap::getDefault();
}

// static
KeyEvent* UiControllerImpl::changeTimeRepeat(KeyEvent* event, int64_t eventTime,
        int newRepeat) {
    return KeyEvent::obtain(event->getDownTime(), eventTime, event->getAction(),
            event->getKeyCode(), newRepeat, event->getMetaState(),
            event->getDeviceId(), event->getScanCode(), event->getFlags(),
            event->getSource());
}

void UiControllerImpl::loopMainThreadUntilIdle() {
    initialize();
    if (Looper::myLooper() != mMainLooper) {
        throw std::runtime_error("Expecting to be on main thread!");
    }
    do {
        std::vector<IdleCondition> condChecks;
        // AOSP adds ASYNC_TASKS_HAVE_IDLED / COMPAT_TASKS_HAVE_IDLED here;
        // CDROID has no AsyncTask ecosystem to monitor, so only the dynamic
        // IdlingResource condition remains.
        if (!mIdlingResourceRegistry.allResourcesAreIdle()) {
            condChecks.push_back(DYNAMIC_TASKS_HAVE_IDLED);
        }
        try {
            loopUntil(condChecks);
        } catch (...) {
            mIdlingResourceRegistry.cancelIdleMonitor();
            throw;
        }
        mIdlingResourceRegistry.cancelIdleMonitor();
    } while (!mIdlingResourceRegistry.allResourcesAreIdle());
}

void UiControllerImpl::loopMainThreadForAtLeast(int64_t millisDelay) {
    initialize();
    if (Looper::myLooper() != mMainLooper) {
        throw std::runtime_error("Expecting to be on main thread!");
    }
    if (isSignaled(DELAY_HAS_PAST)) {
        throw std::runtime_error("recursion detected!");
    }
    if (millisDelay <= 0) {
        throw std::invalid_argument("millisDelay must be greater than 0");
    }
    const int generation = mGeneration;
    // AOSP: postDelayed(new SignalingTask(NO_OP, DELAY_HAS_PAST, generation), millisDelay)
    postDelayed(Runnable([this, generation] {
        sendIdleSignal(DELAY_HAS_PAST, generation);  // SignalingTask.done()
    }), millisDelay);
    loopUntil(DELAY_HAS_PAST);
    loopMainThreadUntilIdle();
}

void UiControllerImpl::handleMessage(Message& msg) {
    // IdleCondition.handleMessage: raise the signal unless it is from a
    // previous generation.
    if (msg.what < 0 || msg.what >= IDLE_CONDITION_COUNT) {
        LOGI("Unknown message type: %d", msg.what);
    } else {
        if (msg.arg1 == mGeneration) {
            mConditionSet.set((size_t)msg.what);
        } else {
            LOGW("ignoring signal of: %d from previous generation: %d current generation: %d",
                 msg.what, msg.arg1, mGeneration);
        }
    }
}

void UiControllerImpl::loopUntil(IdleCondition condition) {
    loopUntil(std::vector<IdleCondition>{condition});
}

void UiControllerImpl::loopUntil(const std::vector<IdleCondition>& conditions) {
    if (mLooping) {
        throw std::runtime_error("Recursive looping detected!");
    }
    mLooping = true;
    int loopCount = 0;
    const int64_t start = SystemClock::uptimeMillis();
    const int64_t end = start + MASTER_IDLE_TIMEOUT_MS;
    bool timedOut = true;
    try {
        while (SystemClock::uptimeMillis() < end) {
            bool conditionsMet = true;
            const bool shouldLogConditionState = loopCount > 0 && loopCount % 100 == 0;

            for (IdleCondition condition : conditions) {
                if (!isSignaled(condition)) {
                    conditionsMet = false;
                    if (shouldLogConditionState) {
                        LOGW("Waiting for: %d for %d iterations.", (int)condition, loopCount);
                    } else {
                        break;
                    }
                }
            }

            // Poll-based stand-in for IdlingResourceRegistry's idle-transition
            // callback driving DYNAMIC_TASKS_HAVE_IDLED: satisfy the bit as
            // soon as every resource reports idle, then re-evaluate.
            for (IdleCondition condition : conditions) {
                if (condition == DYNAMIC_TASKS_HAVE_IDLED && !isSignaled(condition)
                        && mIdlingResourceRegistry.allResourcesAreIdle()) {
                    mConditionSet.set((size_t)DYNAMIC_TASKS_HAVE_IDLED);
                }
            }
            if (!conditionsMet) {
                conditionsMet = true;
                for (IdleCondition condition : conditions) {
                    if (!isSignaled(condition)) { conditionsMet = false; break; }
                }
            }

            if (conditionsMet) {
                // AOSP's exact head check (peek): EMPTY / TASK_DUE_LONG means
                // idle; TASK_DUE_SOON and BARRIER keep pumping — nextDue()
                // transparently skips past a barrier, and its remover retires it.
                QueueInterrogator::QueueState queueState =
                        mQueueInterrogator.determineQueueState();
                if (queueState == QueueInterrogator::QueueState::EMPTY
                        || queueState == QueueInterrogator::QueueState::TASK_DUE_LONG) {
                    timedOut = false;
                    break;
                }
            }

            Message* message = mQueueInterrogator.getNextMessage();
            if (message != nullptr) {
                // AOSP: message.getTarget().dispatchMessage(message)
                if (message->target) message->target->dispatchMessage(message);
                message->recycle();
            } else {
                // Nothing due on the Java queue. AOSP blocks inside
                // MessageQueue.next() until the next message is due; the
                // bounded pollOnce tick also runs the EventHandler drains —
                // injected input delivery (enqueue-only seam) needs it.
                int result = mMainLooper->pollOnce(
                        conditionsMet ? QueueInterrogator::LOOKAHEAD_MILLIS : 16);
                (void)result;
            }
            loopCount++;
        }
        if (timedOut) {
            std::vector<std::string> idleConditions;
            for (IdleCondition condition : conditions) {
                if (!isSignaled(condition)) {
                    idleConditions.push_back(std::to_string((int)condition));
                }
            }
            std::string busy;
            for (const auto& name : mIdlingResourceRegistry.getBusyResourceNames()) {
                busy += (busy.empty() ? "" : ", ") + name;
            }
            // AOSP: masterIdlePolicy.handleTimeout(idleConditions, ...)
            throw AppNotIdleException("Looped for " + std::to_string(loopCount)
                    + " iterations over 60 SECONDS. Busy resources: [" + busy + "]");
        }
    } catch (...) {
        mLooping = false;
        mGeneration++;
        for (IdleCondition condition : conditions) {
            resetCondition(condition);
        }
        throw;
    }
    mLooping = false;
    mGeneration++;
    for (IdleCondition condition : conditions) {
        resetCondition(condition);
    }
}

} /*endof namespace espresso*/
} /*endof namespace cdroid*/
