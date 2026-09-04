#ifndef __DESKCLOCK_CIRCLEVIEW_H__
#define __DESKCLOCK_CIRCLEVIEW_H__
/*********************************************************************************
 * Port of com.android.deskclock.widget.CircleView — a View that draws primitive
 * circles (fill color, center/radius or gravity-driven), with Property wrappers
 * for animated fillColor/radius (alarm pulse).
 *********************************************************************************/
#include <animation/property.h>
#include <view/view.h>

namespace cdroid {
namespace deskclock {

class CircleView : public View {
private:
    /** The ARGB color used to fill the circle (mCirclePaint.color upstream). */
    int mCircleColor;

    /** the current Gravity used to align/size the circle */
    int mGravity;

    float mCenterX;
    float mCenterY;

    /** the radius of the circle */
    float mRadius;

public:
    CircleView(Context* ctx, const AttributeSet* attrs);

    int getGravity() const { return mGravity; }
    float getRadius() const { return mRadius; }

    void onRtlPropertiesChanged(int layoutDirection) override;

    void onLayout(bool changed, int left, int top, int right, int bottom) override;

    void onDraw(Canvas& canvas) override;

    bool hasOverlappingRendering() const override;

    /** Describes how to align/size the circle relative to the view's bounds. */
    CircleView& setGravity(int gravity);

    /** @return the ARGB color used to fill the circle */
    int getFillColor() const { return mCircleColor; }

    /** Sets the ARGB fill color, invalidating only the affected area. */
    CircleView& setFillColor(int color);

    /** Sets the x-coordinate for the circle center; clears horizontal gravity. */
    CircleView& setCenterX(float centerX);

    /** Sets the y-coordinate for the circle center; clears vertical gravity. */
    CircleView& setCenterY(float centerY);

    /** Sets the radius; clears the fill gravity flags. */
    CircleView& setRadius(float radius);

    /** Property wrapper around fillColor (FILL_COLOR). */
    static const Property* FILL_COLOR();

    /** Property wrapper around radius (RADIUS). */
    static const Property* RADIUS();

private:
    /** Invalidates the rect that circumscribes the circle. */
    void invalidateCircle(float centerX, float centerY, float radius);

    /** Applies gravity/layoutDirection to the circle's alignment and size. */
    void applyGravity(int gravity, int layoutDirection);
};

} // namespace deskclock
} // namespace cdroid

#endif // __DESKCLOCK_CIRCLEVIEW_H__
