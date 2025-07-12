/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *********************************************************************************/
#include <widgetEx/recyclerview/itemtouchuiutil.h>
#include <widget/R.h>
namespace cdroid{
void ItemTouchUIUtilImpl::onDraw(Canvas& c, RecyclerView& recyclerView, View& view, float dX, float dY,
        int actionState, bool isCurrentlyActive){
    if (isCurrentlyActive) {
        void* tag = view.getTag(R::id::item_touch_helper_previous_elevation);
        if (tag == nullptr) {
            long originalElevation = long(view.getElevation());
            int newElevation = 1.f + findMaxElevation(recyclerView, view);
            view.setElevation(newElevation);
            view.setTag(R::id::item_touch_helper_previous_elevation, (void*)originalElevation);
        }
    }
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
    const long* tag = (const long*)view.getTag(R::id::item_touch_helper_previous_elevation);
    if (tag != nullptr/* && tag instanceof Float*/) {
        view.setElevation(float((long)tag));
    }
    view.setTag(R::id::item_touch_helper_previous_elevation, nullptr);

    view.setTranslationX(0.f);
    view.setTranslationY(0.f);
}

void ItemTouchUIUtilImpl::onSelected(View& view){
}
}/*endof namespace*/

