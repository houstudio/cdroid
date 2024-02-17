#include <widgetEx/recyclerview/itemtouchuiutil.h>
namespace cdroid{
void ItemTouchUIUtilImpl::onDraw(Canvas& c, RecyclerView& recyclerView, View& view, float dX, float dY,
        int actionState, bool isCurrentlyActive){
    /*if (isCurrentlyActive) {
        Object* originalElevation = view.getTag(R.id.item_touch_helper_previous_elevation);
        if (originalElevation == null) {
            originalElevation = ciew.getElevation();
            float newElevation = 1.f + findMaxElevation(recyclerView, view);
            view.setElevation(newElevation);
            view.setTag(R.id.item_touch_helper_previous_elevation, originalElevation);
        }
    }*/
    view.setTranslationX(dX);
    view.setTranslationY(dY);
}

float ItemTouchUIUtilImpl::findMaxElevation(RecyclerView& recyclerView, View& itemView) {
    const int childCount = recyclerView.getChildCount();
    float max = 0;
    for (int i = 0; i < childCount; i++) {
        View* child = recyclerView.getChildAt(i);
        if (child == &itemView) {
            continue;
        }
        const float elevation = child->getElevation();
        if (elevation > max) {
            max = elevation;
        }
    }
    return max;
}

void ItemTouchUIUtilImpl::onDrawOver(Canvas& c, RecyclerView& recyclerView, View& view, float dX, float dY,
        int actionState, bool isCurrentlyActive){
}

void ItemTouchUIUtilImpl::clearView(View& view){
    /*Object* tag = view.getTag(R.id.item_touch_helper_previous_elevation);
    if (tag != nullptr && tag instanceof Float) {
        view.setElevation((Float) tag);
    }
    view.setTag(R.id.item_touch_helper_previous_elevation, null);
    */

    view.setTranslationX(0.f);
    view.setTranslationY(0.f);
}

void ItemTouchUIUtilImpl::onSelected(View& view){
}
}/*endof namespace*/

