#include <view/roundscrollbarrenderer.h>
#include <color.h>
#include <cdlog.h>


namespace cdroid{
    const int SCROLLBAR_ANGLE_RANGE = 90;
    const int MAX_SCROLLBAR_ANGLE_SWIPE = 16;
    const int MIN_SCROLLBAR_ANGLE_SWIPE = 6;
    const float WIDTH_PERCENTAGE = 0.02f;
    const int DEFAULT_THUMB_COLOR = 0xFFE8EAED;
    const int DEFAULT_TRACK_COLOR = 0x4CFFFFFF;

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
        const float maxScroll = mParent->computeVerticalScrollRange();
        const float scrollExtent = mParent->computeVerticalScrollExtent();
        if (scrollExtent <= 0 || maxScroll <= scrollExtent) {
            return;
        }
        const float currentScroll = std::max(0, mParent->computeVerticalScrollOffset());
        const float linearThumbLength = mParent->computeVerticalScrollExtent();
        const float thumbWidth = mParent->getWidth() * WIDTH_PERCENTAGE;

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
        mRect.inset(thumbWidth/2,0);//inflate(-thumbWidth /2,0);

        const double radius = double(mRect.width)/2.0;
        canvas.save();
        canvas.set_line_width(thumbWidth);
        canvas.set_color(mTrackColor);
        canvas.set_antialias(Cairo::ANTIALIAS_DEFAULT);
        canvas.set_line_cap(Cairo::Context::LineCap::ROUND);
        canvas.translate(mRect.centerX(),mRect.centerY());

        canvas.scale(1.f,double(mRect.height)/mRect.width);
        canvas.begin_new_sub_path();
        canvas.arc(0,0,radius, -M_PI*SCROLLBAR_ANGLE_RANGE/360.f,M_PI*SCROLLBAR_ANGLE_RANGE/360.f);
        canvas.stroke();

        canvas.set_color(mThumbColor);
        canvas.begin_new_sub_path();
        canvas.arc(0,0,radius, startAngle*M_PI/180.f,(startAngle+sweepAngle)*M_PI/180.f);
        canvas.stroke();
        canvas.restore();
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
        const int alphaByte = (int) (Color::alpha(color) * alpha);
        return Color::toArgb(Color::red(color), Color::green(color), Color::blue(color),alphaByte);
    }

    void RoundScrollbarRenderer::setThumbColor(int thumbColor) {
        mThumbColor = thumbColor;
    }

    void RoundScrollbarRenderer::setTrackColor(int trackColor) {
        mTrackColor = trackColor;
    }
}
