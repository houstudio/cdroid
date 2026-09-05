// Port of com.wm.remusic.widget.SildingFinishLayout — see sildingfinish.h.
#include "sildingfinish.h"

#include <cmath>

#include <animation/valueanimator.h>
#include <view/motionevent.h>
#include <view/viewconfiguration.h>

using namespace cdroid;

namespace remusic {

SildingFinishLayout::SildingFinishLayout(Context* ctx) : FrameLayout(ctx) {}

bool SildingFinishLayout::onInterceptTouchEvent(MotionEvent& event) {
    // Steal horizontally-directed drags from children (lists) once the slop
    // says right; vertical intent stays with the child.
    switch (event.getActionMasked()) {
    case MotionEvent::ACTION_DOWN:
        mDownX = (int) event.getX();
        mDownY = (int) event.getY();
        mSilding = false;
        break;
    case MotionEvent::ACTION_MOVE: {
        const int dx = (int) event.getX() - mDownX;
        const int dy = (int) event.getY() - mDownY;
        const int slop = ViewConfiguration::get(getContext()).getScaledTouchSlop();
        if (dx > slop && std::abs(dy) < slop) {
            mSilding = true;
            return true;
        }
        break;
    }
    default:
        break;
    }
    return false;
}

bool SildingFinishLayout::onTouchEvent(MotionEvent& event) {
    switch (event.getActionMasked()) {
    case MotionEvent::ACTION_MOVE:
        if (mSilding) {
            const float x = event.getX() - mDownX;
            setTranslationX(std::max(0.f, x));
        }
        break;
    case MotionEvent::ACTION_UP:
    case MotionEvent::ACTION_CANCEL: {
        const bool finish = getTranslationX() > getWidth() / 3.f;
        settleTo(finish ? (float) getWidth() : 0.f);
        if (finish && mOnFinish) {
            // Fire after the slide-out starts, like the original's
            // onSildingFinish at animation end.
            postDelayed(mOnFinish, 150);
        }
        mSilding = false;
        break;
    }
    default:
        break;
    }
    return true;
}

void SildingFinishLayout::settleTo(float targetX) {
    auto* anim = ValueAnimator::ofFloat({getTranslationX(), targetX});
    anim->setDuration(180);
    anim->addUpdateListener([this](ValueAnimator& animator) {
        setTranslationX(nonstd::get<float>(animator.getAnimatedValue()));
    });
    anim->start();
}

} // namespace remusic
