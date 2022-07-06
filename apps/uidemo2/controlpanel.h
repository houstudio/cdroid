#ifndef __CONTROL_PANEL_H__
#define __CONTROL_PANEL_H__
#include <cdroid.h>
#include <homeadapters.h>
namespace cdroid{

class ControlPanel:public FrameLayout{
public:
    ControlPanel(Context*ctx,const AttributeSet&atts);
    void loadFrame(const std::string&resid);
    virtual void init();
};

class SencePanel:public ControlPanel{
private:
    GridView*mGridView;
public:
    SencePanel(Context*ctx,const AttributeSet&atts);
    void init()override;
    ~SencePanel()override;
};

class DevicePanel:public ControlPanel{
private:
    int mLastDevice;/*last select devicetype*/
    ViewPager*mPager;
    std::map<int,PagerAdapter*>mDevices;
protected:
    void onClick(View&);
public:
    DevicePanel(Context*ctx,const AttributeSet&atts);
    void init()override;
    void loadDevice(const std::string&);
    ~DevicePanel()override;
};

}
#endif

