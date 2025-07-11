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
#include <widget/adapter.h>
#include <cdlog.h>

namespace cdroid{

void Adapter::registerDataSetObserver(DataSetObserver* observer) {
    if(observer!=nullptr)
        mObservers.push_back(observer);
}

void Adapter::unregisterDataSetObserver(DataSetObserver* observer) {
    auto it=std::find(mObservers.begin(),mObservers.end(),observer);
    if(it!=mObservers.end())mObservers.erase(it);
}

void Adapter::notifyDataSetChanged(){
    int sz=mObservers.size()-1;
    for(int i=sz;i>=0;i--){
        DataSetObserver*obs=mObservers[i];
        obs->onChanged();
    }
}

void Adapter::notifyDataSetInvalidated(){
    int sz=mObservers.size()-1;
    for(int i=sz;i>=0;i--) mObservers[i]->onInvalidated();
}

long Adapter::getItemId(int position)const{
    return position;
}

bool Adapter::hasStableIds()const{
    return true;
}

View*Adapter::getDropDownView(int position, View* convertView, ViewGroup* parent){
    return nullptr;
}

bool Adapter::areAllItemsEnabled()const{
    return true;
}

bool Adapter::isEnabled(int position)const{
    return true;
}

int Adapter::getItemViewType(int position)const{
    return 0;
}

int Adapter::getViewTypeCount()const{
    return 1;
}

bool Adapter::isEmpty()const{
    return getCount() == 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////
PagerAdapter::PagerAdapter(){
    mViewPagerObserver = nullptr;
}

PagerAdapter::~PagerAdapter(){
}

int PagerAdapter::getCount(){
    return 0;
}

void PagerAdapter::startUpdate(ViewGroup* container){
}

void*PagerAdapter::instantiateItem(ViewGroup* container, int position){
    throw std::runtime_error("Required method instantiateItem was not overridden");
    return nullptr;
}


void PagerAdapter::destroyItem(ViewGroup* container, int position, void* object){
    throw std::runtime_error("Required method destroyItem was not overridden");
}

void PagerAdapter::finishUpdate(ViewGroup* container){
}

bool PagerAdapter::isViewFromObject(View*view,void*obj){
    return view==obj;
}

int PagerAdapter::getItemPosition(void* object){
    return POSITION_UNCHANGED;
}

void PagerAdapter::setPrimaryItem(ViewGroup* container, int position,void*object) {
}

void PagerAdapter::notifyDataSetChanged(){
    const int sz=mObservers.size()-1;
    if(mViewPagerObserver)
        mViewPagerObserver->onChanged();
    for(int i=sz;i>=0;i--){
        DataSetObserver*obs=mObservers[i];
        obs->onChanged();
    }
}

void PagerAdapter::registerDataSetObserver(DataSetObserver* observer){
    mObservers.push_back(observer);
};

void PagerAdapter::unregisterDataSetObserver(DataSetObserver* observer){
    auto it=std::find(mObservers.begin(),mObservers.end(),observer);
    if(it!=mObservers.end())mObservers.erase(it);
}

void PagerAdapter::setViewPagerObserver(DataSetObserver* observer){
    mViewPagerObserver = observer;
}
}//namespace
