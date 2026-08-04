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
#include <fragment/listfragment.h>
#include <widget/adapter.h>
namespace cdroid{
namespace fragment{

ListFragment::ListFragment(){}

cdroid::View* ListFragment::onCreateView(cdroid::LayoutInflater* /*inflater*/,
                                         cdroid::ViewGroup* /*container*/,
                                         cdroid::Bundle* /*savedInstanceState*/){
    mList = new cdroid::ListView(0, 0);
    return mList;
}

void ListFragment::onViewCreated(cdroid::View* view, cdroid::Bundle* savedInstanceState){
    Fragment::onViewCreated(view, savedInstanceState);
    if(mAdapter && mList) mList->setAdapter(mAdapter);
}

void ListFragment::setListAdapter(cdroid::Adapter* adapter){
    mAdapter = adapter;
    if(mList) mList->setAdapter(adapter);
}

}//namespace fragment
}//namespace cdroid
