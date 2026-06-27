#ifndef __LINE_BACKGROUND_SPAN_H__
#define __LINE_BACKGROUND_SPAN_H__
#include <text/parcelablespan.h>
namespace cdroid {

class Canvas;
class Paint;

class LineBackgroundSpan : public ParagraphStyle {
public:
    virtual int getLineBackground() const { return 0; }
    virtual void drawBackground(Canvas&, Paint& paint, int left, int right, int top, int baseline,
            int bottom, CharSequence* text, int start, int end, int lineNumber) const {}
};

} /* end namespace */
#endif /* __LINE_BACKGROUND_SPAN_H__ */
