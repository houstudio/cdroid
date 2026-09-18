// Port of com.wm.remusic.widget.SildingFinishLayout — the iOS-style
// swipe-right-to-finish surface. Deviation: the content translates via the
// RenderNode (translationX, settled with a ValueAnimator) instead of the
// original's parent scrollBy + Scroller computeScroll choreography — same
// look, none of the scroll plumbing. Like the original, the activity's
// content becomes this layout's child.
#ifndef __REMUSIC_SILDINGFINISH_H__
#define __REMUSIC_SILDINGFINISH_H__

#include <functional>

#include <widget/framelayout.h>

namespace remusic {

class SildingFinishLayout : public cdroid::FrameLayout {
public:
    explicit SildingFinishLayout(cdroid::Context* ctx);

    void setOnSildingFinishListener(std::function<void()> l) { mOnFinish = std::move(l); }

    bool onInterceptTouchEvent(cdroid::MotionEvent& event) override;
    bool onTouchEvent(cdroid::MotionEvent& event) override;

private:
    void settleTo(float targetX);

    int mDownX = 0;
    int mDownY = 0;
    bool mSilding = false;
    std::function<void()> mOnFinish;
};

} // namespace remusic
#endif
