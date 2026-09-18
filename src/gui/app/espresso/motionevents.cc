#include <app/espresso/motionevents.h>

#include <stdexcept>

#include <app/espresso/espressoexception.h>
#include <app/espresso/uicontroller.h>

#include <core/systemclock.h>
#include <porting/cdlog.h>
#include <view/motionevent.h>
#include <view/viewconfiguration.h>

namespace cdroid {
namespace espresso {

static const char* TAG = "MotionEvents";

MotionEvents::DownResultHolder MotionEvents::sendDown(UiController& uiController,
        const FloatCoord& coordinates, const FloatCoord& precision) {
    for (int retry = 0; retry < MAX_CLICK_ATTEMPTS; retry++) {
        MotionEvent* motionEvent = nullptr;
        try {
            // Algorithm of sending click event adopted from android.test.TouchUtils.
            // When the click event was first initiated. Needs to be same for both
            // down and up press events.
            const int64_t downTime = SystemClock::uptimeMillis();

            // Down press.
            motionEvent = MotionEvent::obtain(downTime,
                    SystemClock::uptimeMillis(),
                    MotionEvent::ACTION_DOWN,
                    coordinates[0],
                    coordinates[1],
                    0,             // pressure
                    1,             // size
                    0,             // metaState
                    precision[0],  // xPrecision
                    precision[1],  // yPrecision
                    0,             // deviceId
                    0,             // edgeFlags
                    0,             // source
                    0);            // displayId
            // The down event should be considered a tap if it is long enough to
            // be detected but short enough not to be a long-press. Assume that
            // TapTimeout is set at least twice the detection time for a tap.
            const int64_t isTapAt = downTime + (ViewConfiguration::getTapTimeout() / 2);

            const bool injectEventSucceeded = uiController.injectMotionEvent(*motionEvent);

            while (true) {
                const int64_t delayToBeTap = isTapAt - SystemClock::uptimeMillis();
                if (delayToBeTap <= 10) {
                    break;
                }
                // Sleep only a fraction of the time, since there may be other
                // events in the UI queue that could cause us to start sleeping
                // late, and then oversleep.
                uiController.loopMainThreadForAtLeast(delayToBeTap / 4);
            }

            bool longPress = false;
            if (SystemClock::uptimeMillis()
                    > (downTime + ViewConfiguration::getLongPressTimeout())) {
                longPress = true;
                LOGE("Overslept and turned a tap into a long press");
            }

            if (!injectEventSucceeded) {
                motionEvent->recycle();
                motionEvent = nullptr;
                continue;
            }

            return DownResultHolder{motionEvent, longPress};
        } catch (InjectEventSecurityException& e) {
            throw PerformException::Builder()
                    .withActionDescription("Send down motion event")
                    .withViewDescription("unknown")  // likely to be replaced by FailureHandler
                    .withCause(std::current_exception())
                    .build();
        }
    }
    throw PerformException::Builder()
            .withActionDescription("click (after 3 attempts)")
            .withViewDescription("unknown")  // likely to be replaced by FailureHandler
            .build();
}

bool MotionEvents::sendUp(UiController& uiController, MotionEvent& downEvent) {
    return sendUp(uiController, downEvent, FloatCoord{downEvent.getX(), downEvent.getY()});
}

bool MotionEvents::sendUp(UiController& uiController, MotionEvent& downEvent,
        const FloatCoord& coordinates) {
    MotionEvent* motionEvent = nullptr;
    try {
        // Up press.
        motionEvent = MotionEvent::obtain(downEvent.getDownTime(),
                SystemClock::uptimeMillis(),
                MotionEvent::ACTION_UP,
                coordinates[0],
                coordinates[1],
                0);
        const bool injectEventSucceeded = uiController.injectMotionEvent(*motionEvent);

        if (!injectEventSucceeded) {
            LOGE("Injection of up event failed (corresponding down event: %lld)",
                 (long long)downEvent.getDownTime());
            return false;
        }
    } catch (InjectEventSecurityException& e) {
        throw PerformException::Builder()
                .withActionDescription("inject up event")
                .withViewDescription("unknown")
                .withCause(std::current_exception())
                .build();
    }
    motionEvent->recycle();
    return true;
}

void MotionEvents::sendCancel(UiController& uiController, MotionEvent& downEvent) {
    MotionEvent* motionEvent = nullptr;
    try {
        // Cancel press.
        motionEvent = MotionEvent::obtain(downEvent.getDownTime(),
                SystemClock::uptimeMillis(),
                MotionEvent::ACTION_CANCEL,
                downEvent.getX(),
                downEvent.getY(),
                0);
        const bool injectEventSucceeded = uiController.injectMotionEvent(*motionEvent);

        if (!injectEventSucceeded) {
            LOGE("Injection of cancel event failed (corresponding down event: %lld)",
                 (long long)downEvent.getDownTime());
            return;
        }
    } catch (InjectEventSecurityException& e) {
        throw PerformException::Builder()
                .withActionDescription("inject cancel event")
                .withViewDescription("unknown")
                .withCause(std::current_exception())
                .build();
    }
    motionEvent->recycle();
}

bool MotionEvents::sendMovement(UiController& uiController, MotionEvent& downEvent,
        const FloatCoord& coordinates) {
    MotionEvent* motionEvent = nullptr;
    try {
        motionEvent = MotionEvent::obtain(downEvent.getDownTime(),
                SystemClock::uptimeMillis(),
                MotionEvent::ACTION_MOVE,
                coordinates[0],
                coordinates[1],
                0);
        const bool injectEventSucceeded = uiController.injectMotionEvent(*motionEvent);

        if (!injectEventSucceeded) {
            LOGE("Injection of motion event failed (corresponding down event: %lld)",
                 (long long)downEvent.getDownTime());
            return false;
        }
    } catch (InjectEventSecurityException& e) {
        throw PerformException::Builder()
                .withActionDescription("inject motion event")
                .withViewDescription("unknown")
                .withCause(std::current_exception())
                .build();
    }
    motionEvent->recycle();
    return true;
}

} /*endof namespace espresso*/
} /*endof namespace cdroid*/
