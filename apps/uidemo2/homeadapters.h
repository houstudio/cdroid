#ifndef __HOME_ADAPTERS_H__
#define __HOME_ADAPTERS_H__
#include <vector>
#include <string>
#include <widget/adapter.h>
namespace cdroid{

class FunctionItem{
public:
    std::string text;
    std::string icon;
    std::string layout;
    FunctionItem(const std::string&text,const std::string&icon,const std::string&layout){
        this->text=text;
        this->icon=icon;
        this->layout=layout;
    }
};

class Device{
public:
    std::string mName;
    std::string mIccon;
    int mSubType;
    bool mPowerOn;
    bool mInline;
    Device(const std::string&name,bool power=true,bool _Inline=false){
        mName=name;
        mPowerOn=power;
        mInline=_Inline;
    }
};

class FunctionAdapter:public ArrayAdapter<FunctionItem>{
public:
    FunctionAdapter(){}
    View*getView(int position, View* convertView, ViewGroup* parent)override;
    int load(const std::string&path);
};

template<typename T>
class DevicePagerAdapter:public PagerAdapter{
private:
    float mPageWidth;
protected:
    std::vector<T>mItems;
    std::string mResourceId;
    int mDeviceId;
public:
    DevicePagerAdapter(const std::string&resid,int id=-1){
        mResourceId=resid;
        mDeviceId = id;
        mPageWidth = 1.f;
    }
    int getCount()override{ return mItems.size(); }
    virtual void setDeviceProperties(View*v,T&){};
    bool isViewFromObject(View* view, void*object)override{
         return view==object;
    }
    void* instantiateItem(ViewGroup* container, int position)override{
        T& t=mItems.at(position);
        View*view=LayoutInflater::from(container->getContext())->inflate(mResourceId,nullptr,false);
        container->addView(view);
        setDeviceProperties(view,t);
        return view;
    }
    void destroyItem(ViewGroup* container, int position,void* object)override{
        container->removeView((View*)object);
        delete (View*)object;
    }
    void add(const T&t){
        mItems.push_back(t); 
    }
    void setPageWidth(float w){
        mPageWidth = w;
    }
    float getPageWidth(int position)override{
        return mPageWidth;
    }
};

class LightPagerAdapter:public DevicePagerAdapter<Device>{
public:
    LightPagerAdapter(const std::string&resid,int id=-1);
    void* instantiateItem(ViewGroup* container, int position)override;
};

class CurtainPagerAdapter:public LightPagerAdapter{
public:
    CurtainPagerAdapter(const std::string&resid,int id=-1);
    void* instantiateItem(ViewGroup* container, int position)override;
};

typedef DevicePagerAdapter<Device> TVPagerAdapter;
typedef DevicePagerAdapter<Device> LockPagerAdapter;
typedef DevicePagerAdapter<Device> SocketPagerAdapter;
typedef DevicePagerAdapter<Device> HangerPagerAdapter;

class AircondPagerAdapter:public DevicePagerAdapter<Device>{
public:
    AircondPagerAdapter(const std::string&resid,int id);
    void setDeviceProperties(View*v,Device&t)override;
};
}//endofnamesapce
#endif
