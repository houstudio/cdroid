#ifndef __LINE_BACKGROUND_SPAN_H__
#define __LINE_BACKGROUND_SPAN_H__
#include <text/parcelablespan.h>
namespace cdroid {

class Canvas;
class Paint;

class LineBackgroundSpan : public ParagraphStyle {
public:
    virtual int getLineBackground() const { return 0; }
    virtual void drawBackground(Canvas& canvas,const Paint& paint, int left, int right, int top, int baseline,
            int bottom, CharSequence* text, int start, int end, int lineNumber) const {
        const int color = getLineBackground();
        if (color == 0) return;
        canvas.save();
        canvas.set_color(color);
        canvas.rectangle(left, top, right - left, bottom - top);
        canvas.fill();
        canvas.restore();
    }
};

} /* end namespace */
#endif /* __LINE_BACKGROUND_SPAN_H__ */
