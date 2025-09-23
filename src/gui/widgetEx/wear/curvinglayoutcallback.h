#ifndef __CURVING_LAYOUT_CALLBACK_H__
#define __CURVING_LAYOUT_CALLBACK_H__
#include <widgetEx/recyclerview/recyclerview.h>
#include <core/path.h>
namespace cdroid{
class PathMeasure;
class CurvingLayoutCallback{
private:
    static constexpr float EPSILON = 0.001f;

    Path* mCurvePath;
    PathMeasure* mPathMeasure;
    int mCurvePathHeight;
    int mXCurveOffset;
    float mPathLength;
    float mCurveBottom;
    float mCurveTop;
    float mLineGradient;
    float mPathPoints[2];
    float mPathTangent[2];
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
