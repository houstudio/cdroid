/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/
#ifndef __PAGE_INDICATOR_VIEW_H__
#define __PAGE_INDICATOR_VIEW_H__
#include <core/canvas.h>
#include <core/rect.h>
#include <view/view.h>
#include <text/paint.h>
#include <widget/viewpager.h>
#include <widgetEx/wear/simpleanimatorlistener.h>
namespace cdroid{

class PagerAdapter;

// Line-aligned port of androidx.wear.widget.drawer.PageIndicatorView
// (drawer/PageIndicatorView.java). A page indicator for ViewPager which
// identifies the current page in relation to all available pages. Pages are
// represented as dots; the current page can be highlighted with a different
// color or size dot. The default behavior is to fade out the dots when the
// pager is idle (not settling or being dragged) — see setDotFadeWhenIdle.
// Use setPager(ViewPager*) to connect this view to a pager instance.
class PageIndicatorView:public View,public ViewPager::OnPageChangeListener{
public:
    // Minimal java.util.concurrent.TimeUnit subset for the fade setters.
    enum class TimeUnit{
        NANOSECONDS, MICROSECONDS, MILLISECONDS, SECONDS, MINUTES, HOURS, DAYS
    };

    PageIndicatorView(Context* context);
    PageIndicatorView(Context* context,const AttributeSet* attrs);
    PageIndicatorView(Context* context,const AttributeSet* attrs,int defStyleAttr);
    ~PageIndicatorView() override;

    /**
     * Supplies the ViewPager instance, and attaches this views OnPageChangeListener to the
     * pager.
     */
    void setPager(ViewPager* pager);

    /**
     * Gets the center-to-center distance between page dots.
     * @return the distance between page dots
     */
    float getDotSpacing() const;

    /**
     * Sets the center-to-center distance between page dots.
     * @param spacing the distance between page dots
     */
    void setDotSpacing(int spacing);

    /**
     * Gets the radius of the page dots.
     * @return the radius of the page dots
     */
    float getDotRadius() const;

    /**
     * Sets the radius of the page dots.
     * @param radius the radius of the page dots
     */
    void setDotRadius(int radius);

    /**
     * Gets the radius of the page dot for the selected page.
     * @return the radius of the selected page dot
     */
    float getDotRadiusSelected() const;

    /**
     * Sets the radius of the page dot for the selected page.
     * @param radius the radius of the selected page dot
     */
    void setDotRadiusSelected(int radius);

    /**
     * Returns the color used for dots other than the selected page.
     */
    int getDotColor() const;

    /**
     * Sets the color used for dots other than the selected page.
     */
    void setDotColor(int color);

    /**
     * Returns the color of the dot for the selected page.
     */
    int getDotColorSelected() const;

    /**
     * Sets the color of the dot for the selected page.
     */
    void setDotColorSelected(int color);

    /**
     * Indicates if the dots fade out when the pager is idle.
     */
    bool getDotFadeWhenIdle() const;

    /**
     * Sets whether the dots fade out when the pager is idle.
     */
    void setDotFadeWhenIdle(bool fade);

    /**
     * Returns the duration of fade out animation, in milliseconds.
     */
    int getDotFadeOutDuration() const;

    /**
     * Sets the duration of the fade out animation.
     */
    void setDotFadeOutDuration(int duration, TimeUnit unit);

    /**
     * Returns the duration of the fade in duration, in milliseconds.
     */
    int getDotFadeInDuration() const;

    /**
     * Sets the duration of the fade in animation.
     */
    void setDotFadeInDuration(int duration, TimeUnit unit);

    /**
     * Sets the delay between the pager arriving at an idle state, and the fade out animation
     * beginning, in milliseconds.
     */
    int getDotFadeOutDelay() const;

    /**
     * Sets the delay between the pager arriving at an idle state, and the fade out animation
     * beginning, in milliseconds.
     */
    void setDotFadeOutDelay(int delay);

    /**
     * Sets the pixel radius of shadows drawn beneath the dots.
     */
    float getDotShadowRadius() const;

    /**
     * Sets the pixel radius of shadows drawn beneath the dots.
     */
    void setDotShadowRadius(float radius);

    /**
     * Returns the horizontal offset of shadows drawn beneath the dots.
     */
    float getDotShadowDx() const;

    /**
     * Sets the horizontal offset of shadows drawn beneath the dots.
     */
    void setDotShadowDx(float dx);

    /**
     * Returns the vertical offset of shadows drawn beneath the dots.
     */
    float getDotShadowDy() const;

    /**
     * Sets the vertical offset of shadows drawn beneath the dots.
     */
    void setDotShadowDy(float dy);

    /**
     * Returns the color of the shadows drawn beneath the dots.
     */
    int getDotShadowColor() const;

    /**
     * Sets the color of the shadows drawn beneath the dots.
     */
    void setDotShadowColor(int color);

    // OnPageChangeListener (upstream @Override methods; the CDROID listener is
    // a value-semantics EventSet — the ctor assigns its function members to
    // forward into these).

    void onPageScrolled(int position, float positionOffset, int positionOffsetPixels);

    void onPageSelected(int position);

    void onPageScrollStateChanged(int state);

    /**
     * Sets the PagerAdapter.
     */
    void setPagerAdapter(PagerAdapter* adapter);

    /**
     * Notifies the view that the data set has changed.
     */
    void notifyDataSetChanged();

    // package-private in Java (library-internal state)
    int mDotFadeOutDelay = 0;
    int mDotFadeOutDuration = 0;
    bool mVisible = false;
protected:
    void onMeasure(int widthMeasureSpec, int heightMeasureSpec) override;
    void onDraw(Canvas& canvas) override;
private:
    static constexpr const char* TAG = "Dots";

    Paint mDotPaint;
    Paint mDotPaintShadow;
    Paint mDotPaintSelected;
    Paint mDotPaintShadowSelected;
    int mDotSpacing = 0;
    float mDotRadius = 0;
    float mDotRadiusSelected = 0;
    int mDotColor = 0;
    int mDotColorSelected = 0;
    bool mDotFadeWhenIdle = false;
    int mDotFadeInDuration = 0;
    float mDotShadowDx = 0;
    float mDotShadowDy = 0;
    float mDotShadowRadius = 0;
    int mDotShadowColor = 0;
    PagerAdapter* mAdapter = nullptr;
    int mNumberOfPositions = 0;
    int mSelectedPosition = 0;
    int mCurrentViewPagerState = 0;

    void updateShadows();
    void updateDotPaint(Paint& dotPaint, Paint& shadowPaint, float baseRadius,
            float shadowRadius, int color, int shadowColor);
    void positionChanged(int position);
    void updateNumberOfPositions();
    void fadeIn();
    void fadeOut(long delayMillis);
    void fadeInOut();
    static int64_t toMillis(int64_t duration, TimeUnit unit);
    // android.graphics.Canvas.drawCircle(cx, cy, radius, paint) — honors the
    // paint's shader (gradient) or its color.
    static void drawCircle(Canvas& canvas, float cx, float cy, float radius, const Paint& paint);

    // Upstream: an anonymous SimpleAnimatorListener subclass inside fadeInOut()
    // that fades back out once the fade-in completes. The base listener's
    // EventSet hooks dispatch through the object itself, so it must outlive the
    // animation — one stable by-value member replaces the per-call anonymous
    // instance (and sidesteps the non-virtual-dtor delete).
    class FadeInAnimatorListener: public SimpleAnimatorListener {
    private:
        PageIndicatorView* mView;
    public:
        explicit FadeInAnimatorListener(PageIndicatorView* view);
        void onAnimationComplete(Animator& animator) override;
    };
    FadeInAnimatorListener mFadeInAnimatorListener;
};

}/*endof namespace*/
#endif/*__PAGE_INDICATOR_VIEW_H__*/
