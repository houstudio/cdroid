#ifndef CDROID_ESPRESSO_UICONTROLLER_H
#define CDROID_ESPRESSO_UICONTROLLER_H

/*
 * android.support.test.espresso.UiController — base-level UI operations
 * (injection of MotionEvents/KeyEvents) used to build user actions such as
 * clicks, scrolls, swipes, plus the main-thread idle synchronization that
 * replaces parts of android.app.Instrumentation.
 */

#include <string>

namespace cdroid {
class KeyEvent;
class MotionEvent;

namespace espresso {

class UiController {
public:
    virtual ~UiController() = default;

    /**
     * Injects a motion event into the application.
     * @return true if the event was injected, false otherwise.
     * @throws InjectEventSecurityException if the event could not be injected
     *         because it would interact with another application.
     */
    virtual bool injectMotionEvent(MotionEvent& event) = 0;

    /**
     * Injects a key event into the application.
     * @return true if the event was injected, false otherwise.
     */
    virtual bool injectKeyEvent(KeyEvent& event) = 0;

    /**
     * Types a string into the application using a series of KeyEvents. It is
     * up to the implementor to decide how to map the string to KeyEvent
     * objects. If you need specific control over the key events generated use
     * injectKeyEvent(KeyEvent).
     */
    virtual bool injectString(const std::string& str) = 0;

    /**
     * Loops the main thread until the application goes idle.
     * An empty task is immediately inserted into the task queue to ensure
     * that if we're idle at this moment we'll return instantly.
     */
    virtual void loopMainThreadUntilIdle() = 0;

    /**
     * Loops the main thread for a specified period of time.
     * Control may not return immediately, instead it'll return after the time
     * has passed and the queue is in an idle state again.
     */
    virtual void loopMainThreadForAtLeast(int64_t millisDelay) = 0;
};

} /*endof namespace espresso*/
} /*endof namespace cdroid*/

#endif /*CDROID_ESPRESSO_UICONTROLLER_H*/
