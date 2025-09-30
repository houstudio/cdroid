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
#include <core/pathmeasure.h>
#include <widgetEx/wear/curvinglayoutcallback.h>
namespace cdroid{

CurvingLayoutCallback::CurvingLayoutCallback(Context* context) {
    mCurvePath = std::make_shared<cdroid::Path>();
    mPathMeasure = new PathMeasure();
    mParentView= nullptr;
    mIsScreenRound = true;//context.getResources().getConfiguration().isScreenRound();
    mXCurveOffset = context->getDimensionPixelSize("@cdroid:dimen/ws_wrv_curve_default_x_offset");
}

CurvingLayoutCallback::~CurvingLayoutCallback(){
    delete mPathMeasure;
}

void CurvingLayoutCallback::onLayoutFinished(View& child, RecyclerView& parent) {
    if (mParentView != &parent || (mParentView != nullptr && (
            mParentView->getWidth() != parent.getWidth()
                    || mParentView->getHeight() != parent.getHeight()))) {
        mParentView = &parent;
        mLayoutWidth = mParentView->getWidth();
        mLayoutHeight = mParentView->getHeight();
    }
    if (mIsScreenRound) {
        maybeSetUpCircularInitialLayout(mLayoutWidth, mLayoutHeight);
        mAnchorOffsetXY[0] = mXCurveOffset;
        mAnchorOffsetXY[1] = child.getHeight() / 2.0f;
        adjustAnchorOffsetXY(child, mAnchorOffsetXY);
        const float minCenter = -(float) child.getHeight() / 2;
        const float maxCenter = mLayoutHeight + (float) child.getHeight() / 2;
        const float range = maxCenter - minCenter;
        const float verticalAnchor = (float) child.getTop() + mAnchorOffsetXY[1];
        const float mYScrollProgress = (verticalAnchor + std::abs(minCenter)) / range;

        mPathMeasure->getPosTan(mYScrollProgress * mPathLength, (PointD*)mPathPoints, (PointD*)mPathTangent);

        const bool topClusterRisk = (std::abs(mPathPoints[1] - mCurveBottom) < EPSILON)
                        && (minCenter < mPathPoints[1]);
        const bool bottomClusterRisk = (std::abs(mPathPoints[1] - mCurveTop) < EPSILON)
                        && (maxCenter > mPathPoints[1]);
        // Continue offsetting the child along the straight-line part of the curve, if it
        // has not gone off the screen when it reached the end of the original curve.
        if (topClusterRisk || bottomClusterRisk) {
            mPathPoints[1] = verticalAnchor;
            mPathPoints[0] = (std::abs(verticalAnchor) * mLineGradient);
        }

        // Offset the View to match the provided anchor point.
        const int newLeft = (int) (mPathPoints[0] - mAnchorOffsetXY[0]);
        child.offsetLeftAndRight(newLeft - child.getLeft());
        const float verticalTranslation = mPathPoints[1] - verticalAnchor;
        child.setTranslationY(verticalTranslation);
    } else {
        child.setTranslationY(0);
    }
}

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
void CurvingLayoutCallback::adjustAnchorOffsetXY(View& child, float* anchorOffsetXY) {
    return;
}

void CurvingLayoutCallback::setRound(bool isScreenRound) {
    mIsScreenRound = isScreenRound;
}

void CurvingLayoutCallback::setOffset(int offset) {
    mXCurveOffset = offset;
}

void CurvingLayoutCallback::maybeSetUpCircularInitialLayout(int width, int height) {
    // The values in this function are custom to the curve we use.
    if (mCurvePathHeight != height) {
        mCurvePathHeight = height;
        mCurveBottom = -0.048f * height;
        mCurveTop = 1.048f * height;
        mLineGradient = 0.5f / 0.048f;
        mCurvePath->reset();
        mCurvePath->move_to(0.5f * width, mCurveBottom);
        mCurvePath->line_to(0.34f * width, 0.075f * height);
        mCurvePath->curve_to(//cubic_to(
            0.22f * width, 0.17f * height,
            0.13f * width, 0.32f * height,
            0.13f * width, height / 2);
        mCurvePath->curve_to(//cubic_to(
            0.13f * width, 0.68f * height,
            0.22f * width, 0.83f * height,
            0.34f * width, 0.925f * height);
        mCurvePath->line_to(width / 2, mCurveTop);
        mPathMeasure->setPath(mCurvePath);//, false);
        mPathLength = mPathMeasure->getLength();
    }
}
}/*endof namespace*/
