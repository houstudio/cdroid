#ifndef __DESKCLOCK_VERTICALVIEWPAGER_H__
#define __DESKCLOCK_VERTICALVIEWPAGER_H__
/*********************************************************************************
 * Port of com.android.deskclock.VerticalViewPager — a ViewPager whose pages
 * transit vertically: touch coordinates are swapped X/Y on the way in, and a
 * page transformer counteracts the default horizontal slide.
 *********************************************************************************/
#include <widget/viewpager.h>

namespace cdroid {
namespace deskclock {

class VerticalViewPager : public ViewPager {
private:
    class VerticalPageTransformer;

public:
    VerticalViewPager(Context* context, const AttributeSet* attrs);

    /** @return false since a vertical view pager can never be scrolled horizontally. */
    bool canScrollHorizontally(int direction) override;

    /** @return true iff a normal view pager would support horizontal scrolling at this time. */
    bool canScrollVertically(int direction) override;

protected:
    bool onInterceptTouchEvent(MotionEvent& ev) override;
    bool onTouchEvent(MotionEvent& ev) override;

private:
    /** Swaps the event's x/y into the coordinate space the horizontal pager expects. */
    MotionEvent& flipXY(MotionEvent& ev);

    void init();
};

} // namespace deskclock
} // namespace cdroid

#endif // __DESKCLOCK_VERTICALVIEWPAGER_H__
