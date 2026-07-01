#ifndef __WRAP_TOGETHER_SPAN_H__
#define __WRAP_TOGETHER_SPAN_H__
#include <text/parcelablespan.h>
namespace cdroid {

class WrapTogetherSpan : public ParagraphStyle {
public:
    WrapTogetherSpan* clone() const override { return new WrapTogetherSpan(*this); }
};

} /* end namespace */
#endif /* __WRAP_TOGETHER_SPAN_H__ */
