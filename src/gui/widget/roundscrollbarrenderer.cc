#include <widget/roundscrollbarrenderer.h>
#include <color.h>
#include <cdlog.h>


namespace cdroid{
    const int SCROLLBAR_ANGLE_RANGE = 90;
    const int MAX_SCROLLBAR_ANGLE_SWIPE = 16;
    const int MIN_SCROLLBAR_ANGLE_SWIPE = 6;
    const float WIDTH_PERCENTAGE = 0.02f;
    const int DEFAULT_THUMB_COLOR = 0x4CFFFFFF;
    const int DEFAULT_TRACK_COLOR = 0x26FFFFFF;

    RoundScrollbarRenderer::RoundScrollbarRenderer(View*parent) {
        // Paints for the round scrollbar.
        // Set up the thumb paint

        mThumbColor = DEFAULT_THUMB_COLOR;
        mTrackColor = DEFAULT_TRACK_COLOR;
        mParent = parent;
    }

    void RoundScrollbarRenderer::drawRoundScrollbars(Canvas&canvas, float alpha,const Rect&bounds) {
        if (alpha == 0)return;
        // Get information about the current scroll state of the parent view.
        float maxScroll = mParent->computeVerticalScrollRange();
        float scrollExtent = mParent->computeVerticalScrollExtent();
        if (scrollExtent <= 0 || maxScroll <= scrollExtent) {
            return;
        }
        float currentScroll = std::max(0, mParent->computeVerticalScrollOffset());
        float linearThumbLength = mParent->computeVerticalScrollExtent();
        float thumbWidth = mParent->getWidth() * WIDTH_PERCENTAGE;

        setThumbColor(applyAlpha(DEFAULT_THUMB_COLOR, alpha));
        setTrackColor(applyAlpha(DEFAULT_TRACK_COLOR, alpha));

        // Normalize the sweep angle for the scroll bar.
        float sweepAngle = (linearThumbLength / maxScroll) * SCROLLBAR_ANGLE_RANGE;
        sweepAngle = clamp(sweepAngle, MIN_SCROLLBAR_ANGLE_SWIPE, MAX_SCROLLBAR_ANGLE_SWIPE);
        // Normalize the start angle so that it falls on the track.
        float startAngle = (currentScroll * (SCROLLBAR_ANGLE_RANGE - sweepAngle))
                / (maxScroll - linearThumbLength) - SCROLLBAR_ANGLE_RANGE / 2;
        startAngle = clamp(startAngle, -SCROLLBAR_ANGLE_RANGE / 2, SCROLLBAR_ANGLE_RANGE / 2 - sweepAngle);

        // Draw the track and the scroll bar.
        mRect = bounds;
        mRect.inflate(-thumbWidth /2,0);

        LOGV("alpha=%.2f thumbWidth=%f angle=%f/%f bounds=(%d,%d,%d,%d)",alpha,thumbWidth,
                    startAngle,sweepAngle,mRect.left,mRect.top,mRect.width,mRect.height);
        canvas.set_line_width(thumbWidth);
        canvas.set_color(mThumbColor);
        canvas.set_antialias(Cairo::ANTIALIAS_SUBPIXEL);
        canvas.set_line_cap(Cairo::Context::LineCap::ROUND);
        canvas.arc(mRect.left+mRect.width/2,mRect.top+mRect.height/2,mRect.width/2.,
                    0,M_PI*SCROLLBAR_ANGLE_RANGE/180.);
        canvas.stroke();

        canvas.set_color(mTrackColor);
        canvas.arc(mRect.left+mRect.width/2,mRect.top+mRect.height/2,mRect.width/2.,
            startAngle*M_PI/180.,(startAngle+sweepAngle)*M_PI/180.);
        canvas.stroke();
    }
 
    float RoundScrollbarRenderer::clamp(float val, float min, float max) {
        if (val < min) {
            return min;
        } else if (val > max) {
            return max;
        } else {
            return val;
        }
    }

    int RoundScrollbarRenderer::applyAlpha(int color, float alpha) {
        int alphaByte = (int) (Color::alpha(color) * alpha);
        return Color::toArgb(alphaByte, Color::red(color), Color::green(color), Color::blue(color));
    }

    void RoundScrollbarRenderer::setThumbColor(int thumbColor) {
        mThumbColor=thumbColor;
    }

    void RoundScrollbarRenderer::setTrackColor(int trackColor) {
        mTrackColor=trackColor;
    }
}
