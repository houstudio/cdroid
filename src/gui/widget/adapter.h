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
#ifndef __ADAPTER_H__
#define __ADAPTER_H__
#include <view/viewgroup.h>
#include <widget/textview.h>

namespace cdroid{
class DataSetObserver{
public:
    virtual ~DataSetObserver()=default;
    virtual void onChanged()=0;
    virtual void onInvalidated()=0;
    virtual void clearSavedState()=0;
};

struct SectionIndexer{
    std::function<std::vector<void*>()>getSections;
    std::function<int(int)>getPositionForSection;
    std::function<int(int)>getSectionForPosition;
};

class Adapter{
protected:
    bool mNotifyOnChange;
public:
    Adapter(){
        mNotifyOnChange=false;
    }
    virtual ~Adapter(){};
    void setNotifyOnChange(bool notifyOnChange) {
        mNotifyOnChange = notifyOnChange;
    }
    virtual void registerDataSetObserver(DataSetObserver* observer);
    virtual void unregisterDataSetObserver(DataSetObserver* observer);
    virtual void notifyDataSetChanged();
    virtual void notifyDataSetInvalidated();
    virtual int  getCount()const=0;
    virtual void*getItem(int position)const=0;
    virtual long getItemId(int position)const;
    virtual bool hasStableIds()const;
    virtual View*getView(int position, View* convertView, ViewGroup* parent)=0;
    virtual int  getItemViewType(int position)const;
    virtual int  getViewTypeCount()const;
    virtual bool isEmpty()const;

    virtual bool areAllItemsEnabled()const;//for listadapter
    virtual bool isEnabled(int position)const;//for listadapter
    virtual View*getDropDownView(int position, View* convertView, ViewGroup* parent);//only for spinneradapter
private:
    std::vector<DataSetObserver*> mObservers;
};

typedef Adapter ListAdapter,SpinnerAdapter,BaseAdapter;

// AOSP ArrayAdapter item -> CharSequence: strings verbatim, arithmetic via
// std::to_string (AOSP calls toString()).
template <typename U> struct AdapterItemText { static std::string get(const U& v){ return std::to_string(v); } };
template <> struct AdapterItemText<std::string> { static const std::string& get(const std::string& v){ return v; } };

template<class T>
class ArrayAdapter:public Adapter{
private:
    std::vector<T> mObjects;
protected:
    Context* mContext;
    int  mFieldId;
    int mResource;
    int mDropDownResource;
private:
     // AOSP ArrayAdapter.createViewFromResource (verbatim structure):
     // inflate with the real parent (attachToRoot=false), whole view is the
     // TextView when mFieldId==0, and ALWAYS (re)bind the text — recycled
     // convertView included (the bind was missing entirely before, leaving
     // spinner dropdown items blank).
     View* createViewFromResource(int position,View* convertView,ViewGroup* parent,int resource) {
        View* view = convertView;
        if (view == nullptr && resource != 0) {
            view = LayoutInflater::from(mContext)->inflate(resource, parent, false);
        }
        TextView* text;
        if (mFieldId == 0) {
            text = dynamic_cast<TextView*>(view);
        } else {
            text = (TextView*)view->findViewById(mFieldId);
        }
        if (text != nullptr) {
            text->setText(AdapterItemText<T>::get(getItemAt(position)));
        }
        return view;
    }
public:
    ArrayAdapter():Adapter(){
        mContext = nullptr;
        mFieldId = 0;
    }
    ArrayAdapter(Context*context,int resource,int textViewResourceId/*,onSetTextListener setfun=nullptr*/){
        mContext = context;
        mResource= resource;
        mDropDownResource = resource;
        mFieldId = textViewResourceId;
    }
    void setDropDownViewResource(int resource){
        mDropDownResource = resource;
    }
    void addAll(const std::vector<T>&items){
        mObjects=items;
        if (mNotifyOnChange) notifyDataSetChanged();
    }
    void*getItem(int position)const override{
        if(std::is_class<T>::value)return (void*)&mObjects[position];
        else return  (void*)&mObjects[position];
    }
    T& getItemAt(int position){
        return mObjects[position];
    }
    int getCount()const override {
        return (int)mObjects.size();
    }
    void add(const T& obj){
        mObjects.push_back(obj);
        if(mNotifyOnChange)notifyDataSetChanged();
    }
    void insert(T& obj, int index){
        mObjects.insert(mObjects.begin()+index,obj);
        if(mNotifyOnChange)notifyDataSetChanged();
    }
    void remove(T& obj){
        auto it = std::find(mObjects.begin(),mObjects.end(),obj);
        if(it!=mObjects.end()){
            mObjects.erase(it);
            if(mNotifyOnChange)notifyDataSetChanged();
        }
    }
    void removeAt(int idx){
        mObjects.erase(mObjects.begin()+idx);
        if(mNotifyOnChange)notifyDataSetChanged();
    }
    void clear(){
        mObjects.clear();
        if(mNotifyOnChange)notifyDataSetChanged();
    }
    View*getView(int position, View* convertView, ViewGroup* parent)override{
        return createViewFromResource(position, convertView, parent, mResource);
    }
    View*getDropDownView(int position, View* convertView, ViewGroup* parent)override{
        return createViewFromResource(position, convertView, parent, mDropDownResource);
    }
};

class PagerAdapter{
protected:
    std::vector<DataSetObserver*> mObservers;
    DataSetObserver* mViewPagerObserver;
public:
    static constexpr int POSITION_UNCHANGED =-1;
    static constexpr int POSITION_NONE = -2;
public:
    PagerAdapter();
    virtual ~PagerAdapter();
    virtual int getCount();
    virtual void startUpdate(ViewGroup* container);
    virtual void*instantiateItem(ViewGroup* container, int position);
    virtual void destroyItem(ViewGroup* container, int position, void* object);
    virtual void setPrimaryItem(ViewGroup* container, int position, void* object);
    virtual void finishUpdate(ViewGroup* container);
    virtual bool isViewFromObject(View* view, void* object);
    virtual int getItemPosition(void* object);
    virtual void notifyDataSetChanged();
    virtual Parcelable* saveState();
    virtual void restoreState(Parcelable* state);
    void registerDataSetObserver(DataSetObserver* observer);
    void unregisterDataSetObserver(DataSetObserver* observer);
    void setViewPagerObserver(DataSetObserver* observer);
    virtual std::string getPageTitle(int position){return std::string();}
    /*Returns the proportional width of a given page as a percentage of the
     * ViewPager's measured width from (0.f-1.f]*/
    virtual float getPageWidth(int position){return 1.f;};
};

}//namespace
#endif
