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
 *********************************************************************************/
#include <view/roundscrollbarrenderer.h>
#include <core/color.h>
#include <porting/cdlog.h>
#include <utils/mathutils.h>
namespace cdroid{

RoundScrollbarRenderer::RoundScrollbarRenderer(View*parent) {
    // Paints for the round scrollbar.
    // Set up the thumb paint
    mPreviousMaxScroll = 0;
    mMaxScrollDiff = 0;
    mPreviousCurrentScroll = 0;
    mCurrentScrollDiff = 0;

    mThumbColor = DEFAULT_THUMB_COLOR;
    mTrackColor = DEFAULT_TRACK_COLOR;
    mParent = parent;
}

void RoundScrollbarRenderer::drawRoundScrollbars(Canvas&canvas, float alpha,const Rect&bounds,bool drawToLeft) {
    if (alpha == 0)return;
    // Get information about the current scroll state of the parent view.
    float maxScroll = mParent->computeVerticalScrollRange();
    float scrollExtent = mParent->computeVerticalScrollExtent();
    float newScroll = mParent->computeVerticalScrollOffset();

    if (scrollExtent <= 0) {
        if (!mParent->canScrollVertically(1) && !mParent->canScrollVertically(-1)) {
            return;
        } else {
            scrollExtent = 0;
        }
    }else  if (scrollExtent <= 0){
        return;
    }

    // Make changes to the VerticalScrollRange happen gradually
    if ((std::abs(maxScroll - mPreviousMaxScroll) > RESIZING_THRESHOLD_PX)
            && (mPreviousMaxScroll != 0)) {
        mMaxScrollDiff += maxScroll - mPreviousMaxScroll;
        mCurrentScrollDiff += newScroll - mPreviousCurrentScroll;
    }

    mPreviousMaxScroll = maxScroll;
    mPreviousCurrentScroll = newScroll;

    if ((std::abs(mMaxScrollDiff) > RESIZING_THRESHOLD_PX)
            || (std::abs(mCurrentScrollDiff) > RESIZING_THRESHOLD_PX)) {
        mMaxScrollDiff *= RESIZING_RATE;
        mCurrentScrollDiff *= RESIZING_RATE;

        maxScroll -= mMaxScrollDiff;
        newScroll -= mCurrentScrollDiff;
    } else {
        mMaxScrollDiff = 0;
        mCurrentScrollDiff = 0;
    }

    const float currentScroll = std::max(0.f, newScroll);
    const float linearThumbLength = scrollExtent;
    const float thumbWidth = dpToPx(THUMB_WIDTH_DP);//mParent->getWidth() * WIDTH_PERCENTAGE;

    setThumbColor(applyAlpha(DEFAULT_THUMB_COLOR, alpha));
    setTrackColor(applyAlpha(DEFAULT_TRACK_COLOR, alpha));

    // Normalize the sweep angle for the scroll bar.
    float sweepAngle = (linearThumbLength / maxScroll) * SCROLLBAR_ANGLE_RANGE;
    sweepAngle = MathUtils::clamp(sweepAngle, MIN_SCROLLBAR_ANGLE_SWIPE, MAX_SCROLLBAR_ANGLE_SWIPE);
    // Normalize the start angle so that it falls on the track.
    float startAngle = (currentScroll * (SCROLLBAR_ANGLE_RANGE - sweepAngle))
            / (maxScroll - linearThumbLength) - SCROLLBAR_ANGLE_RANGE / 2.f;
    startAngle = MathUtils::clamp(startAngle, -SCROLLBAR_ANGLE_RANGE / 2.f, SCROLLBAR_ANGLE_RANGE / 2.f - sweepAngle);

    // Draw the track and the scroll bar.
    mRect = bounds;
    const float inset = thumbWidth/2+mMaskThickness;
    mRect.inset(thumbWidth/2,0);//inflate(-thumbWidth /2,0);

    const double radius = double(mRect.width)/2.0;
    canvas.save();
    canvas.set_line_width(thumbWidth);
    canvas.set_color(mTrackColor);
    canvas.set_antialias(Cairo::ANTIALIAS_DEFAULT);
    canvas.set_line_cap(Cairo::Context::LineCap::ROUND);
    canvas.translate(mRect.centerX(),mRect.centerY());

    if(drawToLeft){
        canvas.scale(1.f,double(mRect.height)/mRect.width);
        canvas.begin_new_sub_path();
        canvas.arc(0,0,radius, (180+SCROLLBAR_ANGLE_RANGE/2.f)*M_PI/180.f,(180-SCROLLBAR_ANGLE_RANGE/2.f)*M_PI/180.f);
        canvas.stroke();

        canvas.set_color(mThumbColor);
        canvas.begin_new_sub_path();
        canvas.arc(0,0,radius, (180-startAngle)*M_PI/180.f,(startAngle+sweepAngle)*M_PI/180.f);
        canvas.stroke();
        canvas.restore();
    }else{
        canvas.scale(1.f,double(mRect.height)/mRect.width);
        canvas.begin_new_sub_path();
        canvas.arc_negative(0,0,radius, -SCROLLBAR_ANGLE_RANGE*M_PI/360.f,SCROLLBAR_ANGLE_RANGE*M_PI/360.f);
        canvas.stroke();

        canvas.set_color(mThumbColor);
        canvas.begin_new_sub_path();
        canvas.arc_negative(0,0,radius, startAngle*M_PI/180.f,(startAngle+sweepAngle)*M_PI/180.f);
        canvas.stroke();
        canvas.restore();
    }
}

void RoundScrollbarRenderer::getRoundVerticalScrollBarBounds(Rect& bounds) {
    const float padding = dpToPx(OUTER_PADDING_DP);
    const int width = mParent->getWidth();//mRight - mParent->mLeft;
    const int height = mParent->getHeight();//mBottom - mParent->mTop;
    bounds.left = mParent->getScrollX() + (int) padding;
    bounds.top = mParent->getScrollY() + (int) padding;
    bounds.width =  width - static_cast<int>(padding*2.f);
    bounds.height = height- static_cast<int>( padding*2.f);
}

int RoundScrollbarRenderer::applyAlpha(int color, float alpha) {
    const int alphaByte = (int) (Color::alpha(color) * alpha);
    return Color::toArgb(Color::red(color), Color::green(color), Color::blue(color),alphaByte);
}

void RoundScrollbarRenderer::setThumbColor(int thumbColor) {
    mThumbColor = thumbColor;
}

void RoundScrollbarRenderer::setTrackColor(int trackColor) {
    mTrackColor = trackColor;
}

float RoundScrollbarRenderer::dpToPx(float dp) {
    return dp * ((float) mParent->getContext()->getDisplayMetrics().densityDpi)
            / DisplayMetrics::DENSITY_DEFAULT;
}
}/*endof namespace*/
