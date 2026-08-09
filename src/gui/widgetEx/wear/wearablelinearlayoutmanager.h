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
#ifndef __WEARABLE_LINEAR_LAYOUTMANAGER_H__
#define __WEARABLE_LINEAR_LAYOUTMANAGER_H__
#include <widgetEx/recyclerview/linearlayoutmanager.h>
namespace cdroid{
class CurvingLayoutCallback;
class WearableLinearLayoutManager:public LinearLayoutManager {
public:
    DECLARE_UIEVENT(void,LayoutCallback,View&,RecyclerView&);
private:
    CurvingLayoutCallback*mCurvingLayoutCallback;
    LayoutCallback mLayoutCallback;
    void updateLayout();
public:
    WearableLinearLayoutManager(Context* context);
    WearableLinearLayoutManager(Context* context,const LayoutCallback& layoutCallback);
    ~WearableLinearLayoutManager()override;

    void setLayoutCallback(const LayoutCallback& layoutCallback);
    LayoutCallback getLayoutCallback() const;

    int scrollVerticallyBy(int dy, RecyclerView::Recycler& recycler, RecyclerView::State& state) override;
    void onLayoutChildren(RecyclerView::Recycler& recycler, RecyclerView::State& state) override;
};
}/*endof namespace*/
#endif/*__WEARABLE_LINEAR_LAYOUTMANAGER_H__*/
