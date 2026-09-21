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
#include <cmath>
#include <algorithm>
#include <widget/adapter.h>
#include <widgetEx/widgetex_styleable.h>
#include <widgetEx/wear/pageindicatorview.h>

namespace cdroid{
using namespace cdroid::internal;

DECLARE_WIDGET2(PageIndicatorView, "androidx.wear.widget.drawer.PageIndicatorView");

PageIndicatorView::PageIndicatorView(Context* context):PageIndicatorView(context,nullptr){}

PageIndicatorView::PageIndicatorView(Context* context,const AttributeSet* attrs)
    :PageIndicatorView(context,attrs,0){}

PageIndicatorView::PageIndicatorView(Context* context,const AttributeSet* attrs,int defStyleAttr)
    :View(context,attrs,defStyleAttr)
    ,mFadeInAnimatorListener(this){
    // Wire this view as its own OnPageChangeListener (upstream: the class
    // implements OnPageChangeListener and setPager registers `this`).
    ViewPager::OnPageChangeListener::onPageScrolled =
            [this](int position, float positionOffset, int positionOffsetPixels){
                onPageScrolled(position, positionOffset, positionOffsetPixels);
            };
    ViewPager::OnPageChangeListener::onPageSelected = [this](int position){
        onPageSelected(position);
    };
    ViewPager::OnPageChangeListener::onPageScrollStateChanged = [this](int state){
        onPageScrollStateChanged(state);
    };

    auto a = context->obtainStyledAttributes(attrs, internal::R::styleable::PageIndicatorView,
            defStyleAttr, internal::R::style::WsPageIndicatorViewStyle);
    mDotSpacing = a->getDimensionPixelOffset(
            internal::R::styleable::PageIndicatorView_wsPageIndicatorDotSpacing, 0);
    mDotRadius = a->getDimension(internal::R::styleable::PageIndicatorView_wsPageIndicatorDotRadius, 0);
    mDotRadiusSelected = a->getDimension(
            internal::R::styleable::PageIndicatorView_wsPageIndicatorDotRadiusSelected, 0);
    mDotColor = a->getColor(internal::R::styleable::PageIndicatorView_wsPageIndicatorDotColor, 0);
    mDotColorSelected = a->getColor(
            internal::R::styleable::PageIndicatorView_wsPageIndicatorDotColorSelected, 0);
    mDotFadeOutDelay = a->getInt(
            internal::R::styleable::PageIndicatorView_wsPageIndicatorDotFadeOutDelay, 0);
    mDotFadeOutDuration = a->getInt(
            internal::R::styleable::PageIndicatorView_wsPageIndicatorDotFadeOutDuration, 0);
    mDotFadeInDuration = a->getInt(
            internal::R::styleable::PageIndicatorView_wsPageIndicatorDotFadeInDuration, 0);
    mDotFadeWhenIdle = a->getBoolean(
            internal::R::styleable::PageIndicatorView_wsPageIndicatorDotFadeWhenIdle, false);
    mDotShadowDx = a->getDimension(internal::R::styleable::PageIndicatorView_wsPageIndicatorDotShadowDx, 0);
    mDotShadowDy = a->getDimension(internal::R::styleable::PageIndicatorView_wsPageIndicatorDotShadowDy, 0);
    mDotShadowRadius = a->getDimension(
            internal::R::styleable::PageIndicatorView_wsPageIndicatorDotShadowRadius, 0);
    mDotShadowColor = a->getColor(
            internal::R::styleable::PageIndicatorView_wsPageIndicatorDotShadowColor, 0);
    // a.recycle(); — the CDROID TypedArray is a unique_ptr.

    // mDotPaint = new Paint(Paint.ANTI_ALIAS_FLAG); ...
    mDotPaint.setAntiAlias(true);
    mDotPaint.setColor(mDotColor);
    mDotPaint.setStyle(Paint::FILL);

    mDotPaintSelected.setAntiAlias(true);
    mDotPaintSelected.setColor(mDotColorSelected);
    mDotPaintSelected.setStyle(Paint::FILL);
    mDotPaintShadow.setAntiAlias(true);
    mDotPaintShadowSelected.setAntiAlias(true);

    mCurrentViewPagerState = ViewPager::SCROLL_STATE_IDLE;
    if (isInEditMode()) {
        // When displayed in layout preview:
        // Simulate 5 positions, currently on the 3rd position.
        mNumberOfPositions = 5;
        mSelectedPosition = 2;
        mDotFadeWhenIdle = false;
    }

    if (mDotFadeWhenIdle) {
        mVisible = false;
        animate().alpha(0.f).setStartDelay(2000).setDuration(mDotFadeOutDuration).start();
    } else {
        animate().cancel();
        setAlpha(1.f);
    }
    updateShadows();
}

PageIndicatorView::~PageIndicatorView() {
    // The fade animation must not fire listener callbacks into a half-destroyed
    // view (Java relies on GC here).
    animate().cancel();
}

void PageIndicatorView::updateShadows() {
    updateDotPaint(mDotPaint, mDotPaintShadow, mDotRadius, mDotShadowRadius, mDotColor,
            mDotShadowColor);
    updateDotPaint(mDotPaintSelected, mDotPaintShadowSelected, mDotRadiusSelected,
            mDotShadowRadius, mDotColorSelected, mDotShadowColor);
}

void PageIndicatorView::updateDotPaint(Paint& dotPaint, Paint& shadowPaint, float baseRadius,
        float shadowRadius, int color, int shadowColor) {
    float radius = baseRadius + shadowRadius;
    float shadowStart = baseRadius / radius;
    // new RadialGradient(0, 0, radius, {shadowColor, shadowColor, TRANSPARENT},
    //                     {0f, shadowStart, 1f}, TileMode.CLAMP)
    Cairo::RefPtr<Cairo::RadialGradient> gradient =
            Cairo::RadialGradient::create(0, 0, 0, 0, 0, radius);
    const int colors[] = {shadowColor, shadowColor, Color::TRANSPARENT};
    const float stops[] = {0.f, shadowStart, 1.f};
    for (int i = 0; i < 3; i++) {
        gradient->add_color_stop_rgba(stops[i], Color::red(colors[i]) / 255.0,
                Color::green(colors[i]) / 255.0, Color::blue(colors[i]) / 255.0,
                Color::alpha(colors[i]) / 255.0);
    }
    gradient->set_extend(Cairo::Pattern::Extend::PAD);   // Shader.TileMode.CLAMP

    shadowPaint.setShader(gradient);
    dotPaint.setColor(color);
    dotPaint.setStyle(Paint::FILL);
}

/**
 * Supplies the ViewPager instance, and attaches this views OnPageChangeListener to the
 * pager.
 */
void PageIndicatorView::setPager(ViewPager* pager) {
    pager->addOnPageChangeListener(*this);
    setPagerAdapter(pager->getAdapter());
    mAdapter = pager->getAdapter();
    if (mAdapter != nullptr && mAdapter->getCount() > 0) {
        positionChanged(0);
    }
}

/**
 * Gets the center-to-center distance between page dots.
 */
float PageIndicatorView::getDotSpacing() const {
    return mDotSpacing;
}

/**
 * Sets the center-to-center distance between page dots.
 */
void PageIndicatorView::setDotSpacing(int spacing) {
    if (mDotSpacing != spacing) {
        mDotSpacing = spacing;
        requestLayout();
    }
}

/**
 * Gets the radius of the page dots.
 */
float PageIndicatorView::getDotRadius() const {
    return mDotRadius;
}

/**
 * Sets the radius of the page dots.
 */
void PageIndicatorView::setDotRadius(int radius) {
    if (mDotRadius != radius) {
        mDotRadius = radius;
        updateShadows();
        invalidate();
    }
}

/**
 * Gets the radius of the page dot for the selected page.
 */
float PageIndicatorView::getDotRadiusSelected() const {
    return mDotRadiusSelected;
}

/**
 * Sets the radius of the page dot for the selected page.
 */
void PageIndicatorView::setDotRadiusSelected(int radius) {
    if (mDotRadiusSelected != radius) {
        mDotRadiusSelected = radius;
        updateShadows();
        invalidate();
    }
}

/**
 * Returns the color used for dots other than the selected page.
 */
int PageIndicatorView::getDotColor() const {
    return mDotColor;
}

/**
 * Sets the color used for dots other than the selected page.
 */
void PageIndicatorView::setDotColor(int color) {
    if (mDotColor != color) {
        mDotColor = color;
        invalidate();
    }
}

/**
 * Returns the color of the dot for the selected page.
 */
int PageIndicatorView::getDotColorSelected() const {
    return mDotColorSelected;
}

/**
 * Sets the color of the dot for the selected page.
 */
void PageIndicatorView::setDotColorSelected(int color) {
    if (mDotColorSelected != color) {
        mDotColorSelected = color;
        invalidate();
    }
}

/**
 * Indicates if the dots fade out when the pager is idle.
 */
bool PageIndicatorView::getDotFadeWhenIdle() const {
    return mDotFadeWhenIdle;
}

/**
 * Sets whether the dots fade out when the pager is idle.
 */
void PageIndicatorView::setDotFadeWhenIdle(bool fade) {
    mDotFadeWhenIdle = fade;
    if (!fade) {
        fadeIn();
    }
}

/**
 * Returns the duration of fade out animation, in milliseconds.
 */
int PageIndicatorView::getDotFadeOutDuration() const {
    return mDotFadeOutDuration;
}

/**
 * Sets the duration of the fade out animation.
 */
void PageIndicatorView::setDotFadeOutDuration(int duration, TimeUnit unit) {
    mDotFadeOutDuration = (int)toMillis(duration, unit);   // TimeUnit.MILLISECONDS.convert
}

/**
 * Returns the duration of the fade in duration, in milliseconds.
 */
int PageIndicatorView::getDotFadeInDuration() const {
    return mDotFadeInDuration;
}

/**
 * Sets the duration of the fade in animation.
 */
void PageIndicatorView::setDotFadeInDuration(int duration, TimeUnit unit) {
    mDotFadeInDuration = (int)toMillis(duration, unit);   // TimeUnit.MILLISECONDS.convert
}

/**
 * Sets the delay between the pager arriving at an idle state, and the fade out animation
 * beginning, in milliseconds.
 */
int PageIndicatorView::getDotFadeOutDelay() const {
    return mDotFadeOutDelay;
}

/**
 * Sets the delay between the pager arriving at an idle state, and the fade out animation
 * beginning, in milliseconds.
 */
void PageIndicatorView::setDotFadeOutDelay(int delay) {
    mDotFadeOutDelay = delay;
}

/**
 * Sets the pixel radius of shadows drawn beneath the dots.
 */
float PageIndicatorView::getDotShadowRadius() const {
    return mDotShadowRadius;
}

/**
 * Sets the pixel radius of shadows drawn beneath the dots.
 */
void PageIndicatorView::setDotShadowRadius(float radius) {
    if (mDotShadowRadius != radius) {
        mDotShadowRadius = radius;
        updateShadows();
        invalidate();
    }
}

/**
 * Returns the horizontal offset of shadows drawn beneath the dots.
 */
float PageIndicatorView::getDotShadowDx() const {
    return mDotShadowDx;
}

/**
 * Sets the horizontal offset of shadows drawn beneath the dots.
 */
void PageIndicatorView::setDotShadowDx(float dx) {
    mDotShadowDx = dx;
    invalidate();
}

/**
 * Returns the vertical offset of shadows drawn beneath the dots.
 */
float PageIndicatorView::getDotShadowDy() const {
    return mDotShadowDy;
}

/**
 * Sets the vertical offset of shadows drawn beneath the dots.
 */
void PageIndicatorView::setDotShadowDy(float dy) {
    mDotShadowDy = dy;
    invalidate();
}

/**
 * Returns the color of the shadows drawn beneath the dots.
 */
int PageIndicatorView::getDotShadowColor() const {
    return mDotShadowColor;
}

/**
 * Sets the color of the shadows drawn beneath the dots.
 */
void PageIndicatorView::setDotShadowColor(int color) {
    mDotShadowColor = color;
    updateShadows();
    invalidate();
}

void PageIndicatorView::positionChanged(int position) {
    mSelectedPosition = position;
    invalidate();
}

void PageIndicatorView::updateNumberOfPositions() {
    int count = mAdapter->getCount();
    if (count != mNumberOfPositions) {
        mNumberOfPositions = count;
        requestLayout();
    }
}

void PageIndicatorView::fadeIn() {
    mVisible = true;
    animate().cancel();
    animate().alpha(1.f).setStartDelay(0).setDuration(mDotFadeInDuration).start();
}

void PageIndicatorView::fadeOut(long delayMillis) {
    mVisible = false;
    animate().cancel();
    animate().alpha(0.f).setStartDelay(delayMillis).setDuration(mDotFadeOutDuration).start();
}

void PageIndicatorView::fadeInOut() {
    mVisible = true;
    animate().cancel();
    // .setListener(new SimpleAnimatorListener() { onAnimationComplete ... })
    animate().alpha(1.f)
            .setStartDelay(0)
            .setDuration(mDotFadeInDuration)
            .setListener(mFadeInAnimatorListener)
            .start();
}

void PageIndicatorView::onPageScrolled(int position, float positionOffset, int positionOffsetPixels) {
    if (mDotFadeWhenIdle) {
        if (mCurrentViewPagerState == ViewPager::SCROLL_STATE_DRAGGING) {
            if (positionOffset != 0) {
                if (!mVisible) {
                    fadeIn();
                }
            } else {
                if (mVisible) {
                    fadeOut(0);
                }
            }
        }
    }
}

void PageIndicatorView::onPageSelected(int position) {
    if (position != mSelectedPosition) {
        positionChanged(position);
    }
}

void PageIndicatorView::onPageScrollStateChanged(int state) {
    if (mCurrentViewPagerState != state) {
        mCurrentViewPagerState = state;
        if (mDotFadeWhenIdle) {
            if (state == ViewPager::SCROLL_STATE_IDLE) {
                if (mVisible) {
                    fadeOut(mDotFadeOutDelay);
                } else {
                    fadeInOut();
                }
            }
        }
    }
}

/**
 * Sets the PagerAdapter.
 */
void PageIndicatorView::setPagerAdapter(PagerAdapter* adapter) {
    mAdapter = adapter;
    if (mAdapter != nullptr) {
        updateNumberOfPositions();
        if (mDotFadeWhenIdle) {
            fadeInOut();
        }
    }
}

void PageIndicatorView::onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
    int totalWidth;
    if (MeasureSpec::getMode(widthMeasureSpec) == MeasureSpec::EXACTLY) {
        totalWidth = MeasureSpec::getSize(widthMeasureSpec);
    } else {
        int contentWidth = mNumberOfPositions * mDotSpacing;
        totalWidth = contentWidth + getPaddingLeft() + getPaddingRight();
    }
    int totalHeight;
    if (MeasureSpec::getMode(heightMeasureSpec) == MeasureSpec::EXACTLY) {
        totalHeight = MeasureSpec::getSize(heightMeasureSpec);
    } else {
        float maxRadius = std::max(mDotRadius + mDotShadowRadius, mDotRadiusSelected + mDotShadowRadius);
        int contentHeight = (int)std::ceil(maxRadius * 2);
        contentHeight = (int)(contentHeight + mDotShadowDy);
        totalHeight = contentHeight + getPaddingTop() + getPaddingBottom();
    }
    setMeasuredDimension(
            resolveSizeAndState(totalWidth, widthMeasureSpec, 0),
            resolveSizeAndState(totalHeight, heightMeasureSpec, 0));
}

void PageIndicatorView::onDraw(Canvas& canvas) {
    View::onDraw(canvas);

    if (mNumberOfPositions > 1) {
        float dotCenterLeft = getPaddingLeft() + (mDotSpacing / 2.f);
        float dotCenterTop = getHeight() / 2.f;
        canvas.save();
        canvas.translate(dotCenterLeft, dotCenterTop);
        for (int i = 0; i < mNumberOfPositions; i++) {
            if (i == mSelectedPosition) {
                float radius = mDotRadiusSelected + mDotShadowRadius;
                drawCircle(canvas, mDotShadowDx, mDotShadowDy, radius, mDotPaintShadowSelected);
                drawCircle(canvas, 0, 0, mDotRadiusSelected, mDotPaintSelected);
            } else {
                float radius = mDotRadius + mDotShadowRadius;
                drawCircle(canvas, mDotShadowDx, mDotShadowDy, radius, mDotPaintShadow);
                drawCircle(canvas, 0, 0, mDotRadius, mDotPaint);
            }
            canvas.translate(mDotSpacing, 0);
        }
        canvas.restore();
    }
}

/**
 * Notifies the view that the data set has changed.
 */
void PageIndicatorView::notifyDataSetChanged() {
    if (mAdapter != nullptr && mAdapter->getCount() > 0) {
        updateNumberOfPositions();
    }
}

int64_t PageIndicatorView::toMillis(int64_t duration, TimeUnit unit) {
    // TimeUnit.MILLISECONDS.convert(duration, unit)
    switch (unit) {
        case TimeUnit::NANOSECONDS:  return duration / 1000000;
        case TimeUnit::MICROSECONDS: return duration / 1000;
        case TimeUnit::MILLISECONDS: return duration;
        case TimeUnit::SECONDS:      return duration * 1000;
        case TimeUnit::MINUTES:      return duration * 60 * 1000;
        case TimeUnit::HOURS:        return duration * 60 * 60 * 1000;
        case TimeUnit::DAYS:         return duration * 24 * 60 * 60 * 1000;
    }
    return duration;
}

void PageIndicatorView::drawCircle(Canvas& canvas, float cx, float cy, float radius,
        const Paint& paint) {
    if (paint.getShader()) {
        canvas.set_source(paint.getShader());
    } else {
        canvas.set_color(paint.getColor());
    }
    canvas.arc(cx, cy, radius, 0, M_PI * 2);
    canvas.fill();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// PageIndicatorView::FadeInAnimatorListener
///////////////////////////////////////////////////////////////////////////////////////////////////

PageIndicatorView::FadeInAnimatorListener::FadeInAnimatorListener(PageIndicatorView* view)
    : mView(view) {
}

void PageIndicatorView::FadeInAnimatorListener::onAnimationComplete(Animator& animator) {
    mView->mVisible = false;
    mView->animate().alpha(0.f)
            .setListener(Animator::AnimatorListener())   // setListener(null)
            .setStartDelay(mView->mDotFadeOutDelay)
            .setDuration(mView->mDotFadeOutDuration)
            .start();
}

}/*endof namespace*/
