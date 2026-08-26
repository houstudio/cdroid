/*********************************************************************************
 * Key-sending helpers for the keynav suite: the C++ equivalent of Android
 * instrumentation's sendKeys()/waitForIdleSync(). A logical press is an
 * ACTION_DOWN + ACTION_UP pair sharing a downTime, dispatched through the
 * window so events traverse the same pipeline as real input
 * (Window::dispatchKeyEvent -> focused view -> performFocusNavigation),
 * mirroring AOSP ViewRootImpl's key delivery.
 *
 * Tests that model a full AOSP activity build their tree in a private Window
 * (one activity = one window, so directional focus search never leaks into
 * the harness' shared stage tree); pass it as the target. Default target is
 * the shared stage.
 *********************************************************************************/
#ifndef KEYNAV_KEYSENDER_H
#define KEYNAV_KEYSENDER_H
#include <cdroid.h>
#include <guienvironment.h>

namespace keynav {

inline Window* targetWindow(Window* target) {
    return target ? target : GUIEnvironment::stage();
}

/* instrumentation.sendKeyDownSync / sendKeyUpSync: the two halves of a press,
   for tests that observe the unhandled-key fallthrough of one leg. */
inline bool sendKeyDown(int keyCode, nsecs_t downTime, int metaState = 0, Window* target = nullptr) {
    KeyEvent* e = KeyEvent::obtain(downTime, downTime, KeyEvent::ACTION_DOWN,
            keyCode, 0, metaState, 0, 0, 0, 0);
    const bool handled = targetWindow(target)->dispatchKeyEvent(*e);
    delete e;
    return handled;
}

inline bool sendKeyUp(int keyCode, nsecs_t downTime, int metaState = 0, Window* target = nullptr) {
    KeyEvent* e = KeyEvent::obtain(downTime, SystemClock::uptimeMillis(), KeyEvent::ACTION_UP,
            keyCode, 0, metaState, 0, 0, 0, 0);
    const bool handled = targetWindow(target)->dispatchKeyEvent(*e);
    delete e;
    return handled;
}

/* instrumentation.sendKeySync(keyCode): one full press (DOWN + UP). */
inline bool sendKey(int keyCode, int metaState = 0, Window* target = nullptr) {
    const nsecs_t downTime = SystemClock::uptimeMillis();
    const bool handledDown = sendKeyDown(keyCode, downTime, metaState, target);
    const bool handledUp = sendKeyUp(keyCode, downTime, metaState, target);
    pumpUntilIdle();
    return handledDown || handledUp;
}

/* instrumentation.sendKeys(keyCodes...): presses each key in order. */
inline void sendKeys(std::initializer_list<int> keyCodes, Window* target = nullptr) {
    for (int keyCode : keyCodes) sendKey(keyCode, 0, target);
}

/* instrumentation.sendKeys("N*KEYCODE"): one keycode pressed count times. */
inline void sendRepeatedKeys(int count, int keyCode, Window* target = nullptr) {
    for (int i = 0; i < count; i++) sendKey(keyCode, 0, target);
}

} // namespace keynav
#endif
