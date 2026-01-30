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
#ifndef __MENU_ADAPTER_H__
#define __MENU_ADAPTER_H__
#include <widget/adapter.h>
namespace cdroid{
class MenuBuilder;
class MenuPopup;
class MenuAdapter:public BaseAdapter {
private:
    int mExpandedIndex = -1;
    bool mForceShowIcon;
    bool mOverflowOnly;
    LayoutInflater* mInflater;
    std::string mItemLayoutRes;
protected:
    friend MenuPopup;
    MenuBuilder* mAdapterMenu;
public:
    MenuAdapter(MenuBuilder* menu, LayoutInflater* inflater, bool overflowOnly,const std::string& itemLayoutRes);
    bool getForceShowIcon()const;
    void setForceShowIcon(bool forceShow);
    int getCount()const override;

    MenuBuilder* getAdapterMenu()const;
    void* getItem(int position)const override;

    long getItemId(int position);
    View* getView(int position, View* convertView, ViewGroup* parent)override;
    void findExpandedIndex();
    void notifyDataSetChanged() override;
};
}/*endof namespace*/
#endif/*__MENU_ADAPTER_H__*/
