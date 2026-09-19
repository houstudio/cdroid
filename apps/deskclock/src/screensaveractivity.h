#ifndef __DESKCLOCK_SCREENSAVERACTIVITY_H__
#define __DESKCLOCK_SCREENSAVERACTIVITY_H__
/*********************************************************************************
 * Port of com.android.deskclock.ScreensaverActivity — the full-screen clock
 * saver. Reduced on cdroid: the saver clock (analog/digital per the
 * screensaver_clock_style preference) is built programmatically; night mode
 * dims the clock; any touch/key dismisses. The upstream MoveScreensaverRunnable
 * drift is deferred (clock stays centered).
 *********************************************************************************/
#include <widget/analogclock.h>
#include <widget/cdwindow.h>

#include <texttime.h>

namespace cdroid {
namespace deskclock {

class ScreensaverActivity : public Window {
private:
    Runnable mMoveSaverCallback;

public:
    ScreensaverActivity();

    void onCreate(Bundle* savedInstanceState) override;
    bool dispatchTouchEvent(MotionEvent& ev) override;
    bool onKeyDown(int keyCode, KeyEvent& event) override;

private:
    void dismissSaver();
};

} // namespace deskclock
} // namespace cdroid

#endif // __DESKCLOCK_SCREENSAVERACTIVITY_H__
