#ifndef __LINE_HEIGHT_SPAN_H__
#define __LINE_HEIGHT_SPAN_H__
#include <text/parcelablespan.h>
#include <text/paint.h>
#include <text/textpaint.h>
namespace cdroid {

class LineHeightSpan : public ParagraphStyle {
public:
    class WithDensity;
    virtual void chooseHeight(CharSequence* text, int start, int end,
            int spanstartv, int lineHeight, Paint::FontMetricsInt& fm) const {}
};

class LineHeightSpan::WithDensity : public LineHeightSpan {
public:
    virtual void chooseHeight(CharSequence* text, int start, int end,
            int spanstartv, int lineHeight,
            Paint::FontMetricsInt& fm, TextPaint* paint) const {}
};

} /* end namespace */
#endif /* __LINE_HEIGHT_SPAN_H__ */
