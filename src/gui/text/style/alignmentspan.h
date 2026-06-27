#ifndef __ALIGNMENT_SPAN_H__
#define __ALIGNMENT_SPAN_H__
#include <text/layout.h>
#include <text/parcelablespan.h>
namespace cdroid {

class AlignmentSpan : public ParagraphStyle {
public:
    explicit AlignmentSpan(Layout::Alignment alignment = Layout::ALIGN_NORMAL)
            : mAlignment(alignment) {}
    int getAlignment() const { return mAlignment; }

    class Standard;

private:
    int mAlignment;
};

class AlignmentSpan::Standard : public AlignmentSpan {
public:
    explicit Standard(Layout::Alignment alignment) : AlignmentSpan(alignment) {}
};

} /* end namespace */
#endif /* __ALIGNMENT_SPAN_H__ */
