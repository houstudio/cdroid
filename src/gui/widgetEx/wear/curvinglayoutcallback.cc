#include <widgetEx/wear/curvinglayoutcallback.h>
#include <drawables/hwpathmeasure.h>
namespace cdroid{

CurvingLayoutCallback::CurvingLayoutCallback(Context* context) {
    mCurvePath = new Path();
    mPathMeasure = new PathMeasure();
    mIsScreenRound = true;//context.getResources().getConfiguration().isScreenRound();
    mXCurveOffset = context->getDimensionPixelSize("@cdroid:dimen/ws_wrv_curve_default_x_offset");
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
        float minCenter = -(float) child.getHeight() / 2;
        float maxCenter = mLayoutHeight + (float) child.getHeight() / 2;
        float range = maxCenter - minCenter;
        float verticalAnchor = (float) child.getTop() + mAnchorOffsetXY[1];
        float mYScrollProgress = (verticalAnchor + std::abs(minCenter)) / range;

        mPathMeasure.getPosTan(mYScrollProgress * mPathLength, mPathPoints, mPathTangent);

        bool topClusterRisk =
                std::abs(mPathPoints[1] - mCurveBottom) < EPSILON
                        && minCenter < mPathPoints[1];
        bool bottomClusterRisk =
                std::abs(mPathPoints[1] - mCurveTop) < EPSILON
                        && maxCenter > mPathPoints[1];
        // Continue offsetting the child along the straight-line part of the curve, if it
        // has not gone off the screen when it reached the end of the original curve.
        if (topClusterRisk || bottomClusterRisk) {
            mPathPoints[1] = verticalAnchor;
            mPathPoints[0] = (std::abs(verticalAnchor) * mLineGradient);
        }

        // Offset the View to match the provided anchor point.
        int newLeft = (int) (mPathPoints[0] - mAnchorOffsetXY[0]);
        child.offsetLeftAndRight(newLeft - child.getLeft());
        float verticalTranslation = mPathPoints[1] - verticalAnchor;
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
        mCurvePath.reset();
        mCurvePath.moveTo(0.5f * width, mCurveBottom);
        mCurvePath.lineTo(0.34f * width, 0.075f * height);
        mCurvePath.cubicTo(
                0.22f * width, 0.17f * height, 0.13f * width, 0.32f * height, 0.13f * width,
                height / 2);
        mCurvePath.cubicTo(
                0.13f * width,
                0.68f * height,
                0.22f * width,
                0.83f * height,
                0.34f * width,
                0.925f * height);
        mCurvePath.lineTo(width / 2, mCurveTop);
        mPathMeasure.setPath(mCurvePath, false);
        mPathLength = mPathMeasure->getLength();
    }
}
}/*endof namespace*/
