#include <verticalviewpager.h>

using namespace ::deskclock;

namespace cdroid {
namespace deskclock {

/** Applies the vertical page translation, counteracting the default slide. */
class VerticalViewPager::VerticalPageTransformer : public ViewPager::PageTransformer {
public:
    void transformPage(View& view, float position) override {
        const int pageWidth = view.getWidth();
        const int pageHeight = view.getHeight();
        if (position < -1.0f) {
            // This page is way off-screen to the left.
            view.setAlpha(0.0f);
        } else if (position <= 1.0f) {
            view.setAlpha(1.0f);
            // Counteract the default slide transition.
            view.setTranslationX(pageWidth * -position);
            // Set Y position to swipe in from top.
            view.setTranslationY(position * pageHeight);
        } else {
            // This page is way off-screen to the right.
            view.setAlpha(0.0f);
        }
    }
};

VerticalViewPager::VerticalViewPager(Context* context, const AttributeSet* attrs)
    : ViewPager(context, attrs) {
    init();
}

void VerticalViewPager::init() {
    // Make page transit vertical.
    static VerticalPageTransformer sTransformer;
    setPageTransformer(true /* reverseDrawingOrder */, &sTransformer);
    // Get rid of the overscroll drawing that happens on the left and right (the ripple).
    setOverScrollMode(View::OVER_SCROLL_NEVER);
}

bool VerticalViewPager::canScrollHorizontally(int /*direction*/) {
    return false;
}

bool VerticalViewPager::canScrollVertically(int direction) {
    return ViewPager::canScrollHorizontally(direction);
}

bool VerticalViewPager::onInterceptTouchEvent(MotionEvent& ev) {
    const bool toIntercept = ViewPager::onInterceptTouchEvent(flipXY(ev));
    // Return MotionEvent to its normal orientation.
    flipXY(ev);
    return toIntercept;
}

bool VerticalViewPager::onTouchEvent(MotionEvent& ev) {
    const bool toHandle = ViewPager::onTouchEvent(flipXY(ev));
    // Return MotionEvent to its normal orientation.
    flipXY(ev);
    return toHandle;
}

MotionEvent& VerticalViewPager::flipXY(MotionEvent& ev) {
    const float width = (float) getWidth();
    const float height = (float) getHeight();

    const float x = ev.getY() / height * width;
    const float y = ev.getX() / width * height;

    ev.setLocation(x, y);
    return ev;
}

} // namespace deskclock

typedef cdroid::deskclock::VerticalViewPager VerticalViewPager;
DECLARE_WIDGET2(VerticalViewPager, "VerticalViewPager");

} // namespace cdroid
