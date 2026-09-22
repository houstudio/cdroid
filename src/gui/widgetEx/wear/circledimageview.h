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
#ifndef __CIRCLED_IMAGE_VIEW_H__
#define __CIRCLED_IMAGE_VIEW_H__
#include <core/canvas.h>
#include <core/rect.h>
#include <core/attributeset.h>
#include <view/view.h>
#include <text/paint.h>
#include <animation/valueanimator.h>
#include <widgetEx/wear/progressdrawable.h>
namespace cdroid{

class ColorStateList;

// Line-aligned port of androidx.wear.widget.CircledImageView
// (CircledImageView.java:52-800). An image view surrounded by a circle.
class CircledImageView:public View{
public:
    CircledImageView(Context* context);
    CircledImageView(Context* context,const AttributeSet* attrs);
    CircledImageView(Context* context,const AttributeSet* attrs,int defStyle);
    ~CircledImageView() override;

    /** Sets the circle to be hidden. */
    void setCircleHidden(bool circleHidden);

    /** Sets the image given a resource. */
    void setImageResource(int resId);

    /** Sets the size of the image based on a percentage in [0, 1]. */
    void setImageCirclePercentage(float percentage);

    /** Sets the horizontal offset given a percentage in [0, 1]. */
    void setImageHorizontalOffcenterPercentage(float percentage);

    /** Sets the tint. */
    void setImageTint(int tint);

    /** Returns the circle radius. */
    float getCircleRadius() const;

    /** Sets the circle radius. */
    void setCircleRadius(float circleRadius);

    /** Gets the circle radius percent. */
    float getCircleRadiusPercent() const;

    /** Sets the radius of the circle to be a percentage of the largest dimension of the view. */
    void setCircleRadiusPercent(float circleRadiusPercent);

    /** Gets the circle radius when pressed. */
    float getCircleRadiusPressed() const;

    /** Sets the circle radius when pressed. */
    void setCircleRadiusPressed(float circleRadiusPressed);

    /** Gets the circle radius when pressed as a percent. */
    float getCircleRadiusPressedPercent() const;

    /** Sets the radius of the circle to be a percentage of the largest dimension of the view when pressed. */
    void setCircleRadiusPressedPercent(float circleRadiusPressedPercent);

    /** Sets the circle color. */
    void setCircleColor(int circleColor);

    /** Gets the circle color. */
    RefPtr<ColorStateList> getCircleColorStateList() const;

    /** Sets the circle color. */
    void setCircleColorStateList(const RefPtr<ColorStateList>& circleColor);

    /** Gets the default circle color. */
    int getDefaultCircleColor() const;

    /**
     * Show the circle border as an indeterminate progress spinner. The views circle border width
     * and color must be set for this to have an effect.
     */
    void showIndeterminateProgress(bool show);

    /** Sets the progress. */
    void setProgress(float progress);

    /**
     * Set how much of the shadow should be shown.
     * @param shadowVisibility Value between 0 and 1.
     */
    void setShadowVisibility(float shadowVisibility);

    float getInitialCircleRadius() const;

    void setCircleBorderColor(int circleBorderColor);

    /**
     * Set the border around the circle.
     * @param circleBorderWidth Width of the border around the circle.
     */
    void setCircleBorderWidth(float circleBorderWidth);

    /**
     * Set the stroke cap for the border around the circle.
     * @param circleBorderCap Stroke cap (Paint.Cap ordinal: 0=butt, 1=round, 2=square —
     *        the Cairo::Context::LineCap values).
     */
    void setCircleBorderCap(int circleBorderCap);

    // AOSP View.setPressed is overridable; CDROID View::setPressed is not
    // virtual, so this hides the base method — calls through a CircledImageView*
    // run the pressed-radius update, base-class dispatch does not.
    void setPressed(bool pressed);

    void setPadding(int left, int top, int right, int bottom) override;

    Drawable* getImageDrawable();

    /** Sets the image drawable (takes ownership). */
    void setImageDrawable(Drawable* drawable);

    /** @return the milliseconds duration of the transition animation when the color changes. */
    int64_t getColorChangeAnimationDuration() const;

    /**
     * @param durationMillis the milliseconds duration of the color change animation. The color
     * change animation will run if the color changes with setCircleColor or as a result of the
     * active state changing.
     */
    void setColorChangeAnimationDuration(int64_t durationMillis);

    // package-private in Java (library-internal state, e.g. SinglePageUi peers)
    int mCurrentColor = 0;
protected:
    bool onSetAlpha(int alpha) override;
    void onDraw(Canvas& canvas) override;
    void onMeasure(int widthMeasureSpec, int heightMeasureSpec) override;
    // CDROID View::onLayout takes (changed, left, top, width, height).
    void onLayout(bool changed, int left, int top, int width, int height) override;
    void drawableStateChanged() override;
    void onVisibilityChanged(View& changedView, int visibility) override;
    void onWindowVisibilityChanged(int visibility) override;
    void onSizeChanged(int w, int h, int oldw, int oldh) override;
private:
    static constexpr int SQUARE_DIMEN_NONE = 0;
    static constexpr int SQUARE_DIMEN_HEIGHT = 1;
    static constexpr int SQUARE_DIMEN_WIDTH = 2;

    /**
     * Helper class taking care of painting a shadow behind the displayed image.
     * TODO(amad): Replace this with elevation, when moving to support/wearable?
     */
    class OvalShadowPainter {
    public:
        OvalShadowPainter(float shadowWidth, float shadowVisibility,
                float innerCircleRadius, float innerCircleBorderWidth);

        void draw(Canvas& canvas, float alpha);

        void setBounds(int left, int top, int right, int bottom);

        void setInnerCircleRadius(float newInnerCircleRadius);

        void setInnerCircleBorderWidth(float newInnerCircleBorderWidth);

        void setShadowVisibility(float newShadowVisibility);

        // package-private in Java (read by CircledImageView)
        const float mShadowWidth;
        float mShadowVisibility;
    private:
        void updateRadialGradient();

        static constexpr uint32_t SHADER_COLORS[] = {Color::BLACK, Color::TRANSPARENT};
        static constexpr float SHADER_STOPS[] = {0.6f, 1.f};

        RectF mBounds;
        Paint mShadowPaint;
        float mShadowRadius;
        float mInnerCircleRadius;
        float mInnerCircleBorderWidth;
    };

    RectF mOval;
    Paint mPaint;
    OvalShadowPainter* mShadowPainter = nullptr;
    RefPtr<ColorStateList> mCircleColor;
    Drawable* mDrawable = nullptr;
    float mCircleRadius = 0;
    float mCircleRadiusPercent = 0;
    float mCircleRadiusPressed = 0;
    float mCircleRadiusPressedPercent = 0;
    float mRadiusInset = 0;
    int mCircleBorderColor = 0;
    int mCircleBorderCap = 0;   // Paint.Cap (Cairo::Context::LineCap ordinal)
    float mCircleBorderWidth = 0;
    bool mCircleHidden = false;
    float mProgress = 1.f;
    bool mPressed = false;
    bool mProgressIndeterminate = false;
    bool mVisible = false;
    bool mWindowVisible = false;
    int64_t mColorChangeAnimationDurationMs = 0;
    float mImageCirclePercentage = 1.f;
    float mImageHorizontalOffcenterPercentage = 0.f;
    // Java holds these as nullable Integers; CDROID keeps the value plus a set flag.
    int mImageTint = 0;
    bool mImageTintSet = false;
    int mSquareDimen = SQUARE_DIMEN_NONE;
    bool mSquareDimenSet = false;

    float mInitialCircleRadius = 0;

    ProgressDrawable* mIndeterminateDrawable = nullptr;
    Rect mIndeterminateBounds;   // android.graphics.Rect default-constructs to {0,0,0,0}
    // Upstream retains an anonymous Drawable.Callback member (weakly held by the
    // drawable); CDROID View IS a Drawable::Callback whose invalidateDrawable
    // invalidates the view, so `this` is handed to setCallback instead.

    ValueAnimator::AnimatorUpdateListener mAnimationListener;

    ValueAnimator* mColorAnimator = nullptr;

    void setColorForCurrentState();
};

}/*endof namespace*/
#endif/*__CIRCLED_IMAGE_VIEW_H__*/
