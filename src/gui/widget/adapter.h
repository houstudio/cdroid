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
template<class T>
class ArrayAdapter:public Adapter{
private:
    std::vector<T> mObjects;
protected:
    Context* mContext;
    std::string mResource;
    std::string mDropDownResource;
    int  mFieldId;
private:
     View* createViewFromResource(int position,View* convertView,ViewGroup* parent,const std::string& resource) {
        View*view = convertView?convertView:LayoutInflater::from(mContext)->inflate(resource,nullptr, false);
        //If no custom field is assigned, assume the whole resource is a TextView
        //Otherwise, find the TextView field within the layout
        TextView* text = (mFieldId==0)?(TextView*)view:(TextView*)view->findViewById(mFieldId);
        T& item = getItemAt(position);
        return view;
    }
public:
    ArrayAdapter():Adapter(){
        mContext = nullptr;
        mFieldId = 0;
    }
    ArrayAdapter(Context*context,const std::string&resource,int textViewResourceId/*,onSetTextListener setfun=nullptr*/){
        mContext = context;
        mResource= resource;
        mDropDownResource = resource;
        mFieldId = textViewResourceId;
    }
    void setDropDownViewResource(const std::string&resource){
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
        return mContext==nullptr?nullptr:createViewFromResource(position, convertView, parent, mResource);
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
