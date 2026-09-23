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
#include <widgetEx/wear/arclayout.h>
#include <widgetEx/widgetex_styleable.h>
namespace cdroid{
ArcLayout::LayoutParams::LayoutParams(Context* context, const AttributeSet& attrs)
    :ViewGroup::MarginLayoutParams(context, attrs){

    // ArcLayout.java:170-182 (binary AXML styleable; enums compile to ints)
    auto a = context->obtainStyledAttributes(&attrs, internal::R::styleable::ArcLayoutLayout);
    if (a) {
        mRotated = a->getBoolean(internal::R::styleable::ArcLayoutLayout_layout_rotate, true);
        mVerticalAlignment = a->getInt(internal::R::styleable::ArcLayoutLayout_layout_valign, VERTICAL_ALIGN_CENTER);
        mWeight = a->getFloat(internal::R::styleable::ArcLayoutLayout_layout_weight, 0.f);
    } else {
        mRotated = true;
        mVerticalAlignment = VERTICAL_ALIGN_CENTER;
        mWeight = 0.f;
    }
}

ArcLayout::LayoutParams::LayoutParams(int width, int height)
    :ViewGroup::MarginLayoutParams(width, height){
}

ArcLayout::LayoutParams::LayoutParams(const ViewGroup::LayoutParams& source)
    :ViewGroup::MarginLayoutParams(source){
}

bool ArcLayout::LayoutParams::isRotated() const{
    return mRotated;
}

void ArcLayout::LayoutParams::setRotated(bool rotated) {
    mRotated = rotated;
}

int ArcLayout::LayoutParams::getVerticalAlignment() const{
    return mVerticalAlignment;
}

void ArcLayout::LayoutParams::setVerticalAlignment(int verticalAlignment) {
    mVerticalAlignment = verticalAlignment;
}

float ArcLayout::LayoutParams::getWeight() const{
    return mWeight;
}

void ArcLayout::LayoutParams::setWeight(float weight) {
    mWeight = weight;
}

/////////////////////////////////////////////////////////////////////////////////////////////////

DECLARE_WIDGET2(ArcLayout, "androidx.wear.widget.ArcLayout");

ArcLayout::Widget::~Widget() {}

ArcLayout::ArcLayout(Context* context,const AttributeSet* attrs):ArcLayout(context,attrs,0){}

ArcLayout::ArcLayout(Context* context,const AttributeSet* pAttrs,int defStyleAttr)
    :ViewGroup(context, pAttrs, defStyleAttr){
    // androidx ArcLayout.java:309 — final field, constructed inline.
    mChildArcAngles = new ChildArcAngles();

    // ArcLayout.java:328-342 (binary AXML styleable; enums compile to ints)
    auto a = context->obtainStyledAttributes(pAttrs, internal::R::styleable::ArcLayout, defStyleAttr);
    if (a) {
        mAnchorType = a->getInt(internal::R::styleable::ArcLayout_anchorPosition, DEFAULT_ANCHOR_TYPE);
        mAnchorAngleDegrees = a->getFloat(internal::R::styleable::ArcLayout_anchorAngleDegrees, DEFAULT_START_ANGLE_DEGREES);
        mClockwise = a->getBoolean(internal::R::styleable::ArcLayout_clockwise, DEFAULT_LAYOUT_DIRECTION_IS_CLOCKWISE);
    } else {
        mAnchorType = DEFAULT_ANCHOR_TYPE;
        mAnchorAngleDegrees = DEFAULT_START_ANGLE_DEGREES;
        mClockwise = DEFAULT_LAYOUT_DIRECTION_IS_CLOCKWISE;
    }
}

ArcLayout::~ArcLayout() {
    delete mChildArcAngles;
}

void ArcLayout::requestLayout() {
    ViewGroup::requestLayout();

    for (int i = 0; i < getChildCount(); i++) {
        getChildAt(i)->forceLayout();
    }
}

void ArcLayout::onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
    // Need to derive the thickness of the curve from the children. We're a curve, so the
    // children can only be sized up to (width or height)/2 units. This currently only
    // supports fitting to a circle.
    //
    // No matter what, fit to the given size, be it a maximum or a fixed size. It doesn't make
    // sense for this container to wrap its children.
    int actualWidthPx = MeasureSpec::getSize(widthMeasureSpec);
    int actualHeightPx = MeasureSpec::getSize(heightMeasureSpec);

    if (MeasureSpec::getMode(widthMeasureSpec) == MeasureSpec::UNSPECIFIED
            && MeasureSpec::getMode(heightMeasureSpec) == MeasureSpec::UNSPECIFIED) {
        // We can't actually resolve this.
        // Let's fit to the screen dimensions, for need of anything better...
        const DisplayMetrics displayMetrics = getContext()->getDisplayMetrics();
        actualWidthPx = displayMetrics.widthPixels;
        actualHeightPx = displayMetrics.heightPixels;
    }

    // Fit to a square.
    if (actualWidthPx < actualHeightPx) {
        actualHeightPx = actualWidthPx;
    } else if (actualHeightPx < actualWidthPx) {
        actualWidthPx = actualHeightPx;
    }

    int maxChildDimension = actualHeightPx / 2;

    // Measure all children in the new measurespec, and cache the largest.
    int childMeasureSpec = MeasureSpec::makeMeasureSpec(maxChildDimension, MeasureSpec::AT_MOST);

    // We need to do two measure passes. First, we need to measure all "normal" children, and
    // get the thickness of all "CurvedContainer" children. Once we have that, we know the
    // maximum thickness, and we can lay out the "CurvedContainer" children, taking into
    // account their vertical alignment.
    int maxChildHeightPx = 0;
    int childState = 0;
    for (int i = 0; i < getChildCount(); i++) {
        View* child = getChildAt(i);

        if (child->getVisibility() == GONE) {
            continue;
        }

        // ArcLayoutWidget is a special case. Because of how it draws, fit it to the size
        // of the whole widget.
        int childMeasuredHeight;
        if (dynamic_cast<ArcLayout::Widget*>(child) != nullptr) {
            childMeasuredHeight = dynamic_cast<ArcLayout::Widget*>(child)->getThickness();
        } else {
            measureChild(child,
                    getChildMeasureSpec(childMeasureSpec, 0, child->getLayoutParams()->width),
                    getChildMeasureSpec(childMeasureSpec, 0, child->getLayoutParams()->height)
            );
            childMeasuredHeight = child->getMeasuredHeight();
            childState = combineMeasuredStates(childState, child->getMeasuredState());

        }
        LayoutParams* childLayoutParams = (LayoutParams*) child->getLayoutParams();
        maxChildHeightPx = std::max(maxChildHeightPx, childMeasuredHeight
                + childLayoutParams->topMargin + childLayoutParams->bottomMargin);
    }

    mThicknessPx = maxChildHeightPx;

    // And now do the pass for the ArcLayoutWidgets
    for (int i = 0; i < getChildCount(); i++) {
        View* child = getChildAt(i);

        if (child->getVisibility() == GONE) {
            continue;
        }
        if (dynamic_cast<ArcLayout::Widget*>(child) != nullptr) {
            LayoutParams* childLayoutParams = (LayoutParams*) child->getLayoutParams();

            const float insetPx = getChildTopInset(child);

            const int innerChildMeasureSpec = MeasureSpec::makeMeasureSpec(
                            maxChildDimension * 2 - round(insetPx * 2), MeasureSpec::EXACTLY);

            measureChild(child,
                    getChildMeasureSpec(innerChildMeasureSpec, 0, childLayoutParams->width),
                    getChildMeasureSpec(innerChildMeasureSpec, 0, childLayoutParams->height)
            );

            childState = combineMeasuredStates(childState, child->getMeasuredState());
        }
    }

    setMeasuredDimension(
            resolveSizeAndState(actualWidthPx, widthMeasureSpec, childState),
            resolveSizeAndState(actualHeightPx, heightMeasureSpec, childState));
}

void ArcLayout::onLayout(bool changed, int l, int t, int r, int b) {
    const bool isLayoutRtl = getLayoutDirection() == LAYOUT_DIRECTION_RTL;

    // != is equivalent to xor, we want to invert clockwise when the layout is rtl
    const float multiplier = mClockwise != isLayoutRtl ? 1.f : -1.f;

    // Layout the children in the arc, computing the center angle where they should be drawn.
    float currentCumulativeAngle = calculateInitialRotation(multiplier);

    // Compute the sum of any weights and the sum of the angles take up by fixed sized children.
    // Unfortunately we can't move this to measure because calculateArcAngle relies upon
    // getMeasuredWidth() which returns 0 in measure.
    float totalAngle = 0.f;
    float weightSum = 0.f;
    for (int i = 0; i < getChildCount(); i++) {
        View* child = getChildAt(i);

        if (child->getVisibility() == GONE) {
            continue;
        }

        LayoutParams* childLayoutParams = (LayoutParams*) child->getLayoutParams();
        if (childLayoutParams->mWeight > 0) {
            weightSum += childLayoutParams->mWeight;
            calculateArcAngle(child, mChildArcAngles);
            totalAngle +=
                    mChildArcAngles->leftMarginAsAngle + mChildArcAngles->rightMarginAsAngle;
        } else {
            calculateArcAngle(child, mChildArcAngles);
            totalAngle += mChildArcAngles->getTotalAngle();
        }
    }

    float weightMultiplier = 0.f;
    if (weightSum > 0.f) {
        weightMultiplier = (mMaxAngleDegrees - totalAngle) / weightSum;
    }

    // Now perform the layout.
    for (int i = 0; i < getChildCount(); i++) {
        View* child = getChildAt(i);

        if (child->getVisibility() == GONE) {
            continue;
        }

        calculateArcAngle(child, mChildArcAngles);
        LayoutParams* childLayoutParams = (LayoutParams*) child->getLayoutParams();
        if (childLayoutParams->mWeight > 0) {
            mChildArcAngles->actualChildAngle = childLayoutParams->mWeight * weightMultiplier;
            if (dynamic_cast<ArcLayout::Widget*>(child) != nullptr) {
                // NB we need to be careful since the child itself may set this value dueing
                // measure.
                dynamic_cast<ArcLayout::Widget*>(child)->setSweepAngleDegrees(mChildArcAngles->actualChildAngle);
            } else {
                throw std::invalid_argument("ArcLayout.LayoutParams with non zero weights"
                        " are only supported for views implementing ArcLayout.Widget");
            }
        }
        const float preRotation = mChildArcAngles->leftMarginAsAngle
                + mChildArcAngles->actualChildAngle / 2.f;
        const float middleAngle = multiplier * (currentCumulativeAngle + preRotation);
        childLayoutParams->mMiddleAngle = middleAngle;

        // Distance from the center of the ArcLayout to the center of the child widget
        float centerToCenterDistance = (getMeasuredHeight() - child->getMeasuredHeight()) / 2
                - getChildTopInset(child);
        // Move the center of the widget in the circle centered on this ArcLayout, and with
        // radius centerToCenterDistance
        childLayoutParams->mCenterX = (float) (getMeasuredWidth() / 2.f
                        + centerToCenterDistance * std::sin(middleAngle * M_PI / 180.f));
        childLayoutParams->mCenterY  = (float) (getMeasuredHeight() / 2.f
                        - centerToCenterDistance * std::cos(middleAngle * M_PI / 180.f));

        currentCumulativeAngle += mChildArcAngles->getTotalAngle();

        // Curved container widgets have been measured so that the "arc" inside their widget
        // will touch the outside of the box they have been measured in, taking into account
        // the vertical alignment. Just grow them from the center.
        if (dynamic_cast<ArcLayout::Widget*>(child) != nullptr) {
            const int leftPx = std::round((getMeasuredWidth() / 2.f) - (child->getMeasuredWidth() / 2.f));
            const int topPx = std::round((getMeasuredHeight() / 2.f) - (child->getMeasuredHeight() / 2.f));

            child->layout(leftPx,topPx, child->getMeasuredWidth(), child->getMeasuredHeight());
        } else 
        {
            // Normal widget's centers need to be placed on their final position,
            // the only thing left for drawing is to maybe rotate them.
            const int leftPx = std::round(childLayoutParams->mCenterX - child->getMeasuredWidth() / 2.f);
            const int topPx = std::round(childLayoutParams->mCenterY - child->getMeasuredHeight() / 2.f);

            child->layout(leftPx, topPx, child->getMeasuredWidth(), child->getMeasuredHeight());
        }
    }
}

bool ArcLayout::onInterceptTouchEvent(MotionEvent& event) {
    if ((mTouchedView == nullptr) && event.getActionMasked() == MotionEvent::ACTION_DOWN) {
        for (int i = 0; i < getChildCount(); i++) {
            View* child = getChildAt(i);
            // Ensure that the view is visible
            if (child->getVisibility() != VISIBLE) {
                continue;
            }

            // Map the event to the child's coordinate system
            LayoutParams* childLayoutParams = (LayoutParams*) child->getLayoutParams();
            float angle = childLayoutParams->mMiddleAngle;

            float point[2]={event.getX(), event.getY()};
            mapPoint(child, angle, point);

            // Check if the click is actually in the child area
            float x = point[0];
            float y = point[1];

            if (insideChildClickArea(child, x, y)) {
                mTouchedView = child;
                break;
            }
        }
    }
    // We can't do normal dispatching because it will capture touch in the original position
    // of children.
    return true;
}

bool ArcLayout::insideChildClickArea(View* child, float x, float y) {
    if (dynamic_cast<ArcLayout::Widget*>(child) != nullptr) {
        return dynamic_cast<ArcLayout::Widget*>(child)->isPointInsideClickArea(x, y);
    }
    return (x >= 0) && (x < child->getMeasuredWidth()) && (y >= 0) && (y < child->getMeasuredHeight());
}

// Map a point to local child coordinates.
void ArcLayout::mapPoint(View* child, float angle, float* point) {
    Cairo::Matrix m = Cairo::identity_matrix();

    LayoutParams* childLayoutParams = (LayoutParams*) child->getLayoutParams();
    if (dynamic_cast<ArcLayout::Widget*>(child) != nullptr) {
        // java postRotate(-a, c) postTranslate(-x,-y): M = T(-x,-y)·R_c — cairo
        // composes later calls first, so rotate-about-c then translate.
        m.translate(getMeasuredWidth() / 2.f, getMeasuredHeight() / 2.f);
        m.rotate(-angle * M_PI / 180.f);
        m.translate(-getMeasuredWidth() / 2.f, -getMeasuredHeight() / 2.f);
        m.translate(-child->getX(), -child->getY());
    } else {
        // java postTranslate(-c) postRotate(-a) postTranslate(+k):
        // M = T(k)·R·T(-c) — cairo call order is the reverse point-application order.
        m.translate(child->getWidth() / 2.f, child->getHeight() / 2.f);
        if (childLayoutParams->isRotated()) {
            m.rotate(-angle * M_PI / 180.f);
        }
        m.translate(-childLayoutParams->mCenterX, -childLayoutParams->mCenterY);
    }
    double x = point[0];
    double y = point[1];
    m.transform_point(x,y);
    point[0] = x;
    point[1] = y;
}

bool ArcLayout::onTouchEvent(MotionEvent& event) {
    if (mTouchedView != nullptr) {
        // Map the event's coordinates to the child's coordinate space
        float point[2]={event.getX(), event.getY()};
        LayoutParams* touchedViewLayoutParams = (LayoutParams*) mTouchedView->getLayoutParams();
        mapPoint(mTouchedView, touchedViewLayoutParams->mMiddleAngle, point);

        const float dx = point[0] - event.getX();
        const float dy = point[1] - event.getY();
        event.offsetLocation(dx, dy);

        mTouchedView->dispatchTouchEvent(event);

        if ((event.getActionMasked() == MotionEvent::ACTION_UP)
                || (event.getActionMasked() == MotionEvent::ACTION_CANCEL)) {
            // We have finished handling these series of events.
            mTouchedView = nullptr;
        }
        return true;
    }
    return false;
}

bool ArcLayout::drawChild(Canvas& canvas, View* child, int64_t drawingTime) {
    // Rotate the canvas to make the children render in the right place.
    canvas.save();

    LayoutParams* childLayoutParams = (LayoutParams*) child->getLayoutParams();
    float middleAngle = childLayoutParams->mMiddleAngle;
    if (dynamic_cast<ArcLayout::Widget*>(child) != nullptr) {
        // Rotate the child widget. This rotation places child widget in its correct place in
        // the circle. Rotation is done around the center of the circle that components make.
        const float cw = getMeasuredWidth() / 2.f, chh = getMeasuredHeight() / 2.f;
        canvas.translate(cw, chh);
        canvas.rotate_degrees(middleAngle);
        canvas.translate(-cw, -chh);

        dynamic_cast<ArcLayout::Widget*>(child)->checkInvalidAttributeAsChild();
    } else {
        // Normal components already have their center in the right position during layout,
        // the only thing remaining is any needed rotation.
        // This rotation is done in place around the center of the
        // child to adjust it based on rotation and clockwise attributes.
        const float angleToRotate = childLayoutParams->isRotated()
                ? middleAngle + (mClockwise ? 0.f : 180.f)
                : 0.f;

        //canvas.rotate(angleToRotate, childLayoutParams->mCenterX, childLayoutParams->mCenterY);
        canvas.translate(childLayoutParams->mCenterX, childLayoutParams->mCenterY);
        canvas.rotate_degrees(angleToRotate);
        canvas.translate(-childLayoutParams->mCenterX, -childLayoutParams->mCenterY);
    }
    bool wasInvalidateIssued = ViewGroup::drawChild(canvas, child, drawingTime);

    canvas.restore();

    return wasInvalidateIssued;
}

float ArcLayout::calculateInitialRotation(float multiplier) {
    if (mAnchorType == ANCHOR_START) {
        return multiplier * mAnchorAngleDegrees;
    }

    float totalArcAngle = 0;

    bool hasWeights = false;
    for (int i = 0; i < getChildCount(); i++) {
        View* child = getChildAt(i);
        LayoutParams* childLayoutParams = (LayoutParams*) child->getLayoutParams();
        if (childLayoutParams->getWeight() > 0.f) {
            hasWeights = true;
        }
        calculateArcAngle(child, mChildArcAngles);
        totalArcAngle += mChildArcAngles->getTotalAngle();
    }

    if (hasWeights && (totalArcAngle < mMaxAngleDegrees)) {
        totalArcAngle = mMaxAngleDegrees;
    }

    if (mAnchorType == ANCHOR_CENTER) {
        return multiplier * mAnchorAngleDegrees - (totalArcAngle / 2.f);
    } else if (mAnchorType == ANCHOR_END) {
        return multiplier * mAnchorAngleDegrees - totalArcAngle;
    }

    return 0;
}

float ArcLayout::widthToAngleDegrees(float widthPx, float radiusPx) {
    return (float) (2.f * std::asin(widthPx / radiusPx / 2.f))*180.f/M_PI;
}

void ArcLayout::calculateArcAngle(View* view, ChildArcAngles* childAngles) {
    if (view->getVisibility() == GONE) {
        childAngles->leftMarginAsAngle = 0;
        childAngles->rightMarginAsAngle = 0;
        childAngles->actualChildAngle = 0;
        return;
    }

    float radiusPx = (getMeasuredWidth() / 2.f) - mThicknessPx;

    LayoutParams* childLayoutParams = (LayoutParams*) view->getLayoutParams();

    childAngles->leftMarginAsAngle =  widthToAngleDegrees(childLayoutParams->leftMargin, radiusPx);
    childAngles->rightMarginAsAngle=  widthToAngleDegrees(childLayoutParams->rightMargin, radiusPx);
    if (dynamic_cast<ArcLayout::Widget*>(view) != nullptr) {
        childAngles->actualChildAngle = dynamic_cast<ArcLayout::Widget*>(view)->getSweepAngleDegrees();
    } else {
        childAngles->actualChildAngle = widthToAngleDegrees(view->getMeasuredWidth(), radiusPx);
    }
}

float ArcLayout::getChildTopInset(View* child) {
    LayoutParams* childLayoutParams = (LayoutParams*) child->getLayoutParams();

    int childHeight = dynamic_cast<ArcLayout::Widget*>(child) != nullptr
            ? dynamic_cast<ArcLayout::Widget*>(child)->getThickness()
            : child->getMeasuredHeight();

    int thicknessDiffPx = mThicknessPx - childLayoutParams->topMargin
              - childLayoutParams->bottomMargin - childHeight;

    int margin = mClockwise ? childLayoutParams->topMargin : childLayoutParams->bottomMargin;
    const float topInset = margin + getChildTopOffset(child);

    switch (childLayoutParams->getVerticalAlignment()) {
        case LayoutParams::VERTICAL_ALIGN_OUTER:
            return topInset;
        case LayoutParams::VERTICAL_ALIGN_CENTER:
            return topInset + thicknessDiffPx / 2.f;
        case LayoutParams::VERTICAL_ALIGN_INNER:
            return topInset + thicknessDiffPx;
        default:
            // Normally unreachable...
            return 0;
    }
}

float ArcLayout::getChildTopOffset(View* child) {
    if (
            (dynamic_cast<ArcLayout::Widget*>(child) != nullptr) ||
            (getMeasuredWidth() >= getMeasuredHeight())
            ) {
        return 0;
    }
    return std::round((getMeasuredHeight() - getMeasuredWidth()) / 2.f);
}

bool ArcLayout::checkLayoutParams(const ViewGroup::LayoutParams* p) const{
    return dynamic_cast<const LayoutParams*>(p);
}

ArcLayout::LayoutParams* ArcLayout::generateLayoutParams(const ViewGroup::LayoutParams* p) const{
    return new LayoutParams(*p);
}

ArcLayout::LayoutParams* ArcLayout::generateLayoutParams(const AttributeSet& attrs) const{
    return new LayoutParams(getContext(), attrs);
}

ArcLayout::LayoutParams* ArcLayout::generateDefaultLayoutParams() const{
    return new LayoutParams(LayoutParams::MATCH_PARENT, LayoutParams::MATCH_PARENT);
}

int ArcLayout::getAnchorType() const{
    return mAnchorType;
}

void ArcLayout::setAnchorType(int anchorType) {
    if (anchorType < ANCHOR_START || anchorType > ANCHOR_END) {
        throw std::invalid_argument("Unknown anchor type");
    }

    mAnchorType = anchorType;
    invalidate();
}

float ArcLayout::getAnchorAngleDegrees() const{
    return mAnchorAngleDegrees;
}

/** Sets the anchor angle used for this container, in degrees. */
void ArcLayout::setAnchorAngleDegrees(float anchorAngleDegrees) {
    mAnchorAngleDegrees = anchorAngleDegrees;
    invalidate();
}

float ArcLayout::getMaxAngleDegrees() const{
    return mMaxAngleDegrees;
}

void ArcLayout::setMaxAngleDegrees(float maxAngleDegrees) {
    mMaxAngleDegrees = maxAngleDegrees;
    invalidate();
    requestLayout();
}

bool ArcLayout::isClockwise() const{
    return mClockwise;
}

void ArcLayout::setClockwise(bool clockwise) {
    mClockwise = clockwise;
    invalidate();
}

}/*endof namespace*/
