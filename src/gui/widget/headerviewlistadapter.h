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
#ifndef __HEADERVIEW_LISTADAPTER_H__
#define __HEADERVIEW_LISTADAPTER_H__
#include <widget/adapter.h>
namespace cdroid{
class FixedViewInfo;

class HeaderViewListAdapter:public Adapter{
private:
    bool areAllListInfosSelectable(const std::vector<FixedViewInfo*>& infos)const;
protected:
    Adapter*mAdapter;
    std::vector<FixedViewInfo*>mHeaderViewInfos;
    std::vector<FixedViewInfo*>mFooterViewInfos;
    bool mAreAllFixedViewsSelectable;
    bool mIsFilterable;
public:
    HeaderViewListAdapter(const std::vector<FixedViewInfo*>& headerViewInfos,
                const std::vector<FixedViewInfo*>& footerViewInfos,Adapter* adapter);
    ~HeaderViewListAdapter();
    int getHeadersCount()const;
    int getFootersCount()const;
    bool isEmpty()const override;
    bool removeHeader(View* v);
    bool removeFooter(View* v);
    int getCount()const override;
    bool areAllItemsEnabled()const override;
    bool isEnabled(int position)const override;
    void* getItem(int position)const override;
    long getItemId(int position)const override;
    bool hasStableIds()const override;
    View* getView(int position, View* convertView, ViewGroup* parent)override;
    int getItemViewType(int position)const override;
    int getViewTypeCount()const override;
    void registerDataSetObserver(DataSetObserver* observer)override;
    void unregisterDataSetObserver(DataSetObserver* observer)override;
    Adapter* getWrappedAdapter();
};

}/*endof namespace*/
#endif
