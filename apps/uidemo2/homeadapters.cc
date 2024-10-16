#include <homeadapters.h>
#include <dirent.h>
#include <cdroid.h>
#include <R.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <R.h>
namespace cdroid{

View*FunctionAdapter::getView(int position, View* convertView, ViewGroup* parent){
    const FunctionItem& fn=getItemAt(position);
    TextView*tv=(TextView*)convertView;
    if(convertView==nullptr){
        tv=new TextView(10,10);
        tv->setGravity(Gravity::CENTER_HORIZONTAL);
    }
    tv->setText(fn.text);
    tv->setPadding(0,20,0,20);
    tv->setGravity(Gravity::CENTER);
    tv->setCompoundDrawablesWithIntrinsicBounds("",fn.icon,"","");
    return tv;
}

int FunctionAdapter::load(const std::string&filepath){
    add(FunctionItem("情景","@mipmap/sidenav_icon_scene_nor","@layout/sence"));
    add(FunctionItem("家电","@mipmap/sidenav_icon_housele_sel","@layout/devices"));
    add(FunctionItem("环境","@mipmap/sidenav_icon_environment_nor",""));
    add(FunctionItem("关于","@mipmap/sidenav_icon_about_nor",""));
    add(FunctionItem("消息","@mipmap/sidenav_icon_news_nor_tip",""));
    return 0;
}

////////////////////////////////////////////////////////////////////////////////

LightPagerAdapter::LightPagerAdapter(const std::string&resid,int id)
   :DevicePagerAdapter<Device>(resid,id){
}

void* LightPagerAdapter::instantiateItem(ViewGroup* container, int position){
    Device& t=mItems.at(position);
    t.mSubType=(position%4+1);
    std::string resid=mResourceId+std::to_string(t.mSubType);    
    View*view=LayoutInflater::from(container->getContext())->inflate(resid,nullptr,false);
    container->addView(view);
    setDeviceProperties(view,t);
    return view;

}
///////////////////////////////////////////////////////////////////////////////
CurtainPagerAdapter::CurtainPagerAdapter(const std::string&resid,int id)
    :LightPagerAdapter(resid,id){
}
void* CurtainPagerAdapter::instantiateItem(ViewGroup* container, int position){
    Device& t=mItems.at(position);
    t.mSubType=(position%2+1);
    std::string resid=mResourceId+std::to_string(t.mSubType);
    View*view=LayoutInflater::from(container->getContext())->inflate(resid,nullptr,false);
    container->addView(view);
    setDeviceProperties(view,t);
    return view;    
}

///////////////////////////////////////////////////////////////////////////////

AircondPagerAdapter::AircondPagerAdapter(const std::string&resid,int id)
      :DevicePagerAdapter<Device>(resid,id){
    mItems.push_back(Device("客厅空调",true,true));
    mItems.push_back(Device("主卧空调",true,false));
    mItems.push_back(Device("客房空调",false,true));
    mItems.push_back(Device("书房空调",false,false));
}

void AircondPagerAdapter::setDeviceProperties(View*v,Device&t){
    TextView*tv=(TextView*)v->findViewById(mDeviceId);
    if(tv)tv->setText(t.mName);
    /*set power state*/
    tv =(TextView*)v->findViewById(uidemo2::R::id::power);
    tv->setText(t.mPowerOn?std::to_string(16+time(nullptr)%8):std::string());

    tv->setActivated(t.mPowerOn);

    tv->setOnClickListener([v](View&pwr){
       StateListDrawable*sd=(StateListDrawable*)pwr.getBackground();
       std::vector<int>state=sd->getState();
       pwr.setActivated(!pwr.isActivated());
       TextView*tv=(TextView*)v->findViewById(uidemo2::R::id::fanspeed);

       std::vector<Drawable*>ads=tv->getCompoundDrawables();
       if(ads.size()==0)return;
       AnimatedRotateDrawable*ad=(AnimatedRotateDrawable*)ads.at(TextView::Drawables::TOP);
       if(pwr.isActivated()){
           ad->setFramesCount(16);
           ad->setFramesDuration(100);
           ad->start();
       }else{
           ad->stop();
       }
   });
}

}//endof namespace
