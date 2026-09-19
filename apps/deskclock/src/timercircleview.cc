#include <timercircleview.h>

#include <R.h>

#include <cmath>

#include <content/resources.h>
#include <core/context.h>

#include <timer.h>
#include <utils.h>

using namespace ::deskclock;

namespace cdroid {
namespace deskclock {

TimerCircleView::TimerCircleView(Context* context, const AttributeSet* attrs)
    : View(context, attrs) {
    Resources& resources = context->getResources();
    const float dotDiameter = resources.getDimension(::deskclock::R::dimen::circletimer_dot_size);

    mDotRadius = dotDiameter / 2.0f;
    mStrokeSize = resources.getDimension(::deskclock::R::dimen::circletimer_circle_size);
    mRadiusOffset = Utils::calculateRadiusOffset(mStrokeSize, dotDiameter, 0.0f);

    mRemainderColor = 0xFFFFFFFF; // Color.WHITE
    // Upstream: ThemeUtils.resolveColor(context, R.attr.colorAccent). Like the
    // other DeskClock views, resolve through obtainStyledAttributes: the theme
    // colorAccent is a reference and Theme.resolveAttribute flattens references
    // to pool values (TypedValue.data with resourceId=0) that are transparent
    // when consumed as ARGB.
    mCompletedColor = 0xFFDA4336;
    {
        const uint32_t attrs[] = {0x01010435 /* android:colorAccent */, 0};
        auto ta = context->obtainStyledAttributes(attrs);
        if (ta != nullptr) {
            mCompletedColor = (int) ta->getColor(0, (uint32_t) mCompletedColor);
        }
    }
}

void TimerCircleView::update(const data::Timer& timer) {
    // Upstream invalidates when the Timer instance changes; the by-value model
    // hands out copies, so key the identity check on the timer id and refresh
    // the snapshot every call (onDraw advances the arc off the stored value).
    const bool changed = !mHasTimer || mTimer.id != timer.id;
    mTimer = timer;
    mHasTimer = true;
    if (changed) {
        postInvalidateOnAnimation();
    }
}

void TimerCircleView::onDraw(Canvas& canvas) {
    if (!mHasTimer) {
        return;
    }

    // Compute the size and location of the circle to be drawn.
    const float xCenter = getWidth() / 2.0f;
    const float yCenter = getHeight() / 2.0f;
    const float radius = std::min(xCenter, yCenter) - mRadiusOffset;

    canvas.set_line_width(mStrokeSize);
    const double toRad = M_PI / 180.0;

    // If the timer is reset, draw a simple white circle.
    float redPercent;
    if (mTimer.isReset()) {
        // Draw a complete white circle; no red arc required.
        canvas.set_color(mRemainderColor);
        canvas.arc(xCenter, yCenter, radius, 0.0, 2.0 * M_PI);
        canvas.stroke();

        // Red percent is 0 since no timer progress has been made.
        redPercent = 0.0f;
    } else if (mTimer.isExpired()) {
        canvas.set_color(mCompletedColor);

        // Draw a complete accent circle; no white arc required.
        canvas.arc(xCenter, yCenter, radius, 0.0, 2.0 * M_PI);
        canvas.stroke();

        // Red percent is 1 since the timer has expired.
        redPercent = 1.0f;
    } else {
        // Draw a combination of accent and white arcs to create a circle.
        redPercent = std::min(1.0f,
                (float) mTimer.getElapsedTime() / (float) mTimer.totalLength);
        const float whitePercent = 1.0f - redPercent;

        // Draw a white arc to indicate the amount of timer that remains.
        canvas.set_color(mRemainderColor);
        {
            const double start = (270.0 + redPercent * 360.0) * toRad;
            const double end = start + whitePercent * 360.0 * toRad;
            canvas.arc(xCenter, yCenter, radius, start, end);
            canvas.stroke();
        }

        // Draw an accent arc to indicate the amount of timer completed.
        canvas.set_color(mCompletedColor);
        {
            const double start = 270.0 * toRad;
            const double end = start + redPercent * 360.0 * toRad;
            canvas.arc(xCenter, yCenter, radius, start, end);
            canvas.stroke();
        }
    }

    // Draw an accent dot to indicate current progress through the timer.
    const double dotAngleDegrees = 270.0 - redPercent * 360.0;
    const double dotAngleRadians = dotAngleDegrees * toRad;
    const double dotX = xCenter + radius * cos(dotAngleRadians);
    const double dotY = yCenter + radius * sin(dotAngleRadians);
    canvas.set_color(mCompletedColor);
    canvas.arc(dotX, dotY, mDotRadius, 0.0, 2.0 * M_PI);
    canvas.fill();

    if (mTimer.isRunning()) {
        postInvalidateOnAnimation();
    }
}

} // namespace deskclock

typedef cdroid::deskclock::TimerCircleView TimerCircleView;
DECLARE_WIDGET2(TimerCircleView, "TimerCircleView");

} // namespace cdroid
