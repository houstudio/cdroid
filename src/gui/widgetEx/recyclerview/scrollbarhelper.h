#ifndef __SCROLLBAR_HELPER_H__
#define __SCROLLBAR_HELPER_H__
#include <widgetEx/recyclerview/recyclerview.h>
namespace cdroid{
class OrientationHelper;
class ScrollbarHelper {
private:
     ScrollbarHelper();
public:
    static int computeScrollOffset(RecyclerView::State& state, OrientationHelper& orientation,
            View* startChild, View* endChild, RecyclerView::LayoutManager& lm,
            bool smoothScrollbarEnabled, bool reverseLayout);
    static int computeScrollExtent(RecyclerView::State& state, OrientationHelper& orientation,
            View* startChild, View* endChild, RecyclerView::LayoutManager& lm, bool smoothScrollbarEnabled);
    static int computeScrollRange(RecyclerView::State& state, OrientationHelper& orientation,
            View* startChild, View* endChild, RecyclerView::LayoutManager& lm, bool smoothScrollbarEnabled);
};
}/*endof namespace*/
#endif/*__SCROLLBAR_HELPER_H__*/
