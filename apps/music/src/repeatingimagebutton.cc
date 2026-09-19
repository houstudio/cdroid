#include <repeatingimagebutton.h>

#include <core/systemclock.h>
#include <view/layoutinflater.h>
#include <widget/internal_R.h>

namespace cdroid {
namespace music {

// Layout tag is "com.android.music.RepeatingImageButton" (see touchinterceptor.cc
// for the last-segment registration note).
DECLARE_WIDGET2(RepeatingImageButton, "RepeatingImageButton");

// FQCN twin for the layout's original spelling (exact-first resolution).
static const int sRepeatingImageButtonFqcn = (LayoutInflater::registerInflater(
        "com.android.music.RepeatingImageButton",
        [](Context* ctx, const AttributeSet& attr) -> View* {
    return new RepeatingImageButton(ctx, &attr);
}), 0);

RepeatingImageButton::RepeatingImageButton(Context* ctx)
    : RepeatingImageButton(ctx, nullptr) {
}

RepeatingImageButton::RepeatingImageButton(Context* ctx, const AttributeSet* attrs)
    : ImageButton(ctx, attrs) {
    setFocusable(true);
    setLongClickable(true);
}

RepeatingImageButton::RepeatingImageButton(Context* ctx, const AttributeSet* attrs,
        int defStyleAttr)
    : ImageButton(ctx, attrs, defStyleAttr) {
    setFocusable(true);
    setLongClickable(true);
}

void RepeatingImageButton::setRepeatListener(const RepeatListener& l, long interval) {
    mListener = l;
    mInterval = interval;
}

bool RepeatingImageButton::performLongClick() {
    mStartTime = SystemClock::elapsedRealtime();
    mRepeatCount = 0;
    mRepeater = Runnable([this] {
        doRepeat(false);
        if (isPressed()) postDelayed(mRepeater, mInterval);
    });
    post(mRepeater);
    return true;
}

bool RepeatingImageButton::onTouchEvent(MotionEvent& event) {
    if (event.getAction() == MotionEvent::ACTION_UP) {
        // remove the repeater, but call the hook one more time
        removeCallbacks(mRepeater);
        if (mStartTime != 0) {
            doRepeat(true);
            mStartTime = 0;
        }
    }
    return ImageButton::onTouchEvent(event);
}

bool RepeatingImageButton::onKeyDown(int keyCode, KeyEvent& event) {
    if (keyCode == KeyEvent::KEYCODE_DPAD_CENTER || keyCode == KeyEvent::KEYCODE_ENTER) {
        // need to call super to make long press work, but return
        // true so that the application doesn't get the down event.
        ImageButton::onKeyDown(keyCode, event);
        return true;
    }
    return ImageButton::onKeyDown(keyCode, event);
}

bool RepeatingImageButton::onKeyUp(int keyCode, KeyEvent& event) {
    if (keyCode == KeyEvent::KEYCODE_DPAD_CENTER || keyCode == KeyEvent::KEYCODE_ENTER) {
        removeCallbacks(mRepeater);
        if (mStartTime != 0) {
            doRepeat(true);
            mStartTime = 0;
        }
    }
    return ImageButton::onKeyUp(keyCode, event);
}

void RepeatingImageButton::doRepeat(bool last) {
    const long now = SystemClock::elapsedRealtime();
    if (mListener) {
        mListener(*this, now - mStartTime, last ? -1 : mRepeatCount++);
    }
}

} // namespace music
} // namespace cdroid
