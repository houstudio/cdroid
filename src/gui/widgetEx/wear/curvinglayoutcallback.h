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
#ifndef __CURVING_LAYOUT_CALLBACK_H__
#define __CURVING_LAYOUT_CALLBACK_H__
#include <widgetEx/recyclerview/recyclerview.h>
#include <core/path.h>
namespace cdroid{
class PathMeasure;
class CurvingLayoutCallback{
private:
    static constexpr float EPSILON = 0.001f;

    Cairo::RefPtr<cdroid::Path> mCurvePath;
    PathMeasure* mPathMeasure;
    int mCurvePathHeight;
    int mXCurveOffset;
    float mPathLength;
    float mCurveBottom;
    float mCurveTop;
    float mLineGradient;
    double mPathPoints[2];
    double mPathTangent[2];
    float mAnchorOffsetXY[2];

    RecyclerView* mParentView;
    bool mIsScreenRound;
    int mLayoutWidth;
    int mLayoutHeight;
private:
    void maybeSetUpCircularInitialLayout(int width, int height);
public:
    CurvingLayoutCallback(Context* context);
    virtual ~CurvingLayoutCallback();
    void onLayoutFinished(View& child, RecyclerView& parent);

    /**
     * Override this method if you wish to adjust the anchor coordinates for each child view
     * during a layout pass. In the override set the new desired anchor coordinates in
     * the provided array. The coordinates should be provided in relation to the child view.
     *
     * @param child          The child view to which the anchor coordinates will apply.
     * @param anchorOffsetXY The anchor coordinates for the provided child view, by default set
     *                       to a pre-defined constant on the horizontal axis and half of the
     *                       child height on the vertical axis (vertical center).
     */
    void adjustAnchorOffsetXY(View& child, float* anchorOffsetXY);

    void setRound(bool isScreenRound);
    void setOffset(int offset);
};
}/*endof namespace*/
#endif/*__CURVING_LAYOUT_CALLBACK_H__*/
