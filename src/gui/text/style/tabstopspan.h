#ifndef __TAB_STOP_SPAN_H__
#define __TAB_STOP_SPAN_H__
#include <text/parcelablespan.h>
namespace cdroid {

class TabStopSpan : public ParagraphStyle {
public:
    virtual int getTabStop() const { return 0; }
};

} /* end namespace */
#endif /* __TAB_STOP_SPAN_H__ */
