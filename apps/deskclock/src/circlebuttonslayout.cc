#include <circlebuttonslayout.h>

#include <R.h>

#include <cmath>

#include <widget/button.h>
#include <widget/textview.h>

#include <utils.h>

using namespace ::deskclock;

namespace cdroid {
namespace deskclock {

CircleButtonsLayout::CircleButtonsLayout(Context* context, const AttributeSet* attrs)
    : FrameLayout(context, attrs) {
    Resources& res = context->getResources();
    const float strokeSize = res.getDimension(R::dimen::circletimer_circle_size);
    const float dotStrokeSize = res.getDimension(R::dimen::circletimer_dot_size);
    const float markerStrokeSize = res.getDimension(R::dimen::circletimer_marker_size);
    mDiamOffset = Utils::calculateRadiusOffset(strokeSize, dotStrokeSize, markerStrokeSize) * 2;
}

void CircleButtonsLayout::onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
    // We must call onMeasure both before and after re-measuring our views because the circle
    // may not always be drawn here yet. The first onMeasure will force the circle to be drawn,
    // and the second will force our re-measurements to take effect.
    FrameLayout::onMeasure(widthMeasureSpec, heightMeasureSpec);
    remeasureViews();
    FrameLayout::onMeasure(widthMeasureSpec, heightMeasureSpec);
}

void CircleButtonsLayout::remeasureViews() {
    if (mLabel == nullptr) {
        mCircleView = findViewById(R::id::timer_time);
        mLabel = (TextView*) findViewById(R::id::timer_label);
        mResetAddButton = (Button*) findViewById(R::id::reset_add);
    }

    if (mCircleView == nullptr) return;

    const int frameWidth = mCircleView->getMeasuredWidth();
    const int frameHeight = mCircleView->getMeasuredHeight();
    const int minBound = std::min(frameWidth, frameHeight);
    const int circleDiam = (int) (minBound - mDiamOffset);

    if (mResetAddButton != nullptr) {
        MarginLayoutParams& resetAddParams =
                *(MarginLayoutParams*) mResetAddButton->getLayoutParams();
        resetAddParams.bottomMargin = circleDiam / 6;
        if (minBound == frameWidth) {
            resetAddParams.bottomMargin += (frameHeight - frameWidth) / 2;
        }
    }

    if (mLabel != nullptr) {
        MarginLayoutParams& labelParams = *(MarginLayoutParams*) mLabel->getLayoutParams();
        labelParams.topMargin = circleDiam / 6;
        if (minBound == frameWidth) {
            labelParams.topMargin += (frameHeight - frameWidth) / 2;
        }
        /* w = 2 * sqrt((r + y) * (r - y)) — see the upstream derivation. */
        // Radius of the circle.
        const double r = circleDiam / 2.0;
        // Y value of the top of the label, calculated from the center of the circle.
        const double y = frameHeight / 2.0 - labelParams.topMargin;
        // New maximum width of the label.
        const double w = (r + y >= 0 && r - y >= 0)
                ? 2.0 * std::sqrt((r + y) * (r - y)) : 0.0;

        mLabel->setMaxWidth((int) w);
    }
}

} // namespace deskclock

typedef cdroid::deskclock::CircleButtonsLayout CircleButtonsLayout;
DECLARE_WIDGET2(CircleButtonsLayout, "CircleButtonsLayout");

} // namespace cdroid
