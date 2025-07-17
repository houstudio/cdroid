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
#include <view/menu.h>
#include <view/submenu.h>
#include <view/menuitem.h>
using namespace Cairo;
namespace cdroid{

Menu::Menu(){
   owner = nullptr;
}

Menu::~Menu(){
   clear();
}

void Menu::popup(int x,int y){
}

void Menu::close(){
}

MenuItem* Menu::add(const std::string&title){
   return add(-1,-1,-1,title);
}

MenuItem* Menu::add(int groupId, int itemId, int order,const std::string&title){
   MenuItem*itm = new MenuItem(); 
   itm->setTitle(title);
   itm->mGroupId=groupId;
   itm->mItemId=itemId;
   itm->mOrder=order;
   mMenuItems.push_back(itm);
   return itm;
}

SubMenu* Menu::addSubMenu(const std::string&title){
   return addSubMenu(-1,-1,-1,title);
}

SubMenu* Menu::addSubMenu(int groupId,int itemId,int order,const std::string& title){
   MenuItem* itm = add(groupId,itemId,order,title);
   itm->mSubMenu = new SubMenu();
   return itm->mSubMenu;
}

void Menu::removeItem(int id){
      
}

void Menu::removeGroup(int groupId){
   for(auto t=mMenuItems.begin();t!=mMenuItems.end();){
       if((*t)->mGroupId==groupId){
           delete (*t);
           t=mMenuItems.erase(t);
       }else t++;
   }
}

void Menu::clear(){
   while(mMenuItems.size()){
       auto t=mMenuItems.begin();
       delete (*t);
       mMenuItems.erase(t);
   }
}

void Menu::setGroupCheckable(int group, bool checkable, bool exclusive){
   for(auto t=mMenuItems.begin();t!=mMenuItems.end();){
       if((*t)->mGroupId==group)
          (*t)->mCheckable=checkable;
   }
}

void Menu::setGroupEnabled(int group, bool enabled){
    for(auto m:mMenuItems){
        if(m->mGroupId==group)
           m->mEnabled=enabled;
    }
}

bool Menu::hasVisibleItems()const{
    for(auto m:mMenuItems)
        if(m->mVisible)return true;
    return true;
}

MenuItem*Menu::findItem(int id)const{
    return nullptr;
}

int Menu::size()const{
    return mMenuItems.size();
}

MenuItem* Menu::getItem(int index)const{
    return mMenuItems.at(index);
}

}//end namespace


