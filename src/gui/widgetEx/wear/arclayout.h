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
#ifndef __WEAR_ARCLAYOUT_H__
#define __WEAR_ARCLAYOUT_H__
#include <view/viewgroup.h>
namespace cdroid{
class ArcLayout:public ViewGroup {
public:
    struct Widget {
        float getSweepAngleDegrees();
        void setSweepAngleDegrees(float sweepAngleDegrees);
        int getThickness();
        void checkInvalidAttributeAsChild();
        bool isPointInsideClickArea(float x, float y);
    };

    class LayoutParams:public ViewGroup::MarginLayoutParams {
    public:
        static constexpr int VERTICAL_ALIGN_OUTER = 0;
        static constexpr int VERTICAL_ALIGN_CENTER = 1;
        static constexpr int VERTICAL_ALIGN_INNER = 2;
    protected:
        friend ArcLayout;
        bool mRotated = true;
        int mVerticalAlignment = VERTICAL_ALIGN_CENTER;

        float mMiddleAngle;
        float mCenterX;
        float mCenterY;
        float mWeight;
    public:
        LayoutParams(Context* context, const AttributeSet& attrs);
        LayoutParams(int width, int height);
        LayoutParams(const ViewGroup::LayoutParams& source);

        bool isRotated() const;
        void setRotated(bool rotated);

        int getVerticalAlignment() const;
        void setVerticalAlignment(int verticalAlignment);

        float getWeight() const;
        void setWeight(float weight);
    };

    /** Annotation for anchor types. */
public:
    static constexpr int ANCHOR_START = 0;
    static constexpr int ANCHOR_CENTER = 1;
    static constexpr int ANCHOR_END = 2;
private:
    static constexpr float DEFAULT_START_ANGLE_DEGREES = 0.f;
    static constexpr bool DEFAULT_LAYOUT_DIRECTION_IS_CLOCKWISE = true; // clockwise
    static constexpr int DEFAULT_ANCHOR_TYPE = ANCHOR_START;

    class ChildArcAngles {
    public:
        float leftMarginAsAngle;
        float rightMarginAsAngle;
        float actualChildAngle;

        float getTotalAngle() {
            return leftMarginAsAngle + rightMarginAsAngle + actualChildAngle;
        }
    };
    int mThicknessPx = 0;
    int mAnchorType;
    float mAnchorAngleDegrees;
    float mMaxAngleDegrees = 360.0f;
    bool mClockwise;
    ChildArcAngles* mChildArcAngles;
    View* mTouchedView = nullptr;
private:
    static bool insideChildClickArea(View* child, float x, float y);
    void mapPoint(View* child, float angle, float* point);
    float calculateInitialRotation(float multiplier);
    static float widthToAngleDegrees(float widthPx, float radiusPx);
    void calculateArcAngle(View* view, ChildArcAngles* childAngles);
    float getChildTopInset(View* child);
    float getChildTopOffset(View* child);
protected:
    void onMeasure(int widthMeasureSpec, int heightMeasureSpec) override;
    void onLayout(bool changed, int l, int t, int r, int b) override;
    bool drawChild(Canvas& canvas, View* child, int64_t drawingTime) override;
    bool checkLayoutParams(const ViewGroup::LayoutParams* p)const override;
    LayoutParams* generateLayoutParams(const ViewGroup::LayoutParams* p)const override;
    LayoutParams* generateDefaultLayoutParams() const override;
public:
    ArcLayout(Context* context, const AttributeSet& attrs);
    void requestLayout() override;

    bool onInterceptTouchEvent(MotionEvent& event) override;
    bool onTouchEvent(MotionEvent& event) override;

    LayoutParams* generateLayoutParams(const AttributeSet& attrs)const override;

    /** Returns the anchor type used for this container. */
    int getAnchorType() const;
    void setAnchorType(int anchorType);

    /** Returns the anchor angle used for this container, in degrees. */
    float getAnchorAngleDegrees() const;
    void setAnchorAngleDegrees(float anchorAngleDegrees);

    float getMaxAngleDegrees() const;
    void setMaxAngleDegrees(float maxAngleDegrees);

    bool isClockwise() const;
    void setClockwise(bool clockwise);
};
}/*endof namespace*/
#endif/*__WEAR_ARCLAYOUT_H__*/
