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
#ifndef __LISTFRAGMENT_H__
#define __LISTFRAGMENT_H__
/*********************************************************************************
 * Port of androidx.fragment.app.ListFragment. A Fragment that hosts a ListView;
 * subclasses override onListItemClick and call setListAdapter.
 *********************************************************************************/
#include <fragment/fragment.h>
#include <widget/listview.h>
namespace cdroid{
namespace fragment{

class ListFragment : public Fragment{
public:
    ListFragment();
    cdroid::View* onCreateView(cdroid::LayoutInflater* inflater, cdroid::ViewGroup* container,
                               cdroid::Bundle* savedInstanceState) override;
    void onViewCreated(cdroid::View* view, cdroid::Bundle* savedInstanceState) override;

    void setListAdapter(cdroid::Adapter* adapter);
    cdroid::ListView* getListView() const { return mList; }
protected:
    // Override to handle item clicks.
    virtual void onListItemClick(cdroid::ListView* l, cdroid::View* v, int position, long id){}
private:
    cdroid::ListView* mList = nullptr;
    cdroid::Adapter* mAdapter = nullptr;
};

}//namespace fragment
}//namespace cdroid
#endif
