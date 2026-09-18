#ifndef CDROID_ESPRESSO_MOTIONEVENTS_H
#define CDROID_ESPRESSO_MOTIONEVENTS_H

/*
 * android.support.test.espresso.action.MotionEvents — facilitates sending of
 * motion events to a UiController.
 */

#include <app/espresso/actioninterfaces.h>

namespace cdroid {
class MotionEvent;

namespace espresso {

class InjectEventSecurityException;
class UiController;

class MotionEvents {
public:
    /** Holds the result of a down motion. */
    struct DownResultHolder {
        MotionEvent* down;
        bool longPress;
    };

    static DownResultHolder sendDown(UiController& uiController,
            const FloatCoord& coordinates, const FloatCoord& precision);

    static bool sendUp(UiController& uiController, MotionEvent& downEvent);
    static bool sendUp(UiController& uiController, MotionEvent& downEvent,
            const FloatCoord& coordinates);

    static void sendCancel(UiController& uiController, MotionEvent& downEvent);

    static bool sendMovement(UiController& uiController, MotionEvent& downEvent,
            const FloatCoord& coordinates);

    static constexpr int MAX_CLICK_ATTEMPTS = 3;

private:
    MotionEvents() = default;
};

} /*endof namespace espresso*/
} /*endof namespace cdroid*/

#endif /*CDROID_ESPRESSO_MOTIONEVENTS_H*/
