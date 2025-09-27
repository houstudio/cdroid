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
*/
#include <widgetEx/wear/wearablerecyclerview.h>
#include <widgetEx/wear/curvinglayoutcallback.h>
#include <widgetEx/wear/wearablelinearlayoutmanager.h>

namespace cdroid{

WearableLinearLayoutManager::WearableLinearLayoutManager(Context* context,const LayoutCallback& layoutCallback)
    :LinearLayoutManager(context, VERTICAL, false){
    mLayoutCallback = layoutCallback;
    mCurvingLayoutCallback = nullptr;
}

WearableLinearLayoutManager::WearableLinearLayoutManager(Context* context)
    :WearableLinearLayoutManager(context, nullptr){
    mCurvingLayoutCallback =new CurvingLayoutCallback(context); 
    mLayoutCallback = std::bind(&CurvingLayoutCallback::onLayoutFinished,mCurvingLayoutCallback,
            std::placeholders::_1,std::placeholders::_2);
}

WearableLinearLayoutManager::~WearableLinearLayoutManager(){
    delete mCurvingLayoutCallback;
}

void WearableLinearLayoutManager::setLayoutCallback(const LayoutCallback& layoutCallback) {
    mLayoutCallback = layoutCallback;
}

WearableLinearLayoutManager::LayoutCallback WearableLinearLayoutManager::getLayoutCallback()const {
    return mLayoutCallback;
}

int WearableLinearLayoutManager::scrollVerticallyBy(int dy, RecyclerView::Recycler& recycler, RecyclerView::State& state) {
    const int scrolled = LinearLayoutManager::scrollVerticallyBy(dy, recycler, state);

    updateLayout();
    return scrolled;
}

void WearableLinearLayoutManager::onLayoutChildren(RecyclerView::Recycler& recycler, RecyclerView::State& state) {
    LinearLayoutManager::onLayoutChildren(recycler, state);
    if (getChildCount() == 0) {
        return;
    }

    updateLayout();
}

void WearableLinearLayoutManager::updateLayout() {
    if (mLayoutCallback == nullptr) {
        return;
    }
    const int childCount = getChildCount();
    for (int count = 0; count < childCount; count++) {
        View* child = getChildAt(count);
        RecyclerView*rv = (RecyclerView*)child->getParent();
        mLayoutCallback/*->onLayoutFinished*/(*child, *rv);
    }
}
}/*endof namespace*/
