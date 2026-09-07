#include <stopwatchcircleview.h>

#include <R.h>

#include <cmath>

#include <content/resources.h>
#include <core/context.h>

#include <datamodel.h>
#include <utils.h>

using namespace ::deskclock;

namespace cdroid {
namespace deskclock {
namespace stopwatch {

using data::DataModel;

StopwatchCircleView::StopwatchCircleView(Context* context, const AttributeSet* attrs)
    : View(context, attrs) {
    Resources& resources = context->getResources();
    const float dotDiameter = resources.getDimension(R::dimen::circletimer_dot_size);

    mDotRadius = dotDiameter / 2.0f;
    mScreenDensity = context->getDisplayMetrics().density;
    const float strokeSize = resources.getDimension(R::dimen::circletimer_circle_size);
    const float markerStrokeSize = resources.getDimension(R::dimen::circletimer_marker_size);
    mRadiusOffset = Utils::calculateRadiusOffset(strokeSize, dotDiameter, markerStrokeSize);
    mStrokeSize = strokeSize;
    mMarkerStrokeSize = markerStrokeSize;

    mRemainderColor = 0xFFFFFFFF; // Color.WHITE
    // ThemeUtils.resolveColor(context, R.attr.colorAccent) == android:colorAccent (0x01010435).
    mCompletedColor = 0xFFDA4336;
    {
        TypedValue value;
        if (context->getTheme().resolveAttribute(0x01010435, &value, true)) {
            mCompletedColor = value.data;
        }
    }
}

void StopwatchCircleView::update() {
    postInvalidateOnAnimation();
}

void StopwatchCircleView::onDraw(Canvas& canvas) {
    // Compute the size and location of the circle to be drawn.
    const float xCenter = getWidth() / 2.0f;
    const float yCenter = getHeight() / 2.0f;
    const float radius = std::min(xCenter, yCenter) - mRadiusOffset;

    const std::vector<data::Lap>& laps = DataModel::getDataModel().getLaps();

    // If a reference lap does not exist or should not be drawn, draw a simple white circle.
    if (laps.empty() || !DataModel::getDataModel().canAddMoreLaps()) {
        // Draw a complete white circle; no red arc required.
        canvas.set_color(mRemainderColor);
        canvas.set_line_width(mStrokeSize);
        canvas.arc(xCenter, yCenter, radius, 0.0, 2.0 * M_PI);
        canvas.stroke();

        // No need to continue animating the plain white circle.
        return;
    }

    // The first lap is the reference lap to which all future laps are compared.
    const data::Stopwatch& stopwatch = DataModel::getDataModel().getStopwatch();
    const int lapCount = (int) laps.size();
    const data::Lap& firstLap = laps[lapCount - 1];
    const data::Lap& priorLap = laps[0];
    const float firstLapTime = (float) firstLap.lapTime;
    const float currentLapTime = (float) (stopwatch.getTotalTime() - priorLap.accumulatedTime);

    // Draw a combination of red and white arcs to create a circle.
    const float redPercent = currentLapTime / firstLapTime;
    const float whitePercent = 1.0f - (redPercent > 1.0f ? 1.0f : redPercent);

    const double toRad = M_PI / 180.0;
    // Draw a white arc to indicate the amount of reference lap that remains.
    canvas.set_color(mRemainderColor);
    canvas.set_line_width(mStrokeSize);
    {
        const double start = (270.0 + (1.0 - whitePercent) * 360.0) * toRad;
        const double end = start + whitePercent * 360.0 * toRad;
        canvas.arc(xCenter, yCenter, radius, start, end);
        canvas.stroke();
    }

    // Draw a red arc to indicate the amount of reference lap completed.
    canvas.set_color(mCompletedColor);
    {
        const double start = 270.0 * toRad;
        const double end = start + redPercent * 360.0 * toRad;
        canvas.arc(xCenter, yCenter, radius, start, end);
        canvas.stroke();
    }

    // Starting on lap 2, a marker can be drawn indicating where the prior lap ended.
    if (lapCount > 1) {
        canvas.set_color(mRemainderColor);
        canvas.set_line_width(mMarkerStrokeSize);
        const float markerAngle = (float) priorLap.lapTime / firstLapTime * 360.0f;
        const double startAngle = 270.0 + markerAngle;
        const double sweepAngle = mScreenDensity * (360.0f / (radius * (float) M_PI));
        canvas.arc(xCenter, yCenter, radius, startAngle * toRad, (startAngle + sweepAngle) * toRad);
        canvas.stroke();
    }

    // Draw a red dot to indicate current position relative to reference lap.
    const double dotAngleDegrees = 270.0 + redPercent * 360.0;
    const double dotAngleRadians = dotAngleDegrees * toRad;
    const double dotX = xCenter + radius * cos(dotAngleRadians);
    const double dotY = yCenter + radius * sin(dotAngleRadians);
    canvas.set_color(mCompletedColor);
    canvas.arc(dotX, dotY, mDotRadius, 0.0, 2.0 * M_PI);
    canvas.fill();

    // If the stopwatch is not running it does not require continuous updates.
    if (stopwatch.isRunning()) {
        postInvalidateOnAnimation();
    }
}

} // namespace stopwatch

typedef cdroid::deskclock::stopwatch::StopwatchCircleView StopwatchCircleView;
DECLARE_WIDGET2(StopwatchCircleView, "StopwatchCircleView");

} // namespace deskclock
} // namespace cdroid
