#include <screensaveractivity.h>

#include <R.h>

#include <core/activityfactory.h>

#include <widget/framelayout.h>
#include <widget/textclock.h>

#include <datamodel.h>
#include <settingsdao.h>

using namespace ::deskclock;

namespace cdroid {
namespace deskclock {

namespace {
constexpr float NIGHT_MODE_ALPHA = 0.15f;
} // namespace

ScreensaverActivity::ScreensaverActivity()
    : Window(0, 0, -1, -1) {
}

void ScreensaverActivity::onCreate(Bundle* savedInstanceState) {
    Window::onCreate(savedInstanceState);

    FrameLayout* content = new FrameLayout(getContext());
    content->setLayoutParams(new ViewGroup::LayoutParams(
            ViewGroup::LayoutParams::MATCH_PARENT, ViewGroup::LayoutParams::MATCH_PARENT));
    // Black saver backdrop.
    content->setBackgroundColor(0xFF000000);
    addView(content);

    const data::ClockStyle style = data::DataModel::getDataModel().getScreensaverClockStyle();
    View* clock = nullptr;
    if (style == data::ClockStyle::ANALOG) {
        AnalogClock* analog = new AnalogClock(getContext());
        analog->setLayoutParams(new FrameLayout::LayoutParams(
                ViewGroup::LayoutParams::WRAP_CONTENT, ViewGroup::LayoutParams::WRAP_CONTENT,
                Gravity::CENTER));
        clock = analog;
    } else {
        TextClock* digital = new TextClock(getContext());
        digital->setLayoutParams(new FrameLayout::LayoutParams(
                ViewGroup::LayoutParams::WRAP_CONTENT, ViewGroup::LayoutParams::WRAP_CONTENT,
                Gravity::CENTER));
        digital->setFormat12Hour("h:mm a");
        digital->setFormat24Hour("HH:mm");
        digital->setTextSize(72.0f);
        digital->setTextColor(0xFFFFFFFF);
        clock = digital;
    }
    // Night mode dims the clock.
    if (data::DataModel::getDataModel().getScreensaverNightModeOn()) {
        clock->setAlpha(NIGHT_MODE_ALPHA);
    }
    content->addView(clock);

    // The saver dismisses on any touch; expose that as a clickable on the root
    // so a11y traversal (the auto-test sweep) can leave the page instead of
    // starving on a screen with no actionable nodes.
    content->setClickable(true);
    content->setContentDescription(
            getContext()->getString(R::string::shortcut_start_screensaver_short));
    content->setOnClickListener([this](View&) { dismissSaver(); });
}

bool ScreensaverActivity::dispatchTouchEvent(MotionEvent& ev) {
    // Any touch dismisses the saver.
    dismissSaver();
    return Window::dispatchTouchEvent(ev);
}

bool ScreensaverActivity::onKeyDown(int keyCode, KeyEvent& event) {
    dismissSaver();
    return Window::onKeyDown(keyCode, event);
}

void ScreensaverActivity::dismissSaver() {
    close();
}

} // namespace deskclock

// Registered under the bare class name the Screensaver menu item's Intent carries.
static const int _cdroid_act_reg_screensaver =
    (::cdroid::ActivityFactory::registerActivity("ScreensaverActivity",
        []() -> ::cdroid::Window* {
            ::cdroid::Window* w = new ::cdroid::deskclock::ScreensaverActivity();
            w->setActivityName("ScreensaverActivity");
            return w;
        }), 0);

} // namespace cdroid
