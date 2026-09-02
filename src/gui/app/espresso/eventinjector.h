#ifndef CDROID_ESPRESSO_EVENTINJECTOR_H
#define CDROID_ESPRESSO_EVENTINJECTOR_H

/*
 * android.support.test.espresso.base.EventInjector — sends events on behalf
 * of UiControllerImpl.
 *
 * AOSP holds an Instrumentation and calls sendKeySync/sendPointerSync
 * (blocking RPCs into the system server, run from UiControllerImpl's
 * keyEventExecutor thread). CDROID's injection seam is the in-process
 * UiAutomation (AOSP android.app.UiAutomation.injectInputEvent) whose
 * InputEventSource enqueue plays the InputDispatcher role. Injection is
 * therefore an enqueue on the SAME thread — the dedicated executor thread
 * disappears (documented in uicontrollerimpl.h) but the
 * KEY/MOTION_INJECTION_HAS_COMPLETED idle-condition handshake is kept.
 */

#include <view/keyevent.h>
#include <view/motionevent.h>
#include <core/inputdevice.h>

#include <app/uiautomation.h>

namespace cdroid {
namespace espresso {

class EventInjector {
public:
    virtual ~EventInjector() = default;

    /**
     * @throws InjectEventSecurityException when injection would interact with
     *         another application (AOSP catches SecurityException from
     *         Instrumentation; the in-process seam cannot cross apps).
     */
    virtual bool injectKeyEvent(KeyEvent& event) = 0;
    virtual bool injectMotionEvent(MotionEvent& event) = 0;
};

/** The default EventInjector — backed by the in-process UiAutomation. */
class UiAutomationEventInjector : public EventInjector {
public:
    bool injectKeyEvent(KeyEvent& event) override {
        // AOSP Instrumentation.sendKeySync sets the keyboard source at this
        // seam; without it the dispatch layer drops the event.
        event.setSource(InputDevice::SOURCE_KEYBOARD);
        return UiAutomation::getInstance().injectInputEvent(event, true /* sync */);
    }
    bool injectMotionEvent(MotionEvent& event) override {
        // AOSP Instrumentation.sendPointerSync sets SOURCE_TOUCHSCREEN here —
        // same seam (events built with the default source were dropped before
        // reaching touch dispatch).
        event.setSource(InputDevice::SOURCE_TOUCHSCREEN);
        return UiAutomation::getInstance().injectInputEvent(event, true /* sync */);
    }
};

} /*endof namespace espresso*/
} /*endof namespace cdroid*/

#endif /*CDROID_ESPRESSO_EVENTINJECTOR_H*/
