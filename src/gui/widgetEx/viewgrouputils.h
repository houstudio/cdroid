#ifndef __VIEWGROUP_UTILS_H__
#define __VIEWGROUP_UTILS_H__
#include <view/viewgroup.h>
namespace cdroid{
class ViewGroupUtils {
private:
    ViewGroupUtils();
    static void offsetDescendantMatrix(ViewGroup* target, View* view, Cairo::Matrix& m);
protected:
    static void offsetDescendantRect(ViewGroup* parent, View* descendant, Rect& rect);
public:
    static void getDescendantRect(ViewGroup* parent, View* descendant, Rect& out);
};
}/*endof namespace*/
#endif
