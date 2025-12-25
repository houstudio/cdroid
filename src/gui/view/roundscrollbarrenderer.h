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
#ifndef __ROUND_SCROLLBAR_RENDERER__
#define __ROUND_SCROLLBAR_RENDERER__
#include <view/view.h>
namespace cdroid{
class RoundScrollbarRenderer{
private:
    static constexpr float SCROLLBAR_ANGLE_RANGE = 28.8f;
    static constexpr float MAX_SCROLLBAR_ANGLE_SWIPE = 26.3f;// 90%
    static constexpr float MIN_SCROLLBAR_ANGLE_SWIPE = 3.1f; // 10%
    static constexpr float THUMB_WIDTH_DP = 4.f;
    static constexpr float OUTER_PADDING_DP = 2.f;
    static constexpr int DEFAULT_THUMB_COLOR = 0xFFFFFFFF;
    static constexpr int DEFAULT_TRACK_COLOR = 0x4CFFFFFF;
    // Rate at which the scrollbar will resize itself when the size of the view changes
    static constexpr float RESIZING_RATE = 0.8f;
    // Threshold at which the scrollbar will stop resizing smoothly and jump to the correct size
    static constexpr int RESIZING_THRESHOLD_PX = 20;
private:
    View* mParent;
    Rect mRect;
    int mThumbColor;
    int mTrackColor;
    int mMaskThickness;

    float mPreviousMaxScroll = 0;
    float mMaxScrollDiff = 0;
    float mPreviousCurrentScroll = 0;
    float mCurrentScrollDiff = 0;
    static int applyAlpha(int color, float alpha);
    void setThumbColor(int thumbColor);
    void setTrackColor(int trackColor);
    float dpToPx(float dp);
public:
    RoundScrollbarRenderer(View* parent);
    void drawRoundScrollbars(Canvas& canvas, float alpha,const Rect& bounds,bool drawToLeft);
    void getRoundVerticalScrollBarBounds(Rect& bounds);
};
}//namespace
#define __ROUND_SCROLLBAR_RENDERER__
#endif
