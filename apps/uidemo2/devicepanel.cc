#include <controlpanel.h>
#include <view/layoutinflater.h>
#include <R.h>

namespace cdroid{

DevicePanel::DevicePanel(Context*ctx,const AttributeSet&atts)
   :ControlPanel(ctx,atts){
    mLastDevice =NO_ID;
}

void DevicePanel::init(){
    mPager=(ViewPager*)findViewById(uidemo2::R::id::viewpager);
    mPager->setPageMargin(5);
    mPager->setPageMarginDrawable(new ColorDrawable(0xFF080808));
    ViewGroup*vg=(ViewGroup*)findViewById(uidemo2::R::id::buttonContainer);
    for(int i=0;i<vg->getChildCount();i++){
        View*v=vg->getChildAt(i);
        v->setOnClickListener(std::bind(&DevicePanel::onClick,this,std::placeholders::_1));
    }
}

void DevicePanel::onClick(View&v){
    const int id = v.getId();
    auto it=mDevices.find(id);
    DevicePagerAdapter<Device>*adapter = nullptr;
    if(it!=mDevices.end()){
        adapter=(DevicePagerAdapter<Device>*)it->second;
    }
    LOGD("click %d",id);
    if(id==mLastDevice)
        return;
    switch(id){
    case uidemo2::R::id::lights:
         if(adapter==nullptr){
             adapter = new LightPagerAdapter("@layout/light");
             adapter->setPageWidth(1.f/3);
             mDevices.insert(std::pair<int,PagerAdapter*>(id,adapter));
         }break;
    case uidemo2::R::id::curtains:
         if(adapter==nullptr){
             adapter = new CurtainPagerAdapter("@layout/curtain");
             adapter->setPageWidth(1.f/2);
             mDevices.insert(std::pair<int,PagerAdapter*>(id,adapter));
         }break;
    case uidemo2::R::id::sockets:
         if(adapter==nullptr){
             adapter = new SocketPagerAdapter("@layout/socket");
             adapter->setPageWidth(1.f/3);
             mDevices.insert(std::pair<int,PagerAdapter*>(id,adapter));
         }break;
    case uidemo2::R::id::airconds:
         if(adapter==nullptr){
             adapter = new AircondPagerAdapter("@layout/aircond",uidemo2::R::id::devicename);
             adapter->setPageWidth(1.f/2);
             mDevices.insert(std::pair<int,PagerAdapter*>(id,adapter));
         }break;
    case uidemo2::R::id::tvsets:
         if(adapter==nullptr){
             adapter = new TVPagerAdapter("@layout/tvset",uidemo2::R::id::devicename);
             mDevices.insert(std::pair<int,PagerAdapter*>(id,adapter));
         }break;
    case uidemo2::R::id::ailocks:
         if(adapter==nullptr){
             adapter = new HangerPagerAdapter("@layout/ailock");
             adapter->setPageWidth(1.f/2);
             mDevices.insert(std::pair<int,PagerAdapter*>(id,adapter));
         }break;
    case uidemo2::R::id::hangers:
         if(adapter==nullptr){
             adapter = new HangerPagerAdapter("@layout/todo");
             mDevices.insert(std::pair<int,PagerAdapter*>(id,adapter));
         }break;
    case uidemo2::R::id::wind://新风系统
         if(adapter==nullptr){
             adapter = new AircondPagerAdapter("@layout/newwind",-1);
             adapter->setPageWidth(1.f/2);
             mDevices.insert(std::pair<int,PagerAdapter*>(id,adapter));
         }break;
    case uidemo2::R::id::acfan:
         if(adapter==nullptr){
             adapter = new AircondPagerAdapter("@layout/acfan",-1);
             adapter->setPageWidth(1.f/2);
             mDevices.insert(std::pair<int,PagerAdapter*>(id,adapter));
         }break;
    }
    if(adapter==nullptr){
         adapter = new HangerPagerAdapter("@layout/todo",-1);
         adapter->setPageWidth(1.f/2);
         mDevices.insert(std::pair<int,PagerAdapter*>(id,adapter));
    }
    mLastDevice = id;
    if(adapter->getCount()==0){
        adapter->add(Device("设备1",true,true));
        adapter->add(Device("设备2",true,false));
        adapter->add(Device("设备3",false,true));
        adapter->add(Device("设备4",false,false));
    }
    mPager->setAdapter(adapter);
}

void DevicePanel::loadDevice(const std::string&deviceitem){
}

DevicePanel::~DevicePanel(){
    LOGD("%d deviceadapters",mDevices.size());
    for(auto d:mDevices){
        PagerAdapter*adapter = d.second;
        delete adapter;
    }
    mDevices.clear();
}

}//endof namespace
