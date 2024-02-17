#ifndef __ITEMTOUCHUI_UTIL_H__
#define __ITEMTOUCHUI_UTIL_H__
#include <widgetEx/recyclerview/recyclerview.h>
namespace cdroid{
class ItemTouchUIUtil {
public:
    virtual void onDraw(Canvas& c, RecyclerView& recyclerView, View& view,
            float dX, float dY, int actionState, bool isCurrentlyActive)=0;

    virtual void onDrawOver(Canvas& c, RecyclerView& recyclerView, View& view,
            float dX, float dY, int actionState, bool isCurrentlyActive)=0;
    virtual void clearView(View& view)=0;
    virtual void onSelected(View& view)=0;
};

class ItemTouchUIUtilImpl:public ItemTouchUIUtil {
private:
    static float findMaxElevation(RecyclerView& recyclerView, View& itemView);
    friend class ItemTouchHelper;
public:
    void onDraw(Canvas& c, RecyclerView& recyclerView, View& view, float dX, float dY,
            int actionState, bool isCurrentlyActive)override;
    void onDrawOver(Canvas& c, RecyclerView& recyclerView, View& view, float dX, float dY,
            int actionState, bool isCurrentlyActive)override;
    void clearView(View& view);
    void onSelected(View& view);
};
}
#endif/*__ITEMTOUCHUI_UTIL_H__*/
